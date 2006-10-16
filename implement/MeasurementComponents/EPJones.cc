//# EPJones.cc: Implementation of Jones classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <synthesis/MeasurementComponents/PBWProjectFT.h>
#include <casa/Containers/Record.h>
#include <calibration/CalTables/SolvableVJDesc.h>
#include <calibration/CalTables/SolvableVJMRec.h>
#include <calibration/CalTables/SolvableVJMCol.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/CalDescColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <synthesis/MeasurementComponents/EPJones.h>
#include <msvis/MSVis/VisBuffer.h>
#include <calibration/CalTables/EPointTable.h>
#include <calibration/CalTables/EPointMCol.h>
#include <calibration/CalTables/EPointVJMRec.h>

#include <coordinates/Coordinates/LinearCoordinate.h>


#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/RefRows.h>

#include <casa/sstream.h>

#include <casa/Arrays.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/LogiArray.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Exceptions/Error.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Stokes.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>

#include <casa/iostream.h>

namespace casa {
EPJones::EPJones()
  :EPTimeVarVisJones(),ftmac_p(0)
{
};

// ------------------------------------------------------------------
// Start of methods for the solveable version i.e. EPJones

// Generic ctor
EPJones::EPJones(VisSet& vs,PBWProjectFT &ftmachine)
  : EPTimeVarVisJones(vs),ftmac_p(&ftmachine)
{
  nPar_ = 2;
  // Set required element mask
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;
};
// Generic ctor
EPJones::EPJones(VisSet& vs)
  : EPTimeVarVisJones(vs),ftmac_p(0)
{
  nPar_ = 2;
  // Set required element mask
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;
};

//EPJones::EPJones(const EPJones& other)
//{
//  //  operator=(other);
//}

EPJones::~EPJones()
{
  //  deleteSolveCache();
}
//
//---------------------------------------------------------------------
//
#undef ASSIGN
#ifdef ASSIGN
// Assignment
EPJones& EPJones::operator=(const EPJones& other)
{
  if(this!=&other)
    {
      EPJones<Type>::operator=(other);
      vs_=other.vs_;
      sumwt_=other.sumwt_;
      chisq_=other.chisq_;
      gS_.resize(other.gS_.shape());
      gS_=other.gS_;
      ggS_.resize(other.ggS_.shape());
      ggS_=other.ggS_;
      fit_=other.fit_;
      fitwt_=other.fitwt_;
      solutionOK_=other.solutionOK_;
      iFit_=other.iFit_;
      iFitwt_=other.iFitwt_;
      iSolutionOK_=other.iSolutionOK_;
      hasData = other.hasData;
    }
  return *this;
}
#endif
//
//---------------------------------------------------------------------
//
void EPJones::setSolverParam(const String& tableName,
			     const Double& interval,
			     const Double& preavg)
{
  interval_=interval;
  preavg_=preavg;
  //  refant_=refant;
  //  if (phaseonly) setMode("phase");
  calTableName_=tableName;

  // Make local VisSet  (this will be used extensively!)
  makeLocalVisSet();

  // Initialize caches
  initMetaCache();
  fillMetaCache();

  initAntGain();

  initSolveCache();

  initThisGain();

  // This is the solve context
  setSolved(True);
  setApplied(False);
}
//
//---------------------------------------------------------------------
//
void EPJones::store (const String& file, const Bool& append)
{
  //
  // Write the solutions to an output calibration table
  // Input:
  //    file           String        Cal table name
  //    append         Bool          Append if true, else overwrite
  //
  // Initialization:
  // No. of rows in cal_main, cal_desc and cal_history
  //
  Int nMain = 0; 
  Int nDesc = 0;
  Int nHist = 0;
  //
  // Calibration table
  //
  EPointTable *tab;
  //
  // Open the output file if it already exists and is being appended to.
  //
  if (append && Table::isWritable (file)) 
    {
      tab  = new EPointTable (file, Table::Update);
      nMain = tab->nRowMain();
      nDesc = tab->nRowDesc();
      nHist = tab->nRowHistory();
    } 
  else 
    {
      //
      // Create a new calibration table
      //
      Table::TableOption access = Table::New;
      tab = new EPointTable (file, typeName(), access);
    };
  //
  // Write every spw w/ max number of channels 
  //  (eventually, CalTable should permit variable-shape cols)
  //
  Int maxNumChan(1);
  if (freqDep()) maxNumChan=max(nSolnChan_);
  //
  // Some default values
  //
  Double dzero = 0;
  IPosition ip(2,1,maxNumChan);
  //
  // CalDesc Sub-table records
  //
  CalDescRecord* descRec;
  Vector<Int> calDescNum(numberSpw_); calDescNum=-1;
  for (Int iSpw=0; iSpw<numberSpw_; iSpw++)
    {
      //
      // Write a CalDesc for each spw which has solutions
      // Note: CalDesc index != SpwId, in general
      //
      if (antGainPar_[iSpw]!=NULL) {
	//
	// Access to existing CAL_DESC columns:
	//
	CalDescColumns cd(*tab);
	//
	// Check if this spw already in CAL_DESC 
	//      cout << "spwCol = " << cd.spwId().getColumn() << endl;
	//
	Bool newCD(True);
	for (Int iCD=0;iCD<nDesc;iCD++)
	  {
	    IPosition iCDip(1,0);
	    if ( iSpw==(cd.spwId()(iCD))(iCDip) )
	      {
		//
		// Don't need new CAL_DESC entry
		//
		newCD=False;
		calDescNum(iSpw)=iCD;
		break;
	      }
	  }

	if (newCD)
	  {
	    //
	    // Cal_desc fields
	    //
	    Vector <Int> spwId(1,iSpw);
	    Matrix <Double> chanFreq(ip, dzero); 
	    Matrix <Double> chanWidth(ip, dzero);
	    Array <String> polznType(ip, "");
	    Cube <Int> chanRange(IPosition(3,2,1,maxNumChan), 0);
	    Vector <Int> numChan(1,nSolnChan_(iSpw));
	    for (Int ichan=0; ichan<nSolnChan_(iSpw); ichan++)
	      {
		chanRange(0,0,ichan)=startChan_(iSpw);
		chanRange(1,0,ichan)=startChan_(iSpw) + nSolnChan_(iSpw) -1;
	      }
	    //	
	    // Fill the cal_desc record
	    //
	    descRec = new CalDescRecord;
	    descRec->defineNumSpw (1);
	    descRec->defineNumChan (numChan);
	    descRec->defineNumReceptors (2);
	    descRec->defineNJones (2);
	    descRec->defineSpwId (spwId);
	    descRec->defineChanFreq (chanFreq);
	    descRec->defineChanWidth (chanWidth);
	    descRec->defineChanRange (chanRange);
	    descRec->definePolznType (polznType);
	    descRec->defineJonesType ("full");
	    descRec->defineMSName ("");
	    //
	    // Write the cal_desc record
	    //
	    tab->putRowDesc (nDesc, *descRec);
	    delete descRec;
	    //
	    // This spw will have this calDesc index in main table
	    //
	    calDescNum(iSpw) = nDesc;
	    nDesc++;
	  }
      }
    }

  //
  // Now write MAIN table in column-wise fashion
  //
  // Starting row in this slot
  //
  for (Int iSpw=0; iSpw<numberSpw_; iSpw++)
    {
      //
      // Write table for spws which have solutions
      //
      if (antGainPar_[iSpw]!=NULL) 
	{
	  //
	  // Create references to cal data for this spw
	  //
	  Vector<Bool>    thisSolOK;        thisSolOK.reference(*(solutionOK_[iSpw]));
	  Vector<Double>  thisMJDTimeStamp; thisMJDTimeStamp.reference(*(MJDTimeStamp_[iSpw]));
	  Vector<Double>  thisMJDStart;     thisMJDStart.reference(*(MJDStart_[iSpw]));
	  Vector<Double>  thisMJDStop;      thisMJDStop.reference(*(MJDStop_[iSpw]));
	  Vector<Int>     thisFieldId;      thisFieldId.reference(*(fieldId_[iSpw]));
	  Vector<String>  thisFieldName;    thisFieldName.reference(*(fieldName_[iSpw]));
	  Vector<String>  thisSourceName;   thisSourceName.reference(*(sourceName_[iSpw]));
	  Vector<Float>   thisFit;          thisFit.reference(*(fit_[iSpw]));
	  Vector<Float>   thisFitwt;        thisFitwt.reference(*(fitwt_[iSpw]));
	  Array<Float>  thisAntGain;      thisAntGain.reference(*(antGainPar_[iSpw]));
	  Matrix<Bool>    thisISolutionOK;  thisISolutionOK.reference(*(iSolutionOK_[iSpw]));
	  Matrix<Float>   thisIFit;         thisIFit.reference(*(iFit_[iSpw]));
	  Matrix<Float>   thisIFitwt;       thisIFitwt.reference(*(iFitwt_[iSpw]));
      
	  //
	  // total rows to be written this Spw
	  //

	  Int nRow; nRow=numberAnt_*ntrue(thisSolOK);
	  //
	  // These are constant columns (with boring values, currently)
	  //
	  Vector<Double> timeEP(nRow,0.0);
	  Vector<Int> feed1(nRow,0);
	  Vector<Int> arrayId(nRow,0);
	  Vector<Int> obsId(nRow,0);
	  Vector<Int> scanNum(nRow,0);
	  Vector<Int> procId(nRow,0);
	  Vector<Int> stateId(nRow,0);
	  Vector<Int> phaseId(nRow,0);
	  Vector<Int> pulsarBin(nRow,0);
	  Vector<Int> pulsarGateId(nRow,0);
	  Vector<Int> freqGroup(nRow,0);
	  Vector<Int> calHistId(nRow,0);
	  //
	  // This is constant
	  //
	  Vector<Int> calDescId(nRow,calDescNum(iSpw));
	  //
	  // These are constant per slot
	  //   (these cols should be incremental)
	  //
	  Vector<Double> time(nRow,0.0);
	  Vector<Double> interval(nRow,0.0);
	  Vector<Int>    fieldId(nRow,0);
	  Vector<String> fieldName(nRow,"");
	  Vector<String> sourceName(nRow,"");
	  Vector<Bool>   totalSolOk(nRow,False);
	  Vector<Float>  totalFit(nRow,0.0);
	  Vector<Float>  totalFitWt(nRow,0.0);
	  //
	  // These vary
	  //
	  Vector<Int>    antenna1(nRow,0);
	  Array<Float>   pointingOffsetCol(IPosition(2,2,nRow));
	  //	  Array<Complex> gain(IPosition(5,2,2,1,maxNumChan,nRow),Complex(0.0,0.0));
	  Cube<Bool>     solOk(1,maxNumChan,nRow,False);
	  Cube<Float>    fit(1,maxNumChan,nRow,0.0);
	  Cube<Float>    fitWt(1,maxNumChan,nRow,0.0);

	  //	  IPosition out(5,0,0,0,0,0);
	  //	  IPosition in(4,0,0,0,0);
	  IPosition out(2,0,0),in(4,0,0,0,0);
	  Int thisRow(0);
	  for (Int islot = 0; islot < numberSlot_(iSpw); islot++) 
	    {
	      in(3)=islot;
	      if (thisSolOK(islot)) 
		{
		  //
	  	  // Fill slot-constant cols:
		  //
		  Slice thisSlice(thisRow,numberAnt_);
		  time(thisSlice)=thisMJDTimeStamp(islot);
		  interval(thisSlice)=(thisMJDStop(islot) - thisMJDStart(islot));
		  fieldId(thisSlice)=thisFieldId(islot);
		  fieldName(thisSlice)=thisFieldName(islot);
		  sourceName(thisSlice)=thisSourceName(islot);
		  totalSolOk(thisSlice)=thisSolOK(islot);
		  totalFit(thisSlice)=thisFit(islot);
		  totalFitWt(thisSlice)=thisFitwt(islot);
		  //
		  // Loop over the number of antennas
		  //
	  	  for (Int iant = 0; iant < numberAnt_; iant++) 
		    {
		      out(1)=thisRow;
		      in(2)=iant;
		      // Antenna index
		      antenna1(thisRow)=iant;
		      //
		      // Gain  (make this matrix type specific!)
		      //
		      for (Int ichan=0; ichan<nSolnChan_(iSpw); ichan++) 
			{
			  in(1)=ichan;
			  switch (nPar_) 
			    {
			    case 1: 
			      {
				in(0)=0;
				for (Int i=0; i<2; i++) 
				  {
				    out(0)=out(1)=i;
				    //				    gain(out)=thisAntGain(in);
				  }
				break;
			      }
			    case 2: 
			      {
				for (Int i=0; i<2; i++) 
				  {
				    out(0)=i;
				    in(0)=i;
				    pointingOffsetCol(out)=thisAntGain(in);
				  }
				break;
			      }
			    case 4: 
			      {
				for (Int i = 0; i < 2; i++) 
				  for (Int j = 0; j < 2; j++) 
				    {
				      out(0)=i; out(1)=j;
				      in(0)=2*i+j;
				      //				      gain(out)=thisAntGain(in);
				    };
				break;
			      };
			    }
			  //
			  // Gain stats  (slot constant, per spw?)
			  //
			  solOk(0,ichan,thisRow) = thisISolutionOK(iant,islot);
			  fit(0,ichan,thisRow) = thisIFit(iant,islot);
			  fitWt(0,ichan,thisRow) = thisIFitwt(iant,islot);
			}
		      //
		      // next time round is next row
		      //
		      thisRow++;
		    };
		};
	    };
	  //
	  // Now push everything to the disk table
	  //
	  tab->addRowMain(nRow);
	  EPointMCol svjmcol(*tab);
	  RefRows refRows(nMain,nMain+nRow-1);
	  svjmcol.time().putColumnCells(refRows,time);
	  svjmcol.timeEP().putColumnCells(refRows,timeEP);
	  svjmcol.interval().putColumnCells(refRows,interval);
	  svjmcol.antenna1().putColumnCells(refRows,antenna1);
	  svjmcol.feed1().putColumnCells(refRows,feed1);
	  svjmcol.fieldId().putColumnCells(refRows,fieldId);
	  svjmcol.arrayId().putColumnCells(refRows,arrayId);
	  svjmcol.obsId().putColumnCells(refRows,obsId);
	  svjmcol.scanNo().putColumnCells(refRows,scanNum);
	  svjmcol.processorId().putColumnCells(refRows,procId);
	  svjmcol.stateId().putColumnCells(refRows,stateId);
	  svjmcol.phaseId().putColumnCells(refRows,phaseId);
	  svjmcol.pulsarBin().putColumnCells(refRows,pulsarBin);
	  svjmcol.pulsarGateId().putColumnCells(refRows,pulsarGateId);
	  svjmcol.freqGrp().putColumnCells(refRows,freqGroup);
	  svjmcol.fieldName().putColumnCells(refRows,fieldName);
	  svjmcol.sourceName().putColumnCells(refRows,sourceName);
	  //	  svjmcol.gain().putColumnCells(refRows,gain);
	  svjmcol.pointingOffset().putColumnCells(refRows,pointingOffsetCol);
	  svjmcol.totalSolnOk().putColumnCells(refRows,totalSolOk);
	  svjmcol.totalFit().putColumnCells(refRows,totalFit);
	  svjmcol.totalFitWgt().putColumnCells(refRows,totalFitWt);
	  svjmcol.solnOk().putColumnCells(refRows,solOk);
	  svjmcol.fit().putColumnCells(refRows,fit);
	  svjmcol.fitWgt().putColumnCells(refRows,fitWt);
	  svjmcol.calDescId().putColumnCells(refRows,calDescId);
	  svjmcol.calHistoryId().putColumnCells(refRows,calHistId);

	  nMain = tab->nRowMain();
      
	  //  cout << typeName() << ": store(columnwise) execution    = " << timer.all_usec()/1e6 << endl;
	  //  cout << typeName() << ": store(columnwise) memory usage = " << Memory::allocatedMemoryInBytes() - startMem << endl;
	}
    }
  delete tab;
};
// Initialize solve-related caches
//
//---------------------------------------------------------------------
//
void EPJones::initSolveCache()
{
  // Delete the cache, in case it already exists
  deleteSolveCache();

  // Resize PBs according to number of Spws
  iSolutionOK_.resize(numberSpw_); iSolutionOK_=NULL;
  iFit_.resize(numberSpw_);        iFit_=NULL;
  iFitwt_.resize(numberSpw_);      iFitwt_=NULL;
  solutionOK_.resize(numberSpw_);  solutionOK_=NULL;
  fit_.resize(numberSpw_);         fit_=NULL;
  fitwt_.resize(numberSpw_);       fitwt_=NULL;

  // Construct and resize pointed-to objects for each available spw
  for (Int ispw=0; ispw<numberSpw_; ispw++)
    {
      uInt nslot=numberSlot_(ispw);
      if (nslot > 0)
	{
	  newPB_(ispw) = True;
	  iSolutionOK_[ispw] = new Matrix<Bool>(numberAnt_,nslot,False);
	  iFit_[ispw]        = new Matrix<Float>(numberAnt_,nslot,0.0);
	  iFitwt_[ispw]      = new Matrix<Float>(numberAnt_,nslot,0.0);
	  solutionOK_[ispw]  = new Vector<Bool>(nslot,False);
	  fit_[ispw]         = new Vector<Float>(nslot,0.0);
	  fitwt_[ispw]       = new Vector<Float>(nslot,0.0);
	}
    }
}
//
//---------------------------------------------------------------------
//
void EPJones::deleteSolveCache()
{
  uInt nCache=iSolutionOK_.nelements();
  if (nCache > 0)
    {
      for (Int ispw=0; ispw<numberSpw_; ispw++)
	{
	  if (newPB_(ispw))
	    {
	      if (iSolutionOK_[ispw]) delete iSolutionOK_[ispw];
	      if (iFit_[ispw])        delete iFit_[ispw];
	      if (iFitwt_[ispw])      delete iFitwt_[ispw];
	      if (solutionOK_[ispw])  delete solutionOK_[ispw];
	      if (fit_[ispw])         delete fit_[ispw];
	      if (fitwt_[ispw])       delete fitwt_[ispw];
	    }
	  iSolutionOK_[ispw]=NULL;
	  iFit_[ispw]=NULL;
	  iFitwt_[ispw]=NULL;
	  solutionOK_[ispw]=NULL;
	  fit_[ispw]=NULL;
	  fitwt_[ispw]=NULL;
	}
    }
}
//
//---------------------------------------------------------------------
//
void EPJones::load (const String& file, 
			       const String& select, 
			       const String& type)
{
  // Load data from a calibration table
  // Input:
  //    file         const String&       Cal table name
  //    select       const String&       Selection string
  //    type         const String&       Jones matrix type
  //                                    (scalar, diagonal or general)
  //
  //
  LogMessage message(LogOrigin("EPJones","load"));
  //  
  // Decode the Jones matrix type
  //
  Int jonesType = 0;
  if (type == "scalar") jonesType = 1;
  if (type == "diagonal") jonesType = 2;
  if (type == "general") jonesType = 3;
  //
  // Open, select, sort the calibration table
  //
  EPointTable svjtab(file);
  svjtab.select2(select);
  //
  // Get no. of antennas and time slots
  //
  numberAnt_ = svjtab.maxAntenna() + 1;

  Int nDesc=svjtab.nRowDesc();
  Vector<Int> spwmap(nDesc,-1);
  for (Int idesc=0;idesc<nDesc;idesc++)
    {
      //
      // This cal desc
      //
      //      CalDescRecord* calDescRec = new CalDescRecord (svjtab.getRowDesc(idesc));
      CalDescRecord calDescRec(svjtab.getRowDesc(idesc));
      //
      // Get this spw ID
      //
      Vector<Int> spwlist;
      calDescRec.getSpwId(spwlist);
      Int nSpw; spwlist.shape(nSpw);
      if (nSpw > 1) {};  // ERROR!!!  Should only be one spw per cal desc!
      spwmap(idesc)=spwlist(0);
      //
      // In next few steps, need to watch for repeat spws in new cal descs!!
      //
      // Get number of channels this spw
      //
      Vector<Int> nchanlist;
      calDescRec.getNumChan(nchanlist);
      //      cout << nchanlist << " " << nchanlist(0) << " " << spwmap(idesc) << endl;
      nSolnChan_(spwmap(idesc))=nchanlist(0);
      //
      // Get channel range / start channel
      //
      Cube<Int> chanRange;
      calDescRec.getChanRange(chanRange);
      startChan_(spwmap(idesc))=chanRange(0,0,0);
      //
      // Get slot count for this desc
      //
      ostringstream thisDesc;
      thisDesc << "CAL_DESC_ID==" << idesc;
      CalTable thisDescTab = svjtab.select(thisDesc.str());
      numberSlot_(spwmap(idesc))=thisDescTab.nRowMain()/numberAnt_;
    }
  //  
  // Get solution interval (assumed constant across table)
  //  EPointVisJonesMRec* solvableVJMRec = 
  //    new EPointVisJonesMRec (svjtab.getRowMain(0));
  SolvableVisJonesMRec* solvableVJMRec = 
    new SolvableVisJonesMRec (svjtab.getRowMain(0));
  solvableVJMRec->getInterval (interval_);
  deltat_ = 0.01 * interval_;
  delete solvableVJMRec;
  //
  // At this point, we know how big our slot-dep caches must be
  //  (in private data), so initialize them
  //
  initMetaCache();
  initAntGain();
  initSolveCache();
  //
  // Remember if we found and filled any solutions
  //
  Bool solfillok(False);
  //
  // Fill per caldesc
  //
  for (Int idesc=0;idesc<nDesc;idesc++) 
    {
      Int thisSpw=spwmap(idesc);
      //
      // Reopen and globally select caltable
      //
      EPointTable svjtabspw(file);
      svjtabspw.select2(select);
      //
      // isolate this caldesc:
      //
      ostringstream selectstr;
      selectstr << "CAL_DESC_ID == " << idesc;
      String caldescsel; caldescsel = selectstr.str();
      svjtabspw.select2(caldescsel);

      Int nrow = svjtabspw.nRowMain();
      //      IPosition out(5,0,0,0,0,0);
      IPosition out(2,0,0);
      IPosition in(4,0,0,0,0);
      if (nrow>0) 
	{
	  //
	  // Found some solutions to fill
	  //
	  solfillok=True;
	  //
	  // Ensure sorted on time
	  //
	  Block <String> sortCol(1,"TIME");
	  svjtabspw.sort2(sortCol);
	  //
	  // Extract the gain table columns
	  //
	  ROEPointMCol svjmcol(svjtabspw);
	  Vector<Int>    calDescId;  svjmcol.calDescId().getColumn(calDescId);
	  Vector<Double> time;       svjmcol.time().getColumn(time);
	  Vector<Double> interval;   svjmcol.interval().getColumn(interval);
	  Vector<Int>    antenna1;   svjmcol.antenna1().getColumn(antenna1);
	  Vector<Int>    fieldId;    svjmcol.fieldId().getColumn(fieldId);
	  Vector<String> fieldName;  svjmcol.fieldName().getColumn(fieldName);
	  Vector<String> sourceName; svjmcol.sourceName().getColumn(sourceName);
	  Vector<Bool>   totalSolOk; svjmcol.totalSolnOk().getColumn(totalSolOk);
	  Vector<Float>  totalFit;   svjmcol.totalFit().getColumn(totalFit);
	  Vector<Float>  totalFitWt; svjmcol.totalFitWgt().getColumn(totalFitWt);
	  try{
	    Array<Complex> gain;       svjmcol.gain().getColumn(gain);
	  } catch (AipsError& x) {
	    cout << x.getMesg() << endl << " No GAIN column found.  That's OK." << endl;
	  }
	  
	  Cube<Bool>     solOk;      svjmcol.solnOk().getColumn(solOk);
	  Cube<Float>    fit;        svjmcol.fit().getColumn(fit);
	  Cube<Float>    fitWt;      svjmcol.fitWgt().getColumn(fitWt);
	  Array<Float>   pointingOffset; svjmcol.pointingOffset().getColumn(pointingOffset);

	  //	  cout << "Pointing Offset shape: " << pointingOffset.shape() << endl;

	  //
	  // Read the calibration information
	  //
	  deltat_ = 0.01 * interval(0);
	  Double lastTime(-1.0), thisTime(0.0), thisInterval(0.0);
	  Int islot(-1);
	  Int iant;
      
	  for (Int irow=0; irow<nrow; irow++) 
	    {
	      out(1)=irow;
	      thisTime=time(irow);
	      //
	      // If this is a new timestamp
	      //
	      if (abs (thisTime - lastTime) > deltat_) 
		{
	  	  islot++;
		  in(3)=islot;
		  
		  thisInterval=interval(irow);
		  (*MJDTimeStamp_[thisSpw])(islot) = thisTime;
		  (*MJDStart_[thisSpw])(islot) = thisTime - thisInterval / 2.0;
		  (*MJDStop_[thisSpw])(islot) = thisTime + thisInterval / 2.0;
		  (*fieldId_[thisSpw])(islot) = fieldId(irow);
		  (*fieldName_[thisSpw])(islot) = fieldName(irow);
		  (*sourceName_[thisSpw])(islot) = sourceName(irow);
		  
		  (*solutionOK_[thisSpw])(islot) = totalSolOk(irow);
		  (*fit_[thisSpw])(islot) = totalFit(irow);
		  (*fitwt_[thisSpw])(islot) = totalFitWt(irow);
		  
		  lastTime = thisTime;
		};
	
	      iant=antenna1(irow);
	      in(2)=iant;

	      (*iSolutionOK_[thisSpw])(iant,islot) = solOk(0,0,irow);
	      (*iFit_[thisSpw])(iant,islot) = fit(0,0,irow);
	      (*iFitwt_[thisSpw])(iant,islot) = fitWt(0,0,irow);
	
	      for (Int ichan=0; ichan<nSolnChan_(thisSpw); ichan++) 
		{
		  (*antGainParOK_[thisSpw])(ichan,iant,islot) = solOk(0,ichan,irow);
		  //		  out(3)=in(1)=ichan;
		  in(1)=ichan;
		  switch (jonesType) 
		    {
		    case 1: 
		      // scalar (diagonal mode will work)
		    case 2: 
		      {
			//
			// diagonal
			//
			for (Int ipar=0; ipar<nPar_; ipar++) 
			  {
			    //			    out(0)=out(1)=in(0)=ipar;
			    out(0)=in(0)=ipar;
			    (*antGainPar_[thisSpw])(in) = pointingOffset(out);
			  }
			break;
		      };
		    case 3: 
		      {
			//
			// general 
			//
			throw(AipsError(" EPoint is not a Jones matrix of type general"));
			/*
			Int i,j;
			for (i = 0; i < 2; i++)
			  for (j = 0; j < 2; j++) 
			    {
			      out(0)=i;out(1)=j;
			      in(0)=i*2+j;
			      (*antGainPar_[thisSpw])(in) = pointingOffset(out);
			    };
			*/
			break;
		      };
		    }; // switch (jonesType)...
		}
	    } // irow
	  //	      cout << (*antGainPar_[thisSpw]) << endl;
	} // nrow>0
    } // idesc

  // If we found no solutions in selected table, abort:
  if (!solfillok) 
    {
      throw(AipsError(" Specified cal table selection selects no solutions in this table."
		      "  Please review setapply settings."));
    }
  hasData=True;
};
//
//---------------------------------------------------------------------
//
Cube<Float>& EPJones::getPar(const VisBuffer& vb)
{
  initThisGain();
  getThisGain(vb,True,False,True);
  return (*(thisAntGainPar_[currentCalSpw_]));
}
//
//---------------------------------------------------------------------
//
Vector<Bool>& EPJones::getFlags(const VisBuffer& vb)
{
  initThisGain();
  getThisGain(vb,True,False,True);
  return (*(thisAntGainOK_[currentCalSpw_]));
}
//
//---------------------------------------------------------------------
//
void EPJones::reset() {ftmac_p->reset();};
//
//---------------------------------------------------------------------
//
VisBuffer& EPJones::apply(VisBuffer& vb, 
			  VisBuffer &gradAzVB,
			  VisBuffer &gradElVB, Int Conj) // Conj=0 by default
{
  //
  // Obtain gain matrices appropriate for this VisBuffer
  //
  initThisGain();
  getThisGain(vb,True,False,True);
  //  
  // Compute the corrupted visibilities (degrid model visibilities
  // using E_{ij} as the gridding convolution function).
  // FTMachine::get iterates over the rows in the VisBuffer.
  //
  Cube<Float>    thisAGP;   thisAGP.  reference( *(thisAntGainPar_[currentCalSpw_]) );
  Vector<Bool>   thisAGPOK; thisAGPOK.reference( *(thisAntGainOK_[currentCalSpw_]) );

  Int DoAllRows=-1;
  //
  // Fill the DATA column of the MS chunk (VisBuffer::visCube()) with
  // the predicted visibilities.
  //
  //  cout << vb.nRow() << " " << vb.uvw().nelements() << endl;
  //  cout << "EPJones::apply(): thisAGP.shape() " << thisAGP.shape() << endl;

  ftmac_p->get(vb,gradAzVB,gradElVB,thisAGP,DoAllRows,
	       FTMachine::MODEL,
	       FTMachine::MODEL,Conj);

  return vb;
}
//
//---------------------------------------------------------------------
//
VisBuffer& EPJones::apply(VisBuffer& vb, Int Conj)
{
  //
  // Obtain gain matrices appropriate for this VisBuffer
  //
  initThisGain();
  getThisGain(vb,True,False,True);
  //  
  // Compute the corrupted visibilities (degrid model visibilities
  // using E_{ij} as the gridding convolution function).
  // FTMachine::get iterates over the rows in the VisBuffer.
  //
  Cube<Float>    thisAGP;   thisAGP.  reference( *(thisAntGainPar_[currentCalSpw_]) );
  Vector<Bool>   thisAGPOK; thisAGPOK.reference( *(thisAntGainOK_[currentCalSpw_]) );

  Int DoAllRows=-1;
  //
  // Fill the DATA column of the MS chunk (VisBuffer::visCube()) with
  // the predicted visibilities.
  //
  //  cout << vb.nRow() << " " << vb.uvw().nelements() << endl;
  ftmac_p->get(vb,thisAGP,DoAllRows,FTMachine::MODEL,Conj);

  return vb;
}
//
//---------------------------------------------------------------------
//
// resize and initialize 1st/2nd derivatives
//
void EPJones::initializeGradients() {

  gS_.resize(nSolnChan_(currentSpw_),numberAnt_);
  Matrix<Complex> gS(2,2); gS=Complex(0.,0.);
  gS_=gS;

  ggS_.resize(nSolnChan_(currentSpw_),numberAnt_);
  Matrix<Float> ggS(2,2); ggS=0.0;
  ggS_=ggS;

  sumwt_=0.0;
  chisq_=0.0;

}
//
//---------------------------------------------------------------------
//
//void EPJones::finalizeGradients() {}
//
//---------------------------------------------------------------------
//
// Add to Gradient Chisq
//
void EPJones::addGradients(const VisBuffer& vb, Int row,
				      const Antenna& a,
				      const Vector<Float>& sumwt,
				      const Vector<Float>& chisq,
				      const Vector<SquareMatrix<Complex,2> >& c, 
				      const Vector<SquareMatrix<Float,2> >& f) 
{
  if (!vb.flagRow()(row)) 
    for (Int chn=0; chn<vb.nChannel(); chn++) 
      if (!vb.flag()(chn,row)) 
	{
	  sumwt_+=sumwt(chn);
	  chisq_+=chisq(chn);
	  gS_(chn,a)+=c(chn);
	  ggS_(chn,a)+=f(chn);
	}
}
//
//---------------------------------------------------------------------
//
// Solve for the  Jones matrix. Updates the VisJones thus found.
// Also inserts it into the VisEquation thus it is not const.
//
Bool EPJones::solve (VisEquation& me)
{
  LogMessage message(LogOrigin("SolvableVisJones","solve"));
  //
  // Set the fit tolerance (convergence is when change in chisq is
  // less than tolerance times the value of chisq) Force B and D
  // solutions to go deeper, just to make sure...
  //
  if (typeName()=="B Jones" || typeName()=="D Jones") setTolerance(0.001);
  else setTolerance(0.1);

  // Save required_, we might change it on-the-fly (single-corr data)
  Matrix<Bool> origreq(required_);

  AlwaysAssert(gain()>0.0,AipsError);
  AlwaysAssert(numberIterations()>0,AipsError);
  AlwaysAssert(tolerance()>0.0,AipsError);
  //
  // Make a local copy of the Measurement Equation so we can change
  // some of the entries
  //
  VisEquation lme(me);

  //
  // Set the Jones matrix in the local copy of the Measurement Equation 
  //
  lme.setVisJones(*this);
  //n
  // Tell the VisEquation to use the internal (chunked) VisSet
  //
  AlwaysAssert(vs_, AipsError);
  lme.setVisSet(*vs_);

  // Count number of failed/zerowt intervals
  Int failed=0, zerowt=0;
  
  VisIter& vi(vs_->iter());
  VisBuffer vb(vi);

  // Iterate chunks
  Int chunk;
  Vector<Int> lastslot(numberSpw_); lastslot=-1;
  Int lastfld=-1;
  currentSlot_=0;
  for(chunk=0, vi.originChunks(); 
      vi.moreChunks(); 
      vi.nextChunk(),chunk++,currentSlot_(currentSpw_)++) {

    // current field Id
    Int fld=vi.fieldId();

    // find currentSpw_
    currentSpw_=vi.spectralWindow();

    // Check correlation population, adjust required_ as necessary
    required_=origreq;
    Int ncorr; 
    Vector<Int> corrtype(vb.corrType());
    corrtype.shape(ncorr);
    if (ncorr==1 && typeName()=="T Jones") {
      Int pol=corrtype(0)%4;

      required_=False;
      if (pol==1) {
	// This means R-only or X-only
	required_(0,0)=True;
      } else {
	// This means L-only or Y-only
	required_(1,1)=True;
      }
    }

    //    cout << "chunk=" << chunk+1;
    //    cout << " currentSlot_=" << currentSlot_+1;
    //    cout << " lastslot=" << lastslot(currentSpw_)+1;
    //    cout << " currentSpw_=" << currentSpw_+1;
    //    cout << " fld=" << fld+1;
    //    cout << " lastfld=" << lastfld+1;
    //    cout << endl;

    // Avoiding per-solution logging for now; will add verbose option later
    //    {
    //      ostringstream o; o<<typeName()<<" Slot="<<chunk+1<<", " 
    //		     << fieldName_(chunk)<<", currentSpw_="<<currentSpw_+1<<": "
    //		     << MVTime(MJDStart_(currentSlot_)/86400.0)<<" to "
    //		     << MVTime(MJDStop_(currentSlot_)/86400.0);
    //      message.message(o);
    //      logSink().post(message);
    //    }
    
    // initialize ChiSquare calculation
    lme.initChiSquare(*this);

 /*
    cout << "chunk=" << chunk << endl;
    for (Int row=0; row < lme.corrected().nRow(); row++) {
      if (lme.corrected().antenna1()(row)!=lme.corrected().antenna2()(row)) {
	cout << "  ";
	cout << row;
	cout << " " << lme.corrected().antenna1()(row);
	cout << "-" << lme.corrected().antenna2()(row);
	cout << " vobs=";
	cout << lme.corrected().visibility()(0,row);
	cout << " vmod=";
	cout << lme.corrupted().visibility()(0,row);
	cout << endl;
      }
    }
 */

    // initialize gradient/chisq calculation
    initializeGradients();

    // Set nominal solution to OK status (will adjust later as necessary)
    iSolutionOK_[currentSpw_]->column(currentSlot_(currentSpw_))=True;
    iFit_[currentSpw_]->column(currentSlot_(currentSpw_))=0.0;
    iFitwt_[currentSpw_]->column(currentSlot_(currentSpw_))=0.0;

    // Invalidate the baseline corrections:
    //    invalidateIGM(currentSpw_);

    // Find gradient and Hessian
    lme.gradientsChiSquared(required_,*this);

    // Assess fit
    Float currentChisq=chisq_;
    Float rms = 0;
    
    Float originalChisq=currentChisq;
    Float previousChisq=currentChisq;
    Float lowestChisq=currentChisq;
    Bool fail=False;

    (*fit_[currentSpw_])(currentSlot_(currentSpw_))=0.0;
    (*fitwt_[currentSpw_])(currentSlot_(currentSpw_))=0.0;

    Vector<Float> slotFit(iFit_[currentSpw_]->column(currentSlot_(currentSpw_)));
    Vector<Float> slotFitwt(iFitwt_[currentSpw_]->column(currentSlot_(currentSpw_)));

    if(sumwt_==0) 
      {
	ostringstream o; 
	o<< "Insufficient data:  "<< "Slot="<<chunk+1<<", " 
	 << (*fieldName_[currentSpw_])(currentSlot_(currentSpw_))
	 <<", spw="<<currentSpw_+1<<": "
	 << MVTime((*MJDStart_[currentSpw_])(currentSlot_(currentSpw_))/86400.0)<<" to "
	 << MVTime((*MJDStop_[currentSpw_])(currentSlot_(currentSpw_))/86400.0);
	message.message(o);
	logSink().post(message);
	fail=True;
	zerowt++;

	//break;
      } 
    else if (currentChisq==0) 
      {
	ostringstream o; o<<typeName()<<"    Current Chisq is already zero";
	message.message(o);
	logSink().post(message);
	
	//break;
      } 
    else 
      {
	AlwaysAssert(currentChisq>0.0, AipsError); 
	AlwaysAssert(sumwt_>0.0, AipsError); 
	rms = sqrt(currentChisq/sumwt_);

	// Iterate    
	Int iter;
	Bool converged(False);
	Bool almostconverged(False);
	for (iter=0;(!fail)&&(iter<numberIterations());iter++)
	  {
	    //
	    // Update antenna gains from gradients. This can be
	    // different for derived classes
	    //
	    updateAntGain();
	    //
	    // Invalidate baseline cache (because antenna solutions
	    // just changed)
	    //
	    validateAGM(currentSpw_);
	    //	    invalidateIGM(currentSpw_);
	    //
	    // Find gradient and Hessian
	    //
	    initializeGradients();
	    lme.gradientsChiSquared(required_,*this);
	    //
	    // Assess fit
	    //
	    previousChisq=currentChisq;
	    currentChisq=chisq_;
	    lowestChisq=min(lowestChisq, currentChisq);

	    AlwaysAssert(sumwt_>0.0, AipsError);
	    AlwaysAssert(chisq_>=0.0, AipsError);
	    rms = sqrt(chisq_/sumwt_);
	    //
	    // Converged?
	    //
	    if(currentChisq<=previousChisq && 
	       //	   iter > 20 &&
	       abs(currentChisq-previousChisq)<tolerance()*currentChisq) 
	      {
		if (almostconverged) 
		  {
		    converged=True;
		    break;
		  };
		almostconverged=True;
		//	  cout << " Almost converged!";
	      } 
	    else 
	      {
		almostconverged=False;
	      };

	// Diverging? We need a better way of identifying divergence.
	    if(iter>(numberIterations()/2)) 
	      {
		if ((currentChisq>originalChisq)&&(currentChisq>previousChisq)) 
		  {
		    fail=True;
		    break;
		  };
	      };
	  } // End of iteration
	
      // Now check for failures
      if (fail || !converged) {
	{
	  ostringstream o; 
	  o<< "Trouble Converging: "<< "Slot="<<chunk+1<<", " 
	   << (*fieldName_[currentSpw_])(currentSlot_(currentSpw_))
	   <<", spw="<<currentSpw_+1<<": "
	   << MVTime((*MJDStart_[currentSpw_])(currentSlot_(currentSpw_))/86400.0)<<" to "
	   << MVTime((*MJDStop_[currentSpw_])(currentSlot_(currentSpw_))/86400.0)
	   << " after " << iter+1 << " NR iterations.";
	  message.message(o);
	  logSink().post(message);
	}
	fail=True;
	failed++;
	invalidateAGM(currentSpw_);
	//	invalidateIGM(currentSpw_);

	// force from-scratch guess from data on next time around
	lastslot(currentSpw_)=-1;
        lastfld=-1;

      } else {
	// Solution is ok, keep it (by copying into antGainPar array, which will
	//  get written out to the cal table
	keep(currentSlot_(currentSpw_));
	// use this solution as first guess for next slot
	lastslot(currentSpw_)=currentSlot_(currentSpw_);
        lastfld=fld;
      }


      // Avoiding per-solution logging for now; will add verbose option later
      // Report final fit (in any case)
      //      {
      //	ostringstream o; 
      //        o << typeName()
      //          <<"    Final   fit per unit weight = "
      //          << rms <<" Jy, after " 
      //          << iter<<" iterations";
      //	message.message(o);
      //	logSink().post(message);
      //      }
    }
    // Accumulate statistics
    (*fit_[currentSpw_])(currentSlot_(currentSpw_))=rms;
    (*fitwt_[currentSpw_])(currentSlot_(currentSpw_))=sumwt_;
    /*
    cout << "currentSlot_=" << currentSlot_;
    cout << " fit_=" << (*fit_[currentSpw_])(currentSlot_);
    cout << " fitwt_=" << (*fitwt_[currentSpw_])(currentSlot_);
    cout << endl;
    */

    lme.chiSquared(slotFit, slotFitwt);
    for (Int iant=0;iant<numberAnt_;iant++) {

      (*iSolutionOK_[currentSpw_])(iant, currentSlot_(currentSpw_))=
	(!fail && (*iFitwt_[currentSpw_])(iant, currentSlot_(currentSpw_))>0.0);
      (*solutionOK_[currentSpw_])(currentSlot_(currentSpw_))=
	( (*solutionOK_[currentSpw_])(currentSlot_(currentSpw_)) || 
	  (*iSolutionOK_[currentSpw_])(iant,currentSlot_(currentSpw_)) );

      if( (*iSolutionOK_[currentSpw_])(iant,currentSlot_(currentSpw_))) {
	(*iFit_[currentSpw_])(iant, currentSlot_(currentSpw_)) 
	  = sqrt( (*iFit_[currentSpw_])(iant,currentSlot_(currentSpw_))/
		  (*iFitwt_[currentSpw_])(iant,currentSlot_(currentSpw_) ));
      };

    };

    //    cout << endl;

  };

  // Report number of good solutions
  {
    ostringstream o; 
    o << "  Found " << chunk-failed-zerowt << " good " << typeName() << " solutions." << endl
      << "        " << failed << " solution intervals failed." << endl
      << "        " << zerowt << " solution intervals had insufficient data.";
    message.message(o);
    logSink().post(message);
  }

  // Re-reference the gain solutions to the reference antenna, if required
  //  reReference(refant());

  // Restore required_ to nominal value
  required_=origreq;

  //  setSolved(False);
  //  setApplied(True);

  return(True);
  
}
//
//---------------------------------------------------------------------
//
// Update formula for general matrices
void EPJones::setAntPar(Int whichSlot, Array<Float>& Par, Bool solOK) 
{
  IPosition ParShape=Par.shape();
  if ((ParShape.nelements() < 2))
    throw(AipsError("EPJones::setAntPar(): Parameter array shape not consistant with "
		    "2 parameters per antenna"));
  
  IPosition outNdx(4,0),inNdx(2,0);
  outNdx(1)=0;// ichan
  outNdx(3)=whichSlot;

  //  outNdx(0) == ipar
  //  outNdx(1) == ichan
  //  outNdx(2) == iant
  //  outNdx(3) == slot 
  //
  //  inNdx(0)  == ipar
  //  inNdx(1)  == iant
  //
  //  (*antGainPar_[currentSpw_])(IPosition(4,ipar,ichan,iant,slot))

  for(outNdx(0)=0,inNdx(0)=0; outNdx(0)<ParShape(0); outNdx(0)++,inNdx(0)++)
    for(outNdx(2)=0,inNdx(1)=0;	outNdx(2)<ParShape(1);	outNdx(2)++,inNdx(1)++)
      {
	(*antGainPar_[currentSpw_])(outNdx) = Par(inNdx);
	//	iSolutionOK_[currentSpw_]->column(currentSlot_(currentSpw_))=True;
	iSolutionOK_[currentSpw_]->column(whichSlot)=solOK;
      }
  (*solutionOK_[currentSpw_])(whichSlot)=solOK;
}
//
//---------------------------------------------------------------------
//
// Update formula for general matrices
void EPJones::updateAntGain() 
{
  Bool phaseOnly=(mode().contains("phase"));

  Double dist=0.0;

  for (Int iant=0;iant<numberAnt_;iant++) 
    for (Int ichan=0;ichan<nSolnChan_(currentSpw_);ichan++) 
      for (Int i=0;i<2;i++) 
	for (Int j=0;j<2;j++) 
	  if(required_(i,j)&&ggS_(ichan,iant)(i,j)>0.0) 
	    {
	      //	      Complex& g=(*thisAntGainMat_[currentSpw_])(ichan,iant)(i,j);
	      //	      Complex gup;
	      //	      gup=gain()*gS_(ichan,iant)(i,j)/ggS_(ichan,iant)(i,j);
	      //	      dist+=norm(gup);
	      //		    
	      //	      g-=gup;
	      //
	      // Handle phaseOnly case (is this necessary, if data is phase-only?)
	      //	      if (phaseOnly) 
	      //		{
	      //		  Float gamp=abs(g);
	      //		  if (gamp>0.0) g/=gamp;
	      //		}
	    }
}
//
//---------------------------------------------------------------------
//
// Temporary method for copying solved-for matrix into "parameterized" storage
//  (eventually, we'll use the parameterized storage directly in solve)
//
void EPJones::keep(const Int& slot) 
{
  //
  // Assume params are the matrix elements, and are in pol order
  // This version works for diagonal (nPar_=2) and scalar (nPar_=1) matrices
  //
  // Loop over antenna, channel and parameter
  for (Int iant=0; iant<numberAnt_; iant++) 
    for (Int ichan=0; ichan<nSolnChan_(currentSpw_); ichan++) 
      for (Int ipar=0; ipar<nPar_;ipar++) 
	(*antGainPar_[currentSpw_])(IPosition(4,ipar,ichan,iant,slot))=
	  (*thisAntGainMat_[currentSpw_])(ichan,iant)(ipar,ipar);
}

}
