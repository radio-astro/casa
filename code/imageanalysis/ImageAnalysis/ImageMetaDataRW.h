//# ImageMetaData.h: Meta information for Images
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
//# $Id$

#ifndef IMAGES_IMAGEMETADATARW_H
#define IMAGES_IMAGEMETADATARW_H

#include <imageanalysis/ImageAnalysis/ImageMetaDataBase.h>

#include <casa/aips.h>

namespace casa {

// <summary>
// A class in which to store and allow read-write access to image metadata.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=ImageInterface>ImageInterface</linkto>
// </prerequisite>

// <etymology>
// The ImageMetaDataRW class name is derived from its role as holding image metadata
// and providing read-write access.
// </etymology>

// <synopsis> 
// The ImageMetaDataRW object is meant to allow access to image metadata (eg, shape,
// coordinate system info such as spectral and polarization axes numbers, etc). It allows
// write access to some values. This class contains the writable methods and extends
// ImageMetaData which contains the read-only methods.
// </synopsis>

// <example>
// Construct an object of this class by passing the associated image to the constructor.
// <srcblock>
// PagedImage<Float> myImage("myImage");
// ImageMetaDataRW<Float> myImageMetaData(myImage);
// </srcblock>
// </example>

// <motivation> 
// This class is meant to provide an object-oriented interface for accessing
// image metadata without polluting the ImageInterface and CoordinateSystem
// classes with these methods.
// </motivation>

class ImageMetaDataRW : public ImageMetaDataBase {

public:

	ImageMetaDataRW(SPIIF image);
	ImageMetaDataRW(SPIIC image);

	// remove, if possible, the specified parameter. Returns True if removal
	// was successful.
	Bool remove(const String& key);

	// remove the specified mask. If the empty string is given, all masks will
	// be removed. Returns True if successful, False otherwise or if the specfied
	// mask does not exist.
	Bool removeMask(const String& maskName);

	Record toRecord(Bool verbose) const;

	// add a key-value pair
	Bool add(const String& key, const ValueHolder& value);

	// set (update) the value associated with the key.
	Bool set(const String& key, const ValueHolder& value);

protected:

	SPCIIF _getFloatImage() const {return _floatImage;}

	SPCIIC _getComplexImage() const {return _complexImage;}

	const ImageInfo& _getInfo() const;

	const CoordinateSystem& _getCoords() const;

	Vector<String> _getAxisNames() const;

	Vector<String> _getAxisUnits() const;

	GaussianBeam _getBeam() const;

	String _getBrightnessUnit() const;

	String _getImType() const;

	vector<Quantity> _getIncrements() const;

	Vector<String> _getMasks() const;

	String _getObject() const;

	String _getEquinox() const;

	MEpoch _getObsDate() const;

	String _getObserver() const;

	String _getProjection() const;

	String _getRefFreqType() const;

	Vector<Double> _getRefPixel() const;

	Vector<Quantity> _getRefValue() const;

	Quantity _getRestFrequency() const;

	String _getTelescope() const;

	Record _getStatistics() const;

	Vector<String> _getStokes() const;

private:
	SPIIF _floatImage;
	SPIIC _complexImage;

	// These are mutable because they are only to be set once and
	// then cached. If this contract is broken, and they are set elsewhere
	// defects will likely occur.
	mutable String _bunit, _imtype, _object, _equinox, _observer, _projection,
		_reffreqtype, _telescope;
	mutable MEpoch _obsdate;
	mutable Quantity _restFreq;
	mutable Vector<String> _masks, _stokes;
	mutable GaussianBeam _beam;
	mutable Vector<String> _axisNames, _axisUnits;
	mutable Vector<Double> _refPixel;
	mutable vector<Quantity> _refVal, _increment;
	mutable Record _header, _stats;

	ImageMetaDataRW() {}

	void _setCoordinateValue(const String& key, const ValueHolder& value);

	String  _getString(const String& key, const ValueHolder& value) const;

	void _setUserDefined(const String& key, const ValueHolder& v);

	Bool _setUnit(const String& unit);

	Bool _setCsys(const CoordinateSystem& csys);

	Bool _setImageInfo(const ImageInfo& info);

	const TableRecord _miscInfo() const;

	void _setMiscInfo(const TableRecord& rec);

	Bool _hasRegion(const String& maskName) const;

	static Quantity _getQuantity(const ValueHolder& v);

	Bool _isWritable() const;
};

} //# NAMESPACE CASA - END

#endif
