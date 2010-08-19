//# VisSetUtil.cc: VisSet Utilities
//# Copyright (C) 1996,1997,1998,1999,2001,2002
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

#include <casa/aips.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Cube.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Utilities/Assert.h>

#include <ms/MeasurementSets/MSColumns.h>

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/MVAngle.h>

#include <casa/Logging/LogIO.h>

#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// </summary>

// <reviewed reviewer="" date="" tests="tMEGI" demos="">

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis> 
// </synopsis> 
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="">
// </todo>
// Calculate sensitivity
void VisSetUtil::Sensitivity(VisSet &vs, Quantity& pointsourcesens, 
			     Double& relativesens, Double& sumwt)
{
  ROVisIter& vi(vs.iter());
   VisSetUtil::Sensitivity(vi, pointsourcesens, relativesens,sumwt);

}
void VisSetUtil::Sensitivity(ROVisIter &vi, Quantity& pointsourcesens, 
			     Double& relativesens, Double& sumwt)
{
  LogIO os(LogOrigin("VisSetUtil", "Sensitivity()", WHERE));
  
  sumwt=0.0;
  Double sumwtsq=0.0;
  Double sumInverseVariance=0.0;
  
  VisBuffer vb(vi);

  // Now iterate through the data
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi++) {
      Int nRow=vb.nRow();
      Int nChan=vb.nChannel();
      for (Int row=0; row<nRow; row++) {
	// TBD: Should probably use weight() here, which updates with calibration
        Double variance=square(vb.sigma()(row));
	for (Int chn=0; chn<nChan; chn++) {
	  if(!vb.flag()(chn,row)&&variance>0.0) {
	    sumwt+=vb.imagingWeight()(chn,row);
	    sumwtsq+=square(vb.imagingWeight()(chn,row))*variance;
	    sumInverseVariance+=1.0/variance;
	  }
	}
      }
    }
  }

  if(sumwt==0.0) {
    os << "Cannot calculate sensitivity: sum of weights is zero" << endl
       << "Perhaps you need to weight the data" << LogIO::EXCEPTION;
  }
  if(sumInverseVariance==0.0) {
    os << "Cannot calculate sensitivity: sum of inverse variances is zero" << endl
       << "Perhaps you need to weight the data" << LogIO::EXCEPTION;
  }

  Double naturalsens=1.0/sqrt(sumInverseVariance);
  pointsourcesens=Quantity(sqrt(sumwtsq)/sumwt, "Jy");
  relativesens=sqrt(sumwtsq)/sumwt/naturalsens;
}
  void VisSetUtil::HanningSmooth(VisSet &vs, const String& dataCol)
{
  VisIter& vi(vs.iter());
  VisSetUtil::HanningSmooth(vi, dataCol);
}
  void VisSetUtil::HanningSmooth(VisIter &vi, const String& dataCol)
{
  LogIO os(LogOrigin("VisSetUtil", "HanningSmooth()"));

  VisBuffer vb(vi);
  Int row, chn, pol;

  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    if (vi.existsWeightSpectrum()) {
      for (vi.origin();vi.more();vi++) {

	Cube<Complex>& vc = ( dataCol=="data" ? vb.visCube() : vb.correctedVisCube());

	Cube<Bool>& fc= vb.flagCube();
	Cube<Float>& wc= vb.weightSpectrum();

	Int nRow=vb.nRow();
	Int nChan=vb.nChannel();
	if (nChan < 3) break;
	Int nPol=vi.visibilityShape()(0);
	Cube<Complex> smoothedData(nPol,nChan,nRow);
	Cube<Bool> newFlag(nPol,nChan,nRow);
	Cube<Float> newWeight(nPol,nChan,nRow);
	for (row=0; row<nRow; row++) {
	  for (pol=0; pol<nPol; pol++) {
	    ///Handle first channel and flag it
	    smoothedData(pol,0,row) = vc(pol,0,row)*0.5 + vc(pol,1,row)*0.5;
	    newWeight(pol,0,row) = 0.0;
	    newFlag(pol,0,row) = True;
	    for (chn=1; chn<nChan-1; chn++) {
	      smoothedData(pol,chn,row) =
		vc(pol,chn-1,row)*0.25 + vc(pol,chn,row)*0.50 +
		vc(pol,chn+1,row)*0.25;
	      if (wc(pol,chn-1,row) != 0 && wc(pol,chn,row) != 0
		  && wc(pol,chn+1,row) != 0) {
		newWeight(pol,chn,row) = 1.0 /
		  (1.0/(wc(pol,chn-1,row)*16.0) + 1.0/(wc(pol,chn,row)*4.0)
		   + 1.0/(wc(pol,chn+1,row)*16.0));
	      } else {
		newWeight(pol,chn,row) = 0.0;
	      }
	      newFlag(pol,chn,row) =
		fc(pol,chn-1,row)||fc(pol,chn,row)||fc(pol,chn+1,row);
	    }
	    //Handle last channel and flag it
	    smoothedData(pol,nChan-1,row) =
	      vc(pol,nChan-2,row)*0.5+vc(pol,nChan-1,row)*0.5;
	    newWeight(pol,nChan-1,row) = 0.0;
	    newFlag(pol,nChan-1,row) = True;  // flag last channel
	  }
	}

	if(dataCol=="data"){
	  vi.setVisAndFlag(smoothedData,newFlag,VisibilityIterator::Observed);
	}
	else{
	  vi.setVisAndFlag(smoothedData,newFlag,VisibilityIterator::Corrected);
	}

	vi.setWeightSpectrum(newWeight);
      }
    } else {
      for (vi.origin();vi.more();vi++) {

	Cube<Complex>& vc = (dataCol=="data" ? vb.visCube() : vb.correctedVisCube());

	Cube<Bool>& fc= vb.flagCube();
	Matrix<Float>& wm = vb.weightMat();

	Int nRow=vb.nRow();
	Int nChan=vb.nChannel();
	if (nChan < 3) break;
	Int nPol=vi.visibilityShape()(0);
	Cube<Complex> smoothedData(nPol,nChan,nRow);
	Cube<Bool> newFlag(nPol,nChan,nRow);
	Matrix<Float> newWeight(nPol, nRow);
	for (row=0; row<nRow; row++) {
	  for (pol=0; pol<nPol; pol++) {
	    ///Handle first channel and flag it
	    smoothedData(pol,0,row) = vc(pol,0,row)*0.5 + vc(pol,1,row)*0.5;
	    newFlag(pol,0,row) = True;
	    ///Handle chan-independent weights
	    newWeight(pol, row) = 8.0*wm(pol, row)/3.0;
	    for (chn=1; chn<nChan-1; chn++) {
	      smoothedData(pol,chn,row) =
		vc(pol,chn-1,row)*0.25 + vc(pol,chn,row)*0.50 +
		vc(pol,chn+1,row)*0.25;
	      newFlag(pol,chn,row) =
		fc(pol,chn-1,row)||fc(pol,chn,row)||fc(pol,chn+1,row);
	    }
	    //Handle last channel and flag it
	    smoothedData(pol,nChan-1,row) =
	      vc(pol,nChan-2,row)*0.5+vc(pol,nChan-1,row)*0.5;
	    newFlag(pol,nChan-1,row) = True;  // flag last channel
	  }
	}

	if(dataCol=="data"){
	  vi.setVisAndFlag(smoothedData,newFlag,VisibilityIterator::Observed);
	}
	else{
	  vi.setVisAndFlag(smoothedData,newFlag,VisibilityIterator::Corrected);
	}

	vi.setWeightMat(newWeight);
      }
    }
  }
}
void VisSetUtil::UVSub(VisSet &vs, Bool reverse)
{
  VisIter& vi(vs.iter());
  VisSetUtil::UVSub(vi, reverse);
}
void VisSetUtil::UVSub(VisIter &vi, Bool reverse)
{
  LogIO os(LogOrigin("VisSetUtil", "UVSub()"));


  VisBuffer vb(vi);

  Int row, chn, pol;

  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi++) {

      Cube<Complex>& vc= vb.correctedVisCube();
      Cube<Complex>& mc= vb.modelVisCube();

      Int nRow=vb.nRow();
      Int nChan=vb.nChannel();
      Int nPol=vi.visibilityShape()(0);
      Cube<Complex> residualData(nPol,nChan,nRow);
      if (reverse) {
	for (row=0; row<nRow; row++) {
	  for (pol=0; pol<nPol; pol++) {
	    for (chn=0; chn<nChan; chn++) {
	      residualData(pol,chn,row) = vc(pol,chn,row)+mc(pol,chn ,row);
	    }
	  }
	}
      } else {
	for (row=0; row<nRow; row++) {
	  for (pol=0; pol<nPol; pol++) {
	    for (chn=0; chn<nChan; chn++) {
	      residualData(pol,chn,row) = vc(pol,chn,row)-mc(pol,chn ,row);
	    }
	  }
	}
      }
      vi.setVis(residualData,VisibilityIterator::Corrected);
    }
  }
}

} //# NAMESPACE CASA - END

