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

#ifndef IMAGEANALYSIS_IMAGEREGRIDDER_H
#define IMAGEANALYSIS_IMAGEREGRIDDER_H

#include <imageanalysis/ImageAnalysis/ImageRegridderBase.h>

#include <scimath/Mathematics/Interpolate2D.h>
#include <casa/namespace.h>

namespace casa {

template <class T> class SubImage;
template <class T> class TempImage;

class ImageRegridder : public ImageRegridderBase<Float> {
	// <summary>
	// Top level interface which regrids an image to a specified coordinate system
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Regrids image.
	// </etymology>

	// <synopsis>
	// High level interface for regridding an image. Note that in the case of a complex-valued
	// image, the image is first divided into its composite real and imaginary parts, and these
	// parts are regridded independently. The resulting regridded images are combined to form
	// the final regridded complex-valued image.
	// </synopsis>

public:

	// if <src>outname</src> is empty, no image will be written
	// if <src>overwrite</src> is True, if image already exists it will be removed
	// if <src>overwrite</src> is False, if image already exists exception will be thrown
	// <group>

	ImageRegridder(
		const SPCIIF image,
		const Record *const regionRec,
		const String& maskInp, const String& outname, Bool overwrite,
		const CoordinateSystem& csysTo, const IPosition& axes,
		const IPosition& shape
	);

	// FIXME Add support to allow image and templateIm to be of different data types
	ImageRegridder(
		const SPCIIF image, const String& outname,
		const SPCIIF templateIm, const IPosition& axes=IPosition(),
		const Record *const regionRec=0,
		const String& maskInp="", Bool overwrite=False,
		const IPosition& shape=IPosition()
	);
	// </group>

	// destructor
	~ImageRegridder();

	// perform the regrid.
	SPIIF regrid() const;

	inline String getClass() const { return _class; }

	void setDebug(Int debug) { _debug = debug; }

private:
	Int _debug;
	static const String _class;

	// disallow default constructor
	ImageRegridder();

	SPIIF _regrid() const;

	SPIIF _regridByVelocity() const;

	static Bool _doImagesOverlap(
		SPCIIF image0,
		SPCIIF image1
	);

	static Vector<std::pair<Double, Double> > _getDirectionCorners(
		const DirectionCoordinate& dc,
		const IPosition& directionShape
	);

	void _checkOutputShape(
		const SubImage<Float>& subImage,
		const std::set<Coordinate::Type>& coordsToRegrid
	) const;

	SPIIF _decimateStokes(SPIIF workIm) const;

	static Bool _doRectanglesIntersect(
		const Vector<std::pair<Double, Double> >& corners0,
		const Vector<std::pair<Double, Double> >& corners1
	);

};
}

#endif
