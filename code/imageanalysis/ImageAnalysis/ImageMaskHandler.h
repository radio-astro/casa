#ifndef IMAGEANALYSIS_IMAGEMASKHANDLER_H
#define IMAGEANALYSIS_IMAGEMASKHANDLER_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

#include <imageanalysis/ImageTypedefs.h>

// #include <lattices/Lattices/LatticeExprNode.h>

#include <set>

using namespace std;

namespace casa {

class ImageRegion;
class LatticeExprNode;
template <class T> class  MaskedLattice;

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

    // copy the mask from the specified lattice to the image's (default) mask. The mask
    // and the image must have the same shape.
    template <class U> void copy(const MaskedLattice<U>& mask);

private:
    const SPIIT _image;

    void _calcmask(
    	const LatticeExprNode& node,
		const String& name, const Bool makedefault
    );

    // Make a block of regions from a Record
    static void _makeRegionBlock(
        PtrBlock<const ImageRegion*>& regions,
        const Record& Regions
    );
};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageMaskHandler.tcc>
#endif

#endif
