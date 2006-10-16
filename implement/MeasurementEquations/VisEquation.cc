//# VisEquation:  Vis Equation
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

#include <synthesis/MeasurementEquations/VisEquation.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisTimeAverager.h>
#include <casa/Quanta/MVTime.h>
#include <casa/iostream.h>

#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ***************************************************************************
// ********************  Start of public member functions ********************
// ***************************************************************************

//----------------------------------------------------------------------
VisEquation::VisEquation(VisSet& vs) :
  vs_(&vs), xc_(0), mfm_(0), mm_(0), km_(0), mj_(0), kj_(0), 
  bj_(0), gj_(0), dj_(0), cj_(0), ej_(0), pj_(0), 
  tj_(0), ac_(0), vj_(0), svc_(0) {
};

//----------------------------------------------------------------------
VisEquation::~VisEquation() {}

//---------------------------------------------------------------------- 
VisEquation& VisEquation::operator=(const VisEquation& other)
{
  if(this!=&other) {
    mj_=other.mj_;
    mfm_=other.mfm_;
    mm_=other.mm_;
    km_=other.km_;
    kj_=other.kj_;
    gj_=other.gj_;
    bj_=other.bj_;
    dj_=other.dj_;
    cj_=other.cj_;
    ej_=other.ej_;
    pj_=other.pj_;
    tj_=other.tj_;
    ac_=other.ac_;
    xc_=other.xc_;
    vs_=other.vs_;
    vj_=other.vj_;
    svc_=other.svc_;
  };
  return *this;
};

//----------------------------------------------------------------------
VisEquation::VisEquation(const VisEquation& other)
{
  operator=(other);
}

//----------------------------------------------------------------------
void VisEquation::setVisSet(VisSet& vs) {
  vs_=&vs;
}
//----------------------------------------------------------------------
void VisEquation::setMJones(MJones& j) {mj_=&j;};
void VisEquation::setACoh(ACoh& c) {ac_=&c;};
void VisEquation::setXCorr(XCorr& x) {xc_=&x;};

//----------------------------------------------------------------------
void VisEquation::setVisJones(VisJones& j) {
  VisJones::Type t=j.type();
  switch(t) {
  case VisJones::K: 
    kj_=&j;
    break;
  case VisJones::B: 
    bj_=&j;
    break;
  case VisJones::G: 
    gj_=&j;
    break;
  case VisJones::D: 
    dj_=&j;
    break;
  case VisJones::C: 
    cj_=&j;
    break;
  case VisJones::E: 
    ej_=&j;
    break;
  case VisJones::P: 
    pj_=&j;
    break;
  case VisJones::T: 
    tj_=&j;
    break;
  default:
    break;
  }
};

//----------------------------------------------------------------------
void VisEquation::unSetVisJones(VisJones& j) {
  VisJones::Type t=j.type();
  switch(t) {
  case VisJones::K:
    kj_=0;
    break;
  case VisJones::B:
    bj_=0;
    break;
  case VisJones::G:
    gj_=0;
    break;
  case VisJones::D:
    dj_=0;
    break;
  case VisJones::C:
    cj_=0;
    break;
  case VisJones::E:
    ej_=0;
    break;
  case VisJones::P:
    pj_=0;
    break;
  case VisJones::T:
    tj_=0;
    break;
  default:
    break;
  }
};

//----------------------------------------------------------------------
void VisEquation::setVisCal(VisCal& vc) {
  VisCal::Type t=vc.type();
  switch(t) {
  case VisCal::Mf: 
    mfm_=&vc;
    break;
  case VisCal::M: 
    mm_=&vc;
    break;
  case VisCal::K: 
    km_=&vc;
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------
void VisEquation::unSetVisCal(VisCal& vc) {
  VisCal::Type t=vc.type();
  switch(t) {
  case VisCal::Mf: 
    mfm_= NULL;
    break;
  case VisCal::M: 
    mm_= NULL;
    break;
  case VisCal::K: 
    km_= NULL;
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------
// Correct the visibility set starting from the Observed visibilities
void VisEquation::correct() {
  AlwaysAssert(ok(),AipsError);
  // Loop over all visibilities
  VisIter& ioc(vs_->iter());
  VisBuffer ovb(ioc), cvb;
  for (ioc.originChunks();ioc.moreChunks();ioc.nextChunk()) {
    for (ioc.origin();ioc.more(); ioc++) {
      cvb=ovb;
      applyInv(cvb);
      ioc.setVis(cvb.visibility(), VisibilityIterator::Corrected);
      ioc.setFlag(cvb.flag());
    }
  }
}

//----------------------------------------------------------------------
// Corrupt the visibility set starting from the Observed visibilities
void VisEquation::corrupt() {
  AlwaysAssert(ok(),AipsError);
  // Loop over all visibilities
  VisIter& ioc(vs_->iter());
  VisBuffer ovb(ioc);
  for (ioc.originChunks();ioc.moreChunks();ioc.nextChunk()) {
    for (ioc.origin();ioc.more(); ioc++) {
      apply(ovb);
      ioc.setVis(ovb.visibility(),VisibilityIterator::Observed);
      ioc.setVis(ovb.visibility(),VisibilityIterator::Corrected);
      ioc.setWeight(ovb.weight());
      ioc.setSigma(ovb.sigma());
    }
  }
}


//----------------------------------------------------------------------
// Initialize the buffers for chi-square and gradient calculations
void VisEquation::initChiSquare(VisJones& vj) {
  
  //  cout << "VisEquation::initChiSquare(VisJones& vj)" << endl;

    AlwaysAssert(vj_==&vj,AipsError);

    // Call generic
    initChiSquare(vj.type(),vj.freqDep(),vj.prenorm(),vj.preavg());

}
  
void VisEquation::initChiSquare(SolvableVisCal& svc) {

  //  cout << "VisEquation::initChiSquare(SolvableVisCal& vj)" << endl;

    AlwaysAssert(svc_==&svc,AipsError);

    // Call generic:
    initChiSquare(svc.type(),svc.freqDepMat(),False,svc.preavg());

}

void VisEquation::initChiSquare(const Int& type,
				const Bool& freqdep,
				const Bool& prenorm,
				const Double& preavg) {

  //  cout << "VisEquation::initChiSquare(*****)" << endl;


  // Types of operators which can act on the Measurement Equation (ME):
  // i) VisJones calibration apply; ii) frequency average; 
  // Note: time averaging treated separately
  enum TermType{VISJONESAPPLY,VISCALAPPLY,FREQAVER};
  const Int nCal=11;

  // The type of the solved-for component
  Int stype(type);

  // Phase only?
  Bool phaseonly(False);

  if (svc_) 
    phaseonly=svc_->mode().contains("phase");
  else if (vj_)
    phaseonly=vj_->mode().contains("phase");
  
  Vector<Bool> freqDep(nCal,True);
  freqDep(stype)=freqdep;

  //  Timer timer;
  //  timer.mark();

  // VisJones components active in the ME
  PtrBlock<VisJones*> jones(nCal);

  // VisCal components active in the ME
  PtrBlock<VisCal*> vcal(nCal);

  // Accumulate LHS & RHS in these lists
  PtrBlock<VisJones*> vjLHS(nCal*2), vjRHS(nCal*2);
  PtrBlock<VisCal*>   vcLHS(nCal*2), vcRHS(nCal*2);

  Block<Int> termTypeLHS(nCal*2), termTypeRHS(nCal*2);

  // Hard-code the form of the ME

  //   the VisJones:
  jones[VisJones::UVMOD] = NULL;
  jones[VisJones::Mf] = NULL;
  jones[VisJones::M]  = NULL;
  jones[VisJones::K]  = NULL;
  jones[VisJones::B]  = bj_;
  jones[VisJones::G]  = gj_;
  jones[VisJones::D]  = dj_;
  jones[VisJones::C]  = cj_;
  jones[VisJones::E]  = ej_;
  jones[VisJones::P]  = pj_;
  jones[VisJones::T]  = tj_;

  //   the VisCals:
  vcal[VisCal::UVMOD] = NULL;
  vcal[VisCal::Mf] = mfm_;
  vcal[VisCal::M]  = mm_;
  vcal[VisCal::K]  = km_;
  vcal[VisCal::B]  = NULL;
  vcal[VisCal::G]  = NULL;
  vcal[VisCal::D]  = NULL;
  vcal[VisCal::C]  = NULL;
  vcal[VisCal::E]  = NULL;
  vcal[VisCal::P]  = NULL;
  vcal[VisCal::T]  = NULL;

  // Determine the frequency dependence along the ME, first from
  // the current VisJones to the right. Retain frequency dependence
  // once it is encountered.

  for (Int i=stype+1; i<nCal; i++) {
    if (jones[i]) {
      freqDep(i)=freqDep(i-1) || jones[i]->freqDep();
    } else if (vcal[i]) {
      freqDep(i)=freqDep(i-1) || vcal[i]->freqDepMat();
    } else {
      freqDep(i)=freqDep(i-1);
    }
  }
  // Now handle part of equation from current VisJones to the left 
  for (Int i=stype-1; i>=0; i--) {
    if (jones[i]) {
      freqDep(i)=freqDep(i+1) || jones[i]->freqDep();
    } else if (vcal[i]) {
      freqDep(i)=freqDep(i+1) || vcal[i]->freqDepMat();
    } else {
      freqDep(i)=freqDep(i+1);
    }
  }

  // Set up the new equation, with operators inserted
  Int nTermRHS=0;
  for (Int i=nCal-1; i>=stype; i--) {
    if (!freqDep(i) && (i==nCal-1 || freqDep(i+1))) {
      // Add frequency averaging operator
      vjRHS[nTermRHS]=NULL; 
      termTypeRHS[nTermRHS]=FREQAVER;
      nTermRHS++;
    }
    if (i>stype) {
      if (jones[i] && jones[i]->isApplied()) {
	// If VisJones is present, then insert a calibration apply operator
	vjRHS[nTermRHS]=jones[i];
	termTypeRHS[nTermRHS]=VISJONESAPPLY;
	nTermRHS++;
      } else if (vcal[i] && vcal[i]->isApplied()) {
	// If VisCal is present, then insert a calibration apply operator
	vcRHS[nTermRHS]=vcal[i];
	termTypeRHS[nTermRHS]=VISCALAPPLY;
	nTermRHS++;
      }
    }
  };

  // Set up the new equation, with operators inserted
  Int nTermLHS=0;
  for (Int i=0; i<=stype; i++) {
    if (!freqDep(i) && (i==0 || freqDep(i-1))) {
      // Add frequency averaging operator
      vjLHS[nTermLHS]=NULL;
      termTypeLHS[nTermLHS]=FREQAVER;
      nTermLHS++;
    }
    if (i<=stype) {
      if (jones[i] && jones[i]->isApplied()) {
	// if VisJones is present, insert calibration apply operator
	vjLHS[nTermLHS]=jones[i];
	termTypeLHS[nTermLHS]=VISJONESAPPLY;
	nTermLHS++;
      } else if (vcal[i] && vcal[i]->isApplied()) {
	// if VisCal is present, insert calibration apply operator
	vcLHS[nTermLHS]=vcal[i];
	termTypeLHS[nTermLHS]=VISCALAPPLY;
	nTermLHS++;
      }
    }
  }

  //   This is useful for debugging the M.E. sorting:
/*
  cout << "The ME is: " << endl;
  cout << " nRHS = " << nTermRHS << endl;
  cout << " RHS:  ";
  for(Int i=0; i<nTermRHS; i++) {
    if (termTypeRHS[i]==VISJONESAPPLY) {
      cout << vjRHS[i]->type() << " ";
    };
    if (termTypeRHS[i]==VISCALAPPLY) {
      cout << vcRHS[i]->typeName() << " ";
    };
    if (termTypeRHS[i]==FREQAVER) {
      cout << "fA ";
    };
  };
  cout << endl;

  cout << " Solving for: " << stype << endl;

  cout << " nLHS = " << nTermLHS << endl;
  cout << " LHS:  ";
  for(Int i=0; i<nTermLHS; i++) {
    if (termTypeLHS[i]==VISJONESAPPLY) {
      cout << vjLHS[i]->type() << " ";
    };
    if (termTypeLHS[i]==VISCALAPPLY) {
      cout << vcLHS[i]->type() << " ";
    };
    if (termTypeLHS[i]==FREQAVER) {
      cout << "fA ";
    };
  };    
  cout << endl;
*/

  // Insert a time averager just to the right of the VisJones
  // being solved for.
  VisTimeAverager* vtaRHS=new VisTimeAverager(vs_->numberAnt(),preavg,prenorm);

  // Insert a time averager just to the left of the VisJones
  // being solved for.
  VisTimeAverager* vtaLHS=new VisTimeAverager(vs_->numberAnt(),preavg,prenorm);

  // Attach to current VisIter
  ROVisIter& vi(vs_->iter());

  vi.origin();
  VisBuffer vbRHS(vi);
  Int icount=0;

  // Iterate through the current VisibilityIterator chunk
  while (vi.more()) {

    // Make copy of current VisBuffer for LHS:
    VisBuffer vbLHS(vbRHS);

    // Evaluate right hand side of equation (corrupt the model visibilities)
    // include all effects up to but not including the current visjones

    // Copy the model visibility to the data column (because
    //   that is where apply works)
    vbRHS.visibility()=vbRHS.modelVisibility();

    // Apply the ME operators to RHS
    for(Int i=0; i<nTermRHS; i++) {
      if (termTypeRHS[i]==VISJONESAPPLY) {
	vjRHS[i]->apply(vbRHS);
      };
      if (termTypeRHS[i]==VISCALAPPLY) {
	vcRHS[i]->apply(vbRHS);
      };
      if (termTypeRHS[i]==FREQAVER) {
	vbRHS.freqAverage(); 
      };
    };

    // Evaluate left hand side of eqn (correct the observed visibilities)
    // include all effects up to but not including the current VisJones

    // Apply the ME operators to LHS
    for(Int i=0; i<nTermLHS; i++) {
      if (termTypeLHS[i]==VISJONESAPPLY) {
	vjLHS[i]->applyInv(vbLHS); 
      };
      if (termTypeLHS[i]==VISCALAPPLY) {
	vcLHS[i]->applyInv(vbLHS); 
      };
      if (termTypeLHS[i]==FREQAVER) {
	vbLHS.freqAverage(); 
      };
    };    

    // avoid ACs and zero data, move time-dep phase to LHS
    AlwaysAssert(vbLHS.nRow()==vbRHS.nRow(),AipsError);
    AlwaysAssert(vbLHS.nChannel()==vbRHS.nChannel(),AipsError);
    for (Int row=0; row<vbRHS.nRow(); row++) {
      Bool autoCorrelation = 
	((vbRHS.antenna1()(row)==vbRHS.antenna2()(row)) ||
	 (vbLHS.antenna1()(row)==vbLHS.antenna2()(row)));

      // if data not already flagged
      if (!vbLHS.flagRow()(row)&&!vbRHS.flagRow()(row)) {
        for (Int channel=0; channel<vbRHS.nChannel(); channel++) {

          // if channel not already flagged...
          if (!vbLHS.flag()(channel,row) && !vbRHS.flag()(channel,row)) {
            // ...and, not AC and neither norm=0
	    if (!autoCorrelation && 
                norm(vbLHS.visibility()(channel,row))!=0.0 &&
                norm(vbRHS.visibility()(channel,row))!=0.0 ) {
	      // multiply both sides by normalized conj(vbRHS)
              //  NB: We do this to move all terms with any phase time-dependence  
              //      to LHS, where, presumably, it will cancel in the net product. 
              //      This ensures that the time average won't lose coherence, and
	      //      always allows full time averaging within the solution interval,
              //      which will optimizes performance of the solver.
              // must loop over CStokesVector

              if (stype!=VisJones::D) {
		CStokesVector& vRHS=vbRHS.visibility()(channel,row);
		CStokesVector& vLHS=vbLHS.visibility()(channel,row);

		for (uInt i=0;i<4;i++) {
		  Complex factor(0,0);
		  Float aRHS=abs(vRHS(i));
		  if (abs(aRHS)>0)
		    factor=conj(vRHS(i))/aRHS/aRHS;
		  vRHS(i)*=factor;
		  vLHS(i)*=factor;
		};
	      };
            } else {
              // set flag
              vbLHS.flag()(channel,row)=vbRHS.flag()(channel,row)=True;
	    }
	  }
        }
      } else {
        // ensure both flagRow() set
        vbLHS.flagRow()(row)=vbRHS.flagRow()(row)=True;
      }

    }

    // Accumulate the VisBuffer in time
    vtaRHS->accumulate(vbRHS);
    // Accumulate the VisBuffer in time
    vtaLHS->accumulate(vbLHS);
    // Iterate to the next VisBuffer in the chunk
    vi++;
    icount++;
  }

  // Finalize the time average into solver's VisBuffers
  vtaRHS->finalizeAverage(corruptedvb);
  vtaLHS->finalizeAverage(correctedvb);
  
  // Delete the time averagers
  delete vtaRHS;
  delete vtaLHS;

  // Remove unnecessary cols from the vbs:
  corruptedvb.removeScratchCols();
  correctedvb.removeScratchCols();

  // Make correctedvb see model visbilities from corruptedvb
  correctedvb.refModelVis(corruptedvb.visibility());

  if (phaseonly) {

    Int nrow=correctedvb.nRow();
    Int nchan=correctedvb.nChannel();

    for (Int irow=0; irow<nrow; irow++) {

      if (!correctedvb.flagRow()(irow)) {

	for (Int ichan=0;ichan<nchan;ichan++) {

	  if (!correctedvb.flag()(ichan,irow)) {

	    CStokesVector& vis=correctedvb.visibility()(ichan,irow);
	    CStokesVector& mod=correctedvb.modelVisibility()(ichan,irow);
	    for (Int i=0;i<4;i++) {
	      Float vamp=abs(vis(i));
	      if (vamp>0.0) vis(i)/=vamp;
	      Float mamp=abs(mod(i));
	      if (mamp>0.0) mod(i)/=mamp;
	    }
	  }
	}
      }
    }
  }
    
  //  cout << "End of VE::initChiSquare" << endl;

  // Report number of rows in solvers VisBuffers:
  //  cout << "Nrows = " << correctedvb.nRow() << " " << corruptedvb.nRow() << endl;

  //  cout << "New Timing: " << timer.user() << " " << timer.system() << " " << timer.real() << endl;
}

//----------------------------------------------------------------------
// fix up the weights to reflect the sigmas
void VisEquation::fixWeights(VisBuffer& vb) {
  if (vb.nRow()>0) {
    Vector<Float>& wt=vb.weight(); 
    wt=vb.sigma();
    for (Int row=0; row<vb.nRow(); row++) {
      if(wt(row)>0.0) {
	wt(row)=1.0/square(wt(row));
      }
      else {
	wt(row)=1.0;
      }
    }
  }
}

//----------------------------------------------------------------------
// Chi-squared per spectral window and antenna 
void VisEquation::chiSquared(Matrix<Float>& iChisq, Matrix<Float>& iSumwt) {

  AlwaysAssert(ok(),AipsError);

  Int nChan=correctedvb.nChannel();
  Int nRow=correctedvb.nRow();

  VisBuffer rvb=corruptedvb;
  vj_->apply(rvb);
  // Residual Coherence
  rvb-=correctedvb;

  Int spw=rvb.spectralWindow();
  iChisq.column(spw)=0.0; iSumwt.column(spw)=0.0;

  for (Int row=0; row<nRow; row++) {
    if (!rvb.flagRow()(row)) {
      Int ant1=rvb.antenna1()(row);
      Int ant2=rvb.antenna2()(row);
      for (Int chn=0; chn<nChan; chn++) {
        if (!rvb.flag()(chn,row)) {
	  CStokesVector& vr=rvb.visibility()(chn,row);
	  Float wt=rvb.weight()(row);
	  iChisq(ant1,spw)+=wt*real(innerProduct(vr,vr));
	  iSumwt(ant1,spw)+=2*wt;
	  iChisq(ant2,spw)+=wt*real(innerProduct(vr,vr));
	  iSumwt(ant2,spw)+=2*wt;
        }
      }
    }
  }
}
//----------------------------------------------------------------------
// Chi-squared per antenna 
void VisEquation::chiSquared(Vector<Float>& iChisq, Vector<Float>& iSumwt) {

  AlwaysAssert(ok(),AipsError);

  Int nChan=correctedvb.nChannel();
  Int nRow=correctedvb.nRow();

  VisBuffer rvb=corruptedvb;
  vj_->apply(rvb);
  // Residual Coherence
  rvb-=correctedvb;

  iChisq=0.0; iSumwt=0.0;

  for (Int row=0; row<nRow; row++) {
    if (!rvb.flagRow()(row)) {
      Int ant1=rvb.antenna1()(row);
      Int ant2=rvb.antenna2()(row);
      for (Int chn=0; chn<nChan; chn++) {
        if (!rvb.flag()(chn,row)) {
	  CStokesVector& vr=rvb.visibility()(chn,row);
	  Float wt=rvb.weight()(row);
	  iChisq(ant1)+=wt*real(innerProduct(vr,vr));
	  iSumwt(ant1)+=2*wt;
	  iChisq(ant2)+=wt*real(innerProduct(vr,vr));
	  iSumwt(ant2)+=2*wt;
        }
      }
    }
  }
}
//----------------------------------------------------------------------
// Gradient and Hessian of Chisq with respect to a specified Jones matrix
// The gradient and Hessian are stored in the Jones matrix. This form is
// fully general and can apply to matrices of any symmetry because all
// possible gradients can be calculated.
void VisEquation::gradientsChiSquared(const Matrix<Bool>& required,
				      VisJones& vj) {

  AlwaysAssert(ok(),AipsError);
  AlwaysAssert(vj_==&vj,AipsError);

  // Set this Jones Matrix to be a free variable
  vj.setFree();

  // Initialize the gradients stored in the Jones Matrix
  vj.initializeGradients();

  Int nChan=corruptedvb.nChannel();
  Int nRow=corruptedvb.nRow();
  Vector<SquareMatrix<Complex,2> > gs(nChan);
  Vector<SquareMatrix<Float,2> > ggs(nChan);
  Vector<Float> chisq(nChan),sumwt(nChan);

  Matrix<Complex> zeroC(2,2); zeroC=Complex(0.,0.);
  Matrix<Float> zeroF(2,2); zeroF=0.;

  VisBuffer rvb=corruptedvb;
  vj.apply(rvb);	        // Predicted Observed Coherence
  rvb-=correctedvb;

  VisBuffer gradvb[2][2][2];
  uInt i,j;
  for (i=0;i<2;i++) {
    for (j=0;j<2;j++) {
      if (required(i,j)) {
	gradvb[i][j][0]=corruptedvb;
	vj_->applyGradient(gradvb[i][j][0],corruptedvb.antenna2(),i,j,0);
	gradvb[i][j][1]=corruptedvb;
	vj_->applyGradient(gradvb[i][j][1],corruptedvb.antenna1(),i,j,1);
      }
    }
  }
  // At this point, the model visibility will be flagged if the
  // model visibililty amplitude is zero, and the residual
  // visibility will be flagged if any of the gain applications
  // are invalid.
  chisq=0.0; sumwt=0.0; gs=zeroC; ggs=zeroF;
  for (Int row=0; row<nRow; row++) {
    if (!rvb.flagRow()(row)) {
      Float wt=rvb.weight()(row);
      Int chn;
      for (chn=0; chn<nChan; chn++) {
        if (!rvb.flag()(chn,row)) {
	    CStokesVector& vr=rvb.visibility()(chn,row);
	  chisq(chn)=wt*real(innerProduct(vr,vr));
	  sumwt(chn)=2*wt;
	  for (i=0;i<2;i++) {
	    for (j=0;j<2;j++) {
	      if (required(i,j)) {
	        CStokesVector& vgrad=gradvb[i][j][0].visibility()(chn,row);
	        gs(chn)(i,j)=2*wt*innerProduct(vr,vgrad);
		ggs(chn)(i,j)=2*wt*real(innerProduct(vgrad,vgrad));
	      }
	    }
	  }
        }
      }
      vj.addGradients(rvb,row,rvb.antenna1()(row),sumwt, chisq, gs, ggs);
      for (chn=0; chn<nChan; chn++) {
        if (!rvb.flag()(chn,row)) {
	  CStokesVector& vr=rvb.visibility()(chn,row);
	  for (i=0;i<2;i++) {
	    for (j=0;j<2;j++) {
	      if (required(i,j)) {
		CStokesVector& vgrad=gradvb[i][j][1].visibility()(chn,row);
	        gs(chn)(i,j)=2*wt*conj(innerProduct(vr,vgrad));
		ggs(chn)(i,j)=2*wt*real(innerProduct(vgrad,vgrad));
	      }
	    }
	  }
        }
      }
      vj.addGradients(rvb,row,rvb.antenna2()(row), sumwt, chisq, gs, ggs);
    }
  }
  // Finish off any calculations needed
  vj.finalizeGradients();

  // Set the jones matrix to be fixed again.
  vj.setNotFree();
}
//----------------------------------------------------------------------
// Gradient and Hessian of Chisq with respect to a specified Jones matrix
// The gradient and Hessian are stored in the Jones matrix. 
// Only diagonal terms are calculated.
void VisEquation::gradientsChiSquared(MJones& mj) {

  AlwaysAssert(ok(),AipsError);

  // Set this Jones Matrix to be a free variable
  mj.setFree();

  // Initialize the gradients stored in the Jones Matrix
  mj.initializeGradients();

  Cube<Complex> gs, cgs;
  Cube<Float> ggs;
  Vector<Float> chisq, sumwt;

  ROVisIter& ioc(vs_->iter());
  VisBuffer ovb(ioc),mvb;
  Int lastnChan=0;
  // Loop over all visibilities
  for (ioc.origin(); ioc.more(); ioc++) { 
    mvb=ovb;
    mvb.visibility()=ovb.modelVisibility();
    Int nChan=ovb.nChannel();
    Int nRow=ovb.nRow();

    // resize accumulation buffers
    if (nChan!=lastnChan) {
      gs.resize(4,4,nChan); cgs(4,4,nChan); ggs.resize(4,4,nChan);
      chisq.resize(nChan); sumwt.resize(nChan);
       lastnChan=nChan;
    }
    chisq=0.0; sumwt=0.0; gs=Complex(0.,0.); cgs=Complex(0.,0.); ggs=0.0;
 
    Vector<Float> wt=ovb.sigma();
    Int row;
    for (row=0; row<nRow; row++) {
      if(wt(row)>0.0) {
	wt(row)=1.0/square(wt(row));
      }
      else {
	wt(row)=1.0;
      }
    }

    for (row=0; row<nRow; row++) {
      for (Int chn=0; chn<nChan; chn++) {
	if (!mvb.flag()(chn,row)) 
	  mvb.flag()(chn,row)=norm(ovb.visibility()(chn,row))==0.0;
      }
    }

    VisBuffer rvb=mvb;		// Model Coherence
    apply(rvb);		// Predicted Observed Coherence
    for (row=0; row<nRow; row++) {
      for (Int chn=0; chn<nChan; chn++) {
	if (!mvb.flag()(chn,row)) {
	  mvb.flag()(chn,row)=norm(mvb.visibility()(chn,row))==0.0;
	  if (!mvb.flag()(chn,row)) 
	    rvb.visibility()(chn,row)-=ovb.visibility()(chn,row);
	  // Residual  Observed Coherence
	}
      }
    }
    VisBuffer gradvb=mvb;
    applyGradient(gradvb);
    for (row=0; row<nRow; row++) {
      for (Int chn=0; chn<nChan; chn++) {
	if (!mvb.flag()(chn,row)) {
	  CStokesVector& vr=rvb.visibility()(chn,row);
	  CStokesVector& vm=gradvb.visibility()(chn,row);
	  chisq(chn)=wt(row)*real(innerProduct(vr,vr));
	  sumwt(chn)=2*wt(row);
	  for (uInt i=0;i<4;i++) {
	    gs(i,i,chn)=2*wt(row)*vr(i)*conj(vm(i));
	    cgs(i,i,chn)=conj(gs(i,i,chn));
	    ggs(i,i,chn)=2*wt(row)*real(vm(i)*conj(vm(i)));
	  }
	}
      }
      mj.addGradients(mvb, row, mvb.antenna1()(row), mvb.antenna2()(row),
		      sumwt,chisq, gs, ggs);
      mj.addGradients(mvb, row, mvb.antenna2()(row), mvb.antenna1()(row),
		      sumwt,chisq, cgs, ggs);
    }
  }

  // Finish off any calculations needed
  mj.finalizeGradients();

  // Set the jones matrix to be fixed again.
  mj.setNotFree();
}

//----------------------------------------------------------------------
// Solve for the VisJones
Bool VisEquation::solve(VisJones& vj) {
  // Ensure svc_=NULL;
  svc_=NULL;
  // point to solved-for VJ
  vj_ = &vj;
  return vj.solve(*this);
}

//----------------------------------------------------------------------
// Solve for the VisCal
Bool VisEquation::solve(SolvableVisCal& svc) {
  // Ensure vj_=NULL;
  vj_=NULL;
  // point to solved-for VJ
  svc_ = &svc;
  return svc.solve(*this);
}

//----------------------------------------------------------------------
// Solve for the MJones
Bool VisEquation::solve(MJones& mj) {
  setMJones(mj);
  return mj.solve(*this);
}
//----------------------------------------------------------------------
// Solve for the ACoh
Bool VisEquation::solve(ACoh& ac) {
  setACoh(ac);
  return ac.solve(*this);
}

//----------------------------------------------------------------------
// ***************************************************************************
// ********************  Start of protected member functions *****************
// ***************************************************************************

// Apply VisEquation
VisBuffer& VisEquation::apply(VisBuffer& result) {
  if(tj_)  tj_->apply(result);   // Atmospheric absorption/phase
  if(pj_)  pj_->apply(result);   // Parallactic angle
  if(ej_)  ej_->apply(result);   // Gain Curve
  if(cj_)  cj_->apply(result);   // Configuration
  if(dj_)  dj_->apply(result);   // Leakage
  if(gj_)  gj_->apply(result);   // Gain
  if(bj_)  bj_->apply(result);   // Bandpass
  if(kj_)  kj_->apply(result);   // Phase tracking
  if(km_)  km_->apply(result);   // Phase tracking (non-closing)
  if(mm_)  mm_->apply(result);   // Multiplicative closure errors
  if(mfm_) mfm_->apply(result);  // Freq-dep Multiplicative closure errors
  if(mj_)  mj_->apply(result);   // Closure errors
  if(ac_)  ac_->apply(result);   // Additive error
  if(xc_)  xc_->apply(result);   // Non-linear correlator function
  return result;
};

// Apply Inverse of VisEquation
VisBuffer& VisEquation::applyInv(VisBuffer& result) {

  if(xc_)  xc_->applyInv(result);   // Non-linear correlator function
  if(ac_)  ac_->applyInv(result);   // Additive error (inverse is minus)
  if(mj_)  mj_->applyInv(result);   // Closure errors
  if(mfm_) mfm_->applyInv(result);  // Freq-dep Multiplicative closure errors
  if(mm_)  mm_->applyInv(result);   // Multiplicative closure errors
  if(km_)  km_->applyInv(result);   // Phase tracking (non-closing)
  if(kj_)  kj_->applyInv(result);   // Phase tracking
  if(gj_)  gj_->applyInv(result);   // Gain
  if(bj_)  bj_->applyInv(result);   // Bandpass
  if(dj_)  dj_->applyInv(result);   // Leakage
  if(cj_)  cj_->applyInv(result);   // Configuration
  if(ej_)  ej_->applyInv(result);   // Gain Curve
  if(pj_)  pj_->applyInv(result);   // Parallactic angle
  if(tj_)  tj_->applyInv(result);   // Atmospheric absorption/phase
  return result;
  
};

// Apply gradient of VisEquation. Note that currently only G and
// D matrices can be solved for. This can be easily
// changed.
VisBuffer& VisEquation::applyGradient(VisBuffer& result,
				      const Vector<Int>& antenna, 
				      Int i, Int j, Int pos) 
{
  if(tj_) {
    if (tj_->free()) tj_->applyGradient(result, antenna, i, j, pos);
    else tj_->apply(result);
  };
  if(pj_)  pj_->apply(result);   // Parallactic angle
  if(ej_)  ej_->apply(result);   // Gain Curve
  if(cj_)  cj_->apply(result);   // Configuration
  if(dj_) {
    if (dj_->free()) dj_->applyGradient(result, antenna, i, j, pos);
    else dj_->apply(result);
  };
  if(gj_) {
    if (gj_->free()) gj_->applyGradient(result, antenna, i, j, pos);
    else gj_->apply(result);
  };
  if(bj_) {
    if (bj_->free()) bj_->applyGradient(result, antenna, i, j, pos);
    else bj_->apply(result);
  };
  if(kj_)  kj_->apply(result);    // Phase tracking
  if(mj_)  mj_->apply(result);    // Closure errors
  if(ac_)  ac_->apply(result);   // Additive error
  // Note that we neglect for the moment the derivative of XCorr i.e.
  // it should be roughly unity except in the worse cases
  if(xc_)  xc_->apply(result);   // Non-linear correlator function
  return result;
  
};

// Apply gradient of VisEquation with respect to MJones
VisBuffer& VisEquation::applyGradient(VisBuffer& result) {
  
  if(tj_)  tj_->apply(result);   // Atmospheric absorption/phase
  if(pj_)  pj_->apply(result);   // Parallactic angle
  if(ej_)  ej_->apply(result);   // Gain Curve
  if(cj_)  cj_->apply(result);   // Configuration
  if(dj_)  dj_->apply(result);   // Leakage
  if(bj_)  bj_->apply(result);   // Bandpass
  if(gj_)  gj_->apply(result);   // Gain
  if(kj_)  kj_->apply(result);   // Phase tracking

  return result;
};

Bool VisEquation::ok() {

  return(True);
}


} //# NAMESPACE CASA - END

