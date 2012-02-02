//# MFCEMemImageSkyModel.h: Definition for MFCEMemImageSkyModel
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#ifndef SYNTHESIS_MFCEMEMIMAGESKYMODEL_H
#define SYNTHESIS_MFCEMEMIMAGESKYMODEL_H

#include <synthesis/MeasurementComponents/CEMemImageSkyModel.h>
namespace casa { //# NAMESPACE CASA - BEGIN

//forward
class SkyEquation;

// <summary> 
// MF CE Mem Image Sky Model: Image Sky Model implementing the MF Mem algorithm
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=ImageSkyModel>ImageSkyModel</linkto> module
// </prerequisite>
//
// <etymology>
// MFCEMemImageSkyModel implements the MF Clean algorithm.
// It is derived from <linkto class=SkyModel>SkyModel</linkto>.
// </etymology>
//
// <synopsis> 
// Rewrite me for MEM!
// The MF Clean is an FFT-based clean algorithm. Cleaning is
// split into major and minor cycles. In a minor cycle, the
// brightest pixels are cleaned using only the strongest sidelobes
// (and main lobe) of the PSF. In the major cycle, a fully correct
// subtraction of the PSF is done for all points accumulated in the
// minor cycle using an FFT-based convolution for speed.
//
// The MF Clean is implemented using the <linkto class=ClarkCleanModel</linkto>
// class.
//
// Masking is optionally performed using a mask image: only points
// where the mask is non-zero are cleaned. If no mask is specified
// all points in the inner quarter of the image are cleaned.
// </synopsis> 
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="97/10/01">
// <ul> Allow specification of more control parameters
// </todo>

class MFCEMemImageSkyModel : public CEMemImageSkyModel {
public:

  // constructor
  MFCEMemImageSkyModel(Float sigma, 
		       Float targetFlux,
		       Bool constrainFlux,
		       const Vector<String>& priors,
		       const String& entropy);


  // Solve for this SkyModel
  virtual Bool solve (SkyEquation& me);

private:
  // Return the maximum absolute value per field
  Float maxField(Vector<Float>& imagemax, Vector<Float>& imagemin);
  // Return maximum abs outer sidelobe, more than nCenter pixels from the center
  Float maxOuter(Lattice<Float> & lat, const uInt nCenter );

};


} //# NAMESPACE CASA - END

#endif


