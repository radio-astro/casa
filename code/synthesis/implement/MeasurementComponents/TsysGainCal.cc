//# TsysGainCal.cc: Implementation of Tsys/Gain scalings types
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

#include <synthesis/MeasurementComponents/TsysGainCal.h>
// not yet: #include <synthesis/MeasurementComponents/CalCorruptor.h>

#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicMath/Math.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <synthesis/CalTables/CTGlobals.h>

#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
// math.h ?

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  StandardTsys Implementations
//

StandardTsys::StandardTsys(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  BJones(vs),              // immediate parent
  sysCalTabName_(vs.msName()+"/SYSCAL")
{
  if (prtlev()>2) cout << "StandardTsys::StandardTsys(vs)" << endl;

  // TBD: get these directly from the MS
  nChanParList()=vs.numberChan();
  startChanList()=vs.startChan();  // should be 0?
  
}

StandardTsys::StandardTsys(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  BJones(nAnt)
{
  if (prtlev()>2) cout << "StandardTsys::StandardTsys(nAnt)" << endl;
}

StandardTsys::~StandardTsys() {
  if (prtlev()>2) cout << "StandardTsys::~StandardTsys()" << endl;
}

void StandardTsys::setSpecify(const Record& specify) {

  LogMessage message(LogOrigin("StandardTsys","setSpecify"));

  // Escape if SYSCAL table absent
  if (!Table::isReadable(sysCalTabName_))
    throw(AipsError("The SYSCAL subtable is not present in the specified MS."));

  // Not actually applying or solving
  setSolved(False);
  setApplied(False);

  // Collect Cal table parameters
  if (specify.isDefined("caltable")) {
    calTableName()=specify.asString("caltable");

    if (Table::isReadable(calTableName()))
      logSink() << "FYI: We are going to overwrite an existing CalTable: "
		<< calTableName()
		<< LogIO::POST;
  }

  // we are creating a table from scratch
  logSink() << "Creating " << typeName()
	    << " table from MS SYSCAL subtable."
	    << LogIO::POST;
  

  Table sysCalTab(sysCalTabName_,Table::Old);

  // Verify required columns in SYSCAL
  {
    MSSysCal mssc(sysCalTab);
    MSSysCalColumns sscol(mssc);
    if ( (sscol.spectralWindowId().isNull() || 
	  !sscol.spectralWindowId().isDefined(0)) ||
	 (sscol.time().isNull() || 
	  !sscol.time().isDefined(0)) ||
	 (sscol.interval().isNull() || 
	  !sscol.interval().isDefined(0)) ||
	 (sscol.antennaId().isNull() || 
	  !sscol.antennaId().isDefined(0)) ||
	 (sscol.tsysSpectrum().isNull() || 
	  !sscol.tsysSpectrum().isDefined(0)) )
      throw(AipsError("SYSCAL table is incomplete. Cannot proceed."));
  }

  // Create a new caltable to fill up
  createMemCalTable();

  // Setup shape of solveAllRPar
  initSolvePar();

  /* NEWCALTABLE

  Vector<Int> nSlot(nSpw(),0);
  // Count slots per spw, and set nChan  
  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  TableIterator sysCalIter(sysCalTab,columns);

  // Iterate
  Int iter=0;
  while (!sysCalIter.pastEnd()) {
    MSSysCal mssc(sysCalIter.table());
    MSSysCalColumns sccol(mssc);

    Int ispw=sccol.spectralWindowId()(0);
    nSlot(ispw)++;

    sysCalIter.next();
    ++iter;
  }

  // Create a pristine CalSet
  cs_ = new CalSet<Complex>(nSpw());
  cs().initCalTableDesc(typeName(),parType_);
  
  inflate(nChanParList(),startChanList(),nSlot);

  // Set parOK,etc. to true
  for (Int ispw=0;ispw<nSpw();ispw++) {
    if (cs().nTime(ispw)>0) {
      cs().par(ispw).set(0.0);
      cs().parOK(ispw)=True;
      cs().solutionOK(ispw)=True;
    }
  }
  */

}



void StandardTsys::specify(const Record&) {

  // Escape if SYSCAL table absent
  if (!Table::isReadable(sysCalTabName_))
    throw(AipsError("The SYSCAL subtable is not present in the specified MS. Tsys unavailable."));

  // Keep a count of the number of Tsys found per spw/ant
  Matrix<Int> tsyscount(nSpw(),nAnt(),0);

  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  Table sysCalTab(sysCalTabName_,Table::Old);
  TableIterator sysCalIter(sysCalTab,columns);

  // Iterate
  Int iter(0);
  while (!sysCalIter.pastEnd()) {

    // First extract info from SYSCAL
    MSSysCal mssc(sysCalIter.table());
    MSSysCalColumns sccol(mssc);

    Int ispw=sccol.spectralWindowId()(0);
    currSpw()=ispw; // registers everything else!
    Double timestamp=sccol.time()(0);
    Double interval=sccol.interval()(0);

    Vector<Int> ants;
    sccol.antennaId().getColumn(ants);

    Cube<Float> tsys;
    sccol.tsysSpectrum().getColumn(tsys);
    IPosition tsysshape(tsys.shape());

    // Insist only that channel axis matches
    if (tsysshape(1)!=nChanPar())
      throw(AipsError("SYSCAL Tsys Spectrum channel axis shape doesn't match data! Cannot proceed."));

    //  ...and that tsys pol axis makes sense
    if (tsysshape(0)>2)
      throw(AipsError("Tsys pol axis is implausible"));

  /*
    cout << iter << " "
	 << MVTime(timestamp/C::day).string(MVTime::YMD,7)
	 << " spw=" << ispw 
	 << " shape = " << tsys.shape() 
	 << endl;
  */

    // Now prepare to store in a caltable
    currSpw()=ispw; // registers everything else!
    refTime()=timestamp-interval/2.0;
    currField()=-1;  // don't know this yet

    // Initialize solveAllRPar, etc.
    solveAllRPar()=0.0;
    solveAllParOK()=True;  // Assume all ok
    solveAllParErr()=0.1;  // what should we use here?  ~1/bandwidth?
    solveAllParSNR()=1.0;

    IPosition blc(3,0,0,0), trc(3,tsysshape(0)-1,nChanPar()-1,0);
    for (uInt iant=0;iant<ants.nelements();++iant) {
      Int thisant=ants(iant);
      blc(2)=trc(2)=thisant; // the MS antenna index (not loop index)
      Array<Float> currtsys(tsys.xyPlane(iant));
      solveAllRPar()(blc,trc).nonDegenerate(2)=currtsys;
      solveAllParOK()(blc,trc)=True;

      // Increment tsys counter
      ++tsyscount(ispw,thisant);

    }
    keepNCT();

    sysCalIter.next();
    ++iter;
  }

  // Assign scan and fieldid info
  assignCTScanField(*ct_,msName());

  logSink() << "Tsys counts per spw for antenna Ids 0-"<<nElem()-1<<":" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Int> tsyscountspw(tsyscount.row(ispw));
    if (sum(tsyscountspw)>0)
      logSink() << "Spw " << ispw << ": " << tsyscountspw 
		<< " (" << sum(tsyscountspw) << ")" 
		<< LogIO::POST;
    else
      logSink() << "Spw " << ispw << ": NONE." << LogIO::POST;
  }

}

/*
void StandardTsys::specify(const Record&) {

  // Escape if SYSCAL table absent
  if (!Table::isReadable(sysCalTabName_))
    throw(AipsError("The SYSCAL subtable is not present in the specified MS. Tsys unavailable."));
  // Keep a count of the number of Tsys found per spw/ant
  Matrix<Int> tsyscount(nSpw(),nElem(),0);

  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  Table sysCalTab(sysCalTabName_,Table::Old);
  TableIterator sysCalIter(sysCalTab,columns);

  // Iterate
  Int iter(0);
  Vector<Int> islot(nSpw(),0);
  while (!sysCalIter.pastEnd()) {
    MSSysCal mssc(sysCalIter.table());
    MSSysCalColumns sccol(mssc);

    Int ispw=sccol.spectralWindowId()(0);
    Double timestamp=sccol.time()(0);
    Double interval=sccol.interval()(0);

    Vector<Int> ants;
    sccol.antennaId().getColumn(ants);

    Cube<Float> tsys;
    sccol.tsysSpectrum().getColumn(tsys);
    IPosition tsysshape(tsys.shape());

    // Insist only that channel axis matches
    if (tsysshape(1)!=cs().par(ispw).shape()(1))
      throw(AipsError("SYSCAL Tsys Spectrum channel axis shape doesn't match data! Cannot proceed."));

    //  ...and that tsys pol axis makes sense
    if (tsysshape(0)>2)
      throw(AipsError("Tsys pol axis is implausible"));

    Int slot=islot(ispw);

    cs().fieldId(ispw)(slot)=-1;
    cs().time(ispw)(slot)=timestamp;

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(ispw)(slot)=0.0;
    cs().stopTime(ispw)(slot)=interval;

    // For now, just make these non-zero:
    cs().iFit(ispw).column(slot)=1.0;
    cs().iFitwt(ispw).column(slot)=1.0;
    cs().fit(ispw)(slot)=1.0;
    cs().fitwt(ispw)(slot)=1.0;

    for (uInt iant=0;iant<ants.nelements();++iant) {

      Array<Float> currtsys(tsys.xyPlane(iant));
      Int thisant=ants(iant);

      IPosition blc4(4,0,             0,             thisant,slot);
      IPosition trc4(4,tsysshape(0)-1,tsysshape(1)-1,thisant,slot);
      Array<Complex> currpar(cs().par(ispw)(blc4,trc4).nonDegenerate(2));
      convertArray(currpar,currtsys);
      cs().parOK(ispw)(blc4,trc4).nonDegenerate(2)= True;
      cs().parErr(ispw)(blc4,trc4).nonDegenerate(2)= 0.0;
      cs().parSNR(ispw)(blc4,trc4).nonDegenerate(2)= 1.0;

      // Increment counter
      ++tsyscount(ispw,thisant);

    }
    cs().solutionOK(ispw)(slot) = True;

    // increment spw-dep slot counter
    ++islot(ispw);

    sysCalIter.next();
    ++iter;
  }


  logSink() << "Tsys counts per spw for antenna Ids 0-"<<nElem()-1<<":" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Int> tsyscountspw(tsyscount.row(ispw));
    if (sum(tsyscountspw)>0)
      logSink() << "Spw " << ispw << ": " << tsyscountspw 
		<< " (" << sum(tsyscountspw) << ")" 
		<< LogIO::POST;
    else
      logSink() << "Spw " << ispw << ": NONE." << LogIO::POST;
  }



}
*/

void StandardTsys::calcAllJones() {

  // Antenna-based factors are the sqrt(Tsys)
  //  currJElem()=sqrt(currRPar());  NEWCALTABLE
  convertArray(currJElem(),sqrt(currRPar()));
  currJElemOK()=currParOK();

}





// **********************************************************
//  EVLAGainTsys Implementations
//

EVLAGainTsys::EVLAGainTsys(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  GJones(vs),              // immediate parent
  sysPowTabName_(vs.msName()+"/SYSPOWER"),
  calDevTabName_(vs.msName()+"/CALDEVICE"),
  correff_(Float(0.932)),     // EVLA-specific net corr efficiency (4bit)
  frgrotscale_(Float(1.176)), // EVLA-specific fringe rotation mean _scale_
  nyquist_(1.0),
  effChBW_()

{
  if (prtlev()>2) cout << "EVLAGainTsys::EVLAGainTsys(vs)" << endl;

  nChanParList().set(1);
  startChanList().set(0);

  // Get spw total bandwidths
  const ROMSSpWindowColumns& spwcols = vs.iter().msColumns().spectralWindow();
  effChBW_.resize(nSpw());
  for (Int ispw=0;ispw<nSpw();++ispw) 
    effChBW_(ispw)=Vector<Double>(spwcols.effectiveBW()(0))(0);
  
}

EVLAGainTsys::EVLAGainTsys(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt)
{
  if (prtlev()>2) cout << "EVLAGainTsys::EVLAGainTsys(nAnt)" << endl;

  throw(AipsError("Cannot use EVLAGainTsys with generic ctor."));

}

EVLAGainTsys::~EVLAGainTsys() {
  if (prtlev()>2) cout << "EVLAGainTsys::~EVLAGainTsys()" << endl;
}

void EVLAGainTsys::setSpecify(const Record& specify) {

  LogMessage message(LogOrigin("EVLAGainTsys","setSpecify"));

  // Escape if SYSPOWER or CALDEVICE tables absent
  if (!Table::isReadable(sysPowTabName_))
    throw(AipsError("The SYSPOWER subtable is not present in the specified MS."));
  if (!Table::isReadable(calDevTabName_))
    throw(AipsError("The CALDEVICE subtable is not present in the specified MS."));

  // Not actually applying or solving
  setSolved(False);
  setApplied(False);

  // Collect Cal table parameters
  if (specify.isDefined("caltable")) {
    calTableName()=specify.asString("caltable");

    if (Table::isReadable(calTableName()))
      logSink() << "FYI: We are going to overwrite an existing CalTable: "
		<< calTableName()
		<< LogIO::POST;
  }

  // we are creating a table from scratch
  logSink() << "Creating " << typeName()
    //	    << " table from CANNED VALUES."
    	    << " table from MS SYSPOWER/CALDEVICE subtables."
	    << LogIO::POST;

  // Create a caltable to fill up
  createMemCalTable();

  // Init the shapes of solveAllRPar, etc.
  initSolvePar();


  /*  NEWCALTABLE
  Vector<Int> nSlot(nSpw(),0);

  // Count slots per spw, and set nChan  
  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  Table sysPowTab(sysPowTabName_,Table::Old);
  TableIterator sysPowIter(sysPowTab,columns);

  // Iterate
  Int iter=0;
  while (!sysPowIter.pastEnd()) {
    Table itab(sysPowIter.table());

    ROScalarColumn<Int> spwCol(itab,"SPECTRAL_WINDOW_ID");

    Int ispw=spwCol(0);
    nSlot(ispw)++;

    sysPowIter.next();
    ++iter;
  }

  cout << "nSlot = " << nSlot << endl;

  // Create a pristine CalSet
  cs_ = new CalSet<Complex>(nSpw());
  cs().initCalTableDesc(typeName(),parType_);
  
  inflate(nChanParList(),startChanList(),nSlot);
  */

}

void EVLAGainTsys::specify(const Record&) {

  // Escape if SYSPOWER or CALDEVICE tables absent
  if (!Table::isReadable(sysPowTabName_))
    throw(AipsError("The SYSPOWER subtable is not present in the specified MS."));
  if (!Table::isReadable(calDevTabName_))
    throw(AipsError("The CALDEVICE subtable is not present in the specified MS."));
 

  // Fill the Tcals first
  fillTcals();

  // The net digital factor for antenna-based (voltage) gain
  Float dig=sqrt(correff_*frgrotscale_);

  // Keep a count of the number of Tsys found per spw/ant
  Matrix<Int> goodcount(nSpw(),nElem(),0), badcount(nSpw(),nElem(),0);

  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  Table sysPowTab(sysPowTabName_,Table::Old);
  TableIterator sysPowIter(sysPowTab,columns);

  // Iterate
  Int iter(0);
  Vector<Int> islot(nSpw(),0);
  while (!sysPowIter.pastEnd()) {

    Table itab(sysPowIter.table());

    ROScalarColumn<Int> spwCol(itab,"SPECTRAL_WINDOW_ID");
    ROScalarColumn<Double> timeCol(itab,"TIME");
    ROScalarColumn<Double> intervalCol(itab,"INTERVAL");
    ROScalarColumn<Int> antCol(itab,"ANTENNA_ID");
    ROArrayColumn<Float> swsumCol(itab,"SWITCHED_SUM");
    ROArrayColumn<Float> swdiffCol(itab,"SWITCHED_DIFF");

    Int ispw=spwCol(0);
    Double timestamp=timeCol(0);
    Double interval=intervalCol(0);

    Vector<Int> ants;
    antCol.getColumn(ants);

    Matrix<Float> psum,pdif;
    swsumCol.getColumn(psum);
    swdiffCol.getColumn(pdif);
    IPosition psumshape(psum.shape());
    IPosition pdifshape(pdif.shape());

    AlwaysAssert(psumshape.isEqual(pdifshape),AipsError);

    // the number of receptors
    Int nrec=psumshape(0);

    // Now prepare to record pars in the caltable
    currSpw()=ispw;
    refTime()=timestamp;
    currField()=-1;  // TBD
    // currScan()=??

    // Initialize solveAllRPar, etc.
    solveAllRPar()=0.0;
    solveAllParOK()=True;  // Assume all ok?
    solveAllParErr()=0.1;  // what should we use here?  ~1/bandwidth?
    solveAllParSNR()=1.0;

    Vector<Float> gain;  // a Vector for referencing slices of solveAllRPar

    Bool anyantgood(False);
    IPosition blc(3,0,0,0),trc(3,2*nrec-1,0,0),stp(3,nrec,1,1);
    for (uInt iant=0;iant<ants.nelements();++iant) {
      Int thisant=ants(iant);
      Vector<Float> currpsum(psum.column(iant));
      Vector<Float> currpdif(pdif.column(iant));
      Vector<Float> currtcal(tcals_.xyPlane(ispw).column(thisant));

      // If any of the required values are goofy, we'll skip this antenna
      Bool good=(allGT(currtcal,FLT_MIN) &&
		 allGT(currpdif,FLT_MIN) &&
		 allGT(currpsum,FLT_MIN));

      if (!good) 
	// Increment bad counter
	++badcount(ispw,thisant);
      else {

	blc(2)=trc(2)=thisant; // the MS ant index (not loop index)
	
	blc(0)=0;
	gain.reference(solveAllRPar()(blc,trc,stp).nonDegenerate(1));   // 'gain'
	gain=sqrt(currpdif/currtcal);
	gain*=dig; // scale by net digital factor

	blc(0)=1;
	solveAllRPar()(blc,trc,stp).nonDegenerate(1)=(currtcal*currpsum/currpdif/2.0);  // 'tsys'
	
	solveAllParOK().xyPlane(thisant)=True;
      
	// Increment good counter
	++goodcount(ispw,thisant);
	anyantgood=True;

      }

    }

    // Record in the memory caltable
    keepNCT();

    sysPowIter.next();
    ++iter;
  }

  // Set scan and fieldid info
  assignCTScanField(*ct_,msName());

  logSink() << "GOOD gain counts per spw for antenna Ids 0-"<<nElem()-1<<":" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Int> goodcountspw(goodcount.row(ispw));
    if (sum(goodcountspw)>0)
      logSink() << "  Spw " << ispw << ": " << goodcountspw 
		<< " (" << sum(goodcountspw) << ")" 
		<< LogIO::POST;
    else
      logSink() << "Spw " << ispw << ": NONE." << LogIO::POST;
  }

  logSink() << "BAD gain counts per spw for antenna Ids 0-"<<nElem()-1<<":" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Int> badcountspw(badcount.row(ispw));
    if (sum(badcountspw)>0)
      logSink() << "  Spw " << ispw << ": " << badcountspw 
		<< " (" << sum(badcountspw) << ")" 
		<< LogIO::POST;
  }

  logSink() << "BAD gain count FRACTION per spw for antenna Ids 0-"<<nElem()-1<<":" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Float> badcountspw(badcount.row(ispw).shape());
    Vector<Float> goodcountspw(goodcount.row(ispw).shape());
    convertArray(badcountspw,badcount.row(ispw));
    convertArray(goodcountspw,goodcount.row(ispw));
    if (sum(badcountspw)>0.0f || sum(goodcountspw)>0.0f) {
      Vector<Float> fracbad=badcountspw/(badcountspw+goodcountspw);
      fracbad=floor(1000.0f*fracbad)/1000.0f;
      Float fracbadsum=sum(badcountspw)/(sum(badcountspw)+sum(goodcountspw));
      fracbadsum=floor(1000.0f*fracbadsum)/1000.0f;
      logSink() << "  Spw " << ispw << ": " << fracbad
		<< " (" << fracbadsum << ")" 
		<< LogIO::POST;
    }
  }


}

void EVLAGainTsys::fillTcals() {

  logSink() << "Filling Tcals from the CALDEVICE table." << LogIO::POST;

  Block<String> columns(2);
  columns[0] = "SPECTRAL_WINDOW_ID";
  columns[1] = "ANTENNA_ID";
  Table calDevTab(calDevTabName_,Table::Old);
  TableIterator calDevIter(calDevTab,columns);

  tcals_.resize(2,nElem(),nSpw());
  tcals_.set(-1.0f);

  // Iterate over CALDEVICE table
  Int iter(0);
  Vector<Int> islot(nSpw(),0);
  while (!calDevIter.pastEnd()) {

    Table itab(calDevIter.table());

    ROScalarColumn<Int> spwCol(itab,"SPECTRAL_WINDOW_ID");
    ROScalarColumn<Double> timeCol(itab,"TIME");
    ROScalarColumn<Double> intervalCol(itab,"INTERVAL");
    ROScalarColumn<Int> antCol(itab,"ANTENNA_ID");
    ROScalarColumn<Int> numLoadCol(itab,"NUM_CAL_LOAD");

    ROArrayColumn<Float> noiseCalCol(itab,"NOISE_CAL");

    Int ispw=spwCol(0);
    Int iant=antCol(0);
    Int nTcal=noiseCalCol(0).nelements();
    
    Vector<Float> thisTcal=noiseCalCol(0);

    if (nTcal==1) {
      AlwaysAssert(thisTcal.nelements()==1,AipsError);
      tcals_.xyPlane(ispw).column(iant)=thisTcal(0);
    }
    else {
      AlwaysAssert(thisTcal.nelements()==2,AipsError);
      tcals_.xyPlane(ispw).column(iant)=thisTcal;
    }

    // Increment the iterator
    ++iter;
    calDevIter.next();
  }

}

// Local version to extract integration time and bandwidth
void EVLAGainTsys::applyCal(VisBuffer& vb, Cube<Complex>& Vout,
			    Bool avoidACs) {

  // Calculate Nyquist factor so we get weights right
  //  (sqrt because it will be applied in antenna-based manner)
  nyquist_=Float(sqrt(2.0*vb.exposure()(0)*effChBW_(currSpw())));
		 
  //  cout << "currSpw() = " << currSpw() << " nyquist_= " << nyquist_ << endl;

  // Call parent to get the rest done as usual
  GJones::applyCal(vb,Vout,avoidACs);

}
  


void EVLAGainTsys::calcAllJones() {

  // 0th and 2nd pars are the gains
  //  currJElem()=currRPar()(Slice(0,2,2),Slice(),Slice()); // NEWCALTABLE
  convertArray(currJElem(),currRPar()(Slice(0,2,2),Slice(),Slice()));
  currJElemOK()=currParOK()(Slice(0,2,2),Slice(),Slice());

}

void EVLAGainTsys::syncWtScale() {

  Int nPolWt=currRPar().shape()(0)/2;
  currWtScale().resize(nPolWt,nAnt());

  Matrix<Float> Tsys(currRPar()(Slice(1,2,2),Slice(),Slice()).nonDegenerate(1));
  Tsys(Tsys<FLT_MIN)=1.0;  // OK?

  currWtScale() = 1.0f/Tsys;
  currWtScale()*=correff_; // reduce by correlator efficiency (per ant)
  currWtScale()*=nyquist_;

  //  cout << "Tsys = " << Tsys << endl;
  //  cout << "currWtScale() = " << currWtScale() << endl;

}

/*
void EVLAGainTsys::updateWt(Vector<Float>& wt,const Int& a1,const Int& a2) {

  Vector<Float> ws1(currWtScale().column(a1));
  Vector<Float> ws2(currWtScale().column(a2));

  if (a1==0 && a2==1) {
    cout << "wt = " << wt << endl;
    cout << "ws1 = " << ws1 << endl;
    cout << "ws2 = " << ws2 << endl;
  }

  VisJones::updateWt(wt,a1,a2);

  if (a1==0 && a2==1) {
    cout << "wt = " << wt << endl;
  }
}
*/

} //# NAMESPACE CASA - END
