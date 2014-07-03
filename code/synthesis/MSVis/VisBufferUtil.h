//# VisBufferUtil.h: Definitions for VisBuffer utilities
//# Copyright (C) 1996-2014
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

#ifndef MSVIS_VISBUFFERUTIL_H
#define MSVIS_VISBUFFERUTIL_H

#include <casa/aips.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MFrequency.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
namespace casa { //# NAMESPACE CASA - BEGIN
//forward 
class VisBuffer;
// <summary> 
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

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
class VisBufferUtil{

public:

  //Empty Constructor
  VisBufferUtil();

  // Construct from a VisBuffer (sets a MeasFrame)
  VisBufferUtil(const VisBuffer& vb);

  // Same as above but with VisBuffer2
  VisBufferUtil(const vi::VisBuffer2& vb);
  // Make PSF VisBuffer
  void makePSFVisBuffer(VisBuffer& vb);
  

  //Regrid the data on a new frequency grid (defined by outFreqGrid) , on the frequency 
  //frame (defined by freqFrame). It will pass back the interpolated data and flags 
 
  Bool interpolateFrequency(Cube<Complex>& data, 
			    Cube<Bool>& flags, 
			    const VisBuffer& vb,
			    const Vector<Float>& outFreqGrid, const MS::PredefinedColumns whichCol=MS::DATA,
			    const MFrequency::Types freqFrame=MFrequency::LSRK,
			    const InterpolateArray1D< Float, Complex>::InterpolationMethod interp = 
			    (InterpolateArray1D<Float,Complex>::nearestNeighbour));

  // Converts the frequency in this visbuffer to the frame requested
  void convertFrequency(Vector<Double>& outFreq, 
			const VisBuffer& vb, 
			const MFrequency::Types freqFrame);

  // Converts the frequency in this VisBuffer to velocity in the frame/def requested
  void toVelocity(Vector<Double>& outVel, 
		  const VisBuffer& vb, 
		  const MFrequency::Types freqFrame,
		  const MVFrequency restFreq,
		  const MDoppler::Types veldef);


  //Rotate the uvw in the vb along with giving the phase needed to convert the visibilities to a new phasecenter
  // will return a False if it is a NoOP...don't need then to waste time
  //applying the phasor etc 
   Bool rotateUVW(const vi::VisBuffer2&vb, const MDirection& desiredDir,
		  Matrix<Double>& uvw, Vector<Double>& dphase);

 private:
  void swapyz(Cube<Bool>& out, const Cube<Bool>& in);
  void swapyz(Cube<Complex>& out, const Cube<Complex>& in);

  // A MeasFrame for conversions
  MeasFrame mframe_;
  

};

} //# NAMESPACE CASA - END

#endif

