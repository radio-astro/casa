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

#ifndef IMAGEANALYSIS_SPECTRALCOLLAPSER_H
#define IMAGEANALYSIS_SPECTRALCOLLAPSER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class SubImage;

class SpectralCollapser {
	// <summary>
	// TODO: Documentation in the header, add a test program 'tSpectralCollapser'
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

public:
	enum CollapseType {
		PMEAN,
		PMEDIAN,
		PSUM,
		CUNKNOWN
		//PVRMSE,
	};

	enum CollapseError {
		PNOERROR,
		PERMSE,
		PPROPAG,
		EUNKNOWN
	};

	// constructor
	SpectralCollapser(ImageInterface<Float> * &image);

	// constructor
	SpectralCollapser(ImageInterface<Float> * &image, const String storePath);

	// destructor
	virtual ~SpectralCollapser();

	virtual Bool collapse(const Vector<Float> &specVals, const Float startVal, const Float endVal,
			const String &unit, const SpectralCollapser::CollapseType &collType, SpectralCollapser::CollapseError &collError, String &outname, String &msg);

	String summaryHeader() const;

    void collapseTypeToString(const SpectralCollapser::CollapseType &collType, String &colType);

	static void stringToCollapseType(const String &text,  SpectralCollapser::CollapseType &collType);
    static void stringToCollapseError(const String &text, SpectralCollapser::CollapseError &collError);

private:
   ImageInterface<Float> * _image;
   LogIO *_log;

   String _storePath;
   String _all; // = CasacRegionManager::ALL;
   IPosition _specAxis;
   Bool _hasQualAxis;

	// disallow default constructor
   SpectralCollapser();

   void _setUp();

   Bool _getQualitySubImg(const ImageInterface<Float>* image, const Bool &data, SubImage<Float> &qualitySub);

   Bool _getQualitySubImgs(ImageInterface<Float>* image, SubImage<Float> &subData, SubImage<Float> &subError);

   String _getOutputName(const String &wcsInp, const Bool &data);

   Bool _collapse(const ImageInterface<Float> *image, const String &aggString,
   		const String& chanInp, const String& outname);
};
}

#endif
