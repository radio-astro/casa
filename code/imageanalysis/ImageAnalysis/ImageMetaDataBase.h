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

#ifndef IMAGES_IMAGEMETADATABASE_H
#define IMAGES_IMAGEMETADATABASE_H

#include <images/Images/ImageInterface.h>
#include <imageanalysis/ImageTypedefs.h>
#include <casa/aips.h>

#include <tr1/memory>

namespace casa {

// <summary>
// Base class for image metadata access.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=ImageInterface>ImageInterface</linkto>
// </prerequisite>

// <etymology>
// </etymology>

// <synopsis> 
// </synopsis>

// <example>
// </example>

// <motivation> 

// </motivation>


class ImageMetaDataBase {

public:

	const static String MASKS;

	virtual ~ImageMetaDataBase() {}

	uInt nChannels() const;

	// Is the specified channel number valid for this image?
	Bool isChannelNumberValid(const uInt chan) const;

	// Get the pixel number on the polarization axis of the specified stokes parameter.
	// If the specified stokes parameter does not exist in the image, the value returned
	// is not gauranteed to be anything other than outside the range of 0 to nStokes-1
	// inclusive. Return -1 if the specified stokes parameter is not present or
	// if this image does not have a polarization axis.
 
	Int stokesPixelNumber(const String& stokesString) const;

	// get the stokes parameter at the specified pixel value on the polarization axis.
	// returns "" if the specified pixel is out of range or if no polarization axis.

	String stokesAtPixel(const uInt pixel) const;

	// Get the number of stokes parameters in this image.
	uInt nStokes() const;

	// is the specified stokes parameter present in the image?
	Bool isStokesValid(const String& stokesString) const;

	// Get the shape of the direction axes. Returns a two element
	// Vector if there is a direction coordinate, if not returns a zero element
	// vector.

	Vector<Int> directionShape() const;

	// if the specified stokes parameter is valid. A message suitable for
	// error notification is returned in the form of an in-out parameter
	//if one or both of these is invalid.
	Bool areChannelAndStokesValid(
		String& message, const uInt chan, const String& stokesString
    ) const;

	// convert the header info to a Record and list to logger if verbose=True
	virtual Record toRecord(Bool verbose) const = 0;

	// get the value of the datum corresponding to the given FITS keyword.
	ValueHolder getFITSValue(const String& key) const;

protected:
	const static String _BEAMMAJOR, _BEAMMINOR, _BEAMPA, _BMAJ, _BMIN, _BPA,
		_BUNIT, _CDELT, _CRPIX, _CRVAL, _CTYPE, _CUNIT, _DATAMAX, _DATAMIN,
		_EPOCH, _EQUINOX, _IMTYPE, _MAXPIXPOS, _MAXPOS, _MINPIXPOS,
		_MINPOS, _OBJECT, _OBSDATE, _OBSERVER, _PROJECTION,
		_RESTFREQ, _REFFREQTYPE, _SHAPE, _TELESCOPE;

	virtual SPCIIF _getFloatImage() const = 0;

	virtual SPCIIC _getComplexImage() const = 0;

	ImageMetaDataBase() : _log() {}

	LogIO& _getLog() { return _log; }

	virtual const ImageInfo& _getInfo() const = 0;

	virtual const CoordinateSystem& _getCoords() const = 0;

	virtual Vector<String> _getAxisNames() const = 0;

	virtual Vector<String> _getAxisUnits() const = 0;

	virtual GaussianBeam _getBeam() const = 0;

	virtual String _getBrightnessUnit() const = 0;

	virtual String _getImType() const = 0;

	virtual vector<Quantity> _getIncrements() const = 0;

	virtual Vector<String> _getMasks() const = 0;

	virtual String _getObject() const = 0;

	virtual String _getEquinox() const = 0;

	virtual MEpoch _getObsDate() const = 0;

	String _getEpochString() const;

	virtual String _getObserver() const = 0;

	virtual String _getProjection() const;

	virtual String _getRefFreqType() const = 0;

	virtual Vector<Double> _getRefPixel() const = 0;

	virtual Vector<Quantity> _getRefValue() const = 0;

	virtual Quantity _getRestFrequency() const = 0;

	IPosition _getShape() const;

	virtual Record _getStatistics() const = 0;

	virtual String _getTelescope() const = 0;

	Record _makeHeader() const;

	void _toLog(const Record& header) const;

	uInt _getAxisNumber(const String& key) const;

	Record _calcStats() const;

	const TableRecord _miscInfo() const;

	uInt _ndim() const;

	virtual Vector<String> _getStokes() const = 0;

private:

	mutable LogIO _log;

	mutable IPosition _shape;

	// precision < 0 => use default precision when printing numbers
	void _fieldToLog(const Record& header, const String& field, Int precision=-1) const;

	String _doStandardFormat(Double value, const String& unit) const;

	template <class T> Record _calcStatsT(
		std::tr1::shared_ptr<const ImageInterface<T> > image
	) const;



};



} //# NAMESPACE CASA - END

#endif
