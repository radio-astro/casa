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
		RMS,
		STDDEV,
		SUM,
		VARIANCE,
		// set all pixels in output image to 0
		ZERO,
		FLUX,
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
