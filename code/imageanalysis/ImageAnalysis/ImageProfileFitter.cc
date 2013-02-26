
//# tSubImage.cc: Test program for class SubImage
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
#include <casa/Utilities/Precision.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/Combinatorics.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <imageanalysis/IO/ProfileFitterEstimatesFileParser.h>

#include <memory>

namespace casa {

const String ImageProfileFitter::_class = "ImageProfileFitter";
const String ImageProfileFitter::_CONVERGED = "converged";
const String ImageProfileFitter::_SUCCEEDED = "succeeded";
const String ImageProfileFitter::_VALID = "valid";

const uInt ImageProfileFitter::_nOthers = 2;
const uInt ImageProfileFitter::_gsPlane = 0;
const uInt ImageProfileFitter::_lsPlane = 1;

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
	_residual(""), _model(""), _xUnit(""), _centerName(""),
	_centerErrName(""), _fwhmName(""), _fwhmErrName(""),
	_ampName(""), _ampErrName(""), _integralName(""),
	_integralErrName(""), _sigmaName(""),_multiFit(False),
	_deleteImageOnDestruct(False), _logResults(True), _polyOrder(-1),
	_fitAxis(axis), _nGaussSinglets(ngauss), _nGaussMultiplets(0),
	_nLorentzSinglets(0), _minGoodPoints(0), _results(Record()),
	_nonPolyEstimates(SpectralList()), _goodAmpRange(Vector<Double>(0)),
	_goodCenterRange(Vector<Double>(0)), _goodFWHMRange(Vector<Double>(0)),
	_sigma(0) {
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
			default:
				*_getLog() << "Logic error: Only gaussian singlets, "
					<< "gaussian multiplets, and lorentzian singlets are "
				    << "permitted in the spectralList input parameter"
				    << LogIO::EXCEPTION;
			}
    	}

    	*_getLog() << LogIO::NORMAL << "Number of gaussian singlets to fit found to be "
    			<< _nGaussSinglets << " from provided spectral element list"
    			<< LogIO::POST;

    	*_getLog() << LogIO::NORMAL << "Number of gaussian multiplets to fit found to be "
    			<< _nGaussMultiplets << " from provided spectral element list"
    			<< LogIO::POST;

    	*_getLog() << LogIO::NORMAL << "Number of lorentzian singlets to fit found to be "
    			<< _nLorentzSinglets << " from provided spectral element list"
    			<< LogIO::POST;
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
	catch (AipsError exc) {
		*_getLog() << "Exception during fit: " << exc.getMesg()
			<< LogIO::EXCEPTION;
	}
	_setResults();
    *_getLog() << logOrigin;
    if (_logResults || ! _getLogfile().empty()) {
    	_resultsToLog();
    }
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

void ImageProfileFitter::_getOutputStruct(
    vector<ImageInputProcessor::OutputStruct>& outputs
) {
	outputs.resize(0);
    if (! _model.empty()) {
    	ImageInputProcessor::OutputStruct modelImage;
    	modelImage.label = "model image";
    	modelImage.outputFile = &_model;
    	modelImage.required = True;
    	modelImage.replaceable = False;
    	outputs.push_back(modelImage);
    }
    if (! _residual.empty()) {
    	ImageInputProcessor::OutputStruct residImage;
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
			+ _nLorentzSinglets
		) == 0
	) {
		*_getLog() << "Number of gaussians is 0 and polynomial order is less than zero. "
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

void ImageProfileFitter::_setResults() {
    LogOrigin logOrigin(_class, __FUNCTION__);
    Double fNAN = casa::doubleNaN();
    uInt nComps = _nGaussSinglets + _nGaussMultiplets + _nLorentzSinglets;
    if (_polyOrder >= 0) {
    	nComps++;
    }
	Array<Bool> attemptedArr(IPosition(1, _fitters.size()), False);
	Array<Bool> successArr(IPosition(1, _fitters.size()), False);
	Array<Bool> convergedArr(IPosition(1, _fitters.size()), False);
	Array<Bool> validArr(IPosition(1, _fitters.size()), False);

	Array<Int> niterArr(IPosition(1, _fitters.size()), -1);
	vector<vector<Matrix<Double> > > pcfMatrices(
		NGSOLMATRICES, vector<Matrix<Double> >(_nGaussMultiplets+_nOthers)
	);
	uInt compCount = 0;
	Matrix<Double> blank;
	uInt nSubcomps = 0;

	for (uInt i=0; i<_nGaussMultiplets + _nOthers; i++) {
		if (i == _gsPlane) {
			// gaussian singlets go in position 0
			nSubcomps = _nGaussSinglets;
		}
		else if (i == _lsPlane) {
			// lorentzian singlets go in position 1
			nSubcomps = _nLorentzSinglets;
		}
		else {
			// gaussian multiplets go in positions 2 to 1 + _nGaussianMultiplets
			while (
				_nonPolyEstimates[compCount]->getType() != SpectralElement::GMULTIPLET
			) {
				compCount++;
			}
			nSubcomps = dynamic_cast<GaussianMultipletSpectralElement*>(
					_nonPolyEstimates[compCount]
				)->getGaussians().size();
			compCount++;
		}
		blank.resize(_fitters.size(), nSubcomps, False);
		blank = fNAN;
		for (uInt k=0; k<NGSOLMATRICES; k++) {
			pcfMatrices[k][i] = blank;
		}
	}
	Matrix<String> typeMat(_fitters.size(), nComps, "UNDEF");
	Array<Bool> mask(IPosition(1, _fitters.size()), False);
	Array<Int> nCompArr(IPosition(1, _fitters.size()), -1);
	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters.begin();
	uInt count = 0;
	const CoordinateSystem subimCsys = _subImage.coordinates();
	Vector<Double> world;
	Double increment = fabs(_fitAxisIncrement());
	for (
		inIter.reset();
		! inIter.atEnd() && fitter != _fitters.end();
		inIter++, fitter++, count++
	) {
		IPosition idx(1, count);
		attemptedArr(idx) = fitter->getList().nelements() > 0;
		successArr(idx) = fitter->succeeded();
		convergedArr(idx) = attemptedArr(idx) && successArr(idx)
			&& fitter->converged();
		validArr(idx) = convergedArr(idx) && fitter->isValid();
		if (validArr(idx)) {
			IPosition subimPos = inIter.position();
			mask(idx) = anyTrue(inIter.getMask());
			niterArr(idx) = (Int)fitter->getNumberIterations();
			nCompArr(idx) = (Int)fitter->getList().nelements();
			SpectralList solutions = fitter->getList();
			uInt gCount = 0;
			uInt gmCount = 0;
			uInt lseCount = 0;
			for (uInt i=0; i<solutions.nelements(); i++) {
				SpectralElement::Types type = solutions[i]->getType();
				typeMat(count, i) = SpectralElement::fromType(type);
				switch (type) {
				case SpectralElement::POLYNOMIAL:
					break;
				case SpectralElement::GAUSSIAN:
					// allow fall through because gaussians and lorentzians use common code
				case SpectralElement::LORENTZIAN:
					{
						const PCFSpectralElement *pcf = dynamic_cast<
							const PCFSpectralElement*
						>(solutions[i]);
						uInt plane = _lsPlane;
						uInt col = lseCount;
						// if block because we allow case fall through
						if (type == SpectralElement::LORENTZIAN) {
							lseCount++;
						}
						else {
							plane = _gsPlane;
							col = gCount;
							gCount++;
						}
						_insertPCF(
							pcfMatrices, plane, *pcf, count, col,
							subimPos, increment
						);
					}
					break;
				case SpectralElement::GMULTIPLET:
					{
						const GaussianMultipletSpectralElement *gm = dynamic_cast<
							const GaussianMultipletSpectralElement*
						>(solutions[i]);
						const Vector<GaussianSpectralElement> g = gm->getGaussians();
						for (uInt k=0; k<g.size(); k++) {
							_insertPCF(
								pcfMatrices, gmCount + 2, g[k], count,
								k, subimPos, increment
							);
						}
						gmCount++;
					}
					break;

				default:
					*_getLog() << "Logic Error: Unhandled Spectral Element type"
						<< LogIO::EXCEPTION;
					break;
				}
			}
		}
	}
	{
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		ostringstream oss;
		oss << "Number of profiles       = " << _fitters.size();
		String str = oss.str();
		*_getLog() << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", True, False);
		oss.str("");
		oss << "Number of fits attempted = " << ntrue(attemptedArr);
		str = oss.str();
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number succeeded         = " << ntrue(successArr);
		str = oss.str();
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number converged         = " << ntrue(convergedArr);
		str = oss.str();
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number valid             = " << ntrue(validArr) << endl;
		str = oss.str();
		*_getLog() << LogOrigin(_class, __FUNCTION__);
		*_getLog() << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
	}
	Bool someConverged = anyTrue(convergedArr);
	String key;
	IPosition axes(1, _fitAxis);
	ImageCollapser collapser(
		&_subImage, axes, False, ImageCollapser::ZERO, String(""), False
	);
	std::auto_ptr<TempImage<Float> > myTemplate(
		static_cast<TempImage<Float>* >(collapser.collapse(True))
	);
	IPosition shape = myTemplate->shape();
	_results.define("attempted", attemptedArr.reform(shape));
	_results.define(_SUCCEEDED, successArr.reform(shape));
	_results.define(_CONVERGED, convergedArr.reform(shape));
	_results.define(_VALID, validArr.reform(shape));
	_results.define("niter", niterArr.reform(shape));
	_results.define("ncomps", nCompArr.reform(shape));
	_results.define("xUnit", _xUnit);
	const String yUnit = _getImage()->units().getName();
	_results.define("yUnit", yUnit);
	IPosition typeShape = shape;
	typeShape.resize(shape.size() + 1, True);
	typeShape[typeShape.size() - 1] = nComps;
	_results.define( "type", typeMat.reform(typeShape));
	for (uInt i=0; i<_nGaussMultiplets+_nOthers; i++) {
		if (i == _gsPlane && _nGaussSinglets == 0) {
			continue;
		}
		else if (i == _lsPlane && _nLorentzSinglets == 0) {
			continue;
		}
		Record rec;
		IPosition solArrShape = shape;
		solArrShape.resize(shape.size()+1, True);
		solArrShape[solArrShape.size()-1] = pcfMatrices[AMP][i].shape()[pcfMatrices[AMP][i].shape().size()-1];
		rec.define("center", pcfMatrices[CENTER][i].reform(solArrShape));
		rec.define("fwhm", pcfMatrices[FWHM][i].reform(solArrShape));
		rec.define("amp", pcfMatrices[AMP][i].reform(solArrShape));
		rec.define("integral", pcfMatrices[INTEGRAL][i].reform(solArrShape));
		rec.define("centerErr", pcfMatrices[CENTERERR][i].reform(solArrShape));
		rec.define("fwhmErr", pcfMatrices[FWHMERR][i].reform(solArrShape));
		rec.define("ampErr", pcfMatrices[AMPERR][i].reform(solArrShape));
		rec.define("integralErr", pcfMatrices[INTEGRALERR][i].reform(solArrShape));
		String description = i == _gsPlane
			? "Gaussian singlets results"
			: i == _lsPlane
			  ? "Lorentzian singlets"
			  : "Gaussian multiplet number " + String::toString(i-1) + " results";
		rec.define("description", description);
		_results.defineRecord(_getTag(i), rec);
	}
	Bool writeSolutionImages = (
		! _centerName.empty() || ! _centerErrName.empty()
		|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
		|| ! _ampName.empty() || ! _ampErrName.empty()
		|| ! _integralName.empty() || ! _integralErrName.empty()
	);
	if (
		! _multiFit && writeSolutionImages
	) {
		*_getLog() << LogIO::WARN << "This was not a multi-pixel fit request so solution "
			<< "images will not be written" << LogIO::POST;
	}
	if (
		_multiFit && writeSolutionImages
	) {
		if (
			_nGaussSinglets + _nGaussMultiplets + _nLorentzSinglets == 0
		) {
			*_getLog() << LogIO::WARN << "No gaussians or lorentzians were fit "
				<< "so no solution images will be written" << LogIO::POST;
		}
		else {
			if (someConverged) {
				_writeImages(myTemplate->coordinates(), mask, yUnit);
			}
			else {
				*_getLog() << LogIO::WARN << "No solutions converged, solution images will not be written"
					<< LogIO::POST;
			}
		}
	}
}



String ImageProfileFitter::_getTag(const uInt i) const {
	return i == _gsPlane
		? "gs"
		: i == _lsPlane
		    ? "ls"
		    : "gm" + String::toString(i-2);
}

void ImageProfileFitter::_insertPCF(
	vector<vector<Matrix<Double> > >& pcfMatrices, /*Bool& isSolutionSane,*/
	const uInt idx, const PCFSpectralElement& pcf,
	const uInt row, const uInt col, const IPosition& pos,
	const Double increment/*, const uInt npix*/
) const {
	pcfMatrices[CENTER][idx](row, col) = _centerWorld(pcf, pos);
	pcfMatrices[FWHM][idx](row, col) = pcf.getFWHM() * increment;
	pcfMatrices[AMP][idx](row, col) = pcf.getAmpl();
	pcfMatrices[CENTERERR][idx](row, col) = pcf.getCenterErr() * increment;
	pcfMatrices[FWHMERR][idx](row, col) = pcf.getFWHMErr() * increment;
	pcfMatrices[AMPERR][idx](row, col) = pcf.getAmplErr();
	pcfMatrices[INTEGRAL][idx](row, col) = pcf.getIntegral() * increment;
	pcfMatrices[INTEGRALERR][idx](row, col) = pcf.getIntegralErr() * increment;
}

void ImageProfileFitter::_writeImages(
	const CoordinateSystem& xcsys,
	const Array<Bool>& mask, const String& yUnit
) const {
	// add a linear coordinate for the individual components
	Vector<Double> crpix(1, 0);
	Vector<Double> crval(1, 0);
	Vector<Double> cdelt(1, 1);
	Matrix<Double> pc(1, 1, 0);
	pc.diagonal() = 1.0;
	Vector<String> name(1, "Component Number");
	Vector<String> units(1, "");
	LinearCoordinate componentCoord(name, units, crval, cdelt, pc, crpix);
	CoordinateSystem mycsys = CoordinateSystem(xcsys);

	mycsys.addCoordinate(componentCoord);

	map<String, String> mymap;
	map<String, String> unitmap;
	mymap["center"] = _centerName;
	mymap["centerErr"] = _centerErrName;
	mymap["fwhm"] = _fwhmName;
	mymap["fwhmErr"] = _fwhmErrName;
	mymap["amp"] = _ampName;
	mymap["ampErr"] = _ampErrName;
	mymap["integral"] = _integralName;
	mymap["integralErr"] = _integralErrName;
	mymap["center"] = _centerName;

	unitmap["center"] = _xUnit;
	unitmap["centerErr"] = _xUnit;
	unitmap["fwhm"] = _xUnit;
	unitmap["fwhmErr"] = _xUnit;
	unitmap["amp"] = yUnit;
	unitmap["ampErr"] = yUnit;
	unitmap["integral"] = _xUnit + "." + yUnit;
	unitmap["integralErr"] = _xUnit + "." + yUnit;
	for (uInt i=0; i<_nGaussMultiplets+_nOthers; i++) {
		if (i == _gsPlane && _nGaussSinglets == 0) {
			continue;
		}
		else if (i == _lsPlane && _nLorentzSinglets == 0) {
			continue;
		}
		String id = _getTag(i);
		IPosition maskShape = _results.asRecord(id).asArrayDouble("amp").shape();
		Array<Bool> fMask(maskShape);
		uInt n = maskShape[maskShape.size()-1];
		maskShape[maskShape.size()-1] = 1;
		Array<Bool> reshapedMask = mask.reform(maskShape);
		AlwaysAssert(ntrue(mask) == ntrue(reshapedMask), AipsError);

		IPosition shape = fMask.shape();
		IPosition begin(shape.size(), 0);
		IPosition end = shape - 1;
		for (uInt j=0; j<n; j++) {
			begin[shape.size() - 1] = j;
			end[shape.size() - 1] = j;
			fMask(begin, end) = reshapedMask;
		}

		for (
			map<String, String>::const_iterator iter=mymap.begin();
			iter!=mymap.end(); iter++
		) {
			String imagename = iter->second;
			String suffix = i == _gsPlane
				? ""
				: i == _lsPlane
				  ? "_ls"
				  : _nGaussMultiplets <= 1
				    ? "_gm"
				    : "_gm" + String::toString(i-_nOthers);
			imagename += suffix;
			if (! iter->second.empty()) {
				_makeSolutionImage(
					imagename, mycsys,
					_results.asRecord(id).asArrayDouble(iter->first),
					unitmap.find(iter->first)->second, fMask
				);
			}
		}
	}
}

Bool ImageProfileFitter::_inVelocitySpace() const {
	return _fitAxis == _subImage.coordinates().spectralAxisNumber()
		&& Quantity(1, _xUnit).isConform("m/s");
}

Double ImageProfileFitter::_fitAxisIncrement() const {
	if (_inVelocitySpace()) {
		Vector<Double> pixels(2);
		pixels[0] = 0;
		pixels[1] = 1;
		Vector<Double> velocities(2);
		_subImage.coordinates().spectralCoordinate().pixelToVelocity(
			velocities, pixels
		);
		return velocities[1] - velocities[0];
	}
	else {
		return _subImage.coordinates().increment()[_fitAxis];
	}
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
	else if ( wavelength  ){
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


Double ImageProfileFitter::_centerWorld(
    const PCFSpectralElement& solution, const IPosition& imPos
) const {
	Vector<Double> pixel(imPos.size());
	for (uInt i=0; i<pixel.size(); i++) {
		pixel[i] = imPos[i];
	}
	Vector<Double> world(pixel.size());
	// in pixels here
	pixel[_fitAxis] = solution.getCenter();
	_subImage.coordinates().toWorld(world, pixel);
	if (_inVelocitySpace()) {
		Double velocity;
		_subImage.coordinates().spectralCoordinate().frequencyToVelocity(velocity, world(_fitAxis));
		return velocity;
	}
	else {
		return world(_fitAxis);
	}
}

String ImageProfileFitter::_radToRa(Float ras) const {
	Int h, m;
	Float rah = ras * 12 / C::pi;
	h = (int)floor(rah);
	Float ram = (rah - h) * 60;
	m = (int)floor(ram);
	ras = (ram - m) * 60;
	ras = (int)(1000 * ras) / 1000.;
	String raStr = (h < 10) ? "0" : "";
	raStr.append(String::toString(h)).append(String(":"))
        .append(String((m < 10) ? "0" : ""))
        .append(String::toString(m)).append(String(":")) 
        .append(String((ras < 10) ? "0" : ""))
        .append(String::toString(ras));
	return raStr;
}

void ImageProfileFitter::_resultsToLog() {
	ostringstream summary;
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << _summaryHeader();
	if (_goodAmpRange.size() == 2) {
		summary << "       --- valid amplitude range: " << _goodAmpRange << endl;
	}
	if (_goodCenterRange.size() == 2) {
		summary << "       --- valid center range: " << _goodCenterRange << endl;
	}
	if (_goodFWHMRange.size() == 2) {
		summary << "       --- valid FWHM range: " << _goodFWHMRange << endl;
	}
	summary << "       --- number of Gaussian singlets: " << _nGaussSinglets << endl;
	summary << "       --- number of Gaussian multiplets: " << _nGaussMultiplets << endl;
	if (_nGaussMultiplets > 0) {
		for (uInt i=0; i<_nGaussMultiplets; i++) {
			Array<Double> amp = _results.asRecord("gm" + String::toString(i)).asArrayDouble(AMP);
			uInt n = amp.shape()[amp.ndim()-1];
			summary << "           --- number of components in Gaussian multiplet "
				<< i << ": " << n << endl;
		}
	}
	if (_polyOrder >= 0) {
		summary << "       --- polynomial order:    " << _polyOrder << endl;
	}
	else {
		summary << "       --- no polynomial fit " << endl;
	}

	if (_multiFit) {
		summary << "       --- Multiple profiles fit, one per pixel over selected region" << endl;
	}
	else {
		summary << "       --- One profile fit, averaged over several pixels if necessary" << endl;
	}
	if (_logResults) {
		*_getLog() << LogIO::NORMAL << summary.str() << LogIO::POST;
	}
	if (! _getLogfile().empty()) {
		_writeLogfile(summary.str(), False, False);
	}
	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	CoordinateSystem csysSub = _subImage.coordinates();
	CoordinateSystem csys = _getImage()->coordinates();
	Vector<Double> worldStart;
	if (! csysSub.toWorld(worldStart, inIter.position())) {
		*_getLog() << csysSub.errorMessage() << LogIO::EXCEPTION;
	}
	CoordinateSystem csysIm = _getImage()->coordinates();
	Vector<Double> pixStart;
	if (! csysIm.toPixel(pixStart, worldStart)) {
		*_getLog() << csysIm.errorMessage() << LogIO::EXCEPTION;
	}
	if (_multiFit) {
		for (uInt i=0; i<pixStart.size(); i++) {
			pixStart[i] = (Int)std::floor( pixStart[i] + 0.5);
		}
	}
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters.begin();
	Vector<String> axesNames = csysSub.worldAxisNames();
	Vector<Double> imPix(pixStart.size());
	Vector<Double> world;
	IPosition subimPos;
	SpectralList solutions;
	String axisUnit = csysIm.worldAxisUnits()[_fitAxis];
	Int pixPrecision = _multiFit ? 0 : 3;

	for (
		Vector<String>::iterator iter = axesNames.begin();
		iter != axesNames.end(); iter++
	) {
		iter->upcase();
	}
	_pixelPositions.resize( _fitters.size());
	uint index = 0;
	for (
		inIter.reset();
		! inIter.atEnd() && fitter != _fitters.end();
		inIter++, fitter++
	) {
		summary.str("");
		Int basePrecision = summary.precision(1);
		summary.precision(basePrecision);
		subimPos = inIter.position();
		if (csysSub.toWorld(world, subimPos)) {
			summary << "Fit   :" << endl;
			for (uInt i=0; i<world.size(); i++) {
				if ((Int)i != _fitAxis) {
					if (axesNames[i].startsWith("RIG")) {
						// right ascension
						summary << "    RA           :   "
							<< _radToRa(world[i]) << endl;
					}
					else if (axesNames[i].startsWith("DEC")) {
						// declination
						summary << "    Dec          : "
							<< MVAngle(world[i]).string(MVAngle::ANGLE_CLEAN, 8) << endl;
					}
					else if (axesNames[i].startsWith("FREQ")) {
						// frequency
						summary << "    Freq         : "
							<< world[i]
							<< csysSub.spectralCoordinate().formatUnit() << endl;
					}
					else if (axesNames[i].startsWith("STO")) {
						// stokes
						summary << "    Stokes       : "
							<< Stokes::name(Stokes::type((Int)world[i])) << endl;
					}
				}
			}
		}
		else {
			*_getLog() << csysSub.errorMessage() << LogIO::EXCEPTION;
		}
		for (uInt i=0; i<pixStart.size(); i++) {
			imPix[i] = pixStart[i] + subimPos[i];
		}
		_pixelPositions[index] = imPix;
		index++;
		summary.setf(ios::fixed);
		summary << setprecision(pixPrecision) << "    Pixel        : [";
		for (uInt i=0; i<imPix.size(); i++) {
			if (i == (uInt)_fitAxis) {
				summary << " *";
			}
			else {
				summary << imPix[i];
			}
			if (i != imPix.size()-1) {
				summary << ", ";
			}
		}
		summary << "]" << setprecision(basePrecision) << endl;
		summary.unsetf(ios::fixed);
		Bool attempted = fitter->getList().nelements() > 0;
		summary << "    Attempted    : "
			<< (attempted ? "YES" : "NO") << endl;
		if (attempted) {
			String converged = fitter->converged() ? "YES" : "NO";
			summary << "    Converged    : " << converged << endl;
			if (fitter->converged()) {
				summary << "    Iterations   : " << fitter->getNumberIterations() << endl;
				String valid = fitter->isValid() ? "YES" : "NO";
				summary << "    Valid        : " << valid << endl;
				if (fitter->isValid()) {
					solutions = fitter->getList();
					for (uInt i=0; i<solutions.nelements(); i++) {
						SpectralElement::Types type = solutions[i]->getType();
						summary << "    Results for component " << i << ":" << endl;
						switch(type) {
						case SpectralElement::GAUSSIAN:
							// allow fall through; gaussians and lorentzians use the same
							// method for output
						case SpectralElement::LORENTZIAN:
							{
								const PCFSpectralElement *pcf
									= dynamic_cast<const PCFSpectralElement*>(solutions[i]);
								summary << _pcfToString(
									pcf, csys, world.copy(), subimPos
								);
							}
							break;
						case SpectralElement::GMULTIPLET:
							{
								const GaussianMultipletSpectralElement *gm
									= dynamic_cast<const GaussianMultipletSpectralElement*>(solutions[i]);
								summary << _gaussianMultipletToString(
									*gm, csys, world.copy(), subimPos
								);
								break;
							}
						case SpectralElement::POLYNOMIAL:
							{
								const PolynomialSpectralElement *p
									= dynamic_cast<const PolynomialSpectralElement*>(solutions[i]);
								summary << _polynomialToString(*p, csys, imPix, world);
							}
							break;
						default:
							*_getLog() << "Logic Error: Unhandled spectral element type"
							    << LogIO::EXCEPTION;
						}
					}
				}
			}
		}
    	if (_logResults) {
    		*_getLog() << LogIO::NORMAL << summary.str() << endl << LogIO::POST;
    	}
    	if (! _getLogfile().empty()) {
    		_writeLogfile(summary.str(), False, False);
    	}
	}
	_closeLogfile();
}

String ImageProfileFitter::_elementToString(
	const Double value, const Double error,
	const String& unit
) const {
	Unit myUnit(unit);
	Vector<String> unitPrefix;
	String outUnit;
	Quantity qVal(value, unit);
	Quantity qErr(error, unit);

	if (myUnit.getValue() == UnitVal::ANGLE) {
		Vector<String> angUnits(5);
		angUnits[0] = "deg";
		angUnits[1] = "arcmin";
		angUnits[2] = "arcsec";
		angUnits[3] = "marcsec";
		angUnits[4] = "uarcsec";
	    for (uInt i=0; i<angUnits.size(); i++) {
	    	outUnit = angUnits[i];
	    	if (fabs(qVal.getValue(outUnit)) > 1) {
	    		qVal.convert(outUnit);
	    		qErr.convert(outUnit);
	    		break;
	    	}
	    }
	}
	else if (unit.empty() || Quantity(1, myUnit).isConform(Quantity(1, "m/s"))) {
		// do nothing
	}
    else {
		Vector<String> unitPrefix(10);
		unitPrefix[0] = "T";
		unitPrefix[1] = "G";
		unitPrefix[2] = "M";
		unitPrefix[3] = "k";
		unitPrefix[4] = "";
		unitPrefix[5] = "m";
		unitPrefix[6] = "u";
		unitPrefix[7] = "n";
		unitPrefix[8] = "p";
		unitPrefix[9] = "f";

		for (uInt i=0; i<unitPrefix.size(); i++) {
			outUnit = unitPrefix[i] + unit;
			if (fabs(qVal.getValue(outUnit)) > 1) {
				qVal.convert(outUnit);
				qErr.convert(outUnit);
				break;
			}
		}
	}
    Vector<Double> valErr(2);
    valErr[0] = qVal.getValue();
    valErr[1] = qErr.getValue();

    uInt precision = precisionForValueErrorPairs(valErr, Vector<Double>());
	ostringstream out;
    out << std::fixed << setprecision(precision);
    out << qVal.getValue() << " +/- " << qErr.getValue()
    	<< " " << qVal.getUnit();
    return out.str();
}

String ImageProfileFitter::_pcfToString(
	const PCFSpectralElement *const &pcf, const CoordinateSystem& csys,
	const Vector<Double> world, const IPosition imPos,
	const Bool showTypeString, const String& indent
) const {
	Vector<Double> myWorld = world;
    String yUnit = _getImage()->units().getName();
	ostringstream summary;
	if (showTypeString) {
		summary << indent << "        Type     : "
			<< SpectralElement::fromType(pcf->getType()) << endl;
	}
	summary << indent << "        Peak     : "
		<< _elementToString(
			pcf->getAmpl(), pcf->getAmplErr(), yUnit
		) << endl;
	Double center = _centerWorld(
	    *pcf, imPos
	);

	Double increment = fabs(_fitAxisIncrement());

	Double centerErr = pcf->getCenterErr() * increment;
	Double fwhm = pcf->getFWHM() * increment;
	Double fwhmErr = pcf->getFWHMErr() * increment;

	Double pCenter = 0;
	Double pCenterErr = 0;
	Double pFWHM = 0;
	Double pFWHMErr = 0;
	Int specCoordIndex = csys.findCoordinate(Coordinate::SPECTRAL);
	Bool convertedCenterToPix = True;
	Bool convertedFWHMToPix = True;

    if (
    	specCoordIndex >= 0
    	&& _fitAxis == csys.pixelAxes(specCoordIndex)[0]
    	&& ! csys.spectralCoordinate(specCoordIndex).velocityUnit().empty()
    ) {
    	if (csys.spectralCoordinate(specCoordIndex).velocityToPixel(pCenter, center)) {
    		Double nextVel;
    		csys.spectralCoordinate(specCoordIndex).pixelToVelocity(nextVel, pCenter+1);
    		Double velInc = fabs(center - nextVel);
    		pCenterErr = centerErr/velInc;
    		pFWHM = fwhm/velInc;
    		pFWHMErr = fwhmErr/velInc;
    	}
    	else {
    		convertedCenterToPix = False;
    		convertedFWHMToPix = False;
    	}
    }
    else {
    	Vector<Double> pixel(myWorld.size());
    	myWorld[_fitAxis] = center;
    	Double delta = csys.increment()[_fitAxis];
    	if (csys.toPixel(pixel, myWorld)) {
    		pCenter = pixel[_fitAxis];
    		pCenterErr = centerErr/delta;
    	}
    	else {
    		convertedCenterToPix = False;
    	}
    	pFWHM = fwhm/delta;
    	pFWHMErr = fwhmErr/delta;
    }
	summary << indent << "        Center   : "
		<< _elementToString(
			center, centerErr, _xUnit
		) << endl;
	if (convertedCenterToPix) {
		summary << indent << "                   "
			<< _elementToString(
				pCenter, pCenterErr, "pixel"
			) << endl;
	}
	else {
		summary << indent << "                  Could not convert world to pixel for center" << endl;
	}
	summary << indent << "        FWHM     : "
		<< _elementToString(
			fwhm, fwhmErr, _xUnit
		)
		<< endl;
	if (convertedFWHMToPix) {
		summary << indent << "                   "
			<< _elementToString(pFWHM, pFWHMErr, "pixel")
			<< endl;
	}
	else {
		summary << indent << "                  Could not convert FWHM to pixel" << endl;
	}
	Double integral = pcf->getIntegral()*increment;
	Double integralErr = pcf->getIntegralErr()*increment;
	String integUnit = (Quantity(1.0 ,yUnit)*Quantity(1.0, _xUnit)).getUnit();
	summary << indent << "        Integral : "
		<< _elementToString(integral, integralErr, integUnit)
		<< endl;
	return summary.str();
}

String ImageProfileFitter::_gaussianMultipletToString(
	const GaussianMultipletSpectralElement& gm,
	const CoordinateSystem& csys,
	const Vector<Double> world, const IPosition imPos
) const {
	Vector<GaussianSpectralElement> g = gm.getGaussians();
	ostringstream summary;
	summary << "        Type     : GAUSSIAN MULTIPLET" << endl;
	for (uInt i=0; i<g.size(); i++) {
		summary << "        Results for subcomponent "
			<< i << ":" << endl;
		summary
			<< _pcfToString(&g[i], csys, world, imPos, False, "    ")
			<< endl;
	}
	return summary.str();
}

String ImageProfileFitter::_polynomialToString(
	const PolynomialSpectralElement& poly, const CoordinateSystem& csys,
	const Vector<Double> imPix, const Vector<Double> world
) const {
	ostringstream summary;
	summary << "        Type: POLYNOMIAL" << endl;
	Vector<Double> parms, errs;
	poly.get(parms);
	poly.getError(errs);
	for (uInt j=0; j<parms.size(); j++) {
		String unit = _getImage()->units().getName();
        if (j > 0) {
			unit = unit + "/((pixel)" + String::toString(j) + ")";
		}
		summary << "         c" << j << " : "
            << _elementToString(parms[j], errs[j], unit) << endl;
	}
    // coefficients in pixel coordinates
    Double x0;
    Double deltaX = _fitAxisIncrement();

    if (Quantity(1,_xUnit).isConform(Quantity(1, "m/s"))) {
        csys.spectralCoordinate(csys.findCoordinate(Coordinate::SPECTRAL)).pixelToVelocity(x0, 0);
    }
    else {
        Vector<Double> p0 = imPix;
        p0[_fitAxis] = 0;
        Vector<Double> world0 = world;
        csys.toWorld(world0, p0);
        x0 = world0[_fitAxis];
       // deltaX = csys.increment()[_fitAxis];
    }
    Vector<Double> pCoeff(_polyOrder + 1, 0);
    Vector<Double> pCoeffErr(_polyOrder + 1, 0);
    for (Int j=0; j<=_polyOrder; j++) {
        Double sumsq = 0;
        for (Int k=j; k<=_polyOrder; k++) {
        	/*
            Double multiplier = Combinatorics::choose(k, j)
                * casa::pow(x0, Float(k-j))
                * casa::pow(deltaX, Float(j));
            */
            Double multiplier = Combinatorics::choose(k, k-j)
				* casa::pow(x0, Float(k - j))
				* casa::pow(1/deltaX, Float(k));
            if ((k-j) % 2 == 1) {
            	multiplier *= -1;
            }
            pCoeff[j] += multiplier * parms[k];
            Double errCoeff = multiplier * errs[k];
            sumsq += errCoeff * errCoeff;
        }
        pCoeffErr[j] = casa::sqrt(sumsq);
        summary << "         c" << j << " : ";
		String unit = _getImage()->units().getName();
		if (j > 0 ) {
			unit = unit + "/" + "((" + _xUnit + ")" + String::toString(j) + ")";
		}
        summary << _elementToString(pCoeff[j], pCoeffErr[j], unit) << endl;
    }
	return summary.str();
}

void ImageProfileFitter::_makeSolutionImage(
	const String& name, const CoordinateSystem& csys,
	const Array<Double>& values, const String& unit,
	const Array<Bool>& mask
) {
	IPosition shape = values.shape();
	PagedImage<Float> image(shape, csys, name);
	Vector<Float> dataCopy(values.size());
	Vector<Double>::const_iterator iter;
	// isNaN(Array<Double>&) works, isNaN(Array<Float>&) gives spurious results
	Array<Bool> nanInfMask = ! (isNaN(values) || isInf(values));

	Vector<Float>::iterator jiter = dataCopy.begin();
	for (iter=values.begin(); iter!=values.end(); iter++, jiter++) {
		*jiter = (Float)*iter;
	}
	image.put(dataCopy.reform(shape));
	Bool hasPixMask = ! allTrue(mask);
	Bool hasNanMask = ! allTrue(nanInfMask);
	if (hasNanMask || hasPixMask) {
		Array<Bool> resMask(shape);
		String maskName = image.makeUniqueRegionName(
			String("mask"), 0
		);
		image.makeMask(maskName, True, True, False);
		if (hasPixMask) {
			resMask = mask.copy().reform(shape);
			if (hasNanMask) {
				resMask = resMask && nanInfMask;
			}
		}
		else {
			resMask = nanInfMask;
		}
		(&image.pixelMask())->put(resMask);
	}
	image.setUnits(Unit(unit));
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

	if (
		! ImageFit1D<Float>::setAbcissaState(
			errMsg, abcissaType, csys, "pix", doppler, _fitAxis
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

		if (! fitter.setData (curPos, abcissaType, True)) {
			*_getLog() << "Unable to set data" << LogIO::EXCEPTION;
		}
		//curPos seems to just contain a list of zeros.
		/*else {
			Vector<Int> curPosVector = curPos.asVector();
			*_getLog() <<LogIO::WARN << "Writing out curPosVector" << LogIO::POST;
			for ( int i = 0; i < curPosVector.size(); i++ ){

				*_getLog()<< LogIO::WARN << curPosVector[i]<<LogIO::POST;
			}
		}*/
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
			if (ok = fitter.fit()) {
				_flagFitterIfNecessary(fitter);
				ok = fitter.isValid();
				if (
					ok && _nonPolyEstimates.nelements() > 0
				) {
					goodPos.push_back(curPos);
				}
			}
		}
		catch (AipsError x) {
			ok = False;                       // Some other error
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
