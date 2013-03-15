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

class MDirection;

namespace casa {

class PVGenerator : public ImageTask {
	// <summary>
	// Top level interface for generatting position-velocity images
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// High level interface for collapsing an image along a single axis.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageCollapser collapser();
	// collapser.collapse();
	// </srcblock>
	// </example>

public:

	// The region selection in the constructor only applies to the non-direction coordinate.
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
		const ImageInterface<Float> *const image,
		const Record *const &regionRec, const String& chanInp,
		const String& stokes, const String& maskInp,
		const String& outname, const Bool overwrite
	);

	// destructor
	~PVGenerator();

	// perform the collapse. If <src>wantReturn</src> is True, return a pointer to the
	// collapsed image. The returned pointer is created via new(); it is the caller's
	// responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
	// a NULL pointer is returned and pointer deletion is performed internally.
	ImageInterface<Float>* generate(const Bool wantReturn) const;

	// set the endpoints of the slice in direction space. Input values represent pixel
	// locations.
	void setEndpoints(
		const Double startx, const Double starty,
		const Double endx, const Double endy
	);

	void setHalfWidth(const Double halfwidth);

	String getClass() const;

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

private:
	std::auto_ptr<vector<Double> > _start, _end;
	Double _halfwidth;
	static const String _class;


	// disallow default constructor
	PVGenerator();

	void _checkWidth(const Int64 xShape, const Int64 yShape) const;


};
}

#endif
