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

  nChanParList()=vs.numberChan();
  startChanList()=vs.startChan();
  
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
  

  Vector<Int> nSlot(nSpw(),0);

  // Count slots per spw, and set nChan  
  Block<String> columns(2);
  columns[0] = "TIME";
  columns[1] = "SPECTRAL_WINDOW_ID";
  Table sysCalTab(sysCalTabName_,Table::Old);
  TableIterator sysCalIter(sysCalTab,columns);

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

}

void StandardTsys::specify(const Record& specify) {

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

    // Ensure [pol,chan] shapes match
    //  TBD: relax this, and interpolate?
    if (!tsysshape.getFirst(2).isEqual(cs().par(ispw).shape().getFirst(2)))
      throw(AipsError("SYSCAL Tsys Spectrum shape doesn't match data! Cannot proceed."));

  /*
    cout << iter << " "
	 << MVTime(timestamp/C::day).string(MVTime::YMD,7)
	 << " spw=" << ispw 
	 << " nrow = " << mssc.nrow() 
	 << " shape = " << tsys.shape() 
	 << endl;
  */

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


void StandardTsys::calcAllJones() {

  // Antenna-based factors are the sqrt(Tsys)
  currJElem()=sqrt(currCPar());
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
  calDevTabName_(vs.msName()+"/CALDEVICE")
{
  if (prtlev()>2) cout << "EVLAGainTsys::EVLAGainTsys(vs)" << endl;

  nChanParList().set(1);
  startChanList().set(0);
  
}

EVLAGainTsys::EVLAGainTsys(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt)
{
  if (prtlev()>2) cout << "EVLAGainTsys::EVLAGainTsys(nAnt)" << endl;
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

}

void EVLAGainTsys::specify(const Record& specify) {

  // Escape if SYSPOWER or CALDEVICE tables absent
  if (!Table::isReadable(sysPowTabName_))
    throw(AipsError("The SYSPOWER subtable is not present in the specified MS."));
  if (!Table::isReadable(calDevTabName_))
    throw(AipsError("The CALDEVICE subtable is not present in the specified MS."));
 

  // Fill the Tcals first
  fillTcals();

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
  
    Bool anyantgood(False);
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

	anyantgood=True;
	IPosition blc4(4,0,0,thisant,slot);
	//	IPosition trc4(4,1,0,thisant,slot);
	//	IPosition stp4(4,1,1,1,1);
	IPosition trc4(4,2*nrec-1,0,thisant,slot);
	IPosition stp4(4,nrec,1,1,1);
	Vector<Complex> currgain(cs().par(ispw)(blc4,trc4,stp4).nonDegenerate(1));
	convertArray(currgain,sqrt(currpdif/currtcal));
	cs().parOK(ispw)(blc4,trc4,stp4).nonDegenerate(2)= True;
	cs().parErr(ispw)(blc4,trc4,stp4).nonDegenerate(2)= 0.0;
	cs().parSNR(ispw)(blc4,trc4,stp4).nonDegenerate(2)= 1.0;
	
	blc4(0)=1;
	Vector<Complex> currtsys(cs().par(ispw)(blc4,trc4,stp4).nonDegenerate(1));
	convertArray(currtsys,currtcal*currpsum/currpdif/2.0);
	cs().parOK(ispw)(blc4,trc4,stp4).nonDegenerate(2)= True;
	cs().parErr(ispw)(blc4,trc4,stp4).nonDegenerate(2)= 0.0;
	cs().parSNR(ispw)(blc4,trc4,stp4).nonDegenerate(2)= 1.0;

	// Increment good counter
	++goodcount(ispw,thisant);

      }

    }
    cs().solutionOK(ispw)(slot) = anyantgood;

    // increment spw-dep slot counter
    ++islot(ispw);
    sysPowIter.next();
    ++iter;
  }

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

void EVLAGainTsys::calcAllJones() {

  // 0th and 2nd pars are the gains
  currJElem()=currCPar()(Slice(0,2,2),Slice(),Slice());
  currJElemOK()=currParOK()(Slice(0,2,2),Slice(),Slice());

}

void EVLAGainTsys::syncWtScale() {

  Int nPolWt=currCPar().shape()(0)/2;

  currWtScale().resize(nPolWt,nAnt());

  Matrix<Complex> cCP(currCPar().nonDegenerate(0));
  Matrix<Float> Tsys(nPolWt,nAnt());
  Tsys=real(cCP(Slice(1,2,2),Slice()));
  Tsys(Tsys<FLT_MIN)=1.0;

  currWtScale() = 1.0f/Tsys;

}

} //# NAMESPACE CASA - END
