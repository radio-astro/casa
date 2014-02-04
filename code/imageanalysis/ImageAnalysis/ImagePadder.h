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

#ifndef IMAGES_IMAGEPADDER_H
#define IMAGES_IMAGEPADDER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

#include <tr1/memory>

class MDirection;

namespace casa {

class ImagePadder : public ImageTask<Float> {
	// <summary>
	// Top level interface for padding an image in direction space.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// High level interface for padding an image.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImagePadder padder();
	// padder.pad();
	// </srcblock>
	// </example>

public:
	// <group>

	ImagePadder(
			const SPCIIF  image,
		const Record *const regionRec=0,
		const String& box="", const String& chanInp="",
		const String& stokes="", const String& maskInp="",
		const String& outname="", const Bool overwrite=""
	);

	// </group>

	// destructor
	~ImagePadder();

	// perform the padding. If <src>wantReturn</src> is True, return a pointer to the
	// padded image. The returned pointer is created via new(); it is the caller's
	// responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
	// a NULL pointer is returned and pointer deletion is performed internally.
	ImageInterface<Float>* pad(const Bool wantReturn) const;

	// set the number of pixels to use for padding and their values and if they should be
	// masked on each edge of the direction plane. <src>good</src>=True means the padding
	// pixels will not be masked (set to good, mask values = True).
	void setPaddingPixels(const uInt nPixels, const Double value=0, const Bool good=False);

	String getClass() const;

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		vector<Coordinate::Type> v;
		v.push_back(Coordinate::DIRECTION);
		return v;
 	}

private:
	uInt _nPixels;
	Double _value;
	Bool _good;
	static const String _class;

	// disallow default constructor
	ImagePadder();
};
}

#endif
