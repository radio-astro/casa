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

namespace casa {

template<class T> ImageMetaData<T>::ImageMetaData(
	const ImageInterface<T> *const &image
) : ImageMetaDataBase<T>(), _image(image), _info(image->imageInfo()),
	_csys(image->coordinates()) {}

template<class T> Record ImageMetaData<T>::toRecord(Bool verbose) const {
	if (_header.empty()) {
		_header = this->_makeHeader();
	}
	if (verbose) {
		this->_toLog(_header);
	}
	return _header;
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

template<class T> Record ImageMetaData<T>::_getStatistics() const {
	if (_stats.nfields() == 0) {
		_stats = this->_calcStats();
	}
	return _stats;
}


template<class T> String ImageMetaData<T>::_getTelescope() const {
	if (_telescope.empty()) {
		_telescope = _getCoords().obsInfo().telescope();
	}
	return _telescope;
}

} //# NAMESPACE CASA - END

