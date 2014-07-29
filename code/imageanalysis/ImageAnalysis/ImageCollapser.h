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

#ifndef IMAGES_IMAGECOLLAPSER_H
#define IMAGES_IMAGECOLLAPSER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {

template <class T> class TempImage;
template <class T> class SubImage;


class ImageCollapser : public ImageTask {
	// <summary>
	// Top level interface which allows collapsing of images along a single axis. An aggregate method
	// (average, sum, etc) is applied to the collapsed pixels.
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

	enum AggregateType {
		//AVDEV,
		MAX,
		MEAN,
		MEDIAN,
		MIN,
		RMS,
		STDDEV,
		SUM,
		VARIANCE,
		// set all pixels in output image to 0
		ZERO,
		UNKNOWN
	};

	// if <src>outname</src> is empty, no image will be written
	// if <src>overwrite</src> is True, if image already exists it will be removed
	// if <src>overwrite</src> is False, if image already exists exception will be thrown
	//
	// <group>

	ImageCollapser(
		String aggString, const ImageTask::shCImFloat image,
		const String& region, const Record *const regionRec,
		const String& box,
		const String& chanInp, const String& stokes,
		const String& maskInp, const IPosition& axes,
		const String& outname, const Bool overwrite
	);

	ImageCollapser(
		const ImageTask::shCImFloat image,
		const IPosition& axes, const Bool invertAxesSelection,
		const AggregateType aggregateType,
		const String& outname, const Bool overwrite
	);
	// </group>

	// destructor
	~ImageCollapser();

	// perform the collapse. If <src>wantReturn</src> is True, return a pointer to the
	// collapsed image. The returned pointer is created via new(); it is the caller's
	// responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
	// a NULL pointer is returned and pointer deletion is performed internally.
	ImageInterface<Float>* collapse(const Bool wantReturn) const;

	static ImageCollapser::AggregateType aggregateType(String& aggString);

	static const map<uInt, Float (*)(const Array<Float>&)>* funcMap();
	static const map<uInt, String>* funcNameMap();
	static const map<uInt, String>* minMatchMap();

	inline String getClass() const { return _class; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return vector<Coordinate::Type>(0);
	}

private:
	Bool _invertAxesSelection;
	IPosition _axes;
	AggregateType _aggType;
	static const String _class;

	static map<uInt, Float (*)(const Array<Float>&)> _funcMap;
	static map<uInt, String> *_funcNameMap, *_minMatchMap;

	// disallow default constructor
	ImageCollapser();

	void _invert();

	//std::vector<ImageInputProcessor::OutputStruct> _getOutputStruct();

	void _finishConstruction();

	// necessary to improve performance
	void _doMedian(
		const SubImage<Float>& subImage,
		TempImage<Float>& outImage
	) const;

	void _attachOutputMask(
		TempImage<Float>& outImage,
		const Array<Bool>& outMask
	) const;

	static const map<uInt, Float (*)(const Array<Float>&)>& _getFuncMap();


};
}

#endif
