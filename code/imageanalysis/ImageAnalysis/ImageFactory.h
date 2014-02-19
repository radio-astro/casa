//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_IMAGEFACTORY_H
#define IMAGEANALYSIS_IMAGEFACTORY_H

#include <imageanalysis/ImageTypedefs.h>

#include <casa/BasicSL/String.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/namespace.h>
#include <utility>
#include <vector>
namespace casa {

class CoordinateSystem;
class IPosition;
class Record;
template <class T> class Vector;

class ImageFactory {
	// <summary>
	// Static methods for creating images 
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

public:

    ~ImageFactory() {};

    // Create a TempImage if outfile is empty, otherwise a PagedImage.
    // If log is True, log useful messages, quiet if False. Created image
    // has all pixel values set to zero and is unmasked.
    template <class T> static SPIIT createImage(
        const String& outfile,
        const CoordinateSystem& cSys, const IPosition& shape,
        Bool log, Bool overwrite,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs
    );

    // create an image with the specified shape and specified coordinate system.
    // If outfile is blank, the returned object is a TempImage, PagedImage otherwise.
    // If csys is empty,
    // a default coordiante system is attached to the image, and if linear
    // is True, it has linear coordinates in place of the direction coordinate.

    static SPIIF floatImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear=True,
    	Bool overwrite=False, Bool verbose=True,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs=0
    );

    static SPIIC complexImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear=True,
    	Bool overwrite=False, Bool verbose=True,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs=0
    );

private:
	ImageFactory() {};

	template <class T> static SPIIT _fromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs
	);

	static void _centerRefPix(
		CoordinateSystem& csys, const IPosition& shape
	);

    // Convert a Record to a CoordinateSystem
    static CoordinateSystem* _makeCoordinateSystem(
        const casa::Record& cSys,
        const casa::IPosition& shape
    );
};
}
#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageFactory.tcc>
#endif

#endif
