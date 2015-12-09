#include <imageanalysis/ImageAnalysis/ImageDecomposerTask.h>

#include <imageanalysis/ImageAnalysis/ImageDecomposer.h>

namespace casa {

template<class T> ImageDecomposerTask<T>::ImageDecomposerTask(
	const SPCIIT image, const Record *const region,
	const String& maskInp
) : ImageTask<T>(image, region, maskInp, "", False) {
	this->_construct();
}

template<class T> void ImageDecomposerTask<T>::setDeblendOptions(
	Double threshold, Int nContour, Int minRange, Int nAxis
) {
	ThrowIf(threshold < 0, "Threshold cannot be negative");
	_threshold = threshold;
	_ncontour = nContour;
	_minrange = minRange;
	_naxis = nAxis;
}

template<class T> void ImageDecomposerTask<T>::setFitOptions(
	Double maxrms, Int maxRetry, Int maxIter, Double convCriteria
) {
	_maxrms = maxrms;
	_maxretry = maxRetry;
	_maxiter = maxIter;
	_convcriteria = convCriteria;
}

template<class T> Matrix<T> ImageDecomposerTask<T>::decompose(
	Matrix<Int>& blcs, Matrix<Int>& trcs
) {
	AxesSpecifier axesSpec(False);
	auto subImage = SubImageFactory<T>::createSubImageRO(
		*this->_getImage(), *this->_getRegion(), this->_getMask(),
		this->_getLog().get(), axesSpec, this->_getStretch()
	);
	// Make finder
	ImageDecomposer<T> decomposer(*subImage);
	decomposer.setDeblend(! _simple);
	decomposer.setDeblendOptions(_threshold, _ncontour, _minrange, _naxis);
	decomposer.setFit(_fit);
	decomposer.setFitOptions(_maxrms, _maxretry, _maxiter, _convcriteria);
	decomposer.decomposeImage();
	decomposer.printComponents();
	Block<IPosition> blcspos(decomposer.numRegions());
	Block<IPosition> trcspos(decomposer.numRegions());
	decomposer.boundRegions(blcspos, trcspos);
	if(! blcspos.empty()) {
		auto n = blcspos.nelements();
		blcs.resize(n, blcspos[0].asVector().size());
		trcs.resize(n, trcspos[0].asVector().size());
		for (uInt k=0; k < n; ++k){
			blcs.row(k)=blcspos[k].asVector();
			trcs.row(k)=trcspos[k].asVector();
		}
	}
	// As yet no methods to put the results into an output container
	// (Note: component list can be output as a Matrix.)
	return decomposer.componentList();
}


}
