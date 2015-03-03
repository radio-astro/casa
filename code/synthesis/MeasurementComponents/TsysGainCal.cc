//# TsysGainCal.cc: Implementation of Tsys calibration
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

#include <casa/Arrays/MaskArrMath.h>
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
  sysCalTabName_(vs.sysCalTableName()),
  freqDepCalWt_(False)
{
  if (prtlev()>2) cout << "StandardTsys::StandardTsys(vs)" << endl;

  // TBD: get these directly from the MS
  nChanParList()=vs.numberChan();
  startChanList()=vs.startChan();  // should be 0?
  
}

StandardTsys::StandardTsys(String msname,Int MSnAnt,Int MSnSpw) :
  VisCal(msname,MSnAnt,MSnSpw),             // virtual base
  VisMueller(msname,MSnAnt,MSnSpw),         // virtual base
  BJones(msname,MSnAnt,MSnSpw),              // immediate parent
  sysCalTabName_(""),
  freqDepCalWt_(False)
{
  if (prtlev()>2) cout << "StandardTsys::StandardTsys(msname,MSnAnt,MSnSpw)" << endl;

  // Set the SYSCAL table name
  MeasurementSet ms(msname);
  sysCalTabName_ = ms.sysCalTableName();

  // OK?
  ROMSColumns mscol(ms);
  const ROMSSpWindowColumns& spwcols = mscol.spectralWindow();
  nChanParList()=spwcols.numChan().getColumn();
  startChanList().set(0);

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
    ROMSSysCalColumns sscol(mssc);
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

}



void StandardTsys::specify(const Record&) {

  // Escape if SYSCAL table absent
  if (!Table::isReadable(sysCalTabName_))
    throw(AipsError("The SYSCAL subtable is not present in the specified MS. Tsys unavailable."));

  // Keep a count of the number of Tsys found per spw/ant
  Matrix<Int> tsyscount(nSpw(),nAnt(),0);
  Matrix<Int> negTsys(nSpw(),nAnt(),0);

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
    ROMSSysCalColumns sccol(mssc);

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

    // Now prepare to store in a caltable
    currSpw()=ispw; // registers everything else!
    refTime()=timestamp-interval/2.0;
    currField()=-1;  // don't know this yet

    // Initialize solveAllRPar, etc.
    solveAllRPar()=0.0;
    solveAllParOK()=True;  // Assume all ok
    solveAllParErr()=0.1;  // what should we use here?  ~1/bandwidth?
    solveAllParSNR()=1.0;

    Int npol=tsysshape(0);
    IPosition blc(3,0,0,0), trc(3,npol-1,nChanPar()-1,0);
    for (uInt iant=0;iant<ants.nelements();++iant) {
      Int thisant=ants(iant);
      blc(2)=trc(2)=thisant; // the MS antenna index (not loop index)
      Array<Float> currtsys(tsys.xyPlane(iant));
      solveAllRPar()(blc,trc).nonDegenerate(2)=currtsys;
      solveAllParOK()(blc,trc)=True;

      // Increment tsys counter
      ++tsyscount(ispw,thisant);

      negTsys(ispw,thisant)+=ntrue(currtsys<FLT_MIN);

      // Issue warnings for completely bogus Tsys spectra (per pol)
      for (Int ipol=0;ipol<npol;++ipol) {
        if (nfalse(Matrix<Float>(currtsys).row(ipol)<FLT_MIN)==0)
	  logSink() << "  Tsys data for ant id=" 
                    << iant << " (pol=" << ipol<< ")"
		    << " in spw " << ispw
		    << " at t=" << MVTime(refTime()/C::day).string(MVTime::YMD,7) 
		    << " are all negative or zero will be entirely flagged."
		    << LogIO::WARN << LogIO::POST;
      }
    }
    // Flag any Tsys<=0.0
    
    LogicalArray mask((solveAllRPar()<FLT_MIN));
    MaskedArray<Bool> negs(solveAllParOK(),mask);
    negs=False;

    keepNCT();

    sysCalIter.next();
    ++iter;
  }

  // Assign scan and fieldid info
  assignCTScanField(*ct_,msName());

  logSink() << "Tsys spectra counts per spw for antenna Ids 0-"<<nElem()-1<<" (per pol):" << LogIO::POST;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Int> tsyscountspw(tsyscount.row(ispw));
    if (sum(tsyscountspw)>0) {
      logSink() << "Spw " << ispw << ": " << tsyscountspw 
		<< " (=" << sum(tsyscountspw) << " spectra;" 
		<< " " << nChanParList()(ispw) << " chans per spectra, per pol)" 
      		<< LogIO::POST;
      for (Int iant=0;iant<nAnt();++iant) {
        if (negTsys(ispw,iant)>0)
	  logSink() << "  (Found and flagged " << negTsys(ispw,iant) 
		    << " spurious negative (or zero) Tsys channels for ant id=" 
		    << iant << " in spw " << ispw << ".)"
		    << LogIO::POST;
      }
    }
    //    else
    //      logSink() << "Spw " << ispw << ": NONE." << LogIO::POST;
  }

}

void StandardTsys::correct2(vi::VisBuffer2& vb, Bool trial, Bool doWtSp) {

  // Signal channelized weight calibration downstream
  freqDepCalWt_=doWtSp;

  // Call parent:
  BJones::correct2(vb,trial,doWtSp);
}


// Specialized calcPar that does some sanity checking
void StandardTsys::calcPar() {

  // Call parent (this drives generalized interpolation)
  SolvableVisCal::calcPar();

  // Since some interpolation types may unwittingly yield
  //  negative Tsys, we'll trap that here by flagging and zeroing them
  Cube<Bool> mask(currRPar()<Float(0.0));
  currParOK()(mask)=False;
  currRPar()(mask)=0.0;  // avoids NaN generation in sqrt, even for flagged points
}



// Specialized Jones Matrix calculation
void StandardTsys::calcAllJones() {
  
  // Antenna-based factors are the sqrt(Tsys)
  convertArray(currJElem(),sqrt(currRPar()));
  currJElemOK()=currParOK();

}

void StandardTsys::syncWtScale() {

  //  cout << "VJ::syncWtScale (" << typeName() << ")" << endl;

  if (freqDepCalWt_) {

    // We _will_ do a channelized weight calibration, so shape currWtScale() appropriately

    // Ensure proper size according to Jones matrix type
    switch (this->jonesType()) {
    case Jones::Scalar: 
    case Jones::Diagonal: {
      currWtScale().resize(currRPar().shape());  // same as Tsys cube
      currWtScale()=0.0;
      break;
    }
    default: {
      // Only diag and scalar versions can adjust weights
      //    cout<< "Turning off calWt()" << endl;
      calWt()=False;
      return;
      break;
    }
    }

    // Calculate the weight scale factors spectrally
    calcWtScale2();


  //  cout << "VJ::syncWtScale: currWtScale() = " << currWtScale() << endl;


  }
  else

    // Just do the single-chan thing
    BJones::syncWtScale();

}


// Calculate weight update
void StandardTsys::calcWtScale() {

  // Initialize  to 1.0
  //   (will only be replaced if a valid calculation is possible)
  currWtScale().set(1.0);

  IPosition ash(currRPar().shape());
  ash(1)=1;  // only on channel weight (so far)
  Cube<Float> cWS(currWtScale());

  // For each pol and antenna, form 1/mean(Tsys(f))
  IPosition it3(2,0,2);
  ArrayIterator<Float> Tsys(currRPar(),it3,False);
  ArrayIterator<Bool> Tok(currParOK(),it3,False);
  ArrayIterator<Float> cWSi(cWS,it3,False);

  while (!Tsys.pastEnd()) {

    // mask out flagged channels
    MaskedArray<Float> Tsysm(Tsys.array()(Tok.array()));

    // If any good Tsys this ant/pol, calc the wt scale (else it remains 1.0)
    if (Tsysm.nelementsValid()>0) {
      Float meanTsys=mean(Tsysm);
      if (meanTsys>0.0)
	cWSi.array().set(1./meanTsys);
    }

    Tsys.next();
    Tok.next();
    cWSi.next();
  }

}

// Calculate weight update
void StandardTsys::calcWtScale2() {

  // 1/Tsys (only where ok)
  currWtScale()(currParOK())=1.f/currRPar()(currParOK());

}

} //# NAMESPACE CASA - END
