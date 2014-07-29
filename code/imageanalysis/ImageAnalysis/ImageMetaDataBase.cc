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

#include <imageanalysis/ImageAnalysis/ImageMetaDataBase.h>

#include <casa/aips.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/ValueHolder.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/DataType.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageStatistics.h>
#include <measures/Measures/MeasureHolder.h>

#include <iostream>
#include <iomanip>

#include <boost/regex.hpp>

#define _ORIGINB LogOrigin("ImageMetaDataBase", __FUNCTION__, WHERE)

namespace casa {

const String ImageMetaDataBase::_BEAMMAJOR = "beammajor";
const String ImageMetaDataBase::_BEAMMINOR = "beamminor";
const String ImageMetaDataBase::_BEAMPA = "beampa";
const String ImageMetaDataBase::_BMAJ = "bmaj";
const String ImageMetaDataBase::_BMIN = "bmin";
const String ImageMetaDataBase::_BPA = "bpa";
const String ImageMetaDataBase::_BUNIT = "bunit";
const String ImageMetaDataBase::_CDELT = "cdelt";
const String ImageMetaDataBase::_CRPIX = "crpix";
const String ImageMetaDataBase::_CRVAL = "crval";
const String ImageMetaDataBase::_CTYPE = "ctype";
const String ImageMetaDataBase::_CUNIT = "cunit";
const String ImageMetaDataBase::_DATAMAX = "datamax";
const String ImageMetaDataBase::_DATAMIN = "datamin";
const String ImageMetaDataBase::_EPOCH = "epoch";
const String ImageMetaDataBase::_EQUINOX = "equinox";
const String ImageMetaDataBase::_IMTYPE = "imtype";
const String ImageMetaDataBase::MASKS = "masks";
const String ImageMetaDataBase::_MAXPIXPOS = "maxpixpos";
const String ImageMetaDataBase::_MAXPOS = "maxpos";
const String ImageMetaDataBase::_MINPIXPOS = "minpixpos";
const String ImageMetaDataBase::_MINPOS = "minpos";
const String ImageMetaDataBase::_OBJECT = "object";
const String ImageMetaDataBase::_OBSDATE = "date-obs";
const String ImageMetaDataBase::_OBSERVER = "observer";
const String ImageMetaDataBase::_PROJECTION = "projection";
const String ImageMetaDataBase::_RESTFREQ = "restfreq";
const String ImageMetaDataBase::_REFFREQTYPE = "reffreqtype";
const String ImageMetaDataBase::_SHAPE = "shape";
const String ImageMetaDataBase::_TELESCOPE = "telescope";

Record ImageMetaDataBase::_makeHeader() const {
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
		static const String recName = "perplanebeams";
		Record beamRec = rec.asRecord(recName);
		beamRec.defineRecord(
			"median area beam", info.getBeamSet().getMedianAreaBeam().toRecord()
		);
		header.defineRecord(recName, beamRec);
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

const TableRecord ImageMetaDataBase::_miscInfo() const {
	std::tr1::shared_ptr<const ImageInterface<Float> > imf = _getFloatImage();
	std::tr1::shared_ptr<const ImageInterface<Complex> > imc = _getComplexImage();
	const TableRecord misc = imf ? imf->miscInfo() : imc->miscInfo();
	return misc;
}

ValueHolder ImageMetaDataBase::getFITSValue(const String& key) const {
	String c = key;
	c.downcase();
	const TableRecord info = _miscInfo();
	if (c == _BUNIT) {
		return ValueHolder(_getBrightnessUnit());
	}
	else if (
		c.startsWith(_CDELT) || c.startsWith(_CRPIX)
		|| c.startsWith(_CRVAL) || c.startsWith(_CTYPE)
		|| c.startsWith(_CUNIT)
	) {
		String prefix = c.substr(0, 5);
		uInt n = _getAxisNumber(c);
		if (prefix == _CDELT) {
			return ValueHolder(
				QuantumHolder(
					_getIncrements()[n-1]
			    ).toRecord()
			);
		}
		else if (prefix == _CRPIX) {
			return ValueHolder(_getRefPixel()[n-1]);
		}
		else if (prefix == _CRVAL) {
			if (_getCoords().polarizationAxisNumber(False) == (Int)(n-1)) {
				return ValueHolder(
					_getStokes()
				);
			}
			else {
				return ValueHolder(
					QuantumHolder(_getRefValue()[n-1]).toRecord()
				);
			}
		}
		else if (prefix == _CTYPE) {
			return ValueHolder(_getAxisNames()[n-1]);
		}
		else if (prefix == _CUNIT) {
			return ValueHolder(_getAxisUnits()[n-1]);
		}
	}
	else if (c == _EQUINOX) {
		return ValueHolder(_getEquinox());
	}
	else if (c == _IMTYPE) {
		return ValueHolder(_getImType());
	}
	else if (c == MASKS) {
		return ValueHolder(_getMasks());
	}
	else if (c == _OBJECT) {
		return ValueHolder(_getObject());
	}
	else if (c == _OBSDATE || c == _EPOCH) {
		return ValueHolder(_getEpochString());
	}
	else if (c == _OBSERVER) {
		return ValueHolder(_getObserver());
	}
	else if (c == _PROJECTION) {
		return ValueHolder(_getProjection());
	}
	else if (c == _REFFREQTYPE) {
		return ValueHolder(_getRefFreqType());
	}
	else if (c == _RESTFREQ) {
		return ValueHolder(
			QuantumHolder(_getRestFrequency()).toRecord()
		);
	}
	else if (c == _SHAPE) {
		return ValueHolder(_getShape().asVector());
	}
	else if (c == _TELESCOPE) {
		return ValueHolder(_getTelescope());
	}
	else if (
		c == _BEAMMAJOR || c == _BEAMMINOR || c == _BEAMPA
		|| c == _BMAJ || c == _BMIN || c == _BPA
	) {
		GaussianBeam beam = _getBeam();
		if (c == _BEAMMAJOR || c == _BMAJ) {
			return ValueHolder(QuantumHolder(beam.getMajor()).toRecord());
		}
		else if (c == _BEAMMINOR || c == _BMIN) {
			return ValueHolder(QuantumHolder(beam.getMinor()).toRecord());
		}
		else {
			return ValueHolder(QuantumHolder(beam.getPA()).toRecord());
		}
	}
	else if (
		c == _DATAMIN || c == _DATAMAX || c == _MINPIXPOS
        || c == _MINPOS || c == _MAXPIXPOS || c == _MAXPOS
    ) {
		Record x = _getStatistics();
		if (c == _DATAMIN || c == _DATAMAX) {
			if (_getFloatImage()) {
				Float val;
				x.get(c, val);
				return ValueHolder(val);
			}
			else {
				Complex val;
				x.get(c, val);
				return ValueHolder(val);
			}
		}
		else if (c == _MINPOS || c == _MAXPOS) {
			return ValueHolder(x.asString(c));
		}
		else if (c == _MINPIXPOS || c == _MAXPIXPOS) {
			return ValueHolder(x.asArrayInt(c));
		}
	}
	else if (
		info.isDefined(key)	|| info.isDefined(c)
	) {
		String x = info.isDefined(key) ? key : c;
		switch (info.type(info.fieldNumber(x))) {
		case TpString:
			return ValueHolder(info.asString(x));
			break;
		case TpInt:
			return ValueHolder(info.asInt(x));
			break;
		case TpDouble:
			return ValueHolder(info.asDouble(x));
			break;
		case TpRecord:
			// allow fall through
		case TpQuantity: {
			return ValueHolder(info.asRecord(x));
			break;
		}
		default:
			ostringstream os;
			os << info.type(info.fieldNumber(x));
			ThrowCc(
				"Unhandled data type "
				+ os.str()
				+ " for user defined type. Send us a bug report"
			);
		}
	}
	ThrowCc(
		"Unknown keyword " + c
	);
	return ValueHolder();
}

uInt ImageMetaDataBase::_ndim() const {
	std::tr1::shared_ptr<const ImageInterface<Float> > imf = _getFloatImage();
	std::tr1::shared_ptr<const ImageInterface<Complex> > imc = _getComplexImage();
	uInt ndim = imf ? imf->ndim() : imc->ndim();
	return ndim;
}


uInt ImageMetaDataBase::_getAxisNumber(
	const String& key
) const {
	uInt n = 0;
	string sre = key.substr(0, 5) + "[0-9]+";
	boost::regex re;
	re.assign(sre, boost::regex_constants::icase);
	if (boost::regex_match(key, re)) {
		n = String::toInt(key.substr(5));
		uInt ndim = _ndim();
		if (n == 0) {
			_log << _ORIGINB << "The FITS convention is that axes "
				<< "are 1-based. Therefore, " << key << " is not a valid "
				<< "FITS keyword specification" << LogIO::EXCEPTION;
		}
		else if (n > ndim) {
			_log << _ORIGINB << "This image only has " << ndim
				<< " axes." << LogIO::EXCEPTION;
		}
	}
	else {
		_log << "Unsupported key " << key << LogIO::EXCEPTION;
	}
	return n;
}

String ImageMetaDataBase::_getEpochString() const {
	return MVTime(_getObsDate().getValue()).string(MVTime::YMD);
}

IPosition ImageMetaDataBase::_getShape() const {
	if (_shape.empty()) {
		SPCIIF imf = _getFloatImage();
		SPCIIC imc = _getComplexImage();
		_shape = imf ? imf->shape() : imc->shape();
	}
	return _shape;
}

void ImageMetaDataBase::_fieldToLog(
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

void ImageMetaDataBase::_toLog(const Record& header) const {
	_log << _ORIGINB << "General --" << LogIO::POST;
	_fieldToLog(header, _IMTYPE);
	_fieldToLog(header, _OBJECT);
	_fieldToLog(header, _EQUINOX);
	_fieldToLog(header, _OBSDATE);
	_fieldToLog(header, _OBSERVER);
	_fieldToLog(header, _PROJECTION);
	if (header.isDefined(_RESTFREQ)) {
		_log << "        -- " << _RESTFREQ << ": ";
		_log.output() << std::fixed << std::setprecision(1);
		_log <<  header.asArrayDouble(_RESTFREQ) << LogIO::POST;
	}
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

String ImageMetaDataBase::_doStandardFormat(
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

uInt ImageMetaDataBase::nChannels() const {
	const CoordinateSystem csys = _getCoords();
	if (! csys.hasSpectralAxis()) {
		return 0;
	}
	return _getShape()[csys.spectralAxisNumber()];
}

Bool ImageMetaDataBase::isChannelNumberValid(
	const uInt chan
) const {
	if (! _getCoords().hasSpectralAxis()) {
		return False;
    }
	return (chan < nChannels());
}

uInt ImageMetaDataBase::nStokes() const {
	const CoordinateSystem& csys = _getCoords();

	if (! csys.hasPolarizationCoordinate()) {
		return 0;
    }
	return _getShape()[csys.polarizationAxisNumber()];
}

Int ImageMetaDataBase::stokesPixelNumber(
	const String& stokesString) const {
	Int pixNum = _getCoords().stokesPixelNumber(stokesString);
	if (pixNum >= (Int)nStokes()) {
		pixNum = -1;
    }
	return pixNum;
}

String ImageMetaDataBase::_getProjection() const {
	const CoordinateSystem csys = _getCoords();
	if (! csys.hasDirectionCoordinate()) {
		return "";
	}
	const DirectionCoordinate dc = csys.directionCoordinate();
	Projection proj = dc.projection();
	if (proj.type() == Projection::SIN) {
		Vector<Double> pars =  proj.parameters();
		if (dc.isNCP()) {
			ostringstream os;
			os << "SIN (" << pars << "): NCP";
			return os.str();
		}
		else if(pars.size() == 2 && (anyNE(pars, 0.0))) {
			// modified SIN
			ostringstream os;
			os << "SIN (" << pars << ")";
			return os.str();
		}
	}
	return proj.name();
}

String ImageMetaDataBase::stokesAtPixel(
	const uInt pixel
) const {
	const CoordinateSystem& csys = _getCoords();
	if (! csys.hasPolarizationCoordinate() || pixel >= nStokes()) {
             return "";
        }
	return csys.stokesAtPixel(pixel);
}

Bool ImageMetaDataBase::isStokesValid(
	const String& stokesString
) const {
	if (! _getCoords().hasPolarizationCoordinate()) {
		return False;
    }
	Int stokesPixNum = stokesPixelNumber(stokesString);
	return stokesPixNum >= 0 && stokesPixNum < (Int)nStokes();
}

Vector<Int> ImageMetaDataBase::directionShape() const {
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

Bool ImageMetaDataBase::areChannelAndStokesValid(
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

Record ImageMetaDataBase::_calcStats() const {
	std::tr1::shared_ptr<const ImageInterface<Float> > imf = _getFloatImage();
	std::tr1::shared_ptr<const ImageInterface<Complex> > imc = _getComplexImage();
	if (imf) {
		return _calcStatsT(imf);
	}
	else {
		return _calcStatsT(imc);
	}
}

template <class T> Record ImageMetaDataBase::_calcStatsT(
	std::tr1::shared_ptr<const ImageInterface<T> > image
) const {
	if ( _getComplexImage()) {
		// the min and max and associated positions
		// cannot be calculated for complex images
		return Record();
	}
	ImageStatistics<T> stats(*image);
	Array<typename NumericTraits<T>::PrecisionType> min;
	stats.getStatistic(min, LatticeStatsBase::MIN);
	Record x;
	x.define(ImageMetaDataBase::_DATAMIN, min(IPosition(min.ndim(), 0)));
	Array<typename NumericTraits<T>::PrecisionType> max;
	stats.getStatistic(max, LatticeStatsBase::MAX);
	x.define(ImageMetaDataBase::_DATAMAX, max(IPosition(max.ndim(), 0)));
	IPosition minPixPos, maxPixPos;
	stats.getMinMaxPos(minPixPos, maxPixPos);
	x.define(ImageMetaDataBase::_MINPIXPOS, minPixPos.asVector());
	x.define(ImageMetaDataBase::_MAXPIXPOS, maxPixPos.asVector());

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
	x.define(ImageMetaDataBase::_MINPOS, minFormat);
	x.define(ImageMetaDataBase::_MAXPOS, maxFormat);
	return x;
}

} //# NAMESPACE CASA - END

