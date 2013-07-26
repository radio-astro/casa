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

#include <casa/aips.h>

#include <casa/Quanta/QuantumHolder.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageStatistics.h>
#include <measures/Measures/MeasureHolder.h>

#include <stdcasa/StdCasa/CasacSupport.h>
#include <stdcasa/variant.h>

#include <iostream>
#include <iomanip>

#include <boost/regex.hpp>

#define _ORIGINA LogOrigin("ImageMetaDataRW<T>", __FUNCTION__, WHERE)

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

template<class T> Bool ImageMetaDataRW<T>::remove(const String& key) {
	String c = key;
	LogIO log = this->_getLog();
	log << _ORIGINA;
	c.downcase();
	if (c == ImageMetaData<T>::_BUNIT) {
		if (_getImage()->setUnits(Unit(""))) {
			_bunit = "";
			log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
		}
		else {
			log << LogIO::WARN << "Unable to clear BUNIT" << LogIO::POST;
			return False;
		}
	}
	else if (
		c.startsWith(ImageMetaData<T>::_CDELT)
		|| c.startsWith(ImageMetaData<T>::_CRPIX)
		|| c.startsWith(ImageMetaData<T>::_CRVAL)
		||  c.startsWith(ImageMetaData<T>::_CTYPE)
		||  c.startsWith(ImageMetaData<T>::_CUNIT)
	) {
		log << LogIO::WARN << key << " pertains to a "
			<< "coordinate system axis attribute. It may be "
			<< "modified, but it may not be removed."
			<< LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_EQUINOX) {
		log << LogIO::WARN << "Although the direction reference frame ("
			<< key << ") may be modified, it may not be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_IMTYPE) {
		log << LogIO::WARN << "Although the image type ("
			<< key << ") may be modified, it may not be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::MASKS) {
		log << "Logic Error: removeMask() should be called instead" << LogIO::EXCEPTION;
	}
	else if (c == ImageMetaData<T>::_OBJECT) {
		ImageInfo info = _getInfo();
		info.setObjectName("");
		_image->setImageInfo(info);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
	}
	else if (c == ImageMetaData<T>::_OBSDATE || c == ImageMetaData<T>::_EPOCH) {
		log << LogIO::WARN << "Although the epoch (" << key
			<< ") may be modified, it cannot be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_OBSERVER) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setObserver("");
		csys.setObsInfo(info);
		_image->setCoordinateInfo(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
	}
	else if (c == ImageMetaData<T>::_PROJECTION) {
		log << LogIO::WARN << "Although the projection (" << key
			<< ") may be modified, it cannot be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_REFFREQTYPE) {
		log << LogIO::WARN << "Although the velocity reference frame (" << key
			<< ") may be modified, it cannot be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_RESTFREQ) {
		log << LogIO::WARN << "Although the rest frequency (" << key
			<< ") may be modified, it cannot be removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_SHAPE) {
		log << LogIO::WARN << "The shape is intrinsic to the image and may "
			"not be modified nor removed." << LogIO::POST;
		return False;
	}
	else if (c == ImageMetaData<T>::_TELESCOPE) {
		CoordinateSystem csys = _getCoords();
		ObsInfo info = csys.obsInfo();
		info.setTelescope("");
		csys.setObsInfo(info);
		_image->setCoordinateInfo(csys);
		log << LogIO::NORMAL << "Setting " << key << " to empty string" << LogIO::POST;
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
		}
		else {
			log << LogIO::WARN << "This image has no beam(s) to remove." << LogIO::POST;
			return False;
		}
	}
	else if (
		c == ImageMetaData<T>::_DATAMIN
		|| c == ImageMetaData<T>::_DATAMAX
		|| c == ImageMetaData<T>::_MINPIXPOS
        || c == ImageMetaData<T>::_MAXPIXPOS
        || c == ImageMetaData<T>::_MINPOS
        || c == ImageMetaData<T>::_MAXPOS
    ) {
		log << LogIO::WARN << key << " is is a statistic of the image and may "
			<< "not be modified nor removed by this application." << LogIO::POST;
		return False;
	}
	else if (_image->miscInfo().isDefined(key)) {
		TableRecord info = _image->miscInfo();
		info.removeField(key);
		_image->setMiscInfo(info);
		log << LogIO::POST << "Removed user-defined keyword " << key << endl;
	}
	else if (_image->miscInfo().isDefined(c)) {
		TableRecord info = _image->miscInfo();
		info.removeField(c);
		_image->setMiscInfo(info);
		log << LogIO::POST << "Removed user-defined keyword " << c << endl;
	}
	else {
		log << "Unknown keyword " << c << LogIO::EXCEPTION;
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
		if (image->hasRegion(maskName, RegionHandler::Masks)) {
			image->removeRegion(maskName, RegionHandler::Masks);
			if (image->hasRegion(maskName, RegionHandler::Masks)) {
				log << LogIO::WARN << "Unable to remove mask "
					<< maskName << LogIO::POST;
				return False;
			}
			else {
				_masks.resize(0);
				log << LogIO::NORMAL << "Removed mask named " << maskName << endl;
				_header.assign(Record());
				return True;
			}
		}
		else {
			log << LogIO::WARN << "No mask named " << maskName << " found" << LogIO::POST;
			return False;
		}
	}
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
	if (_reffreqtype.empty() && csys.hasSpectralAxis()) {
		_reffreqtype = MFrequency::showType(csys.spectralCoordinate().type());
	}
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

