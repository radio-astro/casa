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
#include <measures/Measures/MeasureHolder.h>
#include <casa/Utilities/DataType.h>

#include <stdcasa/StdCasa/CasacSupport.h>
#include <stdcasa/variant.h>

#include <iostream>
#include <iomanip>

#include <boost/regex.hpp>

#define _ORIGINB LogOrigin("ImageMetaData<T>", __FUNCTION__, WHERE)

namespace casa {

template<class T> const String ImageMetaData<T>::_BEAMMAJOR = "beammajor";
template<class T> const String ImageMetaData<T>::_BEAMMINOR = "beamminor";
template<class T> const String ImageMetaData<T>::_BEAMPA = "beampa";
template<class T> const String ImageMetaData<T>::_BMAJ = "bmaj";
template<class T> const String ImageMetaData<T>::_BMIN = "bmin";
template<class T> const String ImageMetaData<T>::_BPA = "bpa";
template<class T> const String ImageMetaData<T>::_BUNIT = "bunit";
template<class T> const String ImageMetaData<T>::_CDELT = "cdelt";
template<class T> const String ImageMetaData<T>::_CRPIX = "crpix";
template<class T> const String ImageMetaData<T>::_CRVAL = "crval";
template<class T> const String ImageMetaData<T>::_CTYPE = "ctype";
template<class T> const String ImageMetaData<T>::_CUNIT = "cunit";
template<class T> const String ImageMetaData<T>::_DATAMAX = "datamax";
template<class T> const String ImageMetaData<T>::_DATAMIN = "datamin";
template<class T> const String ImageMetaData<T>::_EPOCH = "epoch";
template<class T> const String ImageMetaData<T>::_EQUINOX = "equinox";
template<class T> const String ImageMetaData<T>::_IMTYPE = "imtype";
template<class T> const String ImageMetaData<T>::MASKS = "masks";
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
	const ImageInterface<T> *const &image
) : _image(image), _log(), _info(image->imageInfo()),
	_csys(image->coordinates()) {}

template<class T> Record ImageMetaData<T>::_makeHeader() const {
	Record header;
		header.define(_IMTYPE, _getImType());
		header.define(_OBJECT, _getObject());

		const CoordinateSystem& csys = _getCoords();

		if (csys.hasDirectionCoordinate()) {
			const DirectionCoordinate& dc = csys.directionCoordinate();
			String equinox = MDirection::showType(
				dc.directionType()
			);
			header.define(_EQUINOX, _getEquinox());
			header.define(_PROJECTION, _getProjection());
		}
		header.define(_OBSDATE, _getEpochString());
		header.define(MASKS, _getMasks());
		header.define(_OBSERVER, _getObserver());
		header.define(_SHAPE, _getShape().asVector());
		header.define(_TELESCOPE, _getTelescope());
		header.define(_BUNIT, _getBrightnessUnit());

		if (csys.hasSpectralAxis()) {
			const SpectralCoordinate& sc = csys.spectralCoordinate();
			header.define(_RESTFREQ, sc.restFrequencies());
			header.define(
				_REFFREQTYPE , _getRefFreqType()
			);
		}

		const ImageInfo& info = _getInfo();
		if (info.hasSingleBeam()) {
			GaussianBeam beam = _getBeam();
			header.defineRecord(
				_BEAMMAJOR,
				QuantumHolder(beam.getMajor()).toRecord()
			);
			header.defineRecord(
				_BEAMMINOR,
				QuantumHolder(beam.getMinor()).toRecord()
			);
			header.defineRecord(
				_BEAMPA,
				QuantumHolder(beam.getPA(True)).toRecord()
			);
		}
		else if (info.hasMultipleBeams()) {
			String error;
			Record rec;
			info.toRecord(error, rec);
			header.defineRecord(
				"perplanebeams", rec.asRecord("perplanebeams")
			);
		}
		vector<Quantity> cdelt = _getIncrements();
		Vector<String> units = _getAxisUnits();
		Vector<Double> crpix = _getRefPixel();
		Vector<Quantity> crval = _getRefValue();
		Vector<String> types = _getAxisNames();
		header.merge(_getStatistics());

		for (uInt i=0; i<cdelt.size(); i++) {
			String iString = String::toString(i + 1);
			String delt = _CDELT + iString;
			header.define(delt, cdelt[i].getValue());
			String unit = _CUNIT + iString;
			header.define(unit, units[i]);
			String pix = _CRPIX + iString;
			header.define(pix, crpix[i]);
			String val = _CRVAL + iString;
			header.define(val, crval[i].getValue());
			String type = _CTYPE + iString;
			header.define(type, types[i]);
		}
	return header;
}

template<class T> Record ImageMetaData<T>::toRecord(Bool verbose) const {
	if (_header.empty()) {
		_header = _makeHeader();
	}
	if (verbose) {
		_toLog(_header);
	}
	return _header;
}


template<class T> casac::variant ImageMetaData<T>::getFITSValue(const String& key) const {
	String c = key;
	c.downcase();
	if (c == _BUNIT) {
		return _getBrightnessUnit();
	}
	else if (
		c.startsWith(_CDELT) || c.startsWith(_CRPIX)
		|| c.startsWith(_CRVAL) ||  c.startsWith(_CTYPE)
		||  c.startsWith(_CUNIT)
	) {
		String prefix = c.substr(0, 5);
		uInt n = _getAxisNumber(c);
		if (prefix == _CDELT) {
			return casac::variant(
				fromRecord(
					QuantumHolder(_getIncrements()[n-1]).toRecord()
				)
			);
		}
		else if (prefix == _CRPIX) {
			return _getRefPixel()[n-1];
		}
		else if (prefix == _CRVAL) {
			return casac::variant(
				fromRecord(
					QuantumHolder(_getRefValue()[n-1]).toRecord()
				)
			);
		}
		else if (prefix == _CTYPE) {
			return _getAxisNames()[n-1];
		}
		else if (prefix == _CUNIT) {
			return _getAxisUnits()[n-1];
		}
	}
	else if (c == _EQUINOX) {
		return _getEquinox();
	}
	else if (c == _IMTYPE) {
		return casac::variant(_getImType());
	}
	else if (c == MASKS) {
		Vector<String>  masks = _getMasks().tovector();
		vector<string> v(masks.size());
		for (uInt i=0; i<masks.size(); i++) {
			v[i] = std::string(masks[i].c_str());
		}
		return casac::variant(v);
	}
	else if (c == _OBJECT) {
		return _getObject();
	}
	else if (c == _OBSDATE || c == _EPOCH) {
		return _getEpochString();
	}
	else if (c == _OBSERVER) {
		return _getObserver();
	}
	else if (c == _PROJECTION) {
		return _getProjection();
	}
	else if (c == _REFFREQTYPE) {
		return _getRefFreqType();
	}
	else if (c == _RESTFREQ) {
		QuantumHolder qh(_getRestFrequency());
		casac::record *rec = fromRecord(qh.toRecord());
		return casac::variant(rec);
	}
	else if (c == _SHAPE) {
		return casac::variant(_getShape().asVector().tovector());
	}
	else if (c == _TELESCOPE) {
		return _getTelescope();
	}
	else if (
		c == _BEAMMAJOR || c == _BEAMMINOR || c == _BEAMPA
		|| c == _BMAJ || c == _BMIN || c == _BPA
	) {
		GaussianBeam beam = _getBeam();
		casac::record *rec;
		if (c == _BEAMMAJOR || c == _BMAJ) {
			rec = fromRecord(QuantumHolder(beam.getMajor()).toRecord());
		}
		else if (c == _BEAMMINOR || c == _BMIN) {
			rec = fromRecord(QuantumHolder(beam.getMinor()).toRecord());
		}
		else {
			rec = fromRecord(QuantumHolder(beam.getPA()).toRecord());
		}
		return casac::variant(rec);
	}
	else if (
		c==_DATAMIN || c == _DATAMAX || c == _MINPIXPOS
        || c == _MINPOS || c == _MAXPIXPOS || c == _MAXPOS
    ) {
		Record x = _getStatistics();
		if (c == _DATAMIN || c == _DATAMAX) {
			T val;
			x.get(c, val);
			return val;
		}
		else if (c == _MINPOS || c == _MAXPOS) {
			return x.asString(c);
		}
		else if (c == _MINPIXPOS || c == _MAXPIXPOS) {
			return casac::variant(x.asArrayInt(c).tovector());
		}
	}
	else if (
		_getImage()->miscInfo().isDefined(key)
		|| _getImage()->miscInfo().isDefined(c)
	) {
		TableRecord info = _getImage()->miscInfo();
		String x = info.isDefined(key) ? key : c;
		switch (info.type(info.fieldNumber(x))) {
		case TpString:
			return casac::variant(info.asString(x));
		case TpInt:
			return casac::variant(info.asInt(x));
		case TpDouble:
			return casac::variant(info.asDouble(x));
		case TpRecord:
			// allow fall through
		case TpQuantity: {
			std::auto_ptr<casac::record> rec(fromRecord(info.asRecord(x)));
			return casac::variant(*rec);
		}
		default:
			_log << _ORIGINB << "Unhandled data type "
				<< info.type(info.fieldNumber(x)) << " for "
				<< "user defined type. Send us a bug report."
				<< LogIO::EXCEPTION;
			break;
		}
	}
	_log << _ORIGINB << "Unknown keyword " << c << LogIO::EXCEPTION;
	return casac::variant();
}

template<class T> uInt ImageMetaData<T>::_getAxisNumber(
	const String& key
) const {
	uInt n = 0;
	string sre = key.substr(0, 5) + "[0-9]+";
	boost::regex re;
	re.assign(sre, boost::regex_constants::icase);
	if (boost::regex_match(key, re)) {
		n = String::toInt(key.substr(5));
		uInt naxes = _getImage()->ndim();
		if (n == 0) {
			_log << _ORIGINB << "The FITS convention is that axes "
				<< "are 1-based. Therefore, " << key << " is not a valid "
				<< "FITS keyword specification" << LogIO::EXCEPTION;
		}
		else if (n > naxes) {
			_log << _ORIGINB << "This image only has " << naxes
				<< " axes." << LogIO::EXCEPTION;
		}
	}
	else {
		_log << "Unsupported key " << key << LogIO::EXCEPTION;
	}
	return n;
}


template<class T> String ImageMetaData<T>::_getEpochString() const {
	return MVTime(_getObsDate().getValue()).string(MVTime::YMD);
}


template<class T> Vector<String> ImageMetaData<T>::_getAxisNames() const {
	if (_axisNames.size() == 0) {
		_axisNames = _getCoords().worldAxisNames();
	}
	return _axisNames;
}

template<class T> Vector<String> ImageMetaData<T>::_getAxisUnits() const {
	if (_axisUnits.size() == 0) {
		_axisUnits = _getCoords().worldAxisUnits();
	}
	return _axisUnits;
}

template<class T> GaussianBeam ImageMetaData<T>::_getBeam() const {
	const ImageInfo& info = _getInfo();
	if (info.hasSingleBeam()) {
		if (_beam == GaussianBeam::NULL_BEAM) {
			_beam = info.restoringBeam(-1, -1);
		}
		return _beam;
	}
	else if (info.hasMultipleBeams()) {
		throw AipsError("This image has multiple beams.");
	}
	else {
		throw AipsError("This image has no beam(s).");
	}
}

template<class T> String ImageMetaData<T>::_getBrightnessUnit() const {
	if (_bunit.empty()) {
		_bunit = _getImage()->units().getName();
	}
	return _bunit;
}

template<class T> String ImageMetaData<T>::_getEquinox() const {
	if (_equinox.empty()) {
		if (_getCoords().hasDirectionCoordinate()) {
			_equinox = MDirection::showType(
				_getCoords().directionCoordinate().directionType()
			);
		}
	}
	return _equinox;
}

template<class T> String ImageMetaData<T>::_getImType() const {
	if (_imtype.empty()) {
		_imtype = ImageInfo::imageType(_getInfo().imageType());
	}
	return _imtype;
}

template<class T> vector<Quantity> ImageMetaData<T>::_getIncrements() const {
	if (_increment.size() == 0) {
		Vector<Double> incs = _getCoords().increment();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<incs.size(); i++) {
			_increment.push_back(Quantity(incs[i], units[i]));
		}
	}
	return _increment;
}

template<class T> String ImageMetaData<T>::_getObject() const {
	if (_object.empty()) {
		_object = _getInfo().objectName();
	}
	return _object;
}

template<class T> Vector<String> ImageMetaData<T>::_getMasks() const {
	if (_masks.empty()) {
		_masks = _getImage()->regionNames(RegionHandler::Masks);
	}
	return _masks;
}

template<class T> MEpoch ImageMetaData<T>::_getObsDate() const {
	if (_obsdate.get("s") == 0) {
		_obsdate = _getCoords().obsInfo().obsDate();
	}
	return _obsdate;
}

template<class T> String ImageMetaData<T>::_getObserver() const {
	if (_observer.empty()) {
		_observer = _getCoords().obsInfo().observer();
	}
	return _observer;
}

template<class T> String ImageMetaData<T>::_getProjection() const {
	if (_projection.empty() && _getCoords().hasDirectionCoordinate()) {
		_projection = _getCoords().directionCoordinate().projection().name();
	}
	return _projection;
}

template<class T> Vector<Double> ImageMetaData<T>::_getRefPixel() const {
	if (_refPixel.size() == 0) {
		_refPixel = _getCoords().referencePixel();
	}
	return _refPixel;
}

template<class T> Vector<Quantity> ImageMetaData<T>::_getRefValue() const {
	if (_refVal.size() == 0) {
		Vector<Double> vals = _getCoords().referenceValue();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<vals.size(); i++) {
			_refVal.push_back(Quantity(vals[i], units[i]));
		}
	}
	return _refVal;
}

template<class T> String ImageMetaData<T>::_getRefFreqType() const {
	if (_reffreqtype.empty() && _getCoords().hasSpectralAxis()) {
		_reffreqtype = MFrequency::showType(
			_getCoords().spectralCoordinate().frequencySystem(False)
		);
	}
	return _reffreqtype;
}

template<class T> Quantity ImageMetaData<T>::_getRestFrequency() const {
	const CoordinateSystem& csys = _getCoords();
	if (_restFreq.getValue() == 0 && csys.hasSpectralAxis()) {
		_restFreq = Quantity(
			csys.spectralCoordinate().restFrequency(),
			csys.spectralCoordinate().worldAxisUnits()[0]
		);
	}
	return _restFreq;
}

template<class T> IPosition ImageMetaData<T>::_getShape() const {
	if (_shape.empty()) {
		_shape = _getImage()->shape();
	}
	return _shape;
}

template<class T> Record ImageMetaData<T>::_getStatistics() const {
	if (_stats.nfields() == 0) {
		ImageStatistics<T> stats(*_getImage());
		Array<Double> min;
		stats.getStatistic(min, LatticeStatsBase::MIN);
		Record x;
		x.define(_DATAMIN, min(IPosition(min.ndim(), 0)));
		Array<Double> max;
		stats.getStatistic(max, LatticeStatsBase::MAX);
		x.define(_DATAMAX, max(IPosition(max.ndim(), 0)));
		IPosition minPixPos, maxPixPos;
		stats.getMinMaxPos(minPixPos, maxPixPos);
		x.define(_MINPIXPOS, minPixPos.asVector());
		x.define(_MAXPIXPOS, maxPixPos.asVector());
		const CoordinateSystem& csys = _getCoords();
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
		x.define(_MINPOS, minFormat);
		x.define(_MAXPOS, maxFormat);
		_stats = x;
	}
	return _stats;
}


template<class T> String ImageMetaData<T>::_getTelescope() const {
	if (_telescope.empty()) {
		_telescope = _getCoords().obsInfo().telescope();
	}
	return _telescope;
}

template<class T> void ImageMetaData<T>::_fieldToLog(
	const Record& header,const String& field, Int precision
) const {
	_log << "        -- " << field << ": ";
	if (header.isDefined(field)) {
		DataType type = header.type(header.idToNumber(field));
		if (precision >= 0) {
			_log.output() << setprecision(precision);
		}
		switch (type) {
			case TpArrayDouble: {
				_log << header.asArrayDouble(field);
				break;
			}
			case TpArrayInt: {
				_log << header.asArrayInt(field);
				break;
			}
			case TpArrayString: {
				_log << header.asArrayString(field);
				break;
			}
			case TpDouble: {
				_log << header.asDouble(field);
				break;
			}
			case TpRecord: {
				Record r = header.asRecord(field);
				QuantumHolder qh;
				String error;
				if (qh.fromRecord(error, r) && qh.isQuantity()) {
					Quantity q = qh.asQuantity();
					_log << q.getValue() << q.getUnit();
				}
				else {
					_log << "Logic Error: Don't know how to deal with records of this type "
						<< LogIO::EXCEPTION;
				}
				break;
			}
			case TpString: {
				_log << header.asString(field);
				break;
			}

			default: {
				_log << "Logic Error: Unsupported type "
					<< type << LogIO::EXCEPTION;
				break;
			}
		}
	}
	else {
		_log << "Not found";
	}
	_log << LogIO::POST;
}

template<class T> void ImageMetaData<T>::_toLog(const Record& header) const {
	_log << _ORIGINB << "General --" << LogIO::POST;
	_fieldToLog(header, _IMTYPE);
	_fieldToLog(header, _OBJECT);
	_fieldToLog(header, _EQUINOX);
	_fieldToLog(header, _OBSDATE);
	_fieldToLog(header, _OBSERVER);
	_fieldToLog(header, _PROJECTION);
	_log << "        -- " << _RESTFREQ << ": ";
	_log.output() << std::fixed << std::setprecision(1);
	_log <<  header.asArrayDouble(_RESTFREQ) << LogIO::POST;
	_fieldToLog(header, _REFFREQTYPE);
	_fieldToLog(header, _TELESCOPE);
	_fieldToLog(header, _BEAMMAJOR, 12);
	_fieldToLog(header, _BEAMMINOR, 12);
	_fieldToLog(header, _BEAMPA, 12);
	_fieldToLog(header, _BUNIT);
	_fieldToLog(header, MASKS);
	_fieldToLog(header, _SHAPE);
	_fieldToLog(header, _DATAMIN);
	_fieldToLog(header, _DATAMAX);
	_fieldToLog(header, _MINPOS);
	_fieldToLog(header, _MINPIXPOS);
	_fieldToLog(header, _MAXPOS);
	_fieldToLog(header, _MAXPIXPOS);

	uInt i = 1;
	_log << LogIO::NORMAL << "axes --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CTYPE + iString;
		if (! header.isDefined(key)) {
			break;
		}
		_log << "        -- " << key << ": "
			<< header.asString(key) << LogIO::POST;
		String unit = _CUNIT + iString;
		i++;
	}
	i = 1;
	_log << LogIO::NORMAL << _CRPIX << " --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CRPIX + iString;
		if (! header.isDefined(key)) {
			break;
		}
		_log.output() << std::fixed << std::setprecision(1);
		_log << "        -- " << key << ": " << header.asDouble(key)
			<< LogIO::POST;
		i++;
	}
	i = 1;
	_log << LogIO::NORMAL << _CRVAL << " --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CRVAL + iString;
		if (! header.isDefined(key)) {
			break;
		}
		_log << "        -- " << key << ": ";
		ostringstream x;
		Double val = header.asDouble(key);
		x << val;
		String unit = _CUNIT + iString;
		if (header.isDefined(unit)) {
			x << header.asString(unit);
		}
		String valunit = x.str();
		if (header.isDefined(unit)) {
			String myunit = header.asString(unit);
			if (header.asString(unit).empty()) {
				String ctype = _CTYPE + iString;
				if (
					header.isDefined(ctype)
					&& header.asString(ctype) == "Stokes"
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
		_log << valunit << LogIO::POST;
		i++;
	}
	i = 1;
	_log << LogIO::NORMAL << _CDELT << " --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CDELT + iString;
		if (! header.isDefined(key)) {
			break;
		}
		_log << "        -- " << key << ": ";
		Double val = header.asDouble(key);
		String unit = _CUNIT + iString;
		String myunit;
		if (header.isDefined(unit)) {
			myunit = header.asString(unit);
		}
		ostringstream x;
		x << val << myunit;
		String valunit = x.str();
		if (header.isDefined(unit)) {
			String myunit = header.asString(unit);
			if (! header.asString(unit).empty()) {
				String tmp = _doStandardFormat(val, myunit);
				if (! tmp.empty()) {
					valunit = tmp;
				}
			}
		}
		_log << valunit << LogIO::POST;
		i++;
	}
	i = 1;
	_log << LogIO::NORMAL << "units --" << LogIO::POST;
	while (True) {
		String iString = String::toString(i);
		String key = _CUNIT + iString;
		if (! header.isDefined(key)) {
			break;
		}
		_log << "        -- " << key << ": "
			<< header.asString(key) << LogIO::POST;
		String unit = _CUNIT + iString;
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
	const CoordinateSystem csys = _getCoords();
	if (! csys.hasSpectralAxis()) {
		return 0;
	}
	return _getImage()->shape()[csys.spectralAxisNumber()];
}

template <class T> Bool ImageMetaData<T>::isChannelNumberValid(
	const uInt chan
) const {
	if (! _getCoords().hasSpectralAxis()) {
		return False;
    }
	return (chan < nChannels());
}

template <class T> uInt ImageMetaData<T>::nStokes() const {
	const CoordinateSystem& csys = _getCoords();

	if (! csys.hasPolarizationCoordinate()) {
		return 0;
    }
	return _getImage()->shape()[csys.polarizationAxisNumber()];
}

template <class T> Int ImageMetaData<T>::stokesPixelNumber(
	const String& stokesString) const {
	Int pixNum = _getCoords().stokesPixelNumber(stokesString);
	if (pixNum >= (Int)nStokes()) {
		pixNum = -1;
    }
	return pixNum;
}

template <class T> String ImageMetaData<T>::stokesAtPixel(
	const uInt pixel
) const {
	const CoordinateSystem& csys = _getCoords();
	if (! csys.hasPolarizationCoordinate() || pixel >= nStokes()) {
             return "";
        }
	return csys.stokesAtPixel(pixel);
}

template <class T> Bool ImageMetaData<T>::isStokesValid(
	const String& stokesString
) const {
	if (! _getCoords().hasPolarizationCoordinate()) {
		return False;
    }
	Int stokesPixNum = stokesPixelNumber(stokesString);
	return stokesPixNum >= 0 && stokesPixNum < (Int)nStokes();
}

template <class T> Vector<Int> ImageMetaData<T>::directionShape() const {
	Vector<Int> dirAxesNums = _getCoords().directionAxesNumbers();
	if (dirAxesNums.nelements() == 0) {
		return Vector<Int>();
	}
	Vector<Int> dirShape(2);
	IPosition shape = _getShape();
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

