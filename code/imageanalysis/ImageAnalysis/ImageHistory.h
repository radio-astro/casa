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

	ImageHistory(
		const SPIIT image
	);

	// destructor
	~ImageHistory() {}

	// add a line to the history
	void addHistory(
		const String& origin,
		const String& history
	);

	void addHistory(
		const LogOrigin& origin,
		const String& history
    );
    // add multiple history lines, all which have the same origin
	void addHistory(
		const String& origin,
		const vector<String>& history
	);

	void addHistory(
		const String& origin,
		const vector<string>& history
	);

    void addHistory(
		const vector<std::pair<LogOrigin, String> >& history
	);

	vector<String> get(Bool list) const;
	vector<string> getAsStdStrings(Bool list) const;

	//Append the specified history to this image's history
	void append(SPCIIF image);

	void append(SPCIIC image);

	String getClass() const { const static String s = "ImageHistory"; return s; }

	LogIO& getLogSink();

private:

	const SPIIT _image;

	// disallow default constructor
	ImageHistory() {}

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageHistory.tcc>
#endif

#endif
