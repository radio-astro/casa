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

#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

    uInt ImageMetaData::nChannels() const {
        if (! _coordinates.hasSpectralAxis()) {
            return 0;
        }
        return _shape[_coordinates.spectralAxisNumber()];
    }

    Bool ImageMetaData::isChannelNumberValid(const uInt chan) const {
        if (! _coordinates.hasSpectralAxis()) {
            return False;
        }
        return (chan < nChannels());
    }

    uInt ImageMetaData::nStokes() const {
        if (! _coordinates.hasPolarizationCoordinate()) {
            return 0;
        }
        return _shape[_coordinates.polarizationAxisNumber()];
    }

    Int ImageMetaData::stokesPixelNumber(const String& stokesString) const {
    	Int pixNum = _coordinates.stokesPixelNumber(stokesString);
    	if (pixNum >= (Int)nStokes()) {
    		pixNum = -1;
    	}
    	return pixNum;
    }

    String ImageMetaData::stokesAtPixel(const uInt pixel) const {
        if (! _coordinates.hasPolarizationCoordinate() || pixel >= nStokes()) {
             return "";
        }
        return _coordinates.stokesAtPixel(pixel);
    }

    Bool ImageMetaData::isStokesValid(const String& stokesString) const {
        if (! _coordinates.hasPolarizationCoordinate()) {
            return False;
        }
        Int stokesPixNum = stokesPixelNumber(stokesString);
        return stokesPixNum >= 0 && stokesPixNum < (Int)nStokes(); 
    }

    Vector<Int> ImageMetaData::directionShape() const {
    	Vector<Int> dirAxesNums = _coordinates.directionAxesNumbers();
    	if (dirAxesNums.nelements() == 0) {
    		return Vector<Int>();
    	}
    	Vector<Int> dirShape(2);
    	dirShape[0] = _shape[dirAxesNums[0]];
    	dirShape[1] = _shape[dirAxesNums[1]];
    	return dirShape;
    }


    Bool ImageMetaData::areChannelAndStokesValid(
        String& message, const uInt chan, const String& stokesString
    ) const {
        ostringstream os;
        Bool areValid = True;
        if (! isChannelNumberValid(chan)) {
            os << "Zero-based channel number " << chan << " is too large. There are only "
                << nChannels() << " spectral channels in this image.";
            areValid = False;
        }    
        if (! isStokesValid(stokesString)) {
            if (! areValid) {
                os << " and ";
            }
            os << "Stokes parameter " << stokesString << " is not in image";
            areValid = False;
        }
        if (! areValid) {
            message = os.str();
        }    
        return areValid;
    }

} //# NAMESPACE CASA - END

