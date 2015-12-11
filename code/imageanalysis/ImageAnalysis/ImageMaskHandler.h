#ifndef IMAGEANALYSIS_IMAGEMASKHANDLER_H
#define IMAGEANALYSIS_IMAGEMASKHANDLER_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

#include <imageanalysis/ImageTypedefs.h>

#include <set>

using namespace std;

namespace casa {

template <class T> class ImageMaskHandler {

    // <summary>
    // Perform various image mask operations
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // </etymology>

    // <synopsis>
    // </synopsis>

public:
	ImageMaskHandler(SPIIT);

    ~ImageMaskHandler();

    // set default mask. Empty string means unset default mask.
    void set(const String& name);

    // return the default mask name
    String defaultMask() const;

    // delete specified masks
    void deleteMasks(const std::set<String>& masks);

    // rename a mask
    void rename(const String& oldName, const String& newName);

    // get all mask names
    Vector<String> get() const;

    // copy mask
    void copy(const String& currentName, const String& newName);

    void calcmask(
    	const String& mask, Record& regions,
		const String& name, Bool asdefault=True
    );

private:
    const SPIIT _image;

    void _calcmask(
    	const LatticeExprNode& node,
		const String& name, const Bool makedefault
    );

};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageMaskHandler.tcc>
#endif

#endif
