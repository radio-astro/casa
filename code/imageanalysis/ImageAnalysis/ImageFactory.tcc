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
//# $Id: $

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>

#include <imageanalysis/ImageAnalysis/ImageHistory.h>
namespace casa {

template <class T> SPIIT ImageFactory::createImage(
    const String& outfile,
    const CoordinateSystem& cSys, const IPosition& shape,
    Bool log, Bool overwrite 
) {
    Bool blank = outfile.empty();
    if (! overwrite && ! blank) {
        NewFile validfile;
        String errmsg;
        ThrowIf(
            !validfile.valueOK(outfile, errmsg),
            errmsg
        );
    }    
    ThrowIf(
        shape.nelements() != cSys.nPixelAxes(),
        "Supplied CoordinateSystem and image shape are inconsistent"
    );
    SPIIT image;
    if (outfile.empty()) {
        image.reset(new TempImage<T>(shape, cSys));
        ThrowIf(
            ! image,
            "Failed to create TempImage"
        );
    }
    else {
        image.reset(new PagedImage<T>(shape, cSys, outfile));
        ThrowIf(
            ! image,
            "Failed to create PagedImage"
        );
    }
    ostringstream os;
    T *x;
    os << "Created "
       << (blank ? "Temp" : "Paged") << " image "
       << (blank ? "" : "'" + outfile + "'")
       << " of shape " << shape << " with "
       << whatType(x) << " valued pixels.";
    ImageHistory<T> hist(image);
    LogOrigin lor("ImageFactory", __func__);
    hist.addHistory(lor, os.str());
    if (log) {
        LogIO mylog;
        mylog << LogIO::NORMAL << os.str() << LogIO::POST; 
    }
    return image;
}
}

