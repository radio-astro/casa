#ifndef IMAGEANALYSIS_STATIMAGECREATOR_H
#define IMAGEANALYSIS_STATIMAGECREATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casacore/scimath/Mathematics/Interpolate2D.h>

#include <casa/namespace.h>

namespace casa {

class StatImageCreator : public ImageTask<Float> {
	// <summary>
	// Create a "statistic" image from an image.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
    // See CAS-9195.
	// </synopsis>

public:

	StatImageCreator() = delete;

	// <src>region</src> is the usual region used in image analysis tasks.
	// It is the region in the image over which to perform the computations.
	// The subregion over which to carry out a single statistic computation is
	// set separately after the object has been constructed.
	StatImageCreator(
		SPCIIF image, const Record *const region, const String& mask,
		const String& outname, Bool overwrite
	);

	// destructor
	~StatImageCreator() {}

	SPIIF compute() const;

	String getClass() const { const static String s = "StatImageCreator"; return s; }

	// set the anchor pixel value. Note that this applies to the image used at construction,
	// not the resulting subimage if a region was also specified.
	void setAnchorPosition(Int x, Int y);

	// set spacing between grid pixels.
	void setGridSpacing(uInt x, uInt y);

	// set interpolation algortihm
	void setInterpAlgorithm(Interpolate2D::Method alg);

	// set radius for circle subregion over which to carry out individual statistics computations.
	void setRadius(const Quantity& radius);

	// set a rectangular subregion over which to carry out individual statistics computations.
	void setRectangle(const Quantity& xLength, const Quantity& yLength);

	void setStatType(casacore::LatticeStatsBase::StatisticsTypes s);

	void setStatType(const String& s);

protected:

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>(1, Coordinate::DIRECTION);
	}

	Bool _mustHaveSquareDirectionPixels() const { return false; }

    Bool _supportsMultipleBeams() const {return true;}

private:

    Quantity _xlen = Quantity(1, "pix");
    Quantity _ylen = Quantity(1, "pix");
    std::pair<uInt, uInt> _grid = std::make_pair(1,1);
    // _anchor pixel world coordinates
    Vector<Double> _anchor;
    IPosition _dirAxes = IPosition(2);
    // FIXME allow user to set interpolation method
    casacore::Interpolate2D _interpolater = Interpolate2D(Interpolate2D::LINEAR);
    String _interpName = "LINEAR";
    casacore::String _statName = "standard deviation";
    casacore::LatticeStatsBase::StatisticsTypes _statType
        = casacore::LatticeStatsBase::SIGMA;


    // compute the storage lattice
    void _computeStorage(
        Lattice<Float> *const& writeTo, SPCIIF subImage,
        uInt nxpts, uInt nypts, Int xstart, Int ystart
    ) const;

    void _doInterpolation(
        TempImage<Float>& output, ArrayLattice<Float>& store,
        SPCIIF subImage, uInt nxpts, uInt nypts, Int xstart, Int ystart
    ) const;

    // start is the pixel offset in the result matrix relative the
    // storage matrix. If one or both values are 0, then pixel 0 of the
    // result matrix corresponds to pixel 0 in the storage matrix. If one or both
    // of the start values are positive, then pixel 0 in the result matrix
    // corresponds to pixel (start/pointsPerCell - 1) in the storage matrix (which
    // is always negative and always greater than -1).
    void _interpolate(
        Matrix<Float>& result, const Matrix<Float>& storage,
        const std::pair<uInt, uInt>& start
    ) const;

};
}

#endif
