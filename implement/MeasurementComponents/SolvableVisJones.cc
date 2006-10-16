//# SolvableVisJones.cc: Implementation of Jones classes
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

#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <calibration/CalTables/SolvableVJDesc.h>
#include <calibration/CalTables/SolvableVJMRec.h>
#include <calibration/CalTables/SolvableVJMCol.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/CalDescColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <msvis/MSVis/VisBuffer.h>

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

#include <ms/MeasurementSets/MSHistoryHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for the solveable version i.e. SolvableVisJones

// Generic ctor
SolvableVisJones::SolvableVisJones(VisSet& vs)
  : TimeVarVisJones(vs)
{};

SolvableVisJones::SolvableVisJones(const SolvableVisJones& other)
{
  //  operator=(other);
}

SolvableVisJones::~SolvableVisJones() {
  deleteSolveCache();
}


#undef ASSIGN
#ifdef ASSIGN
// Assignment
SolvableVisJones& SolvableVisJones::operator=(const SolvableVisJones& other) {
  if(this!=&other) {
    TimeVarVisJones::operator=(other);
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
  }
  return *this;
}
#endif


void SolvableVisJones::setInterpolation(const Record& interpolation)
{
// Set up interpolation of applied types (solvable)
//  Inputs:
//    interpolation           Record&       Contains application params
//  Inputs from private/protected data
//    

  LogMessage message(LogOrigin("SolvableVisJones","setInterpolation"));

  String table,select,interp("nearest");
  Double interval;
  Vector<Int> spwmap;

  // Collect parameters
  if (interpolation.isDefined("table")) 
    table=interpolation.asString("table");
  if (interpolation.isDefined("select"))
    select=interpolation.asString("select");
  if (interpolation.isDefined("t"))
    interval=interpolation.asFloat("t");

  setInterpolationParam(table,select,interval);

  if (interpolation.isDefined("interp")) 
    interp=interpolation.asString("interp");
  if (interpolation.isDefined("spwmap")) 
    spwmap=interpolation.asArrayInt("spwmap");

  initInterp(interp,spwmap);

}

void SolvableVisJones::setInterpolationParam(const String& table,
					     const String& select,
					     const Double& interval) {

  calTableName_=table;
  calSelect_=select;
  interval_=interval;

  // TODO:
  //  Need to make a "matrixType" data member to 
  //  store matrix type = "general","diagonal","scalar"

  // Load the table from disk
  String matType("diagonal");
  if (typeName()=="D Jones") matType="general";
  if (typeName()=="T Jones") matType="scalar";

  load(calTableName_,calSelect_,matType);

  // Setup Gain Cache
  initThisGain();

  // This is apply context
  setSolved(False);
  setApplied(True);

}

void SolvableVisJones::setSolver(const Record& solver) 
{ 
//  Setup solve context 
//  Inputs:
//    solver            Record&       Contains solve params
//  Inputs from private/protected data
//    

  Double interval(0.0),preavg(0.0);
  Int refant(-1);
  Bool phaseonly(False);
  String table("");
   
  // Collect parameters
  if (solver.isDefined("t"))
    interval=solver.asFloat("t");
  if (solver.isDefined("preavg"))
    preavg=solver.asFloat("preavg");
  if (solver.isDefined("refant")) 
    refant=solver.asInt("refant");
  if (solver.isDefined("phaseonly"))
    phaseonly=solver.asBool("phaseonly");
  if (solver.isDefined("table")) 
    table=solver.asString("table");

  setSolverParam(table,interval,preavg,refant,phaseonly);

}

void SolvableVisJones::setSolverParam(const String& table,
				      const Double& interval,
				      const Double preavg,
				      const Int refant,
				      const Bool phaseonly)
{ 

  interval_=interval;
  preavg_=preavg;
  refant_=refant;
  if (phaseonly) setMode("phase");
  calTableName_=table;

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
  

void SolvableVisJones::setAccumulate(const String& table,
				     const String& select,
				     const Double& interval,
				     const Int& refant) {

  LogMessage message(LogOrigin("SolvableVisJones","setAccumulate"));


  // If interval<0, this signals a valid existing input cumulative table
  if (interval<0.0) {

    ostringstream o;
    o << "Loading existing " << typeName() 
      << " table: " << table 
      << " for accumulation.";
    message.message(o);
    logSink().post(message);

    // Load whole thing as if applying this VJ
    //  (this generates thisJonesMueller, which is not needed!)
    setInterpolationParam(table,"",0.0);
    // Not actually applying
    setApplied(False);
  } 

  // else, we are creating a cumulative table from scratch
  else {

    ostringstream o;
    o << "Creating " << typeName() 
      << " table for accumulation.";
    message.message(o);
    logSink().post(message);

    // Intialize as if solving for this VJ
    //  (this generates thisJonesMueller, which is not needed!)
    setSolverParam(table,interval,interval,refant);
    // Not actually solving
    setSolved(False);

    // Solutions are all nominal, all good:
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (iSolutionOK_[ispw]) {

	(*iSolutionOK_[ispw]) = True;
	(*iFit_[ispw])        = 0.0;
	(*iFitwt_[ispw])      = 1.0;
	(*solutionOK_[ispw])  = True;
	(*fit_[ispw])         = 0.0;
	(*fitwt_[ispw])       = 1.0;
      }
    }

  }
}


// Initialize solve-related caches
void SolvableVisJones::initSolveCache() {

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
  for (Int ispw=0; ispw<numberSpw_; ispw++) {
    uInt nslot=numberSlot_(ispw);
    if (nslot > 0) {
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

void SolvableVisJones::deleteSolveCache() {

  uInt nCache=iSolutionOK_.nelements();
  if (nCache > 0) {
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (newPB_(ispw)) {
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


// Apply the gradient. The number of multiplications can be
// reduced immensely by being a little smart here.
// the following reduces the indexing time, but makes the code
// pretty unreadable. It's safe, because visbuffer arrays are guaranteed simple:
// zero origin, no stride
VisBuffer& SolvableVisJones::applyGradient(VisBuffer& vb, 
					   const Vector<Int>& antenna,
					   Int i, Int j, Int pos) {
  Int spw=vb.spectralWindow();
  Int nSolnChan=nSolnChan_(spw);
  Int startChan=startChan_(spw);
  Int nRow=vb.nRow();
  Int nDataChan=vb.nChannel();

  RigidVector<Complex,2> vec;
  Bool polSwitch=polznSwitch(vb);
  Bool* flagRowp=&vb.flagRow()(0);
  Double* timep=&vb.time()(0);
  const Int* antp=&antenna(0);
  Bool* flagp=&vb.flag()(0,0);
  CStokesVector* visp=&vb.visibility()(0,0);
  Int* dataChan;

  Bool* antOKp= &((*thisJonesOK_[spw])(0));

  for (Int row=0; row<nRow; row++,flagRowp++,antp++,timep++) {
    if (!*flagRowp) {
      if(antOKp[*antp]) {

	mjJones2* gainp=&((*thisJonesMat_[spw])(0,*antp));
	dataChan=&vb.channel()(0);

	for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++,dataChan++) {
	  // inc soln ch axis if freq-dependent (and dataChan within soln range)
	  if (freqDep() &&      
	      ((*dataChan)>startChan && (*dataChan)<(startChan+nSolnChan)) ) gainp++;  
	  // if this data channel unflagged
	  if (!*flagp) {

	    mjJones2 ag=(*gainp);

	    if (polSwitch) polznMap(*visp);
	    CStokesVector& res=*visp;
	    if (pos==0) {
	      ag.conj();
	      vec(0)=res(2*j);
	      vec(1)=res(2*j+1);
	      vec*= ag;
	      //	      vec*= (*gainp);
	      res=Complex(0.0);
	      res(2*i)  =vec(0);
	      res(2*i+1)=vec(1);
	    } else {
	      vec(0)=res(j);
	      vec(1)=res(j+2);
	      vec*= ag;
	      //	      vec*= (*gainp);
	      res=Complex(0.0);
	      res(i)  =vec(0);
	      res(i+2)=vec(1);
	    }
	    if (polSwitch) polznUnmap(*visp);
	  }
	}

      }
      else {
        vb.flagRow()(row)=True;
      }
    }
    else {
      flagp+=nDataChan; visp+=nDataChan;
    }
  }
  return vb;
}

// resize and initialize 1st/2nd derivatives
void SolvableVisJones::initializeGradients() {

  gS_.resize(nSolnChan_(currentSpw_),numberAnt_);
  Matrix<Complex> gS(2,2); gS=Complex(0.,0.);
  gS_=gS;

  ggS_.resize(nSolnChan_(currentSpw_),numberAnt_);
  Matrix<Float> ggS(2,2); ggS=0.0;
  ggS_=ggS;

  sumwt_=0.0;
  chisq_=0.0;

}

void SolvableVisJones::finalizeGradients() {}

// Add to Gradient Chisq
void SolvableVisJones::addGradients(const VisBuffer& vb, Int row,
				    const Antenna& a,
				    const Vector<Float>& sumwt,
				    const Vector<Float>& chisq,
				    const Vector<SquareMatrix<Complex,2> >& c, 
				    const Vector<SquareMatrix<Float,2> >& f) {
  
  if (!vb.flagRow()(row)) {
    for (Int chn=0; chn<vb.nChannel(); chn++) {
      if (!vb.flag()(chn,row)) {
	sumwt_+=sumwt(chn);
	chisq_+=chisq(chn);
	gS_(chn,a)+=c(chn);
	ggS_(chn,a)+=f(chn);
      }
    }
    //    cout << " addGradients: sumwt_ = " << sumwt_ << " " << sumwt(0) << " " << vb.flag()(0,0) << endl;
  }
}

// Solve for the  Jones matrix. Updates the VisJones thus found.
// Also inserts it into the VisEquation thus it is not const.
Bool SolvableVisJones::solve (VisEquation& me)
{
  LogMessage message(LogOrigin("SolvableVisJones","solve"));


  // Set the fit tolerance  (convergence is when change in
  //  chisq is less than tolerance times the value of chisq)
  // Force B and D solutions to go deeper, just to make sure...
  if (typeName()=="B Jones" || typeName()=="D Jones") {
    setTolerance(0.001);
  } else {
    setTolerance(0.1);
  }

  // Save required_, we might change it on-the-fly (single-corr data)
  Matrix<Bool> origreq(required_);

  AlwaysAssert(gain()>0.0,AipsError);
  AlwaysAssert(numberIterations()>0,AipsError);
  AlwaysAssert(tolerance()>0.0,AipsError);

  // Make a local copy of the Measurement Equation so we can change
  // some of the entries
  VisEquation lme(me);

  // Set the Jones matrix in the local copy of the Measurement Equation 
  //  lme.setVisJones(*this);
  
  // Tell the VisEquation to use the internal (chunked) VisSet
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

    // Get amplitude guess from data
    if (type() == VisJones::G || type() == VisJones::T) {
      if (lastslot(currentSpw_)==-1 || fld!=lastfld) {

	// Set an initial value for the G gain solutions (thisJonesMat_)
	// based on the modulus of the "corrected" data from
	// initChiSquare (which is really corrected-data/corrupted-model)
	// for these types

	// After first slot (each spw, fld), use last solution 
	//  (that is what will be in thisJonesMat_ already)

	const VisBuffer& cvb(lme.corrected());
	Bool polSwitch=polznSwitch(cvb);
	Int nVis(0);
	Double factor(0.0);
	for (Int irow=0; irow<cvb.nRow(); irow++) {
	  // Avoid ACs:
	  if (cvb.antenna1()(irow) != cvb.antenna2()(irow)) { 
	    for (Int ichan=0; ichan<cvb.nChannel(); ichan++) {
	      if (!cvb.flag()(ichan,irow)) {
		CStokesVector vis=cvb.visibility()(ichan,irow);
		if (polSwitch) polznMap(vis);
		factor+=(abs(vis(0)) + abs(vis(3)));
		nVis+=2;
	      };
	    };
	  };
	}; 
        factor=sqrt(factor/Double(nVis));
	// cout << "factor=" << factor << endl;
	if (factor > 0.0) {
	  for (Int iant=0; iant<numberAnt_; iant++) {
	    for (Int i=0;i<2;i++) {
	      for (Int j=0;j<2;j++) {
		if (required_(i,j) > 0.0) {
		  (*thisJonesMat_[currentSpw_])(0,iant)(i,j) *= factor;
		}
	      }
	    }
	  };
	};
      }
    }


    
    // check visbuffer
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
    invalidateMM(currentSpw_);

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

    if(sumwt_==0) {
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
    } else if (currentChisq==0) {
      ostringstream o; o<<typeName()<<"    Current Chisq is already zero";
      message.message(o);
      logSink().post(message);
      
      //break;
    } else {
      AlwaysAssert(currentChisq>0.0, AipsError); 
      AlwaysAssert(sumwt_>0.0, AipsError); 
      rms = sqrt(currentChisq/sumwt_);

      // Avoiding per-solution log messages for now; will add verbose option later
      //      ostringstream o; o<<typeName()<<"    Initial fit per unit weight = "<<rms<<
      //		      " Jy, sum of weights = "<<sumwt_;
      //      message.message(o);
      //      logSink().post(message);

      // Don't need to call chiSquared here, it is done later...........
      // Determine if there is any data for each antenna. This is
      // used in the gradient calculation.
      //      lme.chiSquared(slotFit, slotFitwt);
      //      for (Int iant=0;iant<numberAnt_;iant++) {
      //	iSolutionOK_(iant, currentSpw_, currentSlot_)= 
      //              (iFitwt_(iant, currentSpw_, currentSlot_)>0.0);
      //      };

      // Iterate    
      Int iter;
      Bool converged(False);
      Bool almostconverged(False);
      for (iter=0;(!fail)&&(iter<numberIterations());iter++) {

	// Update antenna gains from gradients. This can be different for
	// derived classes
	updateAntGain();

	// Invalidate baseline cache (because antenna solutions just changed)
	validateJM(currentSpw_);
	invalidateMM(currentSpw_);

	// Find gradient and Hessian
	initializeGradients();
	lme.gradientsChiSquared(required_,*this);

	// Assess fit
	previousChisq=currentChisq;
	currentChisq=chisq_;
	lowestChisq=min(lowestChisq, currentChisq);

	AlwaysAssert(sumwt_>0.0, AipsError);
	AlwaysAssert(chisq_>=0.0, AipsError);
	rms = sqrt(chisq_/sumwt_);
      
	// Converged?
	if(currentChisq<=previousChisq && 
	   //	   iter > 20 &&
	   abs(currentChisq-previousChisq)<tolerance()*currentChisq) {
          if (almostconverged) {
	    converged=True;
	    break;
	  };
	  almostconverged=True;
	  //	  cout << " Almost converged!";
	} else {
	  almostconverged=False;
        };

	// Diverging? We need a better way of identifying divergence.
	if(iter>(numberIterations()/2)) {
	  if ((currentChisq>originalChisq)&&(currentChisq>previousChisq)) {
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
	invalidateJM(currentSpw_);
	invalidateMM(currentSpw_);

	// force from-scratch guess from data on next time around
	lastslot(currentSpw_)=-1;
        lastfld=-1;

      } else {
	// Solution is ok, keep it (by copying into jonesPar array, which will
	//  get written out to the cal table
	keep(currentSpw_,currentSlot_(currentSpw_));
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
  reReference(refant());

  // Restore required_ to nominal value
  required_=origreq;

  //  setSolved(False);
  //  setApplied(True);

  return(True);
  
}

// Update formula for general matrices
void SolvableVisJones::updateAntGain() {
  
  Bool phaseOnly=(mode().contains("phase"));

  Double dist=0.0;

  for (Int iant=0;iant<numberAnt_;iant++) {
    for (Int ichan=0;ichan<nSolnChan_(currentSpw_);ichan++) {
      for (Int i=0;i<2;i++) {
	for (Int j=0;j<2;j++) {
	  if(required_(i,j)&&ggS_(ichan,iant)(i,j)>0.0) {
	    Complex& g=(*thisJonesMat_[currentSpw_])(ichan,iant)(i,j);
	    Complex gup;
	    gup=gain()*gS_(ichan,iant)(i,j)/ggS_(ichan,iant)(i,j);
	    dist+=norm(gup);
	    
	    g-=gup;

	    // Handle phaseOnly case (is this necessary, if data is phase-only?)
	    if (phaseOnly) {
	      Float gamp=abs(g);
	      if (gamp>0.0) g/=gamp;
	    }

	  }
	}
      }
    }
  }

  //  cout << "dist = " << sqrt(dist) << endl;

}


// Method for copying solved-for matrix into "parameterized" storage
//  (eventually, we'll use the parameterized storage directly in solve)

void SolvableVisJones::keep(const Int& spw,
			    const Int& slot) {

  // Assume params are the matrix elements, and are in pol order
  // This version works for diagonal (nPar_=2) and scalar (nPar_=1) matrices

  // Loop over antenna, channel and parameter
  for (Int iant=0; iant<numberAnt_; iant++) {
    for (Int ichan=0; ichan<nSolnChan_(spw); ichan++) {
      for (Int ipar=0; ipar<nPar_;ipar++) {
	(*jonesPar_[spw])(IPosition(4,ipar,ichan,iant,slot))=
	  (*thisJonesMat_[spw])(ichan,iant)(ipar,ipar);
      }
    }
  }

}

// Re-reference the derived gain solutions
void SolvableVisJones::reReference(const Int& refAnt)
{
  if (refAnt >= 0) {

    IPosition blc,trc;
    IPosition refshape(2,nPar_,nSolnChan_(currentSpw_));
    Matrix<Complex> refphasor(refshape,Complex(1.0,0.0));

    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (jonesPar_[ispw]!=NULL) {

	IPosition ip(4,0,0,0,0);
	for (Int islot=0; islot<numberSlot_(ispw); islot++) {
	  ip(3)=islot;
	  // loop over all ants, chans, pars
	  for (Int iant=0; iant<numberAnt_; iant++) {
	    ip(2)=iant;
	    for (Int ichan=0; ichan<nSolnChan_(ispw); ichan++) {
	      ip(1)=ichan;
	      for (Int ipar=0; ipar<nPar_; ipar++) {
		ip(0)=ipar;

		// First time for this par/chan, normalize ref phasor
		//  (to reference to one of many pars, make refphasor 
                //   per-channel only move this if clause out of par loop.)
		if (iant==0) {
		  IPosition ref(4,ipar,ichan,refAnt,islot);
		  refphasor(ipar,ichan) = conj((*jonesPar_[ispw])(ref));
		  Float refamp=abs(refphasor(ipar,ichan));
		  if (refamp>0.0) 
		    refphasor(ipar,ichan) = refphasor(ipar,ichan)/refamp;
		}

		// Reference this ipar,ichan,iant,islot's gain parameter
		(*jonesPar_[ispw])(ip) *= refphasor(ipar,ichan);

	      } // ipar
	    } // ichan
	  } // iant
	} // islot
      } // jonesPar_[ispw]!=NULL
    } // ispw
  } // refAnt>=0
}


void SolvableVisJones::fluxscale(const Vector<Int>& refFieldIn, 
				 const Vector<Int>& tranFieldIn,
				 const Vector<Int>& inRefSpwMap,
				 Matrix<Double>& fluxScaleFactor) {

  // For updating the MS History Table
  LogSink logSink_p = LogSink(LogMessage::NORMAL, False);
  logSink_p.clearLocally();
  LogIO oss(LogOrigin("calibrater", "fluxscale()"), logSink_p);

  //  cout << "refFieldIn  = " << refFieldIn << endl;
  //  cout << "tranFieldIn = " << tranFieldIn << endl;

  // PtrBlocks to hold mean gain moduli and related
  PtrBlock< Matrix<Bool>* >   ANTOK;
  PtrBlock< Matrix<Double>* > MGNORM;
  PtrBlock< Matrix<Double>* > MGNORM2;
  PtrBlock< Matrix<Double>* > MGNWT;
  PtrBlock< Matrix<Double>* > MGNVAR;
  PtrBlock< Matrix<Int>* >    MGNN;

  Vector<String> fldNames;
  {  
    const ROMSColumns& mscol(vs_->iter().msColumns());
    const ROMSFieldColumns& fldcol(mscol.field());
    //    MeasurementSet ms(vs_->msName());
    //    MSFieldColumns msfld(ms.field());
    fldNames=fldcol.name().getColumn();
  }

  Int nMSFld; fldNames.shape(nMSFld);

  // assemble complete list of available fields
  Vector<Int> fldList;
  for (Int iSpw=0;iSpw<numberSpw_;iSpw++) {
    Int currlen;
    fldList.shape(currlen);
    //    cout << "iSpw = " << iSpw << " currlen = " << currlen << endl;
    if (fieldId_[iSpw]!=NULL) {

      //      cout << "(*fieldId_[iSpw]) = " << (*fieldId_[iSpw]) << endl;

      Vector<Int> thisFldList; thisFldList=(*fieldId_[iSpw]);
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
      oss << LogIO::WARN 
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

    // Use implicit transfer field list, or check for missing tran fields
    if (nTran==1 && tranField(0)<0) {
      tranField.reference(implTranField);
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
	oss << LogIO::WARN 
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
    oss << " Found reference field(s): " << refNames << LogIO::POST;
    String tranNames(fldNames(tranField(0)));
    for (Int iTran=1; iTran<nTran; iTran++) {
      tranNames+=" ";
      tranNames+=fldNames(tranField(iTran));
    }
    oss << " Found transfer field(s):  " << tranNames << LogIO::POST;
    
    //    cout << "fldList = " << fldList << endl;
    
    //    cout << "nFld = " << nFld << endl;
    

    // Handle spw referencing
    Vector<Int> refSpwMap;
    refSpwMap.resize(numberSpw_);
    indgen(refSpwMap);
    
    if (inRefSpwMap(0)>-1) {
      if (inRefSpwMap.nelements()==1) {
	refSpwMap=inRefSpwMap(0);
	oss << " All spectral windows will be referenced to spw=" << inRefSpwMap(0)+1 << LogIO::POST;
      } else {
	for (Int i=0; i<Int(inRefSpwMap.nelements()); i++) {
	  if (inRefSpwMap(i)>-1 && inRefSpwMap(i)!=i) {
	    refSpwMap(i)=inRefSpwMap(i);
	    oss << " Spw=" << i+1 << " will be referenced to spw=" << inRefSpwMap(i)+1 << LogIO::POST;
	  }
	}
      }
    }
    
    // Scale factor info
    fluxScaleFactor.resize(numberSpw_,nFld); fluxScaleFactor=-1.0;

    Matrix<Double> fluxScaleError(numberSpw_,nFld,-1.0);
    Matrix<Double> gainScaleFactor(numberSpw_,nFld,-1.0);
    Matrix<Bool> scaleOK(numberSpw_,nFld,False);
    
    Matrix<Double> fluxScaleRatio(numberSpw_,nFld,0.0);
    Matrix<Double> fluxScaleRerr(numberSpw_,nFld,0.0);
    
    // Field names for log messages
    Vector<String> fldname(nFld,"");
    
    //    cout << "Filling mgnorms....";
    
    // fill per-ant -fld, -spw  mean gain moduli
    for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
      if (jonesPar_[iSpw]!=NULL) {

	for (Int islot=0; islot<numberSlot_(iSpw); islot++) {
	  Int iFld=(*fieldId_[iSpw])(islot);
	  if (ANTOK[iFld]==NULL) {
	    // First time this field, allocate ant/spw matrices
	    ANTOK[iFld]   = new Matrix<Bool>(numberAnt_,numberSpw_,False);
	    MGNORM[iFld]  = new Matrix<Double>(numberAnt_,numberSpw_,0.0);
	    MGNORM2[iFld] = new Matrix<Double>(numberAnt_,numberSpw_,0.0);
	    MGNWT[iFld]   = new Matrix<Double>(numberAnt_,numberSpw_,0.0);
	    MGNVAR[iFld]  = new Matrix<Double>(numberAnt_,numberSpw_,0.0);
	    MGNN[iFld]    = new Matrix<Int>(numberAnt_,numberSpw_,0);
	    
	    // record name
	    fldname(iFld) = (*fieldName_[iSpw])(islot);
	  }
	  // References to PBs for syntactical convenience
	  Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
	  Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
	  Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
	  Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
	  Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));
      
	  for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
	    if ((*iSolutionOK_[iSpw])(iAnt,islot)) {
	      // a good solution, so accumulate
	      antOK(iAnt,iSpw)=True;
	      Double wt=(*iFitwt_[iSpw])(iAnt,islot);

	      for (Int ipar=0; ipar<nPar_; ipar++) {
		IPosition ip(4,ipar,0,iAnt,islot);
		Double gn=norm( (*jonesPar_[iSpw])(ip) );
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
      
      // Have data for this field?
      if (ANTOK[iFld]!=NULL) {
	
	// References to PBs for syntactical convenience
	Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
	Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
	Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
	Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
	Matrix<Double> mgnvar;  mgnvar.reference(*(MGNVAR[iFld]));
	Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));
	
	for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
	  for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
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
      
      for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
	
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
	  
	  for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
	    
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
	  
	  //	  cout << endl;
	  //	  cout << "Tran = " << sftran << " +/- " << sqrt(sftranvar) << endl;
	  //	  cout << "Ref  = " << sfref  << " +/- " << sqrt(sfrefvar) << endl;
	  
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
	    oss << " Flux density for " << fldname(tranidx) 
		<< " in SpW=" << iSpw;
	    if (refSpw!=iSpw) oss << " (ref SpW=" << refSpw << ")";
	    oss << " is: " << fluxScaleFactor(iSpw,tranidx)
		<< " +/- " << fluxScaleError(iSpw,tranidx)
		<< " (SNR = " << fluxScaleFactor(iSpw,tranidx)/fluxScaleError(iSpw,tranidx)
		<< ")"
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
	    oss << LogIO::WARN
		<< " Insufficient information to calculate scale factor for "
		<< fldname(tranidx)
		<< " in SpW="<< iSpw
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
    for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
      if (jonesPar_[iSpw]!=NULL) {
	for (Int islot=0; islot<numberSlot_[iSpw]; islot++) {
	  Int iFld=(*fieldId_[iSpw])(islot);
	  // If this is a tran fld and gainScaleFactor ok
	  if (scaleOK(iSpw,iFld)) {
	    for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
	      if ((*iSolutionOK_[iSpw])(iAnt,islot)) {
		// a good solution, so apply scaling to all pars
		for (Int ipar=0; ipar<nPar_; ipar++) {
		  IPosition ip(4,ipar,0,iAnt,islot);
		  (*jonesPar_[iSpw])(ip)*=gainScaleFactor(iSpw,iFld);
		}
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

 
void SolvableVisJones::accumulate(SolvableVisJones& incr, 
				  const Vector<Int>& fields) {

  Int nfield(fields.nelements());

  Bool fldok(True);
  Matrix<mjJones2> thisJM, incrJM;

  // For each spw
  for (Int ispw=0; ispw<numberSpw_; ispw++) {

    // For each slot in this 
    for (Int islot=0; islot<numberSlot_(ispw); islot++) {
      
      // Is current field among those we need to update?
      Int thisfield((*fieldId_[ispw])(islot));
      fldok = (nfield==0 || anyEQ(fields,thisfield));

      if (fldok) {
	  
	// Advance this Jones Matrix to the current slot
	thisJM.reference(syncJonesMat(ispw,islot));
	
	// Sync incr to current slot's timestamp
	Double time((*MJDTimeStamp_[ispw])(islot));
	incrJM.reference( (incr.syncJonesMat(ispw,time)) );
	
	
	// Do the multiplication each ant, chan
	for (Int iant=0; iant<numberAnt_; iant++) {
	  for (Int ichan=0; ichan<nSolnChan_(ispw); ichan++) {
	    thisJM(ichan,iant)*=incrJM(ichan,iant);
	  }

	  (*iSolutionOK_[ispw])(iant,islot)=True;
	  (*solutionOK_[ispw])(islot) = True;
	  (*iFit_[ispw])(iant,islot) = 0.0;
	  (*iFitwt_[ispw])(iant,islot) = 1.0;

	}

	(*fit_[ispw])(islot)=0.0;
	(*fitwt_[ispw])(islot)=1.0;


	// Re-package matrix elements into parameter array
	keep(ispw,islot);

      }
    }
  }
}



#undef SMOOTH
#ifdef SMOOTH

// Proxy methods to translate the signature to a generic type.
//
template <class T> inline T medianFilterProxy(const Array<T>& d){return median(d);}
template <class T> inline T meanFilterProxy(const Array<T>& d){return mean(d);}

//
// Filter the in{X,Y} using the filterType filter of time constant
// timeConstant. Time gap > scanGap marks the "scan" boundary and each
// scan is seperately filtered.  The filtered data is returned in
// out{X,Y}. timeConstant and scanGap(=300 by default) are in seconds.
//
void SolvableVisJones::slidingFilter(Vector<Double>& inX,  Vector<Array<Double> > &inY,
				     Vector<Bool> &inFlags,
				     Vector<Double>& outX, Vector<Array<Double> > &outY,
				     FilterType filterType, Float timeConstant, Float scanGap)
{
  Int N=inX.nelements(),nSlots=0;
  IPosition yShape = inY(0).shape();
  Vector<Int> scanMarkers;
  Double t0,t;
  uInt i;
  Double (*yFilter)(const Array<Double>&);
  
  if (N==0) return;
  //
  // Cache the user selection of the type of filter as an appropriate
  // function pointer.
  //
  // Using proxy inlined functions to convert the signature of various
  // filters into one generic type.
  //

  switch (filterType)
    {
    case slidingMean:   {yFilter = &::meanFilterProxy;break;}
    case slidingMedian: {yFilter = &::medianFilterProxy;break;}
    }

  //
  // Find the number of "scans" and scan boundary markers.  Look for
  // time jumps >=scanGap as a scan boundary marker.  A rather loose
  // defintion and hence hardly robust!
  // 
  Int ndx=0,start=0,stop=0;
  t0=inX(ndx);

  outX.resize(N);
  outY.resize(N);

  for (Int i=0;i<N;i++) outY(i).resize(yShape);

  start=stop=0;
  Bool windowSizeReached=False;
  while(start < N)
    {
      Vector<Double> tmpY;
      Int k;

      t0=inX(start);t=inX(stop);
//       cerr << "Start,Stop = " << start << " " << stop << " " 
// 	   << t0 << " " << t << " " << t-t0 << " " << windowSizeReached << endl;

      tmpY.resize(stop-start+1);
      
      for (Int ii=0;ii<2;ii++)
	for (Int jj=0;jj<2;jj++)
	  for (Int axis=0;axis<2;axis++)
	    {
	      k=0;
	      for (Int i=start;i<=stop;i++)
		{
		  if (inFlags(i))
		    tmpY(k++) = inY(i)(IPosition(3,axis,ii,jj));
		}

	      if (k)
		{
		  tmpY.resize(k);
		  outY(ndx)(IPosition(3,axis,ii,jj)) = (yFilter)(tmpY);
		}
	    }
      outX(ndx) = inX(ndx);
      ndx++;
      if (ndx < N)
	{
	  if (windowSizeReached) 
	    {
	      start++;stop++;
	      stop=(stop<N)?stop:N-1;
	    }
	  else 
	    {
	      stop = 2*ndx -start;
	      stop = (stop<N)?stop:N-1;
	      if (fabs(inX(start)-inX(stop)) >= timeConstant) 
		{
		  start++;
		  windowSizeReached=True;
		  if (stop != N-1) 
		    while(fabs(inX(start)-inX(stop)) > timeConstant) stop--;
		}
	    }
	}
      else break;
    }
}

void SolvableVisJones::filter(Vector<Double>& inX,  Vector<Array<Double> > &inY,
			      Vector<Bool> &inFlags,
			      Vector<Double>& outX, Vector<Array<Double> > &outY,
			      FilterType filterType, Float timeConstant, Float scanGap)
{
  Int N=inX.nelements(),nSlots=0;
  IPosition yShape = inY(0).shape();
  Vector<Int> scanMarkers;
  Double t0,t;
  uInt i;
  Double (*xFilter)(const Array<Double>&);
  DComplex (*yFilter)(const Array<DComplex>&);
  
  if (N==0) return;
  //
  // Cache the user selection of the type of filter as an appropriate
  // function pointer.
  //
  // Using proxy inlined functions to convert the signature of various
  // filters into one generic type.
  //

  switch (filterType)
    {
    case meanFilter: {xFilter = &::meanFilterProxy; yFilter = &::meanFilterProxy;break;}
    case medianFilter: {xFilter = &::medianFilterProxy; yFilter=&::medianFilterProxy;break;}
    }

  //
  // Find the number of "scans" and scan boundary markers.  Look for
  // time jumps >=scanGap as a scan boundary marker.  A rather loose
  // defintion and hence hardly robust!
  // 
  t0=inX(0);
  for (i=0;i<N;i++)
    if (fabs(inX(i)-t0) > scanGap) // If time gap > scanGap, call it a "scan boundary"!
      {
	nSlots++;
	scanMarkers.resize(nSlots,True);
	t0=inX(i);
	i--;
	scanMarkers[nSlots-1]=i;
	//	cerr << i<<":"<<scanMarkers[nSlots-1] << " ";
      }
  nSlots++;
  scanMarkers.resize(nSlots,True);
  scanMarkers[nSlots-1]=i;
  //  cerr << i<<":"<<scanMarkers[nSlots-1] << " ";
  //  cerr << endl;
  
  //
  // Extract the data corresponding to the time scale given by
  // TimeConstant, into a vector (slice) and run the filter on this
  // data.  The slicing stops at scan markers determined above.  The
  // filtered data is then returned in tX and tY.
  //
  uInt subMarker,smStart, smStop, stop,start,ndx,slotNo;
  DComplex smoothedValue;
  Int len;

  ndx=0; smStart=0; smStop=scanMarkers(0);
  start=slotNo=0;
  subMarker=smStart;

  //  cerr << "Slots: ";
  while(ndx < scanMarkers.nelements())
    {
      smStop  = scanMarkers(ndx);
      start=smStart;
      while (start<smStop)
	{
	  t0=inX(start);  // The origin of x-axis for the sub-interval within the scan
	  //
	  // Get the end of the sub-interval
	  //
	  for (subMarker=start;subMarker<smStop;subMarker++)
	    if (fabs(t0-inX(subMarker)) > timeConstant) {subMarker--;break;}

	  stop=(subMarker);
	  stop=(stop>=smStop)?smStop-1:stop;
	  stop=(stop<start)?start:stop;// If all's well, this should
                                       //  always be redundant
                                       //  statement - just a safety
                                       //  valve for now!
	  //	  cerr << "[" << start<<","<<stop<<"] ";

	  //
	  // Harvest the data for the sub-interval and apply the filter.
	  //
	  // Start a new scope - since the slicer has trouble if the
	  // shape of the LHS of X(slice) is neither 0 nor the same as
	  // X(slice).shape().
	  //
	  {
	    Vector<Double> tmpX;
	    Vector<Array<Double> > tmpY;
	    Vector<DComplex> anotherTmp;

	    Slicer slice(IPosition(1,start),
			 IPosition(1,stop),
			 IPosition(1,1),
			 Slicer::endIsLast);
	    tmpX = inX(slice);
	    tmpY = inY(slice);

	    outX.resize(slotNo+1,True);
	    outY.resize(slotNo+1,True);
	    outY(slotNo).resize(yShape);
	    //
	    // Apply the chosen filter to the data for the
	    // sub-interval.  Store the result in tX and tY
	    //
	    outX(slotNo) = (xFilter)(tmpX);
	    //
	    // Fill in these values - since the maping from filtered
	    // data to slotNo can be lost
	    //
	    fieldid_(slotNo) = fieldid_(start);
	    len=tmpY.nelements();
	    anotherTmp.resize(len,True);
	    uInt nGoodData=0;
	    for (Int ii=0;ii<2;ii++)
	      for (Int jj=0;jj<2;jj++)
		{
		  for (Int k=0;k<len;k++)
		    if (inFlags(k)){
		      anotherTmp(k) = DComplex(tmpY(k)(IPosition(3,0,ii,jj)),
					       tmpY(k)(IPosition(3,1,ii,jj)));
		      nGoodData++;
		    }
		  smoothedValue = (yFilter)(anotherTmp);
		  outY(slotNo)(IPosition(3,0,ii,jj)) = smoothedValue.real();
		  outY(slotNo)(IPosition(3,1,ii,jj)) = smoothedValue.imag();
		}
	    //
	    // If not even a single data point went into the filter, this slot
	    // has no valid filtered data.  Just negate the corresponding time
	    // value.  The filterFrom() method will use this info.
	    //
	    if (nGoodData==0) outX(slotNo) *= -1;
	  }
	  //	  start = subMarker+1;
	  start = stop+1;
	  slotNo++;
	}
      smStart = start; 
      ndx++;
    }

  //  cerr << ndx << " " << outX.nelements() << endl;
}
//
// Method to filter the other SolvableVisJones and fill this
// SolvableVisJones by the result. Filter time constant (timeConstant)
// and scanGap are in seconds.
//
void  SolvableVisJones::filterFrom(SolvableVisJones& other, 
				   FilterType filterType, Float timeConstant,
				   Bool doAmpPhase,
				   Float scanGap)
{
  AlwaysAssert(other.numberAnt_==numberAnt_,AipsError);
  AlwaysAssert(other.numberSpw_==numberSpw_,AipsError);

  // We declare an array of Doubles to hold (real,imag) or
  // (amp,phase) for each term for each antenna. This is
  // passed to the Interpolate1D class as one chunk.
  IPosition gainShape(IPosition(3,2,2,2));

  Int islot, iant, ispw;
  Int nSlots, nAnt, nSPW;
  Bool tmp;

  nSlots = this->numberOfSlots_;
  nAnt  = this->numberAnt_;
  nSPW = this->numberSpw_;

  nSlots = other.numberOfSlots_;
  nAnt  = other.numberAnt_;
  nSPW = other.numberSpw_;

  for (ispw=0;ispw<this->numberSpw_;ispw++) {
    for (iant=0;iant<this->numberAnt_;iant++) {
      // Fill in times of gain solutions
      Int goodSlot=0;
      for (islot=0;islot<other.numberOfSlots_;islot++) {
	if(other.iSolutionOK_(iant,ispw,islot)) goodSlot++;
	tmp = other.iSolutionOK_(iant,ispw,islot);
      }
      Int numberOfOtherSlots=goodSlot;

      numberOfOtherSlots = other.numberOfSlots_;

      // If we have only good slot then just copy
      if(numberOfOtherSlots==1) {
	for (islot=0;islot<this->numberOfSlots_;islot++) {
	  this->antGain_(iant,ispw,islot)=other.antGain_(iant,ispw,0);
	  this->iSolutionOK_(iant,ispw,islot)=other.iSolutionOK_(iant,ispw,0);
	}
	//	return;
      }
      // If we have no good slots then flag everything
      else if(numberOfOtherSlots==0) {
	for (islot=0;islot<this->numberOfSlots_;islot++) {
	  this->antGain_(iant,ispw,islot)=Complex(1.0,0.0);
	  this->iSolutionOK_(iant,ispw,islot)=False;
	}
	//	return;
      }
      else {
      // Fill in the other times 
	Vector<Double> otherTimes(numberOfOtherSlots);
	Vector<Bool> otherFlags(numberOfOtherSlots);

	for (islot=0,goodSlot=0;islot<other.numberOfSlots_;islot++) {
	  //	  if(other.iSolutionOK_(iant,ispw,islot)) 
	    {
	      otherTimes(goodSlot)=0.5*(other.MJDStart_(islot)+other.MJDStop_(islot));
	      goodSlot++;
	    }
	}
	
	// Fill in the vector of gain arrays
	//	Vector<Array<Double> > otherGains(other.numberOfSlots_);
	Vector<Array<Double> > otherGains(numberOfOtherSlots);
	
	// Loop over all antennas
	for (islot=0,goodSlot=0;islot<other.numberOfSlots_;islot++) 
	  {
	    //	    if(other.iSolutionOK_(iant,ispw,islot)) 
	      {
		otherFlags(islot)=other.iSolutionOK_(iant,ispw,islot);
		otherGains(goodSlot).resize(gainShape);
	      
		for (Int j=0; j<2; j++) 
		  {
		    for (Int i=0; i<2; i++) 
		      {
			Complex g=((const mjJones2&)other.antGain_(iant,ispw,islot))(i,j);
			Complex val;
			if(doAmpPhase) val=Complex(abs(g),arg(g));
			else           val=g;
			otherGains(goodSlot)(IPosition(3,0,i,j))=real(val);
			otherGains(goodSlot)(IPosition(3,1,i,j))=imag(val);
		      }
		  }
		goodSlot++;
	      }
	  }
	
	
// 	cerr << "Sizes: " << otherTimes.nelements() << " " << otherGains.nelements() << endl;
// 	for (islot=0;islot<numberOfOtherSlots;islot++) 
// 	  cout << "y= " << iant 
// 	       << " " << 0.5*(other.MJDStart_(islot)+other.MJDStop_(islot))-
// 	    0.5*(other.MJDStart_(0)+other.MJDStop_(0))
// 	       << " " << otherGains(islot)(IPosition(3,0,0,0))
// 	       << " " << otherGains(islot)(IPosition(3,1,0,0)) 
// 	       << " " << iant << " " << ispw << " " << islot << " "
// 	       << iSolutionOK_(iant,ispw,islot) << endl;
	//
	// Set all flags for this iant and ispw to False.  Later, set the ones which 
	// have valid filtered data to True.
	//
	//	for (uInt islot=0;islot<numberOfSlots_;islot++) iSolutionOK_(iant,ispw,islot)=False;
	
	{
	  Vector<Double> tX;
	  Vector<Array<Double> > tY;
	  Vector<Bool> tFlags;
	  tFlags.resize(other.numberOfSlots_);
	  for (uInt i=0;i<tFlags.nelements();i++) tFlags(i)=other.iSolutionOK_(iant,ispw,i);

// 	  cerr << "Sizes: " << otherTimes.nelements() << " " << otherGains.nelements() << " "
// 	       << tX.nelements() << " " << tY.nelements() << endl;
	  

	  if ((filterType == slidingMean) || (filterType == slidingMedian))
	    slidingFilter(otherTimes,otherGains, otherFlags, tX, tY, filterType, 
			    timeConstant,scanGap);
	  else
	    filter(otherTimes,otherGains, otherFlags, tX, tY, filterType, timeConstant,scanGap);

	  //
	  // Resize the antGain_, MJDStart/Stop and other arrays and fill
	  // them.  This is not true resizing - simply change the numberOfSlots_
	  // value to the length out the filtered array.  This assumes that the
	  // latter is always equal to or smaller in length than the original
	  // numberOfSlots_.
	  //
	  // If the filted time is negative, that indicates that the
	  // filtered gains are not valid for that slot.  So fill in
	  // absolute of the filtered time in MJD{Start,Stop}_, set the
	  // iSolutionOK_ for this time slot to False and don't bother
	  // about the gains.
	  //
	  numberOfSlots_=tX.nelements();
	  Matrix<Complex> antResult(2,2);
	  DComplex val;
	  for (uInt islot=0;islot<numberOfSlots_;islot++) {
	    MJDStart_(islot) = MJDStop_(islot) = fabs(tX(islot));
	    //	    if (tX(islot) > 0)
	      {
	      //	      iSolutionOK_(iant,ispw,islot)=True;
	      for (Int j=0; j<2; j++) {
		for (Int i=0; i<2; i++) {
		  if(doAmpPhase) val=polar(tY(islot)(IPosition(3,0,i,j)),
					   tY(islot)(IPosition(3,1,i,j)));
		  else           val=DComplex(tY(islot)(IPosition(3,0,i,j)),
					      tY(islot)(IPosition(3,1,i,j)));
		  antResult(i,j)=Complex(val.real(), val.imag());
		}
	      }
	    }
	    antGain_(iant,ispw,islot)=antResult;
// 	    cerr << "fy= "
// 		 << MJDStart_(islot)-MJDStart_(0) << " "
// 		 << antGain_(iant,ispw,islot)(0,0).imag() << " "
// 		 << iant << " " << ispw << " " << islot << " "
// 		 << iSolutionOK_(iant,ispw,islot) << endl;
	  }
	}
      }
    }
  }
}

// Interpolate this from the other. Since we have to pay attention
// to the status of each solution, we loop through for each
// antenna and spectral window.
void SolvableVisJones::interpolateFrom(SolvableVisJones& other, 
				       InterpolaterType interpolaterType,
				       Bool doAmpPhase) 
{
  
  AlwaysAssert(other.numberAnt_==numberAnt_,AipsError);
  AlwaysAssert(other.numberSpw_==numberSpw_,AipsError);
  
  // We declare an array of Doubles to hold (real,imag) or
  // (amp,phase) for each term for each antenna. This is
  // passed to the Interpolate1D class as one chunk.
  IPosition gainShape(IPosition(3,2,2,2));
  //  IPosition gainShape(IPosition(2,2,2));
  
  Int islot, iant, ispw;
  
  for (ispw=0;ispw<this->numberSpw_;ispw++) {
    for (iant=0;iant<this->numberAnt_;iant++) {
      //      cout << endl;
      // Fill in times of gain solutions
      Int goodSlot=0;
      for (islot=0;islot<other.numberOfSlots_;islot++) {
        if(other.iSolutionOK_(iant,ispw,islot)) goodSlot++;
      }
      Int numberOfOtherSlots=goodSlot;
      
      // If we have only good slot then just copy
      if(numberOfOtherSlots==1) {
	for (islot=0;islot<this->numberOfSlots_;islot++) {
	  this->antGain_(iant,ispw,islot)=other.antGain_(iant,ispw,0);
	  this->iSolutionOK_(iant,ispw,islot)=other.iSolutionOK_(iant,ispw,0);
	}
	//	return;
      }
      // If we have no good slots then flag everything
      else if(numberOfOtherSlots==0) {
	for (islot=0;islot<this->numberOfSlots_;islot++) {
	  this->antGain_(iant,ispw,islot)=Complex(1.0,0.0);
	  this->iSolutionOK_(iant,ispw,islot)=False;
	}
	//	return;
      }
      else {
      // Fill in the other times 
	Vector<Double> otherTimes(numberOfOtherSlots);
	for (islot=0,goodSlot=0;islot<other.numberOfSlots_;islot++) {
	  if(other.iSolutionOK_(iant,ispw,islot)) {
	    otherTimes(goodSlot)=0.5*(other.MJDStart_(islot)+other.MJDStop_(islot));
	    goodSlot++;
	  }
	}
	
	// Fill in the vector of gain arrays
	Vector<Array<Double> > otherGains(numberOfOtherSlots);//other.numberOfSlots_);
	
	// Loop over all antennas
	for (islot=0,goodSlot=0;islot<other.numberOfSlots_;islot++) 
	  {
	    if(other.iSolutionOK_(iant,ispw,islot)) {
	      otherGains(goodSlot).resize(gainShape);
	      
	      for (Int j=0; j<2; j++) 
		{
		  for (Int i=0; i<2; i++) 
		    {
		      Complex g=((const mjJones2&)other.antGain_(iant,ispw,islot))(i,j);
		      Complex val;
		      if(doAmpPhase) val=Complex(abs(g),arg(g));
		      else           val=g;
		      otherGains(goodSlot)(IPosition(3,0,i,j))=real(val);
		      otherGains(goodSlot)(IPosition(3,1,i,j))=imag(val);
		    }
		}
	      //	    cout << endl;
	      goodSlot++;
	    }
	  }
	
	//       Vector<Double> tX;
	//       Vector<Array<Double> > tY;
	//       Filter(otherTimes,otherGains, tX, tY, iant, TimeConstant);
	
	// Functional for x axis: time
	ScalarSampledFunctional<Double> x(otherTimes);
	//      ScalarSampledFunctional<Double> x(tX);
	
	// Make the y axis Functional and the Interpolator
	ScalarSampledFunctional<Array<Double> > y(otherGains);
	//      ScalarSampledFunctional<Array<Double> > y(tY);
	
	
	/*
	  for (int i=0;i<x.nelements();i++)
	  
	  
	  {
	  cerr << "Filtered: " << iant << " " << x(i)/1E9-4.48503 << " ";
	  for (int ii=0;ii<2;ii++)
	  for (int jj=0;jj<2;jj++)
	  cerr << y(i)(IPosition(3,0,ii,jj)) << " " << y(i)(IPosition(3,1,ii,jj)) << " ";
	  cerr << endl;
	  }
	*/	
	
	Interpolate1D<Double,Array<Double> > interpolatedGain(x, y);
	
	// Set the interpolation method: use linear unless we have enough
	// points for a spline
	if(numberOfOtherSlots>4) {
	  //	interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::spline);
	  //	interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::linear);
	  //	interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::cubic);
	  switch(interpolaterType)
	    {
	    case nearestNeighbour:
	      interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::nearestNeighbour);
	      break;
	    case linear:
	      interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::linear);
	      break;
	    case cubic:
	      interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::cubic);
	      break;
	    case spline:
	      interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::spline);
	      break;
	    }
	}
	else
	  interpolatedGain.setMethod(Interpolate1D<Double,Array<Double> >::linear);
	
	
	// Now fill in these gains by calling interpolatedGain() for each
	// time slot. 
	Array<Double> result(gainShape);
	
	for (islot=0;islot<this->numberOfSlots_;islot++) 
	  {
	    result=interpolatedGain(0.5*(this->MJDStart_(islot)+this->MJDStop_(islot)));
	    
	    // Now we need to fill the gains for this time back into the antGain_
	    // repository
	    Matrix<Complex> antResult(2,2);
	    DComplex val;
	    for (Int j=0; j<2; j++) 
	      {
		for (Int i=0; i<2; i++) 
		  {
		    if(doAmpPhase)
		      val=polar(result(IPosition(3,0,i,j)),
				result(IPosition(3,1,i,j)));
		    else
		      val = DComplex(result(IPosition(3,0,i,j)),
				     result(IPosition(3,1,i,j)));
		    
		    antResult(i,j)=Complex(val.real(), val.imag());
		  }
	      }
	    
	    //
	    // Was there any valid data for the current ant and spw?
	    // This should be made more sophisticated - if the interpolatoer
	    // is nearest, only on valid data may be enough.   Else at least
	    // 4 are required before the interpolated data may make any sense.
	    //
	    uInt validData=0;
	    Bool hasData=False;
	    for (uInt i=0;i<numberOfSlots_;i++) 
	      if (iSolutionOK_(iant,ispw,i)) validData++;
	    
	    
	    if ((interpolaterType == nearestNeighbour) && (validData > 2)) hasData=True;
	    else if (validData > 4) hasData=True;
	    
	    this->antGain_(iant,ispw,islot)=antResult;
	    this->iSolutionOK_(iant,ispw,islot)=True;
	    this->solutionOK_(islot)=True;
	    this->fitwt_(islot)=1.0;
	    this->fit_(islot)=1.0;
	    this->iFit_.xyPlane(islot)=1.0;
	    this->iFitwt_.xyPlane(islot)=1.0;
	    
	    
	    /*	  cout << iant 
		  << " " << islot 
		  << " " << 0.5*(this->MJDStart_(islot)+this->MJDStop_(islot))/1E9-4.48503 << " ";
	    */
	    for (int ii=0;ii<2;ii++)
	      for (int jj=0;jj<2;jj++)
		{
		  Double re,im;
		  re = real(antResult(ii,jj));
		  im = imag(antResult(ii,jj));
		  //		cout << sqrt(re*re+im*im) << " " << atan2(im,re) << " ";
		  //cout << real(antResult(ii,jj)) << " " << imag(antResult(ii,jj)) << " " ;
		}
	    //	  cout << endl;
	  }
      }
    }
  }
}

#endif

void SolvableVisJones::store (const String& file, const Bool& append)
{
  // Write the solutions to an output calibration table
  // Input:
  //    file           String        Cal table name
  //    append         Bool          Append if true, else overwrite
  //
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
    tab = new SolvableVisJonesTable (file, typeName(), access);
  };
  
  // Write every spw w/ max number of channels 
  //  (eventually, CalTable should permit variable-shape cols)
  Int maxNumChan(1);
  if (freqDep()) maxNumChan=max(nSolnChan_);

  // Some default values
  Double dzero = 0;
  IPosition ip(2,1,maxNumChan);

  // CalDesc Sub-table records
  CalDescRecord* descRec;
  Vector<Int> calDescNum(numberSpw_); calDescNum=-1;
  for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {

    // Write a CalDesc for each spw which has solutions
    // Note: CalDesc index != SpwId, in general

    if (jonesPar_[iSpw]!=NULL) {

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
	Vector <Int> numChan(1,nSolnChan_(iSpw));
	for (Int ichan=0; ichan<nSolnChan_(iSpw); ichan++) {
	  chanRange(0,0,ichan)=startChan_(iSpw);
	  chanRange(1,0,ichan)=startChan_(iSpw) + nSolnChan_(iSpw) -1;
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
	descRec->defineMSName ("");
	
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

  for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {

    // Write table for spws which have solutions
    if (jonesPar_[iSpw]!=NULL) {

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
      Array<Complex>  thisAntGain;      thisAntGain.reference(*(jonesPar_[iSpw]));
      Matrix<Bool>    thisISolutionOK;  thisISolutionOK.reference(*(iSolutionOK_[iSpw]));
      Matrix<Float>   thisIFit;         thisIFit.reference(*(iFit_[iSpw]));
      Matrix<Float>   thisIFitwt;       thisIFitwt.reference(*(iFitwt_[iSpw]));
      
      // total rows to be written this Spw
      Int nRow; nRow=numberAnt_*ntrue(thisSolOK);

      if (nRow > 0) {
      
	// These are constant columns (with boring values, currently)
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
	
	// This is constant
	Vector<Int> calDescId(nRow,calDescNum(iSpw));
	
	// These are constant per slot
	//   (these cols should be incremental)
	Vector<Double> time(nRow,0.0);
	Vector<Double> interval(nRow,0.0);
	Vector<Int>    fieldId(nRow,0);
	Vector<String> fieldName(nRow,"");
	Vector<String> sourceName(nRow,"");
	Vector<Bool>   totalSolOk(nRow,False);
	Vector<Float>  totalFit(nRow,0.0);
	Vector<Float>  totalFitWt(nRow,0.0);
	
	// These vary
	Vector<Int>    antenna1(nRow,0);
	Array<Complex> gain(IPosition(5,2,2,1,maxNumChan,nRow),Complex(0.0,0.0));
	Cube<Bool>     solOk(1,maxNumChan,nRow,False);
	Cube<Float>    fit(1,maxNumChan,nRow,0.0);
	Cube<Float>    fitWt(1,maxNumChan,nRow,0.0);
	
	IPosition out(5,0,0,0,0,0);
	IPosition in(4,0,0,0,0);
	Int thisRow(0);
	for (Int islot = 0; islot < numberSlot_(iSpw); islot++) {
	  in(3)=islot;
	  if (thisSolOK(islot)) {
	    
	    // Fill slot-constant cols:
	    Slice thisSlice(thisRow,numberAnt_);
	    time(thisSlice)=thisMJDTimeStamp(islot);
	    interval(thisSlice)=(thisMJDStop(islot) - thisMJDStart(islot));
	    fieldId(thisSlice)=thisFieldId(islot);
	    fieldName(thisSlice)=thisFieldName(islot);
	    sourceName(thisSlice)=thisSourceName(islot);
	    totalSolOk(thisSlice)=thisSolOK(islot);
	    totalFit(thisSlice)=thisFit(islot);
	    totalFitWt(thisSlice)=thisFitwt(islot);
	    
	    // Loop over the number of antennas
	    
	    for (Int iant = 0; iant < numberAnt_; iant++) {
	      out(4)=thisRow;
	      in(2)=iant;
	      // Antenna index
	      antenna1(thisRow)=iant;
	      
	      // Gain  (make this matrix type specific!)
	      for (Int ichan=0; ichan<nSolnChan_(iSpw); ichan++) {
		out(3)=in(1)=ichan;
		switch (nPar_) {
		case 1: {
		  in(0)=0;
		  for (Int i=0; i<2; i++) {
		    out(0)=out(1)=i;
		    gain(out)=thisAntGain(in);
		  }
		  break;
		}
		case 2: {
		  for (Int i=0; i<2; i++) {
		    out(0)=out(1)=in(0)=i;
		    gain(out)=thisAntGain(in);
		  }
		  break;
		}
		case 4: {
		  for (Int i = 0; i < 2; i++) {
		    for (Int j = 0; j < 2; j++) {
		      out(0)=i; out(1)=j;
		      in(0)=2*i+j;
		      gain(out)=thisAntGain(in);
		    };
		  };
		  break;
		};
		}
		// Gain stats  (slot constant, per spw?)
		solOk(0,ichan,thisRow) = thisISolutionOK(iant,islot);
		fit(0,ichan,thisRow) = thisIFit(iant,islot);
		fitWt(0,ichan,thisRow) = thisIFitwt(iant,islot);
	      }
	      
	      // next time round is next row
	      thisRow++;
	    };
	    
	  };
	};
	
	// Now push everything to the disk table
	tab->addRowMain(nRow);
	SolvableVisJonesMCol svjmcol(*tab);
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
	svjmcol.gain().putColumnCells(refRows,gain);
	svjmcol.totalSolnOk().putColumnCells(refRows,totalSolOk);
	svjmcol.totalFit().putColumnCells(refRows,totalFit);
	svjmcol.totalFitWgt().putColumnCells(refRows,totalFitWt);
	svjmcol.solnOk().putColumnCells(refRows,solOk);
	svjmcol.fit().putColumnCells(refRows,fit);
	svjmcol.fitWgt().putColumnCells(refRows,fitWt);
	svjmcol.calDescId().putColumnCells(refRows,calDescId);
	svjmcol.calHistoryId().putColumnCells(refRows,calHistId);
	
	
	nMain = tab->nRowMain();
	
      }
      //  cout << typeName() << ": store(columnwise) execution    = " << timer.all_usec()/1e6 << endl;
      //  cout << typeName() << ": store(columnwise) memory usage = " << Memory::allocatedMemoryInBytes() - startMem << endl;
    }
  }

  delete tab;

};


void SolvableVisJones::load (const String& file, const String& select, 
			     const String& type)
{
  // Load data from a calibration table
  // Input:
  //    file         const String&       Cal table name
  //    select       const String&       Selection string
  //    type         const String&       Jones matrix type
  //                                    (scalar, diagonal or general)
  //
  
  LogMessage message(LogOrigin("SolvableVisJones","load"));
  
  // Decode the Jones matrix type
  Int jonesType = 0;
  if (type == "scalar") jonesType = 1;
  if (type == "diagonal") jonesType = 2;
  if (type == "general") jonesType = 3;

 // Open, select, sort the calibration table
  SolvableVisJonesTable svjtab(file);
  svjtab.select2(select);

  // Abort here if selection fails to find any CalTable rows
  if (svjtab.nRowMain()<1) 
    throw(AipsError(" Specified cal table selection selects no solutions in this table.  Please review setapply settings."));

  // Get no. of antennas and time slots
  numberAnt_ = svjtab.maxAntenna() + 1;

  Int nDesc=svjtab.nRowDesc();
  Vector<Int> spwmap(nDesc,-1);
  for (Int idesc=0;idesc<nDesc;idesc++) {
    // This cal desc
    CalDescRecord* calDescRec = new CalDescRecord (svjtab.getRowDesc(idesc));

    // Get this spw ID
    Vector<Int> spwlist;
    calDescRec->getSpwId(spwlist);
    Int nSpw; spwlist.shape(nSpw);
    if (nSpw > 1) {};  // ERROR!!!  Should only be one spw per cal desc!
    spwmap(idesc)=spwlist(0);

    // In next few steps, need to watch for repeat spws in new cal descs!!

    // Get number of channels this spw
    Vector<Int> nchanlist;
    calDescRec->getNumChan(nchanlist);
    nSolnChan_(spwmap(idesc))=nchanlist(0);

    // Get channel range / start channel
    Cube<Int> chanRange;
    calDescRec->getChanRange(chanRange);
    startChan_(spwmap(idesc))=chanRange(0,0,0);

    // Get slot count for this desc
    ostringstream thisDesc;
    thisDesc << "CAL_DESC_ID==" << idesc;
    CalTable thisDescTab = svjtab.select(thisDesc.str());
    numberSlot_(spwmap(idesc))=thisDescTab.nRowMain()/numberAnt_;

    delete calDescRec;  
  }

  
  // Get solution interval (assumed constant across table)
  SolvableVisJonesMRec* solvableVJMRec = 
    new SolvableVisJonesMRec (svjtab.getRowMain(0));
  solvableVJMRec->getInterval (interval_);
  deltat_ = 0.01 * interval_;
  delete solvableVJMRec;
  
  // At this point, we know how big our slot-dep caches must be
  //  (in private data), so initialize them
  initMetaCache();
  initAntGain();
  initSolveCache();

  // Remember if we found and filled any solutions
  Bool solfillok(False);

  // Fill per caldesc
  for (Int idesc=0;idesc<nDesc;idesc++) {

    Int thisSpw=spwmap(idesc);
      
    // Reopen and globally select caltable
    SolvableVisJonesTable svjtabspw(file);
    svjtabspw.select2(select);

    // isolate this caldesc:
    ostringstream selectstr;
    selectstr << "CAL_DESC_ID == " << idesc;
    String caldescsel; caldescsel = selectstr.str();
    svjtabspw.select2(caldescsel);

    Int nrow = svjtabspw.nRowMain();
    IPosition out(5,0,0,0,0,0);
    IPosition in(4,0,0,0,0);
    if (nrow>0) {

      // Found some solutions to fill
      solfillok=True;

      // Ensure sorted on time
      Block <String> sortCol(1,"TIME");
      svjtabspw.sort2(sortCol);
      
      // Extract the gain table columns
      ROSolvableVisJonesMCol svjmcol(svjtabspw);
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
      Array<Complex> gain;       svjmcol.gain().getColumn(gain);
      Cube<Bool>     solOk;      svjmcol.solnOk().getColumn(solOk);
      Cube<Float>    fit;        svjmcol.fit().getColumn(fit);
      Cube<Float>    fitWt;      svjmcol.fitWgt().getColumn(fitWt);
      
      // Read the calibration information
      deltat_ = 0.01 * interval(0);
      Double lastTime(-1.0), thisTime(0.0), thisInterval(0.0);
      Int islot(-1);
      Int iant, i, j;
      
      for (Int irow=0; irow<nrow; irow++) {
	out(4)=irow;

	thisTime=time(irow);
	
	// If this is a new timestamp
	if (abs (thisTime - lastTime) > deltat_) {
	  
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
	
	for (Int ichan=0; ichan<nSolnChan_(thisSpw); ichan++) {

	  (*jonesParOK_[thisSpw])(ichan,iant,islot) = solOk(0,ichan,irow);

	  out(3)=in(1)=ichan;
	  switch (jonesType) {
	  case 1: 
	    // scalar (diagonal mode will work)
	  case 2: {
	    // diagonal
	    for (Int ipar=0; ipar<nPar_; ipar++) {
	      out(0)=out(1)=in(0)=ipar;
	      (*jonesPar_[thisSpw])(in) = gain(out);
	    }
	    break;
	  };
	  case 3: {
	    // general 
	    for (i = 0; i < 2; i++) {
	      for (j = 0; j < 2; j++) {
		out(0)=i;out(1)=j;
		in(0)=i*2+j;
		(*jonesPar_[thisSpw])(in) = gain(out);
	      };
	    };
	    break;
	  };
	  }; // switch (jonesType)...
	}
	
      } // irow
    } // nrow>0

  } // idesc

  // If we found no solutions in selected table, abort:
  if (!solfillok) {
      throw(AipsError(" Specified cal table selection selects no solutions in this table.  Please review setapply settings."));
  }


};




// ********************************************************************
// ********************************************************************
// ********************************************************************
//
//   Specialized solvable types:
//   ******************************
//


// Generic GJones ctor
GJones::GJones(VisSet& vs)
  : SolvableVisJones(vs) {
 
  // This is a 2 parameter type
  nPar_=2;

  // Set required element mask
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;

}

GJones::~GJones() {
}

// ********************************************************************
// ********************************************************************


// Generic BJones ctor
BJones::BJones(VisSet& vs) :
  GJones(vs) {

  // 
  nSolnChan_=vs.numberChan();
  startChan_=vs.startChan();

}

BJones::~BJones() {}


// ********************************************************************
// ********************************************************************


EVisJones::EVisJones(VisSet& vs) : SolvableVisJones(vs) {

  //  cout << "EVisJones ctor" << endl;
  // This is a 2 parameter type
  //  (gain curve each pol; should be only one)
  nPar_=2;

  gaincoeff_p = 0;
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;
  
}

void EVisJones::setInterpolation(const Record& interpolation) {

  //  cout << "EVisJones::setInterpolation" << endl;

  LogMessage message(LogOrigin("EVisJones","setInterpolation"));

  // Execute TVVJ version
  TimeVarVisJones::setInterpolation(interpolation);

  // Now fill gain curve

  //  cout << "vs_->msName() = " << vs_->msName() << endl;
  //  MeasurementSet ms(vs_->msName());

  const ROMSColumns& mscol(vs_->iter().msColumns());
  
  const ROMSObservationColumns& obscol(mscol.observation());
  Vector<Double> timerange(obscol.timeRange()(0));
  Double obstime(timerange(0));
  
  const ROMSAntennaColumns& antcol(mscol.antenna());
  uInt nant=vs_->numberAnt();
  Vector<String> antnames(antcol.name().getColumn());
  
  const ROMSSpWindowColumns& spwcol(mscol.spectralWindow());
  uInt nspw=vs_->numberSpw();
  Vector<Double> reffreq(spwcol.refFrequency().getColumn());
  
  IPosition gainsize(4,4,2,nant,nspw);
  gaincoeff_p = new Array<Float>(gainsize,0.0);
  
  // Assemble path to raw gain curve table
  //  (Eventually, fillers will do this and fill a GAINCURVE subtable...)
  String telescope=obscol.telescopeName()(0);
  if (telescope=="VLA") telescope="/nrao/VLA/GainCurves";
  String drgaintab= Aipsrc::aipsRoot() + "/data" + telescope;

  // Try VLA table if path fails
  if (!Table::isReadable(drgaintab)) drgaintab=Aipsrc::aipsRoot() + "/data/nrao/VLA/GainCurves";

  { ostringstream o; 
  o<< "  Searching for gain curve data in " 
   << drgaintab;
  message.message(o);
  logSink().post(message);
  }

  // Open raw gain table
  Table rawgaintab(drgaintab);
  
  // Select on Time
  Table timegaintab = rawgaintab(rawgaintab.col("BTIME") <= obstime
                                 && rawgaintab.col("ETIME") > obstime);
 
  // Fill gaincoeff_p...
  
  IPosition blc(4),trc(4);
  blc(0)=0; trc(0)=3;
  blc(1)=0; trc(1)=1;
  // ...for each spw:
  for (uInt ispw=0; ispw<nspw; ispw++) {
    blc(3)=ispw; trc(3)=ispw;
    
    // Select on freq:
    Table freqgaintab = timegaintab(timegaintab.col("BFREQ") <= reffreq(ispw)
				    && timegaintab.col("EFREQ") > reffreq(ispw));
    
    if (freqgaintab.nrow() > 0) {

      { ostringstream o; 
      o<< "  Found the following gain curve coefficient data" << endl
       << "  for spectral window = "  << ispw << " (ref freq=" 
       << reffreq(ispw) << "):";
      message.message(o);
      logSink().post(message);
      }
      
      // Find nominal gain curve
      //  Nominal antenna is named "0" (this is a VLA convention)
      Matrix<Float> nomgain(4,2,0.0);
      {
        Table nomgaintab = freqgaintab(freqgaintab.col("ANTENNA")=="0");
        if (nomgaintab.nrow() > 0) {
          ROArrayColumn<Float> gain(nomgaintab,"GAIN");
          nomgain=gain(0);
        } else {
          // nominal gain is unity
          nomgain(0,0)=1.0;
          nomgain(0,1)=1.0;
        }
      }
      for (uInt iant=0; iant<nant; iant++) {
        blc(2)=iant; trc(2)=iant;
        Table antgaintab = freqgaintab(freqgaintab.col("ANTENNA")==antnames(iant));
	String nom;
        if (antgaintab.nrow() > 0) {
          ROArrayColumn<Float> gain(antgaintab,"GAIN");
          (*gaincoeff_p)(blc,trc).nonDegenerate()=gain(0);
	  nom=" ";
        } else {
          (*gaincoeff_p)(blc,trc).nonDegenerate()=nomgain;
	  nom="nominal";
        }

	{ ostringstream o; 
	  IPosition blc1(blc),trc1(trc),blc2(blc),trc2(trc);
	  trc1(1)=0;blc2(1)=1;
	  
	  o<< "   Antenna=" << antnames(iant) << ": " 
	   << (*gaincoeff_p)(blc1,trc1).nonDegenerate() << " " 
	   << (*gaincoeff_p)(blc2,trc2).nonDegenerate();
	  
	  message.message(o);
	  logSink().post(message);
	}
      }
    } else {
      { ostringstream o; 
      o<< "Could not find gain curve data for spectral window ="
       << ispw << "(ref freq=" << reffreq(ispw) << ")";
      message.message(o);
      logSink().post(message);
      }
    }
  }
}

EVisJones::~EVisJones() {
  //  cout << "EVisJones dtor" << endl;
  if(gaincoeff_p) delete gaincoeff_p; gaincoeff_p=NULL;
}

// Calculate matrix elements from opacity and elevation
void EVisJones::calcAntGainPar(const VisBuffer& vb)
{
  
  //  cout << "EVisJones::calAntGainPar()" << endl;

  // Current spectral window
  Int ispw=vb.spectralWindow();
  Double time=vb.time()(0);

  // Vector (over ants) of AZELs at time:
  Vector<MDirection> antazel(vb.azel(time));
  Int nant=static_cast<Int>(antazel.nelements());
  Int numAntennas = (nant < numberAnt_) ? nant : numberAnt_;

  Vector<Complex> vec(2);
  for (Int iant=0; iant<numAntennas; iant++) {

    // Extract zenith angle for this antenna in deg:
    Double za=(C::pi_2 - antazel(iant).getAngle().getValue()(1))*180.0/C::pi;

    uInt nterm=(*gaincoeff_p).shape()(0);

    Vector<Complex> gainloss(2);
    IPosition gci(4,0,0,iant,ispw);
    for (uInt ipol=0; ipol<2; ipol++) {
      gci(1)=ipol;
      gci(0)=nterm-1;
      gainloss(ipol)=(*gaincoeff_p)(gci);

      for (uInt iterm=nterm-1; iterm>0; iterm--) {
        gci(0)=iterm-1;
        gainloss(ipol)=gainloss(ipol)*za + (*gaincoeff_p)(gci);
      }
      (*thisJonesPar_[ispw])(ipol,0,iant) = gainloss(ipol);

    }
    (*thisJonesOK_[ispw])(iant)=True;
  }

  // Current matrix caches now invalid
  invalidateJM(ispw);
  invalidateMM(ispw);


}

void EVisJones::calcJonesPar(const Int& spw,
			    const Double& time,
			    const Int& field) {

  //  cout << " cJP(EVJ) ";

  // Vector (over ants) of AZELs at time:
  Vector<MDirection> antazel(vs_->iter().azel(time));
  Int nant=static_cast<Int>(antazel.nelements());
  Int numAntennas = (nant < numberAnt_) ? nant : numberAnt_;
  
  Vector<Complex> vec(2);
  for (Int iant=0; iant<numAntennas; iant++) {
    
    // Extract zenith angle for this antenna in deg:
    Double za=(C::pi_2 - antazel(iant).getAngle().getValue()(1))*180.0/C::pi;
    
    uInt nterm=(*gaincoeff_p).shape()(0);
    
    Vector<Complex> gainloss(2);
    IPosition gci(4,0,0,iant,spw);
    for (uInt ipol=0; ipol<2; ipol++) {
      gci(1)=ipol;
      gci(0)=nterm-1;
      gainloss(ipol)=(*gaincoeff_p)(gci);
      
      for (uInt iterm=nterm-1; iterm>0; iterm--) {
        gci(0)=iterm-1;
        gainloss(ipol)=gainloss(ipol)*za + (*gaincoeff_p)(gci);
      }
      (*thisJonesPar_[spw])(ipol,0,iant) = gainloss(ipol);

    }
    (*thisJonesOK_[spw])(iant)=True;
  }

  // Current matrix caches now invalid
  invalidateJM(spw);
  invalidateMM(spw);

}


// ********************************************************************
// ********************************************************************

DJones::DJones(VisSet& vs) : SolvableVisJones(vs) {

  // This is a 4 paramter type (for now):
  //   0 1
  //   2 3

  //  1 and 2 are the D-terms, 0 and 3 present for posang cal

  nPar_=4;

  jMatType_=3;
  required_.resize(2,2);
  required_=True;
  for (uInt id=0;id<2;id++) required_(id,id)=False;
}

DJones::~DJones() {}

void DJones::calcAntGainMat(mjJones2& mat, Vector<Complex>& par) {
  
  //  cout << "cAGM(D)";

  calcJonesMat(mat,par);

}
  
void DJones::calcJonesMat(mjJones2& mat, Vector<Complex>& par) {

  //  cout << "cJM(D)";

  // Requires nPar_=4 (implicitly true for this type)
  mat = (Matrix<Complex>) par.reform(IPosition(2,2,2));

  //  cout << mat.matrix() << endl;

}

void DJones::keep(const Int& spw,
		  const Int& slot) {

  // This keeps the off-diag elements, and puts 1+0i on the diag

  //  cout << "keep(D)" << endl;


  // Loop over antenna, channel and parameter
  for (Int iant=0; iant<numberAnt_; iant++) {
    for (Int ichan=0; ichan<nSolnChan_(spw); ichan++) {
      
      //      cout << (*thisJonesMat_[spw])(ichan,iant).matrix() << endl;

      for (Int i=0; i<2; i++) {
	for (Int j=0; j<2; j++) {
	  Int ipar=i*2+j;
	  IPosition ip(4,ipar,ichan,iant,slot);
	  if (i==j) {
	    (*jonesPar_[spw])(ip)=Complex(1.0,0.0);
	  }
	  else {
	    (*jonesPar_[spw])(ip)=
	      (*thisJonesMat_[spw])(ichan,iant)(i,j);
	  }
	  
    /*
	  cout << iant << " " 
	       << ichan << " " 
	       << i << " "
	       << j << " "
	       << ipar << " "
	       << (*jonesPar_[spw])(ip) << endl;
    */

	}
      }
    }
  }

}



// ********************************************************************
// ********************************************************************

TJones::TJones(VisSet& vs) : SolvableVisJones(vs) {

  // This is a 1 paramter type (scalar T)
  nPar_=1;

  jMatType_=1;
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;
};

TJones::~TJones() {}

void TJones::updateAntGain() {

  Bool phaseOnly=(mode().contains("phase"));

  Complex lscalar;
  for (Int iant=0;iant<numberAnt_;iant++) {
    lscalar=(*thisJonesMat_[currentSpw_])(0,iant)(0,0);
    Float ggS=ggS_(0,iant)(0,0)+ggS_(0,iant)(1,1);
    if (ggS>0.0) {
      Complex gS=gS_(0,iant)(0,0)+gS_(0,iant)(1,1);
      lscalar-=gain()*gS/ggS;
    }
    if(phaseOnly&&abs(lscalar)>0.0) {
      lscalar/=abs(lscalar);
    }
    (*thisJonesMat_[currentSpw_])(0,iant)=lscalar;
  }
}


// ********************************************************************
// ********************************************************************

TOpac::TOpac(VisSet& vs) : TJones(vs) {

  // This is a 1 paramter type (opacity)
  nPar_=1;

};

TOpac::~TOpac() {}

void TOpac::setInterpolation(const Record& interpolation) {

  LogMessage message(LogOrigin("TOpac","setInterpolation"));

  // Execute TVVJ version
  TimeVarVisJones::setInterpolation(interpolation);

  if (interpolation.isDefined("opacity")) 
    opacity_p=interpolation.asFloat("opacity");

  { ostringstream o; 
  o<< "  Elevation-dependent opacity corrections will be applied " << endl 
   << "  using a constant zenith opacity = " << opacity_p; 
  message.message(o);
  logSink().post(message);
  }

}

// Calculate matrix elements from opacity and elevation
void TOpac::calcAntGainPar(const VisBuffer& vb) {

  //  cout << " cAGP(TOpac) ";

  Int ispw=vb.spectralWindow();
  Double time=vb.time()(0);

  // Vector (over ants) of AZELs at time:
  Vector<MDirection> antazel(vb.azel(time));
  Int nant=static_cast<Int>(antazel.nelements());
  Int numAntennas = (nant < numberAnt_) ? nant : numberAnt_;

  for (Int iant=0; iant<numAntennas; iant++) {

    // Extract elevation for this antenna:
    //                       . azel as Vec<quanta> . index=1 value is elev
    Double elev=antazel(iant).getAngle().getValue()(1);

    // Calculate opacity loss
    (*thisJonesPar_[ispw])(0,0,iant)=Float(exp(-1.0 * opacity_p/ cos(C::pi_2-elev)));
    (*thisJonesOK_[ispw])(iant)=True;

  }

  // Current matrix caches now invalid
  invalidateJM(ispw);
  invalidateMM(ispw);

}

void TOpac::calcJonesPar(const Int& spw,const Double& time,const Int& field) {

  // Vector (over ants) of AZELs at time:
  Vector<MDirection> antazel(vs_->iter().azel(time));
  Int nant=static_cast<Int>(antazel.nelements());
  Int numAntennas = (nant < numberAnt_) ? nant : numberAnt_;
  
  for (Int iant=0; iant<numAntennas; iant++) {
    
    // Extract elevation for this antenna:
    //                       . azel as Vec<quanta> . index=1 value is elev
    Double elev=antazel(iant).getAngle().getValue()(1);
    
    // Calculate opacity loss
    (*thisJonesPar_[spw])(0,0,iant)=Float(exp(-1.0 * opacity_p/ cos(C::pi_2-elev)));
    (*thisJonesOK_[spw])(iant)=True;

  }

  // Current matrix caches now invalid
  invalidateJM(spw);
  invalidateMM(spw);

}

 
// ********************************************************************
// ********************************************************************

FVisJones::FVisJones(VisSet& vs) : SolvableVisJones(vs) {

  nPar_=2;

  jMatType_=1;
  required_.resize(2,2);
  required_=False;
  for (uInt id=0;id<2;id++) required_(id,id)=True;
};

FVisJones::~FVisJones() {}

void FVisJones::updateAntGain() {
  Bool phaseOnly=(mode().contains("phase"));

  // This is currently just T!!! 
  Complex lscalar;
  for (Int iant=0;iant<numberAnt_;iant++) {
    lscalar=(*thisJonesMat_[currentSpw_])(0,iant)(0,0);
    Float ggS=ggS_(0,iant)(0,0)+ggS_(0,iant)(1,1);
    if (ggS>0.0) {
      Complex gS=gS_(0,iant)(0,0)+gS_(0,iant)(1,1);
      lscalar-=gain()*gS/ggS;
    }
    if(phaseOnly&&abs(lscalar)>0.0) {
      lscalar/=abs(lscalar);
    }
    (*thisJonesMat_[currentSpw_])(0,iant)=lscalar;
  }

}


} //# NAMESPACE CASA - END

