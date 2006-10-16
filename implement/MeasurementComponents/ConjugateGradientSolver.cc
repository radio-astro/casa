//# ConjugateGradientSolver.cc: Implementation of an iterative ConjugateGradientSolver
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#include <synthesis/MeasurementComponents/ConjugateGradientSolver.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <casa/Arrays/MatrixMath.h>
namespace casa {

  ConjugateGradientSolver::ConjugateGradientSolver(Int nParams,Int nIter, Double tol):
    Iterate()
  {
    setNumberIterations(nIter);
    setTolerance(tol);
    setMaxParams(nParams);
  };
  //
  //---------------------------------------------------------------------------
  // Get a vector of visibilities from all baselines with antenna
  // whichAnt.  It returns N-1 values (fills in the complex conjugate
  // part as well).
  //
  Vector<Complex> ConjugateGradientSolver::getVj(VisBuffer& vb, Int NAnt, Int whichAnt,
						 Double& sumWt)
  {
    Vector<Int> ant1, ant2;
    Vector<Complex> Vj;
    ant1 = vb.antenna1();
    ant2 = vb.antenna2();
    Vj.resize(NAnt);
    Int J=0,N;
    IPosition ndx(3,0);
    Vj = 0.0;
    N = vb.nRow();
    Double wt;
    sumWt = 0;
    for(IPosition ndx(3,0);ndx(2)<N;ndx(2)++)
      if (!residual.flagCube()(ndx)       &&
	  ((ant1[ndx(2)] != ant2[ndx(2)]) && 
	   (ant1[ndx(2)] == whichAnt)     || 
	   (ant2[ndx(2)] == whichAnt)))
	{
	  wt = vb.weight()(ndx(2));
	  sumWt += wt;
	  if (ant1[ndx(2)] > ant2[ndx(2)])  
	    Vj[J] = Complex(wt,0)*vb.modelVisCube()(ndx);
	  else
	    Vj[J] = Complex(wt,0)*conj(vb.modelVisCube()(ndx));
	  J++;
	}
    return Vj;
  }
  //
  //---------------------------------------------------------------------------
  // Compute the penalty function (also called the Goodness-of-fit criteria).
  // For us, its the Chi-square function.
  //
  Double ConjugateGradientSolver::getGOF(VisBuffer& residual, Double& sumWt)
  {
    Float Chisq=0.0;
    Int nRow=residual.nRow();

    sumWt=0.0;
    for (IPosition ndx(3,0);ndx(2)<nRow;ndx(2)++)
      if (!residual.flagCube()(ndx))
	{
	  DComplex Vis;
	  Float wt;

	  Vis=residual.modelVisCube()(ndx);
	  wt=residual.weight()(ndx(2));
	  sumWt += wt*wt;
	  //	  Chisq += wt*wt*real(innerProduct(Vis,Vis));
	  Chisq += wt*wt*real(Vis*conj(Vis));
	}
    if (sumWt > 0) return Chisq;// /sumWt;
    else return 0.0;
  }
  //
  //---------------------------------------------------------------------------
  //Given the VisEquation, this iteratively sovles for the parameters
  //of the supplied VisJones for the time-stamp given by SlotNo.  nAnt
  //is the number of antennas per time stamp.
  //
  Double ConjugateGradientSolver::solve(EPVisEquation& ve, EPJones& epj, 
					Int nAnt, Int SlotNo, Array<Float>& Guess)
  {
    Array<Float> oldOffsets;
    Vector<Complex> ResidualVj, dAzVj, dElVj;
    Double Chisq0,Chisq,sumWt;
    Double Time;
    static Double Time0;
    Double AzHDiag,ElHDiag;
    Timer timer;

    IPosition ndx(2,2,nAnt);

    ResidualVj.resize(nAnt);
    dAzVj.resize(nAnt);
    dElVj.resize(nAnt);
    //    Guess.resize(ndx);

    ndx(0)=1;
    /*
    for(int i=0;i<nAnt;i++) 
      {
	ndx(1)=i;
	ndx(0)=0;Guess(ndx)=0;
	ndx(0)=1;Guess(ndx)=10.0/(57.295*3600.0);
	ndx(0)=1;Guess(ndx)=0;
      }
    */
    Guess = 0;
    Chisq0=0.0;
    epj.setAntPar(SlotNo,Guess);
    ve.setVisJones(epj);
    ve.initChiSquare(epj);
    ve.getResAndGrad(epj, residual, gradient0, gradient1);

    //    Double Sigma=1E-6;
    //    Chisq = getGOF(residual,sumWt)/Sigma;

    Chisq = getGOF(residual,sumWt)*sumWt;

    Time = getCurrentTimeStamp(residual);
    if (SlotNo == 0) Time0 = Time;

    timer.mark();
    Int iter;
    Bool Converged=False;
    cout << "####Sol. int. = " 
	 << SlotNo << " Chisq = " << Chisq << " " << sumWt << endl;
    for (iter=0;iter<numberIterations();iter++)
      {
	oldOffsets = Guess;
	//	epj.reset();
//   	cout << "Iteration = " << iter << " Sol. int. = " 
//   	     << SlotNo << " Chisq = " << Chisq << endl;
	  {
	    for(Int ant=0;ant<nAnt;ant++)
	      {
		ResidualVj = getVj(residual, nAnt, ant, sumWt);
		dAzVj      = getVj(gradient0, nAnt, ant, sumWt);
		dElVj      = getVj(gradient1, nAnt, ant, sumWt);

		if (sumWt > 0)
		  {
		    ndx(1)=ant;

		    Double coVar1, coVar2;
		    coVar1 = coVar2 = 0;
		    if (sumWt > 0) coVar1 = real(innerProduct(dAzVj,dAzVj));// /sumWt;
		    if (sumWt > 0) coVar2 = real(innerProduct(dElVj,dElVj));// /sumWt;
		    AzHDiag = ElHDiag = 0;
		    if (fabs(coVar1) > 1E-7) AzHDiag = 1.0/coVar1;
		    if (fabs(coVar2) > 1E-7) ElHDiag = 1.0/coVar2;
// 		      cout << "Step = " << ant << " " 
// 			   << 2*AzHDiag*gain()*real(innerProduct(ResidualVj,dAzVj)) << " " 
// 			   << 2*ElHDiag*gain()*real(innerProduct(ResidualVj,dElVj)) << " "
// 			   << 1.0/(57.295*3600.0)
// 			   << endl;

		    ndx(0)=0;
		    Guess(ndx) = Guess(ndx)-2*AzHDiag*gain()*
		      real(innerProduct(ResidualVj,dAzVj));// /sumWt;
		    ndx(0)=1;
		    Guess(ndx) = Guess(ndx)-2*ElHDiag*gain()*
		      real(innerProduct(ResidualVj,dElVj));// /sumWt;
		  }
	      }
	    //
	    // Compute the residuals and the gradients with the
	    // updated solutions and check for convergence.
	    //
	    epj.setAntPar(SlotNo,Guess);
	    ve.getResAndGrad(epj,residual,gradient0,gradient1);
	    Chisq0 = Chisq;
	    //	    Chisq  = getGOF(residual,sumWt)/Sigma;

	    Chisq  = getGOF(residual,sumWt)*sumWt;
	  }
	  Double dChisq;
	  dChisq = (Chisq0-Chisq);
	  cout << Chisq0 << " " << Chisq << " " << dChisq << endl;
	  if ((fabs(dChisq) < tolerance()))
	    {
	      Converged=True;
	      break;
	    }
	  if ((dChisq < tolerance()))// && (iter > 0))
	    {
 	      logIO() << LogIO::WARN << "Detected a possible divergence.  dChisq = " 
 		      << dChisq << ". Back tracking. Gain = " << gain()
 		      << LogIO::POST;
	      
	      setGain(gain()/2.0);
	      Guess = oldOffsets;
	      epj.setAntPar(SlotNo,oldOffsets);
	      ve.getResAndGrad(epj,residual,gradient0,gradient1);
	      //	      Chisq  = getGOF(residual,sumWt)/Sigma;

	      Chisq  = getGOF(residual,sumWt)*sumWt;
	      Chisq0 = Chisq;
// 	      Converged = True;
// 	      break;
	    }
	  else
	    {
	      oldOffsets = Guess;
	    }
      }
    cout << "###Sol. int. = " 
	 << SlotNo << " Final Chisq = " << Chisq << endl;

//     if (!Converged)
//       for(ndx(1)=0;ndx(1)<nAnt;ndx(1)++)
// 	for(ndx(0)=0;ndx(0)<2;ndx(0)++)
// 	  Guess(ndx)=0.0;

    epj.setAntPar(SlotNo,Guess,(Converged && (Chisq < nAnt))); // Finalize the solutions
					     // in VisJones internal
					     // cache.

    Chisq = fabs(Chisq0-Chisq);

    logIO() << LogIO::NORMAL << "No. of iterations used: " 
	    << iter
	    << " Time total, per iteration:  " 
	    << timer.all() << ", " << timer.all()/(iter+1) << " sec" << LogIO::POST;

//     for(ndx(1)=0;ndx(1)<nAnt;ndx(1)++)
//       {
// 	cout << "Results : " << SlotNo << " " << ndx(1) << " " << Time0-Time << " ";
// 	for(ndx(0)=0;ndx(0)<2;ndx(0)++)
// 	  cout << Guess(ndx)*57.295*3600.0 << " ";
// 	cout << endl;
//       }

    return (Chisq < tolerance()? Chisq:-Chisq);
  }
  
};
