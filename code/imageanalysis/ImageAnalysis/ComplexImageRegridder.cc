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

#include <imageanalysis/ImageAnalysis/ComplexImageRegridder.h>

#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageRegridder.h>

namespace casa {

const String  ComplexImageRegridder::_class = "ComplexImageRegridder";

ComplexImageRegridder::ComplexImageRegridder(
	const SPCIIC image,
	const Record *const regionRec,
	const String& maskInp, const String& outname, Bool overwrite,
	const CoordinateSystem& csysTo, const IPosition& axes,
	const IPosition& shape
) : ImageRegridderBase<Complex>(
		image, regionRec,
		maskInp, outname, overwrite,
		csysTo, axes, shape
	) {}

template <class T> ComplexImageRegridder::ComplexImageRegridder(
	const SPCIIC image, const String& outname,
	const SPCIIT templateIm, const IPosition& axes,
	const Record *const regionRec, const String& maskInp,
	Bool overwrite, const IPosition& shape
)  : ImageRegridderBase<Complex>(
		image, regionRec, maskInp, outname, overwrite,
		templateIm->coordinates(), axes, shape
) {}

ComplexImageRegridder::~ComplexImageRegridder() {}

SPIIC ComplexImageRegridder::regrid() const {
	SPCIIC myimage = this->_getImage();
	SPIIF realPart = ImageFactory::floatFromComplex(
		myimage, ImageFactory::REAL
	);
	ImageRegridder rgReal(realPart, _getRegion(), _getMask(), "",
		False, _getTemplateCoords(), _getAxes(), _getShape()
	);
	rgReal.setConfiguration(*this);
	SPIIF outReal = rgReal.regrid();
	SPIIF imagPart = ImageFactory::floatFromComplex(
		myimage, ImageFactory::IMAG
	);
	ImageRegridder rgImag(imagPart, _getRegion(), _getMask(), "",
		False, _getTemplateCoords(), _getAxes(), _getShape()
	);
	rgImag.setConfiguration(*this);
	SPIIF outImag = rgImag.regrid();
	SPIIC outImage = ImageFactory::complexFromFloat(
		outReal, outImag->get()
	);
	return this->_prepareOutputImage(*outImage);
}

}
