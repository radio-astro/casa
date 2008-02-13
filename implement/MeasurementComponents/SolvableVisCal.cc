//# SolvableVisCal.cc: Implementation of SolvableVisCal classes
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
//# $Id: VisCal.cc,v 1.15 2006/02/06 19:23:11 gmoellen Exp $

#include <synthesis/MeasurementComponents/SolvableVisCal.h>

#include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/Utilities/GenSort.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <casa/sstream.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include <casa/Quanta/MVTime.h>

#include <fstream>

namespace casa { //# NAMESPACE CASA - BEGIN

// **********************************************************
//  SolvableVisCal Implementations
//

SolvableVisCal::SolvableVisCal(VisSet& vs) :
  VisCal(vs),
  cs_(NULL),
  cint_(NULL),
  calTableName_(""),
  calTableSelect_(""),
  append_(False),
  tInterpType_(""),
  fInterpType_(""),
  spwMap_(vs.numberSpw(),-1),
  refant_(-1),
  solved_(False),
  apmode_(""),
  solnorm_(False),
  minSNR_(0.0f),
  focusChan_(0),
  dataInterval_(0.0),
  fitWt_(0.0),
  fit_(0.0),
  solveCPar_(vs.numberSpw(),NULL),
  solveRPar_(vs.numberSpw(),NULL),
  solveParOK_(vs.numberSpw(),NULL),
  solveParErr_(vs.numberSpw(),NULL),
  solveParSNR_(vs.numberSpw(),NULL),
  srcPolPar_()
{

  if (prtlev()>2) cout << "SVC::SVC(vs)" << endl;

  initSVC();

};

SolvableVisCal::SolvableVisCal(const Int& nAnt) :
  VisCal(nAnt),
  cs_(NULL),
  cint_(NULL),
  calTableName_(""),
  calTableSelect_(""),
  append_(False),
  tInterpType_(""),
  fInterpType_(""),
  spwMap_(1,-1),
  refant_(-1),
  solved_(False),
  apmode_(""),
  solnorm_(False),
  minSNR_(0.0),
  focusChan_(0),
  dataInterval_(0.0),
  fitWt_(0.0),
  fit_(0.0),
  solveCPar_(1,NULL),
  solveRPar_(1,NULL),
  solveParOK_(1,NULL),
  solveParErr_(1,NULL),
  solveParSNR_(1,NULL),
  srcPolPar_()
{  

  if (prtlev()>2) cout << "SVC::SVC(i,j,k)" << endl;

  initSVC();

}

SolvableVisCal::~SolvableVisCal() {

  if (prtlev()>2) cout << "SVC::~SVC()" << endl;

  deleteSVC();

  if (cs_)   delete cs_;   cs_=NULL;
  if (cint_) delete cint_; cint_=NULL;

}


// Generic setapply
void SolvableVisCal::setApply() {

  if (prtlev()>2) cout << "SVC::setApply()" << endl;

  // Generic settings
  calTableName()="<none>";
  calTableSelect()="<none>";
  tInterpType()="nearest";
  indgen(spwMap());
  interval()=DBL_MAX;

  // This is apply context  
  setApplied(True);
  setSolved(False);

  // Assemble inflated (but empty) CalSet from current shapes
  Vector<Int> nTimes(nSpw(),1);
  cs_ = new CalSet<Complex>(nSpw(),nPar(),nChanParList(),nElem(),nTimes);

  // Assemble CalInterp
  cint_ = new CalInterp(cs(),tInterpType(),"nearest");
  ci().setSpwMap(spwMap());

}

// Setapply from a Cal Table, etc.
void SolvableVisCal::setApply(const Record& apply) {
  //  Inputs:
  //    apply           Record&       Contains application params
  //    

  if (prtlev()>2) cout << "SVC::setApply(apply)" << endl;

  // Call VisCal version for generic stuff
  VisCal::setApply(apply);

  // Collect Cal table parameters
  if (apply.isDefined("table")) {
    calTableName()=apply.asString("table");
    verifyCalTable(calTableName());
  }

  if (apply.isDefined("select"))
    calTableSelect()=apply.asString("select");

  else {
    
    calTableSelect()="";
    //    String spwsel("");
    //    if (apply.isDefined("spw")) {
    //      ostringstream os;
    //      os << Vector<Int>(apply.asArrayInt("spw"));
    //      spwsel = os.str();
    //    }
    //    cout << "spwsel = " << spwsel << endl;

    String fldsel("");
    if (apply.isDefined("field")) {
      ostringstream os;
      os << Vector<Int>(apply.asArrayInt("field"));
      if (os.str()!="[]")
	fldsel = os.str();
    }
    //    cout << "fldsel = " << fldsel << endl;

    if (fldsel.length()>0) {
      ostringstream os;
      os << "(FIELD_ID IN " << fldsel << ")";
      calTableSelect() = os.str();
    }
    //   cout << "calTableSelect() = " << calTableSelect() << endl;
  }
  
 
  // Does this belong here?
  if (apply.isDefined("append"))
    append()=apply.asBool("append");

  // Collect interpolation parameters
  if (apply.isDefined("interp"))
    tInterpType()=apply.asString("interp");

  // Protect against non-specific interp
  if (tInterpType()=="")
    tInterpType()="linear";

  // TBD: move spwmap to VisCal version?

  indgen(spwMap());
  if (apply.isDefined("spwmap")) {
    Vector<Int> spwmap(apply.asArrayInt("spwmap"));
    if (allGE(spwmap,0)) {
      // User has specified a valid spwmap
      if (spwmap.nelements()==1)
	spwMap()=spwmap(0);
      else
	spwMap()(IPosition(1,0),IPosition(1,spwmap.nelements()-1))=spwmap;
      // TBD: Report non-trivial spwmap to logger.
      cout << "Note: spwMap() = " << spwMap() << endl;
    }
  }

  AlwaysAssert(allGE(spwMap(),0),AipsError);

  // TBD: move interval to VisCal version?
  if (apply.isDefined("t"))
    interval()=apply.asFloat("t");

  // This is apply context  
  setApplied(True);
  setSolved(False);

  //  TBD:  "Arranging to apply...."


  // Create CalSet, from table
  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());

  // These come from CalSet now, but will eventually come from CalInterp

  //  cout << "nChanParList().shape() = " << nChanParList().shape() << endl;
  //  cout << "nChanParList()         = " << nChanParList() << endl;
  //  cout << "cs().nChan().shape()   = " << cs().nChan().shape() << endl;
  //  cout << "cs().nChan()           = " << cs().nChan() << endl;

  for (Int ispw=0;ispw<nSpw();++ispw) {
    nChanParList()(ispw) = cs().nChan()(spwMap()(ispw));
    startChanList()(ispw) = cs().startChan()(spwMap()(ispw));
  }

  //  cout << "nChanParList().shape() = " << nChanParList().shape() << endl;
  //  cout << "nChanParList()         = " << nChanParList() << endl;
  //  cout << "cs().nChan().shape()   = " << cs().nChan().shape() << endl;
  //  cout << "cs().nChan()           = " << cs().nChan() << endl;

  // Sanity check on parameter channel shape
  //  AlwaysAssert((freqDepPar()||allEQ(nChanParList(),1)),AipsError);

  // Create CalInterp
  cint_ = new CalInterp(cs(),tInterpType(),"nearest");
  ci().setSpwMap(spwMap());

}

String SolvableVisCal::applyinfo() {

  ostringstream o;
  o << typeName()
    << ": table="  << calTableName()
    << " select=" << calTableSelect()
    << " interp=" << tInterpType()
    << " spwmap=" << spwMap()
    << boolalpha
    << " calWt=" << calWt();
    //    << " t="      << interval();

  return String(o);

}

void SolvableVisCal::setSolve() {

  if (prtlev()>2) cout << "SVC::setSolve()" << endl;

  interval()=10.0;
  refant()=-1;
  apmode()="AP";
  calTableName()="<none>";
  solnorm()=False;
  minSNR()=0.0f;

  // This is the solve context
  setSolved(True);
  setApplied(False);

  // Create a pristine CalSet
  //  TBD: move this to inflate()?
  cs_ = new CalSet<Complex>(nSpw());

}

void SolvableVisCal::setSolve(const Record& solve) 
{

  if (prtlev()>2) cout << "SVC::setSolve(solve)" << endl;

  // Collect parameters
  if (solve.isDefined("t"))
    interval()=solve.asFloat("t");

  if (solve.isDefined("preavg"))
    preavg()=solve.asFloat("preavg");

  if (solve.isDefined("refant"))
    refant()=solve.asInt("refant");

  if (solve.isDefined("apmode"))
    apmode()=solve.asString("apmode");

  apmode().upcase();

  if (solve.isDefined("table"))
    calTableName()=solve.asString("table");

  if (solve.isDefined("append"))
    append()=solve.asBool("append");

  if (solve.isDefined("solnorm"))
    solnorm()=solve.asBool("solnorm");

  if (solve.isDefined("minsnr"))
    minSNR()=solve.asFloat("minsnr");

  //  cout << "SVC::setsolve: minSNR() = " << minSNR() << endl;

  // TBD: Warn if table exists (and append=F)!

  // If normalizable & preavg<0, use inteval for preavg 
  //  (or handle this per type, e.g. D)
  // TBD: make a nice log message concerning preavg
  // TBD: make this work better with solnboundary par
  if (normalizable() && preavg()<0.0)
    if (interval()>0.0)
      // use interval
      preavg()=interval();
    else
      // scan-based, so max out preavg() to get full-chunk time-average
      preavg()=DBL_MAX;

  // This is the solve context
  setSolved(True);
  setApplied(False);

  // Create a pristine CalSet
  //  TBD: move this to inflate()?
  cs_ = new CalSet<Complex>(nSpw());

  //  state();

}

String SolvableVisCal::solveinfo() {

  // Get the refant name from the MS
  String refantName("none");
  if (refant()>-1) {
    MeasurementSet ms(msName());
    MSAntennaColumns mscol(ms.antenna());
    refantName=mscol.name()(refant());
  }

  ostringstream o;
  o << boolalpha
    << typeName()
    << ": table="      << calTableName()
    << " append="     << append()
    << " t="          << interval()
    //    << " preavg="     << preavg()
    << " refant="     << "'" << refantName << "'" // (id=" << refant() << ")"
    << " minsnr=" << minSNR()
    << " apmode="  << apmode()
    << " solnorm=" << solnorm();
  return String(o);

}


void SolvableVisCal::setAccumulate(VisSet& vs,
				   const String& table,
				   const String& select,
				   const Double& t,
				   const Int& refAnt) {

  LogMessage message(LogOrigin("SolvableVisCal","setAccumulate"));

  // meta-info
  calTableName()=table;
  calTableSelect()=select;
  interval()=t;

  // Not actually applying or solving
  setSolved(False);
  setApplied(False);

  // If interval<0, this signals an existing input cumulative table
  if (interval()<0.0) {

    logSink() << "Loading existing " << typeName()
	      << " table: " << table
	      << " for accumulation."
	      << LogIO::POST;

    // Create CalSet, from table
    cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());

    nChanParList() = cs().nChan();
    startChanList() = cs().startChan();

    // The following should be for trivial types only!    
    nChanMatList()=nChanParList();

  }

  // else, we are creating a cumulative table from scratch (the VisSet)
  else {

    logSink() << "Creating " << typeName()
	      << " table for accumulation."
	      << LogIO::POST;

    // Create a pristine CalSet
    //  TBD: move this to inflate()?
    cs_ = new CalSet<Complex>(nSpw());

    // Size, inflate CalSet (incl. filling meta data)
    setSolveChannelization(vs);
    // Override single-channel per solve! (trivial types only)
    nChanMatList()=nChanParList();

    inflate(vs,True);

    // Set parOK,etc. to true
    for (Int ispw=0;ispw<nSpw();ispw++) {
      cs().parOK(ispw)=True;
      cs().solutionOK(ispw)=True;
    }

  }

}

void SolvableVisCal::initSolve(VisSet& vs) {

  if (prtlev()>2) cout << "SVC::initSolve(vs)" << endl;

  // Determine solving channelization according to VisSet & type
  setSolveChannelization(vs);

  // Inflate the CalSet according to VisSet
  inflate(vs);

  // Size the solvePar arrays
  initSolvePar();

}


// Inflate the internal CalSet according to VisSet info
void SolvableVisCal::inflate(VisSet& vs, const Bool& fillMeta) {

  if (prtlev()>3) cout << " SVC::inflate(vs)" << endl;

  // This method sets up various shape parameters
  //  according to the current VisSet.  It is necessary
  //  to run this after Calibrater::setdata and before
  //  the main part of the solve.  (In case the setdata
  //  was run after the setsolve.)  This method calls
  //  a generic version to interpret the data shapes
  //  in the proper context-dependent way and size
  //  the CalSet.

  //  TBD: Move this slot counting exercise out to Calibrater?
  //   --> Not clear we should do this...

  //  TBD: How do we generalize this to create bracketing
  //   slots at scan start/stop (for accumulation context)?

  // Count slots in the VisIter
  Vector<Int> nSlot(nSpw(),0);
  {
    VisIter& vi(vs.iter());
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk())
      nSlot(vi.spectralWindow())++;
    vi.originChunks();

    logSink() << "For interval of "<<interval()<<" seconds, found "
	      <<  sum(nSlot)<<" slots"
	      << LogIO::POST;
  }

  // Call generic version to actually inflate the CalSet
  //  (assumes nChanParList()/startChanList() already valid!)
  inflate(nChanParList(),startChanList(),nSlot);

  // Fill the meta data in the CalSet (according to VisSet)
  //  (NB: currently, only used in accumulate context)
  if (fillMeta) fillMetaData(vs);

}



// Inflate the internal CalSet generically
void SolvableVisCal::inflate(const Vector<Int>& nChan,
			     const Vector<Int>& startChan,
			     const Vector<Int>& nSlot) {

  if (prtlev()>3) cout << "  SVC::inflate(,,)" << endl;

  // Size up the CalSet
  cs().resize(nPar(),nChan,nElem(),nSlot);
  cs().setStartChan(startChan);

}


void SolvableVisCal::setSolveChannelization(VisSet& vs) {

  //  TBD: include anticipated decimation when partial freq ave supported?
  //      (NB: note difference between chan-ave on selection [VisSet] and
  //       chan-ave on-the-fly with vb.freqAve())


  Vector<Int> nDatChan(vs.numberChan());
  Vector<Int> startDatChan(vs.startChan());

  // Figure out channel axis shapes (solve context!):

  // If multi-channel pars, this is a frequency-sampled calibration (e.g., B)
  if (freqDepPar()) {
    // Overall par shape follows data shape
    nChanParList() = nDatChan;
    startChanList() = startDatChan;

    // However, for solving, we will only consider one channel at a time:
    nChanMatList() = 1;

  }
  else {
    // Pars are not themselves channel-dependent
    nChanParList() = 1;

    // Check if matrices may still be freq-dep:
    if (freqDepMat()) {
      // cal is an explicit f(freq) (e.g., like delay)
      nChanMatList()  = nDatChan;
      startChanList() = startDatChan;
    } else {
      // cal has no freq dep at all
      nChanMatList()  = Vector<Int>(nSpw(),1);
      startChanList() = Vector<Int>(nSpw(),0);
    }

  }

  // At this point:
  //  1. nChanParList() represents the (per-Spw) overall length of the
  //     output parameter channel axis, appropriate for shaping the
  //     CalSet.  This value is irrelevant during the solve, since
  //     we will only solve for one parameter channel at a time (or 
  //     there is only one channel to solver for).
  //  2. nChanMatList() represents the per-Spw  matrix channel axis length to
  //     be used during the solve, independent of the parameter channel
  //     axis length.  In the solve context, nChanMat()>1 when there is
  //     more than one channel of data upon which the (single channel)
  //     solve parameters depend (e.g., delay, polynomial bandpass, etc.)

}


// Fill the Calset with meta data
void SolvableVisCal::fillMetaData(VisSet& vs) {

  if (prtlev()>3) cout << "  SVC::fillMetaData(vs)" << endl;

  // NB: Currently, this is only used for the accumulate
  //     context; in solve, meta-data is filled on-the-fly
  //     (this ensures more accurate timestamps, etc.)

  // NB: Assumes CalSet already has correct shape!

  // TODO:
  // 1. Can we use setRowBlocking() here to avoid in-chunk iterations?

  // Fill the Calset with meta info
  VisIter& vi(vs.iter());
  vi.originChunks();
  VisBuffer vb(vi);
  Vector<Int> islot(nSpw(),0);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int thisSpw=vi.spectralWindow();

    if (islot(thisSpw)>=cs().nTime(thisSpw))
      throw(AipsError("Error filling CalSet with Meta data."));
	      
    // Set field source info
    cs().fieldId(thisSpw)(islot(thisSpw))  =vi.fieldId();
    cs().fieldName(thisSpw)(islot(thisSpw)) =vi.fieldName();
    cs().sourceName(thisSpw)(islot(thisSpw))=vi.sourceName();

    // Set time info
    vi.origin();
    cs().startTime(thisSpw)(islot(thisSpw))  =vb.time()(0)-vb.timeInterval()(0)/2.0;
    Double timeStamp(0.0);
    Int ntime(0);
    for (vi.origin(); vi.more(); vi++,ntime++) {
      timeStamp+=vb.time()(0);
    }
    cs().stopTime(thisSpw)(islot(thisSpw)) =vb.time()(0)+vb.timeInterval()(0)/2.0;
    cs().time(thisSpw)(islot(thisSpw))=timeStamp/ntime;

    // Advance slot counter (per spw)
    islot(thisSpw)++;

  }

}


Bool SolvableVisCal::syncSolveMeta(VisBuffer& vb, 
				   const Int& fieldId) {

  if (prtlev()>2) cout << "SVC::syncSolveMeta(,,)" << endl;

  // Returns True, only if sum of weights is positive,
  //  i.e., there is data to solve with

  // TBD: Get fieldId from vb (need to make sure fieldId survives averaging)
  // TBD: freq info, etc.

  currSpw()=vb.spectralWindow();
  currField()=fieldId;

  // Row weights as a Doubles
  Vector<Double> dWts;
  dWts.resize(vb.weight().shape());
  convertArray(dWts,vb.weight());
  Vector<Double> times;
  times = vb.time();
  
  // The following assumes flagRow is accurate
  LogicalArray gRows(!vb.flagRow());
  Double sumWts(0.0);
  MaskedArray<Double> gTimes(times,gRows);
  MaskedArray<Double> gWts(dWts,gRows);

  if (sum(gRows)>0) {
    sumWts=sum(gWts);
  }

  if (sumWts>0.0) {
    gTimes*=gWts;
    refTime()=sum(gTimes);
    refTime()/=sumWts;
    return True;
  }
  else
    return False;

}

void SolvableVisCal::enforceAPonData(VisBuffer& vb) {

  // TBD: migrate this to VisEquation?

  if (apmode()!="AP") {
    Int nCorr(vb.corrType().nelements());
    Float amp(1.0);
    Complex cor(1.0);
    Bool *flR=vb.flagRow().data();
    Bool *fl =vb.flag().data();
    Vector<Float> ampCorr(nCorr);
    Vector<Int> n(nCorr,0);
    for (Int irow=0;irow<vb.nRow();++irow,++flR) {
      if (!vb.flagRow()(irow)) {
	ampCorr=0.0f;
	n=0;
	for (Int ich=0;ich<vb.nChannel();++ich,++fl) {
	  if (!vb.flag()(ich,irow)) {
	    for (Int icorr=0;icorr<nCorr;icorr++) {
	      
	      amp=abs(vb.visCube()(icorr,ich,irow));
	      if (amp>0.0f) {
		
		if (apmode()=="P")
		  // we will scale by amp to make data phase-only
		  cor=Complex(amp,0.0);
		else if (apmode()=="A")
		  // we will scale by "phase" to make data amp-only
		  cor=vb.visCube()(icorr,ich,irow)/amp;
		
		// Apply the complex scaling and count
		vb.visCube()(icorr,ich,irow)/=cor;
		ampCorr(icorr)+=amp;
		n(icorr)++;
	      }
	    } // icorr
	  } // !*fl
	} // ich
	// Make appropriate weight adjustment
	for (Int icorr=0;icorr<nCorr;icorr++)
	  if (n(icorr)>0)
	    // weights adjusted by square of the mean(amp)
	    vb.weightMat()(icorr,irow)*=square(ampCorr(icorr)/Float(n(icorr)));
	  else
	    // weights now zero
	    vb.weightMat()(icorr,irow)=0.0f;
      } // !*flR
    } // irow

  } // phase- or amp-only

  //  cout << "amp(vb.visCube())=" << amplitude(vb.visCube().reform(IPosition(1,vb.visCube().nelements()))) << endl;


}

void SolvableVisCal::setUpForPolSolve(VisBuffer& vb) {

  // TBD: migrate this to VisEquation?
  
  // Divide model and data by (scalar) stokes I (which may be resolved!), 
  //  and set model cross-hands to (1,0) so we can solve for fraction 
  //  pol factors.

  Int nCorr(vb.corrType().nelements());
  Bool *flR=vb.flagRow().data();
  Bool *fl =vb.flag().data();
  Vector<Float> ampCorr(nCorr);
  Vector<Int> n(nCorr,0);
  Complex sI(0.0);
  for (Int irow=0;irow<vb.nRow();++irow,++flR) {
    if (!vb.flagRow()(irow)) {
      ampCorr=0.0f;
      n=0;
      for (Int ich=0;ich<vb.nChannel();++ich,++fl) {
	if (!vb.flag()(ich,irow)) {
	  
	  sI=(vb.modelVisCube()(0,ich,irow)+vb.modelVisCube()(3,ich,irow))/Complex(2.0);
	  if (abs(sI)>0.0) {
	    for (Int icorr=0;icorr<nCorr;icorr++) {
	      vb.visCube()(icorr,ich,irow)/=sI;
	      ampCorr(icorr)+=abs(sI);
	      n(icorr)++;
	    } // icorr
	  }
	  else
	    vb.flag()(ich,irow)=True;
	  
	} // !*fl
      } // ich
      // Make appropriate weight adjustment
      for (Int icorr=0;icorr<nCorr;icorr++)
	if (n(icorr)>0)
	  // weights adjusted by square of the mean(amp)
	  vb.weightMat()(icorr,irow)*=square(ampCorr(icorr)/Float(n(icorr)));
	else
	  // weights now zero
	  vb.weightMat()(icorr,irow)=0.0f;
    } // !*flR
  } // irow

  // Model is now all unity  (Is this ok for flagged data? Probably.)
  vb.modelVisCube()=Complex(1.0);


}

// Verify VisBuffer data sufficient for solving (wts, etc.)
Bool SolvableVisCal::verifyForSolve(VisBuffer& vb) {

  //  cout << "verifyForSolve..." << endl;

  Int nAntForSolveFinal(-1);
  Int nAntForSolve(0);

  // We will count baselines and weights per ant
  //   and set solveParOK accordingly
  Vector<Int> blperant(nAnt(),0);
  Vector<Double> wtperant(nAnt(),0.0);
  Vector<Bool> antOK(nAnt(),False);
    
  while (nAntForSolve!=nAntForSolveFinal) {

    nAntForSolveFinal=nAntForSolve;
    nAntForSolve=0;


    // TBD: optimize indexing with pointers in the following
    blperant=0;
    wtperant=0.0;

    for (Int irow=0;irow<vb.nRow();++irow) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);
      if (!vb.flagRow()(irow) && a1!=a2) {
	
	if (!vb.flag()(focusChan(),irow)) {
	  
	  blperant(a1)+=1;
	  blperant(a2)+=1;
	  
	  wtperant(a1)+=Double(sum(vb.weightMat().column(irow)));
	  wtperant(a2)+=Double(sum(vb.weightMat().column(irow)));
	  
	}
      }
    }
    
    antOK=False;
    for (Int iant=0;iant<nAnt();++iant) {
      if (blperant(iant)>3 &&
	  wtperant(iant)>0.0) {
	// This antenna is good, keep it
	nAntForSolve+=1;
	antOK(iant)=True;
      }
      else {
	// This antenna under-represented; flag it
	vb.flag().row(focusChan())(vb.antenna1()==iant)=True;
	vb.flag().row(focusChan())(vb.antenna2()==iant)=True;
	//	vb.flagRow()(vb.antenna1()==iant)=True;
	//	vb.flagRow()(vb.antenna2()==iant)=True;
      }
    }

    //    cout << "blperant     = " << blperant << endl;
    //    cout << "wtperant = " << wtperant << endl;
    //    cout << "nAntForSolve = " << nAntForSolve << " " << antOK << endl;

  }

  // We've converged on the correct good antenna count
  nAntForSolveFinal=nAntForSolve;

  // Set a priori solution flags  
  solveParOK() = False;
  for (Int iant=0;iant<nAnt();++iant)
    if (antOK(iant))
      // This ant ok
      solveParOK().xyPlane(iant) = True;
    else
      // This ant not ok, set soln to zero
      if (parType()==VisCal::Co)
      	solveCPar().xyPlane(iant)=1.0;
      else if (parType()==VisCal::Re)
      	solveRPar().xyPlane(iant)=0.0;
  //  cout << "antOK = " << antOK << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;
  //  cout << "amp(solveCPar()) = " << amplitude(solveCPar()) << endl;

  if (nAntForSolve<4) cout << "Only " << nAntForSolve 
			   << "/" << nAnt() 
			   << " antennas (" 
			   << floor(100*Float(nAntForSolve/nAnt()))
			   << "%) have sufficient baselines at " 
			   << MVTime(refTime()/C::day).string(MVTime::YMD,7)
			   << endl;
  return (nAntForSolve>3);
    
}

void SolvableVisCal::selfSolve(VisSet& vs, VisEquation& ve) {
    
  if (standardSolve())
    throw(AipsError("Spurious call to selfSolve()."));
  else
    throw(AipsError("Attempt to call un-implemented selfSolve()"));

}

     
void SolvableVisCal::updatePar(const Vector<Complex> dCalPar,const Vector<Complex> dSrcPar) {

  AlwaysAssert((solveCPar().nelements()==dCalPar.nelements()),AipsError);

  Cube<Complex> dparcube(dCalPar.reform(solveCPar().shape()));

  // zero flagged increments
  dparcube(!solveParOK())=Complex(0.0);
  
  // Add the increment
  solveCPar()+=dparcube;

  // Update source params
  if (solvePol()) {
    srcPolPar()+=dSrcPar;
    //    cout << "Updated Q,U = " << real(srcPolPar()) << endl;
  }

  // The matrices are nominally out-of-sync now
  invalidateCalMat();

  // Ensure phaseonly-ness, if necessary
  //  if (apmode()=='P') {   
  //  NB: Disable this, for the moment (07May24); testing a fix for
  //      a problem Kumar noticed.  See VC::makeSolnPhaseOnly(), etc.
  if (False) {
    Float amp(0.0);
    for (Int iant=0;iant<nAnt();++iant) {
      for (Int ipar=0;ipar<nPar();++ipar) {
	if (solveParOK()(ipar,0,iant)) {
	  amp=abs(solveCPar()(ipar,0,iant));
	  if (amp>0.0)
	    solveCPar()(ipar,0,iant)/=amp;
	}
      }
    }
  }
}

void SolvableVisCal::formSolveSNR() {

  solveParSNR()=0.0;

  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar) {
      if (solveParOK()(ipar,0,iant) &&
	  solveParErr()(ipar,0,iant)>0.0f) {
	solveParSNR()(ipar,0,iant)=abs(solveCPar()(ipar,0,iant))/solveParErr()(ipar,0,iant);
      }
      else
	// Ensure F if Err<=0  (OK?)
	solveParOK()(ipar,0,iant)=False;
    }
}

void SolvableVisCal::applySNRThreshold() {

  Int nOk1(ntrue(solveParOK()));
  
  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar)
      if (solveParOK()(ipar,0,iant))
	solveParOK()(ipar,0,iant)=(solveParSNR()(ipar,0,iant)>minSNR());
  
  Int nOk2(ntrue(solveParOK()));
  Int nFail=nOk1-nOk2;    
  
  if (False) {
    // Report some stuff re SNR
    cout << endl 
	 << "Time = " << MVTime(refTime()/C::day).string(MVTime::YMD,7) << endl;
    cout << "SNR      = " << solveParSNR() << endl;
    cout << nOk1 << " " << nOk2 << " " << nFail << endl;
    Float meansnr(0.0f);
    if (ntrue(solveParOK())>0) {
      meansnr=mean(solveParSNR()(solveParOK()));
      cout << "mean solution SNR = " << meansnr
	   << " (passing threshold)."
	   << endl;
    }
  }
  
  if (nFail>0)
    cout << nFail << " of " << nOk1
	 << " solutions rejected due to SNR < " << minSNR() 
	 << " in spw=" << currSpw()
	 << " at " << MVTime(refTime()/C::day).string(MVTime::YMD,7)
	 << endl;
  
}

void SolvableVisCal::smooth(Vector<Int>& fields,
			    const String& smtype,
			    const Double& smtime) {

  if (smoothable()) 
    // Call CalSet's global smooth method
    casa::smooth(cs(),smtype,smtime,fields);
  else
    throw(AipsError("This type does not support smoothing!"));

}


void SolvableVisCal::syncPar(const Int& spw, const Int& slot) {

  if (prtlev()>4) cout << "    SVC::syncPar(spw,slot)" << endl;

  IPosition blc(4,0,0,0,slot);
  IPosition trc(4,cs().nPar()-1,cs().nChan(spw)-1,cs().nElem()-1,slot);

  currCPar().reference(cs().par(spw)(blc,trc).nonDegenerate(3));;
  currParOK().reference(cs().parOK(spw)(blc,trc).nonDegenerate(3));

  validateP();
  invalidateCalMat();

}

void SolvableVisCal::syncSolveCal() {

  if (prtlev()>4) cout << "    SVC::syncSolveCal()" << endl;
  
  // Ensure parameters are ready
  syncSolvePar();

  if (!PValid())
    throw(AipsError("No valid parameters in syncSolveCal"));

  // Sync up matrices using current params
  syncCalMat(False);    // NEVER invert in solve context
  syncDiffMat();

}
void SolvableVisCal::syncSolvePar() {

  if (prtlev()>5) cout << "      SVC::syncSolvePar()" << endl;

  // In solve context, reference solveCPar(), etc.
  AlwaysAssert((solveCPar().nelements()>0 || solveRPar().nelements()>0),AipsError);
  currCPar().reference(solveCPar());
  currRPar().reference(solveRPar());
  currParOK().reference(solveParOK());
  validateP();

}

void SolvableVisCal::calcPar() {

  if (prtlev()>6) cout << "      SVC::calcPar()" << endl;

  // This method is relevant only to the apply context

  Bool newcal(False);

  // Interpolate solution
  newcal=ci().interpolate(currTime(),currSpw());

  // TBD: signal failure to find calibration??  (e.g., for a spw?)

  // Parameters now valid (independent of newcal!!)
  //  (TBD: should have ci() to tell us this?)
  validateP();

  // If new cal parameters, use them 
  if (newcal) {

    //    cout << "Found new cal!" << endl;
    
    // Reference result
    currCPar().reference(ci().result());
    currParOK().reference(ci().resultOK());

    // For now, assume all OK
    //    currParOK().resize(nPar(),nChanPar(),nElem()); currParOK()=True;

    // In case we need solution timestamp
    refTime() = ci().slotTime();

    // If new parameters, matrices (generically) are necessarily invalid now
    invalidateCalMat();
  }

}

// Report solved-for QU
void SolvableVisCal::reportSolvedQU() {

  if (solvePol()) {
    logSink() << "Source polarization solution for field " << currField();
    if (freqDepPar())
      logSink() << " (chan = " << focusChan() << ")";
    
    logSink() << ": Q = " << real(srcPolPar()(0)) 
	      << ",  U = " << real(srcPolPar()(1))
	      << LogIO::POST;
  }
}

// File a solved solution (and meta-data) into a slot in the CalSet
void SolvableVisCal::keep(const Int& slot) {

  if (prtlev()>4) cout << " SVC::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       focusChan(),0,        slot);
    IPosition trc4(4,nPar()-1,focusChan(),nElem()-1,slot);
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();

    // TBD:  Handle solveRPar here!

    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();
    cs().parErr(currSpw())(blc4,trc4).nonDegenerate(3)= solveParErr();
    cs().parSNR(currSpw())(blc4,trc4).nonDegenerate(3)= solveParSNR();
    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

  }
  else
    throw(AipsError("SVJ::keep: Attempt to store solution in non-existent CalSet slot"));

}

void SolvableVisCal::globalPostSolveTinker() {

  // Make solutions phase- or amp-only, if required
  if (apmode()!="AP") enforceAPonSoln();
  
  // Apply normalization
  if (solnorm()) normalize();

}

// Divide all solutions by their amplitudes to make them "phase-only"
void SolvableVisCal::enforceAPonSoln() {

  if (cs_) {    

    logSink() << "Enforcing apmode on solutions." 
	      << LogIO::POST;

    for (Int ispw=0;ispw<nSpw();++ispw) {
      if (cs().nTime(ispw)>0) {
	Array<Float> amps(amplitude(cs().par(ispw)));
	// No zeroes:
	cs().par(ispw)(amps==0.0f)=Complex(1.0);
	cs().par(ispw)(operator!(LogicalArray(cs().parOK(ispw))))=Complex(1.0);
	amps(amps==0.0f)=1.0f;
	amps(!cs().parOK(ispw))=1.0f;
	//	amps(operator!(LogicalArray(cs().parOK(ispw))))=1.0f;

	Array<Complex> cor(amps.shape());
	if (apmode()=='P')
	  // we will scale solns by amp to make them phase-only
	  convertArray(cor,amps);
	else if (apmode()=='A') {
	  // we will scale solns by "phase" to make them amp-only
	  cor=cs().par(ispw);
	  cor/=amps;
	}

	if (ntrue(amplitude(cor)==0.0f)==0)
	  cs().par(ispw)/=cor;
	else
	  throw(AipsError("enforceAPonSoln divide-by-zero error."));
      }
    }
  }
  else 
    throw(AipsError("Solution apmode enforcement not supported."));
}

void SolvableVisCal::normalize() {

  // Only if we have a CalSet...
  if (cs_) {

    logSink() << "Normalizing solution amplitudes per spw." 
	      << LogIO::POST;

    // Normalize each non-trivial spw in the CalSet
    for (Int ispw=0;ispw<nSpw();++ispw)
      if (cs().nTime(ispw)>0) 
	normSolnArray(cs().par(ispw),cs().parOK(ispw),False);

  }
  else 
    throw(AipsError("Solution normalization not supported."));
	
}

void SolvableVisCal::store() {

  if (prtlev()>3) cout << " SVC::store()" << endl;

  if (append())
    logSink() << "Appending solutions to table: " << calTableName()
	      << LogIO::POST;
  else
    logSink() << "Writing solutions to table: " << calTableName()
	      << LogIO::POST;

  cs().store(calTableName(),typeName(),msName(),append());

}

void SolvableVisCal::store(const String& table,const Bool& append) {

  if (prtlev()>3) cout << " SVC::store(table,append)" << endl;

  // Override tablename
  calTableName()=table;
  SolvableVisCal::append()=append;

  // Call conventional store
  store();

}

void SolvableVisCal::stateSVC(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisCal::state();

  if (prtlev()>3) cout << "SVC::stateSVC():" << endl;
  cout << boolalpha;

  // Now SVC-specific stuff:
  cout << "  isSolved() = " << isSolved() << endl;
  cout << "  apmode() = " << apmode() << endl;
  cout << "  calTableName() = " << calTableName() << endl;
  cout << "  calTableSelect() = " << calTableSelect() << endl;
  cout << "  tInterpType() = " << tInterpType() << endl;
  cout << "  fInterpType() = " << fInterpType() << endl;
  cout << "  spwMap() = " << spwMap() << endl;
  cout << "  refant() = " << refant() << endl;
  
  cout << "  solveCPar().shape()   = " << solveCPar().shape() 
       << " (" << solveCPar().data() << ")" << endl;
  cout << "  solveRPar().shape()   = " << solveRPar().shape() 
       << " (" << solveRPar().data() << ")" << endl;
  cout << "  solveParOK().shape() = " << solveParOK().shape()
       << " (" << solveParOK().data() << ") "
       << " (ntrue=" << ntrue(solveParOK()) << ")" << endl;

  cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;

}

void SolvableVisCal::normSolnArray(Array<Complex>& sol, 
				   const Array<Bool>& solOK,
				   const Bool doPhase) {

  // Only do something if 2 or more good solutions
  if (ntrue(solOK)>1) {

    Complex factor(1.0);
    
    Array<Float> amp(amplitude(sol));

    // If desired, determine phase part of the normalization
    if (doPhase) {
      // Prepare to divide by amplitudes indiscriminately
      amp(!solOK)=1.0f;
      Array<Complex> sol1=sol/amp;
      sol1(!solOK)=Complex(0.0);
      factor=sum(sol1);
      factor/=abs(factor);
    }

    // Determine amplitude normalization
    amp(!solOK)=0.0f;
    factor*=Complex(sum(amp)/Float(ntrue(solOK)));
    
    // Apply the normalization factor, if non-zero
    if (abs(factor) > 0.0)
      sol/=factor;
    
  } // ntrue > 0

}



void SolvableVisCal::currMetaNote() {

  cout << "   ("
       << "time=" << MVTime(refTime()/C::day).string(MVTime::YMD,7)
       << " field=" << currField()
       << " spw=" << currSpw()
       << " chan=" << focusChan()
       << ")"
       << endl;

}


void SolvableVisCal::initSVC() {

  if (prtlev()>2) cout << " SVC::initSVC()" << endl;

  for (Int ispw=0;ispw<nSpw(); ispw++) {
    solveCPar_[ispw] = new Cube<Complex>();
    solveRPar_[ispw] = new Cube<Float>();
    solveParOK_[ispw] = new Cube<Bool>();
    solveParErr_[ispw] = new Cube<Float>();
    solveParSNR_[ispw] = new Cube<Float>();
  }

}

void SolvableVisCal::deleteSVC() {

  if (prtlev()>2) cout << " SVC::deleteSVC()" << endl;

  for (Int ispw=0; ispw<nSpw(); ispw++) {
    if (solveCPar_[ispw])  delete solveCPar_[ispw];
    if (solveRPar_[ispw])  delete solveRPar_[ispw];
    if (solveParOK_[ispw]) delete solveParOK_[ispw];
    if (solveParErr_[ispw]) delete solveParErr_[ispw];
    if (solveParSNR_[ispw]) delete solveParSNR_[ispw];
  }
  solveCPar_=NULL;
  solveRPar_=NULL;
  solveParOK_=NULL;
  solveParErr_=NULL;
  solveParSNR_=NULL;
}

void SolvableVisCal::verifyCalTable(const String& caltablename) {

  // Call external method to get type (will throw if bad table)
  String calType=calTableType(caltablename);

  // Check if proper Calibration type...
  if (calType!=typeName()) {
    ostringstream o;
    o << "Table " << caltablename 
      << " has wrong Calibration type: " << calType;
    throw(AipsError(String(o)));
  }
}




// **********************************************************
//  SolvableVisMueller Implementations
//


SolvableVisMueller::SolvableVisMueller(VisSet& vs) :
  VisCal(vs),
  VisMueller(vs),
  SolvableVisCal(vs),
  dM_(NULL),
  diffMElem_(),
  DMValid_(False)
{
  if (prtlev()>2) cout << "SVM::SVM(vs)" << endl;
}

SolvableVisMueller::SolvableVisMueller(const Int& nAnt) :
  VisCal(nAnt),
  VisMueller(nAnt),
  SolvableVisCal(nAnt),
  dM_(NULL),
  diffMElem_(),
  DMValid_(False)
{
  if (prtlev()>2) cout << "SVM::SVM(i,j,k)" << endl;
}

SolvableVisMueller::~SolvableVisMueller() {

  if (prtlev()>2) cout << "SVM::~SVM()" << endl;

}

// Setup solvePar shape (Mueller version)
void SolvableVisMueller::initSolvePar() {

  if (prtlev()>3) cout << " SVM::initSolvePar()" << endl;
  
  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;

    // TBD: Consider per-baseline solving (3rd=1)
    solveCPar().resize(nPar(),nChanPar(),nBln());
    solveParOK().resize(nPar(),nChanPar(),nBln());
    solveParErr().resize(nPar(),nChanPar(),nBln());
    solveParSNR().resize(nPar(),nChanPar(),nBln());

    // TBD: solveRPar()?
    
    solveCPar()=Complex(1.0);
    solveParOK()=True;
    solveParErr()=0.0;
    solveParSNR()=0.0;
  }
  currSpw()=0;

}

void SolvableVisMueller::syncDiffMat() {

  if (prtlev()>5) cout << "     SVM::syncDiffMat()" 
		       << " (DMValid()=" << DMValid() << ")" << endl;

  // Sync the diff'd Mueller matrices
  if (!DMValid()) syncDiffMueller();

}

void SolvableVisMueller::syncDiffMueller() {

  if (prtlev()>6) cout << "      SVM::syncDiffMueller()" << endl;

  // TBD:  validateDM() for trivialMuellerElem()=True??
  //    (cf. where does invalidateDM() occur?)

  if (trivialDM())
    // Ensure trivial matrices ready
    initTrivDM();
  else {
    diffMElem().resize(IPosition(4,muellerType(),nPar(),nChanMat(),nCalMat()));
    diffMElem().unique();
    invalidateDM();

    // Calculate for all blns/chans
    calcAllDiffMueller();

  }

  // Ensure diff'd Mueller matrix renders are OK
  createDiffMueller();

  // diff'd Mueller matrices now valid
  validateDM();

}

void SolvableVisMueller::calcAllDiffMueller() {

  if (prtlev()>6) cout << "       SVM::calcAllDiffMueller" << endl;

  // Should handle OK flags in this method, and only
  //  do  calc if OK

  // Sanity check on parameter channel axis
  AlwaysAssert((solveCPar().shape()(1)==1),AipsError);

  // Referencing arrays, per baseline
  Matrix<Complex> oneDM;       //  (nElem,nPar)
  Vector<Complex> onePar;      //  (nPar)

  ArrayIterator<Complex> dMiter(diffMElem(),2);
  ArrayIterator<Complex> Piter(solveCPar(),1);
  
  for (Int ibln=0; ibln<nCalMat(); ++ibln) {

    // Solving parameters are NEVER channel-dependent
    //  (even if data & matrices are)
    onePar.reference(Piter.array());
      
    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneDM.reference(dMiter.array());

      // Calculate the DM matrices for each par on this bln/chan
      calcOneDiffMueller(oneDM,onePar);

      // Advance iterators
      dMiter.next();

    }
    Piter.next();
  }

}

void SolvableVisMueller::calcOneDiffMueller(Matrix<Complex>& mat, 
					    const Vector<Complex>& par) {

  if (prtlev()>10) cout << "        SVM::calcOneDiffMueller()" << endl;

  // If Mueller matrix is trivial, shouldn't get here
  if (trivialMuellerElem()) 
    throw(AipsError("Trivial Mueller Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown non-trivial dMueller-from-parameter calculation requested."));

}

void SolvableVisMueller::createDiffMueller() {

  if (prtlev()>6) cout << "       SVM::createDiffMueller()" << endl;

  Mueller::MuellerType mtype(muellerType());

  // Delete if wrong type
  if (dM_ && dM().type() != mtype) delete dM_;
  
  // If needed, construct the correct diff Mueller
  if (!dM_) dM_ = casa::createMueller(mtype);
      
}

void SolvableVisMueller::initTrivDM() {

  if (prtlev()>7) cout << "        SVM::initTrivDM()" << endl;

  // If DM matrice not trivial, shouldn't get here
  if (!trivialDM()) 
    throw(AipsError("Trivial Mueller Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown trivial dM initialization requested."));

}

void SolvableVisMueller::stateSVM(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisMueller::state();
  
  // Get parent's state (w/out VC):
  SolvableVisCal::stateSVC(False);

  if (applyByMueller()) {
    if (prtlev()>3) cout << "SVM::stateSVM()" << endl;
    cout << boolalpha;

  // Now SVM-specific stuff:
    cout << "DMValid() = " << DMValid() << endl;
    cout << "diffMElem().shape() = " << diffMElem().shape() 
	 << " (" << diffMElem().data() << ")" << endl;
    
    cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
  }
}


// **********************************************************
//  SolvableVisJones Implementations
//


SolvableVisJones::SolvableVisJones(VisSet& vs) :
  VisCal(vs),                           // virtual base
  VisMueller(vs),                       // virtual base
  SolvableVisMueller(vs),               // immediate parent
  VisJones(vs),                         // immediate parent
  dJ1_(NULL),                           // data...
  dJ2_(NULL),
  diffJElem_(),
  DJValid_(False) 
{
  if (prtlev()>2) cout << "SVJ::SVJ(vs)" << endl;
}

SolvableVisJones::SolvableVisJones(const Int& nAnt) : 
  VisCal(nAnt),               // virtual base
  VisMueller(nAnt),           // virtual base
  SolvableVisMueller(nAnt),   // immediate parent
  VisJones(nAnt),             // immediate parent
  dJ1_(NULL),                 // data...
  dJ2_(NULL),
  diffJElem_(),
  DJValid_(False) 
{
  if (prtlev()>2) cout << "SVJ::SVJ(i,j,k)" << endl;
}

SolvableVisJones::~SolvableVisJones() {

  if (prtlev()>2) cout << "SVJ::~SVJ()" << endl;

}

void SolvableVisJones::reReference() {

  // TBD: Handle single-poln referencing
  // TBD: Handle missing refant

  // Generic version for trivial types
  if (trivialJonesElem()) {

    // Determine multiplicative complex phase
    Matrix<Complex> refgain;
    refgain=solveCPar().xyPlane(refant());

    //    cout << "refgain add:  " << refgain.data() << " " << solveCPar().xyPlane(refant()).data() << endl;

    Float amp(1.0);
    Complex* rg=refgain.data();
    // TBD: clean this up: there is only 1 solvePar chan!
    //    for (Int ich=0;ich<nChanPar();++ich) {
      for (Int ip=0;ip<nPar();++ip,++rg) {
	amp=abs(*rg);
	if (amp>0.0) {
	  *rg/=amp;
	  *rg=conj(*rg);
	}
	else
	  *rg=Complex(1.0);
      }
      //    }

      //      cout << "amp(refgain) = " << amplitude(refgain) << endl;


    // Apply complex phase to each ant
    Matrix<Complex> antgain;
    for (Int ia=0;ia<nAnt();++ia) {
      antgain.reference(solveCPar().xyPlane(ia));
      antgain*=refgain;
    }
  }
  else 
    throw(AipsError("Attempt to reference non-trivial calibration type."));


}

void SolvableVisJones::differentiate(VisBuffer& vb,
				     Cube<Complex>& Vout, 
				     Array<Complex>& dVout,
				     Matrix<Bool>& Vflg) {
    
  if (prtlev()>3) cout << "  SVJ::differentiate()" << endl;

  // NB: For freqDepPar()=True, the data and solutions are
  //     multi-channel, but nChanMat()=1 because we only 
  //     consider one channel at a time.  In this case,
  //     focusChan is the specific channel under consideration.
  //     Otherwise, we will use all channels in the vb 
  //     simultaneously

  // Some vb shape info
  Int& nRow(vb.nRow());
  Int nCorr(vb.corrType().nelements());

  // Size up the output data arrays
  // Vout = [nCorr,nChan,nRow]
  // dVout = [nCorr,nPar,nChan,nRow,2]   (1 set of dV for both ants on baseline)
  Vout.resize(IPosition(3,nCorr,nChanMat(),nRow));
  Vout.unique();             // ensure unique storage

  dVout.resize(IPosition(5,nCorr,nPar(),nChanMat(),nRow,2));
  dVout.unique();
  dVout=Complex(0.0);
  
  // Copy the input model data from the VisBuffer to Vout
  //  for in-place application (do this according to focusChan)
  //  (also flags)
  Matrix<Bool> fl;
  if (freqDepPar()) {
    // Copy just the focusChan; all work below is single-channel
    AlwaysAssert((nChanMat()==1),AipsError);  // sanity
    AlwaysAssert((focusChan()>-1),AipsError);       // sanity

    Vout = vb.modelVisCube()(IPosition(3,0,      focusChan(),0     ),
			     IPosition(3,nCorr-1,focusChan(),nRow-1));

    Vflg.resize(IPosition(2,1,nRow));   // proper single channel size
    Vflg.unique();                      // unique storage
    Vflg = vb.flag()(IPosition(2,focusChan(),0     ),
		     IPosition(2,focusChan(),nRow-1));
  }
  else {
    // Copy all channels in the vb
    Vout = vb.modelVisCube();
    Vflg.reference(vb.flag());   // Just reference whole flag array
  }

  // "Apply" the current Q,U estimates to the crosshand model
  if (solvePol()) {
    const Complex pol=Complex(real(srcPolPar()(0)),real(srcPolPar()(1)));
    IPosition blc(3,1,0,0), trc(3,1,nChanMat()-1,nRow-1);
    Array<Complex> RL(Vout(blc,trc));
    RL*=pol;
    blc(0)=trc(0)=2;
    Array<Complex> LR(Vout(blc,trc));
    LR*=conj(pol);
  }

  // Visibility vector renderers
  VisVector::VisType vt(visType(nCorr));
  VisVector cVm(vt);  // The model data corrupted by trial solution
  VisVector dV1(vt);  // The deriv of V wrt pars of 1st ant in bln 
  VisVector dV2(vt);  // The deriv of V wrt pars of 2nd ant in bln 

  // Temporary non-iterating VisVectors to hold partial applies
  VisVector J1V(vt,True);
  VisVector VJ2(vt,True);

  // Starting synchronization for output visibility data
  cVm.sync(Vout(0,0,0));
  dV1.sync(dVout(IPosition(5,0,0,0,0,0)));
  dV2.sync(dVout(IPosition(5,0,0,0,0,1)));

  // Synchronize current calibration pars/matrices
  syncSolveCal();

  // Nominal synchronization of dJs
  dJ1().sync(diffJElem()(IPosition(4,0,0,0,0)));
  dJ2().sync(diffJElem()(IPosition(4,0,0,0,0)));

  // VisBuffer indices
  Double* time=  vb.time().data();
  Int*    a1=    vb.antenna1().data();
  Int*    a2=    vb.antenna2().data();
  Bool*   flagR= vb.flagRow().data();
  Bool*   flag=  Vflg.data();            // via local reference
  
  // TBD: set weights according to flags??

  // iterate rows
  for (Int irow=0; irow<nRow; irow++,flagR++,a1++,a2++,time++) {
    
    // Avoid ACs
    if (*a1==*a2) *flagR=True;

    if (!*flagR) {  // if this row unflagged
	
      // Re-update matrices if time changes
      if (timeDepMat() && *time != lastTime()) {
	currTime()=*time;
	invalidateDiffCalMat();
	syncCalMat();
	syncDiffMat();
	lastTime()=currTime();
      }

      // Synchronize Jones renderers for the ants on this baseline
      J1().sync(currJElem()(0,0,*a1),currJElemOK()(0,0,*a1));
      J2().sync(currJElem()(0,0,*a2),currJElemOK()(0,0,*a2));

      // Synchronize differentiated Jones renderers for this baseline
      if (trivialDJ()) {
	dJ1().origin();
	dJ2().origin();
      } else {
	dJ1().sync(diffJElem()(IPosition(4,0,0,0,*a1)));
	dJ2().sync(diffJElem()(IPosition(4,0,0,0,*a2)));
      }

      // Assumes all iterating quantities have nChanMat() channelization
      for (Int ich=0; ich<nChanMat();ich++,flag++,
	     cVm++, J1()++, J2()++) {
	     
	// if channel unflagged an cal ok
	//	if (!*flag && (*J1Ok && *J2Ok) ) {  
	if (!*flag) { 
	  
	  // Partial applies for repeated use below
	  VJ2=cVm;                    
	  J2().applyLeft(VJ2,*flag);      // VJ2 = Vm*J2, used below

	  J1().applyRight(cVm,*flag);     
	  J1V=cVm;                        // J1V = J1*Vm, used below

	  // Finish trial corruption
	  J2().applyLeft(cVm,*flag);      // cVm = (J1*Vm)*J2

	}

	// Only continue with diff-ing, if we aren't flagged yet
	if (!*flag) {

	  // Differentiation per par
	  for (Int ip=0;ip<nPar();ip++,
		 dV1++,dJ1()++,
		 dV2++,dJ2()++) {
	    
	    dV1=VJ2;
	    dJ1().applyRight(dV1);  // dV1 = dJ1(ip)*(Vm*J2)
	    
	    dV2=J1V;
	    dJ2().applyLeft(dV2);   // dV2 = (J1*Vm)*dJ2(ip)
	  }
	  
	} // (!*flag)
	else {
	  // set trial corruption to zero
	  cVm.zero();
	  
	  // Advance all par-dep pointers over flagged channel
	  dV1.advance(nPar());
	  dV2.advance(nPar());
	  dJ1().advance(nPar());
	  dJ2().advance(nPar());
	}

      } // chn
		
    } // !*flagR
    else {
      // Must advance all chan-, par-dep pointers over flagged row
      flag+=nChanMat(); 
      cVm.advance(nChanMat());
      J1().advance(nChanMat());
      J2().advance(nChanMat());
      Int chpar(nChanMat()*nPar());
      dV1.advance(chpar);
      dV2.advance(chpar);
      dJ1().advance(chpar);
      dJ2().advance(chpar);
    }
  }

}

void SolvableVisJones::diffSrc(VisBuffer& vb,
			       Array<Complex>& dVout) {
    
  if (prtlev()>3) cout << "  SVJ::diffSrc()" << endl;

  // Some vb shape info
  Int& nRow(vb.nRow());
  Int nCorr(vb.corrType().nelements());

  // Size up the output data arrays
  dVout.resize(IPosition(4,nCorr,nChanMat(),nRow,2));
  dVout.unique();
  dVout=Complex(0.0);
  
  IPosition blc(4,0,0,0,0), trc(4,0,nChanMat()-1,nRow-1,0);
  blc(0)=1;
  trc(0)=2;
  blc(3)=trc(3)=0;
  dVout(blc,trc)=Complex(1.0);
  blc(3)=trc(3)=1;
  blc(0)=trc(0)=1;
  dVout(blc,trc)=Complex(0.0,1.0);
  blc(0)=trc(0)=2;
  dVout(blc,trc)=Complex(0.0,-1.0);

  // Visibility vector renderers
  VisVector::VisType vt(visType(nCorr));
  VisVector dSm1(vt);  // The model data corrupted by trial solution
  VisVector dSm2(vt);  // The model data corrupted by trial solution

  // Starting synchronization for output visibility data
  dSm1.sync(dVout(IPosition(4,0,0,0,0)));
  dSm2.sync(dVout(IPosition(4,0,0,0,1)));

  // Synchronize current calibration pars/matrices
  syncSolveCal();

  // Nominal synchronization of dJs
  dJ1().sync(diffJElem()(IPosition(4,0,0,0,0)));
  dJ2().sync(diffJElem()(IPosition(4,0,0,0,0)));

  // VisBuffer indices
  Double* time=  vb.time().data();
  Int*    a1=    vb.antenna1().data();
  Int*    a2=    vb.antenna2().data();
  Bool*   flagR= vb.flagRow().data();
  Bool*   flag=  vb.flag().data();
  
  // TBD: set weights according to flags??

  // iterate rows
  for (Int irow=0; irow<nRow; irow++,flagR++,a1++,a2++,time++) {
    
    // Avoid ACs
    if (*a1==*a2) *flagR=True;

    if (!*flagR) {  // if this row unflagged
	
      // Re-update matrices if time changes
      if (timeDepMat() && *time != lastTime()) {
	currTime()=*time;
	invalidateDiffCalMat();
	syncCalMat();
	syncDiffMat();
	lastTime()=currTime();
      }

      // Synchronize Jones renderers for the ants on this baseline
      J1().sync(currJElem()(0,0,*a1),currJElemOK()(0,0,*a1));
      J2().sync(currJElem()(0,0,*a2),currJElemOK()(0,0,*a2));

      // Synchronize differentiated Jones renderers for this baseline
      if (trivialDJ()) {
	dJ1().origin();
	dJ2().origin();
      } else {
	dJ1().sync(diffJElem()(IPosition(4,0,0,0,*a1)));
	dJ2().sync(diffJElem()(IPosition(4,0,0,0,*a2)));
      }

      // Assumes all iterating quantities have nChanMat() channelization
      for (Int ich=0; ich<nChanMat();ich++,flag++,
	     dSm1++, dSm2++, J1()++, J2()++) {
	     
	// if channel unflagged an cal ok
	if (!*flag) { 
	  
	  J1().applyRight(dSm1);
	  J2().applyLeft(dSm1);
	  J1().applyRight(dSm2);
	  J2().applyLeft(dSm2);

	}

      } // chn
		
    } // !*flagR
    else {
      // Must advance all chan-, par-dep pointers over flagged row
      flag+=nChanMat(); 
      dSm1.advance(nChanMat());
      dSm2.advance(nChanMat());
      J1().advance(nChanMat());
      J2().advance(nChanMat());
    }
  }

}

void SolvableVisJones::accumulate(SolvableVisCal* incr,
                                  const Vector<Int>& fields) {

  // Use SVJ interface for the incremental component
  //  (this should always be safe at this point?)
  SolvableVisJones* svj = dynamic_cast<SolvableVisJones*>(incr);

  // Catch bad SVJ conversion or fundamental type mismatch
  if (svj==NULL || svj->type() != this->type())
    throw(AipsError("Incremental calibration is not of compatible type."));

  Int nfield(fields.nelements());

  Bool fldok(True);

  for (Int ispw=0; ispw<nSpw(); ispw++) {

    // Only update spws which are available in the incr table:
    if (incr->spwOK()(ispw)) {

      currSpw()=ispw;
      
      Int nSlot(cs().nTime(ispw));
      
      // For each slot in this spw
      for (Int islot=0; islot<nSlot; islot++) {
	
	Double thistime=cs().time(ispw)(islot);
	Int thisfield=cs().fieldId(ispw)(islot);
	
	// TBD: proper frequency?  (from CalSet?)
	Vector<Double> thisfreq(nSpw(),0.0);
	
	// Is current field among those we need to update?
	fldok = (nfield==0 || anyEQ(fields,thisfield));

	if (fldok) {

	  // TBD: group following into syncCal(cs)?
	  syncMeta(currSpw(),thistime,thisfield,thisfreq,nChanPar());
	  syncPar(currSpw(),islot);
	  syncCalMat(False);
	  
	  // Sync svj with this
	  svj->syncCal(*this);
	  
	  // Relevant channels to update are bounded by
	  //  channels available in incr table:
	  Int prestep(svj->startChan());
	  Int poststep(nChanMat()-prestep-svj->nChanMat());
	  
	  AlwaysAssert((prestep<nChanMat()),AipsError);
	  AlwaysAssert((poststep>-1),AipsError);

	  // Do the multiplication each ant, chan
	  for (Int iant=0; iant<nAnt(); iant++) {

	    J1().advance(prestep);
	    for (Int ichan=0; ichan<svj->nChanMat(); ichan++) {
	      J1()*=(svj->J1());
	      J1()++;
	      svj->J1()++;
	    } // ichan
	    J1().advance(poststep);
	  } // iant
	  IPosition blc(4,0,0,0,islot);
	  IPosition trc(cs().parOK(currSpw()).shape());
	  trc-=1;
	  trc(3)=islot;
	  cs().solutionOK(currSpw())(islot)=
	    anyEQ(cs().parOK(currSpw())(blc,trc),True);
	} // fldok
      } // islot
    } // spwOK
  } // ispw
}



// Setup solvePar shape (Jones version)
void SolvableVisJones::initSolvePar() {

  if (prtlev()>3) cout << " SVJ::initSolvePar()" << endl;

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    switch (parType()) {
    case VisCal::Co: {
      solveCPar().resize(nPar(),1,nAnt());
      solveCPar()=Complex(1.0);
      break;
    }
    case VisCal::Re: {
      solveRPar().resize(nPar(),1,nAnt());
      solveRPar()=0.0;
      break;
    }
    default:
      throw(AipsError("Parameters must be entirely Real or entirely Complex for now!"));
    }

    solveParOK().resize(nPar(),1,nAnt());
    solveParOK()=True;
    solveParErr().resize(nPar(),1,nAnt());
    solveParErr()=0.0;
    solveParSNR().resize(nPar(),1,nAnt());
    solveParSNR()=0.0;

  }
  currSpw()=0;

}

void SolvableVisJones::syncDiffMat() {

  if (prtlev()>5) cout << "     SVJ::syncDiffMat()" 
		       << " (DJValid()=" << DJValid() << ")" << endl;

  // Sync the diff'd Jones matrices
  if (!DJValid()) syncDiffJones();

  // Sync up Muellers, if necessary
  //  if (applyByMueller()) 
      // Do nothing for now!  --All dJ applied directly in differentiate
      // if (!DMValid()) syncDiffMueller();

}

void SolvableVisJones::syncDiffJones() {

  if (prtlev()>6) cout << "      SVJ::syncDiffJones()" << endl;

  // If differentiated Jones are trivial, we are
  // already referencing the type-dep trivial versions
  //  TBD: Review this for D, where trivialJonesElem()=False,
  //   but diffJ is "trivial-ish"!!!  (Probably need trivDiffJonesElem(),
  //   or override this method in D to make it no-op)

  if (trivialDJ())
    // Ensure trivial matrices ready
    initTrivDJ();
  else {
    diffJElem().resize(IPosition(4,jonesNPar(jonesType()),nPar(),nChanMat(),nCalMat()));
    diffJElem().unique();
    invalidateDJ();

    // Calculate for all ants/chans
    calcAllDiffJones();

  }

  // Ensure diff'd Jones matrix renders are OK
  createDiffJones();

  // diff'd Jones matrices now valid
  validateDJ();
  invalidateDM();   // dMs still invalid, probably forever

}

void SolvableVisJones::calcAllDiffJones() {

  if (prtlev()>6) cout << "       SVJ::calcAllDiffJones" << endl;

  // Should handle OK flags in this method, and only
  //  do  calc if OK

  Matrix<Complex> oneDJ;   //  (nElem,nPar)
  Vector<Complex> onePar;      //  (nPar)

  ArrayIterator<Complex> dJiter(diffJElem(),2);
  ArrayIterator<Complex> Piter(currCPar(),1);
  
  for (Int iant=0; iant<nCalMat(); iant++) {

    // Solving parameters are NEVER channel-dependent
    //  (even if data & matrices are)
    onePar.reference(Piter.array());
      
    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneDJ.reference(dJiter.array());

      // Calculate the DJ matrices w.r.t. each par on this ant/chan
      calcOneDiffJones(oneDJ,onePar);

      // Advance iterators
      dJiter.next();

    }
    Piter.next();
  }

}

void SolvableVisJones::calcOneDiffJones(Matrix<Complex>& mat, 
					const Vector<Complex>& par) {

  if (prtlev()>10) cout << "        SVJ::calcOneDiffJones()" << endl;

  // If Jones matrix is trivial, shouldn't get here
  if (trivialJonesElem()) 
    throw(AipsError("Trivial Jones Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown non-trivial dJones-from-parameter calculation requested."));

}

void SolvableVisJones::createDiffJones() {

  if (prtlev()>6) cout << "       SVJ::createDiffJones()" << endl;

  Jones::JonesType jtype(jonesType());

  // Delete if wrong type
  if (dJ1_ && dJ1().type() != jtype) delete dJ1_;
  if (dJ2_ && dJ2().type() != jtype) delete dJ2_;
  
  // If needed, construct the correct diff Jones
  if (!dJ1_) dJ1_ = casa::createJones(jtype);
  if (!dJ2_) dJ2_ = casa::createJones(jtype);
      
}

void SolvableVisJones::initTrivDJ() {

  if (prtlev()>7) cout << "        SVJ::initTrivDJ()" << endl;

  // If DJ matrice not trivial, shouldn't get here
  if (!trivialDJ()) 
    throw(AipsError("Trivial Jones Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown trivial dJ initialization requested."));

}

void SolvableVisJones::stateSVJ(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisJones::state();

  // Get parent's state (w/out VC):
  SolvableVisMueller::stateSVM(False);

  if (applyByJones()) {
    if (prtlev()>3) cout << "SVJ::stateSVJ()" << endl;
    cout << boolalpha;
    
    // Now SVJ-specific stuff:
    cout << "  DJValid() = " << DJValid() << endl;
    
    cout << "  diffJElem().shape() = " << diffJElem().shape() 
	 << " (" << diffJElem().data() << ")" << endl;
    cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
  }
}

void SolvableVisJones::globalPostSolveTinker() {

  // Re-reference the phase, if requested
  if (refant()>-1) applyRefAnt();

  // Apply more general post-solve stuff
  SolvableVisCal::globalPostSolveTinker();

}

void SolvableVisJones::applyRefAnt() {

  // TBD:
  // 1. Synchronize refant changes on par axis
  // 2. Implement minimum mean deviation algorithm

  if (refant()<0) 
    throw(AipsError("No refant specified."));

  // Get the refant name from the nMS
  String refantName("none");
  MeasurementSet ms(msName());
  MSAntennaColumns msantcol(ms.antenna());
  refantName=msantcol.name()(refant());

  logSink() << "Applying refant: " << refantName
	    << LogIO::POST;
  // , optimizing phase continuity even if refant sometimes missing." 

  Bool newway(True);

  // Use ANTENNA-table-ordered refant list if user's choice bad
  //  The first two entries in this list are the user's refant,
  //   and the one that was used on the previous interval.  This
  //   makes the priorities correct all the time.
  Vector<Int> refantlist(nAnt()+2);
  indgen(refantlist);
  refantlist-=2;
  refantlist(0)=refantlist(1)=refant();

  Bool usedaltrefant(False);

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    // TBD: use spwOK here when spwOK is activated in solve context
    if (cs().nTime(ispw)>0) {

    // References to ease access to solutions
    Array<Complex> sol(cs().par(ispw));
    Array<Bool> sok(cs().parOK(ispw));

    // Find time-ordered slot index
    Vector<uInt> ord;
    genSort(ord,cs().time(ispw));
    Int nslots(ord.nelements());

    // Reference each channel and par independently (?)
    for (Int ichan=0;ichan<nChanPar();++ichan) {
      for (Int ipar=0;ipar<nPar();++ipar) {
	
	Int iref=0;
	Int lastrefant(-1);
	
	// Apply refant to first slot
	if (cs().nTime(ispw)>0) {
	  
	  IPosition blc1(4,ipar,ichan,0,ord(0));
	  IPosition trc1(4,ipar,ichan,nAnt()-1,ord(0));
	  if (ntrue(sok(blc1,trc1))>0) {
	    
	    while ( iref<(nAnt()+2) &&
		    !sok(IPosition(4,ipar,ichan,refantlist(iref),ord(0))) ) {
	      ++iref;
	    }
	    
	    if (iref<nAnt()+2) {
	      // found a refant, use it
	      
	      if (iref>0) usedaltrefant=True;
	      
	      Int currrefant=refantlist(iref);

	      // Only report if using an alternate refant
	      if (currrefant!=lastrefant && iref>0) {
		logSink() 
			  << "At " 
			  << MVTime(cs().time(ispw)(ord(0))/C::day).string(MVTime::YMD,7) 
			  << " ("
			  << "Spw=" << ispw 
			  << ", Fld=" << cs().fieldId(ispw)(ord(0))
			  << ", pol=" << ipar 
			  << ", chan=" << ichan 
			  << ")"
			  << ", using refant " << msantcol.name()(currrefant)
			  << " (id=" << currrefant 
			  << ")" << " (alternate)"
			  << LogIO::POST;
	      }  

	      Complex rph=sol(IPosition(4,ipar,ichan,currrefant,ord(0)));

	      // make reference phasor phase-only
	      Float amp=abs(rph);
	      if (amp>0.0) {
		rph/=Complex(amp);

		// Adjust each good ant by this phasor
		for (Int iant=0;iant<nAnt();++iant) 
		  if (sok(IPosition(4,ipar,ichan,iant,ord(0)))) 
		    sol(IPosition(4,ipar,ichan,iant,ord(0)))/=rph;
	      }
	      else
		// This shouldn't happen
		cout << "Refant has undefined phase (zero amplitude) " << endl;

	      lastrefant=currrefant;

	    }
	    else {
	      // unlikely?
	      cout << "Couldn't find a refant for the first slot in spw = " << currSpw() << endl;
	    }
	  }
	}

	
	// Only if time-dep referencing required....
	if (cs().nTime(ispw) > 1) {
	  
	if (newway) {
	    
          Int islot0(0);
          Int islot1(0);

          IPosition blc0(4,ipar,ichan,0,ord(islot0));
          IPosition trc0(4,ipar,ichan,nAnt()-1,ord(islot0));
          IPosition blc1(4,ipar,ichan,0,ord(islot1));
          IPosition trc1(4,ipar,ichan,nAnt()-1,ord(islot1));

	  while (islot0<nslots && ntrue(sok(blc0,trc0))<1) {
	    ++islot0;
	    blc0(3)=trc0(3)=ord(islot0);
	  }
	  islot1=islot0+1;

	  // Only if we found more than one good slot
	  if (islot1<nslots) {

	    Complex rph(1.0);
	    Float refantph(0.0);
	    Int currrefant(-1);
	    while (islot1<nslots) {
	      
	      blc1(3)=trc1(3)=ord(islot1);
	      
	      // If current slot has good solutions in it,
	      //  attempt to reference their phases
	      if (ntrue(sok(blc1,trc1))>0) {
		
		iref=0;
		while ( iref<(nAnt()+2) &&
			(!sok(IPosition(4,ipar,ichan,refantlist(iref),ord(islot0))) ||
			 !sok(IPosition(4,ipar,ichan,refantlist(iref),ord(islot1))))   ) {
		  ++iref;
		}
		
		if (iref<nAnt()+2) {
		  // found a refant, use it
		  
		  if (iref>0) usedaltrefant=True;
		  
		  currrefant=refantlist(iref);
  
		  // Only report if using an alternate refant
		  if (currrefant!=lastrefant && iref>0) {
		    logSink() << LogIO::NORMAL
			      << "At " 
			      << MVTime(cs().time(ispw)(ord(islot1))/C::day).string(MVTime::YMD,7) 
			      << " ("
			      << "Spw=" << ispw 
			      << ", Fld=" << cs().fieldId(ispw)(ord(islot1))
			      << ", pol=" << ipar 
			      << ", chan=" << ichan 
			      << ")"
			      << ", using refant " << msantcol.name()(currrefant)
			      << " (id=" << currrefant 
			      << ")" << " (alternate)"
			      << LogIO::POST;
		  }
		  
		  lastrefant=currrefant;
		  
		  // 2nd priority refant on next iteration is the one used this iteration
		  refantlist(1)=currrefant;
		  
		  Complex &r0=sol(IPosition(4,ipar,ichan,currrefant,ord(islot0)));
		  Complex &r1=sol(IPosition(4,ipar,ichan,currrefant,ord(islot1)));
		  
		  // If we can calculate a meaningful ref phasor, do it
		  if (abs(r0)>0.0f && abs(r1)>0.0f) {
		    
		    rph=Complex(DComplex(r1)/DComplex(r0));
		    rph/=abs(rph);
		    
		    // Remember first encountered refant phase
		    if (refantph==0.0 && iref==0) refantph=arg(r0);
		    
		    // TBD: attempt to optimize precision...
		    //DComplex drph=(DComplex(r1)/DComplex(r0));
		    //drph/=abs(drph);
		    
		    // Adjust each good ant by this phasor
		    for (Int iant=0;iant<nAnt();++iant) 
		      if (sok(IPosition(4,ipar,ichan,iant,ord(islot1)))) {
			sol(IPosition(4,ipar,ichan,iant,ord(islot1)))/=rph;
			
			// TBD: attempt to optimize precision
			//DComplex stmp=sol(IPosition(4,ipar,ichan,iant,ord(islot1)));
			//stmp/=drph;
			//sol(IPosition(4,ipar,ichan,iant,ord(islot1)))=Complex(stmp);
		      }
		    
		    //		  cout << " --> " << arg(r1)*180.0/C::pi << endl;
		    
		  } // non-zero reference amps
		  else 
		    // Should be impossible to reach here!
		    cout << "Bad referencing phasors." << endl;
		  
		  
		} // refant ok
		else {
		  // Continuity broken...
		  logSink() << LogIO::WARN
			    << " Phase continuity broken at "
			    << MVTime(cs().time(ispw)(ord(islot1))/C::day).string(MVTime::YMD,7) 
			    << " ("
			    << "Spw=" << ispw 
			    << ", Fld=" << cs().fieldId(ispw)(ord(islot1))
			    << ", pol=" << ipar 
			    << ")" << endl
			    << "  due to lack of overlapping antennas with previous interval."
			    << LogIO::POST;
		}
		
		// Good slot is basis for referencing the next slot
		islot0=islot1;
		blc0(3)=trc0(3)=ord(islot0);
		
	      }
	      ++islot1;
	    }
	    
	    // Zero initial refant phase
	    if (refantph!=0.0) {
	      //	    cout << "refantph = " << refantph*180.0/C::pi << endl;
	      rph=Complex(cos(refantph),sin(refantph));
	      for (Int islot=0;islot<nslots;++islot)
		for (Int iant=0;iant<nAnt();++iant) 
		  if (sok(IPosition(4,ipar,ichan,iant,ord(islot))))
		    sol(IPosition(4,ipar,ichan,iant,ord(islot)))/=rph;
	    }
	    
	  } // islot0<nslots
	  }
	  else {

	  // This is the old way (out of time order)

	  IPosition blc(3,ipar,ichan,0);
	  IPosition trc(3,ipar,ichan,nAnt()-1);

	  ArrayIterator<Complex> sol(cs().par(ispw),3);
	  ArrayIterator<Bool> sOk(cs().parOK(ispw),3);

	  Int islot(0);
	  
	  // Advance to first slot that has some good solutions
	  while (ntrue(sOk.array()(blc,trc))<1 &&
		 !sol.pastEnd()) {
	    sol.next();
	    sOk.next();
	    islot++;
	  }

	  // Arrays for referencing slices
	  Cube<Complex> s0, s1;
	  Cube<Bool> ok0, ok1;

	  // We are at the initial "prior" solution
	  s0.reference(sol.array());
	  ok0.reference(sOk.array());
	  
	  // Nominally, first solution to adjust is the next one
	  sol.next();
	  sOk.next();
	  islot++;
	
	  Complex rph(1.0);
	  Int lastiref(-1);
	  while (!sol.pastEnd()) {
	
	    // Do referencing if current slot has any good solutions
	    if (ntrue(sOk.array()(blc,trc))>0) {
	      
	      // The current solution
	      s1.reference(sol.array());
	      ok1.reference(sOk.array());

	      // Find first refant this and prev slot have in common
	      Int iref=0;
	      while ( iref<(nAnt()+1) &&
		      (!ok0(ipar,ichan,refantlist(iref)) || 
		       !ok1(ipar,ichan,refantlist(iref)))  ) iref++;

	      if (iref>nAnt())
		cout << "No antenna overlap..." << endl;
	      else {
		// found a refant, use it

		if (iref!=lastiref)
		  cout << "Using refant id=" << refantlist(iref) << " at "
		       << MVTime(cs().time(ispw)(islot)/C::day).string(MVTime::YMD,7) << " "
		       << "(islot=" << islot 
		       << ", Spw=" << ispw 
		       << ", Fld=" << cs().fieldId(ispw)(islot)
		       << ", ipar=" << ipar 
		       << ")" << endl;

		lastiref=iref;
	    
	      		
		Complex &r0=s0(ipar,ichan,refantlist(iref));
		Complex &r1=s1(ipar,ichan,refantlist(iref));

		//		cout << arg(r0)*180.0/C::pi << " "
		//		     << arg(r1)*180.0/C::pi << " ";

		
		// If we can calculate a meaningful ref phasor, do it
		if (abs(r0)>0.0f && abs(r1)>0.0f) {
		  
		  rph=r1/r0;
		  rph/=abs(rph);
		  
		  //		  cout << "(" << arg(rph)*180.0/C::pi << ") ";


		  /*
		    cout << islot << " " << ichan << " " << ipar << " "
		    << " refant = " << refantlist(iref) << " ph="
		    << arg(rph)*180.0/C::pi 
		    << endl;
		  */
		  
		  // Adjust each good ant by this phasor
		  for (Int iant=0;iant<nAnt();++iant) 
		    if (ok1(ipar,ichan,iant))
		      s1(ipar,ichan,iant)/=rph;

		  //		  cout << " --> " << arg(r1)*180.0/C::pi << endl;

		} // non-zero reference amps
		else 
		  cout << "Bad referencing phasors." << endl;


	      } // refant ok

	      // This slot is now basis for referencing the next one 
	      s0.reference(s1);
	      ok0.reference(ok1);
	      
	    } // a good interval

	    // Advance to next solution
	    islot++;
	    sol.next();
	    sOk.next();

	  } // !pastEnd
	  
	  } // newway

	} // nTime>1
	
      } // ipar
    } // ichan
    }      
  } // ispw


  if (usedaltrefant)
    logSink() << LogIO::NORMAL
	      << " NB: An alternate refant was used at least once to maintain" << endl
	      << "  phase continuity where the user's refant drops out." << endl
	      << "  This may introduce apparent phase jumps in " << endl
	      << "  the reference antenna; these are generally harmless."
	      << LogIO::POST;

}

void SolvableVisJones::fluxscale(const Vector<Int>& refFieldIn,
                                 const Vector<Int>& tranFieldIn,
                                 const Vector<Int>& inRefSpwMap,
				 const Vector<String>& fldNames,
                                 Matrix<Double>& fluxScaleFactor) {

  // For updating the MS History Table
  //  LogSink logSink_p = LogSink(LogMessage::NORMAL, False);
  //  logSink_p.clearLocally();
  //  LogIO oss(LogOrigin("calibrater", "fluxscale()"), logSink_p);

  // PtrBlocks to hold mean gain moduli and related
  PtrBlock< Matrix<Bool>* >   ANTOK;
  PtrBlock< Matrix<Double>* > MGNORM;
  PtrBlock< Matrix<Double>* > MGNORM2;
  PtrBlock< Matrix<Double>* > MGNWT;
  PtrBlock< Matrix<Double>* > MGNVAR;
  PtrBlock< Matrix<Int>* >    MGNN;

  Int nMSFld; fldNames.shape(nMSFld);

  // assemble complete list of available fields
  Vector<Int> fldList;
  for (Int iSpw=0;iSpw<nSpw();iSpw++) {
    Int currlen;
    fldList.shape(currlen);

    if (cs().nTime(iSpw) > 0) {

      //      cout << "cs().fieldId(iSpw) = " << cs().fieldId(iSpw) << endl;

      Vector<Int> thisFldList; thisFldList=cs().fieldId(iSpw);
      Int nThisFldList=genSort(thisFldList,(Sort::QuickSort | Sort::NoDuplicates));
      thisFldList.resize(nThisFldList,True);
      fldList.resize(currlen+nThisFldList,True);
      for (Int ifld=0;ifld<nThisFldList;ifld++) {
        fldList(currlen+ifld) = thisFldList(ifld);
      }
    }
  }
  Int nFldList=genSort(fldList,(Sort::QuickSort | Sort::NoDuplicates));
  fldList.resize(nFldList,True);

  //  cout << "fldList = " << fldList << endl;

  Int nFld=max(fldList)+1;

  try {

    // Resize, NULL-initialize PtrBlocks
    ANTOK.resize(nFld);   ANTOK=NULL;
    MGNORM.resize(nFld);  MGNORM=NULL;
    MGNORM2.resize(nFld); MGNORM2=NULL;
    MGNWT.resize(nFld);   MGNWT=NULL;
    MGNVAR.resize(nFld);  MGNVAR=NULL;
    MGNN.resize(nFld);    MGNN=NULL;

    // sort user-specified fields
    Vector<Int> refField; refField = refFieldIn;
    Vector<Int> tranField; tranField = tranFieldIn;
    Int nRef,nTran;
    nRef=genSort(refField,(Sort::QuickSort | Sort::NoDuplicates));
    nTran=genSort(tranField,(Sort::QuickSort | Sort::NoDuplicates));

    // make masks for ref/tran among available fields
    Vector<Bool> tranmask(nFldList,True);
    Vector<Bool> refmask(nFldList,False);
    for (Int iFld=0; iFld<nFldList; iFld++) {
      if ( anyEQ(refField,fldList(iFld)) ) {
        // this is a ref field
        refmask(iFld)=True;
        tranmask(iFld)=False;
      }
    }

    // Check availability of all ref fields
    if (ntrue(refmask)==0) {
      throw(AipsError(" Cannot find specified reference field(s)"));
    }
    // Any fields present other than ref fields?
    if (ntrue(tranmask)==0) {
      throw(AipsError(" Cannot find solutions for transfer field(s)"));
    }

    // make implicit reference field list
    MaskedArray<Int> mRefFldList(fldList,LogicalArray(refmask));
    Vector<Int> implRefField(mRefFldList.getCompressedArray());

    //    cout << "implRefField = " << implRefField << endl;
    // Check for missing reference fields
    if (Int(ntrue(refmask)) < nRef) {
      ostringstream x;
      for (Int iRef=0; iRef<nRef; iRef++) {
        if ( !anyEQ(fldList,refField(iRef)) ) {
          if (refField(iRef)>-1 && refField(iRef) < nMSFld) x << fldNames(refField(iRef)) << " ";
          else x << "Index="<<refField(iRef)+1<<"=out-of-range ";
        }
      }
      String noRefSol=x.str();
      logSink() << LogIO::WARN
		<< " The following reference fields have no solutions available: "
		<< noRefSol
		<< LogIO::POST;
      refField.reference(implRefField);
    }
    refField.shape(nRef);

    // make implicit tranfer field list
    MaskedArray<Int> mTranFldList(fldList,LogicalArray(tranmask));
    Vector<Int> implTranField(mTranFldList.getCompressedArray());
    Int nImplTran; implTranField.shape(nImplTran);

    //    cout << "implTranField = " << implTranField << endl;

    // Check availability of transfer fields

    // If user specified no transfer fields, use implicit 
    //  transfer field list, ELSE check for missing tran fields
    //  among those they specified
    if (nTran==0) {
      tranField.reference(implTranField);
      logSink() << LogIO::NORMAL
		<< " Assuming all non-reference fields are transfer fields."
		<< LogIO::POST;
    } else {
      if ( !(nTran==nImplTran &&
             allEQ(tranField,implTranField)) ) {
        ostringstream x;
        for (Int iTran=0; iTran<nTran; iTran++) {
          if ( !anyEQ(implTranField,tranField(iTran)) ) {
            if (tranField(iTran)>-1 && tranField(iTran) < nMSFld) x << fldNames(tranField(iTran)) << " ";
            else x << "Index="<<tranField(iTran)+1<<"=out-of-range ";
          }
        }
        String noTranSol=x.str();
	logSink() << LogIO::WARN
		  << " The following transfer fields have no solutions available: "
		  << noTranSol
		  << LogIO::POST;
        tranField.reference(implTranField);
      }
    }
    tranField.shape(nTran);

    // Report ref, tran field info
    String refNames(fldNames(refField(0)));
    for (Int iRef=1; iRef<nRef; iRef++) {
      refNames+=" ";
      refNames+=fldNames(refField(iRef));
    }
    logSink() << " Found reference field(s): " << refNames 
	      << LogIO::POST;
    String tranNames(fldNames(tranField(0)));
    for (Int iTran=1; iTran<nTran; iTran++) {
      tranNames+=" ";
      tranNames+=fldNames(tranField(iTran));
    }
    logSink() << " Found transfer field(s):  " << tranNames 
	      << LogIO::POST;

    //    cout << "fldList = " << fldList << endl;

    //    cout << "nFld = " << nFld << endl;


    // Handle spw referencing
    Vector<Int> refSpwMap;
    refSpwMap.resize(nSpw());
    indgen(refSpwMap);

    if (inRefSpwMap(0)>-1) {
      if (inRefSpwMap.nelements()==1) {
        refSpwMap=inRefSpwMap(0);
        logSink() << " All spectral windows will be referenced to spw=" << inRefSpwMap(0) 
		  << LogIO::POST;
      } else {
        for (Int i=0; i<Int(inRefSpwMap.nelements()); i++) {
          if (inRefSpwMap(i)>-1 && inRefSpwMap(i)!=i) {
            refSpwMap(i)=inRefSpwMap(i);
	    logSink() << " Spw=" << i << " will be referenced to spw=" << inRefSpwMap(i) 
		      << LogIO::POST;
          }
        }
      }
    }

    // Scale factor info
    fluxScaleFactor.resize(nSpw(),nFld); fluxScaleFactor=-1.0;

    Matrix<Double> fluxScaleError(nSpw(),nFld,-1.0);
    Matrix<Double> gainScaleFactor(nSpw(),nFld,-1.0);
    Matrix<Bool> scaleOK(nSpw(),nFld,False);

    Matrix<Double> fluxScaleRatio(nSpw(),nFld,0.0);
    Matrix<Double> fluxScaleRerr(nSpw(),nFld,0.0);

    // Field names for log messages
    Vector<String> fldname(nFld,"");

    //    cout << "Filling mgnorms....";

    // fill per-ant -fld, -spw  mean gain moduli
    for (Int iSpw=0; iSpw<nSpw(); iSpw++) {

      if (cs().nTime(iSpw) > 0 ) {

        for (Int islot=0; islot<cs().nTime(iSpw); islot++) {
          Int iFld=cs().fieldId(iSpw)(islot);
          if (ANTOK[iFld]==NULL) {
            // First time this field, allocate ant/spw matrices
            ANTOK[iFld]   = new Matrix<Bool>(nElem(),nSpw(),False);
            MGNORM[iFld]  = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNORM2[iFld] = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNWT[iFld]   = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNVAR[iFld]  = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNN[iFld]    = new Matrix<Int>(nElem(),nSpw(),0);

            // record name
            fldname(iFld) = cs().fieldName(iSpw)(islot);
          }
          // References to PBs for syntactical convenience
          Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
          Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
          Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
          Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
          Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));

          for (Int iAnt=0; iAnt<nElem(); iAnt++) {
	    Double wt=cs().iFitwt(iSpw)(iAnt,islot);
	    
	    for (Int ipar=0; ipar<nPar(); ipar++) {
	      IPosition ip(4,ipar,0,iAnt,islot);
	      if (cs().parOK(iSpw)(ip)) {
		antOK(iAnt,iSpw)=True;
		Double gn=norm( cs().par(iSpw)(ip) );
		mgnorm(iAnt,iSpw) += (wt*gn);
		mgnorm2(iAnt,iSpw)+= (wt*gn*gn);
		mgnn(iAnt,iSpw)++;
		mgnwt(iAnt,iSpw)+=wt;
	      }
            }
          }
        }
      }
    }
    //    cout << "done." << endl;

    //    cout << "Normalizing mgnorms...";

    // normalize mgn
    for (Int iFld=0; iFld<nFld; iFld++) {

      //      cout << "iFld = " << iFld << " " << ANTOK[iFld]->column(0) << endl;

      // Have data for this field?
      if (ANTOK[iFld]!=NULL) {

        // References to PBs for syntactical convenience
        Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
        Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
        Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
        Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
        Matrix<Double> mgnvar;  mgnvar.reference(*(MGNVAR[iFld]));
        Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));

        for (Int iSpw=0; iSpw<nSpw(); iSpw++) {
          for (Int iAnt=0; iAnt<nElem(); iAnt++) {
            if ( antOK(iAnt,iSpw) && mgnwt(iAnt,iSpw)>0.0 ) {
              mgnorm(iAnt,iSpw)/=mgnwt(iAnt,iSpw);
              mgnorm2(iAnt,iSpw)/=mgnwt(iAnt,iSpw);
              // Per-ant, per-spw variance (non-zero only if sufficient data)
              if (mgnn(iAnt,iSpw) > 2) {
                mgnvar(iAnt,iSpw) = (mgnorm2(iAnt,iSpw) - pow(mgnorm(iAnt,iSpw),2.0))/(mgnn(iAnt,iSpw)-1);
              }
            } else {
              mgnorm(iAnt,iSpw)=0.0;
              mgnwt(iAnt,iSpw)=0.0;
              antOK(iAnt,iSpw)=False;
            }
       /*
            cout << endl;
            cout << "iFld = " << iFld;
            cout << " iAnt = " << iAnt;
            cout << " mgnorm = " << mgnorm(iAnt,iSpw);
            cout << " +/- " << sqrt(1.0/mgnwt(iAnt,iSpw));
            cout << " SNR = " << mgnorm(iAnt,iSpw)/sqrt(1.0/mgnwt(iAnt,iSpw));
            cout << "  " << mgnn(iAnt,iSpw);
            cout << endl;
       */
          }
        }


      }
    }

    //    cout << "done." << endl;
    //    cout << "nTran = " << nTran << endl;

    //    cout << "Calculating scale factors...";

    // Scale factor calculation, per spw, trans fld
    for (Int iTran=0; iTran<nTran; iTran++) {
      Int tranidx=tranField(iTran);

      // References to PBs for syntactical convenience
      Matrix<Bool>   antOKT;  antOKT.reference(*(ANTOK[tranidx]));
      Matrix<Double> mgnormT; mgnormT.reference(*(MGNORM[tranidx]));
      Matrix<Double> mgnvarT; mgnvarT.reference(*(MGNVAR[tranidx]));
      Matrix<Double> mgnwtT;  mgnwtT.reference(*(MGNWT[tranidx]));

      for (Int iSpw=0; iSpw<nSpw(); iSpw++) {

        // Reference spw may be different
        Int refSpw(refSpwMap(iSpw));

        // Only if anything good for this spw
        if (ntrue(antOKT.column(iSpw)) > 0) {

          // Numerator/Denominator for this spw, trans fld
          Double sfref=0.0;
          Double sfrefwt=0.0;
          Double sfrefvar=0.0;
          Int sfrefn=0;

          Double sftran=0.0;
          Double sftranwt=0.0;
          Double sftranvar=0.0;
          Int sftrann=0;

          Double sfrat=0.0;
          Double sfrat2=0.0;
          Int sfrn=0;

	  Int nAntUsed(0);
          for (Int iAnt=0; iAnt<nElem(); iAnt++) {

            // this ant OK for this tran field?
            if ( antOKT(iAnt,iSpw) ) {

              // Check ref fields and accumulate them
              Bool refOK=False;
              Double sfref1=0.0;
              Double sfrefwt1=0.0;
              Double sfrefvar1=0.0;
              Int sfrefn1=0;
              for (Int iRef=0; iRef<nRef; iRef++) {

                Int refidx=refField(iRef);
                Matrix<Bool> antOKR; antOKR.reference(*(ANTOK[refidx]));
                Bool thisRefOK(antOKR(iAnt,refSpw));
                refOK = refOK || thisRefOK;
                if (thisRefOK) {
                  Matrix<Double> mgnormR; mgnormR.reference(*(MGNORM[refidx]));
                  Matrix<Double> mgnwtR;  mgnwtR.reference(*(MGNWT[refidx]));
                  Matrix<Double> mgnvarR; mgnvarR.reference(*(MGNVAR[refidx]));
                  sfref1   += mgnwtR(iAnt,refSpw)*mgnormR(iAnt,refSpw);
                  sfrefwt1 += mgnwtR(iAnt,refSpw);
                  if (mgnvarR(iAnt,refSpw)>0.0) {
                    sfrefvar1+= (mgnwtR(iAnt,refSpw)/mgnvarR(iAnt,refSpw));
                    sfrefn1++;
                  }

                }
              }

              // If ref field accumulation ok for this ant, accumulate
              if (refOK) {
		nAntUsed+=1;
                sftran   += (mgnwtT(iAnt,iSpw)*mgnormT(iAnt,iSpw));
                sftranwt += mgnwtT(iAnt,iSpw);
                if (mgnvarT(iAnt,iSpw)>0.0) {
                  sftranvar+= (mgnwtT(iAnt,iSpw)/mgnvarT(iAnt,iSpw));
                  sftrann++;
                }

                sfref    += sfref1;
                sfrefwt  += sfrefwt1;
                sfrefvar += sfrefvar1;
                sfrefn   += sfrefn1;

                // Accumlate per-ant ratio
                Double thissfrat= mgnormT(iAnt,iSpw)/(sfref1/sfrefwt1);
                sfrat    += thissfrat;
                sfrat2   += (thissfrat*thissfrat);
                sfrn++;
                //cout << "Ratio: " << thissfrat << " " << sfrat << " " << sfrat2 << " " << sfrn << endl;
              }



            } // antOKT
          } // iAnt

          // normalize numerator and denominator, variances
          if (sftranwt > 0.0) {
            sftran/=sftranwt;
            sftranvar/=sftranwt;
            if (sftranvar > 0.0 && sftrann > 0) {
              sftranvar*=sftrann;
              sftranvar=1.0/sftranvar;
            }
          }
          if (sfrefwt  > 0.0) {
            sfref/=sfrefwt;
            sfrefvar/=sfrefwt;
            if (sfrefvar > 0.0 && sfrefn > 0) {
              sfrefvar*=sfrefn;
              sfrefvar=1.0/sfrefvar;
            }
          }

          //      cout << endl;
          //      cout << "Tran = " << sftran << " +/- " << sqrt(sftranvar) << endl;
          //      cout << "Ref  = " << sfref  << " +/- " << sqrt(sfrefvar) << endl;

          // form scale factor for this tran fld, spw
          if (sftran > 0.0 && sfref > 0.0) {
            fluxScaleFactor(iSpw,tranidx) = sftran/sfref;
            fluxScaleError(iSpw,tranidx)  = sqrt( sftranvar/(sftran*sftran) + sfrefvar/(sfref*sfref) );
            fluxScaleError(iSpw,tranidx) *= fluxScaleFactor(iSpw,tranidx);
            gainScaleFactor(iSpw,tranidx) = sqrt(1.0/fluxScaleFactor(iSpw,tranidx));
            scaleOK(iSpw,tranidx)=True;

          /*
            cout << "iTran = " << iTran;
            cout << "  fluxScaleFactor = " << fluxScaleFactor(iSpw,tranidx);
            cout << " +/- " << fluxScaleError(iSpw,tranidx);
            cout << "  gainScaleFactor = " << gainScaleFactor(iSpw,tranidx);
            cout << endl;
          */
            // Report flux densities:
	    logSink() << " Flux density for " << fldNames(tranidx)
		      << " in SpW=" << iSpw;
            if (refSpw!=iSpw) 
	      logSink() << " (ref SpW=" << refSpw << ")";

	    logSink() << " is: " << fluxScaleFactor(iSpw,tranidx)
		      << " +/- " << fluxScaleError(iSpw,tranidx)
		      << " (SNR = " << fluxScaleFactor(iSpw,tranidx)/fluxScaleError(iSpw,tranidx)
		      << ", nAnt= " << nAntUsed << ")"
		      << LogIO::POST;

            // form scale factor from mean ratio
            sfrat  /= (Double(sfrn));
            sfrat2 /= (Double(sfrn));
            fluxScaleRatio(iSpw,tranidx) = sfrat;
            fluxScaleRerr(iSpw,tranidx) = sqrt( (sfrat2 - sfrat*sfrat) / Double(sfrn-1) );

          /*
            cout << "iTran = " << iTran;
            cout << "  fluxScaleRatio = " << fluxScaleRatio(iSpw,tranidx);
            cout << " +/- " << fluxScaleRerr(iSpw,tranidx);
            cout << endl;
          */
          } else {
            logSink() << LogIO::WARN
		      << " Insufficient information to calculate scale factor for "
		      << fldname(tranidx)
		      << " in SpW="<< iSpw+1
		      << LogIO::POST;
          }

        } // ntrue(antOKT) > 0
      } // iSpw
    } // iTran

    // quit if no scale factors found
    if (ntrue(scaleOK) == 0) throw(AipsError("No scale factors determined!"));

    //    cout << "done." << endl;

    //    cout << "Adjusting gains...";

    // Adjust tran field's gains here
    for (Int iSpw=0; iSpw<nSpw(); iSpw++) {
      if (cs().nTime(iSpw)>0) {
        for (Int islot=0; islot<cs().nTime(iSpw); islot++) {
          Int iFld=cs().fieldId(iSpw)(islot);
          // If this is a tran fld and gainScaleFactor ok
          if (scaleOK(iSpw,iFld)) {
            for (Int iAnt=0; iAnt<nElem(); iAnt++) {
	      for (Int ipar=0; ipar<nPar(); ipar++) {
		IPosition ip(4,ipar,0,iAnt,islot);
		if (cs().parOK(iSpw)(ip))
                  cs().par(iSpw)(ip)*=gainScaleFactor(iSpw,iFld);
              }
            }
          }
        }
      }
    }

    //    cout << "done." << endl;

    //    cout << "Cleaning up...";

    // Clean up PtrBlocks
    for (Int iFld=0; iFld<nFld; iFld++) {
      if (ANTOK[iFld]!=NULL) {
        delete ANTOK[iFld];
        delete MGNORM[iFld];
        delete MGNORM2[iFld];
        delete MGNWT[iFld];
        delete MGNVAR[iFld];
        delete MGNN[iFld];
      }
    }

    //    cout << "done." << endl;

    // Avoid this since problem with <msname>/SELECTED_TABLE (06Feb02 gmoellen)
    /*
    {

      MeasurementSet ms(vs_->msName().before("/SELECTED"));
      Table historytab = Table(ms.historyTableName(),
                               TableLock(TableLock::UserNoReadLocking),
                               Table::Update);
      MSHistoryHandler hist = MSHistoryHandler(ms, "calibrater");
      historytab.lock(True);
      oss.postLocally();
      hist.addMessage(oss);
      historytab.unlock();
    }
    */
  }
  catch (AipsError x) {

    // Clean up PtrBlocks
    for (Int iFld=0; iFld<nFld; iFld++) {
      if (ANTOK[iFld]!=NULL) {
        delete ANTOK[iFld];
        delete MGNORM[iFld];
        delete MGNORM2[iFld];
        delete MGNWT[iFld];
        delete MGNVAR[iFld];
        delete MGNN[iFld];
      }
    }

    throw(x);

  }

}


void SolvableVisJones::listCal(const Vector<Int> ufldids,
			       const Vector<Int> uantids,
			       const Int& spw, 
			       const Int& chan,
			       const String& listfile,
			       const Int& pagerows) {

/* Function to output calibration table.
   2007sep26 - jcrossle - Modified 'listCal' to produce prompt after printing
     'maxScrRows'.  Prompt allows user to (Q)uit, print (A)ll without prompt,
     or, by default, print 'maxScrRows' more and prompt again (RETURN).  After
     each prompt, the column headings, time stamp, and field name are 
     rewritten.
       Also added additional loop to the output.  First loop
     uses normal 'cout' (sends output to terminal).  Second loop redirects
     'cout' to file ".listcal.out" (filename hard coded).  File output receives
     column headings only once.  No prompts issued while writing to file.
   2007sep27 - gmoellen - Added user-supplied listfile and pagerows
     parameters to control if a file is written, and how many rows
     to write per page.  Remoeved Jared's iout loop, since user can 
     control this directly now.
   2007oct01 - gmoellen - Some fixes:
     1. Reorganized various loops so that non-trivial field selection 
     doesn't cause hang, mainly, by moving the ielem++ back to the for
     statement.
     2. Added header on each page, even when writing to file
     3. Other misc. cleanup
   2007oct10 - gmoellen - Index cal data directly, instead of
     via a variously incremented pointer.  This simplifies Jared's
     pending re-formatting work, and also fixes a channel selection
     bug in listing of B tables.
   2007oct10 - jcrossle - Output format changed, as per efomalont's 
     suggestions.  Looping is now done over 1st- antenna, 2nd- time,
     3rd- polarization.  However, multiple antennas are written per row,
     so "sub-loops" over 'numAntCols' antennas are performed when 
     necessary. New hard-coded parameter, numAntCols, controls
     the number of antennas printed per row.
   2007oct29 - jcrossle - Created local Vector 'pAntids' that holds
     the antenna ID's that will be written; 'numAnts' holds the number
     of elements in pAntids.  The for-loop over antennas is now actually
     a loop over the index of Vector pAntids.  This avoids output problems
     when the user specifies antenna ID's.
   2007nov06 - jcrossle - Format corrections.  Discovered that task 
     parameter antenna takes (exact) antenna names, as a string
     of comma-separated values; '3,7' does Not equal '03,07' 
   2007nov14 - gmoellen - Moved MS name listing out of loops so
     it only appears once.
*/

  // Catch bad spw specification:
  if (spw<0 || spw>=nSpw() || cs().nTime(spw)==0)
    throw(AipsError("Nothing to list for specified spw."));

  // Do specified channel, nominally
  Int dochan=chan;
  const Int nchan=cs().par(spw).shape()(1);
  if (dochan>(nchan-1)) {
    dochan=0;
  }

// /* Query the terminal size. */   // DISABLED
//   struct winsize mywinsize;
//   ioctl(1, TIOCGWINSZ, &mywinsize);
//   // diagnostic message
//   printf("screen size = rows: %d, columns: %d\n", mywinsize.ws_row, mywinsize.ws_col);
// /* Set listCal output to correspond to terminal size. */
//   Int maxScrRows(mywinsize.ws_row); // number of rows to print before prompting

  Int maxScrRows(pagerows);
  Int scrRows(0); // screen row counter, reset after continue prompt
  Bool endOutput = False; // if true, end output immediately
  Int isol = 0; // total solution counter
  Bool prompt = True; // if true, issue prompt
  
  // Redirect cout to listfile
  ofstream file;
  streambuf* sbuf = cout.rdbuf();
  if(listfile!="") {
    // non-interactive
    prompt = False;
    
    // Guard against trampling existing file
    File diskfile(listfile);
    if (diskfile.exists()) {
      String errmsg = "File: " + listfile + 
      " already exists; delete it or choose a different name.";
      throw(AipsError(errmsg));
    }
    else
      cout << "Writing output to file: " << listfile << endl;

    file.open(listfile.data());
    cout.rdbuf(file.rdbuf());
  }

  // Access to ms for meta info
  MeasurementSet ms(msName());
  MSAntennaColumns msant(ms.antenna());
  Vector<String> antname(msant.name().getColumn());
  MSFieldColumns msfld(ms.field());
  Vector<String> fldname(msfld.name().getColumn());
  
  // Setup the column widths.  Check width of each string.
  int timeLength = 10;
  int maxFldstrLength = 0; 
  for (Int ielem=0;ielem<cs().nElem(); ielem++) {
    for (Int itime=0;itime<cs().nTime(spw);++itime) {
      Int fldid(cs().fieldId(spw)(itime));
      String fldstr=(fldname(fldid)); 
      Int fldstrLength = fldstr.length();
      if (maxFldstrLength < fldstrLength) { maxFldstrLength = fldstrLength; }
    }
  }
  
  int numAntCols = 4; // Number of antenna columns

  logSink() << LogIO::NORMAL1
       << "Listing CalTable: " << calTableName()
       << "   (" << typeName() << ") "
       << LogIO::POST;
  cout << endl
       << "Listing CalTable: " << calTableName()
       << "   (" << typeName() << ") "
       << endl;

  String dateTimeStr0=MVTime(cs().time(spw)(0)/C::day).string(MVTime::YMD,7);
  String dateStr0=dateTimeStr0.substr(0,10);
  
  // Default header info.
  logSink() << LogIO::NORMAL1 << "MS name = " << msName() << LogIO::POST;
  logSink() << LogIO::NORMAL << "Date= " << dateStr0 << endl
                             << "SpwId= " << spw << endl
                             << "Channel= " << dochan << endl;
  cout << "MS name = " << msName() << ", "
       << "Date= " << dateStr0 << ", "
       << "SpwId= " << spw << ", "
       << "Channel= " << dochan << "."
       << endl
       << "---------------------------------------------------------------"
       << endl;

  // labels for flagged solutions
  Vector<String> flagstr(2); 
  flagstr(0)="F";
  flagstr(1)=" ";

  //  Complex *g=cs().par(spw).data()+dochan;
  //  Bool *gok=cs().parOK(spw).data()+dochan;
  
  Array<Complex> g;
  g.reference(cs().par(spw));
  Array<Bool> gok;
  gok.reference(cs().parOK(spw));
  IPosition gidx(4,0,dochan,0,0);

  // Setup a Vector of antenna ID's to ease the process of printing 
  //  multiple antenna columns per row.
  Int numAnts; // Hold number of antennas to be output.
  Vector<Int> pAntids(cs().nElem()); // Vector of antenna ID's.
  if (uantids.nelements()==0) { // Print all antennas.
    numAnts = cs().nElem(); 
    for(int i=0; i<cs().nElem(); pAntids[i]=i++); // Fill pAntids with all antenna ID's.
  } else { // Use the user-specified antenna ID's.
    numAnts = uantids.nelements(); 
    pAntids.resize(numAnts);
    pAntids = uantids;
  }

  // loop over antenna elements
  for (Int ielem=0;ielem<numAnts;ielem=ielem+numAntCols) { 
    gidx(2)=pAntids(ielem);

    Bool header=True; // New antenna, require print header

    // If antenna element is among selected antennas, print it
    if (uantids.nelements()==0 || anyEQ(uantids,pAntids(ielem))) {
    
      // Begin loop over time
      for (Int itime=0;itime<cs().nTime(spw);++itime) { 
        gidx(3)=itime;

        Int fldid(cs().fieldId(spw)(itime));

        // Get date-time string
        String dateTimeStr=MVTime(cs().time(spw)(itime)/C::day).string(MVTime::YMD,7);
        String dateStr=dateTimeStr.substr(0,10);
        String timeStr=dateTimeStr.substr(11,10);
        // Get field string
        String fldStr=(fldname(fldid));

        String tmp_timestr = timeStr; // tmp_ variables get reset during the loop
        String tmp_fldstr = fldStr; //

        // If no user-specified fields, or fldid is in user's list
        if (ufldids.nelements()==0 || anyEQ(ufldids,fldid) ) {
         
          // If beginning new screen, print the header
          if (scrRows == 0 || header) {  //(irow%cs().nElem()==0) 
            header=False;
            string AmpPhaseStr = "  Amp    Phase ";
            string AmpPhaseDel = "---------------"; // 15 chars long
            
            // Begin writing Antenna line (put spaces over the time, field cols)
            for(int k=0; k<(timeLength+1+maxFldstrLength); k++) { cout<<" "; }
            // Finish antenna line
            for(int k=0; (k<numAntCols) and (ielem+k<=numAnts-1); k++) {
              cout << "| ";
              // Center antenna name between polarization columns
              //  For now, the maximum antenna name length is 8. Could be as
              //  high as 32-6=24.
              int antNameLength = ( antname(pAntids(ielem+k)).length() < 8)
                                  ? antname(pAntids(ielem+k)).length() : 8;
              float halfspace = ( 32 - 6 - antNameLength ) / 2.0; 
              // cout << antname(pAntids(ielem+k)).length() << "," << halfspace << "," << int(halfspace) << "," << int(halfspace+0.5);
              for(int j=1; j<=int(halfspace); j++) { cout<<" "; }
              cout << "Ant = "
                   << setw(antNameLength) << antname(pAntids(ielem+k));
              for(int j=1; j<=int(halfspace+0.5); j++) { cout<<" "; }
            }
            cout << endl;
              
              // Write time and field headings (only once)
            cout << setiosflags(ios::left)
                 << setw(timeLength) << "Time"  << " "
                 << setw(maxFldstrLength) << "Field";
              
            // Write Amp and Phase headings for each antenna column
            for(int k=0; (k<numAntCols) and (ielem+k<=numAnts-1); k++) {
              cout << "| " << AmpPhaseStr << "  " << AmpPhaseStr;
            }
            cout << endl;
            // Write delimiters
            for(int k=0; k<timeLength; k++) { cout << "-"; } // time col
            cout << " ";
            for(int k=0; k<maxFldstrLength; k++) { cout << "-"; } // field col
            for(int k=0; (k<numAntCols) and (ielem+k<=numAnts-1); k++) {
              cout << "| " << AmpPhaseDel << "  " << AmpPhaseDel;
            }
            cout << endl;
            scrRows+=4;
            // End of header
          }
          
          cout << setw(timeLength) << timeStr << " "
               << setw(maxFldstrLength) << fldStr;
          // Set i/o flags for writing data
          cout << setiosflags(ios::fixed) << setiosflags(ios::right);

          // Write data for each antenna column
          for (Int kelem=ielem; (kelem<ielem+numAntCols) and (kelem<=numAnts-1); 
               kelem++) {
            gidx(2)=pAntids(kelem);
            
            // Loop over polarization
            for (Int ipar=0;ipar<nPar();++ipar) {
              gidx(0)=ipar; 

              cout << "| "
		   << setprecision(3) << setw(6) << abs(g(gidx)) 
		   << flagstr(Int(gok(gidx))) << " "
		   << setprecision(1) << setw(6) << arg(g(gidx))*180.0/C::pi 
		   << flagstr(Int(gok(gidx)));
            }
          }

          cout << resetiosflags(ios::right) << endl;
          isol=isol+numAntCols; scrRows++;

          // If at end of screen, signal new page (new header)
          if (maxScrRows>0 && (scrRows >= maxScrRows-1) ) {  

            // signal a new page
            scrRows = 0;

            // query the user, if we are interactive
            if (prompt) {
              string contStr;
              cout << "Type Q to quit, A to list all, or RETURN to continue [continue]: ";
              getline(cin,contStr);
              if ( (contStr.compare(0,1,"q") == 0) or 
                 (contStr.compare(0,1,"Q") == 0) ) { endOutput=True; }
              if ( (contStr.compare(0,1,"a") == 0) or 
                 (contStr.compare(0,1,"A") == 0) ) { prompt = False; }
            }
          }
        } // end if (field)

        if (endOutput) {break;} // break out of itime loop
      } // itime

    } // end if (antenna)

    if (endOutput) {break;} // break out of ielem loop
  } // ielem
  logSink() << LogIO::NORMAL1
            << "Listed " << isol << " antenna solutions." 
            << LogIO::POST; 
  cout << endl
       << "Listed " << isol << " antenna solutions." 
       << endl << endl;

  // restore cout
  if(listfile!="")
    cout.rdbuf(sbuf);

}// end function listCal


// Globals

// Return a cal table's type, verifying its existence
String calTableType(const String& tablename) {

  // Check existence...
  if (!Table::isReadable(tablename)) {
    ostringstream o;
    o << "Table " << tablename
      << " does not exist.";
    throw(AipsError(String(o)));
  }

  // Table exists...
  
  TableInfo ti(Table::tableInfo(tablename));
  
  // ...Check if Calibration table....
  if (ti.type()!="Calibration") {
    ostringstream o;
    o << "Table " << tablename
      << " is not a valid Calibration table.";
    throw(AipsError(String(o)));
    
  }
  
  // If we get here, we have a calibration table,
  //  so return its type
  return ti.subType();

}


} //# NAMESPACE CASA - END
