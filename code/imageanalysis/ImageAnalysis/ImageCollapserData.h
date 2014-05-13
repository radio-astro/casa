#ifndef IMAGES_IMAGECOLLAPSERDATA_H
#define IMAGES_IMAGECOLLAPSERDATA_H

#include <casa/typeinfo.h>

#include <map>

namespace casa {

class String;

class ImageCollapserData {
	// <summary>
	// Non-templated data related bits for ImageCollapser.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Data for ImageCollapser.
	// </etymology>

	// <synopsis>
	// </synopsis>

	// <example>
	// </example>

public:

	enum AggregateType {
		//AVDEV,
		MAX,
		MEAN,
		MEDIAN,
		MIN,
		NPTS,
		RMS,
		STDDEV,
		SUM,
		VARIANCE,
		// set all pixels in output image to 0
		ZERO,
		FLUX,
		// SQRTSUM and MEANSQRT are bizarre statistics, but
		// ImageAnalysis::getFreqProfile() supported this for the GUI profiler,
		// so sadly, we must support them here. I actually suspect the intent was for
		// the sums in these to be the sum of the squares, at least that's what the comments
		// in ImageAnalysis::getFreqProfile() seemed to imply, but they were implemented
		// as straight sums, so I followed the implementation because there is no design
		// or requirements doc to draw from. What a surprise.
		// the square root of the sum, sqrt(sum)
		SQRTSUM,
		// the square root of the sum divided by the number of pixels, sqrt(sum)/npix
		SQRTSUM_NPIX,
		// the square root of the sum divided by the number of pixels per beam, sqrt(sum)/(beam_area/pixel_area)
		// Yes that's how it was implemented in ImageAnalysis::getFreqProfile().
		SQRTSUM_NPIX_BEAM,
		UNKNOWN
	};

	// destructor
	~ImageCollapserData() {}

	static AggregateType aggregateType(const String& aggString);

	static const std::map<uInt, String>* funcNameMap();
	static const std::map<uInt, String>* minMatchMap();

private:

	static std::map<uInt, String> *_funcNameMap, *_minMatchMap;

	// disallow default constructor
	ImageCollapserData();

};
}

#endif
