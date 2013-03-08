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

#ifndef IMAGES_IMAGEMETADATA_H
#define IMAGES_IMAGEMETADATA_H

#include <images/Images/ImageInterface.h>
#include <casa/aips.h>
#include <tr1/memory>

namespace casa {

// <summary>
// A class in which to store and allow read-only access to image metadata.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=ImageInterface>ImageInterface</linkto>
// </prerequisite>

// <etymology>
// The ImageMetaData class name is derived from its role as holding image metadata.
// </etymology>

// <synopsis> 
// The ImageMetaData object is meant to allow access to image metadata (eg, shape,
// coordinate system info such as spectral and polarization axes numbers, etc).
// </synopsis>

// <example>
// Construct an object of this class by passing the associated image to the constructor.
// <srcblock>
// PagedImage<Float> myImage("myImage");
// ImageMetaData<Float> myImageMetaData(myImage);
// </srcblock>
// </example>

// <motivation> 
// This class is meant to provide an object-oriented interface for accessing
// image metadata without polluting the ImageInterface and CoordinateSystem
// classes with these methods.
// </motivation>
// <todo>
// Merge ImageInfo class into this class.
// </todo>

template <class T> class ImageMetaData {

public:

	ImageMetaData(const ImageInterface<Float> *const &image);

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
	Record toRecord(Bool verbose);

private:
	const static String _BEAMMAJOR, _BEAMMINOR, _BEAMPA,
		_CTYPE, _DATAMAX, _DATAMIN, _EQUINOX, _IMTYPE,
		_MASKS, _MAXPIXPOS, _MAXPOS, _MINPIXPOS, _MINPOS,
		_OBJECT, _OBSDATE, _OBSERVER, _PROJECTION,
		_RESTFREQ, _REFFREQTYPE, _SHAPE, _TELESCOPE;

	const ImageInterface<Float> *const _image;
	std::auto_ptr<LogIO> _log;
	Record _header;

	ImageMetaData() : _image(0), _log(0) {}

	void _toLog() const;

	void _fieldToLog(const String& field) const;
};



} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageMetaData.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
