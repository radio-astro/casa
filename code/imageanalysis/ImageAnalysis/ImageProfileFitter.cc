
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>

#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
// #include <casa/Utilities/Precision.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/Combinatorics.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <imageanalysis/IO/ProfileFitterEstimatesFileParser.h>
#include <imageanalysis/IO/ImageProfileFitterResults.h>

namespace casa {

const String ImageProfileFitter::_class = "ImageProfileFitter";

ImageProfileFitter::ImageProfileFitter(
	const ImageInterface<Float> *const &image, const String& region,
	const Record *const &regionPtr,	const String& box,
	const String& chans, const String& stokes,
	const String& mask, const Int axis,
	const uInt ngauss, const String& estimatesFilename,
	const SpectralList& spectralList
) : ImageTask(
		image, region, regionPtr, box, chans, stokes,
		mask, "", False
	),
	_residual(), _model(), _xUnit(), _centerName(),
	_centerErrName(), _fwhmName(), _fwhmErrName(),
	_ampName(), _ampErrName(), _integralName(),
	_integralErrName(), _plpName(), _plpErrName(), _sigmaName(),_multiFit(False),
	_deleteImageOnDestruct(False), _logResults(True), _polyOrder(-1),
	_fitAxis(axis), _nGaussSinglets(ngauss), _nGaussMultiplets(0),
	_nLorentzSinglets(0), _nPLPCoeffs(0),
	_minGoodPoints(0), _results(Record()),
	_nonPolyEstimates(SpectralList()), _goodAmpRange(Vector<Double>(0)),
	_goodCenterRange(Vector<Double>(0)), _goodFWHMRange(Vector<Double>(0)),
	_sigma(0), _abscissaDivisor(1.0) {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
    if (! estimatesFilename.empty()) {
    	if (spectralList.nelements() > 0) {
    		*_getLog() << "Logic error: both a non-empty estimatesFilename "
    			<< "and a non-zero element spectralList cannot be specified"
    			<< LogIO::EXCEPTION;
    	}

    	ProfileFitterEstimatesFileParser parser(estimatesFilename);
    	_nonPolyEstimates = parser.getEstimates();
    	_nGaussSinglets = _nonPolyEstimates.nelements();

    	*_getLog() << LogIO::NORMAL << "Number of gaussian singlets to fit found to be "
    		<<_nGaussSinglets << " in estimates file " << estimatesFilename
    		<< LogIO::POST;
    }
    else if (spectralList.nelements() > 0) {
    	_nonPolyEstimates = spectralList;
    	_nGaussSinglets = 0;
    	_nGaussMultiplets = 0;
    	Bool havePLP = False;
    	for (uInt i=0; i<_nonPolyEstimates.nelements(); i++) {
    		SpectralElement::Types myType = _nonPolyEstimates[i]->getType();
			switch(myType) {
			case SpectralElement::GAUSSIAN:
    			_nGaussSinglets++;
    			break;
			case SpectralElement::GMULTIPLET:
    			_nGaussMultiplets++;
    			break;
			case SpectralElement::LORENTZIAN:
				_nLorentzSinglets++;
				break;
			case SpectralElement::POWERLOGPOLY:
				if (_nonPolyEstimates.nelements() > 1 || _polyOrder > 0) {
					*_getLog() << "A power logarithmic polynomial cannot be fit simultaneously with other functions"
						<< LogIO::EXCEPTION;
				}
				if (havePLP) {
					*_getLog() << "Fitting of multiple power logarithmic polynomials is not supported."
						<< LogIO::EXCEPTION;
				}
				_nPLPCoeffs = _nonPolyEstimates[i]->get().size();
				havePLP = True;
				break;
			default:
				*_getLog() << "Logic error: Only gaussian singlets, "
					<< "gaussian multiplets, lorentzian singlets, and a single power "
					<< "logarithmic polynomial are "
				    << "permitted in the spectralList input parameter"
				    << LogIO::EXCEPTION;
				break;
			}
    	}

    	if (_nGaussSinglets > 0) {
    		*_getLog() << LogIO::NORMAL << "Number of gaussian singlets to fit found to be "
    			<< _nGaussSinglets << " from provided spectral element list"
    			<< LogIO::POST;
    	}
    	if (_nGaussMultiplets > 0) {
    		*_getLog() << LogIO::NORMAL << "Number of gaussian multiplets to fit found to be "
    			<< _nGaussMultiplets << " from provided spectral element list"
    			<< LogIO::POST;
    	}
    	if (_nLorentzSinglets > 0) {
    		*_getLog() << LogIO::NORMAL << "Number of lorentzian singlets to fit found to be "
    			<< _nLorentzSinglets << " from provided spectral element list"
    			<< LogIO::POST;
    	}
    	if (havePLP) {
    		*_getLog() << LogIO::NORMAL << "Will fit a single power logarithmic polynomial "
    			<< " from provided spectral element list" << LogIO::POST;
    	}
    }
    if (_nonPolyEstimates.nelements() > 0 && ngauss > 0) {
    	*_getLog() << LogIO::WARN << "Estimates specified so ignoring input value of ngauss"
    		<<LogIO::POST;
    }
    _construct();
    _finishConstruction();
}

ImageProfileFitter::~ImageProfileFitter() {}

Record ImageProfileFitter::fit() {

	// do this check here rather than at construction because _polyOrder can be set
	// after construction but before fit() is called
    _checkNGaussAndPolyOrder();
    LogOrigin logOrigin(_class, __FUNCTION__);
    *_getLog() << logOrigin;
    std::auto_ptr<ImageInterface<Float> > originalSigma(0);
    {
    	std::auto_ptr<ImageInterface<Float> > clone(
    		_getImage()->cloneII()
    	);
    	_subImage = SubImageFactory<Float>::createSubImage(
    		*clone, *_getRegion(),
    		_getMask(), 0, False, AxesSpecifier(), _getStretch()
    	);
    	if (_sigma.get()) {
    		if (! _sigmaName.empty()) {
    			originalSigma.reset(_sigma->cloneII());
    		}
    		SubImage<Float> sigmaSubImage = SubImageFactory<Float>::createSubImage(
    			*_sigma, *_getRegion(),
    			_getMask(), 0, False, AxesSpecifier(), _getStretch()
			);
    		_sigma.reset(
    			new TempImage<Float>(
    				sigmaSubImage.shape(), sigmaSubImage.coordinates()
    			)
    		);
    		_sigma->put(sigmaSubImage.get());
    	}
    }
    *_getLog() << logOrigin;
	try {
		if (! _multiFit) {
			ImageCollapser collapser(
				&_subImage, IPosition(1, _fitAxis), True,
				ImageCollapser::MEAN, "", True
			);
			std::auto_ptr<ImageInterface<Float> > x(
				collapser.collapse(True)
			);
			// _subImage needs to be a SubImage<Float> object
			_subImage = SubImageFactory<Float>::createSubImage(
				*x, Record(), "", _getLog().get(),
				False, AxesSpecifier(), False
			);
			if (_sigma.get()) {
				Array<Bool> sigmaMask = _sigma->get() != Array<Float>(_sigma->shape(), 0.0);
				if (anyTrue(! sigmaMask)) {
					if (_sigma->hasPixelMask()) {
						sigmaMask = sigmaMask && _sigma->pixelMask().get();
					}
					else {
						_sigma->makeMask("sigmamask", True, True, False);
					}
					_sigma->pixelMask().put(sigmaMask);
				}
				ImageCollapser collapsedSigma(
					_sigma.get(), IPosition(1, _fitAxis), True,
					ImageCollapser::MEAN, "", True
				);
				_sigma.reset(
					dynamic_cast<TempImage<Float> *>(
						collapsedSigma.collapse(True)
					)
				);
			}
		}
		_fitallprofiles();
	    *_getLog() << logOrigin;
	}
	catch (const AipsError& x) {
		*_getLog() << "Exception during fit: " << x.getMesg()
			<< LogIO::EXCEPTION;
	}

	ImageProfileFitterResults resultHandler(
		_getLog(), _getImage()->coordinates(), &_fitters,
		_nonPolyEstimates, &_subImage, _polyOrder,
		_fitAxis, _nGaussSinglets, _nGaussMultiplets,
		_nLorentzSinglets, _nPLPCoeffs, _logResults,
		_multiFit, _getLogFile(), _xUnit, _summaryHeader()
	);
	resultHandler.setAmpName(_ampName);
	resultHandler.setAmpErrName(_ampErrName);
	resultHandler.setCenterName(_centerName);
	resultHandler.setCenterErrName(_centerErrName);
	resultHandler.setFWHMName(_fwhmName);
	resultHandler.setFWHMErrName(_fwhmErrName);
	resultHandler.setIntegralName(_integralName);
	resultHandler.setIntegralErrName(_integralErrName);
	resultHandler.setPLPName(_plpName);
	resultHandler.setPLPErrName(_plpErrName);
	resultHandler.setOutputSigmaImage(_sigmaName);
	resultHandler.createResults();
	_results = resultHandler.getResults();
    if (originalSigma.get() && ! _sigmaName.empty()) {
    	_removeExistingFileIfNecessary(_sigmaName, True);
    	PagedImage<Float> outputSigma(
    		originalSigma->shape(), originalSigma->coordinates(),
    		_sigmaName
    	);
    	outputSigma.put(originalSigma->get());
    	ImageUtilities::copyMiscellaneous(outputSigma, *originalSigma);
    }
	return _results;
}

void ImageProfileFitter::setGoodAmpRange(const Double min, const Double max) {
	_goodAmpRange.resize(2);
	_goodAmpRange[0] = min < max ? min : max;
	_goodAmpRange[1] = min < max ? max : min;
}

void ImageProfileFitter::setGoodCenterRange(const Double min, const Double max) {
	_goodCenterRange.resize(2);
	_goodCenterRange[0] = min < max ? min : max;
	_goodCenterRange[1] = min < max ? max : min;
}

void ImageProfileFitter::setGoodFWHMRange(const Double min, const Double max) {
	_goodFWHMRange.resize(2);
	_goodFWHMRange[0] = min < max ? min : max;
	_goodFWHMRange[1] = min < max ? max : min;
}

void ImageProfileFitter::setSigma(const Array<Float>& sigma) {
	std::auto_ptr<TempImage<Float> > temp(0);
	if (sigma.ndim() == _getImage()->ndim()) {
		temp.reset(new TempImage<Float>(
			sigma.shape(), _getImage()->coordinates())
		);
	}
	else if (sigma.ndim() == 1) {
		SpectralCoordinate sp;
		CoordinateSystem csys;
		csys.addCoordinate(sp);
		temp.reset(new TempImage<Float>(sigma.shape(), csys));
	}
	temp->put(sigma);
	setSigma(temp.get());
}


void ImageProfileFitter::setSigma(const ImageInterface<Float> *const &sigma) {
	if (anyTrue(sigma->get() < Array<Float>(sigma->shape(), 0.0))) {
		*_getLog() << "All sigma values must be non-negative" << LogIO::EXCEPTION;
	}
	Float mymax = fabs(max(sigma->get()));
	if (
		sigma->ndim() == _getImage()->ndim()
		&& sigma->shape() == _getImage()->shape()
	) {
		ImageInterface<Float> *clone = sigma->cloneII();
		_sigma.reset(dynamic_cast<TempImage<Float> *>(clone));
		if (! _sigma.get()) {
			delete clone;
			std::auto_ptr<ImageInterface<Float> > x(0);
			if (
				ImageAnalysis::makeExternalImage(
					x, "", sigma->coordinates(), sigma->shape(), *sigma, *_getLog(), False, True, True
				)
			) {
				_sigma.reset(dynamic_cast<TempImage<Float> *>(x.release()));
				if (! _sigma.get()) {
					*_getLog() << "Unable to create temporary weights image" << LogIO::EXCEPTION;
				}
			}
		}
		if (mymax != 1) {
			_sigma->put(_sigma->get()/mymax);
		}
	}
	else if (
		sigma->ndim() == _getImage()->ndim()
		|| sigma->ndim() == 1
	) {
		if (sigma->ndim() == _getImage()->ndim()) {
			IPosition expShape(_getImage()->ndim(), 1);
			expShape[_fitAxis] = _getImage()->shape()[_fitAxis];
			if (sigma->shape() != expShape) {
				*_getLog() << "If the shape of the standard deviation image differs from the shape of "
					<< "the input image, the shape of the standard deviation image must be " << expShape
					<< LogIO::EXCEPTION;
			}
		}
		else if (sigma->ndim() == 1) {
			if (sigma->shape()[0] != _getImage()->shape()[_fitAxis]) {
				*_getLog() << "A one dimensional standard deviation spectrum must have the same number "
					<< "of pixels as the input image has along its axis to be fit"
					<< LogIO::EXCEPTION;
			}
		}
		IPosition dataToInsertShape(_getImage()->ndim(), 1);
		dataToInsertShape[_fitAxis] = _getImage()->shape()[_fitAxis];
		_sigma.reset(new TempImage<Float>(_getImage()->shape(), _getImage()->coordinates()));
		Array<Float> dataToInsert(IPosition(1, _getImage()->shape()[_fitAxis]));
		dataToInsert = sigma->get(sigma->ndim() == _getImage()->ndim());
		// normalize
		if (mymax != 1) {
			dataToInsert /= mymax;
		}
		Array<Float> sigmaData = _sigma->get();
		ArrayIterator<Float> iter(sigmaData, IPosition(1, _fitAxis), True);
		while(! iter.pastEnd()) {
			iter.array() = dataToInsert;
			iter.next();
		}
		_sigma->put(sigmaData);
	}
	else {
		*_getLog() << "Illegal shape of standard deviation image" << LogIO::EXCEPTION;
	}
	if (! _sigma->coordinates().near(_getImage()->coordinates())) {
		_sigma->setCoordinateInfo(_getImage()->coordinates());
	}
}

Record ImageProfileFitter::getResults() const {
	return _results;
}

void ImageProfileFitter::setAbscissaDivisor(Double d) {
	if (_nPLPCoeffs == 0) {
		*_getLog() << LogIO::WARN << "This object is not configured to fit a power logarithmic polynomial "
			<< "and so setting the abscissa divisor will have no effect in the fitting process."
			<< LogIO::POST;
	}
	_abscissaDivisor = d;
}


void ImageProfileFitter::_getOutputStruct(
    vector<OutputDestinationChecker::OutputStruct>& outputs
) {
	outputs.resize(0);
    if (! _model.empty()) {
    	OutputDestinationChecker::OutputStruct modelImage;
    	modelImage.label = "model image";
    	modelImage.outputFile = &_model;
    	modelImage.required = True;
    	modelImage.replaceable = False;
    	outputs.push_back(modelImage);
    }
    if (! _residual.empty()) {
    	OutputDestinationChecker::OutputStruct residImage;
    	residImage.label = "residual image";
    	residImage.outputFile = &_residual;
    	residImage.required = True;
    	residImage.replaceable = False;
    	outputs.push_back(residImage);
    }
}

void ImageProfileFitter::_checkNGaussAndPolyOrder() const {
	LogOrigin logOrigin(_class, __FUNCTION__);
	if (
		_polyOrder < 0
		&& (
			_nGaussSinglets + _nGaussMultiplets
			+ _nLorentzSinglets + _nPLPCoeffs
		) == 0
	) {
		*_getLog() << "Number of non-polynomials is 0 and polynomial order is less than zero. "
			<< "According to these inputs there is nothing to fit."
			<< LogIO::EXCEPTION;
	}
}

void ImageProfileFitter::_finishConstruction() {
    LogOrigin logOrigin(_class, __FUNCTION__);

    if (_fitAxis >= (Int)_getImage()->ndim()) {
    	*_getLog() << "Specified fit axis " << _fitAxis
    		<< " must be less than the number of image axes ("
    		<< _getImage()->ndim() << ")" << LogIO::EXCEPTION;
    }
    if (_fitAxis < 0) {
		if (! _getImage()->coordinates().hasSpectralAxis()) {
			_fitAxis = 0;
			*_getLog() << LogIO::WARN << "No spectral coordinate found in image, "
                << "using axis 0 as fit axis" << LogIO::POST;
		}
		else {
            _fitAxis = _getImage()->coordinates().spectralAxisNumber();
			*_getLog() << LogIO::NORMAL << "Using spectral axis (axis " << _fitAxis
				<< ") as fit axis" << LogIO::POST;
		}
	}
    this->_setSupportsLogfile(True);
}

Bool ImageProfileFitter::_inVelocitySpace() const {
	return _fitAxis == _subImage.coordinates().spectralAxisNumber()
		&& Quantity(1, _xUnit).isConform("m/s");
}

const Vector<Double> ImageProfileFitter::getPixelCenter( uint index ) const {
	Vector<Double> pos;
	if ( index < _pixelPositions.size()){
		pos = _pixelPositions[index];
	}
	return pos;
}

Double ImageProfileFitter::getWorldValue(
    double pixelVal, const IPosition& imPos, const String& units,
    bool velocity, bool wavelength ) const {
	Vector<Double> pixel(imPos.size());
	for (uInt i=0; i<pixel.size(); i++) {
		pixel[i] = imPos[i];
	}
	Vector<Double> world(pixel.size());
	// in pixels here
	pixel[_fitAxis] = pixelVal;
	_subImage.coordinates().toWorld(world, pixel);
	SpectralCoordinate spectCoord = _subImage.coordinates().spectralCoordinate();
	Double convertedVal;
	if (velocity) {
		spectCoord.setVelocity( units );
		spectCoord.frequencyToVelocity(convertedVal, world(_fitAxis));
	}
	else if ( wavelength  ) {
		spectCoord.setWavelengthUnit( units );
		Vector<Double> worldVal(1);
		worldVal[0] = world(_fitAxis);
		Vector<Double> waveVal(1);
		spectCoord.frequencyToWavelength(waveVal, worldVal);
		convertedVal = waveVal[0];
	}
	else {
		convertedVal = world(_fitAxis);
	}
	return convertedVal;
}

void ImageProfileFitter::_fitallprofiles() {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	IPosition imageShape = _subImage.shape();
	// Set default axis
	CoordinateSystem cSys = _subImage.coordinates();
	Int pAxis = CoordinateUtil::findSpectralAxis(cSys);
	Int axis2 = _fitAxis;
	if (axis2 < 0) {
		if (pAxis != -1) {
			axis2 = pAxis;
		}
		else {
			axis2 = _subImage.ndim() - 1;
		}
	}
	// Create output images with a mask
	std::auto_ptr<ImageInterface<Float> > fitImage(0), residImage(0);
	if (
		! _model.empty()
		&& ! ImageAnalysis::makeExternalImage(
			fitImage, _model, cSys, imageShape, _subImage,
			*_getLog(), True, False, True
		)
	) {
		*_getLog() << LogIO::WARN << "Failed to create model image" << LogIO::POST;
	}
	if (
		! _residual.empty()
		&& ! ImageAnalysis::makeExternalImage(
			residImage, _residual, cSys, imageShape,
			_subImage, *_getLog(), True, False, True
		)
	) {
		*_getLog() << LogIO::WARN << "Failed to create residual image" << LogIO::POST;
	}
	// Do fits
	// FIXME give users the option to show a progress bar
	Bool showProgress = False;
	if (_nonPolyEstimates.nelements() > 0) {
		String doppler = "";
		ImageUtilities::getUnitAndDoppler(
			_xUnit, doppler, _fitAxis, cSys
		);
	}
	_fitProfiles(
		fitImage, residImage,
		showProgress
	);
}

// moved from ImageUtilities
void ImageProfileFitter::_fitProfiles(
	std::auto_ptr<ImageInterface<Float> >& pFit,
	std::auto_ptr<ImageInterface<Float> >& pResid,
    const Bool showProgress
) {
	IPosition inShape = _subImage.shape();
	if (pFit.get()) {
		AlwaysAssert(inShape.isEqual(pFit->shape()), AipsError);
	}
	if (pResid.get()) {
		AlwaysAssert(inShape.isEqual(pResid->shape()), AipsError);
	}

	// Check axis

	const uInt nDim = _subImage.ndim();

	// Progress Meter

	std::auto_ptr<ProgressMeter> pProgressMeter(0);
	if (showProgress) {
		Double nMin = 0.0;
		Double nMax = 1.0;
		for (uInt i=0; i<inShape.nelements(); i++) {
			if ((Int)i != _fitAxis) {
				nMax *= inShape(i);
			}
		}
		ostringstream oss;
		oss << "Fit profiles on axis " << _fitAxis+1;
		pProgressMeter.reset(
			new ProgressMeter(
				nMin, nMax, String(oss),
				String("Fits"),
				String(""), String(""),
				True, max(1,Int(nMax/20))
			)
		);
	}
	Lattice<Bool>* pFitMask = 0;
	if (pFit.get() && pFit->hasPixelMask() && pFit->pixelMask().isWritable()) {
		pFitMask = &(pFit->pixelMask());
	}
	Lattice<Bool>* pResidMask = 0;
	if (pResid.get() && pResid->hasPixelMask() && pResid->pixelMask().isWritable()) {
		pResidMask = &(pResid->pixelMask());
	}
	//
	IPosition sliceShape(nDim, 1);
	sliceShape(_fitAxis) = inShape(_fitAxis);
	Array<Float> failData(sliceShape);
	failData = NAN;
	Array<Bool> failMask(sliceShape);
	failMask = False;
	Array<Float> resultData(sliceShape);
	Array<Bool> resultMask(sliceShape);

    String doppler = "";
	CoordinateSystem csys = _subImage.coordinates();
	ImageUtilities::getUnitAndDoppler(
		_xUnit, doppler, _fitAxis, csys
	);
	String errMsg;
	ImageFit1D<Float>::AbcissaType abcissaType;
	String abscissaUnits = _nPLPCoeffs > 0 ? "native" : "pix";
	if (
		! ImageFit1D<Float>::setAbcissaState(
			errMsg, abcissaType, csys, abscissaUnits, doppler, _fitAxis
		)
	) {
		*_getLog() << errMsg << LogIO::EXCEPTION;
	}



	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);

	uInt nProfiles = 0;
	uInt nFit = 0;
	IPosition fitterShape = inShape;
	fitterShape[_fitAxis] = 1;
	_fitters.resize(fitterShape);
	Int nPoints = fitterShape.product();
	uInt count = 0;
	vector<IPosition> goodPos(0);
	Bool checkMinPts = _minGoodPoints > 0 && _subImage.isMasked();
	SpectralList newEstimates = _nonPolyEstimates;

	ImageFit1D<Float> fitter = (_sigma.get() == 0)
		? ImageFit1D<Float>(_subImage, _fitAxis)
		: ImageFit1D<Float>(_subImage, *_sigma, _fitAxis);
	Double *divisorPtr = _nPLPCoeffs > 0 && _abscissaDivisor != 1 ? &_abscissaDivisor : 0;
	Bool isSpectral = _fitAxis == csys.spectralAxisNumber();

	// calculate the abscissa values only once if they will not change
	// with position
	Vector<Double> abscissaValues = isSpectral
		? fitter.makeAbscissa(
			abcissaType, True, divisorPtr
		) : Vector<Double>(0);
	Bool abscissaSet = abscissaValues.size() > 0;
	std::auto_ptr<PolynomialSpectralElement> polyEl(0);
	if (_polyOrder >= 0) {
		polyEl.reset(new PolynomialSpectralElement(_polyOrder));
		if (newEstimates.nelements() > 0) {
			newEstimates.add(*polyEl);
		}
	}
	uInt nOrigComps = newEstimates.nelements();
	*_getLog() << LogOrigin(_class, __FUNCTION__);


	for (inIter.reset(); !inIter.atEnd(); inIter++, nProfiles++) {
		if (count % 1000 == 0 && count > 0) {
			*_getLog() << LogIO::NORMAL << "Fitting profile number "
				<< count << " of " << nPoints << LogIO::POST;
		}
		const IPosition& curPos = inIter.position();
		if (checkMinPts) {
			IPosition sliceShape = inShape;
			sliceShape = 1;
			sliceShape[_fitAxis] = inShape[_fitAxis];
			if (
				ntrue(_subImage.getMaskSlice(curPos, sliceShape, True))
				< _minGoodPoints
			) {
				// not enough good points, just use the dummy fitter
				// already in place and go to the next position
				continue;
			}
		}
		fitter.clearList();
		if (abscissaSet) {
			fitter.setAbscissa(abscissaValues);
		}
		if (! fitter.setData (curPos, abcissaType, True, divisorPtr)) {
			*_getLog() << "Unable to set data" << LogIO::EXCEPTION;
		}
		if (_nonPolyEstimates.nelements() == 0) {
			if (! fitter.setGaussianElements (_nGaussSinglets)) {
				*_getLog() << "Unable to set gaussian elements"
					<< LogIO::EXCEPTION;
			}
			if (polyEl.get() != 0) {
				fitter.addElement(*polyEl);
			}
		}
		else {
			// user supplied initial estimates
			if (goodPos.size() > 0) {
				IPosition nearest;
				Int minDist2 = 0;
				for (
					IPosition::const_iterator iter=fitterShape.begin();
					iter!=fitterShape.end(); iter++
				) {
					minDist2 += *iter * *iter;
				}
				for (
					vector<IPosition>::const_reverse_iterator iter=goodPos.rbegin();
					iter != goodPos.rend(); iter++
				) {
					IPosition diff = curPos - *iter;
					Int dist2 = 0;
					Bool larger = False;
					for (
						IPosition::const_iterator ipositer=diff.begin();
						ipositer!=diff.end(); ipositer++
					) {
						dist2 += *ipositer * *ipositer;
						if(dist2 >= minDist2) {
							larger = True;
							break;
						}
					}
					if (
						_fitters(*iter).getList().nelements() == nOrigComps
						&& ! larger
					) {
						minDist2 = dist2;
						nearest = *iter;
						if (minDist2 == 1) {
							// can't get any nearer than this
							break;
						}
					}
				}
				newEstimates = _fitters(nearest).getList();
			}
			fitter.setElements(newEstimates);
		}
		nFit++;
		Bool ok = False;
		try {
			ok = fitter.fit();
			if (ok) {
				_flagFitterIfNecessary(fitter);
				ok = fitter.isValid();
				if (
					ok && _nonPolyEstimates.nelements() > 0
				) {
					goodPos.push_back(curPos);
				}
			}
		}
		catch (const AipsError& x) {
			ok = False;
		}
		_fitters(curPos) = fitter;
		// Evaluate and fill
		if (ok) {
			Array<Bool> resultMask = fitter.getTotalMask().reform(sliceShape);
			if (pFit.get()) {
				Array<Float> resultData = fitter.getFit().reform(sliceShape);
				pFit->putSlice (resultData, curPos);
				if (pFitMask) {
					pFitMask->putSlice(resultMask, curPos);
				}
			}
			if (pResid.get()) {
				Array<Float> resultData = fitter.getResidual().reform(sliceShape);
				pResid->putSlice (resultData, curPos);
				if (pResidMask) pResidMask->putSlice(resultMask, curPos);
			}
		}
		else {
			if (pFit.get()) {
				pFit->putSlice (failData, curPos);
				if (pFitMask) {
					pFitMask->putSlice(failMask, curPos);
				}
			}
			if (pResid.get()) {
				pResid->putSlice (failData, curPos);
				if (pResidMask) {
					pResidMask->putSlice(failMask, curPos);
				}
			}
		}
		if (showProgress) {
			pProgressMeter->update(Double(nProfiles));
		}
		count++;
	}
}

void ImageProfileFitter::_flagFitterIfNecessary(
	ImageFit1D<Float>& fitter
) const {
	if (
		! fitter.converged()
		|| (
			_goodAmpRange.size() == 0 && _goodCenterRange.size() == 0
			&& _goodFWHMRange.size() == 0
		)
	) {
		return;
	}
	SpectralList solutions = fitter.getList(True);
	for (uInt i=0; i<solutions.nelements(); i++) {
		switch (solutions[i]->getType()) {
		case SpectralElement::GAUSSIAN:
		// allow fall through
		case SpectralElement::LORENTZIAN: {
			if (
				! _isPCFSolutionOK(
					dynamic_cast<
						const PCFSpectralElement*
					>(solutions[i])
				)
			) {
				fitter.invalidate();
				return;
			}
			break;
		}
		case SpectralElement::GMULTIPLET: {
			const GaussianMultipletSpectralElement *gm = dynamic_cast<
					const GaussianMultipletSpectralElement*
				>(solutions[i]);
			Vector<GaussianSpectralElement> gse = gm->getGaussians();
			for (uInt j=0; j<gse.size(); j++) {
				if (! _isPCFSolutionOK(&gse[i])) {
					fitter.invalidate();
					return;
				}
			}
			break;
		}
		default:
			continue;
		}
	}
}

Bool ImageProfileFitter::_isPCFSolutionOK(
	const PCFSpectralElement *const &pcf
) const {
	if (_goodAmpRange.size() == 2) {
		Double amp = pcf->getAmpl();
		if (
			amp < _goodAmpRange[0]
			|| amp > _goodAmpRange[1]
			|| fabs(pcf->getAmplErr()/amp) > 100
		) {
			return False;
		}
	}
	if (_goodCenterRange.size() == 2) {
		Double center = pcf->getCenter();
		if (
			center < _goodCenterRange[0]
			|| center > _goodCenterRange[1]
		) {
			return False;
		}
	}
	if (_goodFWHMRange.size() == 2) {
		Double fwhm = pcf->getFWHM();
        if (
			fwhm < _goodFWHMRange[0]
			|| fwhm > _goodFWHMRange[1]
			|| fabs(pcf->getFWHMErr()/fwhm) > 100
		) {
			return False;
		}
	}
	return True;
}

const Array<ImageFit1D<Float> >& ImageProfileFitter::getFitters() const{
	return _fitters;
}

} //# NAMESPACE CASA - END
