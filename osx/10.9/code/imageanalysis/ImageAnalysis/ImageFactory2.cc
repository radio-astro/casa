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

namespace casa {

	SPIIF ImageFactory::floatImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear,
    	Bool overwrite, Bool verbose,
        const vector<std::pair<LogOrigin, String> > *const &msgs
    ) {
		return _fromShape<Float>(
			outfile, shape, csys, linear,
            overwrite, verbose, msgs
		);
	}

	SPIIC ImageFactory::complexImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear,
    	Bool overwrite, Bool verbose,
        const vector<std::pair<LogOrigin, String> > *const &msgs
    ) {
		return _fromShape<Complex>(
			outfile, shape, csys, linear,
            overwrite, verbose, msgs
		);
	}

void ImageFactory::_centerRefPix(
	CoordinateSystem& csys, const IPosition& shape
) {
	Int after = -1;
	Int iS = csys.findCoordinate(Coordinate::STOKES, after);
	Int sP = -1;
	if (iS >= 0) {
		Vector<Int> pixelAxes = csys.pixelAxes(iS);
		sP = pixelAxes(0);
	}
	Vector<Double> refPix = csys.referencePixel();
	for (Int i = 0; i < Int(refPix.nelements()); i++) {
		if (i != sP)
			refPix(i) = Double(shape(i) / 2);
	}
	csys.setReferencePixel(refPix);
}

CoordinateSystem* ImageFactory::_makeCoordinateSystem(
    const Record& coordinates, const IPosition& shape
) {
    std::auto_ptr<CoordinateSystem> pCS;
    if (coordinates.nfields() == 1) { 
        // must be a record as an element
        Record tmp(coordinates.asRecord(RecordFieldId(0)));
        pCS.reset(CoordinateSystem::restore(tmp, ""));
    } 
    else {
        pCS.reset(CoordinateSystem::restore(coordinates, ""));
    }
    // Fix up any body longitude ranges...
    String errMsg;
    ThrowIf(
        ! CoordinateUtil::cylindricalFix(*pCS, errMsg, shape),
        errMsg
    ); 
    return pCS.release();
}

}
