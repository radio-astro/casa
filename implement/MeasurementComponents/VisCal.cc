//# VisCal.cc: Implementation of Jones classes
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

#include <synthesis/MeasurementComponents/VisCal.h>
#include <msvis/MSVis/VisBuffer.h>
#include <synthesis/MeasurementEquations/VisEquation.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include <casa/Quanta/MVTime.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for a non-solveable Jones matrix for any solely
// visibility indexed gain. This caches the interferometer gains and
// the inverses according to time. It could be generalized to cache on
// something else. The antenna gain is calculated as needed per slot.

// From VisSet. 


// **********************************************************
//  VisCal Implementations
//

VisCal::VisCal(VisSet& vs) :
  vs_(&vs), 
  nSpw_(vs.numberSpw()),
  nPar_(0),
  nAnt_(vs.numberAnt()),
  nBln_(0),
  currSpw_(-1),
  nChanDat_(vs.numberSpw(),1),
  nChanPar_(vs.numberSpw(),1),
  nChanMat_(vs.numberSpw(),1),
  startChan_(vs.numberSpw(),0),
  timeDepMat_(False),
  interval_(0.0),
  applied_(False)
{
// Generic TVVJ constructor, from VisSet alone
//   Makes a virtually brain-dead VisJones; the real action is in
//     setInterpolation (and setSolver in SVJ)
// Input:
//    vs                    VisSet&       VisSet (w/ implicit selection)
// Output to protected/private data:
//    vs_                   VisSet*       Pointer to parent VisSet
//    applied_=F            Bool          applied?
//    interval_=0.0         Double        Iteration interval
//    
//    numberAnt_            Int           Number of antennas in VisSet
//    numberSpw_            Int           Number of spws in VisSet

//  cout << "VisCal ctor " << endl;

  // Number of baselines
  nBln_ = nAnt_*(nAnt_+1)/2;

}

VisCal::~VisCal() {}

void VisCal::setApply(const Record& apply) 
{
  // Do nothing, for now, until non-solvable types need this
}


// **********************************************************
//  SolvableVisCal Implementations
//

SolvableVisCal::SolvableVisCal(VisSet& vs) :
  VisCal(vs),
  calTableName_(""),
  calTableSelect_(""),
  tInterpType_(""),
  fInterpType_(""),
  spwMap_(vs.numberSpw(),-1),
  refant_(-1),
  preavg_(DBL_MAX),
  solved_(False),
  mode_("") {

  //  cout << "SolvableVisCal ctor " << endl;

};
 
void SolvableVisCal::setApply(const Record& apply) {
  //  Inputs:
  //    apply           Record&       Contains application params
  //    

  // Collect Cal table parameters
  if (apply.isDefined("table"))
    calTableName()=apply.asString("table");
  if (apply.isDefined("select"))
    calTableSelect()=apply.asString("select");

  // Collect interpolation parameters
  if (apply.isDefined("interp"))
    tInterpType()=apply.asString("interp");
  if (apply.isDefined("spwmap")) {
    Vector<Int> spwmap(apply.asArrayInt("spwmap"));
    spwMap()(IPosition(1,0),IPosition(1,spwmap.nelements()-1))=spwmap;
  }

  if (apply.isDefined("t"))
    interval()=apply.asFloat("t");

  // This is apply context  
  setApplied(True);
  setSolved(False);


}


void SolvableVisCal::setSolve(const Record& solve) 
{

  // Collect parameters
  if (solve.isDefined("t"))
    interval()=solve.asFloat("t");

  //  if (solve.isDefined("preavg"))
  //    preavg()=solve.asFloat("preavg");

  if (solve.isDefined("refant"))
    refant()=solve.asInt("refant");

  if (solve.isDefined("phaseonly"))
    if (solve.asBool("phaseonly"))
      setMode("phaseonly");

  if (solve.isDefined("table"))
    calTableName()=solve.asString("table");

  if (solve.isDefined("preavg"))
    preavg()=solve.asFloat("preavg");

  // This is the solve context
  setSolved(True);
  setApplied(False);

}




// **********************************************************
//  VisMueller Implementations
//

VisMueller::VisMueller(VisSet& vs) :
  VisCal(vs),
  cs_(NULL),
  cint_(NULL),
  currTime_(vs.numberSpw(),0.0),
  currField_(vs.numberSpw(),-1),
  currFreq_(vs.numberSpw(),-1),
  lastTime_(vs.numberSpw(),0.0),
  refTime_(0.0),
  refFreq_(0.0),
  MMValid_(vs.numberSpw(),False),
  thisPar_(vs.numberSpw(),NULL),
  thisParOK_(vs.numberSpw(),NULL),
  thisMueller_(vs.numberSpw(),NULL),
  thisMuellerOK_(vs.numberSpw(),NULL)
{
  //  cout << "VisMueller ctor " << endl;
  
  for (Int ispw=0;ispw<nSpw(); ispw++) {
    thisPar_[ispw] = new Cube<Complex>();
    thisParOK_[ispw] = new Matrix<Bool>();
    thisMueller_[ispw] = new Matrix<mMat>();
    thisMuellerOK_[ispw] = new Matrix<Bool>();
  }
}

VisMueller::~VisMueller() {
  deleteMuellerCache();
  if (cs_)   delete cs_;   cs_=NULL;
  if (cint_) delete cint_; cint_=NULL;
}

// Apply calibration to visibilties (corrupt data)
VisBuffer& VisMueller::apply(VisBuffer& vb) {
  applyCal(vb,False);
  return vb;
}

// Apply inverse of calibration to visibilities  (correct data)
VisBuffer& VisMueller::applyInv(VisBuffer& vb) {
  applyCal(vb,True);
  return vb;
}

// Apply this calibration to VisBuffer visibilities
VisBuffer& VisMueller::applyCal(VisBuffer& vb,const Bool& doInv) {

  // ASSUMES ONLY *ONE* TIMESTAMP IN VISBUFFER

  //  cout << " apply " << typeName() << ": ";

  // Obtain gain matrices appropriate to this VisBuffer
  //   (Ignoring fieldid for now)
  syncGain(vb,doInv);

  // Data info/indices
  Int* dataChan;
  Bool* flagR=&vb.flagRow()(0);
  Int* a1=&vb.antenna1()(0);
  Int* a2=&vb.antenna2()(0);
  Bool* flag=&vb.flag()(0,0);
  Bool polSwitch=polznSwitch(vb);
  CStokesVector* visp=&vb.visibility()(0,0);
  
  // iterate rows
  Int nRow=vb.nRow();
  for (Int row=0; row<nRow; row++,flagR++,a1++,a2++) {
    
    if (!*flagR) {  // if this row unflagged
      
      dataChan=&vb.channel()(0);
      
      Int solCh0(0);
      if (freqDepMat()) {
	solCh0=(*dataChan)-startChan()(currSpw());
	if (solCh0 < 0) solCh0=0;
      }
    
      // A pointer to this baseline's gain (spectrum) matrix
      mMat* gainp   = &(thisMueller(currSpw(),solCh0,*a1,*a2));
      Bool* gainOKp = &(thisMuellerOK(currSpw(),solCh0,*a1,*a2));
      
      for (Int chn=0; chn<nChanDat()(currSpw()); chn++,flag++,visp++,dataChan++) {

	if (*gainOKp) {
	  
	  // if this data channel unflagged
	  if (!*flag) {  
	    if (polSwitch) polznMap(*visp);
	    (*visp) *= (*gainp);
	    if (polSwitch) polznUnmap(*visp);
	  };
	}
	else {
	  *flag=True;
	}

	// inc soln ch axis if freq-dependent (and next dataChan within soln)
	if (freqDepMat() && 
	    ( *dataChan+1>startChan()(currSpw()) &&
	      (*dataChan+1)<(startChan()(currSpw())+nChanMat()(currSpw()) ) ) ) {
	  gainp++; gainOKp++;  
	}

      } // chn


    } // !*flagR
    else {
      flag+=nChanDat()(currSpw()); visp+=nChanDat()(currSpw());
    }
  }
  return vb;
}


void VisMueller::deleteMuellerCache() {
  for (Int ispw=0; ispw<nSpw(); ispw++) {
    if (thisPar_[ispw])       {delete thisPar_[ispw];       thisPar_[ispw]=NULL;};
    if (thisParOK_[ispw])     {delete thisParOK_[ispw];     thisParOK_[ispw]=NULL;};
    if (thisMueller_[ispw])   {delete thisMueller_[ispw];   thisMueller_[ispw]=NULL;};
    if (thisMuellerOK_[ispw]) {delete thisMuellerOK_[ispw]; thisMuellerOK_[ispw]=NULL;};
  }
}


void VisMueller::syncGain(const VisBuffer& vb,
			  const Bool& doInv) {

  // Remember which indices to which we are syncing
  currSpw()=vb.spectralWindow();
  currTime()=vb.time()(0);
  currField()=vb.fieldId();
  currFreq().resize(); currFreq()=vb.frequency();

  currFreq()/=1.0e9;  // In GHz

  // Data size from vb, of course
  nChanDat()(currSpw())=vb.nChannel();

  refFreq()=currFreq()(nChanDat()(currSpw())/2);

  // Ensure matrix channel axis length is correct
  if (freqDepPar())
    // Matrix size currently follows parameters
    nChanMat()(currSpw())=nChanPar()(currSpw());
  else
    if (freqDepMat())
      // Matrix size follows data when mats are freq dep, but pars are not
      nChanMat()(currSpw())=vb.nChannel();
    else
      nChanMat()(currSpw())=1;

  // If new time stamp (for currSpw()), procede with sync
  if (currTime() != lastTime() ) {
    syncGain(currSpw(),currTime(),currField(),doInv);
    lastTime()=currTime();

    // If matrices are time-dep within interval, 
    //  this will ensure new matrix calculation
    //   (even if pars aren't new)
    if (timeDepMat()) invalidateMM(currSpw());

  }

}

void VisMueller::syncGain(const Int& spw,
			  const Double& time,
			  const Int& field,
			  const Bool& doInv) {

    // Synchronize the Mueller parameters
    syncPar(spw,time,field);
    
    // If Mueller matrices invalid (new parameters), re-sync them
    if (!MMValid(spw)) syncMueller(spw,time,field,doInv);
    
}

void VisMueller::syncPar(const Int& spw,
			 const Double& time,
			 const Int& field) {

  // Break any previous reference
  thisPar(spw).resize();

  // Calculate Mueller parameters by some means (e.g., interpolation):
  calcPar(spw,time,field);

}


void VisMueller::calcPar(const Int& spw,
			 const Double& time,
			 const Int& field) {

  Bool newcal(False);

  // Interpolate solution
  newcal=ci().interpolate(time,spw);

  // Reference result
  thisPar(spw).reference(ci().result());

  // In case we need solution timestamp
  refTime() = ci().slotTime();

  // If new, Mueller matrices are now out-of-date
  if (newcal) invalidateMM(spw);

}


void VisMueller::syncMueller(const Int& spw,
			     const Double& time,
			     const Int& field,
			     const Bool& doInv) {

  //  cout << "VM::syncMueller" << endl;

  // Set the size of the Mueller matrix cache
  IPosition mip(2,nChanMat()(currSpw()),nBln());
  
  // If size has changed, resize and invalidate
  if (!mip.isEqual(thisMueller(currSpw()).shape())) {

    thisMueller(currSpw()).resize(mip);
    thisMuellerOK(currSpw()).resize(mip);
    thisMuellerOK(currSpw())=True;
   
    // Muellers necessarily invalid now 
    invalidateMM(currSpw());
  }

  currTime()=time;

  // Generate Mueller for each bln & ch
  Matrix<Complex> thisBL;
  Vector<Complex> parvec;
  for (Int ibln=0; ibln<nBln(); ibln++) {

    // The current baseline's pars as a matrix
    thisBL.reference(thisPar(spw).xyPlane(ibln));

    // The following assumes that nChanPar()=1 or nChanMat()

    // First (only?) channel's pars as a vector
    parvec.reference(thisBL.column(0));
    for (Int ich=0; ich<nChanMat()(spw); ich++) {
      
      // Keep channelized pars in sync with data & matrices
      if (freqDepPar())
	parvec.reference(thisBL.column(ich));

      // Calculate the Mueller matrix
      calcMueller(thisMueller(spw)(ich,ibln),parvec,ibln,ich);

    }
  }

  // Invert, if requested
  if (doInv) invMueller(spw);

  // Mueller matrices now valid
  validateMM(spw);

}

void VisMueller::calcMueller(mMat& mat, Vector<Complex>& par, 
			     const Int& baseline, const Int& channel ) {

  // Default version, creates either scalar (nPar_=1) or diagonal

  switch (nPar()) {
  case 1: { mat=par[0]; break; };
  default: { 
    for (uInt i=0;i<par.nelements();i++)
      if (abs(par(i))==0.0) par(i)=Complex(1.0,0.0);
    mat=par; 
    break; };
  }    
}

void VisMueller::invMueller(const Int& spw) {

  //  cout << " invM ";

  mMat invmat;

  for (Int ibln=0;ibln<nBln();ibln++) {
    for (Int ichan=0; ichan<nChanMat()(spw); ichan++) {
      //      if (thisMuellerOK(spw)(ibln,ichan)) {
	thisMueller(spw)(ichan,ibln).inverse(invmat);
	thisMueller(spw)(ichan,ibln)=invmat;
	//      }
    }
  }
}

// Determine if polarization re-sequencing is required
Bool VisMueller::polznSwitch(const VisBuffer& vb)
{
  Vector<Int> corr=vb.corrType();
  Bool needToSwitch=False;
  if (corr.nelements() > 2) {
    needToSwitch=(corr(0)==Stokes::XX && corr(1)==Stokes::YY) ||
      (corr(0)==Stokes::RR && corr(1)==Stokes::LL);
  };
  return needToSwitch;
};

// Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
void VisMueller::polznMap(CStokesVector& vis)
{
  Complex vswap=vis(1);
  vis(1)=vis(2);
  vis(2)=vis(3);
  vis(3)=vswap;
};

// Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
void VisMueller::polznUnmap(CStokesVector& vis)
{
  Complex vswap=vis(3);
  vis(3)=vis(2);
  vis(2)=vis(1);
  vis(1)=vswap;
};


// **********************************************************
//  SolvableVisMueller Implementations
//


SolvableVisMueller::SolvableVisMueller(VisSet& vs) :
  VisCal(vs),
  VisMueller(vs),
  SolvableVisCal(vs),
  cs_(NULL),
  cint_(NULL),
  R_(),
  dR_(),
  DMValid_(False),
  diffMueller_(),
  chiSq_(),
  lastChiSq_(),
  sumWt_(),
  grad_(),
  nHess_(0),
  hess_(),
  dg_(),
  almostconverged_(False),
  tolerance_(0.1)
 {
  //  cout << "SolvableVisMueller ctor " << endl;

   nHess()=nBln();
}

SolvableVisMueller::~SolvableVisMueller() {
  if (cs_)   delete cs_;   cs_=NULL;
  if (cint_) delete cint_; cint_=NULL;

}


void SolvableVisMueller::setApply(const Record& apply) {
  //  Inputs:
  //    apply           Record&       Contains application params
  //    

  // Call parent
  SolvableVisCal::setApply(apply);

  // Create CalSet, from table
  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());

  // These come from CalSet now, but will eventually come from CalInterp
  nChanPar() = cs().nChan();
  startChan() = cs().startChan();

  // Create CalInterp
  cint_ = new CalInterp(cs(),tInterpType(),"nearest");
  ci().setSpwMap(spwMap());

}

void SolvableVisMueller::setSolve(const Record& solve) 
{
  
  // Call parent to establish parameters
  SolvableVisCal::setSolve(solve);

  // Create a pristine CalSet
  cs_ = new CalSet<Complex>(nSpw());

}


Bool SolvableVisMueller::solve(VisEquation& ve) {

  //  cout << "SVM::solve" << endl;

  LogSink logsink;

  // Local copy of VisEq
  VisEquation lme(ve);

  // Local copy of VisSet, with correct chunking
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }

  VisSet localVS(vs(),columns,interval());

  // Use internal VisSet
  lme.setVisSet(localVS);

  // The VisIter/VisBuffer
  VisIter& vi(localVS.iter());
  VisBuffer vb(vi);

  Vector<Int> nSlot(nSpw(),0);

  // Find out how many solutions we have
  for (vi.originChunks(); vi.moreChunks();vi.nextChunk()) {
    // count number of slots per spw
    nSlot(vi.spectralWindow())++;
  }

  {
    LogMessage message(LogOrigin("SolvableVisMueller", "solve"));
    ostringstream o; o<<"For interval of "<<interval()<<" seconds, found "<<
		       sum(nSlot)<<" slots";message.message(o);
    logsink.post(message);
  }

  // For now, adopt pre-initChiSq data shape for matrix shape
  //  (When we support partially freq-averaged data for solve, (and freq-dep interp)
  //    we will have to be more clever here.)
  if (freqDepMat()) {
    nChanMat()  = localVS.numberChan();
    startChan() = localVS.startChan();
  } else {
    nChanMat()  = Vector<Int>(nSpw(),1);
    startChan() = Vector<Int>(nSpw(),0);
  }

  // For parameter shape, follow matrix shape if freqDepPar, else 1
  if (freqDepPar()) 
    nChanPar()  = nChanMat();
  else 
    nChanPar()  = Vector<Int>(nSpw(),1);

  // Resize the Calset
  cs().resize(nPar(),nChanPar(),nElem(),nSlot);
  cs().setStartChan(startChan());

  // Obtain solutions for each chunk 
  Vector<Int> islot(nSpw(),-1);
  Int solcount(0);
  for (vi.originChunks();
       vi.moreChunks();
       vi.nextChunk()) {

    Int thisSpw=currSpw()=vi.spectralWindow();

    islot(thisSpw)++;
    
    // Set field source info
    cs().fieldId(thisSpw)(islot(thisSpw))   =vi.fieldId();
    cs().fieldName(thisSpw)(islot(thisSpw)) =vi.fieldName();
    cs().sourceName(thisSpw)(islot(thisSpw))=vi.sourceName();

    // Set time info
    vi.origin();
    cs().startTime(thisSpw)(islot(thisSpw))  =vb.time()(0)-vb.timeInterval()(0)/2.0;

    refTime()=0.0;
    Int ntime(0);
    for (vi.origin(); vi.more(); vi++,ntime++) {
      refTime()+=vb.time()(0);
    }
    refTime()/=ntime;
    cs().stopTime(thisSpw)(islot(thisSpw)) =vb.time()(0)+vb.timeInterval()(0)/2.0;
    cs().time(thisSpw)(islot(thisSpw))=refTime();
    vi.origin();

    // Arrange VisEq, applying existing calibration
    //    lme.initChiSquare(*static_cast<SolvableVisCal*>(this) );

    //    cout << "Arranging data for solve" << endl;

    lme.initChiSquare(*this);

    // Remember access to the collapsed VB for use in solving internals
    VisBuffer cvb(lme.corrected());
    svb_=&cvb;

    nChanDat()(currSpw()) = svb().nChannel();

    // Current frequencies (this is constant in this chunk)
    currFreq().resize();  // (Break any previous reference)

    currFreq()=svb().frequency();
    currFreq()/=1.0e9;  // In GHz

    refFreq()=currFreq()(nChanDat()(currSpw())/2);

    // Obtain solution for this vb, deposit solution in correct slot,
    //   increment good solution count if solution successful

    //    cout << "Solving the VB" << endl;

    if ( solvevb(islot(thisSpw)) ) solcount++;

  } // chunks

  {
    LogMessage message(LogOrigin("SolvableVisMueller", "solve"));
    ostringstream o; 
    o<<"  Found  "<<solcount<<" good " << typeName() << " solutions." << endl
     <<"         "<<sum(nSlot)-solcount<< " solution intervals failed, or had insufficient data." << endl;  
    message.message(o);
    logsink.post(message);
  }
  {
    LogMessage message(LogOrigin("SolvableVisMueller", "solve"));
    ostringstream o; 
    o<<"  Storing " << typeName() << " solutions in table " << calTableName();
    message.message(o);
    logsink.post(message);
  }

  // Store it (no append yet)
  cs().store(calTableName(),typeName(),False);

  return True;
}

void SolvableVisMueller::initSolve() {
  // Assumes freqDepPar()=F

  //  cout << "SVM::initSolve" << endl;

  // Chi2 and weights
  chiSq().resize(nHess());
  lastChiSq().resize(nHess());
  sumWt().resize(nHess());

  chiSq()=0.0;
  sumWt()=0.0;

  // Diff'd Muellers necessarily invalid
  invalidateDM();

  // Gradient and Hessian
  grad().resize(nPar(),nHess());
  grad()=0.0;

  // hess().resize(nPar(),nHess()); // Diag version
  hess().resize(nPar(),nPar(),nHess()); // non-Diag version
  hess()=0.0;

  dg().resize(nPar(),nHess());
  dg()=0.0;

}

void SolvableVisMueller::chiSquare() {

  //  cout << "SVM::chiSquare" << endl;

  // Remember prior chi2, zero current
  lastChiSq()=chiSq();
  chiSq()=0.0;
  sumWt()=0.0;

  // Get residuals w.r.t. current parameters
  residual();

  // Pointer access to vb elements
  const Bool*  flr=&svb().flagRow()(0);
  const Bool*  fl= &svb().flag()(0,0);
  const Int*   a1= &svb().antenna1()(0);
  const Int*   a2= &svb().antenna2()(0);
  const Float* wt= &svb().weight()(0);

  CStokesVector* res;

  // Loop over row/channel
  Int ih(0),irow(0),ich(0);
  for (irow=0;irow<svb().nRow();irow++,flr++,a1++,a2++,wt++) {
    if (!(*flr)) {
      ih=min((nHess()-1),blnidx(*a1,*a2));
      res=&R()(0,irow);
      fl=&svb().flag()(0,irow);
      for (ich=0;ich<svb().nChannel();ich++,fl++,res++) {
	if (!(*fl)) {
	  chiSq()(ih) += (*wt)*real(innerProduct((*res),(*res)));
	  sumWt()(ih) += (*wt);
	}
      }
    }
  }

  // normalize 
  for (Int ih=0;ih<nHess();ih++) 
    if (sumWt()(ih)>0.0)
      chiSq()(ih)/=sumWt()(ih);
    else 
      chiSq()(ih)=sumWt()(ih)=0.0;
  

  //  cout << "End of SVM::chiSquare" << chiSq() << endl;

}
  
//  Vobs - (M)*Vmod,  - (dM)*Vmod
void SolvableVisMueller::residual() {  

  //  cout << "SVM::residual" << endl;

  // (This method can be vastly simplified by coordinating with VM::applyCal)

  // Data info/indices
  const Bool*    flagR= &svb().flagRow()(0);
  const Bool*    flag=  &svb().flag()(0,0);
  const Int*     a1=    &svb().antenna1()(0);
  const Int*     a2=    &svb().antenna2()(0);
  const Double*  time=  &svb().time()(0);
  CStokesVector* vis=   &svb().visibility()(0,0);
  CStokesVector* mod=   &svb().modelVisibility()(0,0);

  Bool polSwitch=polznSwitch(svb());
  
  // Shapes
  Int spw=  svb().spectralWindow();
  Int nrow= svb().nRow();
  Int nchan=svb().nChannel();

  // Prepare residuals array
  R().resize(nchan,nrow);
  R()=Complex(0.0);

  // Prepare differentials array
  dR().resize(nPar(),nchan,nrow);
  dR()=Complex(0.0);

  // Advance time to first non-zero value
  while (*time<0.1) time++;
  Double lastTime(*time);
  currTime()=*time;
  time=  &svb().time()(0);  // (reset to zeroth row)

  // Ensure Mueller/DiffMueller matrices up-to-date (for zeroth row)
  if (!MMValid(spw) || !DMValid()) syncDiffMueller(0);

  // iterate rows
  Int nRow=svb().nRow();
  for (Int row=0; row<nRow; row++,flagR++,a1++,a2++,time++) {

    Int bl=blnidx(*a1,*a2);

    if (!*flagR) {  // if this row unflagged

      // If timestamp has changed, update all Muellers if 
      //  matrix calc is time-dep
      if (timeDepMat() && *time != lastTime) {
	currTime()=*time;
	syncDiffMueller(row);
	lastTime= *time;
      }

      // A pointer to this baseline's first channel gain matrix
      //  NB: in solve context, data and soln have same number of channels,
      //      or soln only has 1.  (i.e., no channel offset to worry about)
      mMat* M = &(thisMueller(currSpw(),0,*a1,*a2));

      for (Int chn=0,schn=0; chn<nchan; chn++,flag++,vis++,mod++) {
	
	// if this data channel unflagged
	if (!*flag) {
	  if (polSwitch) {
	    polznMap(*vis);
	    polznMap(*mod);
	  }

	  R()(chn,row) = (*vis);
	  R()(chn,row) -= ((*M)*(*mod));

	  for (Int ipar=0;ipar<nPar();ipar++) {
	    dR()(ipar,chn,row) = (diffMueller()(ipar,schn,bl))*(*mod);
	    dR()(ipar,chn,row).operator-();
	  }

	  if (polSwitch) {
	    polznUnmap(*vis);
	    polznUnmap(*mod);
	  }
	};
      
	// inc soln ch axis if freq-dependent 
	if (freqDepMat()) { M++; schn++; };
	
      } // chn
      
      
    } // !*flagR
    else {
      flag+=nchan;
      vis+=nchan;
      mod+=nchan;
    }
  }

  //  cout << "End of SVM::residual" << endl;


}

void SolvableVisMueller::calcGradHess() {
  // Assumes freqDepPar()=F
  // Assumes we've already calculated R()  (via chiSquare/residual)

  //  cout << "SVM::calcGradHess" << endl;
  
  const Bool*  flagR= &svb().flagRow()(0);
  const Bool*  flag=  &svb().flag()(0,0);
  const Int*   a1=    &svb().antenna1()(0);
  const Int*   a2=    &svb().antenna2()(0);
  const Float* wt=    &svb().weight()(0);

  grad()=0.0;
  hess()=0.0;

  //  cout << endl;

  for (Int irow=0;irow<svb().nRow();irow++,a1++,a2++,flagR++,wt++) {
   
    if (!(*flagR)) {

      Int ih=min((nHess()-1),blnidx(*a1,*a2));
      
      flag=  &svb().flag()(0,irow);
      for (Int ich=0;ich<svb().nChannel();ich++,flag++) {
	
	if (!(*flag)) {


	  //	  cout << irow << " " << ich
	  //	       << " R() = " << R()(ich,irow) << endl;

	  for (Int ipar0=0;ipar0<nPar();ipar0++) {

	    //	    cout << "   " << ipar0
	    //		 << " dR() = " << dR()(ipar0,ich,irow) << endl;

	    grad()(ipar0,ih) += ((*wt)*innerProduct(  R()(ich,irow),       dR()(ipar0,ich,irow) ));

	    hess()(ipar0,ipar0,ih) += ((*wt)*innerProduct( dR()(ipar0,ich,irow), dR()(ipar0,ich,irow) ));
	    for (Int ipar1=ipar0+1;ipar1<nPar();ipar1++) {
	      hess()(ipar0,ipar1,ih) += ((*wt)*innerProduct( dR()(ipar0,ich,irow), dR()(ipar1,ich,irow) ));
	    }

	  }
	}

      }
    }
  }

  //  cout << "grad() = " << grad().column(1) << endl;
  //  cout << "hess() = " << hess().xyPlane(1) << endl;



}

void SolvableVisMueller::solveGradHess() {
  // Assumes freqDepPar()=F

  //  cout << "SVM::solveGradHess" << endl;

/*
  dg()=0.0;
  for (Int ibl=0;ibl<nBln();ibl++) {
    if (sumWt()(ibl) > 0.0) {
      //      cout << "  ibl=" << ibl << " grad() = " << grad().column(ibl) << endl;
      //      cout << "  ibl=" << ibl << " hess() = " << hess().column(ibl) << endl;

      for (Int ipar=0;ipar<nPar();ipar++) {
	if (hess()(ipar,ibl)>0.0) 
	  dg()(ipar,ibl) = (grad()(ipar,ibl)/hess()(ipar,ibl));
      }
      //      cout << "  ibl=" << ibl << " dg()   = " << dg().column(ibl) << " ";
      //      cout << abs(dg().column(ibl)) << endl;
    }
  }
*/

  char uplo('U');
  Int n(nPar());
  Int nrhs(1);
  Bool deleteaa, deletebb;
  Int lda(n);
  Int ldb(n);
  Int info;
  Complex *aa;
  Complex *bb;
    
  for (Int ih=0;ih<nHess();ih++) {
    if (sumWt()(ih) > 0.0) {


      /*
      hess().xyPlane(ibl).column(nPar()-1)=0.0;
      hess().xyPlane(ibl).row(nPar()-1)=0.0;
      hess().xyPlane(ibl)(nPar()-1,nPar()-1)=1.0;
      grad().column(ibl)(nPar()-1)=0.0;
      */

      aa = hess().xyPlane(ih).getStorage(deleteaa);
      bb = grad().column(ih).getStorage(deletebb);
      
      posv(&uplo, &n, &nrhs, aa, &lda, bb, &ldb, &info);
      
      //      if (info!=0) cout << "info = " << info << endl;
      
      hess().xyPlane(ih).putStorage(aa,deleteaa);
      grad().column(ih).putStorage(bb,deletebb);
    }
  }
  dg().resize();
  dg().reference(grad());


  //  cout << "dg() = " << dg() << endl;

}

void SolvableVisMueller::updgain() {
  // Assumes freqDepPar()=F

  //  cout << "SVM::updgain" << endl;

  for (Int ih=0;ih<nHess();ih++) 
    if (sumWt()(ih) > 0.0) 
      for (Int ipar=0;ipar<nPar();ipar++) 
	thisPar(currSpw())(ipar,0,ih)-=dg()(ipar,ih);
  
  // Associated matrices now invalid
  invalidateMM(currSpw());
  invalidateDM();
  
}

Bool SolvableVisMueller::converged(const Int& iter) {

  //  cout << "SVM::converged" << endl;

  // Insist on at least 2 iterations
  if (iter<2) return False;

  // If any baseline not yet converged, we aren't even almostconverged, 
  //  so return False  (this should be 
  Bool goodstep(False);
  for (Int ih=0;ih<nHess();ih++) 
    if (sumWt()(ih) > 0.0) {

      goodstep=False;
      Float dChiSq=(lastChiSq()(ih)-chiSq()(ih))/chiSq()(ih);
      if (dChiSq>-1.0e-8) goodstep=True;
      
      /*
      cout << "   " << chiSq()(ih) << " " << lastChiSq()(ih) << " "
	   << dChiSq << " " 
	   << (dChiSq>tolerance()) << " ";
      */

      if (chiSq()(ih)>1.0e-10 && abs(dChiSq)>tolerance()) {
	//	cout << "--> NO!" << endl;
	almostconverged()=False;  // Can't be! 
	return False;
      } 
    }

  // If we reach here, we are at least "almostconverged", which means
  //  we have passed the basic convergence criterion once.  We insist
  //  on a second iteration that does so to be "fully" converged

  // Only are almostconverged or converged if chiSq decreased
  if (goodstep) {

    // If already almostconverged now converged & return True;
    if (almostconverged()) { 
      //      cout << "--> CONVERGED!" << endl;
      return True;
    }
    else
      // We are now almostconverged
      almostconverged()=True;
  
    //    cout << "--> ALMOSTCONVERGED!" << endl;
  }
  //  else
  //  cout << "--> UPSTEP!" << endl;

  // If we reach here, at least one more pass
  return False;

}

// Synchronize Muellers AND derivatives w.r.t. them
void SolvableVisMueller::syncDiffMueller(const Int& row) {

  //  cout << "SVM::syncDiffMueller" << endl;

  // Set the size of the Mueller matrix cache
  IPosition mip2(2,nChanMat()(currSpw()),nBln());

  // If size has changed, resize and invalidate
  if (!mip2.isEqual(thisMueller(currSpw()).shape())) {
    thisMueller(currSpw()).resize(mip2);
    thisMuellerOK(currSpw()).resize(mip2);
    thisMuellerOK(currSpw())=True;
    // Muellers necessarily invalid now
    invalidateMM(currSpw());
  }

  // Set the size of the diffMueller matrix cache
  IPosition mip3(3,nPar(),nChanMat()(currSpw()),nBln());

  // If size has changed, resize and necessarily invalidate
  if (!mip3.isEqual(diffMueller().shape())) {
    diffMueller().resize(mip3);
    invalidateDM();
  }

  // Generate Mueller for each bln & ch
  Matrix<Complex> thisPar2;
  Vector<Complex> thisPar1;

  Matrix<mMat> thisDM2;
  Vector<mMat> thisDM1;

  for (Int ibln=0; ibln<nBln(); ibln++) {

    if (anyEQ(True,thisParOK(currSpw()).column(min(ibln,(nHess()-1))))) {
      
      // The current pars as a matrix
      thisPar2.reference(thisPar(currSpw()).xyPlane(min(ibln,(nHess()-1))));
      thisDM2.reference(diffMueller().xyPlane(min(ibln,(nHess()-1))));
      
      // First (only?) channel's pars as a vector
      thisPar1.reference(thisPar2.column(0));
      for (Int ich=0; ich<nChanMat()(currSpw()); ich++) {
	
	// Keep channelized pars in sync
	if (freqDepPar()) 
	  thisPar1.reference(thisPar2.column(ich));
	
	thisDM1.reference(thisDM2.column(ich));
	
	// Calculate the Mueller and diff'd Mueller matrix (per par)
	calcMueller(thisMueller(currSpw())(ich,ibln),thisPar1,ibln,ich);
	calcDiffMueller(thisDM1,thisPar1,ibln,ich);

	//	for (Int ipar=0;ipar<nPar();ipar++) 
	//	  cout << ibln << " " << ich << " " << ipar << " dM = " << thisDM1(ipar).matrix();

      }
    }
  }

  // diffMueller matrices now valid
  validateMM(currSpw());
  validateDM();

  //  cout << "End of SVM::sDM" << endl;

}




// **********************************************************
//  VisJones2 Implementations
//

VisJones2::VisJones2(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  JMValid_(vs.numberSpw(),False),
  thisPar_(vs.numberSpw(),NULL),
  thisParOK_(vs.numberSpw(),NULL),
  thisJones_(vs.numberSpw(),NULL),
  thisJonesConj_(vs.numberSpw(),NULL),
  thisJonesOK_(vs.numberSpw(),NULL)
{
  //  cout << "VisJones2 ctor " << endl;
  
  for (Int ispw=0;ispw<nSpw(); ispw++) {
    thisPar_[ispw] = new Cube<Complex>();
    thisParOK_[ispw] = new Matrix<Bool>();
    thisJones_[ispw] = new Matrix<jMat>();
    thisJonesConj_[ispw] = new Matrix<jMat>();
    thisJonesOK_[ispw] = new Matrix<Bool>();
  }
}

VisJones2::~VisJones2() {
  deleteJonesCache();
}

void VisJones2::syncMueller(const Int& spw,
			    const Double& time,
			    const Int& field,
			    const Bool& doInv) {

  // First, sync up Jones, if necessary (should be)
  if (!JMValid(spw)) syncJones(spw,time,field,doInv);

  // Create conjugated Jones matrices for use in outer product
  conjJones(spw);

  // Generate Mueller for each bln & ch
  for (Int iant1=0;iant1<nAnt()-1;iant1++) {
    for (Int iant2=iant1+1;iant1<nAnt();iant2++) {

      for (Int ich=0; ich<nChanMat()(spw); ich++) {
	
	// Calculate the Mueller matrix from Jones matrices
	calcMueller(thisMueller(spw,ich,iant1,iant2),
		    thisJones(spw,ich,iant1),
		    thisJonesConj(spw,ich,iant2));
	
      }
    }
  }

  // Mueller matrices now valid
  validateMM(spw);

}

void VisJones2::syncJones(const Int& spw,
			  const Double& time,
			  const Int& field,
			  const Bool& doInv) {

  // Set the size of the Jones matrix cache
  IPosition jip(2,nChanMat()(currSpw()),nAnt());
  
  // If size has changed, resize and invalidate
  if (!jip.isEqual(thisJones(currSpw()).shape())) {

    thisJones(spw).resize(jip);
    thisJonesOK(spw).resize(jip);
    thisJonesOK(spw)=True;

    // Jones & Mueller necessarily invalid now 
    invalidateJM(currSpw());
    invalidateMM(currSpw());
  }


  // Generate Jones for each ant & ch
  Matrix<Complex> thisAN;
  Vector<Complex> parvec;
  for (Int iant=0; iant<nAnt(); iant++) {

    // The current baseline's pars as a matrix
    thisAN.reference(thisPar(spw).xyPlane(iant));

    for (Int ich=0; ich<nChanMat()(spw); ich++) {

      // Current channel's pars as a vector
      parvec.reference(thisAN.column(ich));

      // Calculate the Jones matrix
      calcJones(thisJones(spw)(ich,iant),parvec);

    }
  }

  // Invert, if requested
  if (doInv) invJones(spw);

  // Jones matrices now valid
  validateJM(spw);


}



void VisJones2::calcPar(const Int& spw,
			const Double& time,
			const Int& field) {

  Bool newcal(False);

  // Interpolate solution
  newcal=ci().interpolate(time,spw);

  // Reference result
  thisPar(currSpw()).reference(ci().result());

  // If new, Jones and Mueller matrices are now out-of-date
  if (newcal) {
    invalidateJM(spw);
    invalidateMM(spw);
  }

}


void VisJones2::calcJones(jMat& mat, Vector<Complex>& par ) {

  // Default version, creates either scalar (nPar_=1) or diagonal

  switch (nPar()) {
  case 1: { mat=par[0]; break; };
  default: { 
    for (uInt i=0;i<par.nelements();i++)
      if (abs(par(i))==0.0) par(i)=Complex(1.0,0.0);
    mat=par; 
    break; };
  }    
  
  
}

void VisJones2::invJones(const Int& spw) {

  //  cout << " iJM ";

  jMat invmat;
  IPosition jip(thisJones(spw).shape());

  for (Int iant=0;iant<jip(1);iant++) {
    for (Int ichan=0; ichan<jip(0); ichan++) {
      //      if (thisJonesOK(spw)(ibln,ichan)) {
	thisJones(spw)(ichan,iant).inverse(invmat);
	thisJones(spw)(ichan,iant)=invmat;
	//      }
    }
  }
}

void VisJones2::conjJones(const Int& spw) {

  IPosition jip(thisJones(spw).shape());

  // Ensure thisJonesConj has correct shape
  thisJonesConj(spw).resize(jip);
  
  // Form conjugates (copy + conj)
  for (Int iant=0;iant<jip(1);iant++) {
    for (Int ichan=0; ichan<jip(0); ichan++) {
      thisJonesConj(spw)(ichan,iant)=thisJones(spw)(ichan,iant);
      thisJonesConj(spw)(ichan,iant).conj();
    }
  }

}

void VisJones2::calcMueller(mMat& mmat, jMat& jmat1, jMat& jmat2) {
  
  // Form outer (direct) product
  directProduct(mmat,jmat1,jmat2);

}

void VisJones2::deleteJonesCache() {
  for (Int ispw=0; ispw<nSpw(); ispw++) {
    if (thisPar_[ispw])     {delete thisPar_[ispw];     thisPar_[ispw]=NULL;};
    if (thisParOK_[ispw])   {delete thisParOK_[ispw];   thisParOK_[ispw]=NULL;};
    if (thisJones_[ispw])   {delete thisJones_[ispw];   thisJones_[ispw]=NULL;};
    if (thisJonesConj_[ispw])   {delete thisJonesConj_[ispw];   thisJonesConj_[ispw]=NULL;};
    if (thisJonesOK_[ispw]) {delete thisJonesOK_[ispw]; thisJonesOK_[ispw]=NULL;};
  }
}


// **********************************************************
//  SolvableVisJones2 Implementations
//


SolvableVisJones2::SolvableVisJones2(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  SolvableVisMueller(vs),
  VisJones2(vs),
  dJ_(Complex(1.0)),
  DJValid_(vs.numberSpw(),False) {

  //  cout << "SolvableVisJones2 ctor " << endl;

}


Bool SolvableVisJones2::solve(VisEquation& ve) {

  LogSink logsink;

  // Local copy of VisEq
  VisEquation lme(ve);

  // Local copy of VisSet, with correct chunking
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }

  VisSet localVS(vs(),columns,interval());

  // Use internal VisSet
  lme.setVisSet(localVS);

  // The VisIter/VisBuffer
  VisIter& vi(localVS.iter());
  VisBuffer vb(vi);

  Vector<Int> nSlot(nSpw(),0);

  // Find out how many solutions we have
  for (vi.originChunks(); vi.moreChunks();vi.nextChunk()) {
    // count number of slots per spw
    nSlot(vi.spectralWindow())++;
  }

  {
    LogMessage message(LogOrigin("SolvableVisJones2", "solve"));
    ostringstream o; 
    o<<"For interval of "<<interval()<<" seconds, found "<<
		       sum(nSlot)<<" slots";
    message.message(o);
    logsink.post(message);
  }

  // Sort out channel counting
  nChanDat()=localVS.numberChan();

  if (freqDepPar()) {
    nChanPar()  = nChanMat() = nChanDat();
    startChan() = localVS.startChan();
  } else {
    nChanPar()  = Vector<Int>(nSpw(),1);
    startChan() = Vector<Int>(nSpw(),0);
    if (freqDepMat())
      nChanMat()=nChanDat();
    else
      nChanMat()=1;
  }

  // Resize the Calset
  cs().resize(nPar(),nChanPar(),nElem(),nSlot);
  cs().setStartChan(startChan());

  // Obtain solution 
  Vector<Int> islot(nSpw(),-1);
  for (vi.originChunks(); 
       vi.moreChunks();
       vi.nextChunk()) {

    Int thisSpw;
    thisSpw= currSpw() = vi.spectralWindow();
    
    islot(thisSpw)++;

    // Set field source info
    cs().fieldId(currSpw())(islot(thisSpw))   =vi.fieldId();
    cs().fieldName(currSpw())(islot(thisSpw)) =vi.fieldName();
    cs().sourceName(currSpw())(islot(thisSpw))=vi.sourceName();

    // Set time info (get this from vb after initChiSquare?)
    vi.origin();
    cs().startTime(currSpw())(islot(thisSpw))  =vb.time()(0)-vb.timeInterval()(0)/2.0;
    Double timeStamp(0.0);
    Int ntime(0);
    for (vi.origin(); vi.more(); vi++,ntime++) {
      timeStamp+=vb.time()(0);
    }
    cs().stopTime(currSpw())(islot(thisSpw)) =vb.time()(0)+vb.timeInterval()(0)/2.0;
    cs().time(currSpw())(islot(thisSpw))=timeStamp/ntime;
    vi.origin();

    // Arrange VisEq, applying existing calibration relative to solved-for term
    //    lme.initChiSquare(*static_cast<SolvableVisCal*>(this) );
    lme.initChiSquare(*this);

    // The VB for solving  (****NEED lme.corrupted(), TOO!)
    //    const VisBuffer& svb(lme.corrected());

    // Prepare local space for solutions
    thisPar(currSpw()).resize(nPar(),nChanPar()(currSpw()),nElem());
    thisParOK(currSpw()).resize(nPar(),nElem());

 /*
    cout << "cs().nPar()     = " << cs().nPar() << endl;
    cout << "cs().nChan()    = " << cs().nChan() << endl;
    cout << "cs().nElem()    = " << cs().nElem() << endl;
    cout << "cs().nTime()    = " << cs().nTime() << endl;

    cout << "nBln()          = " << nBln() << endl;
    cout << "nAnt()          = " << nAnt() << endl;
    cout << "thisPar(currSpw()).shape() = " << thisPar(currSpw()).shape() << endl;

    cout << "cs().parOK(thisSpw).shape() = " << cs().parOK(thisSpw).shape() << endl;

    cout << "cvb.nRow() = " << cvb.nRow() << endl;
 */



    // CALL SOLVER WITH svb
    //solver.solve(svb);


  }

  //  {
  //    LogMessage message(LogOrigin("SolvableVisMueller", "solve"));
  //    ostringstream o; 
  //    o<<"  Found  "<<solcount<<" good " << typeName() << " solutions."<< endl
  //     <<"         "<<sum(nSlot)-solcount<< " solutions intervals failed, or had insufficient data." << endl;  
  //    logsink.post(message);
  //  }


  // Store it (no append yet)
  cs().store(calTableName(),typeName(),False);

  return True;
}


//  Vobs - (JxJ)*Vmod
/*
void SolvableVisJones2::residual(VisBuffer& vb,
				 Matrix<CStokesVector>& R) {

  // Shapes
  Int spw(vb.spectralWindow());
  Int nrow(vb.nRow());
  Int nchan(vb.nChannel());
  Double time(vb.time()(0));

  // Ensure Jones matrices up-to-date 
  if (!JMValid(spw)) syncJones(spw,time,-1,False);

  Int* ant1=&vb.antenna1()(0);
  Int* ant2=&vb.antenna2()(0);
  CStokesVector* vis=&vb.visibility()(0,0);
  CStokesVector* mod=&vb.modelVisibility()(0,0);


  IPosition ip0a(4,nPar(),nchan,2,nrow);

  R.resize(nchan,nrow);
  R=Complex(0.0);

  mMat mmat0;  // Utility Mueller matrix

  for (Int irow=0; irow<nrow; irow++,ant1++,ant2++) {
    for (Int ich=0; ich<nchan; ich++,mod++,vis++) {
      R(ich,irow) = (*vis);
      R(ich,irow)-= (directProduct(mmat0, 
				   thisJones(spw,ich,*ant1), 
				   conj(thisJones(spw,ich,*ant2))) * (*mod));
    }
  }

}
*/
void SolvableVisJones2::differentiate(VisBuffer& vb) {

  Int& spw(vb.spectralWindow());
  Double& time(vb.time()(0));

  if (!JMValid(spw)) syncJones(spw,time,-1,False);

  validateDJ(spw);

}
  
/*
void SolvableVisJones2::applyDerivative(VisBuffer& vb,
					PtrBlock<Array<CStokesVector>*>& dR) {

  Int spw(vb.spectralWindow());

  if (!DJValid(spw)) differentiate(vb);

  Int nrow(vb.nRow());
  Int nchan(vb.nChannel());

  Int* ant1=&vb.antenna1()(0);
  Int* ant2=&vb.antenna2()(0);
  CStokesVector* mod=&vb.modelVisibility()(0,0);


  IPosition ip0a(4,nPar(),nchan,2,nrow);
  dR[0]->resize(ip0a);
  (*dR[0])=Complex(0.0);

  IPosition ip1(4,nPar(),nPar(),nchan,nrow);
  dR[1]->resize(ip1);
  (*dR[1])=Complex(0.0);

  mMat mmat0;  // Utility Mueller matrix

  IPosition ip0b=ip0a;
  ip0a(2)=0; ip0b(2)=1;

  for (Int irow=0; irow<nrow; irow++,ant1++,ant2++) {
    ip0a(3)=ip0b(3)=ip1(3)=irow;

    for (Int ich=0; ich<nchan; ich++,mod++) {
      ip0a(1)=ip0b(1)=ip1(2)=ich;
      for (Int ipar=0; ipar<nPar(); ipar++) {
	ip0a(0)=ip0b(0)=ip1(1)=ipar;
	(*dR[0])(ip0a) = directProduct(mmat0, dJ(ipar,ich,*ant1),             conj(thisJones(currSpw(),ich,*ant2))) * (*mod);
	(*dR[0])(ip0b) = directProduct(mmat0, thisJones(currSpw(),ich,*ant1), conj(dJ(ipar,ich,*ant2))) * (*mod);

	for (Int ipar2=0;ipar<nPar(); ipar++) {
	  ip1(0)=ipar2;
	  (*dR[1])(ip1) = directProduct(mmat0,dJ(*ant1,ich,ipar),conj(dJ(*ant2,ich,ipar2))) * (*mod);
	}
	
      }
    }
  }

}
*/
void SolvableVisJones2::addGain(const Cube<Complex> gaininc ) {

  // Update current gains with supplied increment
  thisPar(currSpw())+=gaininc;

  // All derived matrices are now invalid
  invalidateDJ(currSpw());
  invalidateJM(currSpw());
  invalidateMM(currSpw());

}


// **********************************************************
//  MMueller Implementations
//

MMueller::MMueller(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  SolvableVisMueller(vs) 
{
  //  cout << "MMueller ctor " << endl;

  nPar()=4;
}

void MMueller::setSolve(const Record& solve) 
{
  // Call parent
  SolvableVisMueller::setSolve(solve);

  // Force preavg to infinity
  preavg()=DBL_MAX;

}


// Specific per-vb solve mechanism is just a copy of the data
Bool MMueller::solvevb(const Int& slot) {
  
  //  cout << "M::solvevb" << endl;

  // Reference CalSet slot
  Cube<Complex> thisPar;
  IPosition blc(4,0,0,0,slot);
  IPosition trc(4,nPar()-1,nChanPar()(currSpw())-1,nElem()-1,slot);
  
  thisPar.reference(cs().par(currSpw())(blc,trc).nonDegenerate(3));
  
  Int rowok(0);
  for (Int irow=0;irow<svb().nRow();irow++) {
    
    if (!svb().flagRow()(irow)) {
      Int bl(blnidx(svb().antenna1()(irow),svb().antenna2()(irow)));
      
      Int chok(0);
      for (Int ichan=0;ichan<svb().nChannel();ichan++) {
	thisPar.xyPlane(bl).column(ichan) = 
	  svb().visibility()(ichan,irow).vector();
	
	cs().parOK(currSpw()).xyPlane(slot).column(bl)(ichan) =
	  !svb().flag()(ichan,irow);
	
	if (!svb().flag()(ichan,irow)) chok++;
	
      }
      // If more then one channel good, then this row is good
      if (chok>0) {
	cs().iSolutionOK(currSpw())(irow,slot)=True;
	rowok++;
      }
      
    }
  }

  // If more than one row is good, then this chunk is good
  if (rowok>0) {
    cs().solutionOK(currSpw())(slot)=True;
    return True;
  } else {
    cs().solutionOK(currSpw())(slot)=False;
    return False;
  }
}


// **********************************************************
//  MfMueller Implementations
//

MfMueller::MfMueller(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  MMueller(vs) {

  //  cout << "MfMueller ctor " << endl;

  nPar()=4;
}


// **********************************************************
//  KMueller Implementations
//

KMueller::KMueller(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  SolvableVisMueller(vs) 
{
  //    cout << "KMueller ctor " << endl;
  
  timeDepMat()=True;
  nPar()=5;
}

void KMueller::setSolve(const Record& solve) 
{
  // Call parent
  SolvableVisMueller::setSolve(solve);

  // Force no preaveraging
  preavg()=0.0;

}

void KMueller::calcMueller(mMat& mat, Vector<Complex>& par, 
			   const Int& baseline, const Int& channel ) {

  //  cout << "KMueller::calcMueller" << endl;

  // Fill matrix diagonal with par  (mimic MMueller)
  //  mat=par;

  // phase and delay
  Vector<Complex> diag(4,0.0);
  Double& freq(currFreq()(channel));
  Double twopidf(2.0*C::pi*(freq-refFreq()));
  Double twopifdt( 2.0*C::pi*freq*(currTime()-refTime())/1000.0 );

  //  diag(0)=par(0)*pow(par(1),freq);
  diag(0)=par(0);
  diag(0)*=exp( Complex( 0.0,twopidf*real(par(1)) ) );
  diag(0)*=exp( Complex( 0.0,twopifdt*real(par(4)) ) );

  diag(1)=diag(2)=1.0;

  //  diag(3)=par(2)*pow(par(3),freq);
  diag(3)=par(2);
  diag(3)*=exp( Complex( 0.0,twopidf*real(par(3)) ) );
  diag(3)*=exp( Complex( 0.0,twopifdt*real(par(4)) ) );

  // Fill Mueller
  mat=diag;
  
  //  cout << " mat = " << mat.matrix() << endl;

}

void KMueller::calcDiffMueller(Vector<mMat>& mat, Vector<Complex>& par, 
			       const Int& baseline, const Int& channel ) {

  //  cout << "KMueller::calcDiffMueller" << endl;

  // Mimic MMueller
  //  Vector<Complex> diag(4,0.0);
  //  for (Int ipar=0;ipar<nPar();ipar++) {
  //    diag=0.0;
  //   diag(ipar)=1.0;
  //   mat(ipar)=diag;
  //  }

  Vector<Complex> diag(4,0.0);
  Double& freq(currFreq()(channel));
  Double twopidf(2.0*C::pi*(freq-refFreq()));
  Double twopifdt( 2.0*C::pi*freq*(currTime()-refTime())*1e-3 );

  // A derivative matrix for each par
  for (Int ipar=0;ipar<nPar();ipar++) {
    diag=0.0;
    if (ipar==0) {
      diag(0)=exp(Complex(0.0,(twopidf*real(par(1))+twopifdt*real(par(4))) ));

    } else if (ipar==1) {
      diag(0)=par(0);
      diag(0)*=Complex(0.0,twopidf);
      diag(0)*=exp(Complex(0.0,(twopidf*real(par(1))+twopifdt*real(par(4))) ));

    } else if (ipar==2) {
      diag(3)=exp(Complex(0.0,(twopidf*real(par(3))+twopifdt*real(par(4))) ));

    } else if (ipar==3) {
      diag(3)=par(2);
      diag(3)*=Complex(0.0,twopidf);
      diag(3)*=exp(Complex(0.0,(twopidf*real(par(3))+twopifdt*real(par(4))) ));

    } else if (ipar==4) {
      diag(0)=par(0);
      diag(0)*=Complex(0.0,twopifdt);
      diag(0)*=exp(Complex(0.0,(twopidf*real(par(1))+twopifdt*real(par(4))) ));
      diag(3)=par(2);
      diag(3)*=Complex(0.0,twopifdt);
      diag(3)*=exp(Complex(0.0,(twopidf*real(par(3))+twopifdt*real(par(4))) ));
    }
    mat(ipar)=diag;

  }

}

// Specific per-vb solve mechanism 
Bool KMueller::solvevb(const Int& slot) {

  //  cout << "K::solvevb" << endl;
  
  // Use local space for current solution (will copy into CalSet later)
  thisPar(currSpw()).resize(nPar(),nChanPar()(currSpw()),nElem());
  thisParOK(currSpw()).resize(nChanPar()(currSpw()),nElem());
  
  // If first slot, initialize to (1,0) (otherwise will use previous)
  //  if (slot==0) thisPar(currSpw())=1.0;
  if (slot==0) {
    thisPar(currSpw())=1.0;
    thisPar(currSpw())(IPosition(3,1,0,0),IPosition(3,1,nChanPar()(currSpw())-1,nElem()-1))=0.0;
    thisPar(currSpw())(IPosition(3,3,0,0),IPosition(3,3,nChanPar()(currSpw())-1,nElem()-1))=0.0;
    thisPar(currSpw())(IPosition(3,4,0,0),IPosition(3,4,nChanPar()(currSpw())-1,nElem()-1))=0.0;
  }

  initSolve();

  thisParOK(currSpw())=True; //   False;

  // Get current chi2 for all baselines
  chiSquare();

  for (Int ielm=0;ielm<nElem();ielm++) {
    if (sumWt()(ielm)>0.0) {
      thisParOK(currSpw()).column(ielm)=True;
    }
  }
  
  // Attempt soluion if we have some data
  if (sum(sumWt())>0.0) {

    tolerance()=0.01;
    almostconverged()=False;
    
    // Until converged, iterate on solution
    Int iter=0;
    //    cout << "iter = " << iter << endl;

    Bool cvrgd=converged(iter);
    while (!cvrgd && iter<10) {
      
      iter++;
      
      calcGradHess();

      solveGradHess();

      updgain();

   /*
      Vector<Complex> par;
      par.reference(thisPar(currSpw()).xyPlane(1).column(0));

      cout << iter 
	   << "  thisPar() = " 
	   << arg(par(0))*180.0/C::pi << " "
	   << real(par(1)) << " "   
	   << arg(par(2))*180.0/C::pi << " "
	   << real(par(3)) << " "   	   
	   << real(par(4)) << " "   
	   << endl;
   */
      chiSquare();

      //      cout << "iter = " << iter << endl;
      
      cvrgd=converged(iter);
    }

    if (cvrgd) {
      Int ngood=0;
      for (Int ielm=0;ielm<nElem();ielm++) {
	if (sumWt()(ielm)>0.0) {
	  cs().iSolutionOK(currSpw())(ielm,slot)=True;
	  ngood++;
	}
      }
      if (ngood>0) {
	cs().solutionOK(currSpw())(slot)=True;
	IPosition blc(4,0,0,0,slot);
	IPosition trc(4,nPar()-1,nChanPar()(currSpw())-1,nElem()-1,slot);
	cs().par(currSpw())(blc,trc).nonDegenerate(3) = thisPar(currSpw());
      }
    
      if (ngood>0) return True;
    }
    
  }

  // Something went wrong...
  return False;

}

void KMueller::updgain() {
  // Assumes freqDepPar()=F

  //  cout << "K::updgain" << endl;

  // pars 1 & 3,4 are real part only
  Vector<Complex> rp;

  rp=dg().row(1);
  rp=rp+conj(rp);
  rp=rp/2.0;
  dg().row(1)=rp;

  rp=dg().row(3);
  rp=rp+conj(rp);
  rp=rp/2.0;
  dg().row(3)=rp;

  rp=dg().row(4);
  rp=rp+conj(rp);
  rp=rp/2.0;
  dg().row(4)=rp;

  // Call parent
  SolvableVisMueller::updgain();

}


// **********************************************************
//  UVpMod Implementations
//

UVpMod::UVpMod(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  SolvableVisMueller(vs),
  nElem_(1)
{
  //  cout << "UVpMod ctor " << endl;

  nHess() = 1;
  timeDepMat()=True;
  nPar()=1;
}

void UVpMod::setSolve(const Record& solve) 
{

  //  cout << "UVpMod::setSolve" << endl;

  // Call parent
  SolvableVisMueller::setSolve(solve);

  // Force no preaveraging
  preavg()=0.0;

}

// Specific per-vb solve mechanism 
Bool UVpMod::solvevb(const Int& slot) {

  //  cout << "UVpMod::solvevb" << endl;
  
  // Use local space for current solution 
  // NB: Only one solution (not per bl or ant)
  thisPar(currSpw()).resize(nPar(),nChanPar()(currSpw()),nElem());
  thisParOK(currSpw()).resize(nChanPar()(currSpw()),nElem());
  
  // Initialize to (1,0) (each slot is different field and/or spw)
  thisPar(currSpw())(0,0,0)=1.0;

  initSolve();

  thisParOK(currSpw())=True; //   False;

  // Get current chi2 for all baselines
  chiSquare();

  for (Int ielm=0;ielm<nElem();ielm++) {
    if (sumWt()(ielm)>0.0) {
      thisParOK(currSpw()).column(ielm)=True;
    }
  }
  
  // Attempt soluion if we have some data
  if (sum(sumWt())>0.0) {

    tolerance()=0.01;
    almostconverged()=False;
    
    // Until converged, iterate on solution
    Int iter=0;
    //    cout << "iter = " << iter << endl;
    
    cout << iter 
	 << "  thisPar() = " 
	 << real(thisPar(currSpw())(0,0,0))
	 << "  chiSq = " 
	 << chiSq()
	 << endl;
    
    Bool cvrgd=converged(iter);
    while (!cvrgd && iter<10) {
      
      iter++;
      
      //      cout << "********************************************************************" << endl;

      calcGradHess();

      solveGradHess();

      updgain();

      chiSquare();


      cout << iter 
	   << "  thisPar() = " 
	   << real(thisPar(currSpw())(0,0,0))
	   << "  chiSq = " 
	   << chiSq()
	   << endl;

      
      cvrgd=converged(iter);
    }

    //    cout << "Finished solving!" << endl;

    if (cvrgd) {
      Int ngood=0;
      for (Int ibl=0;ibl<nElem();ibl++) {
	if (sumWt()(ibl)>0.0) {
	  cs().iSolutionOK(currSpw())(ibl,slot)=True;
	  ngood++;
	}
      }
      if (ngood>0) {
	cs().solutionOK(currSpw())(slot)=True;
	IPosition blc(4,0,0,0,slot);
	IPosition trc(4,nPar()-1,nChanPar()(currSpw())-1,nElem()-1,slot);
	cs().par(currSpw())(blc,trc).nonDegenerate(3) = thisPar(currSpw());
      }
    
      //      cout << "ngood = " << ngood << endl;

      if (ngood>0) return True;
    }
    
  }

  // Something went wrong...
  return False;

}

void UVpMod::syncDiffMueller(const Int& row) {

  //  cout << "UVpMod::syncDiffMueller" << endl;
  // Need to synchronize the uvw values for this type
  syncUVW(row);

  // Now do usual synch of M & dM
  SolvableVisMueller::syncDiffMueller(row);

}


void UVpMod::syncUVW(const Int& row) {

  //  cout << "UVpMod::syncUVW " << row << endl;

  // Ensure correct size, initialize
  thisUV().resize(2,nBln());
  thisUV()=0.0;
  
  // Access uvw in VB, per baseline
  Int* a1 = &svb().antenna1()(row);
  Int* a2 = &svb().antenna2()(row);
  RigidVector<Double,3> *uvw = &svb().uvw()(row);

  // First baseline
  Int bl(blnidx(*a1,*a2)),lastbl(-1);

  // While bl index increases (OR SHOULD WE WATCH UNFLAGGED TIMES?)
  Int irow=row;
  while (bl>lastbl) {

    // copy u & v to work space
    thisUV()(0,bl)=(*uvw)(0);
    thisUV()(1,bl)=(*uvw)(1);
   
    lastbl=bl;

    irow++;

    if (irow<svb().nRow()) {
      // Advance pointers
      a1++; a2++; uvw++;
      bl=blnidx(*a1,*a2);
    } else 
      break;
    
  }

  //  cout << "thisUV() = " << thisUV() << endl;
  //  cout << "refFreq() = " << refFreq() << endl;
  //  cout << "mult = " << (refFreq()*1.0e9*C::pi/648000.0/C::c) << endl;


  // convert to cycles/arcsec
  thisUV()*=(refFreq()*1.0e9*C::pi/648000.0/C::c);

  //  cout << "thisUV() = " << thisUV() << endl;

}



void UVpMod::calcMueller(mMat& mat, Vector<Complex>& par, 
			 const Int& baseline, const Int& channel ) {

  //  cout << "UVpMod::calcMueller" << endl;

  // working matrix
  Matrix<Complex> M(4,4); 
  M=0.0;

  // Fill corners with f.d./2
  M(0,0)=M(0,3)=M(3,0)=M(3,3)=par(0)/2.0;

  // copy to SquareMatrix
  mat=M;
  
  //  cout << " mat = " << mat.matrix() << endl;

}

void UVpMod::calcDiffMueller(Vector<mMat>& mat, Vector<Complex>& par, 
			     const Int& baseline, const Int& channel ) {

  //  cout << "UVpMod::calcDiffMueller" << endl;

  // working matrix
  Matrix<Complex> dM(4,4); 
  dM=0.0;

  // Fill corners with 0.5
  dM(0,0)=dM(0,3)=dM(3,0)=dM(3,3)=0.5;

  // copy to SquareMatrix
  mat(0)=dM;
  
  //    if (channel==1) cout << "par = " << ipar << " dM = " << diag << endl;

}




// **********************************************************
//  TJones (new) Implementations
//

newTJones::newTJones(VisSet& vs) :
  VisCal(vs), VisMueller(vs),
  SolvableVisJones2(vs) 
{
  //  cout << "newTJones ctor " << endl;

  nPar()=1;
}

// **********************************************************
//  KJones Implementations
//

/*

KJones::KJones(VisSet& vs) :
  VisCal(vs), VisJones(vs),
  SolvableVisJones2(vs) 
{
  //  cout << "KJones ctor " << endl;

  nPar()=4;
}



KJones::differentiate(const VisBuffer& vb) {

  Int spw(vb.spectralWindow());
  if (!JMValid(spw)) syncJones(spw);

  for (Int iant=0;iant<nAnt();iant++) {
    for (Int ich=0;ich<nChan();ich++) {

      dJ()(0,ich,iant) = 
      dJ()(1,ich,iant) = 
      dJ()(2,ich,iant) = 
      dJ()(3,ich,iant) = 
      dJ()(4,ich,iant) = 



  validateDJ();

}

*/

} //# NAMESPACE CASA - END

