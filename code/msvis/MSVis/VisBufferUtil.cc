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
#include <measures/Measures/UVWMachine.h>
#include <measures/Measures/MeasTable.h>

#include <msvis/MSVis/VisBufferUtil.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer.h>
#include <ms/MeasurementSets/MSColumns.h>
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


VisBufferUtil::VisBufferUtil() {};


// Construct from a VisBuffer (sets frame info)
VisBufferUtil::VisBufferUtil(const VisBuffer& vb) {

  // The nominal epoch 
  MEpoch ep=vb.msColumns().timeMeas()(0);

  // The nominal position
  String observatory;
  MPosition pos;
  if (vb.msColumns().observation().nrow() > 0) {
    observatory = vb.msColumns().observation().telescopeName()
      (vb.msColumns().observationId()(0));
  }
  if (observatory.length() == 0 || 
      !MeasTable::Observatory(pos,observatory)) {
    // unknown observatory, use first antenna
    pos=vb.msColumns().antenna().positionMeas()(0);
  }
 
  // The nominal direction
  MDirection dir=vb.phaseCenter();

  // The nominal MeasFrame
  mframe_=MeasFrame(ep, pos, dir);

}

// Construct from a VisBuffer (sets frame info)
VisBufferUtil::VisBufferUtil(const vi::VisBuffer2& vb) {

	ROMSColumns msc(vb.getVi()->ms());
  // The nominal epoch
  MEpoch ep=msc.timeMeas()(0);

  // The nominal position
  String observatory;
  MPosition pos;
  if (msc.observation().nrow() > 0) {
    observatory = msc.observation().telescopeName()
      (msc.observationId()(0));
  }
  if (observatory.length() == 0 ||
      !MeasTable::Observatory(pos,observatory)) {
    // unknown observatory, use first antenna
    pos=msc.antenna().positionMeas()(0);
  }

  // The nominal direction
  MDirection dir=vb.phaseCenter();

  // The nominal MeasFrame
  mframe_=MeasFrame(ep, pos, dir);

}

  Bool VisBufferUtil::rotateUVW(const vi::VisBuffer2&vb, const MDirection& desiredDir,
				Matrix<Double>& uvw, Vector<Double>& dphase){

    Bool retval=True;
    mframe_.resetEpoch(vb.time()(0));
    UVWMachine uvwMachine(desiredDir, vb.phaseCenter(), mframe_,
			False, False);
    retval = !uvwMachine.isNOP();
    dphase.resize(vb.nRows());
    uvw.resize(3, vb.nRows());
    for (Int row=0; row< vb.nRows(); ++row){
      Vector<Double> eluvw(uvw.column(row));
      uvwMachine.convertUVW(dphase(row), eluvw);
    }
    
    return retval;
  }

// Set the visibility buffer for a PSF
void VisBufferUtil::makePSFVisBuffer(VisBuffer& vb) {
  CStokesVector coh(Complex(1.0), Complex(0.0), Complex(0.0), Complex(1.0));
  vb.correctedVisibility()=coh;
}

Bool VisBufferUtil::interpolateFrequency(Cube<Complex>& data, 
					 Cube<Bool>& flag, 
					 const VisBuffer& vb,
					 const Vector<Float>& outFreqGrid, 	
					 const MS::PredefinedColumns whichCol, 
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

void VisBufferUtil::convertFrequency(Vector<Double>& outFreq, 
				     const VisBuffer& vb, 
				     const MFrequency::Types freqFrame){
   Int spw=vb.spectralWindow();
   MFrequency::Types obsMFreqType=(MFrequency::Types)(vb.msColumns().spectralWindow().measFreqRef()(spw));

   // The input frequencies (a reference)
   Vector<Double> inFreq(vb.frequency());

   // The output frequencies
   outFreq.resize(inFreq.nelements());

   MFrequency::Types newMFreqType=freqFrame;
   if (freqFrame==MFrequency::N_Types)
     // Opt out of conversion
     newMFreqType=obsMFreqType;


   // Only convert if the requested frame differs from observed frame
   if(obsMFreqType != newMFreqType){

     // Setting epoch to the first in this iteration
     //     MEpoch ep=vb.msColumns().timeMeas()(0);
     //     MEpoch ep(MVEpoch(vb.time()(0)/86400.0),MEpoch::UTC);
     //     cout << "Time = " << ep.getValue()  << endl;

     // Reset the timestamp (ASSUMES TIME is constant in the VisBuffer)
     mframe_.resetEpoch(vb.time()(0)/86400.0);

     // Reset the direction (ASSUMES phaseCenter is constant in the VisBuffer)
     mframe_.resetDirection(vb.msColumns().field().phaseDirMeasCol()(vb.fieldId())(IPosition(1,0)));

     //     cout << "Frame = " << mframe_ << endl;

     // The conversion engine:
     MFrequency::Convert toNewFrame(obsMFreqType, 
				    MFrequency::Ref(newMFreqType, mframe_));

     // Do the conversion
     for (uInt k=0; k< inFreq.nelements(); ++k)
       outFreq(k)=toNewFrame(inFreq(k)).getValue().getValue();
     
   }
   else{
     // The requested frame is the same as the observed frame
     outFreq=inFreq;
   }

 }

 void VisBufferUtil::toVelocity(Vector<Double>& outVel, 
				const VisBuffer& vb, 
				const MFrequency::Types freqFrame,
				const MVFrequency restFreq,
				const MDoppler::Types veldef){

   // The input frequencies (a reference)
   Vector<Double> inFreq(vb.frequency());

   // The output velocities
   outVel.resize(inFreq.nelements());

   // Reset the timestamp (ASSUMES TIME is constant in the VisBuffer)
   mframe_.resetEpoch(vb.time()(0)/86400.0);
   
   // Reset the direction (ASSUMES phaseCenter is constant in the VisBuffer)
   //mframe_.resetDirection(vb.phaseCenter());
   mframe_.resetDirection(vb.msColumns().field().phaseDirMeasCol()(vb.fieldId())(IPosition(1,0)));
 
   // The frequency conversion engine:
   Int spw=vb.spectralWindow();
   MFrequency::Types obsMFreqType=(MFrequency::Types)(vb.msColumns().spectralWindow().measFreqRef()(spw));

   MFrequency::Types newMFreqType=freqFrame;
   if (freqFrame==MFrequency::N_Types)
     // Don't convert frame
     newMFreqType=obsMFreqType;

   MFrequency::Convert toNewFrame(obsMFreqType, 
				  MFrequency::Ref(newMFreqType, mframe_));

   // The velocity conversion engine:
   MDoppler::Ref dum1(MDoppler::RELATIVISTIC);
   MDoppler::Ref dum2(veldef);
   MDoppler::Convert dopConv(dum1, dum2);

   // Cope with unspecified rest freq
   MVFrequency rf=restFreq;
   if (restFreq.getValue()<=0.0)
     rf=toNewFrame(inFreq(vb.nChannel()/2)).getValue();

   // Do the conversions
   for (uInt k=0; k< inFreq.nelements(); ++k){
     MDoppler eh = toNewFrame(inFreq(k)).toDoppler(rf);
     MDoppler eh2 = dopConv(eh);
     outVel(k)=eh2.getValue().get().getValue();
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

