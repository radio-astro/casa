#include <imageanalysis/ImageAnalysis/ImageCollapserData.h>

#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>

namespace casa {

std::map<uInt, String> *ImageCollapserData::_funcNameMap = 0;
std::map<uInt, String> *ImageCollapserData::_minMatchMap = 0;

const std::map<uInt, String>* ImageCollapserData::funcNameMap() {
	if (! _funcNameMap) {
		std::map<uInt, String> ref;
		//ref[(uInt)AVDEV] = "avdev";
		ref[(uInt)FLUX] = "flux";
		ref[(uInt)MAX] = "max";
		ref[(uInt)MEAN] = "mean";
		ref[(uInt)MEDIAN] = "median";
		ref[(uInt)MIN] = "min";
		ref[(uInt)NPTS] = "nputs";
		ref[(uInt)RMS] = "rms";
		ref[(uInt)SQRTSUM] = "sqrtsum";
		ref[(uInt)SQRTSUM_NPIX_BEAM] = "sqrtsum_npix_beam";
		ref[(uInt)SQRTSUM_NPIX] = "sqrtsum_npix";
		ref[(uInt)STDDEV] = "stddev";
		ref[(uInt)SUM] = "sum";
		ref[(uInt)VARIANCE] = "variance";
		ref[(uInt)ZERO] = "zero";
		_funcNameMap = new std::map<uInt, String>(ref);
	}
	return _funcNameMap;
}

const std::map<uInt, String>* ImageCollapserData::minMatchMap() {
	if (! _minMatchMap) {
		std::map<uInt, String> ref;
		//ref[(uInt)AVDEV] = "a";
		ref[(uInt)FLUX] = "f";
		ref[(uInt)MAX] = "ma";
		ref[(uInt)MEAN] = "mea";
		ref[(uInt)MEDIAN] = "med";
		ref[(uInt)MIN] = "mi";
		ref[(uInt)NPTS] = "n";
		ref[(uInt)RMS] = "r";
		ref[(uInt)SQRTSUM] = "sqrtsum";
		ref[(uInt)SQRTSUM_NPIX_BEAM] = "sqrtsum_npix_beam";
		ref[(uInt)SQRTSUM_NPIX] = "sqrtsum_npix";
		ref[(uInt)STDDEV] = "st";
		ref[(uInt)SUM] = "su";
		ref[(uInt)VARIANCE] = "v";
		ref[(uInt)ZERO] = "z";
		_minMatchMap = new std:: map<uInt, String>(ref);

	}
	return _minMatchMap;
}

ImageCollapserData::AggregateType ImageCollapserData::aggregateType(
	const String& aggString
) {
	ThrowIf (
		aggString.empty(),
		"Aggregate function name is not specified and it must be."
	);
	String agg = aggString;
	agg.downcase();
	const std::map<uInt, String> *funcNamePtr = funcNameMap();
	std::map<uInt, String>::const_iterator iter;
	const std::map<uInt, String> *minMatch = minMatchMap();
	std::map<uInt, String>::const_iterator end = minMatch->end();
	for (iter = minMatch->begin(); iter != end; iter++) {
		uInt key = iter->first;
		String minMatch = iter->second;
		String funcName = (*funcNamePtr).at(key);
		if (
			agg.startsWith(minMatch)
			&& funcName.startsWith(agg)
		) {
			return (AggregateType)key;
		}
	}
	ThrowCc("Unknown aggregate function specified by " + aggString);
}

}
