//# ClarkCleanImageSkyModel.h: Definition for ClarkCleanImageSkyModel
//# Copyright (C) 1996,1997,1998,1999,2000,2003
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

#ifndef SYNTHESIS_CLARKCLEANIMAGESKYMODEL_H
#define SYNTHESIS_CLARKCLEANIMAGESKYMODEL_H

#include <synthesis/MeasurementComponents/CleanImageSkyModel.h>
namespace casa { //# NAMESPACE CASA - BEGIN

//forward
class SkyEquation;
class ClarkCleanProgress;
template <class T> class SubLattice;
template <class T> class RO_LatticeIterator;
// <summary> 
// Clark Image Sky Model: Image Sky Model implementing the Clark Clean algorithm
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=ImageSkyModel>ImageSkyModel</linkto> module
//   <li> <linkto class=ClarkCleanLatModel>ClarkCleanLatModel</linkto> module
// </prerequisite>
//
// <etymology>
// ClarkCleanImageSkyModel implements the Clark Clean algorithm.
// It is derived from <linkto class=SkyModel>SkyModel</linkto>.
// </etymology>
//
// <synopsis> 
// The Clark Clean is an FFT-based clean algorithm. Cleaning is
// split into major and minor cycles. In a minor cycle, the
// brightest pixels are cleaned using only the strongest sidelobes
// (and main lobe) of the PSF. In the major cycle, a fully correct
// subtraction of the PSF is done for all points accumulated in the
// minor cycle using an FFT-based convolution for speed.
//
// The Clark Clean is implemented using the 
// <linkto class=ClarkCleanLatModel>ClarkCleanLatModel</linkto>
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

class ClarkCleanImageSkyModel : public CleanImageSkyModel {
public:

  // constructor
  ClarkCleanImageSkyModel();

  // destructor
  ~ClarkCleanImageSkyModel();

  // Solve for this SkyModel
  virtual Bool solve (SkyEquation& me);

  //Do the image-based clean
  //iterused and maxresidual are return values
  static Bool clean(ImageInterface<Float>& image, ImageInterface<Float> & residual, ImageInterface<Float>& psf, 
		    ImageInterface<Float>& mask, Float& maxresidual, Int& iterused, Float gain, Int numIter,  Float thresh, Float cycleFactor, Bool useMask=False, Bool doPolJoint=True);
 
private:
   //make a mask sub-lattice

  static Lattice<Float>* makeMaskSubLat(const Int& nx, 
				 const Int& ny, Int& newNx, Int& newNy,
				    RO_LatticeIterator<Float>& maskIter,
				    Int& xbeg, Int& xend, 
				    Int& ybeg, Int& yend);

  ClarkCleanProgress *itsProgress;

};


} //# NAMESPACE CASA - END

#endif







