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

#ifndef IMAGEANALYSIS_IMAGEPRIMARYBEAMCORRECTOR_H
#define IMAGEANALYSIS_IMAGEPRIMARYBEAMCORRECTOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <memory>

#include <casa/namespace.h>

#include <memory>
#include <tr1/memory.hpp>

namespace casa {

class ImagePrimaryBeamCorrector : public ImageTask {
	// <summary>
	// Top level interface for primary beam correction.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// High level interface for primary beam correction.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImagePrimaryBeamCorrector corrector(...);
	// corrector.correct();
	// </srcblock>
	// </example>

public:

	enum Mode {
		MULTIPLY,
		DIVIDE
	};

	// if <src>outname</src> is empty, no image will be written
	// if <src>overwrite</src> is True, if image already exists it will be removed
	// if <src>overwrite</src> is False, if image already exists exception will be thrown
	// Only one of <src>regionPtr</src> or <src>region<src> should be specified.
	// <group>

	ImagePrimaryBeamCorrector(
			const ImageTask::shCImFloat image,
			const ImageTask::shCImFloat pbImage,
		const Record * const &regionPtr,
		const String& region, const String& box,
		const String& chanInp, const String& stokes,
		const String& maskInp,
		const String& outname, const Bool overwrite,
		const Float cutoff, const Bool useCutoff, const Mode mode
	);

	ImagePrimaryBeamCorrector(
			const ImageTask::shCImFloat image,
		const Array<Float>& pbArray,
		const Record * const &regionPtr,
		const String& region, const String& box,
		const String& chanInp, const String& stokes,
		const String& maskInp,
		const String& outname, const Bool overwrite,
		const Float cutoff, const Bool useCutoff, const Mode mode
	);

	// </group>

	// destructor
	~ImagePrimaryBeamCorrector();

	// perform the correction. If <src>wantReturn</src> is True, return a pointer to the
	// collapsed image. The returned pointer is created via new(); it is the caller's
	// responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
	// a NULL pointer is returned and pointer deletion is performed internally.
	ImageInterface<Float>* correct(const Bool wantReturn) const;

	String getClass() const;

protected:

	vector<Coordinate::Type> _getNecessaryCoordinates() const;

	CasacRegionManager::StokesControl _getStokesControl() const;


private:
	std::auto_ptr<ImageInterface<Float> > _pbImage;

	Float _cutoff;
	Mode _mode;
	Bool _useCutoff;
	const static String _class;

	ImagePrimaryBeamCorrector();

	void _checkPBSanity();
};

}

#endif
