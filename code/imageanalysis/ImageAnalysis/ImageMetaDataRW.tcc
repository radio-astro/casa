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

#include <imageanalysis/ImageAnalysis/ImageMetaDataRW.h>

#include <stdcasa/StdCasa/CasacSupport.h>
#include <stdcasa/variant.h>
#include <stdcasa/record.h>

#include <casa/aips.h>

#define _LOCATEA "ImageMetaDataRW<T>" << __FUNCTION__ << " "
#define _ORIGINA LogOrigin("ImageMetaDataRW<T>", __FUNCTION__)

namespace casa {

template<class T> ImageMetaDataRW<T>::ImageMetaDataRW(
	ImageInterface<Float> *const &image
) : ImageMetaData<T>(), _image(image) {}


template<class T> Record ImageMetaDataRW<T>::toRecord(Bool verbose) const {
	if (_header.empty()) {
		_header = this->_makeHeader();
	}
	if (verbose) {
		this->_toLog(_header);
	}
	return _header;
}

template<class T> Bool ImageMetaDataRW<T>::add(const String& key, const casac::variant& value) {
	String c = key;
	c.downcase();
	ThrowIf(
		c.startsWith(ImageMetaData<T>::_CDELT)
		|| c.startsWith(ImageMetaData<T>::_CRPIX)
		|| c.startsWith(ImageMetaData<T>::_CRVAL)
		||  c.startsWith(ImageMetaData<T>::_CTYPE)
		||  c.startsWith(ImageMetaData<T>::_CUNIT),
		key + " pertains to a "
		+ "coordinate system axis attribute. It may be "
		+ "modified if it exists, but it may not be added."
	);
	ThrowIf(
		c == ImageMetaData<T>::_EQUINOX,
		"The direction reference frame ("
		+ key + "=" + _getEquinox()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaData<T>::MASKS,
		"This application does not support adding masks."
	);
	ThrowIf(
		c == ImageMetaData<T>::_OBSDATE || c == ImageMetaData<T>::_EPOCH,
		"The epoch (" + key + "=" + this->_getEpochString()
		+ ") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaData<T>::_PROJECTION,
		"The projection ("
		+ key + "=" + _getProjection()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaData<T>::_REFFREQTYPE,
		"The velocity reference frame ("
		+ key + "=" + _getProjection()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaData<T>::_SHAPE,
		"The shape is intrinsic to the image and may "
		"not be added."
	);
	ThrowIf(
		c == ImageMetaData<T>::_BEAMPA || c == ImageMetaData<T>::_BPA,
		"Cannot add a beam position "
		"angle. Add the major or minor axis and then "
		"modify the other and the position angle."
	);
	ThrowIf(
		c == ImageMetaData<T>::_DATAMIN
		|| c == ImageMetaData<T>::_DATAMAX
		|| c == ImageMetaData<T>::_MINPIXPOS
		|| c == ImageMetaData<T>::_MAXPIXPOS
		|| c == ImageMetaData<T>::_MINPOS
		|| c == ImageMetaData<T>::_MAXPOS,
		key + " is is a statistic of the image and may "
		"not be added."
	);
	if (c == ImageMetaData<T>::_BUNIT) {
		ThrowIf(
			! _getBrightnessUnit().empty(),
			key + " is already present and has value "
			+ _getBrightnessUnit() + ". It may be modified but not added."
		);
		String v = value.toString();
		ThrowIf(
			! _getImage()->setUnits(Unit(v)),
			"Unable to set " + key
		);
		_bunit = v;
	}
	else if (c == ImageMetaData<T>::_IMTYPE) {
		String imtype = _getImType();
		ThrowIf(
			! imtype.empty(),
			"The image type ("
			+ key + "=" + ImageInfo::imageType(imtype)
		    + ") already exists. It may be modified but be added."
		);
		set(c, value);
	}
	else if (c == ImageMetaData<T>::_OBJECT) {
		String object = _getObject();
		ThrowIf(
			! object.empty(),
			key + " is already present and has value "
			+ object + ". It may be modified but not added."
		);
		set(c, value);
	}
	else if (c == ImageMetaData<T>::_OBSERVER) {
		String observer = _getObserver();
		ThrowIf(
			! observer.empty(),
			key + " is already present and has value "
			+ observer + ".  It may be modified but not added."
		);
		set(c, value);
	}
	else if (c == ImageMetaData<T>::_RESTFREQ) {
		ThrowIf(
			_getRestFrequency().getValue() > 0,
			"The rest frequency ("
			+ key + "=" + String::toString(_getRestFrequency().getValue())
			+ _getRestFrequency().getUnit()
			+ ") already exists. It may be modified but not added "
			+ "by this application. If you wish to append a rest frequency "
			+ "to an already existing list, use cs.setrestfrequency()."
		);
		set(c, value);
	}
	else if (c == ImageMetaData<T>::_TELESCOPE) {
		String telescope = _getTelescope();
		ThrowIf(
			! telescope.empty(),
			key + " is already present and has value "
			+ telescope + ". It may be modified but not added."
		);
		set(c, value);
	}
	else if (
		c == ImageMetaData<T>::_BEAMMAJOR
		|| c == ImageMetaData<T>::_BEAMMINOR
		|| c == ImageMetaData<T>::_BMAJ
		|| c == ImageMetaData<T>::_BMIN
	) {
		ImageInfo info = _getInfo();
		ThrowIf(
			info.hasBeam(),
			"This image already has a beam(s). Cannot add one."
		);
		set(c, value);
	}
	else if (_image->miscInfo().isDefined(key) || _image->miscInfo().isDefined(c)) {
		ThrowCc("Keyword " + key + " already exists so cannot be added.");
	}
	else {
		_setUserDefined(key, value);
	}
	_header.assign(Record());
	return True;
}

template<class T> Bool ImageMetaDataRW<T>::remove(const String& key) {
	String c = key;
	LogIO log = this->_getLog();
	log << _ORIGINA;
	c.downcase();
	ThrowIf(
		c.startsWith(ImageMetaData<T>::_CDELT)
		|| c.startsWith(ImageMetaData<T>::_CRPIX)
		|| c.startsWith(ImageMetaData<T>::_CRVAL)
		||  c.startsWith(ImageMetaData<T>::_CTYPE)
		||  c.startsWith(ImageMetaData<T>::_CUNIT),
		key + " pertains to a "
		"coordinate system axis attribute. It may be "
		"modified, but it may not be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_EQUINOX,
		"Although the direction reference frame ("
		+ key + ") may be modified, it may not be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_IMTYPE,
		"Although the image type ("
		+ key + ") may be modified, it may not be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::MASKS,
		"Logic Error: removeMask() should be called instead"
	);
	ThrowIf(
		c == ImageMetaData<T>::_OBSDATE || c == ImageMetaData<T>::_EPOCH,
		"Although the epoch (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_PROJECTION,
		"Although the projection (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_REFFREQTYPE,
		"Although the velocity reference frame (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_RESTFREQ,
		"Although the rest frequency (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_SHAPE,
		"The shape is intrinsic to the image and may "
		"not be modified nor removed."
	);
	ThrowIf(
		c == ImageMetaData<T>::_DATAMIN
		|| c == ImageMetaData<T>::_DATAMAX
		|| c == ImageMetaData<T>::_MINPIXPOS
		|| c == ImageMetaData<T>::_MAXPIXPOS
		|| c == ImageMetaData<T>::_MINPOS
		|| c == ImageMetaData<T>::_MAXPOS,
		key + " is is a statistic of the image and may "
		+ "not be modified nor removed by this application."
	);
	if (c == ImageMetaData<T>::_BUNIT) {
		ThrowIf(
			! _getImage()->setUnits(Unit("")),
			"Unable to clear " + key
		);
		_bunit = "";
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
	}
	else if (c == ImageMetaData<T>::_OBJECT) {
		ImageInfo info = _getInfo();
		info.setObjectName("");
		_image->setImageInfo(info);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_object = "";
	}
	else if (c == ImageMetaData<T>::_OBSERVER) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObserver("");
		csys.setObsInfo(info);
		_image->setCoordinateInfo(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_observer = "";
	}
	else if (c == ImageMetaData<T>::_TELESCOPE) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setTelescope("");
		csys.setObsInfo(info);
		_image->setCoordinateInfo(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_telescope = "";
	}
	else if (
		c == ImageMetaData<T>::_BEAMMAJOR
		|| c == ImageMetaData<T>::_BEAMMINOR
		|| c == ImageMetaData<T>::_BEAMPA
		|| c == ImageMetaData<T>::_BMAJ
		|| c == ImageMetaData<T>::_BMIN
		|| c == ImageMetaData<T>::_BPA
	) {
		ImageInfo info = _getInfo();
		if (info.hasBeam()) {
			if (info.hasSingleBeam()) {
				log << LogIO::NORMAL << "Removing this image's single beam"
					<< LogIO::POST;
			}
			else {
				log << LogIO::NORMAL << "Removing all of this image's multiple beams"
					<< LogIO::POST;
			}
			info.removeRestoringBeam();
			_image->setImageInfo(info);
			_beam = GaussianBeam::NULL_BEAM;
		}
		else {
			log << LogIO::WARN << "This image has no beam(s) to remove." << LogIO::POST;
			return False;
		}
	}
	else if (_image->miscInfo().isDefined(key)) {
		TableRecord info = _image->miscInfo();
		info.removeField(key);
		_image->setMiscInfo(info);
		log << LogIO::NORMAL << "Removed user-defined keyword " << key << LogIO::POST;
	}
	else if (_image->miscInfo().isDefined(c)) {
		TableRecord info = _image->miscInfo();
		info.removeField(c);
		_image->setMiscInfo(info);
		log << LogIO::NORMAL << "Removed user-defined keyword " << c << LogIO::POST;
	}
	else {
		ThrowCc("Unknown keyword " + c);
	}
	_header.assign(Record());
	return True;
}

template<class T> Bool ImageMetaDataRW<T>::removeMask(const String& maskName) {
	ImageInterface<T> *image = _getImage();
	LogIO log = this->_getLog();
	log << _ORIGINA;
	if (maskName.empty()) {
		Vector<String> masks = _getMasks().copy();
		if (masks.size() == 0) {
			log << LogIO::WARN << "This image has no masks, so nothing to do."
				<< LogIO::POST;
			return True;
		}
		else {
			Vector<String>::const_iterator end = masks.end();
			for (
				Vector<String>::const_iterator iter=masks.begin();
				iter!=end; iter++
			) {
				cout << "iter " << *iter << endl;
				removeMask(*iter);
			}
			_masks.resize(0);
			return _getMasks().size() == 0;
		}
	}
	else {
		ThrowIf(
			! image->hasRegion(maskName, RegionHandler::Masks),
			"No mask named " + maskName + " found"
		);
		image->removeRegion(maskName, RegionHandler::Masks);
		ThrowIf(
			image->hasRegion(maskName, RegionHandler::Masks),
			"Unable to remove mask " + maskName
		);
		_masks.resize(0);
		log << LogIO::NORMAL << "Removed mask named " << maskName << endl;
		_header.assign(Record());
		return True;
	}
}

template<class T> Bool ImageMetaDataRW<T>::set(
	const String& key, const casac::variant& value
) {
	String c = key;
	c.downcase();
	if (c == ImageMetaData<T>::_BUNIT) {
		_checkString(key, value);
		String v = value.toString();
		if (_getImage()->setUnits(Unit(v))) {
			_bunit = v;
		}
		else {
			ThrowCc("Unable to set " + key);
		}
	}
	else if (
		c.startsWith(ImageMetaData<T>::_CDELT) || c.startsWith(ImageMetaData<T>::_CRPIX)
		|| c.startsWith(ImageMetaData<T>::_CRVAL) ||  c.startsWith(ImageMetaData<T>::_CTYPE)
		||  c.startsWith(ImageMetaData<T>::_CUNIT)
	) {
		_setCoordinateValue(c, value);
	}
	else if (c == ImageMetaData<T>::_EQUINOX) {
		ThrowIf(
			! _getCoords().hasDirectionCoordinate(),
			"This image does not have a direction "
			"coordinate and so a direction projection cannot be added."
		);
		_checkString(key, value);
		String v = value.toString();
		v.upcase();
		MDirection::Types type;
		ThrowIf(
			!MDirection::getType(type, v),
			"Unknown direction reference frame specification"
		);
		CoordinateSystem csys = _getCoords();
		DirectionCoordinate dircoord = csys.directionCoordinate();
		if (dircoord.directionType(False) == type) {
			// nothing to do
			return True;
		}
		dircoord.setReferenceFrame(type);
		csys.replaceCoordinate(dircoord, csys.directionCoordinateNumber());
		_getImage()->setCoordinateInfo(csys);
		_equinox = v;
	}
	else if (c == ImageMetaData<T>::_IMTYPE) {
		_checkString(key, value);
		String imtype = _getImType();
		ImageInfo info = _getImage()->imageInfo();
		String v = value.toString();
		info.setImageType(ImageInfo::imageType(v));
		_getImage()->setImageInfo(info);
		String newType = ImageInfo::imageType(info.imageType());
		_imtype = newType;
	}
	else if (c == ImageMetaData<T>::MASKS) {
		ThrowCc("This application does not support modifying masks.");
	}
	else if (c == ImageMetaData<T>::_OBJECT) {
		_checkString(key, value);
		String object = _getObject();
		ImageInfo info = _getInfo();
		String v = value.toString();
		info.setObjectName(v);
		_getImage()->setImageInfo(info);
		_object = v;
	}
	else if (c == ImageMetaData<T>::_OBSDATE || c == ImageMetaData<T>::_EPOCH) {
		ThrowIf(
			value.type() == casac::variant::STRING && value.toString().empty(),
			key + " value not specified"
		);
		Quantity qval = casaQuantity(value);
		if (! qval.isConform("s")) {
			ostringstream os;
			os << _LOCATEA << key
				<< " value must have units of time or be in a supported time format";
			throw AipsError(os.str());
			//ThrowCc( " value must have units of time or be in a supported time format");

		}
		MEpoch epoch(qval);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObsDate(epoch);
		csys.setObsInfo(info);
		_getImage()->setCoordinateInfo(csys);
		_obsdate = epoch;
	}
	else if (c == ImageMetaData<T>::_OBSERVER) {
		_checkString(key, value);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		String v = value.toString();
		info.setObserver(v);
		csys.setObsInfo(info);
		_getImage()->setCoordinateInfo(csys);
		_observer = v;
	}
	else if (c == ImageMetaData<T>::_PROJECTION) {
		_checkString(key, value);
		ThrowIf(
			! _getCoords().hasDirectionCoordinate(),
			"This image does not have a direction "
			"coordinate and so a direction projection cannot be added."
		);
		String v = value.toString();
		v.upcase();
		Projection::Type ptype = Projection::type(v);
		ThrowIf(
			ptype == Projection::N_PROJ,
			"Unknown projection specification " + v
		);
		CoordinateSystem csys = _getCoords();
		DirectionCoordinate dircoord = csys.directionCoordinate();
		Projection curProj = dircoord.projection();
		if (curProj.type() == ptype) {
			// nothing to do
			return True;
		}
		Vector<Double> curParms = curProj.parameters();
		Projection projection(ptype, curParms);
		dircoord.setProjection(projection);
		csys.replaceCoordinate(dircoord, csys.directionCoordinateNumber());
		_getImage()->setCoordinateInfo(csys);
		_projection = Projection::name(ptype);
	}
	else if (c == ImageMetaData<T>::_REFFREQTYPE) {
		_checkString(key, value);
		ThrowIf(
			! _getCoords().hasSpectralAxis(),
			"This image does not have a spectral coordinate"
			"and so a velocity reference frame cannot be added."
		);
		String v = value.toString();
		v.upcase();
		MFrequency::Types type;
		ThrowIf(
			! MFrequency::getType(type, v),
			"Unknown velocity reference frame specification " + v
		);
		CoordinateSystem csys = _getCoords();
		SpectralCoordinate spcoord = csys.spectralCoordinate();
		if (spcoord.frequencySystem(False) == type) {
			return True;
		}
		spcoord.setFrequencySystem(type);
		cout << "sp coord now is type " << MFrequency::showType(spcoord.frequencySystem(False)) << endl;
		csys.replaceCoordinate(spcoord, csys.spectralCoordinateNumber());
		_getImage()->setCoordinateInfo(csys);
		_reffreqtype = v;
	}
	else if (c == ImageMetaData<T>::_RESTFREQ) {
		ThrowIf(
			! _getCoords().hasSpectralAxis(),
			"This image does not have a spectral coordinate"
			"and so a velocity reference frame cannot be added."
		);
		Quantity rf = casaQuantity(value);
		ThrowIf(
			rf.getValue() <= 0,
			"Unable to set rest frequency to "
			+ value.toString()
		);
		ThrowIf(
			! rf.getUnit().empty() && ! rf.isConform("Hz"),
			"Unable to set rest frequency to "
			+ value.toString() + " because units do not conform to Hz"
		);
		CoordinateSystem csys = _getCoords();
		SpectralCoordinate sp = csys.spectralCoordinate();
		Double v = 0;
		if (
			value.type() == casac::variant::DOUBLE
			|| value.type() == casac::variant::INT
			|| value.type() == casac::variant::LONG
		) {
			v = value.toDouble();
		}
		else {
			v = rf.getValue(sp.worldAxisUnits()[0]);
		}
		sp.setRestFrequency(v);
		csys.replaceCoordinate(sp, csys.spectralCoordinateNumber());
		_getImage()->setCoordinateInfo(csys);
		_restFreq = rf;
	}
	else if (c == ImageMetaData<T>::_SHAPE) {
		ThrowCc(
			"The shape is intrinsic to the image and may "
			"not be modified."
		);
	}
	else if (c == ImageMetaData<T>::_TELESCOPE) {
		_checkString(key, value);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		String v = value.toString();
		info.setTelescope(v);
		csys.setObsInfo(info);
		_getImage()->setCoordinateInfo(csys);
		_telescope = v;
	}
	else if (
		c == ImageMetaData<T>::_BEAMMAJOR
		|| c == ImageMetaData<T>::_BEAMMINOR
		|| c == ImageMetaData<T>::_BEAMPA
		|| c == ImageMetaData<T>::_BMAJ
		|| c == ImageMetaData<T>::_BMIN
		|| c == ImageMetaData<T>::_BPA
	) {
		ImageInfo info = _getInfo();
		ThrowIf(
			info.hasMultipleBeams(),
			"This image has multiple beams. "
			"This application cannot modify beams in such an image."
		)
		Quantity v = casaQuantity(value);
		GaussianBeam beam;
		if (c == ImageMetaData<T>::_BEAMPA || c == ImageMetaData<T>::_BPA) {
			ThrowIf(
				! info.hasBeam(),
				"This image has no beam. This application annot add a beam position "
				"angle to an image with no beam. Add the major or minor axis and then "
				"modify the other and the position angle with put."
			);
			beam = info.getBeamSet()(0, 0);
			beam.setPA(v);
		}
		else if (info.hasBeam()) {
			beam = info.getBeamSet()(0, 0);
			if (
				c == ImageMetaData<T>::_BEAMMAJOR
				|| c == ImageMetaData<T>::_BMAJ
			) {
				beam.setMajorMinor(v, beam.getMinor());
			}
			else {
				beam.setMajorMinor(beam.getMajor(), v);
			}
		}
		else {
			beam = GaussianBeam(v, v, Quantity(0, "deg"));
		}
		info.setRestoringBeam(beam);
		_getImage()->setImageInfo(info);
		_beam = beam;
		this->_getLog() << LogIO::NORMAL << "Updated single beam " << beam
			<< " in image." << LogIO::POST;
	}
	else if (
		c == ImageMetaData<T>::_DATAMIN
		|| c == ImageMetaData<T>::_DATAMAX
		|| c == ImageMetaData<T>::_MINPIXPOS
        || c == ImageMetaData<T>::_MAXPIXPOS
        || c == ImageMetaData<T>::_MINPOS
        || c == ImageMetaData<T>::_MAXPOS
    ) {
		ThrowCc(
			key + " is is a statistic of the image and may "
			"not be added or modified."
		);
	}
	else {
		_setUserDefined(key, value);
	}
	_header.assign(Record());
	return True;
}

template<class T> void ImageMetaDataRW<T>::_setUserDefined(
	const String& key, const casac::variant& value
) {
	TableRecord info = _getImage()->miscInfo();
	switch(value.type()) {
	case casac::variant::BOOL:
		info.define(key, value.getBool());
		break;
	case casac::variant::BOOLVEC:
		info.define(key, Vector<Bool>(value.getBoolVec()));
		break;
	case casac::variant::COMPLEX:
		info.define(key, value.getComplex());
		break;
	case casac::variant::DOUBLE:
		info.define(key, value.getDouble());
		break;
	case casac::variant::DOUBLEVEC:
		info.define(key, Vector<Double>(value.getDoubleVec()));
		break;
	case casac::variant::INT:
		info.define(key, value.getInt());
		break;
	case casac::variant::INTVEC:
		info.define(key, Vector<Int>(value.getIntVec()));
		break;
	case casac::variant::LONG:
		info.define(key, value.getLong());
		break;
	case casac::variant::LONGVEC:
		info.define(key, Vector<Int64>(value.getLongVec()));
		break;
	case casac::variant::RECORD: {
		casac::record m = value.getRecord();
		Record *k = casa::toRecord(m);
		auto_ptr<Record> r(k);
		info.defineRecord(key, *r);
		break;
	}
	case casac::variant::STRING:
		info.define(key, value.getString());
		break;
	case casac::variant::STRINGVEC: {
		vector<std::string> v = value.getStringVec();
		Vector<String> x(v.size());
		Vector<String>::iterator iter = x.begin();
		Vector<String>::const_iterator end = x.end();
		vector<std::string>::const_iterator viter = v.begin();
		while (iter != end) {
			*iter = *viter;
			iter++;
			viter++;
		}
		info.define(key, x);
		break;
	}
	default:
		ThrowCc("Unhandled variant value type.");
		break;
	}
	_getImage()->setMiscInfo(info);
}


template<class T> void ImageMetaDataRW<T>::_setCoordinateValue(
	const String& key, const casac::variant& value
) {
	LogIO log = this->_getLog();
	String prefix = key.substr(0, 5);
	CoordinateSystem csys = _getCoords();
	uInt n = this->_getAxisNumber(key);
	if (prefix == ImageMetaData<T>::_CDELT) {
		Quantity qinc = casaQuantity(value);
		Vector<Double> increments = csys.increment();
		if (qinc.getFullUnit().empty()) {
			qinc.setUnit(_getAxisUnits()[n-1]);
		}
		increments[n-1] = qinc.getValue(_getAxisUnits()[n-1]);
		csys.setIncrement(increments);
		if (! _increment.empty() > 0) {
			_increment[n-1] = qinc;
		}
	}
	else if (prefix == ImageMetaData<T>::_CRPIX) {
		casac::variant::TYPE t = value.type();
		Bool stringIsDouble = False;
		if (t == casac::variant::STRING) {
			String::toDouble(stringIsDouble, value.toString());
		}
		if (
			stringIsDouble
			|| t == casac::variant::INT
			|| t == casac::variant::LONG
			|| t == casac::variant::DOUBLE
		) {
			Vector<Double> refpix = _getRefPixel();
			refpix[n-1] = value.toDouble();
			csys.setReferencePixel(refpix);
			if (! _refPixel.empty()) {
				_refPixel[n-1] = refpix[n-1];
			}
		}
		else {
			ThrowCc("For crpix, value must be numeric");
		}
	}
	else if (prefix == ImageMetaData<T>::_CRVAL) {
		ThrowIf(
			value.type() == casac::variant::STRING && value.toString().empty(),
			key + " value not specified"
		);
		Vector<Double> refval = csys.referenceValue();
		Quantity qval = casaQuantity(value);
		if (qval.getUnit().empty()) {
			qval.setUnit(_getAxisUnits()[n-1]);
		}
		refval[n-1] = qval.getValue(_getAxisUnits()[n-1]);
		csys.setReferenceValue(refval);
		if (! _refVal.empty()) {
			_refVal[n-1] = qval;
		}
	}
	else if (prefix == ImageMetaData<T>::_CTYPE) {
		_checkString(key, value);
		Vector<String> names = _getAxisNames();
		names[n-1] = value.toString();
		csys.setWorldAxisNames(names);
		if (! _axisNames.empty()) {
			_axisNames[n-1] = names[n-1];
		}
	}
	else if (prefix == ImageMetaData<T>::_CUNIT) {
		_checkString(key, value);
		String u = value.toString();
		// Test to see if CASA supports this string as a Unit
		Unit x = Unit(u);
		Vector<String> units = _getAxisUnits();
		units[n-1] = u;
		csys.setWorldAxisUnits(units, True);
		if (! _axisUnits.empty()) {
			_axisUnits[n-1] = units[n-1];
		}
	}
	_getImage()->setCoordinateInfo(csys);
}


template<class T> void ImageMetaDataRW<T>::_checkString(
	const String& key, const casac::variant& v
) const {
	ThrowIf(
		v.type() != casac::variant::STRING,
		key + " value must be a string"
	);
}


template<class T> Vector<String> ImageMetaDataRW<T>::_getAxisNames() const {
	if (_axisNames.size() == 0) {
		_axisNames = _getCoords().worldAxisNames();
	}
	return _axisNames;
}

template<class T> Vector<String> ImageMetaDataRW<T>::_getAxisUnits() const {
	if (_axisUnits.size() == 0) {
		_axisUnits = _getCoords().worldAxisUnits();
	}
	return _axisUnits;
}

template<class T> GaussianBeam ImageMetaDataRW<T>::_getBeam() const {
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

template<class T> String ImageMetaDataRW<T>::_getBrightnessUnit() const {
	if (_bunit.empty()) {
		_bunit = _getImage()->units().getName();
	}
	return _bunit;
}

template<class T> String ImageMetaDataRW<T>::_getEquinox() const {
	const CoordinateSystem& csys = _getCoords();
	if (_equinox.empty()) {
		if (csys.hasDirectionCoordinate()) {
			_equinox = MDirection::showType(
				csys.directionCoordinate().directionType()
			);
		}
	}
	return _equinox;
}

template<class T> String ImageMetaDataRW<T>::_getImType() const {
	if (_imtype.empty()) {
		_imtype = ImageInfo::imageType(_getInfo().imageType());
	}
	return _imtype;
}

template<class T> vector<Quantity> ImageMetaDataRW<T>::_getIncrements() const {
	if (_increment.size() == 0) {
		Vector<Double> incs = _getCoords().increment();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<incs.size(); i++) {
			_increment.push_back(Quantity(incs[i], units[i]));
		}
	}
	return _increment;
}

template<class T> String ImageMetaDataRW<T>::_getObject() const {
	if (_object.empty()) {
		_object = _getInfo().objectName();
	}
	return _object;
}

template<class T> Vector<String> ImageMetaDataRW<T>::_getMasks() const {
	if (_masks.empty()) {
		_masks = _getImage()->regionNames(RegionHandler::Masks);
	}
	return _masks;
}

template<class T> MEpoch ImageMetaDataRW<T>::_getObsDate() const {
	if (_obsdate.get("s") == 0) {
		_obsdate = _getCoords().obsInfo().obsDate();
	}
	return _obsdate;
}

template<class T> String ImageMetaDataRW<T>::_getObserver() const {
	if (_observer.empty()) {
		_observer = _getCoords().obsInfo().observer();
	}
	return _observer;
}

template<class T> String ImageMetaDataRW<T>::_getProjection() const {
	const CoordinateSystem& csys = _getCoords();
	if (_projection.empty() && csys.hasDirectionCoordinate()) {
		_projection = csys.directionCoordinate().projection().name();
	}
	return _projection;
}

template<class T> Vector<Double> ImageMetaDataRW<T>::_getRefPixel() const {
	if (_refPixel.size() == 0) {
		_refPixel = _getCoords().referencePixel();
	}
	return _refPixel;
}

template<class T> Vector<Quantity> ImageMetaDataRW<T>::_getRefValue() const {
	if (_refVal.size() == 0) {
		Vector<Double> vals = _getCoords().referenceValue();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<vals.size(); i++) {
			_refVal.push_back(Quantity(vals[i], units[i]));
		}
	}
	return _refVal;
}

template<class T> String ImageMetaDataRW<T>::_getRefFreqType() const {
	const CoordinateSystem& csys = _getCoords();
	cout << "*** reffreqtype in get before " + _reffreqtype << endl;

	if (_reffreqtype.empty() && csys.hasSpectralAxis()) {
		_reffreqtype = MFrequency::showType(csys.spectralCoordinate().frequencySystem(False));
	}
	cout << "*** reffreqtype in get " + _reffreqtype << endl;
	return _reffreqtype;
}

template<class T> Quantity ImageMetaDataRW<T>::_getRestFrequency() const {
	const CoordinateSystem& csys = _getCoords();
	if (_restFreq.getValue() == 0 && csys.hasSpectralAxis()) {
		_restFreq = Quantity(
			csys.spectralCoordinate().restFrequency(),
			csys.spectralCoordinate().worldAxisUnits()[0]
		);
	}
	return _restFreq;
}

template<class T> String ImageMetaDataRW<T>::_getTelescope() const {
	if (_telescope.empty()) {
		_telescope = _getCoords().obsInfo().telescope();
	}
	return _telescope;
}


} //# NAMESPACE CASA - END

