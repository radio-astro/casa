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

#ifndef IMAGES_IMAGEINPUTPROCESSOR_H
#define IMAGES_IMAGEINPUTPROCESSOR_H

#include <images/Images/ImageInterface.h>
#include <images/Images/ImageMetaData.h>

#include <casa/namespace.h>

namespace casa {

class ImageInputProcessor {
	// <summary>
	// Collection of methods for processing inputs to image analysis applications
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Processes inputs to image analysis apps.
	// </etymology>

	// <synopsis>
	// Collection of methods for processing inputs to image analysis applications
	// </synopsis>

public:
	// instruction if input stokes is blank
	enum StokesControl {
		USE_FIRST_STOKES,
		USE_ALL_STOKES
	};

	//constructor
	ImageInputProcessor();

	//Destructor
	~ImageInputProcessor();

	// Process the inputs. Output parameters are the pointer to the
	// opened <src>image</src>, the specified region as a record (<src>
	// regionRecord</src>, and a <src>diagnostics</src> String describing
	// how the region was chosen. <src>stokesControl</src> indicates default
	// stokes range to use if <src>stokes</src> is blank.
    void process(
    	ImageInterface<Float>*& image, Record& regionRecord,
    	String& diagnostics, const String& imagename,
    	const Record* regionPtr, const String& regionName,
    	const String& box, const String& chans,
    	const String& stokes, const StokesControl& stokesControl
    ) const;

private:
    LogIO *_log;

    void _setRegion(
    	Record& regionRecord, String& diagnostics,
    	const Record* regionPtr
    ) const;

    void _setRegion(Record& regionRecord, String& diagnostics,
    	const ImageInterface<Float> *image, const String& regionName
    ) const;

    void _setRegion(
    	Record& regionRecord, String& diagnostics,
    	const Vector<Double>& boxCorners, const Vector<uInt>& chanEndPts,
    	const Vector<uInt>& polEndPts, const ImageMetaData& md,
    	const ImageInterface<Float> *iamge
    ) const;

    Vector<uInt> _setSpectralRanges(
    	String specification, const ImageMetaData& metaData
    ) const;

    Vector<Double> _setBoxCorners(const String& box) const;

    Vector<uInt> _consolidateAndOrderRanges(const Vector<uInt>& ranges) const;

    Vector<uInt> _setPolarizationRanges(
    	String specification, const ImageMetaData& metaData,
    	const String& imageName, const StokesControl& stokesControl
    ) const;

    String _pairsToString(const Vector<uInt>& pairs) const;

    String _cornersToString(const Vector<Double>& corners) const;
};
}

#endif /* IMAGES_IMAGEINPUTPROCESSOR_H */
