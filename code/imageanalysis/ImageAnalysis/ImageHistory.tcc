#include <imageanalysis/ImageAnalysis/ImageHistory.h>

#include <iomanip>

#include <casa/OS/Time.h>

namespace casa {

template<class T> ImageHistory<T>::ImageHistory(
        const SPIIT image
) : _image(image) {}

template<class T> void ImageHistory<T>::addHistory(
        const casacore::String& origin,
        const casacore::String& history
) {
    vector<casacore::String> x;
    x.push_back(history);
    addHistory(origin, x);
}

template<class T> void ImageHistory<T>::addHistory(
        const casacore::LogOrigin& origin,
        const casacore::String& history
) {
    vector<casacore::String> x;
    x.push_back(history);
    addHistory(origin.toString(), x);
}

template<class T> void ImageHistory<T>::addHistory(
        const vector<std::pair<casacore::LogOrigin, casacore::String> >& history
) {
    casacore::LoggerHolder& log = _image->logger();
    //
    // Make sure we can write into the history table if needed
    //
    log.reopenRW();
    casacore::LogSink& sink = log.sink();
    vector<std::pair<casacore::LogOrigin, casacore::String> >::const_iterator begin = history.begin();
    vector<std::pair<casacore::LogOrigin, casacore::String> >::const_iterator iter = begin;
    vector<std::pair<casacore::LogOrigin, casacore::String> >::const_iterator end = history.end();
    while (iter != end) {
        casacore::String x = iter->second;
        x.trim();
        if (! x.empty()) {
            casacore::LogMessage msg(iter->second, iter->first);
            sink.postLocally(msg);
            iter++;
        }
    }
}


template<class T> void ImageHistory<T>::addHistory(
        const casacore::String& origin,
        const vector<casacore::String>& history
) {
    casacore::LogOrigin lor = origin.empty()
		        ? casacore::LogOrigin(getClass(), __FUNCTION__)
		                : casacore::LogOrigin(origin);

    casacore::LoggerHolder& log = _image->logger();
    //
    // Make sure we can write into the history table if needed
    //
    log.reopenRW();
    casacore::LogSink& sink = log.sink();
    for( casacore::String line: history ) {
        casacore::String x = line;
        x.trim();
        if (! x.empty()) {
            casacore::LogMessage msg(line, lor);
            sink.postLocally(msg);
        }
    }
}

template<class T> void ImageHistory<T>::addHistory(
    const casacore::String& origin,
    const vector<string>& history
) {
    std::vector<casacore::String> x;
    for( casacore::String h: history ) {
        x.push_back(h);
    }
    addHistory(origin, x);
}

template<class T> void ImageHistory<T>::addHistory(
    const casacore::LogOrigin& origin,
    const std::vector<casacore::String>& history
) {
    addHistory(origin.toString(), history);
}

template<class T> casacore::LogIO& ImageHistory<T>::getLogSink() {
    return _image->logSink();
}

template<class T> vector<casacore::String> ImageHistory<T>::get(
        casacore::Bool list
) const {
    vector<casacore::String> t;
    casacore::LoggerHolder& logger = _image->logger();
    casacore::uInt i = 1;
    casacore::LogIO log;
    casacore::LogMessage msg;
    for (casacore::LoggerHolder::const_iterator iter = logger.begin(); iter
    != logger.end(); iter++, i++) {
        if (list) {
            casacore::LogOrigin lor = iter->location().empty()
				        ? casacore::LogOrigin(getClass(), __FUNCTION__)
				                : casacore::LogOrigin(iter->location());
            msg.origin(lor);
            casacore::Double jdn = iter->time()/C::day + C::MJD0;
            casacore::Time t(jdn);
            msg.messageTime(t);
            msg.message(iter->message(), true);
            log.post(msg);
        }
        t.push_back(iter->message());
    }
    return t;
}

template<class T> vector<string> ImageHistory<T>::getAsStdStrings(
        casacore::Bool list
) const {
    vector<casacore::String> hist = get(list);
    vector<string> x;
    for( casacore::String s: hist ) {
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
