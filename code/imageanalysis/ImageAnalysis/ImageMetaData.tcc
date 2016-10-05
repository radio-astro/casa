//# ImageMetaData.cc: Meta information for Images
//# Copyright (C) 2009
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
//# $Id: ImageMetaData.cc 20886 2010-04-29 14:06:56Z gervandiepen $

#include <imageanalysis/ImageAnalysis/ImageMetaData.h>

#include <casacore/casa/Logging/LogFilter.h>

#include <casa/aips.h>

#include <images/Images/ImageSummary.h>


namespace casa {
template<class T> casacore::Record ImageMetaData::_summary(
    SPCIIT image,
    const casacore::String& doppler, const casacore::Bool list,
    const casacore::Bool pixelorder, const casacore::Bool verbose
) {
    auto log = _getLog();
    log << casacore::LogOrigin("ImageMetaData", __func__);
    casacore::Vector<casacore::String> messages;
    casacore::Record retval;
    casacore::ImageSummary<T> s(*image);
    casacore::MDoppler::Types velType;
    if (!casacore::MDoppler::getType(velType, doppler)) {
        log << casacore::LogIO::WARN << "Illegal velocity type, using RADIO"
                << casacore::LogIO::POST;
        velType = casacore::MDoppler::RADIO;
    }

    if (list) {
        messages = s.list(log, velType, false, verbose);
    }
    else {
        // Write messages to local sink only so we can fish them out again
        casacore::LogFilter filter;
        casacore::LogSink sink(filter, false);
        casacore::LogIO osl(sink);
        messages = s.list(osl, velType, true);
    }
    retval.define("messages", messages);
    auto axes = s.axisNames(pixelorder);
    auto crpix = s.referencePixels(false); // 0-rel
    auto crval = s.referenceValues(pixelorder);
    auto cdelt = s.axisIncrements(pixelorder);
    auto axisunits = s.axisUnits(pixelorder);

    auto shape = _getShape();
    retval.define("ndim", (casacore::Int)shape.size());
    retval.define("shape", shape.asVector());
    retval.define("tileshape", s.tileShape().asVector());
    retval.define("axisnames", axes);
    retval.define("refpix", crpix);
    retval.define("refval", crval);
    retval.define("incr", cdelt);
    retval.define("axisunits", axisunits);
    retval.define("unit", _getBrightnessUnit());
    retval.define("hasmask", s.hasAMask());
    retval.define("defaultmask", s.defaultMaskName());
    retval.define("masks", s.maskNames());
    retval.define("imagetype", _getImType());

    const auto& info = image->imageInfo();
    casacore::Record iRec;
    casacore::String error;
    using casacore::AipsError;
    ThrowIf(
        ! info.toRecord(error, iRec),
        "Failed to convert ImageInfo to a record because "
    );
    if (iRec.isDefined("restoringbeam")) {
        retval.defineRecord("restoringbeam", iRec.asRecord("restoringbeam"));
    }
    else if (iRec.isDefined("perplanebeams")) {
        retval.defineRecord("perplanebeams", info.beamToRecord(-1, -1));
    }
    return retval;
}

}

