//# SubImage.h: A (masked) subset of an ImageInterface object
//# Copyright (C) 1998,1999,2000,2001,2003
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
//# $Id: SubImage.h 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#ifndef IMAGESANALYSIS_SUBIMAGEFACTORY_H
#define IMAGESANALYSIS_SUBIMAGEFACTORY_H

#include <images/Images/SubImage.h>

#include <tr1/memory>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Static methods for subimage creation
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="tSubImageFactory.cc">
// </reviewed>
//
// <prerequisite>
//   <li> <linkto class=ImageInterface>SubImage</linkto>
// </prerequisite>
//
// <synopsis> 
// Factory methods for creating subimages
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="2013/02/24">
// </todo>


template <class T> class SubImageFactory {
public: 
  // Factory method to create a SubImage from a region and a WCLELMask string.
  // Moved from ImageAnalysis
  // <src>outRegion</src> Pointer to the corresponding region. Pointer is
  // created internally by new(); it is the caller's responsibility to delete it.
  // <src>outMask</src> Pointer to corresponding mask. Pointer is created
  // internally via new(); it is the caller's responsibility to delete it.
  // <src>inImage</src> input image for which a subimage is desired.
  // <src>region</src> Input region record from which to make the subimage.
  // <src>mask</src> LEL mask description.
  // <src>os</src> Pointer to logger to which to log messages. If null, no logging (except exceptions).
  // <src>writableIfPossible</src> make the subimage writable. If input image is not writable, this
  // will always be False.
  // <src>axesSpecifier</src> Specifier for output axes.
  // <src>extendMask</src> If the mask has one
  // or more of degenerate axes whereas the corresponding axes of <src>inImage</src> are
  // not, extend the mask to match the shape of the input image.
  // <src>preserveAxesOrder</src>. Only used when dropping degenerate axes and coordinate order
  // and axes order are not the same. In that case, if False, the pixel/world axes order of the
  // returned image will be different from the input, if True it will be the same. If not
  // dropping degenerate axes or if coordinate order and axes order are the same in the input
  // image's coordinate system, the output axex order will always be preserved.

  static SubImage<T> createSubImage(
	  ImageRegion*& outRegion, ImageRegion*& outMask,
      ImageInterface<T>& inImage, const Record& region,
      const String& mask, LogIO *const &os, Bool writableIfPossible,
      const AxesSpecifier& axesSpecifier=casa::AxesSpecifier(),
      Bool extendMask=False, Bool preserveAxesOrder=False
  );

  // variant on previous method where caller doesn't have to worry
  // about creating pointers it does not need returned.
  static SubImage<T> createSubImage(
      ImageInterface<T>& inImage, const Record& region,
      const String& mask, LogIO *const &os, Bool writableIfPossible,
      const AxesSpecifier& axesSpecifier=casa::AxesSpecifier(),
      Bool extendMask=False, Bool preserveAxesOrder=False
  );

  // return a true copy (ie underlying data is a copy of the original, not
  // a reference) of the subimage selected in the given region.
  // A PagedImage is returned if outfile is not blank or a SubImage
  // is returned if it is.
  static ImageInterface<T>* createImage(
	  const ImageInterface<T>& image,
	  const String& outfile, const Record& region,
	  const String& mask, Bool dropDegenerateAxes,
	  Bool overwrite, Bool list, Bool extendMask
  );

  private:
    SubImageFactory<T> (); 
};



} //# NAMESPACE CASA - END

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/SubImageFactory.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif
