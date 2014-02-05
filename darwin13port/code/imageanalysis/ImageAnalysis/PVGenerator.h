//# tSubImage.cc: Test program for class SubImage
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGES_PVGENERATOR_H
#define IMAGES_PVGENERATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <casa/namespace.h>
#include <tr1/memory.hpp>

class MDirection;

namespace casa {

class PVGenerator : public ImageTask<Float> {
	// <summary>
	// Top level interface for generating position-velocity images
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// High level interface for generating position-velocity images.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageCollapser collapser();
	// collapser.collapse();
	// </srcblock>
	// </example>

public:

	// The region selection in the constructor only applies to the non-direction coordinates.
	// The direction coordinate limits are effectively set by calling setEndPoints()
	// after construction. The region selection in the constructor is only for things like
	// spectral selection and polarization selection. In addition at most one of <src>regionRec</src>
	// and <src>chanInp/stokes</src> should be supplied. If specifying <src>regionRec</src> that should
	// be a non-null pointer and chanInp and stokes should both be empty strings. If specifying either or
	// both of chanInp and/or stokes, the one(s) being specified should be non-empty strings corresponding
	// to correct syntax for that particular parameter, and regionRec should be null.
	// If you specify <src>regionRec</src>=0
	// and <src>stokes</src>="", and <src>chanInp</src>="", that implies you want to use all
	// spectral channels and all polarization planes in the input image.
	PVGenerator(
		const SPCIIF image,
		const Record *const &regionRec, const String& chanInp,
		const String& stokes, const String& maskInp,
		const String& outname, const Bool overwrite
	);

	// destructor
	~PVGenerator();

	// perform the collapse. If <src>wantReturn</src> is True, return a pointer to the
	// collapsed image.
	std::tr1::shared_ptr<ImageInterface<Float> > generate(const Bool wantReturn) const;

	// set the end points of the slice in direction space. Input values represent pixel
	// coordinates in the input image.
	void setEndpoints(
		const std::pair<Double, Double>& start,
		const std::pair<Double, Double>& end
	);

	// set end points given center in pixels, length of segment in pixels, and position angle
	// taken in the normal astronomical sense, measured from north through east.
	void setEndpoints(
		const std::pair<Double, Double>& center, Double length,
		const Quantity& pa
	);

	void setEndpoints(
		const std::pair<Double, Double>& center, const Quantity& length,
		const Quantity& pa
	);

	void setEndpoints(
		const MVDirection& center, const Quantity& length,
		const Quantity& pa
	);

	// <src>length in pixels</src>
	void setEndpoints(
		const MVDirection& center, Double length,
		const Quantity& pa
	);

	void setEndpoints(
		const MVDirection& start, const MVDirection& end
	);

	// Set the number of pixels perpendicular to the slice for which averaging
	// should occur. Must be odd and >= 1. 1 => just use the pixels coincident with the slice
	// (no averaging). 3 => Average three pixels, one pixel on either side of the slice and the
	// pixel lying on the slice.
	// Note this average is done after the image has been rotated.
	void setWidth(uInt width);
	// This will set the width by rounding <src>q</src> up so that the width is an odd number of pixels.
	void setWidth(const Quantity& q);

	String getClass() const;

	// set the unit to be used for the offset axis in the resulting image (from calling
	// generate()). Must conform to angular units
	void setOffsetUnit(const String& s);


protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		vector<Coordinate::Type> v;
		v.push_back(Coordinate::SPECTRAL);
		v.push_back(Coordinate::DIRECTION);
		return v;
 	}

    virtual Bool _mustHaveSquareDirectionPixels() const {return True;}


private:
	std::auto_ptr<vector<Double> > _start, _end;
	uInt _width;
	String _unit;
	static const String _class;


	// disallow default constructor
	PVGenerator();

	void _checkWidth(const Int64 xShape, const Int64 yShape) const;

	Quantity _increment() const;


};
}

#endif
