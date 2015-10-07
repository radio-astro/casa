//# ImageHistogramCalculatorr.h: 2D convolution of an image
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
//# $Id: Image2DConvolver.h 20229 2008-01-29 15:19:06Z gervandiepen $

#ifndef IMAGES_IMAGEHISTOGRAMSCALCULATOR_H
#define IMAGES_IMAGEHISTOGRAMSCALCULATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/aips.h>

namespace casa {

// <summary>
// This class computes histograms of image pixel values.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>

// <etymology>
//  This class computes histograms of image pixel values.
// </etymology>

// <synopsis>
// This class computes histograms of image pixel values.
// </synopsis>

// <example>
// <srcBlock>
// </srcBlock>
// </example>

// <motivation>
// users like histograms.
// </motivation>
//   <li> 
// </todo>

class ImageHistogramsCalculator : public ImageTask<Float> {
public:

	const static String CLASS_NAME;

	ImageHistogramsCalculator() = delete;

	ImageHistogramsCalculator(
		const SPCIIF image, const Record *const &regionPtr,
	    const String& mask
	);
	
	ImageHistogramsCalculator(const ImageHistogramsCalculator &other) = delete;

	~ImageHistogramsCalculator();

	ImageHistogramsCalculator &operator=(const ImageHistogramsCalculator &other) = delete;

	Record compute() const;

	String getClass() const { return CLASS_NAME; }

	// set cursor axes
	void setAxes(const vector<uInt>& axes) { _axes = axes; };

	// should the histogram be cumulative
	void setCumulative(Bool b) { _cumulative = b; }

	// If true, force the storage lattice to be written to disk.
	void setDisk(Bool b) { _disk = b; }

	// If true, the counts should be returned as log10 of the actual counts
	void setDoLog10(Bool b) { _doLog10 = b; }

	// set include range
	void setIncludeRange(const vector<double>& r) { _includeRange = r; }

	// If true list stats to logger
	void setListStats(Bool b) { _listStats = b; }

	// set number of bins
	void setNBins(uInt n) { _nbins = n; }


protected:

   	CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_ALL_STOKES;
   	}

    vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>();
    }

    inline Bool _supportsMultipleRegions() const {return True;}

private:
    Bool _disk = False;
    Bool _cumulative = False;
    Bool _listStats = False;
    Bool _doLog10 = False;
    vector<uInt> _axes;
    uInt _nbins = 25;
    vector<double> _includeRange;

};

}
#endif
