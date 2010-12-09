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
    AlwaysAssert(tsysshape.getFirst(2).isEqual(cs().par(ispw).shape().getFirst(2)),AipsError);

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
      Int thisant=ants(iant);
      IPosition blc4(4,0,             0,             thisant,slot);
      IPosition trc4(4,tsysshape(0)-1,tsysshape(1)-1,thisant,slot);
      Array<Complex> currpar(cs().par(ispw)(blc4,trc4).nonDegenerate(2));
      Array<Float> currtsys(tsys.xyPlane(thisant));
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
  
  cout << "currJElem().shape() = " << currJElem().shape() << endl;
  cout << "currCPar().shape() = " << currCPar().shape() << endl;

  currJElem()=sqrt(currCPar());
  currJElemOK()=currParOK();

}


} //# NAMESPACE CASA - END
