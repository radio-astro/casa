//# ImageRotator.cc
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001,2002
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
//   

#include <imageanalysis/ImageAnalysis/ImageRotator.h>

#include <imageanalysis/ImageAnalysis/ImageRegridder.h>

#include <memory>

namespace casa {

const String ImageRotator::CLASS_NAME = "ImageRotator";

ImageRotator::ImageRotator(
	const SPCIIF image, const Record *const &regionPtr,
    const String& mask,const String& outname, Bool overwrite
) : ImageTask<Float>(
        image, "", regionPtr, "", "", "", mask, outname, overwrite
    ) {
	this->_construct(True);
}

ImageRotator::~ImageRotator() {}

SPIIF ImageRotator::rotate() {
    *this->_getLog() << LogOrigin(getClass(), __func__);
    if (_shape.empty()) {
        IPosition imShape = this->_getImage()->shape();
        _shape = this->_getDropDegen() ? imShape.nonDegenerate() : imShape;
    }
    auto subImage = SubImageFactory<Float>::createSubImageRO(
        *this->_getImage(), *this->_getRegion(), this->_getMask(),
        this->_getLog().get(), AxesSpecifier(! this->_getDropDegen()), this->_getStretch()
    );
    const auto& cSysFrom = subImage->coordinates();
    auto cSysTo = cSysFrom;
    // We automatically find a DirectionCoordinate or LInearCoordinate
    // These must hold *only* 2 axes at this point (restriction in ImageRegrid)
    Vector<Int> pixelAxes;

    if (cSysTo.hasDirectionCoordinate()) {
        auto index = cSysTo.directionCoordinateNumber();
        pixelAxes = cSysTo.pixelAxes(index);
        std::unique_ptr<DirectionCoordinate> dc(
            dynamic_cast<DirectionCoordinate *>(
                cSysTo.directionCoordinate().rotate(_angle)
            )
        );
        cSysTo.replaceCoordinate(*dc, (uInt)index);
        *this->_getLog() << "Rotating DirectionCoordinate holding axes "
            << pixelAxes << LogIO::POST;
    }
    else if (cSysTo.hasLinearCoordinate()) {
        auto index = cSysTo.linearCoordinateNumber();
        pixelAxes = cSysTo.pixelAxes(index);
        ThrowIf(
            pixelAxes.size() != 2,
            "Can only rotate a linear coordinate with exactly two axes"
        );
        std::unique_ptr<LinearCoordinate> lc(
            dynamic_cast<LinearCoordinate *>(
                cSysTo.linearCoordinate(index).rotate(_angle)
            )
        );
        cSysTo.replaceCoordinate(*lc, (uInt)index);
    }
    else {
        ThrowCc(
            "Can only rotate a direction coordinate or a linear "
            "coordiante with exactly two axes"
        );
    }
    IPosition axes2(pixelAxes);

    ImageRegridder regridder(
        subImage, nullptr, "", this->_getOutname(),
        this->_getOverwrite(), cSysTo, axes2, _shape
    );
    regridder.setDecimate(_decimate);
    regridder.setMethod(_method);
    regridder.setReplicate(_replicate);
    regridder.setShape(_shape);
    return regridder.regrid();
}

}
