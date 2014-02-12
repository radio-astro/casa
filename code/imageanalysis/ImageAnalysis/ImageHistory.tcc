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

template<class T> vector<String> ImageHistory<T>::get(
	Bool list
) const {
	vector<String> t;
	LoggerHolder& logger = _image->logger();
	uInt i = 1;
	LogIO log;
	for (LoggerHolder::const_iterator iter = logger.begin(); iter
			!= logger.end(); iter++, i++) {
		if (list) {
			LogOrigin lor(getClass(), __FUNCTION__);
			if (!(iter->location()).empty()) {
				log << LogOrigin(iter->location());
			} else {
				log << lor;
			}
			log << endl << iter->message() << endl << LogIO::POST;
		}
		t.push_back(iter->message());
	}
	if (list) {
		log << LogIO::POST;
	}
	return t;
}

template<class T> vector<string> ImageHistory<T>::getAsStdStrings(
	Bool list
) const {
	vector<String> hist = get(list);
	vector<string> x;
	foreach_(String s, hist) {
		x.push_back(s.c_str());
	}
	return x;
}

template<class T> void ImageHistory<T>::append(
	SPCIIF image
) {
	_image->logger().append(image->logger());
}

template<class T> void ImageHistory<T>::append(
	SPCIIC image
) {
	_image->logger().append(image->logger());
}

}
