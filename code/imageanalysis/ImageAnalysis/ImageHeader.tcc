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

#include <imageanalysis/ImageAnalysis/ImageHeader.h>

#include <casa/Quanta/QuantumHolder.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageStatistics.h>



namespace casa {

template<class T> const String ImageHeader<T>::_BEAMMAJOR = "beammajor";
template<class T> const String ImageHeader<T>::_BEAMMINOR = "beamminor";
template<class T> const String ImageHeader<T>::_BEAMPA = "beampa";
template<class T> const String ImageHeader<T>::_CTYPE = "ctype";
template<class T> const String ImageHeader<T>::_DATAMAX = "datamax";
template<class T> const String ImageHeader<T>::_DATAMIN = "datamin";
template<class T> const String ImageHeader<T>::_EQUINOX = "equinox";
template<class T> const String ImageHeader<T>::_IMTYPE = "imtype";
template<class T> const String ImageHeader<T>::_MASKS = "masks";
template<class T> const String ImageHeader<T>::_MAXPIXPOS = "maxpixpos";
template<class T> const String ImageHeader<T>::_MAXPOS = "maxpos";
template<class T> const String ImageHeader<T>::_MINPIXPOS = "minpixpos";
template<class T> const String ImageHeader<T>::_MINPOS = "minpos";
template<class T> const String ImageHeader<T>::_OBJECT = "object";
template<class T> const String ImageHeader<T>::_OBSDATE = "obs-date";
template<class T> const String ImageHeader<T>::_OBSERVER = "observer";
template<class T> const String ImageHeader<T>::_PROJECTION = "projection";
template<class T> const String ImageHeader<T>::_RESTFREQ = "reffreq";
template<class T> const String ImageHeader<T>::_REFFREQTYPE = "reffreqtype";
template<class T> const String ImageHeader<T>::_SHAPE = "shape";
template<class T> const String ImageHeader<T>::_TELESCOPE = "telescope";

template<class T> ImageHeader<T>::ImageHeader(
	const std::tr1::shared_ptr<ImageInterface<T> >& image
) : _image(image), _log(new LogIO()) {}

template<class T> Record ImageHeader<T>::toRecord(Bool verbose) {
	ImageSummary<T> summary(*_image);

	_header.define(_IMTYPE, summary.imageType());
	const ImageInfo& info = _image->imageInfo();

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
	Vector<Double> crpix = summary.referencePixels(True);
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

template<class T> void ImageHeader<T>::_fieldToLog(const String& field) const {
	*_log << "        -- " << field << ": ";
	if (_header.isDefined(field)) {
		DataType type = _header.type(_header.idToNumber(field));
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

template<class T> void ImageHeader<T>::_toLog() const {
	*_log << LogOrigin("ImageHeader", __FUNCTION__, WHERE);
	*_log << "General --" << LogIO::POST;
	_fieldToLog(_IMTYPE);
	_fieldToLog(_OBJECT);
	_fieldToLog(_EQUINOX);
	_fieldToLog(_OBSDATE);
	_fieldToLog(_OBSERVER);
	_fieldToLog(_PROJECTION);
	_fieldToLog(_RESTFREQ);
	_fieldToLog(_REFFREQTYPE);
	_fieldToLog(_TELESCOPE);
	_fieldToLog(_BEAMMAJOR);
	_fieldToLog(_BEAMMINOR);
	_fieldToLog(_BEAMPA);
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
		*_log << "        -- " << key << ": " << _header.asDouble(key);
		String unit = "cunit" + iString;
		if (_header.isDefined(unit)) {
			*_log << _header.asString(unit);
		}
		*_log << LogIO::POST;
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
		*_log << "        -- " << key << ": " << _header.asDouble(key);
		String unit = "cunit" + iString;
		if (_header.isDefined(unit)) {
			*_log << _header.asString(unit);
		}
		*_log << LogIO::POST;
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

}


