//# VisBufferUtil.cc: VisBuffer Utilities
//# Copyright (C) 1996,1997,2001
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

#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <msvis/MSVis/VisBufferUtil.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/MeasTable.h>
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

// Set the visibility buffer for a PSF

void VisBufferUtil::makePSFVisBuffer(VisBuffer& vb) {
  CStokesVector coh(Complex(1.0), Complex(0.0), Complex(0.0), Complex(1.0));
  vb.correctedVisibility()=coh;
}

 Bool VisBufferUtil::interpolateFrequency(Cube<Complex>& data, 
					Cube<Bool>& flag, 
					const VisBuffer& vb,
					const Vector<Float>& outFreqGrid, 	const MS::PredefinedColumns whichCol, 
					const MFrequency::Types freqFrame,
					const InterpolateArray1D<Float,Complex>::InterpolationMethod interpMethod){



  Cube<Complex> origdata;
  // Convert the visibility frequency to the frame requested
  Vector<Double> visFreqD;
  convertFrequency(visFreqD, vb, freqFrame);
  //convert it to Float
  Vector<Float> visFreq(visFreqD.nelements());
  convertArray(visFreq, visFreqD);
  
  //Assign which column is to be regridded to origdata
  if(whichCol==MS::MODEL_DATA){
    origdata.reference(vb.modelVisCube());
  }
  else if(whichCol==MS::CORRECTED_DATA){
      origdata.reference(vb.correctedVisCube());
    }
  else if(whichCol==MS::DATA){
      origdata.reference(vb.visCube());
  }
  else{
    throw(AipsError("Don't know which column is being regridded"));
  }
  Cube<Complex> flipdata;
  Cube<Bool> flipflag;
  //The interpolator interpolates on the 3rd axis only...so need to flip the axes (y,z)
  swapyz(flipflag,vb.flagCube());
  swapyz(flipdata,origdata);

  //interpolate the data and the flag to the output frequency grid
  InterpolateArray1D<Float,Complex>::
    interpolate(data,flag, outFreqGrid,visFreq,flipdata,flipflag,interpMethod);
  flipdata.resize();
  //reflip the data and flag to be in the same order as in Visbuffer output
  swapyz(flipdata,data);
  data.resize();
  data.reference(flipdata);
  flipflag.resize();
  swapyz(flipflag,flag);
  flag.resize();     
  flag.reference(flipflag);

  return True;

}

 void VisBufferUtil::convertFrequency(Vector<Double>& outFreq, const VisBuffer& vb, const MFrequency::Types freqFrame){
   Vector<Double> inFreq=vb.frequency();
   Int spw=vb.spectralWindow();
   MFrequency::Types obsMFreqType=(MFrequency::Types)(vb.msColumns().spectralWindow().measFreqRef()(spw));
   if(obsMFreqType != freqFrame){
     // Setting epoch to the first in this iteration
     MEpoch ep=vb.msColumns().timeMeas()(0);
     MDirection dir=vb.phaseCenter();
     // determine the reference frame position
     String observatory;
     MPosition telescopePosition;
     if (vb.msColumns().observation().nrow() > 0) {
       observatory = vb.msColumns().observation().telescopeName()
	 (vb.msColumns().observationId()(0));
     }
     if (observatory.length() == 0 || 
	 !MeasTable::Observatory(telescopePosition,observatory)) {
       // unknown observatory, use first antenna
       telescopePosition=vb.msColumns().antenna().positionMeas()(0);
     }
     MeasFrame frame(ep, telescopePosition, dir);
     MFrequency::Convert toNewFrame(obsMFreqType, 
				    MFrequency::Ref(freqFrame, frame));
     outFreq.resize(inFreq.nelements());
     for (uInt k=0; k< inFreq.nelements(); ++k){
       outFreq(k)=toNewFrame(inFreq(k)).getValue().getValue();
     }


   }
   else{
     // The requested frame is the same as the observed frame
     outFreq.resize();
     outFreq=inFreq;
   }

 }
// helper function to swap the y and z axes of a Cube
 void VisBufferUtil::swapyz(Cube<Complex>& out, const Cube<Complex>& in)
{
  IPosition inShape=in.shape();
  uInt nxx=inShape(0),nyy=inShape(2),nzz=inShape(1);
  //resize breaks  references...so out better have the right shape 
  //if references is not to be broken
  if(out.nelements()==0)
    out.resize(nxx,nyy,nzz);
  Bool deleteIn,deleteOut;
  const Complex* pin = in.getStorage(deleteIn);
  Complex* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nzz; ++iz, zOffset+=nxx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<nyy; ++iy, yOffset+=nxx*nzz) {
      for (uInt ix=0; ix<nxx; ++ix){ 
	pout[i++] = pin[ix+yOffset];
      }
    }
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}

// helper function to swap the y and z axes of a Cube
void VisBufferUtil::swapyz(Cube<Bool>& out, const Cube<Bool>& in)
{
  IPosition inShape=in.shape();
  uInt nxx=inShape(0),nyy=inShape(2),nzz=inShape(1);
  if(out.nelements()==0)
    out.resize(nxx,nyy,nzz);
  Bool deleteIn,deleteOut;
  const Bool* pin = in.getStorage(deleteIn);
  Bool* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nzz; iz++, zOffset+=nxx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<nyy; iy++, yOffset+=nxx*nzz) {
      for (uInt ix=0; ix<nxx; ix++) pout[i++] = pin[ix+yOffset];
    }
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}

 


} //# NAMESPACE CASA - END

