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

#ifndef IMAGEANALYSIS_IMAGECONCATENATOR_H
#define IMAGEANALYSIS_IMAGECONCATENATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <casa/namespace.h>

namespace casacore{

template <class T> class ImageConcat;
}

namespace casa {


template <class T>  class ImageConcatenator : public ImageTask<T> {
	// <summary>
	// Top level interface for concatenating images
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Concatenate an image
	// </etymology>

	// <synopsis>
	// High level interface for concatenating images.
	// </synopsis>

public:

	// <src>image</src> should be the first image in the list of images to be concatenated.
	ImageConcatenator(
		SPCIIT image,
		const casacore::String& outname, casacore::Bool overwrite
	);

	// destructor
	~ImageConcatenator();

	// Perform the concatenation.
	SPIIT concatenate(const vector<casacore::String>& imageNames);

	// Set the axis along which to do the concatenation. A negative value
	// of <src>axis</src> means use the spectral axis. An exception is thrown
	// if <src>axis</src> is negative and the image has no spectral axis.
	void setAxis(casacore::Int axis);

	void setTempClose(casacore::Bool b) { _tempClose = b; }

	void setRelax(casacore::Bool b) { _relax = b; }

	void setReorder(casacore::Bool b) { _reorder = b; }

	casacore::String getClass() const;

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<casacore::Coordinate::Type> _getNecessaryCoordinates() const {
		return vector<casacore::Coordinate::Type>(0);
 	}

private:
	casacore::Int _axis;
	casacore::Bool _tempClose, _relax, _reorder;
	static const casacore::String _class;

	// disallow default constructor
	ImageConcatenator();

	// returns true if world coordinate values increase with pixel coordinate values
	casacore::Bool _minMaxAxisValues(
		casacore::Double& min, casacore::Double& max, casacore::uInt ndim,
		const casacore::CoordinateSystem& csys, const casacore::IPosition& shape
	) const;

	void _addImage(std::unique_ptr<casacore::ImageConcat<T> >& pConcat, const casacore::String& name) const;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageConcatenator.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
