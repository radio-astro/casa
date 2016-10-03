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

#ifndef IMAGEANALYSIS_COMPLEXIMAGEREGRIDDER_H
#define IMAGEANALYSIS_COMPLEXIMAGEREGRIDDER_H

#include <imageanalysis/ImageAnalysis/ImageRegridderBase.h>

#include <scimath/Mathematics/Interpolate2D.h>
#include <casa/namespace.h>

namespace casa {

class ComplexImageRegridder : public ImageRegridderBase<casacore::Complex> {
	// <summary>
	// Top level interface which regrids an image with complex-valued pixels to a specified coordinate system
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
	// if <src>overwrite</src> is true, if image already exists it will be removed
	// if <src>overwrite</src> is false, if image already exists exception will be thrown
	// <group>

	ComplexImageRegridder(
		const SPCIIC image,
		const casacore::Record *const regionRec,
		const casacore::String& maskInp, const casacore::String& outname, casacore::Bool overwrite,
		const casacore::CoordinateSystem& csysTo, const casacore::IPosition& axes,
		const casacore::IPosition& shape
	);

	template <class T> ComplexImageRegridder(
		const SPCIIC image, const casacore::String& outname,
		const SPCIIT templateIm, const casacore::IPosition& axes=casacore::IPosition(),
		const casacore::Record *const regionRec=0,
		const casacore::String& maskInp="", casacore::Bool overwrite=false,
		const casacore::IPosition& shape=casacore::IPosition()
	);
	// </group>

	// destructor
	~ComplexImageRegridder();

	// perform the regrid.
	SPIIC regrid() const;

protected:

	casacore::String getClass() const { return _class; }

private:

	static const casacore::String _class;

	// disallow default constructor
	ComplexImageRegridder();

};
}

#endif
