//# CalSet.cc: Implementation of Calibration parameter cache
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

#include <synthesis/CalTables/CalSet.h>

#include <synthesis/CalTables/CalTable2.h>
//#include <synthesis/CalTables/CalTable.h>
#include <synthesis/CalTables/CalDescColumns2.h>
#include <synthesis/CalTables/SolvableVJTable.h>
#include <synthesis/CalTables/CalMainColumns2.h>

#include <synthesis/CalTables/TimeVarVJDesc.h>
#include <synthesis/CalTables/SolvableVJDesc.h>
#include <synthesis/CalTables/SolvableVJMRec.h>
#include <synthesis/CalTables/SolvableVJMCol.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/RefRows.h>

#include <casa/Arrays.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Timer.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Path.h>
#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------

// From shape only, this is the solve context
template<class T> CalSet<T>::CalSet(const Int& nSpw) :
  calTableName_(""),
  nSpw_(nSpw),
  nPar_(0),
  nChan_(0),
  nElem_(0),
  nTime_(0),
  spwOK_(nSpw_,False),
  startChan_(nSpw_,0),  
  freq_(nSpw_,NULL),
  MJDStart_(nSpw_,NULL),
  MJDStop_(nSpw_,NULL),
  MJDTimeStamp_(nSpw_,NULL),
  fieldId_(nSpw_,NULL),
  fieldName_(nSpw_,NULL),
  sourceName_(nSpw_,NULL),
  par_(nSpw_,NULL),
  parOK_(nSpw_,NULL),
  parErr_(nSpw_,NULL),
  parSNR_(nSpw_,NULL),
  //  iSolutionOK_(nSpw_,NULL),
  iFit_(nSpw_,NULL),
  iFitwt_(nSpw_,NULL),
  solutionOK_(nSpw_,NULL),
  fit_(nSpw_,NULL),
  fitwt_(nSpw_,NULL)
{
  calTabDesc_=NULL;
  calTab_=NULL;
  svjmcol_=NULL;
};

// From shape only, this is the solve context
template<class T> CalSet<T>::CalSet(const Int& nSpw,
				    const Int& nPar,
				    const Vector<Int>& nChan,
				    const Int& nElem,
				    const Vector<Int>& nTime) :
  calTableName_(""),
  nSpw_(nSpw),
  nPar_(nPar),
  nChan_(nChan),
  nElem_(nElem),
  nTime_(nTime),
  spwOK_(nSpw_,False),
  startChan_(nSpw_,0),  
  freq_(nSpw_,NULL),
  MJDStart_(nSpw_,NULL),
  MJDStop_(nSpw_,NULL),
  MJDTimeStamp_(nSpw_,NULL),
  fieldId_(nSpw_,NULL),
  fieldName_(nSpw_,NULL),
  sourceName_(nSpw_,NULL),
  par_(nSpw_,NULL),
  parOK_(nSpw_,NULL),
  parErr_(nSpw_,NULL),
  parSNR_(nSpw_,NULL),
  //  iSolutionOK_(nSpw_,NULL),
  iFit_(nSpw_,NULL),
  iFitwt_(nSpw_,NULL),
  solutionOK_(nSpw_,NULL),
  fit_(nSpw_,NULL),
  fitwt_(nSpw_,NULL)
{
  calTabDesc_=NULL;
  calTab_=NULL;
  svjmcol_=NULL;
  // Resize caches
  inflate();  
};


// From existing CalTable name, this is apply context
template<class T> CalSet<T>::CalSet(const String& calTableName,
				    const String& select,
				    const Int& nSpw,
				    const Int& nPar,
				    const Int& nElem) : 
  calTableName_(calTableName),
  nSpw_(nSpw),
  nPar_(nPar),
  nChan_(nSpw,0),
  nElem_(nElem),
  nTime_(nSpw,0),
  spwOK_(nSpw_,False),
  startChan_(nSpw_,0),  
  freq_(nSpw_,NULL),
  MJDStart_(nSpw_,NULL),
  MJDStop_(nSpw_,NULL),
  MJDTimeStamp_(nSpw_,NULL),
  fieldId_(nSpw_,NULL),
  fieldName_(nSpw_,NULL),
  sourceName_(nSpw_,NULL),
  par_(nSpw_,NULL),
  parOK_(nSpw_,NULL),
  parErr_(nSpw_,NULL),
  parSNR_(nSpw_,NULL),
  //  iSolutionOK_(nSpw_,NULL),
  iFit_(nSpw_,NULL),
  iFitwt_(nSpw_,NULL),
  solutionOK_(nSpw_,NULL),
  fit_(nSpw_,NULL),
  fitwt_(nSpw_,NULL)
{
  calTabDesc_=NULL;
  calTab_=NULL;
  svjmcol_=NULL;

  // Fill from table
  load(calTableName,select);

  // Set spwOK_ according to available calibration
  setSpwOK();

}

template<class T> CalSet<T>::~CalSet() {
  deflate();
}

template<class T> void CalSet<T>::resize(const Int& nPar,
					 const Vector<Int>& nChan,
					 const Int& nElem,
					 const Vector<Int>& nTime) {
  nPar_=nPar;
  nChan_=nChan;
  nElem_=nElem;
  nTime_=nTime;

  inflate();

}



// Inflate cache to proper size
template<class T> void CalSet<T>::inflate() {
  
  // Construct shaped pointed-to objects in cache

  // TODO:
  //  Consider initialization value (per type)

  // Delete exiting cache
  deflate();

  for (Int ispw=0; ispw<nSpw_; ispw++) {
    uInt ntime=nTime_(ispw);
    if (ntime > 0) {

      freq_[ispw]         = new Vector<Double>(nChan_(ispw),0.0);
      
      MJDStart_[ispw]     = new Vector<Double>(ntime,0.0);
      MJDStop_[ispw]      = new Vector<Double>(ntime,0.0);
      MJDTimeStamp_[ispw] = new Vector<Double>(ntime,0.0);
      fieldName_[ispw]    = new Vector<String>(ntime,"");
      sourceName_[ispw]   = new Vector<String>(ntime,"");
      fieldId_[ispw]      = new Vector<Int>(ntime,-1);

      IPosition parshape(4,nPar_,nChan_(ispw),nElem_,ntime);
      par_[ispw]     = new Array<T>(parshape,1.0);
      parOK_[ispw]   = new Array<Bool>(parshape,False);
      parErr_[ispw]  = new Array<Float>(parshape,0.0);
      parSNR_[ispw]  = new Array<Float>(parshape,0.0);

      //      iSolutionOK_[ispw]  = new Matrix<Bool>(nElem_,ntime,False);
      iFit_[ispw]         = new Matrix<Float>(nElem_,ntime,0.0);
      iFitwt_[ispw]       = new Matrix<Float>(nElem_,ntime,0.0);
      solutionOK_[ispw]   = new Vector<Bool>(ntime,False);
      fit_[ispw]          = new Vector<Float>(ntime,0.0);
      fitwt_[ispw]        = new Vector<Float>(ntime,0.0);
    }
  }

}




template<class T> void CalSet<T>::deflate() {
  
  // Delete parameter memory

  for (Int ispw=0; ispw<nSpw_; ispw++) {
    if (MJDStart_[ispw])     delete MJDStart_[ispw];
    if (MJDStop_[ispw])      delete MJDStop_[ispw];
    if (MJDTimeStamp_[ispw]) delete MJDTimeStamp_[ispw];
    if (fieldName_[ispw])    delete fieldName_[ispw];
    if (sourceName_[ispw])   delete sourceName_[ispw];
    if (fieldId_[ispw])      delete fieldId_[ispw];
    if (par_[ispw])          delete par_[ispw];
    if (parOK_[ispw])        delete parOK_[ispw];
    if (parErr_[ispw])       delete parErr_[ispw];
    if (parSNR_[ispw])       delete parSNR_[ispw];
    //    if (iSolutionOK_[ispw])  delete iSolutionOK_[ispw];
    if (iFit_[ispw])         delete iFit_[ispw];
    if (iFitwt_[ispw])       delete iFitwt_[ispw];
    if (solutionOK_[ispw])   delete solutionOK_[ispw];
    if (fit_[ispw])          delete fit_[ispw];
    if (fitwt_[ispw])        delete fitwt_[ispw];
    MJDStart_[ispw]=NULL;
    MJDStop_[ispw]=NULL;
    MJDTimeStamp_[ispw]=NULL;
    fieldName_[ispw]=NULL;
    sourceName_[ispw]=NULL;
    fieldId_[ispw]=NULL;
    par_[ispw]=NULL;
    parOK_[ispw]=NULL;
    parErr_[ispw]=NULL;
    parSNR_[ispw]=NULL;
    //    iSolutionOK_[ispw]=NULL;
    iFit_[ispw]=NULL;
    iFitwt_[ispw]=NULL;
    solutionOK_[ispw]=NULL;
    fit_[ispw]=NULL;
    fitwt_[ispw]=NULL;
  }
}



template<class T> void CalSet<T>::load (const String& file, 
					const String& select)
{
  // Load data from a calibration table
  // Input:
  //    file         const String&       Cal table name
  //    select       const String&       Selection string
  //


  //  cout << "CalSet::load...(nSpw_=" << nSpw_ << ")" << endl;

  Timer timer;
  timer.mark();
  
  LogMessage message(LogOrigin("CalSet","load"));
  
  // Decode the Jones matrix type
  Int jonesType = 0;
  if (nPar_ == 1) jonesType = 1;
  if (nPar_ == 2) jonesType = 2;
  if (nPar_ == 4) jonesType = 3;

 // Open, select, sort the calibration table
  SolvableVisJonesTable svjtab(file);
  svjtab.select2(select);

  // Get no. of antennas and time slots
  Int numberAnt = svjtab.maxAntenna() + 1;

  //  cout << "Initial selection: " << timer.all_usec()/1.0e6 << endl;
  timer.mark();

  AlwaysAssert(numberAnt==nElem_,AipsError)

  Int nDesc=svjtab.nRowDesc();
  Vector<Int> spwmap(nDesc,-1);

  Vector<Int> nRowPerCDI;
  svjtab.rowsPerCalDescId(nRowPerCDI);

  //  cout << "Optimized CDI count: " << timer.all_usec()/1.0e6 << endl;
  timer.mark();

  for (Int idesc=0;idesc<nDesc;idesc++) {

    // This cal desc
    CalDescRecord* calDescRec = new CalDescRecord (svjtab.getRowDesc(idesc));

    // Get this spw ID
    Vector<Int> spwlist;
    calDescRec->getSpwId(spwlist);
    Int nSpw; spwlist.shape(nSpw);
    if (nSpw > 1) {};  // ERROR!!!  Should only be one spw per cal desc!
    spwmap(idesc)=spwlist(0);

    // Trap spwids that do not occur in the MS
    // (Since we rely on the MS meta info for the calibration solutions,
    //  we cannot identify spws that do not occur in the MS.)
    if (spwlist(0)>nSpw_-1)
      throw(AipsError("Caltable '"+file+"' contains spw = "+
		      String::toString(spwlist(0))+
		      " which does not occur in the MS.  Cannot proceed."));

    // In next few steps, need to watch for repeat spws in new cal descs!!

    // Get number of channels this spw
    Vector<Int> nchanlist;

    calDescRec->getNumChan(nchanlist);
    nChan_(spwmap(idesc))=nchanlist(0);

    // Get channel range / start channel
    Cube<Int> chanRange;
    calDescRec->getChanRange(chanRange);
    startChan_(spwmap(idesc))=chanRange(0,0,0);

    // Get slot count for this desc's spw
    nTime_(spwmap(idesc))=nRowPerCDI(idesc)/numberAnt;

    delete calDescRec;  
  }

  //  cout << "CalDesc sizeup: " << timer.all_usec()/1.0e6 << endl;
  //  cout << "nTime_ = " << nTime_ << endl;

  timer.mark();

  // At this point, we know how big our slot-dep caches must be
  //  (in private data), so initialize them
  inflate();

  // Remember if we found and filled any solutions
  Bool solfillok(False);

  //  cout << "CalSet inflated: " << timer.all_usec()/1.0e6 << endl;


  // Fill per caldesc
  Double ttime(0.0);
  for (Int idesc=0;idesc<nDesc;idesc++) {

    //    cout << "CDI = " << idesc << " "<< flush;

    timer.mark();

    Int thisSpw=spwmap(idesc);
      
    // Reopen and globally select caltable
    //SolvableVisJonesTable svjtabspw(file);
    CalTable2 svjtabspw(file);
    svjtabspw.select2(select);

    //    cout << " Sel: " << timer.all_usec()/1.0e6 << flush;

    // isolate this caldesc:
    ostringstream selectstr;
    selectstr << "CAL_DESC_ID == " << idesc;
    String caldescsel; caldescsel = selectstr.str();
    svjtabspw.select2(caldescsel);

    //    cout << " CDIsel: " << timer.all_usec()/1.0e6 << flush;

    //    cout << "Sorting..." << endl;
    Block<String> scol(1);
    scol[0]="TIME";
    svjtabspw.sort2(scol);

    //    cout << " Sort: " << timer.all_usec()/1.0e6 << flush;

    Int nrow = svjtabspw.nRowMain();
    IPosition out(3,0,0,0);   // par, chan, row
    IPosition in(4,0,0,0,0);  // par, chan, ant, slot
    if (nrow>0) {

      // Found some solutions to fill
      solfillok=True;

      // Ensure sorted on time
      Block <String> sortCol(1,"TIME");
      svjtabspw.sort2(sortCol);
      
      // Extract the gain table columns
      //ROSolvableVisJonesMCol svjmcol(svjtabspw);
      ROSolvableCalSetMCol<T> *svjmcol;
      svjmcol= new ROSolvableCalSetMCol<T>(svjtabspw);

      Vector<Int>    calDescId;  (*svjmcol).calDescId().getColumn(calDescId);
      Vector<Double> time;       (*svjmcol).time().getColumn(time);
      Vector<Double> interval;   (*svjmcol).interval().getColumn(interval);
      Vector<Int>    antenna1;   (*svjmcol).antenna1().getColumn(antenna1);
      //      cout << "antennas = " << antenna1 << endl;
      Vector<Int>    fieldId;    (*svjmcol).fieldId().getColumn(fieldId);
      //      cout << "fieldId = " << fieldId << endl;
      Vector<String> fieldName;  (*svjmcol).fieldName().getColumn(fieldName);
      Vector<String> sourceName; (*svjmcol).sourceName().getColumn(sourceName);
      Vector<Bool>   totalSolOk; (*svjmcol).totalSolnOk().getColumn(totalSolOk);
      Vector<Float>  totalFit;   (*svjmcol).totalFit().getColumn(totalFit);
      Vector<Float>  totalFitWt; (*svjmcol).totalFitWgt().getColumn(totalFitWt);
      Array<T> gain;       (*svjmcol).gain().getColumn(gain);
      Cube<Bool>     solOk;      (*svjmcol).solnOk().getColumn(solOk);
      Cube<Float>    fit;        (*svjmcol).fit().getColumn(fit);
      Cube<Float>    fitWt;      (*svjmcol).fitWgt().getColumn(fitWt);
      Cube<Bool>     flag;       (*svjmcol).flag().getColumn(flag);
      Cube<Float>    snr;        (*svjmcol).snr().getColumn(snr);

      // Read the calibration information
      Double lastTime(-1.0), thisTime(0.0), thisInterval(0.0);
      Int islot(-1);
      Int iant;

      for (Int irow=0; irow<nrow; irow++) {
	out(2)=irow;

	thisTime=time(irow);
	
	// If this is a new solution
	if (irow%numberAnt==0) {

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

	//	(*iSolutionOK_[thisSpw])(iant,islot) = solOk(0,0,irow);
	(*iFit_[thisSpw])(iant,islot) = fit(0,0,irow);
	(*iFitwt_[thisSpw])(iant,islot) = fitWt(0,0,irow);
	
	for (Int ichan=0; ichan<nChan_(thisSpw); ichan++) {


	  out(1)=in(1)=ichan;

	  for (Int ipar=0; ipar<nPar_; ipar++) {
	    in(0)=out(0)=ipar;
	    (*par_[thisSpw])(in)=gain(out);
	    (*parOK_[thisSpw])(in) = (solOk(out) && !flag(out));
	    (*parSNR_[thisSpw])(in) = snr(out);
	  }
	}
	
      } // irow
    } // nrow>0

    //    cout << "cs fields = " << *fieldId_[thisSpw] << endl;

    Double itime=timer.all_usec()/1.0e6;
    ttime+=itime;

    //    cout << " Totals: " << itime << " " << ttime << endl;

  } // idesc

  // If we found no solutions in selected table, abort:
  if (!solfillok) {
      throw(AipsError(" Specified cal table selection selects no solutions in this table.  Please review setapply settings."));
  }


};

template<class T> void CalSet<T>::initCalTableDesc(const String& type, const Int& parType)
{
  if (calTabDesc_) {delete calTabDesc_;calTabDesc_=NULL;}
  calTabDesc_=new CalTableDesc2(type,parType);
}

template<class T> void CalSet<T>::attach()
{
}


template<class T> void CalSet<T>::store (const String& file, 
					 const String& type,
					 const Bool& append,
					 const String& msname)
{
  // Write the solutions to an output calibration table
  // Input:
  //    file           String        Cal table name
  //    append         Bool          Append if true, else overwrite
  //

  // total rows to be written per Spw
  Vector<Int> nRow(nSpw_,0);
  for (Int iSpw=0;iSpw<nSpw_;iSpw++) 
    if (solutionOK_[iSpw])
      nRow(iSpw)=nElem_*ntrue(*(solutionOK_[iSpw]));

  // Escape if nothing to write
  if (sum(nRow)==0)
    throw(AipsError("No valid calibration solutions; no table written."));

  // Initialization:
  // No. of rows in cal_main, cal_desc and cal_history
  Int nMain = 0; 
  Int nDesc = 0;
  Int nHist = 0;
  
  if (calTabDesc_ == NULL)
    {
      ostringstream str;
      str << "CalSet::store(): Perhaps CalSet.initCalTableDesc() was not called "
	  << "before calling CalSet::store()?"
	  << "  Jones = " << type << "   File = " << file;
      
      throw(AipsError(str.str()));
    }

  calTabDesc_->addDesc(calTabDesc_->defaultFitDesc(),calTabDesc_->calMainDesc());

  // Calibration table
  //  SolvableVisJonesTable *tab;
  
  // Open the output file if it already exists and is being appended to.
  if (calTab_) delete calTab_; calTab_=NULL;

  if (append && Table::isWritable (file)) {
    //    tab  = new SolvableVisJonesTable (file, Table::Update);
    calTab_  = new CalTable2 (file, *calTabDesc_, Table::Update);
    nMain = calTab_->nRowMain();
    nDesc = calTab_->nRowDesc();
    nHist = calTab_->nRowHistory();
  } else {
    // Create a new calibration table
    Table::TableOption access = Table::New;
    //    tab = new SolvableVisJonesTable (file, type, access);
    calTab_ = new CalTable2 (file, *calTabDesc_, access);
  };
  
  // Write every spw w/ max number of channels 
  //  (eventually, CalTable should permit variable-shape cols)
  Int maxNumChan(1);
  for (Int iSpw=0; iSpw<nSpw_; iSpw++) 
    if (par_[iSpw]!=NULL) 
      maxNumChan=max(maxNumChan,nChan_(iSpw));

  // Some default values
  Double dzero = 0;
  IPosition ip(2,1,maxNumChan);

  // CalDesc Sub-table records
  CalDescRecord* descRec;
  Vector<Int> calDescNum(nSpw_); calDescNum=-1;
  for (Int iSpw=0; iSpw<nSpw_; iSpw++) {

    // Write a CalDesc for each spw which has solutions
    // Note: CalDesc index != SpwId, in general

    if (nRow(iSpw)>0 && par_[iSpw]!=NULL) {

      // Access to existing CAL_DESC columns:
      CalDescColumns2 cd(*calTab_);

      // Check if this spw already in CAL_DESC 
      //      cout << "spwCol = " << cd.spwId().getColumn() << endl;

      Bool newCD(True);
      for (Int iCD=0;iCD<nDesc;iCD++) {

	IPosition iCDip(1,0);
	if ( iSpw==(cd.spwId()(iCD))(iCDip) ) {
	  // Don't need new CAL_DESC entry
	  newCD=False;
	  calDescNum(iSpw)=iCD;
	  break;
	}
      }

      if (newCD) {

	// Cal_desc fields
	Vector <Int> spwId(1,iSpw);
	Matrix <Double> chanFreq(ip, dzero); 
	Matrix <Double> chanWidth(ip, dzero);
	Array <String> polznType(ip, "");
	Cube <Int> chanRange(IPosition(3,2,1,maxNumChan), 0);
	Vector <Int> numChan(1,nChan_(iSpw));
	for (Int ichan=0; ichan<nChan_(iSpw); ichan++) {
	  chanRange(0,0,ichan)=startChan_(iSpw);
	  chanRange(1,0,ichan)=startChan_(iSpw) + nChan_(iSpw) -1;
	}
	
	// Fill the cal_desc record
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
	descRec->defineMSName (Path(msname).baseName());
	//	descRec->defineMSName ("");
	
	// Write the cal_desc record

	calTab_->putRowDesc (nDesc, *descRec);
	delete descRec;
	
	// This spw will have this calDesc index in main table
	calDescNum(iSpw) = nDesc;
	nDesc++;
      }

    }
    
  }


  // Now write MAIN table in column-wise fashion
  
  // Starting row in this slot

  for (Int iSpw=0; iSpw<nSpw_; iSpw++) {

    // Write table for spws which have solutions
    if (par_[iSpw]!=NULL) {

      // Create references to cal data for this spw
      Vector<Bool>    thisSolOK;        thisSolOK.reference(*(solutionOK_[iSpw]));
      Vector<Double>  thisMJDTimeStamp; thisMJDTimeStamp.reference(*(MJDTimeStamp_[iSpw]));
      Vector<Double>  thisMJDStart;     thisMJDStart.reference(*(MJDStart_[iSpw]));
      Vector<Double>  thisMJDStop;      thisMJDStop.reference(*(MJDStop_[iSpw]));
      Vector<Int>     thisFieldId;      thisFieldId.reference(*(fieldId_[iSpw]));
      Vector<String>  thisFieldName;    thisFieldName.reference(*(fieldName_[iSpw]));
      Vector<String>  thisSourceName;   thisSourceName.reference(*(sourceName_[iSpw]));
      Vector<Float>   thisFit;          thisFit.reference(*(fit_[iSpw]));
      Vector<Float>   thisFitwt;        thisFitwt.reference(*(fitwt_[iSpw]));
      //      Array<Complex>  thisAntGain;      thisAntGain.reference(*(par_[iSpw]));
      Array<T>  thisAntGain;      thisAntGain.reference(*(par_[iSpw]));
      //      Matrix<Bool>    thisISolutionOK;  thisISolutionOK.reference(*(iSolutionOK_[iSpw]));
      Matrix<Float>   thisIFit;         thisIFit.reference(*(iFit_[iSpw]));
      Matrix<Float>   thisIFitwt;       thisIFitwt.reference(*(iFitwt_[iSpw]));

      Int thisnRow=nRow(iSpw);
      
      // Only if there are rows to add to table
      if (thisnRow > 0) {

	// These are constant columns (with boring values, currently)
	Vector<Double> timeEP(thisnRow,0.0);
	Vector<Int> feed1(thisnRow,0);
	Vector<Int> arrayId(thisnRow,0);
	Vector<Int> obsId(thisnRow,0);
	Vector<Int> scanNum(thisnRow,0);
	Vector<Int> procId(thisnRow,0);
	Vector<Int> stateId(thisnRow,0);
	Vector<Int> phaseId(thisnRow,0);
	Vector<Int> pulsarBin(thisnRow,0);
	Vector<Int> pulsarGateId(thisnRow,0);
	Vector<Int> freqGroup(thisnRow,0);
	Vector<Int> calHistId(thisnRow,0);
	
	// This is constant
	Vector<Int> calDescId(thisnRow,calDescNum(iSpw));
	
	// These are constant per slot
	//   (these cols should be incremental)
	Vector<Double> time(thisnRow,0.0);
	Vector<Double> interval(thisnRow,0.0);
	Vector<Int>    fieldId(thisnRow,0);
	Vector<String> fieldName(thisnRow,"");
	Vector<String> sourceName(thisnRow,"");
	Vector<Bool>   totalSolOk(thisnRow,False);
	Vector<Float>  totalFit(thisnRow,0.0);
	Vector<Float>  totalFitWt(thisnRow,0.0);
	
	// These vary
	Vector<Int>    antenna1(thisnRow,0);
	Cube<T>  gain(IPosition(3,nPar(),maxNumChan,thisnRow),T(0.0));
	Cube<Bool>     solOk(nPar(),maxNumChan,thisnRow,False);
	Cube<Float>    fit(1,maxNumChan,thisnRow,0.0);
	Cube<Float>    fitWt(1,maxNumChan,thisnRow,0.0);
	Cube<Bool>     flag(nPar(),maxNumChan,thisnRow,False);
	Cube<Float>    snr(nPar(),maxNumChan,thisnRow,False);

	IPosition out(3,0,0,0);   // par, chan, row
	IPosition in(4,0,0,0,0);  // par, chan, ant, slot
	Int thisRow(0);
	for (Int islot = 0; islot < nTime_(iSpw); islot++) {
	  in(3)=islot;
	  if (thisSolOK(islot)) {
	    
	    // Fill slot-constant cols:
	    Slice thisSlice(thisRow,nElem_);
	    time(thisSlice)=thisMJDTimeStamp(islot);
	    Double dt=(thisMJDStop(islot) - thisMJDStart(islot));
	    if (dt<0.0) dt=1.0;
	    interval(thisSlice)=dt;
	    fieldId(thisSlice)=thisFieldId(islot);
	    fieldName(thisSlice)=thisFieldName(islot);
	    sourceName(thisSlice)=thisSourceName(islot);
	    totalSolOk(thisSlice)=thisSolOK(islot);
	    totalFit(thisSlice)=thisFit(islot);
	    totalFitWt(thisSlice)=thisFitwt(islot);
	    
	    // Loop over the number of antennas
	    for (Int iant = 0; iant < nElem_; iant++) {
	      out(2)=thisRow;
	      in(2)=iant;
	      // Antenna index
	      antenna1(thisRow)=iant;
	      
	      gain.xyPlane(thisRow)(IPosition(2,0,0),IPosition(2,nPar()-1,nChan_(iSpw)-1))=
		thisAntGain(IPosition(4,0,0,iant,islot),
			    IPosition(4,nPar()-1,nChan_(iSpw)-1,iant,islot)).nonDegenerate(2);

	      // Per-chan fit pars
	      for (Int ichan=0; ichan<nChan_(iSpw); ichan++) {
		// Gain stats  (slot constant, per spw?)
		//solOk(0,ichan,thisRow) = thisISolutionOK(iant,islot);
		fit(0,ichan,thisRow) = thisIFit(iant,islot);
		fitWt(0,ichan,thisRow) = thisIFitwt(iant,islot);


		for (Int ipar=0; ipar<nPar(); ++ipar) {
		  solOk(ipar,ichan,thisRow) = parOK(iSpw)(IPosition(4,ipar,ichan,iant,islot));
		  flag(ipar,ichan,thisRow) = !solOk(ipar,ichan,thisRow);
		  snr(ipar,ichan,thisRow) = parSNR(iSpw)(IPosition(4,ipar,ichan,iant,islot));
		} // ipar
	      } // ichan
	      
	      // next time round is next row
	      thisRow++;
	    }; // iant

	    
	  }; // thisSolOK(islot)
	}; // islot
	
	// Now push everything to the disk table
	calTab_->addRowMain(thisnRow);
	//	SolvableVisJonesMCol svjmcol(*tab);
	if (svjmcol_) {delete svjmcol_; svjmcol_=NULL;}
	svjmcol_ = new SolvableCalSetMCol<T>(*calTab_);

	RefRows refRows(nMain,nMain+thisnRow-1);
	svjmcol_->time().putColumnCells(refRows,time);
	svjmcol_->timeEP().putColumnCells(refRows,timeEP);
	svjmcol_->interval().putColumnCells(refRows,interval);
	svjmcol_->antenna1().putColumnCells(refRows,antenna1);
	svjmcol_->feed1().putColumnCells(refRows,feed1);
	svjmcol_->fieldId().putColumnCells(refRows,fieldId);
	svjmcol_->arrayId().putColumnCells(refRows,arrayId);
	svjmcol_->obsId().putColumnCells(refRows,obsId);
	svjmcol_->scanNo().putColumnCells(refRows,scanNum);
	svjmcol_->processorId().putColumnCells(refRows,procId);
	svjmcol_->stateId().putColumnCells(refRows,stateId);
	svjmcol_->phaseId().putColumnCells(refRows,phaseId);
	svjmcol_->pulsarBin().putColumnCells(refRows,pulsarBin);
	svjmcol_->pulsarGateId().putColumnCells(refRows,pulsarGateId);
	svjmcol_->freqGrp().putColumnCells(refRows,freqGroup);
	svjmcol_->fieldName().putColumnCells(refRows,fieldName);
	svjmcol_->sourceName().putColumnCells(refRows,sourceName);
	svjmcol_->gain().putColumnCells(refRows,gain);
	svjmcol_->totalSolnOk().putColumnCells(refRows,totalSolOk);
	svjmcol_->totalFit().putColumnCells(refRows,totalFit);
	svjmcol_->totalFitWgt().putColumnCells(refRows,totalFitWt);
	svjmcol_->solnOk().putColumnCells(refRows,solOk);
	svjmcol_->fit().putColumnCells(refRows,fit);
	svjmcol_->fitWgt().putColumnCells(refRows,fitWt);
	svjmcol_->calDescId().putColumnCells(refRows,calDescId);
	svjmcol_->calHistoryId().putColumnCells(refRows,calHistId);
	svjmcol_->flag().putColumnCells(refRows,flag);
	svjmcol_->snr().putColumnCells(refRows,snr);
	
	nMain = calTab_->nRowMain();
	
      } // thisnRow > 0
    } // par_[iSpw]!=NULL
  } // iSpw

  delete calTab_;

};
/*
template<class T> void CalSet<T>::store (const String& file, 
					 const String& type,
					 const String& msname,
					 const Bool& append)
{
  // Write the solutions to an output calibration table
  // Input:
  //    file           String        Cal table name
  //    append         Bool          Append if true, else overwrite
  //

  // total rows to be written per Spw
  Vector<Int> nRow(nSpw_,0);
  for (Int iSpw=0;iSpw<nSpw_;iSpw++) 
    if (solutionOK_[iSpw])
      nRow(iSpw)=nElem_*ntrue(*(solutionOK_[iSpw]));

  // Escape if nothing to write
  if (sum(nRow)==0)
    throw(AipsError("No valid calibration solutions; no table written."));

  // Initialization:
  // No. of rows in cal_main, cal_desc and cal_history
  Int nMain = 0; 
  Int nDesc = 0;
  Int nHist = 0;
  
  // Calibration table
  SolvableVisJonesTable *tab;
  
  // Open the output file if it already exists and is being appended to.
  if (append && Table::isWritable (file)) {
    tab  = new SolvableVisJonesTable (file, Table::Update);
    nMain = tab->nRowMain();
    nDesc = tab->nRowDesc();
    nHist = tab->nRowHistory();
  } else {
    // Create a new calibration table
    Table::TableOption access = Table::New;
    tab = new SolvableVisJonesTable (file, type, access);
  };
  
  // Write every spw w/ max number of channels 
  //  (eventually, CalTable should permit variable-shape cols)
  Int maxNumChan(1);
  for (Int iSpw=0; iSpw<nSpw_; iSpw++) 
    if (par_[iSpw]!=NULL) 
      maxNumChan=max(maxNumChan,nChan_(iSpw));

  // Some default values
  Double dzero = 0;
  IPosition ip(2,1,maxNumChan);

  // CalDesc Sub-table records
  CalDescRecord* descRec;
  Vector<Int> calDescNum(nSpw_); calDescNum=-1;
  for (Int iSpw=0; iSpw<nSpw_; iSpw++) {

    // Write a CalDesc for each spw which has solutions
    // Note: CalDesc index != SpwId, in general

    if (nRow(iSpw)>0 && par_[iSpw]!=NULL) {

      // Access to existing CAL_DESC columns:
      CalDescColumns cd(*tab);

      // Check if this spw already in CAL_DESC 
      //      cout << "spwCol = " << cd.spwId().getColumn() << endl;

      Bool newCD(True);
      for (Int iCD=0;iCD<nDesc;iCD++) {

	IPosition iCDip(1,0);
	if ( iSpw==(cd.spwId()(iCD))(iCDip) ) {
	  // Don't need new CAL_DESC entry
	  newCD=False;
	  calDescNum(iSpw)=iCD;
	  break;
	}
      }

      if (newCD) {

	// Cal_desc fields
	Vector <Int> spwId(1,iSpw);
	Matrix <Double> chanFreq(ip, dzero); 
	Matrix <Double> chanWidth(ip, dzero);
	Array <String> polznType(ip, "");
	Cube <Int> chanRange(IPosition(3,2,1,maxNumChan), 0);
	Vector <Int> numChan(1,nChan_(iSpw));
	for (Int ichan=0; ichan<nChan_(iSpw); ichan++) {
	  chanRange(0,0,ichan)=startChan_(iSpw);
	  chanRange(1,0,ichan)=startChan_(iSpw) + nChan_(iSpw) -1;
	}
	
	// Fill the cal_desc record
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
	descRec->defineMSName (Path(msname).baseName());
	
	// Write the cal_desc record

	tab->putRowDesc (nDesc, *descRec);
	delete descRec;
	
	// This spw will have this calDesc index in main table
	calDescNum(iSpw) = nDesc;
	nDesc++;
      }

    }
    
  }


  // Now write MAIN table in column-wise fashion
  
  // Starting row in this slot

  for (Int iSpw=0; iSpw<nSpw_; iSpw++) {

    // Write table for spws which have solutions
    if (par_[iSpw]!=NULL) {

      // Create references to cal data for this spw
      Vector<Bool>    thisSolOK;        thisSolOK.reference(*(solutionOK_[iSpw]));
      Vector<Double>  thisMJDTimeStamp; thisMJDTimeStamp.reference(*(MJDTimeStamp_[iSpw]));
      Vector<Double>  thisMJDStart;     thisMJDStart.reference(*(MJDStart_[iSpw]));
      Vector<Double>  thisMJDStop;      thisMJDStop.reference(*(MJDStop_[iSpw]));
      Vector<Int>     thisFieldId;      thisFieldId.reference(*(fieldId_[iSpw]));
      Vector<String>  thisFieldName;    thisFieldName.reference(*(fieldName_[iSpw]));
      Vector<String>  thisSourceName;   thisSourceName.reference(*(sourceName_[iSpw]));
      Vector<Float>   thisFit;          thisFit.reference(*(fit_[iSpw]));
      Vector<Float>   thisFitwt;        thisFitwt.reference(*(fitwt_[iSpw]));
      Array<Complex>  thisAntGain;      thisAntGain.reference(*(par_[iSpw]));
      //      Matrix<Bool>    thisISolutionOK;  thisISolutionOK.reference(*(iSolutionOK_[iSpw]));
      Matrix<Float>   thisIFit;         thisIFit.reference(*(iFit_[iSpw]));
      Matrix<Float>   thisIFitwt;       thisIFitwt.reference(*(iFitwt_[iSpw]));

      Int thisnRow=nRow(iSpw);
      
      // Only if there are rows to add to table
      if (thisnRow > 0) {

	// These are constant columns (with boring values, currently)
	Vector<Double> timeEP(thisnRow,0.0);
	Vector<Int> feed1(thisnRow,0);
	Vector<Int> arrayId(thisnRow,0);
	Vector<Int> obsId(thisnRow,0);
	Vector<Int> scanNum(thisnRow,0);
	Vector<Int> procId(thisnRow,0);
	Vector<Int> stateId(thisnRow,0);
	Vector<Int> phaseId(thisnRow,0);
	Vector<Int> pulsarBin(thisnRow,0);
	Vector<Int> pulsarGateId(thisnRow,0);
	Vector<Int> freqGroup(thisnRow,0);
	Vector<Int> calHistId(thisnRow,0);
	
	// This is constant
	Vector<Int> calDescId(thisnRow,calDescNum(iSpw));
	
	// These are constant per slot
	//   (these cols should be incremental)
	Vector<Double> time(thisnRow,0.0);
	Vector<Double> interval(thisnRow,0.0);
	Vector<Int>    fieldId(thisnRow,0);
	Vector<String> fieldName(thisnRow,"");
	Vector<String> sourceName(thisnRow,"");
	Vector<Bool>   totalSolOk(thisnRow,False);
	Vector<Float>  totalFit(thisnRow,0.0);
	Vector<Float>  totalFitWt(thisnRow,0.0);
	
	// These vary
	Vector<Int>    antenna1(thisnRow,0);
	Cube<Complex>  gain(IPosition(3,nPar(),maxNumChan,thisnRow),Complex(0.0,0.0));
	Cube<Bool>     solOk(nPar(),maxNumChan,thisnRow,False);
	Cube<Float>    fit(1,maxNumChan,thisnRow,0.0);
	Cube<Float>    fitWt(1,maxNumChan,thisnRow,0.0);
	Cube<Bool>     flag(nPar(),maxNumChan,thisnRow,False);
	Cube<Float>    snr(nPar(),maxNumChan,thisnRow,False);

	IPosition out(3,0,0,0);   // par, chan, row
	IPosition in(4,0,0,0,0);  // par, chan, ant, slot
	Int thisRow(0);
	for (Int islot = 0; islot < nTime_(iSpw); islot++) {
	  in(3)=islot;
	  if (thisSolOK(islot)) {
	    
	    // Fill slot-constant cols:
	    Slice thisSlice(thisRow,nElem_);
	    time(thisSlice)=thisMJDTimeStamp(islot);
	    Double dt=(thisMJDStop(islot) - thisMJDStart(islot));
	    if (dt<0.0) dt=1.0;
	    interval(thisSlice)=dt;
	    fieldId(thisSlice)=thisFieldId(islot);
	    fieldName(thisSlice)=thisFieldName(islot);
	    sourceName(thisSlice)=thisSourceName(islot);
	    totalSolOk(thisSlice)=thisSolOK(islot);
	    totalFit(thisSlice)=thisFit(islot);
	    totalFitWt(thisSlice)=thisFitwt(islot);
	    
	    // Loop over the number of antennas
	    for (Int iant = 0; iant < nElem_; iant++) {
	      out(2)=thisRow;
	      in(2)=iant;
	      // Antenna index
	      antenna1(thisRow)=iant;
	      

	      gain.xyPlane(thisRow)(IPosition(2,0,0),IPosition(2,nPar()-1,nChan_(iSpw)-1))=
		thisAntGain(IPosition(4,0,0,iant,islot),IPosition(4,nPar()-1,nChan_(iSpw)-1,iant,islot)).nonDegenerate(2);

	      
	      // Per-chan fit pars
	      for (Int ichan=0; ichan<nChan_(iSpw); ichan++) {
		// Gain stats  (slot constant, per spw?)
		//solOk(0,ichan,thisRow) = thisISolutionOK(iant,islot);
		fit(0,ichan,thisRow) = thisIFit(iant,islot);
		fitWt(0,ichan,thisRow) = thisIFitwt(iant,islot);


		for (Int ipar=0; ipar<nPar(); ++ipar) {
		  solOk(ipar,ichan,thisRow) = parOK(iSpw)(IPosition(4,ipar,ichan,iant,islot));
		  flag(ipar,ichan,thisRow) = !solOk(ipar,ichan,thisRow);
		  snr(ipar,ichan,thisRow) = parSNR(iSpw)(IPosition(4,ipar,ichan,iant,islot));
		}
	      }
	      
	      // next time round is next row
	      thisRow++;
	    };
	    
	  };
	};
	
	// Now push everything to the disk table
	tab->addRowMain(thisnRow);
	SolvableVisJonesMCol svjmcol(*tab);
	
	RefRows refRows(nMain,nMain+thisnRow-1);
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
	svjmcol.gain().putColumnCells(refRows,gain);
	svjmcol.totalSolnOk().putColumnCells(refRows,totalSolOk);
	svjmcol.totalFit().putColumnCells(refRows,totalFit);
	svjmcol.totalFitWgt().putColumnCells(refRows,totalFitWt);
	svjmcol.solnOk().putColumnCells(refRows,solOk);
	svjmcol.fit().putColumnCells(refRows,fit);
	svjmcol.fitWgt().putColumnCells(refRows,fitWt);
	svjmcol.calDescId().putColumnCells(refRows,calDescId);
	svjmcol.calHistoryId().putColumnCells(refRows,calHistId);
	svjmcol.flag().putColumnCells(refRows,flag);
	svjmcol.snr().putColumnCells(refRows,snr);
	
	
	nMain = tab->nRowMain();
	
      }
    }
  }

  delete tab;

};
*/
} //# NAMESPACE CASA - END

