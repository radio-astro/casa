
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
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/Combinatorics.h>

#include <imageanalysis/ImageAnalysis/ProfileFitResults.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <imageanalysis/IO/ProfileFitterEstimatesFileParser.h>
#include <imageanalysis/IO/ImageProfileFitterResults.h>

namespace casa {

const String ImageProfileFitter::_class = "ImageProfileFitter";

ImageProfileFitter::ImageProfileFitter(
		const SPCIIF image, const String& region,
	const Record *const &regionPtr,	const String& box,
	const String& chans, const String& stokes,
	const String& mask, const Int axis,
	const uInt ngauss, const String& estimatesFilename,
	const SpectralList& spectralList
) : ImageTask<Float>(
		image, region, regionPtr, box, chans, stokes,
		mask, "", False
	),
	_residual(), _model(), _xUnit(), _centerName(),
	_centerErrName(), _fwhmName(), _fwhmErrName(),
	_ampName(), _ampErrName(), _integralName(),
	_integralErrName(), _plpName(), _plpErrName(), _sigmaName(),
	_abscissaDivisorForDisplay("1"), _multiFit(False),
	_deleteImageOnDestruct(False), _logResults(True), _polyOrder(-1),
	_fitAxis(axis), _nGaussSinglets(ngauss), _nGaussMultiplets(0),
	_nLorentzSinglets(0), _nPLPCoeffs(0), _nLTPCoeffs(0),
	_minGoodPoints(1), _results(Record()),
	_nonPolyEstimates(SpectralList()), _goodAmpRange(Vector<Double>(0)),
	_goodCenterRange(Vector<Double>(0)), _goodFWHMRange(Vector<Double>(0)),
	_sigma(), _abscissaDivisor(1.0) {
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
					*_getLog() << "Only a single power logarithmic polynomial may be fit and it cannot be fit simultaneously with other functions"
						<< LogIO::EXCEPTION;
				}
				_nPLPCoeffs = _nonPolyEstimates[i]->get().size();
				break;
			case SpectralElement::LOGTRANSPOLY:
				if (_nonPolyEstimates.nelements() > 1 || _polyOrder > 0) {
					*_getLog() << "Only a single transformed logarithmic polynomial may be fit and it cannot be fit simultaneously with other functions"
						<< LogIO::EXCEPTION;
				}
				_nLTPCoeffs = _nonPolyEstimates[i]->get().size();
				break;
			default:
				*_getLog() << "Logic error: Only gaussian singlets, "
					<< "gaussian multiplets, and lorentzian singlets, or a single power "
					<< "logarithmic polynomial,  or a single log transformed polynomial are "
				    << "permitted in the spectralList input parameter"
				    << LogIO::EXCEPTION;
				break;
			}

    	}
    	if (ngauss > 0 && ngauss != _nGaussSinglets) {
    		*_getLog() << LogIO::WARN
    			<< "Spectral list supplied and ngauss > 0, ngauss will be ignored "
    			<< "and " << _nGaussSinglets << " Gaussian singlets "
    			<< " as specified in the spectra list will be fit" << LogIO::POST;
    	}

    	if (_nPLPCoeffs  > 0) {
    		*_getLog() << LogIO::NORMAL << "Will fit a single power logarithmic polynomial "
    			<< " from provided spectral element list" << LogIO::POST;
    	}
    	else if (_nLTPCoeffs  > 0) {
    		*_getLog() << LogIO::NORMAL << "Will fit a single logarithmic transformed polynomial "
    			<< " from provided spectral element list" << LogIO::POST;
    	}
    	else {
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
    	}
    }
    _isSpectralIndex = _nPLPCoeffs + _nLTPCoeffs > 0;
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
    	_subImage.reset(
    		new SubImage<Float>(
    			SubImageFactory<Float>::createSubImage(
    				*clone, *_getRegion(), _getMask(), 0,
    				False, AxesSpecifier(), _getStretch()
    			)
    		)
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
			ImageCollapser<Float> collapser(
				_subImage, IPosition(1, _fitAxis), True,
				ImageCollapserData::MEAN, "", True
			);
			SPIIF x(collapser.collapse(True));
			// _subImage needs to be a SubImage<Float> object
			_subImage.reset(new SubImage<Float>(SubImageFactory<Float>::createSubImage(
				*x, Record(), "", _getLog().get(),
				False, AxesSpecifier(), False
			)));
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
				ImageCollapser<Float> collapsedSigma(
					_sigma, IPosition(1, _fitAxis), True,
					ImageCollapserData::MEAN, "", True
				);
				SPIIF collapsed = collapsedSigma.collapse(True);
				std::tr1::shared_ptr<TempImage<Float> >ctmp = std::tr1::dynamic_pointer_cast<TempImage<Float> >(collapsed);
				/*
				TempImage<Float> *ctmp = dynamic_cast<TempImage<Float> *>(
					collapsed.get()
				);
				*/
				ThrowIf(
					! ctmp, "Dynamic cast failed"
				);
				_sigma = ctmp;
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
		_nonPolyEstimates, _subImage, _polyOrder,
		_fitAxis, _nGaussSinglets, _nGaussMultiplets,
		_nLorentzSinglets, _nPLPCoeffs, _nLTPCoeffs, _logResults,
		_multiFit, _getLogFile(), _xUnit, _summaryHeader()
	);
	if (_nPLPCoeffs > 0) {
		resultHandler.setPLPName(_plpName);
		resultHandler.setPLPErrName(_plpErrName);
		resultHandler.setPLPDivisor(_abscissaDivisorForDisplay);
	}
	else if (_nLTPCoeffs > 0) {
		resultHandler.setLTPName(_ltpName);
		resultHandler.setLTPErrName(_ltpErrName);
		resultHandler.setPLPDivisor(_abscissaDivisorForDisplay);
	}
	else {
		resultHandler.setAmpName(_ampName);
		resultHandler.setAmpErrName(_ampErrName);
		resultHandler.setCenterName(_centerName);
		resultHandler.setCenterErrName(_centerErrName);
		resultHandler.setFWHMName(_fwhmName);
		resultHandler.setFWHMErrName(_fwhmErrName);
		resultHandler.setIntegralName(_integralName);
		resultHandler.setIntegralErrName(_integralErrName);
	}
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

void ImageProfileFitter::setPolyOrder(Int p) {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	if (p < 0) {
		*_getLog() << "A polynomial cannot have a negative order" << LogIO::EXCEPTION;
	}
	if (_nPLPCoeffs > 0) {
		*_getLog() << "Cannot simultaneously fit a polynomial and a power logarithmic polynomial."
			<< LogIO::EXCEPTION;
	}
	if (_nLTPCoeffs > 0) {
		*_getLog() << "Cannot simultaneously fit a polynomial and a logarithmic transformed polynomial."
			<< LogIO::EXCEPTION;
	}
    _polyOrder = p;
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
		tr1::shared_ptr<ImageInterface<Float> > clone(sigma->cloneII());
		_sigma = tr1::dynamic_pointer_cast<TempImage<Float> >(clone);
		if (! _sigma) {
			tr1::shared_ptr<ImageInterface<Float> > x = ImageAnalysis::makeExternalImage(
				"", sigma->coordinates(), sigma->shape(),
				*sigma, *_getLog(), False, True, True
			);
			if (x) {
				_sigma = tr1::dynamic_pointer_cast<TempImage<Float> >(x);
				if (! _sigma) {
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
	if (! _isSpectralIndex) {
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::WARN << "This object is not configured to fit a "
			<< "spectral index function, and so setting the abscissa divisor "
			<< "will have no effect in the fitting process." << LogIO::POST;
	}
	_abscissaDivisor = d;
	_abscissaDivisorForDisplay = String::toString(d)
		+ _getImage()->coordinates().worldAxisUnits()[_fitAxis];
}

void ImageProfileFitter::setAbscissaDivisor(const Quantity& q) {
	String fitAxisUnit = _getImage()->coordinates().worldAxisUnits()[_fitAxis];
	if (! q.isConform(fitAxisUnit)) {
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << "Abscissa divisor unit " << q.getUnit() << " is not consistent with fit axis unit."
			<< LogIO::EXCEPTION;
	}
	if (! _isSpectralIndex) {
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::WARN << "This object is not configured to fit a spectral index function "
			<< "and so setting the abscissa divisor will have no effect in the fitting process."
			<< LogIO::POST;
	}
	_abscissaDivisor = q.getValue(fitAxisUnit);
	_abscissaDivisorForDisplay = String::toString(q);
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
	if (
		_polyOrder < 0
		&& (
			_nGaussSinglets + _nGaussMultiplets
			+ _nLorentzSinglets
		) == 0
		&& ! _isSpectralIndex
	) {
		*_getLog() << LogOrigin(_class, __FUNCTION__)
			<< "Number of non-polynomials is 0 and polynomial order is less than zero. "
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
	return _fitAxis == _subImage->coordinates().spectralAxisNumber()
		&& Quantity(1, _xUnit).isConform("m/s");
}

Double ImageProfileFitter::getWorldValue(
    double pixelVal, const IPosition& imPos, const String& units,
    bool velocity, bool wavelength, int tabularIndex, MFrequency::Types freqType ) const {
	Vector<Double> pixel(imPos.size());
	for (uInt i=0; i<pixel.size(); i++) {
		pixel[i] = imPos[i];
	}
	Vector<Double> world(pixel.size());
	// in pixels here
	pixel[_fitAxis] = pixelVal;
	_subImage->coordinates().toWorld(world, pixel);

	SpectralCoordinate spectCoord;
	//Use a tabular index for conversion if one has been specified.
	if ( tabularIndex >= 0 ){
		const CoordinateSystem& cSys = _subImage->coordinates();
		TabularCoordinate tabCoord = cSys.tabularCoordinate( tabularIndex );
		Vector<Double> worlds = tabCoord.worldValues();
		spectCoord = SpectralCoordinate( freqType, worlds );
	}
	//Use the default spectral axis for conversion
	else {
		spectCoord = _subImage->coordinates().spectralCoordinate();
	}
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
	IPosition imageShape = _subImage->shape();
	// Set default axis
	CoordinateSystem cSys = _subImage->coordinates();
	Int pAxis = CoordinateUtil::findSpectralAxis(cSys);
	Int axis2 = _fitAxis;
	if (axis2 < 0) {
		if (pAxis != -1) {
			axis2 = pAxis;
		}
		else {
			axis2 = _subImage->ndim() - 1;
		}
	}
	// Create output images with a mask
	tr1::shared_ptr<ImageInterface<Float> > fitImage, residImage;
	if (
		! _model.empty()
		&& ! (
			fitImage = ImageAnalysis::makeExternalImage(
				_model, cSys, imageShape, *_subImage,
				*_getLog(), True, False, True
			)
		)
	) {
		*_getLog() << LogIO::WARN << "Failed to create model image" << LogIO::POST;
	}
	if (
		! _residual.empty()
		&& ! (
			residImage =  ImageAnalysis::makeExternalImage(
				_residual, cSys, imageShape, *_subImage,
				*_getLog(), True, False, True
			)
		)
	) {
		*_getLog() << LogIO::WARN << "Failed to create residual image" << LogIO::POST;
	}
	// Do fits
	// FIXME give users the option to show a progress bar
	Bool showProgress = True;
	_fitProfiles(
		fitImage, residImage,
		showProgress
	);
}

// moved from ImageUtilities
void ImageProfileFitter::_fitProfiles(
	const tr1::shared_ptr<ImageInterface<Float> > pFit,
	const tr1::shared_ptr<ImageInterface<Float> > pResid,
    const Bool showProgress
) {
	IPosition inShape = _subImage->shape();
	if (pFit.get()) {
		AlwaysAssert(inShape.isEqual(pFit->shape()), AipsError);
	}
	if (pResid.get()) {
		AlwaysAssert(inShape.isEqual(pResid->shape()), AipsError);
	}

	// Check axis

	const uInt nDim = _subImage->ndim();

	// Progress Meter

	std::auto_ptr<ProgressMeter> pProgressMeter(0);

	Lattice<Bool>* pFitMask = 0;
	if (pFit.get() && pFit->hasPixelMask() && pFit->pixelMask().isWritable()) {
		pFitMask = &(pFit->pixelMask());
	}
	Lattice<Bool>* pResidMask = 0;
	if (pResid.get() && pResid->hasPixelMask() && pResid->pixelMask().isWritable()) {
		pResidMask = &(pResid->pixelMask());
	}
	IPosition sliceShape(nDim, 1);
	sliceShape(_fitAxis) = inShape(_fitAxis);
	Array<Float> failData(sliceShape, NAN);
	Array<Bool> failMask(sliceShape, False);
	Array<Float> resultData(sliceShape);
	Array<Bool> resultMask(sliceShape);

    String doppler = "";
	CoordinateSystem csys = _subImage->coordinates();
	if (_isSpectralIndex) {
		_xUnit = csys.spectralCoordinate().worldAxisUnits()[0];
	}
	else {
		ImageUtilities::getUnitAndDoppler(
			_xUnit, doppler, _fitAxis, csys
		);
	}
	String errMsg;
	ImageFit1D<Float>::AbcissaType abcissaType;
	String abscissaUnits = _isSpectralIndex ? "native" : "pix";
	if (
		! ImageFit1D<Float>::setAbcissaState(
			errMsg, abcissaType, csys, abscissaUnits, doppler, _fitAxis
		)
	) {
		*_getLog() << errMsg << LogIO::EXCEPTION;
	}
	IPosition inTileShape = _subImage->niceCursorShape();
	TiledLineStepper stepper (_subImage->shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(*_subImage, stepper);

	uInt nProfiles = 0;
	uInt nFit = 0;
	IPosition fitterShape = inShape;
	fitterShape[_fitAxis] = 1;
	_fitters.resize(fitterShape);

	Int nPoints = fitterShape.product();

	if (showProgress) {
		ostringstream oss;
		oss << "Fit profiles on axis " << _fitAxis+1;
		pProgressMeter.reset(
			new ProgressMeter(
				0, nPoints, oss.str()
			)
		);
	}
	vector<IPosition> goodPos(0);
	Bool checkMinPts = _subImage->isMasked();
	Array<Bool> fitMask;
	if (checkMinPts) {
		fitMask = partialNTrue(_subImage->getMask(False), IPosition(1, _fitAxis)) >= _minGoodPoints;
	}
	SpectralList newEstimates = _nonPolyEstimates;

	ImageFit1D<Float> fitter = (! _sigma)
		? ImageFit1D<Float>(*_subImage, _fitAxis)
		: ImageFit1D<Float>(*_subImage, *_sigma, _fitAxis);

	Bool isSpectral = _fitAxis == csys.spectralAxisNumber();

	// calculate the abscissa values only once if they will not change
	// with position
	Double *divisorPtr = 0;

	Vector<Double> abscissaValues(0);
	if (isSpectral) {
		abscissaValues = fitter.makeAbscissa(
			abcissaType, True, 0
		);
		if (_isSpectralIndex) {
			_setAbscissaDivisorIfNecessary(abscissaValues);
			if (_abscissaDivisor != 1) {
				divisorPtr = &_abscissaDivisor;
				abscissaValues /= _abscissaDivisor;
				if (_nLTPCoeffs > 0) {
					abscissaValues = log(abscissaValues);
				}
			}
		}
	}
	Bool abscissaSet = abscissaValues.size() > 0;

	std::auto_ptr<PolynomialSpectralElement> polyEl(0);
	if (_polyOrder >= 0) {
		polyEl.reset(new PolynomialSpectralElement(_polyOrder));
		if (newEstimates.nelements() > 0) {
			newEstimates.add(*polyEl);
		}
	}
	Array<Double> (*xfunc)(const Array<Double>&) = 0;
	Array<Double> (*yfunc)(const Array<Double>&) = 0;
	if (_nLTPCoeffs > 0) {
		if (! abscissaSet) {
			xfunc = casa::log;
		}
		yfunc = casa::log;
	}
	uInt nOrigComps = newEstimates.nelements();
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	uInt mark = (uInt)max(1000.0, std::pow(10.0, log10(nPoints/100)));
	for (inIter.reset(); !inIter.atEnd(); inIter++, nProfiles++) {
		if (nProfiles % mark == 0 && nProfiles > 0 && showProgress) {
			pProgressMeter->update(Double(nProfiles));
		}
		const IPosition& curPos = inIter.position();
		if (checkMinPts && ! fitMask(curPos)) {
			continue;
		}
		fitter.clearList();
		if (abscissaSet) {
			fitter.setAbscissa(abscissaValues);
			abscissaSet = False;
		}

		if (
			! fitter.setData(
				curPos, abcissaType, True, divisorPtr, xfunc, yfunc
			)
		) {
			*_getLog() << "Unable to set data" << LogIO::EXCEPTION;
		}
		_setFitterElements(
			fitter, newEstimates, polyEl, goodPos,
			fitterShape, curPos, nOrigComps
		);

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
		_fitters(curPos).reset(new ProfileFitResults(fitter));
		// Evaluate and fill
		if (pFit || pResid) {
			_updateModelAndResidual(
				pFit, pResid, ok, fitter, sliceShape, curPos,
				pFitMask, pResidMask, failData, failMask
			);
		}
	}
}

void ImageProfileFitter::_updateModelAndResidual(
    tr1::shared_ptr<ImageInterface<Float> > pFit,
    tr1::shared_ptr<ImageInterface<Float> > pResid,
    Bool fitOK,
    const ImageFit1D<Float>& fitter, const IPosition& sliceShape,
    const IPosition& curPos, Lattice<Bool>* const &pFitMask,
    Lattice<Bool>* const &pResidMask, const Array<Float>& failData,
    const Array<Bool>& failMask
) const {
	Array<Bool> resultMask = fitter.getTotalMask().reform(sliceShape);
    if (fitOK) {
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
		    if (pResidMask) {
                pResidMask->putSlice(resultMask, curPos);
            }
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
}

void ImageProfileFitter::_setFitterElements(
	ImageFit1D<Float>& fitter, SpectralList& newEstimates,
	const std::auto_ptr<PolynomialSpectralElement>& polyEl,
	const std::vector<IPosition>& goodPos,
	const IPosition& fitterShape, const IPosition& curPos,
	uInt nOrigComps


) const {
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
					_fitters(*iter)->getList().nelements() == nOrigComps
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
			newEstimates = _fitters(nearest)->getList();
		}
		fitter.setElements(newEstimates);
	}
}

void ImageProfileFitter::_setAbscissaDivisorIfNecessary(
	const Vector<Double>& abscissaValues
) {
	if (_abscissaDivisor == 0) {
		setAbscissaDivisor(1);
		if (abscissaValues.size() > 0) {
			Double minAbs = min(abs(abscissaValues));
			Double maxAbs = max(abs(abscissaValues));
			Double l = (Int)log10(sqrt(minAbs*maxAbs));
			Double p = std::pow(10.0, l);
			setAbscissaDivisor(p);
		}
	}
	if (_abscissaDivisor != 1) {
		*_getLog() << LogIO::NORMAL << "Dividing abscissa values by "
			<< _abscissaDivisor << " before fitting" << LogIO::POST;
	}
}

void ImageProfileFitter::_flagFitterIfNecessary(
	ImageFit1D<Float>& fitter
) const {
	if (! fitter.converged()) {
		return;
	}
	Bool checkComps = _goodAmpRange.size() > 0 || _goodCenterRange.size() > 0
		|| _goodFWHMRange.size() > 0;
	SpectralList solutions = fitter.getList(True);
	for (uInt i=0; i<solutions.nelements(); i++) {
		if (
			anyTrue(isNaN(solutions[i]->get()))
			|| anyTrue(isNaN(solutions[i]->getError()))
		) {
			fitter.invalidate();
			return;
		}
		if (checkComps) {
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

const Array<std::tr1::shared_ptr<ProfileFitResults> >& ImageProfileFitter::getFitters() const{
	return _fitters;
}

} //# NAMESPACE CASA - END
