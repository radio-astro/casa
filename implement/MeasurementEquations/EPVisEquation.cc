//-*-C++-*-
//# EPVisEquation:  Modified Vis Equation
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

#include <synthesis/MeasurementEquations/EPVisEquation.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
//#include <msvis/MSVis/VisTimeAverager.h>
#include <msvis/MSVis/EPVisTimeAverager.h>
#include <casa/Quanta/MVTime.h>
#include <casa/iostream.h>

#include <casa/OS/Timer.h>

namespace casa {

// ***************************************************************************
// ********************  Start of public member functions ********************
// ***************************************************************************

//----------------------------------------------------------------------
EPVisEquation::EPVisEquation(VisSet& vs) :
  VisEquation(vs) 
{
};

//----------------------------------------------------------------------
EPVisEquation::~EPVisEquation() {}

//---------------------------------------------------------------------- 
EPVisEquation& EPVisEquation::operator=(const EPVisEquation& other)
{
  if(this!=&other) {
    mj_=other.mj_;
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
  };
  return *this;
};

//----------------------------------------------------------------------
EPVisEquation::EPVisEquation(const EPVisEquation& other)
  :VisEquation(other)
{
  operator=(other);
}

//----------------------------------------------------------------------
void EPVisEquation::getResAndGrad(EPJones& EPVJ,
				  VisBuffer& VRes,
				  VisBuffer& grad1,
				  VisBuffer& grad2)
{
  EPVisTimeAverager avgRes(vs_->numberAnt(), EPVJ.preavg(),
			   EPVJ.prenorm(),True,
			   EPVisTimeAverager::MODELVISCUBE);
  EPVisTimeAverager avgGrad1(vs_->numberAnt(), EPVJ.preavg(),
			     EPVJ.prenorm(),True,
			     EPVisTimeAverager::MODELVISCUBE);
  //			     EPVisTimeAverager::VISCUBE);
  EPVisTimeAverager avgGrad2(vs_->numberAnt(), EPVJ.preavg(),
			     EPVJ.prenorm(),True,
			     EPVisTimeAverager::MODELVISCUBE);
  //			     EPVisTimeAverager::VISCUBE);

  //  grad1 = grad2 = VRes = corruptedvb;
  ROVisIter& vi(vs_->iter());
  VisBuffer vb(vi),dvb1,dvb2,tResVB;
  vi.origin();

  IPosition shape(vb.visCube().shape());
  grad1.modelVisCube().resize(shape);
  grad2.modelVisCube().resize(shape);
  VRes.modelVisCube().resize(shape);
  grad1.modelVisCube() = grad2.modelVisCube() = VRes.modelVisCube() = Complex(0,0);

      Cube<Complex> r0,r1;
      Complex sum0,sum1;
      sum0 = sum1 = Complex(0,0);

  while(vi.more())
    {
      dvb1 = dvb2 = vb;
      //
      // Save the corrected vis. in a temp. buffer
      //
      tResVB.modelVisCube() = vb.correctedVisCube();
      //
      // Predict the corrupted vis in vb.modelVisCube().  Also predict
      // the derivatives.
      //
      EPVJ.apply(vb,dvb1,dvb2);

      int N=vb.nRow();
      r0=vb.modelVisCube();
      r1=tResVB.modelVisCube();
      for(IPosition ndx(3,0);ndx(2)<N;ndx(2)++)
	{
// 	  sum0 += vb.modelVisCube()(ndx);
// 	  sum1 += tResVB.modelVisCube()(ndx);
	  /*
	  cout << "Predicted: " << ndx(2) << " " 
	       << abs(tResVB.modelVisCube()(ndx)) << " "
	       << arg(tResVB.modelVisCube()(ndx))*57.295 << " " 
	       << abs(vb.modelVisCube()(ndx)) << " "
	       << arg(vb.modelVisCube()(ndx))*57.295 << " "
	       << abs(r(ndx)) << " " << arg(r(ndx))*57.295 << " " 
	       << vb.flagRow()(ndx(2)) << " "
	       << vb.weight()(ndx(2)) << " "
	       << endl;
	  */
	}

      //
      // Compute the residual vis in vb.modelVisCube
      //
      vb.modelVisCube() -= tResVB.modelVisCube();
      //
      // Accumulate the residual vis and the derivatives (in
      // {vb,dvb1,dvb2}.modelVisCube)
      //
      avgRes.accumulate(vb);
      avgGrad1.accumulate(dvb1);
      avgGrad2.accumulate(dvb2);
      /*
      VRes.modelVisCube() += vb.modelVisCube();
      grad1.modelVisCube() += dvb1.modelVisCube();
      grad2.modelVisCube() += dvb2.modelVisCube();
      */
      vi++;
    }
  //      cout << "Sums = " << sum0 << " " << sum1 << endl;
  //
  // Normalize and return the accumulated buffers
  //
  avgRes.finalizeAverage(VRes);
  avgGrad1.finalizeAverage(grad1);
  avgGrad2.finalizeAverage(grad2);
}
//
//----------------------------------------------------------------------
//
void EPVisEquation::predict(EPJones& EPVJ, VisBuffer& VMod)
{
  VMod = corruptedvb;

  IPosition shape(corruptedvb.visCube().shape());
  VMod.modelVisCube().resize(shape);

  EPVJ.apply(VMod,0);
}
//
//----------------------------------------------------------------------
// Initialize the buffers for chi-square and gradient calculations
//
void EPVisEquation::initChiSquare(EPJones& vj) {

  //  Timer timer;
  //  timer.mark();

  // TODO: add in MJONES, XCORR and ACOH terms

  // Types of operators which can act on the Measurement Equation (ME):
  // i) VisJones calibration apply; ii) frequency average; 
  // Note: time averaging treated separately
  enum TermType{VISJONESAPPLY,FREQAVER};
  const Int nVisJones=13;

  PtrBlock<VisJones*> termLHS(nVisJones*2), termRHS(nVisJones*2);
  Block<Int> termTypeLHS(nVisJones*2), termTypeRHS(nVisJones*2);

  // VisJones components active in the ME, and their frequency dependence
  PtrBlock<VisJones*> jones(nVisJones);
  Vector<Bool> freqDep(nVisJones,True);

  // Hard-code the form of the ME
  jones = NULL;
    /*
  jones[VisJones::K] = NULL;//kj_;
  jones[VisJones::B] = NULL;//bj_;
  jones[VisJones::G] = NULL;//gj_;
  jones[VisJones::D] = NULL;//dj_;
  jones[VisJones::C] = NULL;//cj_;
  jones[VisJones::E] = NULL;//ej_;
  jones[VisJones::P] = NULL;//pj_;
  jones[VisJones::T] = NULL;//tj_;
    */
  jones[VisJones::EP] = epj_;

  // Save the current VisJones for use in the chiSquare calculation
  vj_=&vj;

  // Determine the frequency dependence along the ME, first from
  // the current VisJones to the right. Retain frequency dependence
  // once it is encountered.
  freqDep(vj.type())=vj.freqDep();

  for (Int i=vj.type()+1; i<nVisJones; i++) {
    if (jones[i]) {
      freqDep(i)=freqDep(i-1) || jones[i]->freqDep();
    } else {
      freqDep(i)=freqDep(i-1);
    }
  }
  // Now handle part of equation from current VisJones to the left 
  for (Int i=vj.type()-1; i>=0; i--) {
    if (jones[i]) {
     freqDep(i)=freqDep(i+1) || jones[i]->freqDep();
    } else {
      freqDep(i)=freqDep(i+1);
    }
  }

  Int nTermRHS=0;

  // Set up the new equation, with operators inserted
  for (Int i=nVisJones-1; i>=vj.type(); i--) {
    if (!freqDep(i) && (i==nVisJones-1 || freqDep(i+1))) {
      // Add frequency averaging operator
      termRHS[nTermRHS]=NULL; 
      termTypeRHS[nTermRHS]=FREQAVER;
      nTermRHS++;
    }
    if (i>vj.type() && jones[i] && jones[i]->isApplied()) {
      // If VisJones is present, then insert a calibration apply operator
      termRHS[nTermRHS]=jones[i];
      termTypeRHS[nTermRHS]=VISJONESAPPLY;
      nTermRHS++;
    }
  };

  // Set up the new equation, with operators inserted
  Int nTermLHS=0;
  for (Int i=0; i<=vj.type(); i++) {
    if (!freqDep(i) && (i==0 || freqDep(i-1))) {
      // Add frequency averaging operator
      termLHS[nTermLHS]=NULL;
      termTypeLHS[nTermLHS]=FREQAVER;
      nTermLHS++;
    }
    if (i<=vj.type() && jones[i] && jones[i]->isApplied()) {
      // if VisJones is present, insert calibration apply operator
      termLHS[nTermLHS]=jones[i];
      termTypeLHS[nTermLHS]=VISJONESAPPLY;
      nTermLHS++;
    }
  }

  // Insert a time averager just to the right of the VisJones
  // being solved for.
  //
  // Average the observed data.
  EPVisTimeAverager* vtaRHS=new EPVisTimeAverager(vs_->numberAnt(), vj.preavg(),
				 	      vj.prenorm(),True,
					      //VisTimeAverager::MODELVISCUBE);
					      EPVisTimeAverager::VISCUBE);

  // Insert a time averager just to the left of the VisJones
  // being solved for.
  EPVisTimeAverager* vtaLHS=new EPVisTimeAverager(vs_->numberAnt(), vj.preavg(), 
                                              vj.prenorm(),True,
					      //VisTimeAverager::MODELVISCUBE);
					      EPVisTimeAverager::VISCUBE);


  // Attach to current VisIter
  ROVisIter& vi(vs_->iter());
    vi.origin();
    /*
    cout << "####### " << vi.nRowChunk() << endl;
    {
      Int n=0,nr=0;
      nr=0;n=0;
      while(vi.more())
	{
	  VisBuffer tvb(vi);
	  nr += tvb.nRow();
	  vi++;n++;
	}
      cout << "EPVEq:initChiSq: No. of integrations = " << n << " No. of rows = " << nr << endl;
    }
    */
    return;
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

    //    vbRHS.modelVisibility()=vbRHS.modelVisibility();

    //    cout << "EVP:1 " << vbRHS.nRow() << " " << corruptedvb.nRow() << endl;

    //    cout << "EVP:Time " << vbRHS.time()-4.60203e+9 << endl;
    /*
      // !!!!!!!Confused about how to apply and applyInv before
      // averaging in time.

    // Apply the ME operators to RHS
    for(Int i=0; i<nTermRHS; i++) {
      if (termTypeRHS[i]==VISJONESAPPLY) {
	termRHS[i]->apply(vbRHS);
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
	termLHS[i]->applyInv(vbLHS); 
      };
      if (termTypeLHS[i]==FREQAVER) {
	vbLHS.freqAverage(); 
      };
    };    
    */

    // avoid ACs and zero data, move time-dep phase to LHS
    AlwaysAssert(vbLHS.nRow()==vbRHS.nRow(),AipsError);
    AlwaysAssert(vbLHS.nChannel()==vbRHS.nChannel(),AipsError);
    //    cout << "EVP:2 " << vbRHS.nRow() << " " << corruptedvb.nRow() << endl;

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

              if (vj.type()!=VisJones::D) {
		CStokesVector& vRHS=vbRHS.visibility()(channel,row);
		CStokesVector& vLHS=vbLHS.visibility()(channel,row);

		for (uInt i=0;i<4;i++) {
		  if (abs(vRHS(i))>0.0) {
		    Complex factor=conj(vRHS(i))/abs(vRHS(i));
		    vRHS(i)*=factor;
		    vLHS(i)*=factor;
		  };
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

    //    cout << "EVP:3 " << vbRHS.nRow() << " " << corruptedvb.nRow() << endl;
    // Accumulate the VisBuffer in time
    vtaRHS->accumulate(vbRHS);
    // Accumulate the VisBuffer in time
    vtaLHS->accumulate(vbLHS);
    // Iterate to the next VisBuffer in the chunk
    vi++;
    icount++;
  }
  //  cout << "EVP:4 " << vbRHS.nRow() << " " << corruptedvb.nRow() << endl;

  // Finalize the time average into solver's VisBuffers
  vtaRHS->finalizeAverage(corruptedvb);
  vtaLHS->finalizeAverage(correctedvb);
  
  //  cout << "EVP:Time " << corruptedvb.time()-4.60203e+9 << endl;
  //  cout << "EVP:5 " << vbRHS.nRow() << " " << corruptedvb.nRow() << endl;
  // Delete the time averagers
  delete vtaRHS;
  delete vtaLHS;

  // Remove unnecessary cols from the vbs:
  //  corruptedvb.removeScratchCols();
  //  correctedvb.removeScratchCols();

  // Report number of rows in solvers VisBuffers:
  //  cout << "Nrows = " << correctedvb.nRow() << " " << corruptedvb.nRow() << endl;

  //  cout << "New Timing: " << timer.user() << " " << timer.system() << " " << timer.real() << endl;
}
}
