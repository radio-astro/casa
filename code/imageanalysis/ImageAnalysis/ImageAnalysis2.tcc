//# ImageAnalysis.cc:  Image analysis and handling tool
//# Copyright (C) 1995-2007
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
//# $Id: ImageAnalysis.cc 20491 2009-01-16 08:33:56Z gervandiepen $
//   

// PLEASE DO *NOT* ADD ADDITIONAL METHODS TO THIS CLASS

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>

#include <imageanalysis/ImageAnalysis/ImageFFT.h>
#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

namespace casa {

template<class T> void ImageAnalysis::_destruct(ImageInterface<T>& image) {
	if((image.isPersistent()) && ((image.imageType()) == "PagedImage")) {
		ImageOpener::ImageTypes type = ImageOpener::imageType(image.name());
		if (type == ImageOpener::AIPSPP) {
			Table::relinquishAutoLocks(True);
			(static_cast<PagedImage<T>& >(image)).table().unlock();
		}
	}
}

} // end of  casa namespace
