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

#include <casa/Containers/ValueHolder.h>
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <tables/Tables/TableRecord.h>

#include <casa/aips.h>

#include <tr1/memory>

#define _LOCATEA "ImageMetaDataRW" << __func__ << " "
#define _ORIGINA LogOrigin("ImageMetaDataRW", __func__)

namespace casa {

ImageMetaDataRW::ImageMetaDataRW(
	std::tr1::shared_ptr<ImageInterface<Float> > image
) : ImageMetaDataBase(), _floatImage(image), _complexImage() {}

ImageMetaDataRW::ImageMetaDataRW(
	std::tr1::shared_ptr<ImageInterface<Complex> > image
) : ImageMetaDataBase(), _floatImage(), _complexImage(image) {}

Record ImageMetaDataRW::toRecord(Bool verbose) const {
	if (_header.empty()) {
		_header = _makeHeader();
	}
	if (verbose) {
		_toLog(_header);
	}
	return _header;
}

Bool ImageMetaDataRW::_isWritable() const {
	return (_floatImage && _floatImage->isWritable())
		|| (_complexImage && _complexImage->isWritable());
}

Bool ImageMetaDataRW::_setUnit(const String& unit) {
	if (_floatImage) {
		return _floatImage->setUnits(unit);
	}
	else {
		return _complexImage->setUnits(unit);
	}
}

const TableRecord ImageMetaDataRW::_miscInfo() const {
	if (_floatImage) {
		return _floatImage->miscInfo();
	}
	else {
		return _complexImage->miscInfo();
	}
}

void ImageMetaDataRW::_setMiscInfo(const TableRecord& rec) {
	Bool res = _floatImage
		? _floatImage->setMiscInfo(rec)
		:  _complexImage->setMiscInfo(rec);
	ThrowIf(
		! res,
		"Error setting misc info"
	);
}


Bool ImageMetaDataRW::add(const String& key, const ValueHolder& value) {
	ThrowIf(
		! _isWritable(),
		"This image is not writable; metadata may not be added to it"
	);
	String c = key;
	c.downcase();
	ThrowIf(
		c.startsWith(ImageMetaDataBase::_CDELT)
		|| c.startsWith(ImageMetaDataBase::_CRPIX)
		|| c.startsWith(ImageMetaDataBase::_CRVAL)
		|| c.startsWith(ImageMetaDataBase::_CTYPE)
		|| c.startsWith(ImageMetaDataBase::_CUNIT),
		key + " pertains to a "
		+ "coordinate system axis attribute. It may be "
		+ "modified if it exists, but it may not be added."
	);
	ThrowIf(
		c == ImageMetaDataBase::_EQUINOX,
		"The direction reference frame ("
		+ key + "=" + _getEquinox()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaDataBase::MASKS,
		"This application does not support adding masks."
	);
	ThrowIf(
		c == ImageMetaDataBase::_OBSDATE || c == ImageMetaDataBase::_EPOCH,
		"The epoch (" + key + "=" + _getEpochString()
		+ ") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaDataBase::_PROJECTION,
		"The projection ("
		+ key + "=" + _getProjection()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaDataBase::_REFFREQTYPE,
		"The velocity reference frame ("
		+ key + "=" + _getProjection()
		+") already exists. It may be modified but not added."
	);
	ThrowIf(
		c == ImageMetaDataBase::_SHAPE,
		"The shape is intrinsic to the image and may "
		"not be added."
	);
	ThrowIf(
		c == ImageMetaDataBase::_BEAMPA || c == ImageMetaDataBase::_BPA,
		"Cannot add a beam position "
		"angle. Add the major or minor axis and then "
		"modify the other and the position angle."
	);
	ThrowIf(
		c == ImageMetaDataBase::_DATAMIN
		|| c == ImageMetaDataBase::_DATAMAX
		|| c == ImageMetaDataBase::_MINPIXPOS
		|| c == ImageMetaDataBase::_MAXPIXPOS
		|| c == ImageMetaDataBase::_MINPOS
		|| c == ImageMetaDataBase::_MAXPOS,
		key + " is is a statistic of the image and may "
		"not be added."
	);
	if (c == ImageMetaDataBase::_BUNIT) {
		ThrowIf(
			! _getBrightnessUnit().empty(),
			key + " is already present and has value "
			+ _getBrightnessUnit() + ". It may be modified but not added."
		);
		String v = value.asString();
		ThrowIf(
			! _setUnit(v),
			"Unable to set " + key
		);
		_bunit = v;
	}
	else if (c == ImageMetaDataBase::_IMTYPE) {
		String imtype = _getImType();
		ThrowIf(
			! imtype.empty(),
			"The image type ("
			+ key + "=" + ImageInfo::imageType(imtype)
		    + ") already exists. It may be modified but be added."
		);
		set(c, value);
	}
	else if (c == ImageMetaDataBase::_OBJECT) {
		String object = _getObject();
		ThrowIf(
			! object.empty(),
			key + " is already present and has value "
			+ object + ". It may be modified but not added."
		);
		set(c, value);
	}
	else if (c == ImageMetaDataBase::_OBSERVER) {
		String observer = _getObserver();
		ThrowIf(
			! observer.empty(),
			key + " is already present and has value "
			+ observer + ".  It may be modified but not added."
		);
		set(c, value);
	}
	else if (c == ImageMetaDataBase::_RESTFREQ) {
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
	else if (c == ImageMetaDataBase::_TELESCOPE) {
		String telescope = _getTelescope();
		ThrowIf(
			! telescope.empty(),
			key + " is already present and has value "
			+ telescope + ". It may be modified but not added."
		);
		set(c, value);
	}
	else if (
		c == ImageMetaDataBase::_BEAMMAJOR
		|| c == ImageMetaDataBase::_BEAMMINOR
		|| c == ImageMetaDataBase::_BMAJ
		|| c == ImageMetaDataBase::_BMIN
	) {
		ImageInfo info = _getInfo();
		ThrowIf(
			info.hasBeam(),
			"This image already has a beam(s). Cannot add one."
		);
		set(c, value);
	}
	else if (
		_miscInfo().isDefined(key)
		|| _miscInfo().isDefined(c)
	) {
		ThrowCc(
			"Keyword " + key
			+ " already exists so cannot be added."
		);
	}
	else {
		_setUserDefined(key, value);
	}
	// clear cached header
	_header.assign(Record());
	return True;
}

Bool ImageMetaDataRW::remove(const String& key) {
	ThrowIf(
		! _isWritable(),
		"This image is not writable; metadata may not be removed from it"
	);
	String c = key;
	LogIO log = _getLog();
	log << _ORIGINA;
	c.downcase();
	ThrowIf(
		c.startsWith(ImageMetaDataBase::_CDELT)
		|| c.startsWith(ImageMetaDataBase::_CRPIX)
		|| c.startsWith(ImageMetaDataBase::_CRVAL)
		|| c.startsWith(ImageMetaDataBase::_CTYPE)
		|| c.startsWith(ImageMetaDataBase::_CUNIT),
		key + " pertains to a "
		"coordinate system axis attribute. It may be "
		"modified, but it may not be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_EQUINOX,
		"Although the direction reference frame ("
		+ key + ") may be modified, it may not be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_IMTYPE,
		"Although the image type ("
		+ key + ") may be modified, it may not be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::MASKS,
		"Logic Error: removeMask() should be called instead"
	);
	ThrowIf(
		c == ImageMetaDataBase::_OBSDATE || c == ImageMetaDataBase::_EPOCH,
		"Although the epoch (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_PROJECTION,
		"Although the projection (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_REFFREQTYPE,
		"Although the velocity reference frame (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_RESTFREQ,
		"Although the rest frequency (" + key
		+ ") may be modified, it cannot be removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_SHAPE,
		"The shape is intrinsic to the image and may "
		"not be modified nor removed."
	);
	ThrowIf(
		c == ImageMetaDataBase::_DATAMIN
		|| c == ImageMetaDataBase::_DATAMAX
		|| c == ImageMetaDataBase::_MINPIXPOS
		|| c == ImageMetaDataBase::_MAXPIXPOS
		|| c == ImageMetaDataBase::_MINPOS
		|| c == ImageMetaDataBase::_MAXPOS,
		key + " is is a statistic of the image and may "
		+ "not be modified nor removed by this application."
	);
	if (c == ImageMetaDataBase::_BUNIT) {
		ThrowIf(
			! _setUnit(""),
			"Unable to clear " + key
		);
		_bunit = "";
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
	}
	else if (c == ImageMetaDataBase::_OBJECT) {
		ImageInfo info = _getInfo();
		info.setObjectName("");
		_setImageInfo(info);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_object = "";
	}
	else if (c == ImageMetaDataBase::_OBSERVER) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObserver("");
		csys.setObsInfo(info);
		_setCsys(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_observer = "";
	}
	else if (c == ImageMetaDataBase::_TELESCOPE) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setTelescope("");
		csys.setObsInfo(info);
		_setCsys(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		_telescope = "";
	}
	else if (
		c == ImageMetaDataBase::_BEAMMAJOR
		|| c == ImageMetaDataBase::_BEAMMINOR
		|| c == ImageMetaDataBase::_BEAMPA
		|| c == ImageMetaDataBase::_BMAJ
		|| c == ImageMetaDataBase::_BMIN
		|| c == ImageMetaDataBase::_BPA
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
			_setImageInfo(info);
			_beam = GaussianBeam::NULL_BEAM;
		}
		else {
			log << LogIO::WARN << "This image has no beam(s) to remove." << LogIO::POST;
			return False;
		}
	}
	else if (_miscInfo().isDefined(key)) {
		TableRecord info = _miscInfo();
		info.removeField(key);
		_setMiscInfo(info);
		log << LogIO::NORMAL << "Removed user-defined keyword " << key << LogIO::POST;
	}
	else if (_miscInfo().isDefined(c)) {
		TableRecord info = _miscInfo();
		info.removeField(c);
		_setMiscInfo(info);
		log << LogIO::NORMAL << "Removed user-defined keyword " << c << LogIO::POST;
	}
	else {
		ThrowCc("Unknown keyword " + c);
	}
	_header.assign(Record());
	return True;
}

Bool ImageMetaDataRW::_hasRegion(const String& maskName) const {
	Bool res = _floatImage
		? _floatImage->hasRegion(maskName, RegionHandler::Masks)
		: _complexImage->hasRegion(maskName, RegionHandler::Masks);
	return res;
}

Bool ImageMetaDataRW::removeMask(const String& maskName) {
	ThrowIf(
		! _isWritable(),
		"This image is not writable; a mask(s) may not be removed from it"
	);
	LogIO log = _getLog();
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
				removeMask(*iter);
			}
			_masks.resize(0);
			return _getMasks().size() == 0;
		}
	}
	else {
		ThrowIf(
			! _hasRegion(maskName),
			"No mask named " + maskName + " found"
		);
		if (_floatImage) {
			_floatImage->removeRegion(maskName, RegionHandler::Masks);
		}
		else {
			_complexImage->removeRegion(maskName, RegionHandler::Masks);
		}
		ThrowIf(
			_hasRegion(maskName),
			"Unable to remove mask " + maskName
		);
		_masks.resize(0);
		log << LogIO::NORMAL << "Removed mask named " << maskName << endl;
		_header.assign(Record());
		return True;
	}
}

Bool ImageMetaDataRW::_setCsys(const CoordinateSystem& csys) {
	if (_floatImage) {
		return _floatImage->setCoordinateInfo(csys);
	}
	else {
		return _complexImage->setCoordinateInfo(csys);
	}
}

Bool ImageMetaDataRW::_setImageInfo(const ImageInfo& info) {
	if (_floatImage) {
		return _floatImage->setImageInfo(info);
	}
	else {
		return _complexImage->setImageInfo(info);
	}
}

Quantity ImageMetaDataRW::_getQuantity(const ValueHolder& v) {
	QuantumHolder qh;
	String error;
	DataType type = v.dataType();
	if (type == TpRecord) {
		ThrowIf(
			! qh.fromRecord(error, v.asRecord()),
			"Error converting to Quantity. " + error
		);
		return qh.asQuantity();
	}
	else if (type == TpString) {
		Quantity q;
		ThrowIf(
			! readQuantity(q, v.asString()),
			"Error converting " + v.asString() + " to Quantity"
		);
		return q;
	}
	else {
		ostringstream os;
		os << "Input ValueHolder is of type "
			<< v.dataType() << ", but it must be either "
			<< " a Record or a String";
		ThrowCc(os.str());
	}
}

Bool ImageMetaDataRW::set(
	const String& key, const ValueHolder& value
) {
	ThrowIf(
		! _isWritable(),
		"This image is not writable; metadata may not be modified in it"
	);
	String c = key;
	c.downcase();
	if (c == ImageMetaDataBase::_BUNIT) {
		String v = _getString(key, value);
		if (_setUnit(v)) {
			_bunit = v;
		}
		else {
			ThrowCc("Unable to set " + key);
		}
	}
	else if (
		c.startsWith(ImageMetaDataBase::_CDELT)
		|| c.startsWith(ImageMetaDataBase::_CRPIX)
		|| c.startsWith(ImageMetaDataBase::_CRVAL)
		|| c.startsWith(ImageMetaDataBase::_CTYPE)
		|| c.startsWith(ImageMetaDataBase::_CUNIT)
	) {
		_setCoordinateValue(c, value);
	}
	else if (c == ImageMetaDataBase::_EQUINOX) {
		ThrowIf(
			! _getCoords().hasDirectionCoordinate(),
			"This image does not have a direction "
			"coordinate and so a direction projection cannot be added."
		);
		String v = _getString(key, value);
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
		_setCsys(csys);
		_equinox = v;
	}
	else if (c == ImageMetaDataBase::_IMTYPE) {
		String v = _getString(key, value);
		String imtype = _getImType();
		ImageInfo info = _floatImage
			? _floatImage->imageInfo()
			: _complexImage->imageInfo();
		info.setImageType(ImageInfo::imageType(v));
		_setImageInfo(info);
		String newType = ImageInfo::imageType(info.imageType());
		_imtype = newType;
	}
	else if (c == ImageMetaDataBase::MASKS) {
		ThrowCc("This application does not support modifying masks.");
	}
	else if (c == ImageMetaDataBase::_OBJECT) {
		String v = _getString(key, value);
		String object = _getObject();
		ImageInfo info = _getInfo();
		info.setObjectName(v);
		_setImageInfo(info);
		_object = v;
	}
	else if (c == ImageMetaDataBase::_OBSDATE || c == ImageMetaDataBase::_EPOCH) {
		ThrowIf(
			value.dataType() == TpString
			&& value.asString().empty(),
			key + " value not specified"
		);
		Quantity qval = _getQuantity(value);
		ThrowIf(
			! qval.isConform("s"),
			key + " value must have units of time or be in a supported time format"
		);
		MEpoch epoch(qval);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObsDate(epoch);
		csys.setObsInfo(info);
		_setCsys(csys);
		_obsdate = epoch;
	}
	else if (c == ImageMetaDataBase::_OBSERVER) {
		String v = _getString(key, value);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObserver(v);
		csys.setObsInfo(info);
		_setCsys(csys);
		_observer = v;
	}
	else if (c == ImageMetaDataBase::_PROJECTION) {
		String v = _getString(key, value);
		ThrowIf(
			! _getCoords().hasDirectionCoordinate(),
			"This image does not have a direction "
			"coordinate and so a direction projection cannot be added."
		);
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
		_setCsys(csys);
		_projection = _getProjection();
	}
	else if (c == ImageMetaDataBase::_REFFREQTYPE) {
		String v = _getString(key, value);
		ThrowIf(
			! _getCoords().hasSpectralAxis(),
			"This image does not have a spectral coordinate"
			"and so a velocity reference frame cannot be added."
		);
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
		csys.replaceCoordinate(spcoord, csys.spectralCoordinateNumber());
		_setCsys(csys);
		_reffreqtype = v;
	}
	else if (c == ImageMetaDataBase::_RESTFREQ) {
		ThrowIf(
			! _getCoords().hasSpectralAxis(),
			"This image does not have a spectral coordinate"
			"and so a velocity reference frame cannot be added."
		);
		Quantity rf = _getQuantity(value);
		ThrowIf(
			rf.getValue() <= 0,
			"Unable to set rest frequency to "
			+ value.asString()
		);
		ThrowIf(
			! rf.getUnit().empty() && ! rf.isConform("Hz"),
			"Unable to set rest frequency to "
			+ value.asString() + " because units do not conform to Hz"
		);
		CoordinateSystem csys = _getCoords();
		SpectralCoordinate sp = csys.spectralCoordinate();
		DataType type = value.dataType();
		Double v = (
			type == TpShort || type == TpUShort
			|| type == TpInt || type == TpUInt
			|| type == TpFloat || type == TpDouble
		)
			? value.asDouble()
			: rf.getValue(sp.worldAxisUnits()[0]);
		sp.setRestFrequency(v);
		csys.replaceCoordinate(sp, csys.spectralCoordinateNumber());
		_setCsys(csys);
		_restFreq = rf;
	}
	else if (c == ImageMetaDataBase::_SHAPE) {
		ThrowCc(
			"The shape is intrinsic to the image and may "
			"not be modified."
		);
	}
	else if (c == ImageMetaDataBase::_TELESCOPE) {
		String v = _getString(key, value);
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setTelescope(v);
		csys.setObsInfo(info);
		_setCsys(csys);
		_telescope = v;
	}
	else if (
		c == ImageMetaDataBase::_BEAMMAJOR
		|| c == ImageMetaDataBase::_BEAMMINOR
		|| c == ImageMetaDataBase::_BEAMPA
		|| c == ImageMetaDataBase::_BMAJ
		|| c == ImageMetaDataBase::_BMIN
		|| c == ImageMetaDataBase::_BPA
	) {
		ImageInfo info = _getInfo();
		ThrowIf(
			info.hasMultipleBeams(),
			"This image has multiple beams. "
			"This application cannot modify beams in such an image."
		)
		Quantity v = _getQuantity(value);
		GaussianBeam beam;
		if (c == ImageMetaDataBase::_BEAMPA || c == ImageMetaDataBase::_BPA) {
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
				c == ImageMetaDataBase::_BEAMMAJOR
				|| c == ImageMetaDataBase::_BMAJ
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
		_setImageInfo(info);
		_beam = beam;
		_getLog() << LogIO::NORMAL << "Updated single beam " << beam
			<< " in image." << LogIO::POST;
	}
	else if (
		c == ImageMetaDataBase::_DATAMIN
		|| c == ImageMetaDataBase::_DATAMAX
		|| c == ImageMetaDataBase::_MINPIXPOS
        || c == ImageMetaDataBase::_MAXPIXPOS
        || c == ImageMetaDataBase::_MINPOS
        || c == ImageMetaDataBase::_MAXPOS
    ) {
		ThrowCc(
			key + " is is a statistic of the image and may "
			"not be added or modified."
		);
	}
	else {
		_setUserDefined(key, value);
	}
	// clear the cached header values
	_header.assign(Record());
	return True;
}

void ImageMetaDataRW::_setUserDefined(
	const String& key, const ValueHolder& value
) {
	TableRecord info = _miscInfo();
	DataType type = value.dataType();
	switch(type) {
	case TpBool:
		info.define(key, value.asBool());
		break;
	case TpArrayBool:
		info.define(key, value.asArrayBool());
		break;
	case TpComplex:
		info.define(key, value.asComplex());
		break;
	case TpDouble:
		info.define(key, value.asDouble());
		break;
	case TpArrayDouble:
		info.define(key, value.asArrayDouble());
		break;
	case TpInt:
		info.define(key, value.asInt());
		break;
	case TpArrayInt:
		info.define(key, value.asArrayInt());
		break;
	case TpInt64:
		info.define(key, value.asInt64());
		break;
	case TpArrayInt64:
		info.define(key, value.asArrayInt64());
		break;
	case TpRecord: {
		info.defineRecord(key, value.asRecord());
		break;
	}
	case TpString:
		info.define(key, value.asString());
		break;
	case TpArrayString: {
		info.define(key, value.asArrayString());
		break;
	}
	default:
		ostringstream os;
		os << "Unhandled value type " << type
			<< " for " << key;
		ThrowCc(os.str());
		break;
	}
	_setMiscInfo(info);
}

void ImageMetaDataRW::_setCoordinateValue(
	const String& key, const ValueHolder& value
) {
	LogIO log = _getLog();
	String prefix = key.substr(0, 5);
	CoordinateSystem csys = _getCoords();
	uInt n = _getAxisNumber(key);
	Bool isStokes = csys.hasPolarizationCoordinate()
		&& (Int)n == csys.polarizationAxisNumber(False) + 1;
	if (prefix == ImageMetaDataBase::_CDELT) {
		ThrowIf(
			isStokes,
			"A polarization axis cannot have an increment"
		);
		Quantity qinc = _getQuantity(value);
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
	else if (prefix == ImageMetaDataBase::_CRPIX) {
		ThrowIf(
			isStokes,
			"A polarization axis cannot have a reference pixel"
		);
		DataType t = value.dataType();
		Bool stringIsDouble = False;
		Double x = 0;
		if (t == TpString) {
			x = String::toDouble(stringIsDouble, value.asString());
			ThrowIf (
				! stringIsDouble,
				value.asString()
				+ " is not a String representation of a real numeric type"
			);
		}
		else if (
			t == TpInt || t == TpInt64
			|| t == TpDouble || t == TpFloat
		) {
			x = value.asDouble();
		}
		else {
			ostringstream os;
			os << t;
			ThrowCc("For crpix, value must be numeric, not " + os.str());
		}
		Vector<Double> refpix = _getRefPixel();
		refpix[n-1] = x;
		csys.setReferencePixel(refpix);
		if (! _refPixel.empty()) {
			_refPixel[n-1] = refpix[n-1];
		}
	}
	else if (prefix == ImageMetaDataBase::_CRVAL) {
		DataType dType = value.dataType();
		ThrowIf(
			dType == TpString
			&& value.asString().empty(),
			key + " value not specified"
		);
		Vector<Double> refval = csys.referenceValue();
		if (isStokes) {
			ThrowIf(
				dType != TpString && dType != TpArrayString,
				"Data type to put must be either a string array or string"
			);
			uInt nStokes = csys.stokesCoordinate().stokes().size();
			Vector<String> stokesTypes;
			if (dType == TpString) {
				ThrowIf(
					nStokes > 1,
					"There are " + String::toString(nStokes)
					+ " polarization values, "
					"so a string array of that length is needed"
				);
				stokesTypes = Vector<String>(1, value.asString());
			}
			else if (dType == TpArrayString) {
				ThrowIf(
					value.asArrayString().size() != nStokes,
					"There are " + String::toString(nStokes)
					+ " polarization values, "
					"so a string array of that length is needed"
				);
				stokesTypes = value.asArrayString();
			}
			Vector<Int> stokesNumbers(nStokes);
			Vector<String>::const_iterator begin = stokesTypes.begin();
			Vector<String>::const_iterator typeIter = stokesTypes.begin();
			Vector<String>::const_iterator end = stokesTypes.end();
			Vector<Int>::iterator numberIter = stokesNumbers.begin();
			while (typeIter != end) {
				*numberIter = (Int)Stokes::type(*typeIter);
				typeIter++;
				numberIter++;
			}
			StokesCoordinate coord(stokesNumbers);
			ThrowIf(
				! csys.replaceCoordinate(coord, csys.polarizationCoordinateNumber()),
				"Failed to replace stokes coordinate"
			);
			_stokes = stokesTypes;
		}
		else {
			Quantity qval = _getQuantity(value);
			if (qval.getUnit().empty()) {
				qval.setUnit(_getAxisUnits()[n-1]);
			}
			refval[n-1] = qval.getValue(_getAxisUnits()[n-1]);
			csys.setReferenceValue(refval);
			if (! _refVal.empty()) {
				_refVal[n-1] = qval;
			}
		}
	}
	else if (prefix == ImageMetaDataBase::_CTYPE) {
		Vector<String> names = _getAxisNames();
		names[n-1] = _getString(key, value);
		csys.setWorldAxisNames(names);
		if (! _axisNames.empty()) {
			_axisNames[n-1] = names[n-1];
		}
	}
	else if (prefix == ImageMetaDataBase::_CUNIT) {
		ThrowIf(
			isStokes,
			"A polarization axis cannot have a unit"
		);
		String u = _getString(key, value);
		// Test to see if CASA supports this string as a Unit
		Unit x = Unit(u);
		Vector<String> units = _getAxisUnits();
		units[n-1] = u;
		csys.setWorldAxisUnits(units, True);
		if (! _axisUnits.empty()) {
			_axisUnits[n-1] = units[n-1];
		}
	}
	_setCsys(csys);
	// clear stats because modifying the coordinate system may invalidate
	// min and max value world coordinates
	_stats = Record();
}


String ImageMetaDataRW::_getString(
	const String& key, const ValueHolder& v
) const {
	ThrowIf(
		v.dataType() != TpString,
		key + "value must be a string"
	);
	return v.asString();
}

Vector<String> ImageMetaDataRW::_getAxisNames() const {
	if (_axisNames.size() == 0) {
		_axisNames = _getCoords().worldAxisNames();
	}
	return _axisNames;
}

Vector<String> ImageMetaDataRW::_getAxisUnits() const {
	if (_axisUnits.size() == 0) {
		_axisUnits = _getCoords().worldAxisUnits();
	}
	return _axisUnits;
}

GaussianBeam ImageMetaDataRW::_getBeam() const {
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

String ImageMetaDataRW::_getBrightnessUnit() const {
	if (_bunit.empty()) {
		_bunit = _floatImage
			? _floatImage->units().getName()
			: _complexImage->units().getName();
	}
	return _bunit;
}

String ImageMetaDataRW::_getEquinox() const {
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

String ImageMetaDataRW::_getImType() const {
	if (_imtype.empty()) {
		_imtype = ImageInfo::imageType(_getInfo().imageType());
	}
	return _imtype;
}

vector<Quantity> ImageMetaDataRW::_getIncrements() const {
	if (_increment.size() == 0) {
		Vector<Double> incs = _getCoords().increment();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<incs.size(); i++) {
			_increment.push_back(Quantity(incs[i], units[i]));
		}
	}
	return _increment;
}

String ImageMetaDataRW::_getObject() const {
	if (_object.empty()) {
		_object = _getInfo().objectName();
	}
	return _object;
}

const ImageInfo& ImageMetaDataRW::_getInfo() const {
	const ImageInfo& info = _floatImage
		? _floatImage->imageInfo()
		: _complexImage->imageInfo();
	return info;
}

const CoordinateSystem& ImageMetaDataRW::_getCoords() const {
	const CoordinateSystem& csys = _floatImage
		? _floatImage->coordinates()
		: _complexImage->coordinates();
	return csys;
}

Vector<String> ImageMetaDataRW::_getMasks() const {
	if (_masks.empty()) {
		_masks = _floatImage
			? _floatImage->regionNames(RegionHandler::Masks)
			: _complexImage->regionNames(RegionHandler::Masks);
	}
	return _masks;
}

MEpoch ImageMetaDataRW::_getObsDate() const {
	if (_obsdate.get("s").getValue() == 0) {
		_obsdate = _getCoords().obsInfo().obsDate();
	}
	return _obsdate;
}

String ImageMetaDataRW::_getObserver() const {
	if (_observer.empty()) {
		_observer = _getCoords().obsInfo().observer();
	}
	return _observer;
}

String ImageMetaDataRW::_getProjection() const {
	if (_projection.empty()) {
		_projection = ImageMetaDataBase::_getProjection();
	}
	return _projection;
}

Vector<Double> ImageMetaDataRW::_getRefPixel() const {
	if (_refPixel.size() == 0) {
		_refPixel = _getCoords().referencePixel();
	}
	return _refPixel;
}

Vector<String> ImageMetaDataRW::_getStokes() const {
	const CoordinateSystem csys = _getCoords();
	ThrowIf(
		! csys.hasPolarizationCoordinate(),
		"Logic Error: coordinate system does not have a polarization coordinate"
	);
	if (_stokes.empty()) {
		_stokes = csys.stokesCoordinate().stokesStrings();
	}
	return _stokes;
}

Vector<Quantity> ImageMetaDataRW::_getRefValue() const {
	if (_refVal.size() == 0) {
		Vector<Double> vals = _getCoords().referenceValue();
		Vector<String> units = _getAxisUnits();
		for (uInt i=0; i<vals.size(); i++) {
			_refVal.push_back(Quantity(vals[i], units[i]));
		}
	}
	return _refVal;
}

String ImageMetaDataRW::_getRefFreqType() const {
	const CoordinateSystem& csys = _getCoords();
	if (_reffreqtype.empty() && csys.hasSpectralAxis()) {
		_reffreqtype = MFrequency::showType(csys.spectralCoordinate().frequencySystem(False));
	}
	return _reffreqtype;
}

Quantity ImageMetaDataRW::_getRestFrequency() const {
	const CoordinateSystem& csys = _getCoords();
	ThrowIf(
		! csys.hasSpectralAxis(),
		"Image has no spectral axis so there is no rest frequency"
	);
	if (_restFreq.getValue() == 0) {
		_restFreq = Quantity(
			csys.spectralCoordinate().restFrequency(),
			csys.spectralCoordinate().worldAxisUnits()[0]
		);
	}
	return _restFreq;
}

String ImageMetaDataRW::_getTelescope() const {
	if (_telescope.empty()) {
		_telescope = _getCoords().obsInfo().telescope();
	}
	return _telescope;
}

Record ImageMetaDataRW::_getStatistics() const {
	if (_stats.nfields() == 0 && ! _complexImage) {
		_stats = _calcStats();
	}
	return _stats;
}

} //# NAMESPACE CASA - END

