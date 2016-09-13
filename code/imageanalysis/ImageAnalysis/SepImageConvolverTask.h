//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#ifndef IMAGES_SEPIMAGECONVOLVERTASK_H
#define IMAGES_SEPIMAGECONVOLVERTASK_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/aips.h>

namespace casa {

// <summary>
// This class is the high level interface to image convolving
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="ImageInterface">ImageInterface</linkto>
//   <li> <linkto class="Convolver">Convolver</linkto>
// </prerequisite>

// <etymology>
// This class is the high level interface to image convolving
// </etymology>

// <synopsis>
// This class is the high level interface to image convolving
// </synopsis>

// <example>
// <srcBlock>
// </srcBlock>
// </example>

// <motivation>
// Convolution is a standard image processing requirement.
// </motivation>

// <todo asof="2001/08/28">
//   <li> 
// </todo>

template <class T> class SepImageConvolverTask : public ImageTask<T> {
public:

	const static String CLASS_NAME;

	SepImageConvolverTask() = delete;

	SepImageConvolverTask(
		const SPCIIT image, const Record *const &regionPtr,
	    const String& mask, const String& outname, const Bool overwrite
	);
	
    SepImageConvolverTask(const SepImageConvolverTask<T> &other) = delete;

	~SepImageConvolverTask() {}

	SepImageConvolverTask &operator=(const SepImageConvolverTask<T> &other) = delete;

	SPIIT convolve();

	void setKernels(const Vector<String>& kernels) { _kernels.assign(kernels); }

	void setKernelWidths(const Vector<Quantity>& kernelwidths) { _kernelWidths.assign(kernelwidths); }

	void setScale(Double d) { _scale = d; }

	void setSmoothAxes(const Vector<Int>& axes) { _axes.assign(axes); }

	String getClass() const { return CLASS_NAME; }

protected:

   	CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_ALL_STOKES;
   	}

    vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>();
    }

    inline Bool _supportsMultipleRegions() const {return True;}

private:
    Vector<String> _kernels;
    Vector<Quantity> _kernelWidths;
    Double _scale = 0;
    Vector<Int> _axes;

};

}

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/SepImageConvolverTask.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif



