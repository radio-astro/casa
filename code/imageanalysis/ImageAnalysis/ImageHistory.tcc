#include <imageanalysis/ImageAnalysis/ImageHistory.h>

#include<stdcasa/cboost_foreach.h>

namespace casa {

template<class T> ImageHistory<T>::ImageHistory(
	const SPIIT image
) : _image(image) {}

template<class T> void ImageHistory<T>::addHistory(
	const String& origin,
	const String& history
) {
	vector<String> x;
	x.push_back(history);
	addHistory(origin, x);
}

template<class T> void ImageHistory<T>::addHistory(
	const String& origin,
	const vector<String>& history
) {
	LogOrigin lor = origin.empty()
		? LogOrigin(getClass(), __FUNCTION__)
		: LogOrigin(origin);

	LoggerHolder& log = _image->logger();
	//
	// Make sure we can write into the history table if needed
	//
	log.reopenRW();
	LogSink& sink = log.sink();
	foreach_(String line, history) {
		String x = line;
		x.trim();
		if (! x.empty()) {
			LogMessage msg(line, lor);
            sink.postLocally(msg);
		}
	}
}

template<class T> void ImageHistory<T>::addHistory(
	const String& origin,
	const vector<string>& history
) {
	vector<String> x;
	foreach_(string h, history) {
		x.push_back(h);
	}
	addHistory(origin, x);
}

template<class T> LogIO& ImageHistory<T>::getLogSink() {
	return _image->logSink();
}

}
