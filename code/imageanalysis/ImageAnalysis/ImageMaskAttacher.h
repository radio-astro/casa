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

#ifndef IMAGESANALYSIS_IMAGEMASKATTACHER_H
#define IMAGESANALYSIS_IMAGEMASKATTACHER_H

#include <images/Images/ImageInterface.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Contains tatic method to attach a mask to an image
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="tSubImageFactory.cc">
// </reviewed>
//
// <prerequisite>
// </prerequisite>
//
// <synopsis> 
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="2013/02/24">
// </todo>


template <class T> class ImageMaskAttacher {
public: 
	// Moved from ImageAnalysis
	// Make a mask and define it in the image.
	static Bool makeMask(
		ImageInterface<T>& out,
		String& maskName,
		Bool init, Bool makeDefault,
		LogIO& os, Bool list
	);

private:
    ImageMaskAttacher<T> ();

    ImageMaskAttacher<T> operator=(ImageMaskAttacher<T>);
};



} //# NAMESPACE CASA - END

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif
