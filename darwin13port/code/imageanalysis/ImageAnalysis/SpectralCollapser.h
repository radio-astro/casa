//# SpectralCollapser.h: Header file for class SpectralCollapser
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

#include <images/Images/ImageInterface.h>
#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

#include <tr1/memory.hpp>

namespace casa {

template <class T> class SubImage;

class SpectralCollapser {
	// <summary>
	// Class to collapse an image along the spectral axis.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	//   <li> <linkto class=ImageCollapser>ImageCollapser</linkto>
	// </prerequisite>

	// <etymology>
	// Collapses an image along the spectral axis
	// </etymology>

	// <synopsis>
	// Helper class to collapse an image along the spectral axis. The spectral
	// range to combine is provided interactively from the profiler. The class
	// transforms the range to the channels to be combined. The actual image\
	// combination is then done with the class "ImageCollapser".
	// </synopsis>

public:
	// The different collapse types
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

	// Constructor
	SpectralCollapser(const ImageTask::shCImFloat image);

	// Constructor
	SpectralCollapser(const ImageTask::shCImFloat image, const String storePath);

	// Destructor
	virtual ~SpectralCollapser();

	// Parameters:
	// <src>specVals</src>  - the vector of spectral values
	// <src>startVal</src>  - the spectral value to start the collapse
	// <src>endVal</src>    - the spectral value to end the collapse
	// <src>unit</src>      - the unit of the spectral values
	// <src>collType</src>  - the collapse type (e.g. "mean" or "median")
	// <src>collError</src> - information on what the error
	// <src>outname</src>   - name of the collapsed image (output)
	// <src>msg</src>       - message string (output)
	virtual Bool collapse(const Vector<Float> &specVals, const Float startVal, const Float endVal,
			const String &unit, const SpectralCollapser::CollapseType &collType, const SpectralCollapser::CollapseError &collError, String &outname, String &msg);

	String summaryHeader() const;

   void collapseTypeToVector(const SpectralCollapser::CollapseType &collType, Vector<Int> &momentVec);

   // Convert from string to collapse type
	static void stringToCollapseType(const String &text,  SpectralCollapser::CollapseType &collType);

	// Convert from string to error type
	static void stringToCollapseError(const String &text, SpectralCollapser::CollapseError &collError);

	// Convert from collapse type to string
   static void collapseTypeToString(const SpectralCollapser::CollapseType &collType, String &strCollType);

   // Convert from error type to string
   static void collapseErrorToString(const SpectralCollapser::CollapseError &collError, String &strCollError);

private:
   ImageTask::shCImFloat _image;
   LogIO *_log;

   String _storePath;
   String _all; // = CasacRegionManager::ALL;
   IPosition _specAxis;
   Bool _hasQualAxis;

	// Disallow default constructor
   SpectralCollapser();

   void _setUp();
   Bool _cleanTmpData(const String &tmpFileName) const;
   Bool _cleanTmpData(const String &tmpData, const String &tmpError) const;
   Bool _getQualitySubImg(const ImageInterface<Float>* image, const Bool &data, SubImage<Float> &qualitySub);
   Bool _getQualitySubImgs(ImageTask::shCImFloat image, std::tr1::shared_ptr<SubImage<Float> > subData, std::tr1::shared_ptr<SubImage<Float> >  subError) const;
   Bool _getOutputName(const String &wcsInp, String &outImg, String &outImgData, String &outImgError) const;
   Bool _collapse(const ImageTask::shCImFloat image, const String &aggString,
   		const String& chanInp, const String& outname) const;
   Bool _moments(const ImageInterface<Float> *image, const Vector<Int> &momentVec,
   		const Int & startIndex, const Int &endIndex, const String& outname);
   Bool _mergeDataError(const String &outImg, const String &dataImg, const String &errorImg, const Float &normError=1.0) const;
	void _addMiscInfo(const String &outName, const String &wcsInput, const String &chanInput,
			const SpectralCollapser::CollapseType &collType, const SpectralCollapser::CollapseError &collError) const;
	void _collTypeToImCollString(const SpectralCollapser::CollapseType &collType, String &colType) const;
	void _collErrorToImCollString(const SpectralCollapser::CollapseError &collError, String &colError) const ;

};
}

#endif
