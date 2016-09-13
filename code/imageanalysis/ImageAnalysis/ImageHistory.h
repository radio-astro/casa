#ifndef IMAGEANALYSIS_IMAGEHISTORY_H
#define IMAGEANALYSIS_IMAGEHISTORY_H

#include <imageanalysis/ImageTypedefs.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageHistory {
    // <summary>
    // Top level interface accessing image history data.
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // Image History access
    // </etymology>

    // <synopsis>
    // Top level interface for accessing image history.
    // </synopsis>

public:

    ImageHistory() = delete;
    ImageHistory(const SPIIT image);

    // destructor
    ~ImageHistory() {}

    // add a line to the history
    void addHistory(
        const casacore::String& origin, const casacore::String& history
    );

    void addHistory(
        const casacore::LogOrigin& origin, const casacore::String& history
    );
    // add multiple history lines, all which have the same origin
    void addHistory(
        const casacore::String& origin, const vector<casacore::String>& history
    );

    void addHistory(
        const casacore::LogOrigin& origin, const vector<casacore::String>& history
    );

    void addHistory(
        const casacore::String& origin, const vector<string>& history
    );

    void addHistory(
        const vector<std::pair<casacore::LogOrigin, casacore::String> >& history
    );

    vector<casacore::String> get(casacore::Bool list) const;
    vector<string> getAsStdStrings(casacore::Bool list) const;

    //Append the specified image's history to this image's history
    void append(SPCIIF image);

    void append(SPCIIC image);

    casacore::String getClass() const { const static casacore::String s = "ImageHistory"; return s; }

    casacore::LogIO& getLogSink();

private:

    const SPIIT _image;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageHistory.tcc>
#endif

#endif
