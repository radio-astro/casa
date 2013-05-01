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

#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.h>

namespace casa {

template<class T> ImageMaskAttacher<T>::ImageMaskAttacher() {}


template<class T> Bool ImageMaskAttacher<T>::makeMask(
	ImageInterface<T>& out, String& maskName,
	Bool init, Bool makeDefault, LogIO& os, Bool list
) {
	os << LogOrigin("ImageMaskAttacher", __FUNCTION__);
	if (out.canDefineRegion()) {

		// Generate mask name if not given
		if (maskName.empty())
			maskName = out.makeUniqueRegionName(String("mask"), 0);

		// Make the mask if it does not exist
		if (!out.hasRegion(maskName, RegionHandler::Masks)) {
			out.makeMask(maskName, True, makeDefault, init, True);
			if (list) {
				if (init) {
					os << LogIO::NORMAL << "Created and initialized mask `"
							<< maskName << "'" << LogIO::POST;
				} else {
					os << LogIO::NORMAL << "Created mask `" << maskName << "'"
							<< LogIO::POST;
				}
			}
		}
		return True;
	} else {
		os << LogIO::WARN
				<< "Cannot make requested mask for this type of image" << endl;
		return False;
	}
}


} //# NAMESPACE CASA - END

