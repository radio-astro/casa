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

#include <casa/aips.h>
#include <casa/Quanta/QuantumHolder.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageStatistics.h>

#include <iostream>
#include <iomanip>

namespace casa {

template<class T> const String ImageMetaData<T>::_BEAMMAJOR = "beammajor";
template<class T> const String ImageMetaData<T>::_BEAMMINOR = "beamminor";
template<class T> const String ImageMetaData<T>::_BEAMPA = "beampa";
template<class T> const String ImageMetaData<T>::_BUNIT = "bunit";
template<class T> const String ImageMetaData<T>::_CTYPE = "ctype";
template<class T> const String ImageMetaData<T>::_DATAMAX = "datamax";
template<class T> const String ImageMetaData<T>::_DATAMIN = "datamin";
template<class T> const String ImageMetaData<T>::_EQUINOX = "equinox";
template<class T> const String ImageMetaData<T>::_IMTYPE = "imtype";
template<class T> const String ImageMetaData<T>::_MASKS = "masks";
template<class T> const String ImageMetaData<T>::_MAXPIXPOS = "maxpixpos";
template<class T> const String ImageMetaData<T>::_MAXPOS = "maxpos";
template<class T> const String ImageMetaData<T>::_MINPIXPOS = "minpixpos";
template<class T> const String ImageMetaData<T>::_MINPOS = "minpos";
template<class T> const String ImageMetaData<T>::_OBJECT = "object";
template<class T> const String ImageMetaData<T>::_OBSDATE = "date-obs";
template<class T> const String ImageMetaData<T>::_OBSERVER = "observer";
template<class T> const String ImageMetaData<T>::_PROJECTION = "projection";
template<class T> const String ImageMetaData<T>::_RESTFREQ = "restfreq";
template<class T> const String ImageMetaData<T>::_REFFREQTYPE = "reffreqtype";
template<class T> const String ImageMetaData<T>::_SHAPE = "shape";
template<class T> const String ImageMetaData<T>::_TELESCOPE = "telescope";

template<class T> ImageMetaData<T>::ImageMetaData(
	const ImageInterface<Float> *const &image
) : _image(image), _log(new LogIO()) {}

template<class T> Record ImageMetaData<T>::toRecord(Bool verbose) {
	const ImageInfo& info = _image->imageInfo();
	_header.define(_IMTYPE, ImageInfo::imageType(info.imageType()));

	ImageSummary<T> summary(*_image);

	_header.define(_OBJECT, info.objectName());

	const CoordinateSystem& csys = _image->coordinates();

	if (csys.hasDirectionCoordinate()) {
		const DirectionCoordinate& dc = csys.directionCoordinate();
		String equinox = MDirection::showType(
			dc.directionType()
		);
		_header.define(_EQUINOX, equinox);
		_header.define(_PROJECTION, dc.projection().name());
	}
	MEpoch date;
	_header.define(_OBSDATE, summary.obsDate(date));
	_header.define(_MASKS, summary.maskNames());
	_header.define(_OBSERVER, summary.observer());
	_header.define(_SHAPE, _image->shape().asVector());
	_header.define(_TELESCOPE, summary.telescope());
	_header.define(_BUNIT, summary.units().getName());

	if (csys.hasSpectralAxis()) {
		const SpectralCoordinate& sc = csys.spectralCoordinate();
		_header.define(_RESTFREQ, sc.restFrequencies());
		_header.define(
			_REFFREQTYPE , MFrequency::showType(sc.frequencySystem())
		);
	}

	if (info.hasBeam()) {
		Record beam = info.getBeamSet().toRecord(True);
		_header.merge(beam);
	}
	if (info.hasSingleBeam()) {
		GaussianBeam beam = info.restoringBeam(-1, -1);
		_header.defineRecord(
			_BEAMMAJOR,
			QuantumHolder(beam.getMajor()).toRecord()
		);
		_header.defineRecord(
			_BEAMMINOR,
			QuantumHolder(beam.getMinor()).toRecord()
		);
		_header.defineRecord(
			_BEAMPA,
			QuantumHolder(beam.getPA(True)).toRecord()
		);
	}
	else if (info.hasMultipleBeams()) {
		_header.defineRecord("perplanebeams", info.getBeamSet().toRecord(True));
	}
 	Vector<Double> cdelt = summary.axisIncrements(True);
	Vector<String> units = summary.axisUnits(True);
	Vector<Double> crpix = summary.referencePixels(False);
	Vector<Double> crval = summary.referenceValues(True);
	Vector<String> types = summary.axisNames(True);
	ImageStatistics<T> stats(*_image);
	Array<Double> min;
	stats.getStatistic(min, LatticeStatsBase::MIN);
	_header.define(_DATAMIN, min(IPosition(min.ndim(), 0)));
	Array<Double> max;
	stats.getStatistic(max, LatticeStatsBase::MAX);
	_header.define(_DATAMAX, max(IPosition(max.ndim(), 0)));
	IPosition minPixPos, maxPixPos;
	stats.getMinMaxPos(minPixPos, maxPixPos);
	_header.define(_MINPIXPOS, minPixPos.asVector());
	_header.define(_MAXPIXPOS, maxPixPos.asVector());
	Vector<Double> minPos = csys.toWorld(minPixPos);
	Vector<Double> maxPos = csys.toWorld(maxPixPos);
	String minFormat, maxFormat;
	uInt ndim = csys.nPixelAxes();
	Int spAxis = csys.spectralAxisNumber();
	for (uInt i=0; i<ndim; i++) {
		Int worldAxis = csys.pixelAxisToWorldAxis(i);
		String foundUnit;
		minFormat += csys.format(
	    	foundUnit, Coordinate::DEFAULT,
	    	minPos[i], worldAxis
	    );
		maxFormat += csys.format(
			foundUnit, Coordinate::DEFAULT,
			maxPos[i], worldAxis
		);
		if ((Int)i == spAxis) {
			minFormat += foundUnit;
			maxFormat += foundUnit;
		}
		if (i != ndim-1) {
			minFormat += " ";
			maxFormat += " ";
		}
	}
	_header.define(_MINPOS, minFormat);
	_header.define(_MAXPOS, maxFormat);
	for (uInt i=0; i<cdelt.size(); i++) {
		String iString = String::toString(i + 1);
		String delt = "cdelt" + iString;
		_header.define(delt, cdelt[i]);
		String unit = "cunit" + iString;
		_header.define(unit, units[i]);
		String pix = "crpix" + iString;
		_header.define(pix, crpix[i]);
		String val = "crval" + iString;
		_header.define(val, crval[i]);
		String type = _CTYPE + iString;
		_header.define(type, types[i]);
	}
	if (verbose) {
		_toLog();
	}
	return _header;
}

template<class T> void ImageMetaData<T>::_fieldToLog(const String& field, Int precision) const {
	*_log << "        -- " << field << ": ";
	if (_header.isDefined(field)) {
		DataType type = _header.type(_header.idToNumber(field));
		if (precision >= 0) {
			_log->output() << setprecision(precision);
		}
		switch (type) {
			case TpArrayDouble: {
				*_log << _header.asArrayDouble(field);
				break;
			}
			case TpArrayInt: {
				*_log << _header.asArrayInt(field);
				break;
			}
			case TpArrayString: {
				*_log << _header.asArrayString(field);
				break;
			}
			case TpDouble: {
				*_log << _header.asDouble(field);
				break;
			}
			case TpRecord: {
				Record r = _header.asRecord(field);
				QuantumHolder qh;
				String error;
				if (qh.fromRecord(error, r) && qh.isQuantity()) {
					Quantity q = qh.asQuantity();
					*_log << q.getValue() << q.getUnit();
				}
				else {
					*_log << "Logic Error: Don't know how to deal with records of this type "
						<< LogIO::EXCEPTION;
				}
				break;
			}
			case TpString: {
				*_log << _header.asString(field);
				break;
			}

			default: {
				*_log << "Logic Error: Unsupported type "
					<< type << LogIO::EXCEPTION;
				break;
			}
		}
	}
	else {
		*_log << "Not found";
	}
	*_log << LogIO::POST;
}

template<class T> void ImageMetaData<T>::_toLog() const {
	*_log << LogOrigin("ImageMetaData", __FUNCTION__, WHERE);
	*_log << "General --" << LogIO::POST;
	_fieldToLog(_IMTYPE);
	_fieldToLog(_OBJECT);
	_fieldToLog(_EQUINOX);
	_fieldToLog(_OBSDATE);
	_fieldToLog(_OBSERVER);
	_fieldToLog(_PROJECTION);
	*_log << "        -- " << _RESTFREQ << ": ";
	_log->output() << std::fixed << std::setprecision(1);
	*_log <<  _header.asArrayDouble(_RESTFREQ) << LogIO::POST;
	_fieldToLog(_REFFREQTYPE);
	_fieldToLog(_TELESCOPE);
	_fieldToLog(_BEAMMAJOR, 12);
	_fieldToLog(_BEAMMINOR, 12);
	_fieldToLog(_BEAMPA, 12);
	_fieldToLog(_BUNIT);
	_fieldToLog(_MASKS);
	_fieldToLog(_SHAPE);
	_fieldToLog(_DATAMIN);
	_fieldToLog(_DATAMAX);
	_fieldToLog(_MINPOS);
	_fieldToLog(_MINPIXPOS);
	_fieldToLog(_MAXPOS);
	_fieldToLog(_MAXPIXPOS);

	uInt i = 1;
	*_log << LogIO::NORMAL << "axes --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CTYPE + iString;
		if (! _header.isDefined(key)) {
			break;
		}
		*_log << "        -- " << key << ": "
			<< _header.asString(key) << LogIO::POST;
		String unit = "cunit" + iString;
		i++;
	}
	i = 1;
	*_log << LogIO::NORMAL << "crpix --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = "crpix" + iString;
		if (! _header.isDefined(key)) {
			break;
		}
		_log->output() << std::fixed << std::setprecision(1);
		*_log << "        -- " << key << ": " << _header.asDouble(key)
			<< LogIO::POST;
		i++;
	}
	i = 1;
	*_log << LogIO::NORMAL << "crval --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = "crval" + iString;
		if (! _header.isDefined(key)) {
			break;
		}
		*_log << "        -- " << key << ": ";
		ostringstream x;
		Double val = _header.asDouble(key);
		x << val;
		String unit = "cunit" + iString;
		if (_header.isDefined(unit)) {
			x << _header.asString(unit);
		}
		String valunit = x.str();
		if (_header.isDefined(unit)) {
			String myunit = _header.asString(unit);
			if (_header.asString(unit).empty()) {
				String ctype = _CTYPE + iString;
				if (
					_header.isDefined(ctype)
					&& _header.asString(ctype) == "Stokes"
				) {
					valunit = "['" + Stokes::name((Stokes::StokesTypes)((Int)val)) + "']";
				}
			}
			else {
				String tmp = _doStandardFormat(val, myunit);
				if (! tmp.empty()) {
					valunit = tmp;
				}
			}
		}
		*_log << valunit << LogIO::POST;
		i++;
	}
	i = 1;
	*_log << LogIO::NORMAL << "cdelt --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = "cdelt" + iString;
		if (! _header.isDefined(key)) {
			break;
		}
		*_log << "        -- " << key << ": ";
		Double val = _header.asDouble(key);
		String unit = "cunit" + iString;
		String myunit;
		if (_header.isDefined(unit)) {
			myunit = _header.asString(unit);
		}
		ostringstream x;
		x << val << myunit;
		String valunit = x.str();
		if (_header.isDefined(unit)) {
			String myunit = _header.asString(unit);
			if (! _header.asString(unit).empty()) {
				String tmp = _doStandardFormat(val, myunit);
				if (! tmp.empty()) {
					valunit = tmp;
				}
			}
		}
		*_log << valunit << LogIO::POST;
		i++;
	}
	i = 1;
	*_log << LogIO::NORMAL << "units --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = "cunit" + iString;
		if (! _header.isDefined(key)) {
			break;
		}
		*_log << "        -- " << key << ": "
			<< _header.asString(key) << LogIO::POST;
		String unit = "cunit" + iString;
		i++;
	}

}

template <class T>  String ImageMetaData<T>::_doStandardFormat(
	Double value, const String& unit
) const {
	String valunit;
	try {
		Quantity q(1, unit);
		if (q.isConform(Quantity(1, "rad"))) {
			// to dms
			valunit = MVAngle(Quantity(value, unit)).string(MVAngle::CLEAN, 9) + "deg.min.sec";
		}
		else if (unit == "Hz") {
			ostringstream x;
			x << std::fixed << std::setprecision(1);
			x << value << "Hz";
			valunit = x.str();
		}
	}
	catch (const AipsError& x) {}
	return valunit;
}


template <class T> uInt ImageMetaData<T>::nChannels() const {
	const CoordinateSystem& csys = _image->coordinates();
	if (! csys.hasSpectralAxis()) {
		return 0;
	}
	return _image->shape()[csys.spectralAxisNumber()];
}

template <class T> Bool ImageMetaData<T>::isChannelNumberValid(
	const uInt chan
) const {
	if (! _image->coordinates().hasSpectralAxis()) {
		return False;
    }
	return (chan < nChannels());
}

template <class T> uInt ImageMetaData<T>::nStokes() const {
	const CoordinateSystem& csys = _image->coordinates();

	if (! csys.hasPolarizationCoordinate()) {
		return 0;
    }
	return _image->shape()[csys.polarizationAxisNumber()];
}

template <class T> Int ImageMetaData<T>::stokesPixelNumber(
	const String& stokesString) const {
	Int pixNum = _image->coordinates().stokesPixelNumber(stokesString);
	if (pixNum >= (Int)nStokes()) {
		pixNum = -1;
    }
	return pixNum;
}

template <class T> String ImageMetaData<T>::stokesAtPixel(
	const uInt pixel
) const {
	const CoordinateSystem& csys = _image->coordinates();
	if (! csys.hasPolarizationCoordinate() || pixel >= nStokes()) {
             return "";
        }
	return csys.stokesAtPixel(pixel);
}

template <class T> Bool ImageMetaData<T>::isStokesValid(
	const String& stokesString
) const {
	if (! _image->coordinates().hasPolarizationCoordinate()) {
		return False;
    }
	Int stokesPixNum = stokesPixelNumber(stokesString);
	return stokesPixNum >= 0 && stokesPixNum < (Int)nStokes();
}

template <class T> Vector<Int> ImageMetaData<T>::directionShape() const {
	Vector<Int> dirAxesNums = _image->coordinates().directionAxesNumbers();
	if (dirAxesNums.nelements() == 0) {
		return Vector<Int>();
	}
	Vector<Int> dirShape(2);
	IPosition shape = _image->shape();
	dirShape[0] = shape[dirAxesNums[0]];
	dirShape[1] = shape[dirAxesNums[1]];
	return dirShape;
}

template <class T> Bool ImageMetaData<T>::areChannelAndStokesValid(
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

