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

#include <imageanalysis/ImageAnalysis/ImageFitter.h>

#include <casa/Containers/ContainerIO.h>
#include <casa/Utilities/Precision.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/GaussianShape.h>
#include <components/ComponentModels/PointShape.h>
#include <components/ComponentModels/SpectralModel.h>
#include <lattices/Lattices/LCPixelSet.h>

#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/PeakIntensityFluxDensityConverter.h>
#include <imageanalysis/IO/FitterEstimatesFileParser.h>

// #define DEBUG cout << __FILE__ << " " << __LINE__ << endl;

namespace casa {

const String ImageFitter::_class = "ImageFitter";

ImageFitter::ImageFitter(
	const SPCIIF image, const String& region,
	const Record *const &regionRec, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& estimatesFilename,
	const String& newEstimatesInp, const String& compListName
) : ImageTask<Float>(
		image, region, regionRec, box,
		chanInp, stokes,
		maskInp, "", False
	), _regionString(region), _residual(),_model(),
	_estimatesString(""), _newEstimatesFileName(newEstimatesInp),
	_compListName(compListName), _bUnit(image->units().getName()),
	_includePixelRange(),
	_excludePixelRange(), _estimates(), _fixed(0),
	_fitDone(False), _noBeam(False),
	_doZeroLevel(False), _zeroLevelIsFixed(False),
	_correlatedNoise(image->imageInfo().hasBeam()),
	_fitConverged(0), _peakIntensities(), _rms(-1),
	_writeControl(ImageFitterResults::NO_WRITE), _zeroLevelOffsetEstimate(0),
	_zeroLevelOffsetSolution(0), _zeroLevelOffsetError(0),
	_stokesPixNumber(-1), _chanPixNumber(-1), _results(image, _getLog()),
	_noiseFWHM(), _pixWidth(Quantity(0, "arcsec")) {
	if (
		stokes.empty() && image->coordinates().hasPolarizationCoordinate()
		&& regionRec == 0 && region.empty()
	) {
		const CoordinateSystem& csys = image->coordinates();
		Int polAxis = csys.polarizationAxisNumber();
		Int stokesVal = (Int)csys.toWorld(IPosition(image->ndim(), 0))[polAxis];
		_setStokes(Stokes::name(Stokes::type(stokesVal)));
	}
	_construct();
	_finishConstruction(estimatesFilename);
}

ImageFitter::~ImageFitter() {}

std::pair<ComponentList, ComponentList> ImageFitter::fit() {
	SPIIF modelImage, residualImage, templateImage;
	Bool doResid = ! _residual.empty();
	Bool doModel = ! _model.empty();
	if (doResid || doModel) {
		if (doResid) {
			residualImage = _createImageTemplate();
			templateImage = residualImage;
		}
		if (doModel) {
			modelImage = _createImageTemplate();
			templateImage = modelImage;
		}
	}
	uInt ngauss = _estimates.nelements() > 0 ? _estimates.nelements() : 1;
	Vector<String> models(ngauss, "gaussian");
	if (_doZeroLevel) {
		models.resize(ngauss+1, True);
		models[ngauss] = "level";
		_fixed.resize(ngauss+1, True);
		_fixed[ngauss] = _zeroLevelIsFixed ? "l" : "";
	}
	_useBeamForNoise = _correlatedNoise && ! _noiseFWHM.get()
		&& _getImage()->imageInfo().hasBeam();

	String errmsg;
	ImageStatsCalculator myStats(
		_getImage(), _getRegion(), "", False
	);
	myStats.setList(False);
	myStats.setVerbose(False);
	myStats.setAxes(_getImage()->coordinates().directionAxesNumbers());
	inputStats = myStats.statistics();
	Vector<String> allowFluxUnits(2, "Jy.km/s");
	allowFluxUnits[1] = "K.rad2";
	FluxRep<Double>::setAllowedUnits(allowFluxUnits);
	FluxRep<Float>::setAllowedUnits(allowFluxUnits);
	_results.setStokes(_getStokes());
	String resultsString = _results.resultsHeader(
		_getChans(), _chanVec, _regionString,
		_getMask(), _includePixelRange, _excludePixelRange,
		_estimatesString
	);
	LogOrigin origin(_class, __func__);;
	*_getLog() << origin;
	*_getLog() << LogIO::NORMAL << resultsString << LogIO::POST;
	ComponentList convolvedList, deconvolvedList;
	Bool anyConverged = False;
	Array<Float> residPixels, modelPixels;
	_fitLoop(
		anyConverged, convolvedList, deconvolvedList,
		templateImage, residualImage, modelImage,
		resultsString
	);
	if (anyConverged) {
		_results.writeCompList(
			convolvedList, _compListName,
			_writeControl
		);
	}
	else if (! _compListName.empty()) {
		*_getLog() << LogIO::WARN
			<< "No fits converged. Will not write component list"
			<< LogIO::POST;
	}
	if (residualImage || modelImage) {
		if (residualImage) {
			try {
				_prepareOutputImage(
					*residualImage, 0, 0,
					0, 0, &_residual, True
				);
			}
			catch (const AipsError& x) {
				*_getLog() << LogIO::WARN << "Error writing "
					<< "residual image. The reported error is "
					<< x.getMesg() << LogIO::POST;
			}
		}
		if (modelImage) {
			try {
				_prepareOutputImage(
					*modelImage, 0, 0,
					0, 0, &_model, True
				);
			}
			catch (const AipsError& x) {
				*_getLog() << LogIO::WARN << "Error writing"
					<< "model image. The reported error is "
					<< x.getMesg() << LogIO::POST;
			}
		}
	}
	if (anyConverged && ! _newEstimatesFileName.empty()) {
		_results.setConvolvedList(_curConvolvedList);
		_results.setPeakIntensities(_peakIntensities);
		_results.setMajorAxes(_majorAxes);
		_results.setMinorAxes(_minorAxes);
		_results.setPositionAngles(_positionAngles);
		_results.writeNewEstimatesFile(_newEstimatesFileName);
	}
	_createOutputRecord(convolvedList, deconvolvedList);
	FluxRep<Double>::clearAllowedUnits();
	FluxRep<Float>::clearAllowedUnits();
	_writeLogfile(resultsString);
	std::pair<ComponentList, ComponentList> lists;
	lists.first = convolvedList;
	lists.second = deconvolvedList;
	return lists;
}

void ImageFitter::_createOutputRecord(
	const ComponentList& convolved, const ComponentList& decon
) {
	String error;
	Record allConvolved, allDeconvolved;
	convolved.toRecord(error, allConvolved);
	Bool dodecon = decon.nelements() > 0;
	if (dodecon > 0) {
		decon.toRecord(error, allDeconvolved);
	}
	Bool addBeam = ! _allBeams.empty();
	for (uInt i=0; i<convolved.nelements(); i++) {
		Record peak;
		peak.define("value", _allConvolvedPeakIntensities[i].getValue());
		String unit = _allConvolvedPeakIntensities[i].getUnit();
		peak.define("unit", unit);
		peak.define("error", _allConvolvedPeakIntensityErrors[i].getValue());
		String compString = "component" + String::toString(i);
		Record sub = allConvolved.asRecord(compString);
		sub.defineRecord("peak", peak);
		Record sum;
		sum.define("value", _allSums[i].getValue());
		sum.define("unit", _allSums[i].getUnit());
		sub.defineRecord("sum", sum);
		Record beam;
		if (addBeam) {
			beam.defineRecord("beamarcsec", _allBeams[i].toRecord());
			beam.define("beampixels", _allBeamsPix[i]);
			beam.define("beamster", _allBeamsSter[i]);
			sub.defineRecord("beam", beam);
		}
		Record spectrum = sub.asRecord("spectrum");
		spectrum.define("channel", _allChanNums[i]);
		sub.defineRecord("spectrum", spectrum);
		if (dodecon) {
			sub.define("ispoint", _isPoint[i]);
		}
		allConvolved.defineRecord(compString, sub);
		if (dodecon) {
			Record sub = allDeconvolved.asRecord(compString);
			if (decon.getShape(i)->type() == ComponentType::GAUSSIAN) {
				Double areaRatio = (
					static_cast<const GaussianShape *>(convolved.getShape(i))->getArea()
					/ static_cast<const GaussianShape *>(decon.getShape(i))->getArea()
				).getValue("");
				if (areaRatio < 1e6) {
					Record peak;
					Double x = _allConvolvedPeakIntensities[i].getValue()*areaRatio;
					peak.define("value", x);
					String unit = _allConvolvedPeakIntensities[i].getUnit();
					peak.define("unit", unit);
					peak.define(
						"error",
						_allConvolvedPeakIntensityErrors[i].getValue()*areaRatio
					);
					sub.defineRecord("peak", peak);
				}
				if (addBeam) {
					sub.defineRecord("beam", beam);
				}
			}
			sub.defineRecord("sum", sum);
			Record spectrum = sub.asRecord("spectrum");
			spectrum.define("channel", _allChanNums[i]);
			sub.defineRecord("spectrum", spectrum);
			sub.define("ispoint", _isPoint[i]);
			allDeconvolved.defineRecord(compString, sub);
		}
	}
	_output.defineRecord("results", allConvolved);
	if (dodecon) {
		_output.defineRecord("deconvolved", allDeconvolved);
	}
	_output.define("converged", _fitConverged);
	if (_doZeroLevel) {
		Record z;
		z.define("value", Vector<Double>(_zeroLevelOffsetSolution));
		z.define("unit", _bUnit);
		_output.defineRecord("zerooff", z);
		z.define("value", Vector<Double>(_zeroLevelOffsetError));
		_output.defineRecord("zeroofferr", z);
	}
}

void ImageFitter::_fitLoop(
	Bool& anyConverged, ComponentList& convolvedList,
	ComponentList& deconvolvedList, SPIIF templateImage,
	SPIIF residualImage, SPIIF modelImage,
	/*LCMask& completePixelMask, */ String& resultsString
) {
	Bool converged = False;
	Bool deconvolve = False;
	Bool fit = True;
	Double zeroLevelOffsetSolution, zeroLevelOffsetError;
	Double zeroLevelOffsetEstimate = _doZeroLevel ? _zeroLevelOffsetEstimate : 0;
	uInt ngauss = _estimates.nelements() > 0 ? _estimates.nelements() : 1;
	Vector<String> models(ngauss, "gaussian");
	IPosition planeShape(_getImage()->ndim(), 1);
	ImageMetaData md(_getImage());
	Vector<Int> dirShape = md.directionShape();
	Vector<Int> dirAxisNumbers = _getImage()->coordinates().directionAxesNumbers();
	planeShape[dirAxisNumbers[0]] = dirShape[0];
	planeShape[dirAxisNumbers[1]] = dirShape[1];
	if (_doZeroLevel) {
		models.resize(ngauss+1, True);
		models[ngauss] = "level";
		_fixed.resize(ngauss+1, True);
		_fixed[ngauss] = _zeroLevelIsFixed ? "l" : "";
	}
	String errmsg;
	LogOrigin origin(getClass(), __func__);
	std::pair<Int, Int> pixelOffsets;
	const CoordinateSystem csys = _getImage()->coordinates();
	Bool hasSpectralAxis = csys.hasSpectralAxis();
	uInt spectralAxisNumber = csys.spectralAxisNumber();
	Bool outputImages = residualImage || modelImage;
	std::tr1::shared_ptr<ArrayLattice<Bool> > initMask;
	std::tr1::shared_ptr<TempImage<Float> > tImage;
	IPosition location(_getImage()->ndim(), 0);
	for (_curChan=_chanVec[0]; _curChan<=_chanVec[1]; _curChan++) {
		if (_chanPixNumber >= 0) {
			_chanPixNumber = _curChan;
		}
		Fit2D fitter(*_getLog());
		_setIncludeExclude(fitter);
		Array<Float> pixels;
		Array<Bool> pixelMask;
		_curConvolvedList = ComponentList();
		_curDeconvolvedList = ComponentList();
		try {
			_fitsky(
				fitter, pixels, pixelMask, converged,
				zeroLevelOffsetSolution, zeroLevelOffsetError,
				pixelOffsets, models, fit, deconvolve,
				zeroLevelOffsetEstimate
			);
		}
		catch (const AipsError& x) {
			*_getLog() << origin << LogIO::WARN << "Fit failed to converge "
				<< "because of exception: " << x.getMesg() << LogIO::POST;
			converged = False;
		}
		*_getLog() << origin;
		anyConverged |= converged;

		if (converged) {
			_doConverged(
				convolvedList, deconvolvedList,
				zeroLevelOffsetEstimate, pixelOffsets,
				residualImage, modelImage, tImage,
				initMask, zeroLevelOffsetSolution,
				zeroLevelOffsetError, hasSpectralAxis,
				spectralAxisNumber, outputImages, planeShape,
				pixels, pixelMask, fitter, templateImage
			);
		}
		else {
			if (_doZeroLevel) {
				_zeroLevelOffsetSolution.push_back(doubleNaN());
				_zeroLevelOffsetError.push_back(doubleNaN());
			}
			if (outputImages) {
				if (hasSpectralAxis) {
					location[spectralAxisNumber] = _curChan - _chanVec[0];
				}
				Array<Float> x(templateImage->shape());
				x.set(0);
				if (residualImage) {
					residualImage->putSlice(x, location);
				}
				if (modelImage) {
					modelImage->putSlice(x, location);
				}
			}
		}
		_fitDone = True;
		_fitConverged[_curChan - _chanVec[0]] = converged;
		if(converged) {
			Record estimatesRecord;
			_calculateErrors();
			_setDeconvolvedSizes();
			_curConvolvedList.toRecord(errmsg, estimatesRecord);
			*_getLog() << origin;
		}
		_results.setConvolvedList(_curConvolvedList);
		_results.setFixed(_fixed);
		_results.setFluxDensities(_fluxDensities);
		_results.setFluxDensityErrors(_fluxDensityErrors);
		_results.setMajorAxes(_majorAxes);
		_results.setMinorAxes(_minorAxes);
		_results.setPeakIntensities(_peakIntensities);
		_results.setPeakIntensityErrors(_peakIntensityErrors);
		_results.setPositionAngles(_positionAngles);
		String currentResultsString = _resultsToString(fitter.numberPoints());
		resultsString += currentResultsString;
		*_getLog() << LogIO::NORMAL << currentResultsString << LogIO::POST;
	}
}

void ImageFitter::_doConverged(
	ComponentList& convolvedList, ComponentList& deconvolvedList,
	Double& zeroLevelOffsetEstimate, std::pair<Int, Int>& pixelOffsets,
	SPIIF& residualImage, SPIIF& modelImage,
	std::tr1::shared_ptr<TempImage<Float> >& tImage,
	std::tr1::shared_ptr<ArrayLattice<Bool> >& initMask,
	Double zeroLevelOffsetSolution, Double zeroLevelOffsetError,
	Bool hasSpectralAxis, Int spectralAxisNumber, Bool outputImages,
	const IPosition& planeShape, const Array<Float>& pixels,
	const Array<Bool>& pixelMask, const Fit2D& fitter, SPIIF templateImage
) {
	convolvedList.addList(_curConvolvedList);
	deconvolvedList.addList(_curDeconvolvedList);
	String error;
	if (_doZeroLevel) {
		_zeroLevelOffsetSolution.push_back(
			zeroLevelOffsetSolution
		);
		_zeroLevelOffsetError.push_back(
			zeroLevelOffsetError
		);
		zeroLevelOffsetEstimate = zeroLevelOffsetSolution;
	}
	IPosition location(_getImage()->ndim(), 0);
	if (hasSpectralAxis) {
		location[spectralAxisNumber] = _curChan;
	}
	Array<Float> data = outputImages
		? _getImage()->getSlice(location, planeShape, True)
		: pixels;
	if (! outputImages) {
		pixelOffsets.first = 0;
		pixelOffsets.second = 0;
	}
	Array<Float> curResidPixels, curModelPixels;
	fitter.residual(
		curResidPixels, curModelPixels, data,
		pixelOffsets.first, pixelOffsets.second
	);
	std::tr1::shared_ptr<TempImage<Float> > fittedResid;
	if (outputImages) {
		if (hasSpectralAxis) {
			location[spectralAxisNumber] = _curChan - _chanVec[0];
		}
		Array<Bool> myMask(templateImage->shape(), True);
		residualImage->putSlice(curResidPixels, location);
		if (modelImage) {
			modelImage->putSlice(curModelPixels, location);
		}
		fittedResid = std::tr1::dynamic_pointer_cast<TempImage<Float> >(
			SubImageFactory<Float>::createImage(
				*residualImage, "", *_getRegion(), _getMask(),
				False, False, False, False
			)
		);
		ThrowIf(! fittedResid, "Dynamic cast failed");
		if (! fittedResid->hasPixelMask()) {
			fittedResid->attachMask(
				ArrayLattice<Bool>(
					Array<Bool>(fittedResid->shape(), True)
				)
			);
		}
	}
	else {
		if (! tImage) {
			// coordinates arean't important, just need the stats for a masked lattice.
			tImage.reset(
				new TempImage<Float>(
					curResidPixels.shape(), CoordinateUtil::defaultCoords2D()
				)
			);
			initMask.reset(
				new ArrayLattice<Bool>(
					Array<Bool>(curResidPixels.shape(), True)
				)
			);
			tImage->attachMask(*initMask);
		}
		fittedResid = tImage;
		fittedResid->put(curResidPixels);
	}
	Lattice<Bool> *fittedResidPixelMask = &(fittedResid->pixelMask());
	LCPixelSet lcResidMask(pixelMask, LCBox(pixelMask.shape()));
	fittedResidPixelMask = &lcResidMask;
	std::auto_ptr<MaskedLattice<Float> > maskedLattice(fittedResid->cloneML());
	LatticeStatistics<Float> lStats(*maskedLattice, False);
	Array<Double> stat;
	lStats.getStatistic(stat, LatticeStatistics<Float>::RMS, True);
	_residStats.define("rms", stat[0]);
	lStats.getStatistic(stat, LatticeStatistics<Float>::SIGMA, True);
	_residStats.define("sigma", stat[0]);
	lStats.getStatistic(stat, LatticeStatistics<Float>::NPTS, True);
}

void ImageFitter::setZeroLevelEstimate(
	Double estimate, Bool isFixed
) {
	_doZeroLevel = True;
	_zeroLevelOffsetEstimate = estimate;
	_zeroLevelIsFixed = isFixed;
}

void ImageFitter::unsetZeroLevelEstimate() {
	_doZeroLevel = False;
	_zeroLevelOffsetEstimate = 0;
	_zeroLevelIsFixed = False;
}

void ImageFitter::getZeroLevelSolution(
	vector<Double>& solution, vector<Double>& error
) {
	ThrowIf(! _fitDone, "Fit hasn't been done yet");
	ThrowIf(! _doZeroLevel, "Zero level was not fit");
	solution = _zeroLevelOffsetSolution;
	error = _zeroLevelOffsetError;
}

void ImageFitter::setRMS(const Quantity& rms) {
	Double v = rms.getValue();
	ThrowIf(v <= 0, "rms must be positive.");
	if (rms.getUnit().empty()) {
		_rms = v;
	}
	else {
		ThrowIf(
			! rms.isConform(_bUnit),
			"rms does not conform to units of " + _bUnit
		);
		_rms = rms.getValue(_bUnit);
	}
}

void ImageFitter::setNoiseFWHM(Double d) {
	const DirectionCoordinate dCoord = _getImage()->coordinates().directionCoordinate();
	_noiseFWHM.reset(
		new Quantity(
			d*_pixelWidth()
		)
	);
	_correlatedNoise = d >= 1;
	if (! _correlatedNoise) {
		*_getLog() << LogIO::WARN << "noiseFWHM is less than a pixel width, "
			<< "using uncorrelated noise expressions to calculate uncertainties"
			<< LogIO::POST;
	}
}

Quantity ImageFitter::_pixelWidth() {
	if (_pixWidth.getValue() == 0) {
		const DirectionCoordinate dCoord = _getImage()->coordinates().directionCoordinate();
		_pixWidth = Quantity(
			abs(dCoord.increment()[0]), dCoord.worldAxisUnits()[0]
		);
	}
	return _pixWidth;
}

void ImageFitter::clearNoiseFWHM() {
	_noiseFWHM.reset();
	_correlatedNoise = _getImage()->imageInfo().hasBeam();
	if (! _correlatedNoise) {
		*_getLog() << LogOrigin(getClass(), __func__) <<  LogIO::WARN
			<< "noiseFWHM not specified and image has no beam, "
			<< "using uncorrelated noise expressions to calculate uncertainties"
			<< LogIO::POST;
	}
}

void ImageFitter::setNoiseFWHM(const Quantity& q) {
	ThrowIf(
		! q.isConform("rad"),
		"noiseFWHM unit is not an angular unit"
	);
	_noiseFWHM.reset(new Quantity(q));
	_correlatedNoise = q >= _pixelWidth();
	if (! _correlatedNoise) {
		*_getLog() << LogOrigin(getClass(), __func__) << LogIO::WARN
			<< "noiseFWHM is less than a pixel width, "
			<< "using uncorrelated noise expressions to calculate uncertainties"
			<< LogIO::POST;
	}
}

Double ImageFitter::_correlatedOverallSNR(
	uInt comp, Double a, Double b,  Double signalToNoise
) const {
	Double fac = signalToNoise/2*(sqrt(_majorAxes[comp]*_minorAxes[comp])/(*_noiseFWHM)).getValue("");
	Double p = (*_noiseFWHM/_majorAxes[comp]).getValue("");
	Double fac1 = pow(1 + p*p, a/2);
	Double q = (*_noiseFWHM/_minorAxes[comp]).getValue("");
	Double fac2 = pow(1 + q*q, b/2);
	return fac*fac1*fac2;
}

GaussianBeam ImageFitter::_getCurrentBeam() const {
	return _getImage()->imageInfo().restoringBeam(
		_chanPixNumber, _stokesPixNumber
	);
}

void ImageFitter::_calculateErrors() {
	static const Double f1 = sqrt(8*C::ln2);
	static const Quantity fac(sqrt(C::pi)/f1, "");
	uInt ncomps = _curConvolvedList.nelements();
	_majorAxes.resize(ncomps);
	_majorAxisErrors.resize(ncomps);
	_minorAxes.resize(ncomps);
	_minorAxisErrors.resize(ncomps);
	_positionAngles.resize(ncomps);
	_positionAngleErrors.resize(ncomps);
	_fluxDensities.resize(ncomps);
	_fluxDensityErrors.resize(ncomps);
	_peakIntensities.resize(ncomps);
	_peakIntensityErrors.resize(ncomps);
	Double rms = _getRMS();
	Quantity pixelWidth = _pixelWidth();

	PeakIntensityFluxDensityConverter converter(_getImage());
	converter.setVerbosity(ImageTask<Float>::NORMAL);
	converter.setShape(ComponentType::GAUSSIAN);
	converter.setBeam(_chanPixNumber, _stokesPixNumber);
	if (_useBeamForNoise) {
		GaussianBeam beam = _getCurrentBeam();
		_noiseFWHM.reset(
			new Quantity(
				sqrt(beam.getMajor()*beam.getMinor()).get("arcsec")
			)
		);
		*_getLog() << LogOrigin(getClass(), __func__)
			<< LogIO::NORMAL << "Noise correlation scale length not specified but "
			<< "image has beam(s), will use the geometric mean of beam axes, "
			<< *_noiseFWHM << ", for noise correlation scale length and will "
			<< "calculate errors using correlated noise equations." << LogIO::POST;
	}
	Double signalToNoise = 0;
	for (uInt i=0; i<ncomps; i++) {
		const GaussianShape* gShape = static_cast<const GaussianShape *>(
			_curConvolvedList.getShape(i)
		);
		_majorAxes[i] = gShape->majorAxis();
		_minorAxes[i] = gShape->minorAxis();
		_positionAngles[i]  = gShape->positionAngle();
		Vector<Quantity> fluxQuant;
		_curConvolvedList.getFlux(fluxQuant, i);
		// TODO there is probably a better way to get the flux component we want...
		Vector<String> polarization = _curConvolvedList.getStokes(i);
		uInt polnum;
		for (uInt j=0; j<polarization.size(); j++) {
			if (polarization[j] == _kludgedStokes) {
				_fluxDensities[i] = fluxQuant[j];
				polnum = j;
				break;
			}
		}
		Double baseFac = 0;
		{
			// peak intensities and peak intensity errors

			converter.setSize(
				Angular2DGaussian(_majorAxes[i], _minorAxes[i], Quantity(0, "deg"))
			);
			_peakIntensities[i] = converter.fluxDensityToPeakIntensity(
				_noBeam, _fluxDensities[i]
			);
			// peak is already in brightness unit which is what we want
			signalToNoise = abs(_peakIntensities[i]).getValue()/rms;
			if (_correlatedNoise) {
				Double overallSNR = _correlatedOverallSNR(i, 1.5, 1.5, signalToNoise);
				baseFac = C::sqrt2/overallSNR;
			}
			else {
				// same baseFac used for all uncorrelated noise parameter
				// error calculations
				Quantity fac2 = fac/pixelWidth;
				Double overallSNR = (
					fac2*signalToNoise*sqrt(_majorAxes[i]*_minorAxes[i])
				).getValue("");
				baseFac = C::sqrt2/overallSNR;
			}
			_peakIntensityErrors[i] = _fixed[i].contains("f")
				? Quantity(0, _peakIntensities[i].getUnit())
				: baseFac*abs(_peakIntensities[i]);
			_allConvolvedPeakIntensities.push_back(_peakIntensities[i]);
			_allConvolvedPeakIntensityErrors.push_back(_peakIntensityErrors[i]);
		}
		Double cor1 = ! _correlatedNoise
			|| (_fixed[i].contains("a") && _fixed[i].contains("x"))
			? 0
			: C::sqrt2/_correlatedOverallSNR(
				i, 2.5, 0.5, signalToNoise
			);
		Double cor2 = ! _correlatedNoise
			|| (
				_fixed[i].contains("b") && _fixed[i].contains("y")
				&& _fixed[i].contains("p")
			)
			? 0
			: C::sqrt2/_correlatedOverallSNR(
				i, 0.5, 2.5, signalToNoise
			);
		std::auto_ptr<GaussianShape> newShape(
			dynamic_cast<GaussianShape *>(gShape->clone())
		);
		{
			// major and minor axes and position angle errors
			if (_fixed[i].contains("a")) {
				_majorAxisErrors[i] = Quantity(0, _majorAxes[i].getUnit());
			}
			else {
				if (_correlatedNoise) {
					baseFac = cor1;
				}
				_majorAxisErrors[i] = baseFac*_majorAxes[i];
			}

			if (_fixed[i].contains("b")) {
				_minorAxisErrors[i] = Quantity(0, _minorAxes[i].getUnit());
			}
			else {
				if (_correlatedNoise) {
					baseFac = cor2;
				}
				_minorAxisErrors[i] = baseFac*_minorAxes[i];
			}
			if (_fixed[i].contains("p")) {
				_positionAngleErrors[i] = Quantity(0, "rad");
			}
			else {
				if (_correlatedNoise) {
					baseFac = cor2;
				}
				Double ma = _majorAxes[i].getValue("arcsec");
				Double mi = _minorAxes[i].getValue("arcsec");
				_positionAngleErrors[i] = ma == mi
					? QC::qTurn
					:  Quantity(baseFac*C::sqrt2*(ma*mi/(ma*ma - mi*mi)), "rad");
			}

			_positionAngleErrors[i].convert(_positionAngles[i]);
			newShape->setErrors(
				_majorAxisErrors[i], _minorAxisErrors[i],
				_positionAngleErrors[i]
			);
		}
		{
			// x and y errors
			Bool fixFullPos = _fixed[i].contains("x") && _fixed[i].contains("y");
			Quantity sigmaX0, sigmaY0;
			if (fixFullPos) {
				sigmaX0 = Quantity(0, "arcsec");
				sigmaY0 = Quantity(0, "arcsec");
			}
			else {
				if (_correlatedNoise) {
					baseFac = cor1;
				}
				sigmaX0 = baseFac*_majorAxes[i]/f1;
				if (_correlatedNoise) {
					baseFac = cor2;
				}
				sigmaY0 =  baseFac*_minorAxes[i]/f1;
			}
			Double pr = fixFullPos ? 0 : (-1)*(_positionAngles[i] + QC::qTurn).getValue("rad");
			Double cp = fixFullPos ? 0 : cos(pr);
			Double sp = fixFullPos ? 0 : sin(pr);
			Quantity longErr(0, "arcsec");
			if (! _fixed[i].contains("x")) {
				Double xc = (sigmaX0*cp).getValue("arcsec");
				Double ys = (sigmaY0*sp).getValue("arcsec");
				longErr.setValue(sqrt(xc*xc + ys*ys));
			}
			Quantity latErr(0, "arcsec");
			if (! _fixed[i].contains("y")) {
				Double xs = (sigmaX0*sp).getValue("arcsec");
				Double yc = (sigmaY0*cp).getValue("arcsec");
				latErr.setValue(sqrt(xs*xs + yc*yc));
			}
			newShape->setRefDirectionError(latErr, longErr);
		}
		{
			// flux density errors
			if (_correlatedNoise) {
				Double fracA = (_peakIntensityErrors[i]/_peakIntensities[i]).getValue("");
				Double fracMaj = (_majorAxisErrors[i]/_majorAxes[i]).getValue("");
				Double fracMin = (_minorAxisErrors[i]/_minorAxes[i]).getValue("");
				Double y = (*_noiseFWHM*(*_noiseFWHM)/_majorAxes[i]/_minorAxes[i]).getValue("");
				_fluxDensityErrors[i] = _fluxDensities[i];
				_fluxDensityErrors[i] *= sqrt(
					fracA*fracA + y*(fracMaj*fracMaj + fracMin*fracMin)
				);
			}
			else {
				_fluxDensityErrors[i] = _fixed[i].contains("f") && _fixed[i].contains("a")
					&& _fixed[i].contains("b")
					? 0 : baseFac*_fluxDensities[i];
			}
		}
		_curConvolvedList.setShape(Vector<Int>(1, i), *newShape);
		Vector<std::complex<double> > errors(4, std::complex<double>(0, 0));
		errors[polnum] = std::complex<double>(_fluxDensityErrors[i].getValue(), 0);
		_curConvolvedList.component(i).flux().setErrors(errors);
		_curConvolvedList.component(i).flux().setErrors(errors);
		if (_getImage()->imageInfo().hasBeam()) {
			_curDeconvolvedList.component(i).flux().setErrors(errors);
		}
	}
}

void ImageFitter::_setIncludeExclude(Fit2D& fitter) const {
    *_getLog() << LogOrigin(_class, __func__);
	ThrowIf(
		_includePixelRange && _excludePixelRange,
		"You cannot give both an include and an exclude pixel range"
	);
	if (! _includePixelRange && ! _excludePixelRange) {
		return;
	}
	else if (_includePixelRange) {
		Float *first = &_includePixelRange->first;
		Float *second = &_includePixelRange->second;
		if (near(*first, *second)) {
			*first = -abs(*first);
			*second = -(*first);
		}
		fitter.setIncludeRange(
			*first, *second
		);
		*_getLog() << LogIO::NORMAL << "Selecting pixels from "
			<< *first << " to " << *second
			<< LogIO::POST;
	}
    else {
    	Float *first = &_excludePixelRange->first;
    	Float *second = &_excludePixelRange->second;
    	if (near(*first, *second)) {
    		*first = -abs(*first);
    		*second = -(*first);
		}
    	fitter.setExcludeRange(
			*first, *second
    	);
    	*_getLog() << LogIO::NORMAL << "Excluding pixels from "
    		<< *first << " to " << *second
    		<< LogIO::POST;
		}
	}
}

Bool ImageFitter::converged(uInt plane) const {
	ThrowIf(! _fitDone, "fit has not yet been performed");
	return _fitConverged[plane];
}

Vector<Bool> ImageFitter::converged() const {
	return _fitConverged;
}

void ImageFitter::_getStandardDeviations(Double& inputStdDev, Double& residStdDev) const {
	inputStdDev = _getStatistic("sigma", _curChan - _chanVec[0], inputStats);
	residStdDev = _getStatistic("sigma", 0, _residStats);
}

void ImageFitter::_getRMSs(Double& inputRMS, Double& residRMS) const {
	inputRMS = _getStatistic("rms", _curChan - _chanVec[0], inputStats);
	residRMS = _getStatistic("rms", 0, _residStats);
}

Double ImageFitter::_getStatistic(const String& type, const uInt index, const Record& stats) const {
	Vector<Double> statVec;
	stats.get(stats.fieldNumber(type), statVec);
	return statVec[index];
}

vector<OutputDestinationChecker::OutputStruct> ImageFitter::_getOutputStruct() {
	OutputDestinationChecker::OutputStruct newEstFile;
	newEstFile.label = "new estimates file";
	newEstFile.outputFile = &_newEstimatesFileName;
	newEstFile.required = False;
	newEstFile.replaceable = True;
	vector<OutputDestinationChecker::OutputStruct> outputs(1);
	outputs[0] = newEstFile;
	return outputs;
}

vector<Coordinate::Type> ImageFitter::_getNecessaryCoordinates() const {
	vector<Coordinate::Type> coordType(1);
	coordType[0] = Coordinate::DIRECTION;
	return coordType;
}

CasacRegionManager::StokesControl ImageFitter::_getStokesControl() const {
	return CasacRegionManager::USE_FIRST_STOKES;
}

void ImageFitter::_finishConstruction(const String& estimatesFilename) {
	*_getLog() << LogOrigin(_class, __func__);
	_setSupportsLogfile(True);
	// <todo> kludge because Flux class is really only made for I, Q, U, and V stokes

	_stokesPixNumber = _getImage()->coordinates().hasPolarizationCoordinate()
		? _getImage()->coordinates().stokesPixelNumber(_getStokes())
		: -1;

	String iquv = "IQUV";
	_kludgedStokes = (iquv.index(_getStokes()) == String::npos)
		|| _getStokes().empty()
        ? "I" : String(_getStokes());
	// </todo>
	if(estimatesFilename.empty()) {
		_fixed.resize(1);
		*_getLog() << LogIO::NORMAL
			<< "No estimates file specified, so will attempt to find and fit one gaussian."
			<< LogIO::POST;
	}
	else {
		FitterEstimatesFileParser parser(estimatesFilename, *_getImage());
		_estimates = parser.getEstimates();
		_estimatesString = parser.getContents();
		_fixed = parser.getFixed();
		*_getLog() << LogIO::NORMAL << "File " << estimatesFilename
			<< " has " << _estimates.nelements()
        	<< " specified, so will attempt to fit that many gaussians "
        	<< LogIO::POST;
	}

	CasacRegionManager rm(_getImage()->coordinates());
	uInt nSelectedChannels;
	_chanVec = _getChans().empty()
		? rm.setSpectralRanges(
			nSelectedChannels, _getRegion(), _getImage()->shape()
		)
		: rm.setSpectralRanges(
			_getChans(), nSelectedChannels, _getImage()->shape()
		);
	if (_chanVec.size() == 0) {
		_chanVec.resize(2);
		_chanVec.set(0);
		nSelectedChannels = 1;
		_chanPixNumber = -1;
	}
	else if (_chanVec.size() > 2) {
		*_getLog() << "Only a single contiguous channel range is supported" << LogIO::EXCEPTION;
	}
	else {
		_chanPixNumber = _chanVec[0];
	}
	_fitConverged.resize(nSelectedChannels);
	// check units
	Quantity q = Quantity(1, _bUnit);
	Bool unitOK = q.isConform("Jy/rad2")
		|| q.isConform("Jy*m/s/rad2") || q.isConform("K");
	if (! unitOK) {
		Vector<String> angUnits(2, "beam");
		angUnits[1] = "pixel";
		for (uInt i=0; i<angUnits.size(); i++) {
			if (_bUnit.contains(angUnits[i])) {
				UnitMap::putUser(angUnits[i], UnitVal(1,String("rad2")));
				if (
					Quantity(1, _bUnit).isConform("Jy/rad2")
					|| Quantity(1, _bUnit).isConform("Jy*m/s/rad2")
					|| Quantity(1, _bUnit).isConform("K")
				) {
					unitOK = True;
				}
				UnitMap::removeUser(angUnits[i]);
				UnitMap::clearCache();
				if (unitOK) {
					break;
				}
			}
		}
		if (! unitOK) {
			*_getLog() << LogIO::WARN << "Unrecognized intensity unit " << _bUnit
				<< ". Will assume Jy/pixel" << LogIO::POST;
			_bUnit = "Jy/pixel";
		}
	}
}

String ImageFitter::_resultsToString(uInt nPixels) const {
	ostringstream summary;
	summary << "*** Details of fit for channel number " << _curChan << endl;
	summary << "Number of pixels used in fit: " << nPixels <<  endl;
	uInt relChan = _curChan - _chanVec[0];
	if (_fitConverged[relChan]) {
		if (_noBeam) {
			*_getLog() << LogIO::WARN << "Flux density not reported because "
				<< "there is no clean beam in image header so these quantities cannot "
				<< "be calculated" << LogIO::POST;
		}
		summary << _statisticsToString() << endl;
		if (_doZeroLevel) {
			String units = _getImage()->units().getName();
			if (units.empty()) {
				units = "Jy/pixel";
			}
			summary << "Zero level offset fit: " << _zeroLevelOffsetSolution[relChan]
				<< " +/- " << _zeroLevelOffsetError[relChan] << " "
				<< units << endl;
		}
		uInt n = _curConvolvedList.nelements();
		for (uInt i = 0; i < n; i++) {
			summary << "Fit on " << _getImage()->name(True) << " component " << i << endl;
			summary << _curConvolvedList.component(i).positionToString(
				&(_getImage()->coordinates()), True
			) << endl;
			summary << _sizeToString(i) << endl;
			summary << _results.fluxToString(i, ! _noBeam) << endl;
			summary << _spectrumToString(i) << endl;
		}
	}
	else {
		summary << "*** FIT FAILED ***" << endl;
	}
	return summary.str();
}

String ImageFitter::_statisticsToString() const {
	ostringstream stats;
	// TODO It is not clear how this chi squared value is calculated and atm it does not
	// appear to be useful, so don't report it. In the future, investigate more deeply
	// how it is calculated and see if a useful value for reporting can be derived from
	// it.
	// stats << "       --- Chi-squared of fit " << chiSquared << endl;
	stats << "Input and residual image statistics (to be used as a rough guide only as to goodness of fit)" << endl;
	Double inputStdDev, residStdDev, inputRMS, residRMS;
	_getStandardDeviations(inputStdDev, residStdDev);
	_getRMSs(inputRMS, residRMS);
	String unit = _fluxDensities[0].getUnit();
	stats << "       --- Standard deviation of input image: " << inputStdDev << " " << unit << endl;
	stats << "       --- Standard deviation of residual image: " << residStdDev << " " << unit << endl;
	stats << "       --- RMS of input image: " << inputRMS << " " << unit << endl;
	stats << "       --- RMS of residual image: " << residRMS << " " << unit << endl;
	return stats.str();
}

Double ImageFitter::_getRMS() const {
	if (_rms > 0) {
		return _rms;
	}
	else {
		return Vector<Double>(_residStats.asArrayDouble("rms"))[0];
	}
}

void ImageFitter::_setDeconvolvedSizes() {
	if (! _getImage()->imageInfo().hasBeam()) {
		return;
	}
	GaussianBeam beam = _getImage()->imageInfo().restoringBeam(
		_chanPixNumber, _stokesPixNumber
	);
	uInt n = _curConvolvedList.nelements();
	static const Quantity tiny(1e-60, "arcsec");
	static const Quantity zero(0, "deg");
	_deconvolvedMessages.resize(n);
	_deconvolvedMessages.set("");
	for (uInt i=0; i<n; i++) {
		Quantity maj = _majorAxes[i];
		Quantity minor = _minorAxes[i];
		Quantity pa = _positionAngles[i];
		std::tr1::shared_ptr<GaussianShape> gaussShape(
			static_cast<GaussianShape *>(
				_curConvolvedList.getShape(i)->clone()
			)
		);
		std::tr1::shared_ptr<PointShape> point;
		Quantity emaj = _majorAxisErrors[i];
		Quantity emin = _minorAxisErrors[i];
		Quantity epa  = _positionAngleErrors[i];
		Bool fitSuccess = False;
		Angular2DGaussian bestSol(maj, minor, pa);
		Angular2DGaussian bestDecon;
		Bool isPointSource = True;
		try {
			isPointSource = beam.deconvolve(bestDecon, bestSol);
			fitSuccess = True;
		}
		catch (const AipsError& x) {
			fitSuccess = False;
			isPointSource = True;
		}
		_isPoint.push_back(isPointSource);
		ostringstream size;
		Angular2DGaussian decon;
		if(fitSuccess) {
			if (isPointSource) {
				size << "    Component is a point source" << endl;
				gaussShape->setWidth(tiny, tiny, zero);
				Angular2DGaussian largest(
					maj + emaj,
					minor + emin,
					pa - epa
				);
				Bool isPointSource1 = True;
				Bool fitSuccess1 = False;
				try {
					isPointSource1 = beam.deconvolve(decon, largest);
					fitSuccess = True;
				}
				catch (const AipsError& x) {
					fitSuccess1 = False;
					isPointSource1 = True;
				}
				// note that the code is purposefully written in such a way that
				// fitSuccess* = False => isPointSource* = True and the conditionals
				// following rely on that fact to make the code a bit clearer
				Angular2DGaussian lsize;
				if (! isPointSource1) {
					lsize = decon;
	            }
				largest.setPA(pa + epa);
				Bool isPointSource2 = True;
				Bool fitSuccess2 = False;
				try {
					isPointSource2 = beam.deconvolve(decon, largest);
					fitSuccess2 = True;
	            }
				catch (const AipsError& x) {
					fitSuccess2 = False;
					isPointSource2 = True;
	            }
				if (isPointSource2) {
					if (isPointSource1) {
						size << "    An upper limit on its size cannot be determined" << endl;
						point.reset(new PointShape());
						point->copyDirectionInfo(*gaussShape);
					}
					else {
						size << "    It may be as large as " << std::setprecision(2) << lsize.getMajor()
							<< " x " << lsize.getMinor() << endl;
						gaussShape->setErrors(lsize.getMajor(), lsize.getMinor(), zero);
	                }
				}
				else {
					if (isPointSource1) {
						size << "    It may be as large as " << std::setprecision(2) << decon.getMajor()
							<< " x " << decon.getMinor() << endl;
						gaussShape->setErrors(decon.getMajor(), decon.getMinor(), zero);
					}
					else {
						Quantity lmaj = max(decon.getMajor(), lsize.getMajor());
						Quantity lmin = max(decon.getMinor(), lsize.getMinor());
						size << "    It may be as large as " << std::setprecision(2) << lmaj
							<< " x " << lmin << endl;
						gaussShape->setErrors(lmaj, lmin, zero);
					}
				}
			}
			else {
				Vector<Quantity> majRange(2, maj - emaj);
				majRange[1] = maj + emaj;
				Vector<Quantity> minRange(2, minor - emin);
				minRange[1] = minor + emin;
				Vector<Quantity> paRange(2, pa - epa);
				paRange[1] = pa + epa;
				Angular2DGaussian sourceIn;
				Quantity mymajor, myminor;
				for (uInt i=0; i<2; i++) {
					for (uInt j=0; j<2; j++) {
						// have to check in case ranges overlap, CAS-5211
						mymajor = max(majRange[i], minRange[j]);
						myminor = min(majRange[i], minRange[j]);
						if (mymajor.getValue() > 0 && myminor.getValue() > 0) {
							sourceIn.setMajorMinor(mymajor, myminor);
							for (uInt k=0; k<2; k++) {
								sourceIn.setPA(paRange[k]);
								decon = Angular2DGaussian();
								Bool isPoint;
								try {
									isPoint = beam.deconvolve(decon, sourceIn);
								}
								catch (const AipsError& x) {
									isPoint = True;
								}
								if (! isPoint) {
									Quantity errMaj = abs(bestDecon.getMajor() - decon.getMajor());
									errMaj.convert(emaj.getUnit());
									Quantity errMin = abs(bestDecon.getMinor() - decon.getMinor());
									errMin.convert(emin.getUnit());
									Quantity errPA = abs(bestDecon.getPA(True) - decon.getPA(True));
									errPA = min(errPA, abs(errPA-QC::hTurn));
									errPA.convert(epa.getUnit());
									emaj = max(emaj, errMaj);
									emin = max(emin, errMin);
									epa = max(epa, errPA);
								}
							}
						}
					}
				}
				size << TwoSidedShape::sizeToString(
					bestDecon.getMajor(), bestDecon.getMinor(),
					bestDecon.getPA(False), True, emaj, emin, epa
				);
				gaussShape->setWidth(
					bestDecon.getMajor(), bestDecon.getMinor(),
					bestDecon.getPA(False)
				);
				gaussShape->setErrors(emaj, emin, epa);
			}
		}
		else {
			point.reset(new PointShape());
			point->copyDirectionInfo(*gaussShape);
			size << "    Could not deconvolve source from beam. "
				<< "Source may be (only marginally) resolved in only one direction.";
		}
		_deconvolvedMessages[i] = size.str();
		if (point) {
			_curDeconvolvedList.setShape(Vector<Int>(1, i), *point);
		}
		else {
			_curDeconvolvedList.setShape(Vector<Int>(1, i), *gaussShape);
		}
	}
}

String ImageFitter::_sizeToString(const uInt compNumber) const {
	ostringstream size;
	const ComponentShape* compShape = _curConvolvedList.getShape(compNumber);
	AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);
	Bool hasBeam = _getImage()->imageInfo().hasBeam();
	size << "Image component size";
	if (hasBeam) {
		size << " (convolved with beam)";
	}
	size << " ---" << endl;
	size << compShape->sizeToString() << endl;
	if (hasBeam) {
		GaussianBeam beam = _getImage()->imageInfo().restoringBeam(
			_chanPixNumber, _stokesPixNumber
		);
		size << "Clean beam size ---" << endl;
		// CAS-4577, users want two digits, so just do it explicitly here rather than using
		// TwoSidedShape::sizeToString
		size << std::fixed << std::setprecision(2) << "       --- major axis FWHM: " << beam.getMajor() << endl;
		size << "       --- minor axis FWHM: " << beam.getMinor() << endl;
		size << "       --- position angle: " << beam.getPA(True) << endl;
		size << "Image component size (deconvolved from beam) ---" << endl;
		size << _deconvolvedMessages[compNumber];
	}
	return size.str();
}

String ImageFitter::_spectrumToString(uInt compNumber) const {
	vector<String> unitPrefix = ImageFitterResults::unitPrefixes(True);
	ostringstream spec;
	const SpectralModel& spectrum = _curConvolvedList.component(compNumber).spectrum();
	Quantity frequency = spectrum.refFrequency().get("MHz");
	Quantity c(C::c, "m/s");
	Quantity wavelength = c/frequency;
	String prefUnit;
	for (uInt i=0; i<unitPrefix.size(); i++) {
		prefUnit = unitPrefix[i] + "Hz";
		if (frequency.getValue(prefUnit) > 1) {
			frequency.convert(prefUnit);
			break;
		}
	}
	for (uInt i=0; i<unitPrefix.size(); i++) {
		prefUnit = unitPrefix[i] + "m";
		if (wavelength.getValue(prefUnit) > 1) {
			wavelength.convert(prefUnit);
			break;
		}
	}

	spec << "Spectrum ---" << endl;
	spec << std::setprecision(7) << std::showpoint << "      --- frequency:        " << frequency << " (" << wavelength << ")" << endl;
	return spec.str();
}

SPIIF ImageFitter::_createImageTemplate() const {
	SPIIF x(
		SubImageFactory<Float>::createImage(
			*_getImage(), "", Record(), "",
			False, False, False, False
		)
	);
	x->set(0.0);
	return x;
}

// TODO From here until the end of the file is code extracted directly
// from ImageAnalysis. It is in great need of attention.

void ImageFitter::_fitsky(
	Fit2D& fitter, Array<Float>& pixels, Array<Bool>& pixelMask,
	Bool& converged, Double& zeroLevelOffsetSolution,
   	Double& zeroLevelOffsetError, std::pair<Int, Int>& pixelOffsets,
	const Vector<String>& models, const Bool fitIt,
	const Bool deconvolveIt, const Double zeroLevelEstimate
) {
	LogOrigin origin(_class, __func__);
	*_getLog() << origin;
	String error;
	Vector<SkyComponent> estimate;
	uInt n = _estimates.nelements();
	estimate.resize(n);
	for (uInt i = 0; i < n; i++) {
		estimate(i) = _estimates.component(i);
	}
	converged = False;
	const uInt nModels = models.nelements();
	const uInt nGauss = _doZeroLevel ? nModels - 1 : nModels;
	const uInt nMasks = _fixed.nelements();
	const uInt nEstimates = estimate.nelements();
	ThrowIf(
		nModels == 0,
		"No models have been specified"
	);
	ThrowIf(
		nGauss > 1 && estimate.nelements() < nGauss,
		"An estimate must be specified for each model component"
	);
	ThrowIf(
		! fitIt && nModels > 1,
		"Parameter estimates are only available for a single Gaussian model"
	);
	SPIIF subImageTmp(
		SubImageFactory<Float>::createImage(
			*_getImage(), "", *_getRegion(), _getMask(),
			False, False, False, _getStretch()
		)
	);
	ImageMetaData md(subImageTmp);
	ThrowIf(
		anyTrue(md.directionShape() <= 1),
		"Invalid region specification. The extent of the region in the direction plane must be "
		"at least two pixels in both dimensions"
	);
	Vector<Double> imRefPix = _getImage()->coordinates().directionCoordinate().referencePixel();
	Vector<Double> subRefPix = subImageTmp->coordinates().directionCoordinate().referencePixel();
	pixelOffsets.first = (int)floor(subRefPix[0] - imRefPix[0] + 0.5);
	pixelOffsets.second = (int)floor(subRefPix[1] - imRefPix[1] + 0.5);
	SubImage<Float> allAxesSubImage;
	{
		IPosition imShape = subImageTmp->shape();
		IPosition startPos(imShape.nelements(), 0);
		// Pass in an IPosition here to the constructor
		// this will subtract 1 from each element of the IPosition imShape
		IPosition endPos(imShape - 1);
		IPosition stride(imShape.nelements(), 1);
		const CoordinateSystem& imcsys = subImageTmp->coordinates();
		if (imcsys.hasSpectralAxis()) {
			uInt spectralAxisNumber = imcsys.spectralAxisNumber();
			startPos[spectralAxisNumber] = _curChan - _chanVec[0];
			endPos[spectralAxisNumber] = startPos[spectralAxisNumber];
		}
		if (imcsys.hasPolarizationCoordinate()) {
			uInt stokesAxisNumber = imcsys.polarizationAxisNumber();
			startPos[stokesAxisNumber] = imcsys.stokesPixelNumber(_getStokes());
			endPos[stokesAxisNumber] = startPos[stokesAxisNumber];
		}
		Slicer slice(startPos, endPos, stride, Slicer::endIsLast);
		// CAS-1966, CAS-2633 keep degenerate axes
		allAxesSubImage = SubImage<Float>(
			*subImageTmp, slice, False, AxesSpecifier(True)
		);
	}
	// for some things we don't want the degenerate axes,
	// so make a subimage without them as well
	SubImage<Float> subImage = SubImage<Float>(
		allAxesSubImage, AxesSpecifier(False)
	);
    // Make sure the region is 2D and that it holds the sky.  Exception if not.
	const CoordinateSystem& cSys = subImage.coordinates();
	Bool xIsLong = cSys.isDirectionAbscissaLongitude();
	pixels = subImage.get(True);
	pixelMask = subImage.getMask(True).copy();
	// What Stokes type does this plane hold ?
	Stokes::StokesTypes stokes = Stokes::type(_kludgedStokes);
	// Form masked array and find min/max
	MaskedArray<Float> maskedPixels(pixels, pixelMask, True);
	Float minVal, maxVal;
	IPosition minPos(2), maxPos(2);
	minMax(minVal, maxVal, minPos, maxPos, pixels);
    // Recover just single component estimate if desired and bug out
	// Must use subImage in calls as converting positions to absolute
	// pixel and vice versa
    if (!fitIt) {
		Vector<Double> parameters;
		parameters = _singleParameterEstimate(
			fitter, Fit2D::GAUSSIAN, maskedPixels,
			minVal, maxVal, minPos, maxPos
		);

		// Encode as SkyComponent and return
		Vector<SkyComponent> result(1);
		Double facToJy;
		result(0) = ImageUtilities::encodeSkyComponent(
			*_getLog(), facToJy, allAxesSubImage,
			_convertModelType(Fit2D::GAUSSIAN), parameters, stokes, xIsLong,
			deconvolveIt,
			_getImage()->imageInfo().restoringBeam(_chanPixNumber, _stokesPixNumber)
		);
		_curConvolvedList.add(result(0));
	}
	// For ease of use, make each model have a mask string
	Vector<String> fixedParameters(_fixed.copy());
	fixedParameters.resize(nModels, True);
	for (uInt j=0; j<nModels; j++) {
		if (j >= nMasks) {
			fixedParameters(j) = String("");
		}
	}
	// Add models
	Vector<String> modelTypes(models.copy());
	ThrowIf(
		nEstimates == 0 && nGauss > 1,
		"Can only auto estimate for a gaussian model"
	);
	for (uInt i = 0; i < nModels; i++) {
		// If we ask to fit a POINT component, that really means a
		// Gaussian of shape the restoring beam.  So fix the shape
		// parameters and make it Gaussian
		Fit2D::Types modelType;
		if (ComponentType::shape(models(i)) == ComponentType::POINT) {
			modelTypes(i) = "GAUSSIAN";
			fixedParameters(i) += "abp";
		}
		modelType = Fit2D::type(modelTypes(i));
		Vector<Bool> parameterMask = Fit2D::convertMask(
			fixedParameters(i),
			modelType
		);
		Vector<Double> parameters;
		if (nEstimates == 0 && modelType == Fit2D::GAUSSIAN) {
			// Auto estimate
			parameters = _singleParameterEstimate(
				fitter, modelType, maskedPixels,
				minVal, maxVal, minPos, maxPos
			);
			*_getLog() << origin;
		}
		else if (modelType == Fit2D::LEVEL) {
			parameters.resize(1);
			parameters[0] = zeroLevelEstimate;
		}
		else {
			// Decode parameters from estimate
			const CoordinateSystem& cSys = subImage.coordinates();
			const ImageInfo& imageInfo = subImage.imageInfo();

			if (modelType == Fit2D::GAUSSIAN) {
				parameters = ImageUtilities::decodeSkyComponent(
					estimate(i), imageInfo, cSys,
					_bUnit, stokes, xIsLong
				);
			}
			// The estimate SkyComponent may not be the same type as the
			// model type we are fitting for.  Try and do something about
			// this if need be by adding or removing component shape parameters
			ComponentType::Shape estType = estimate(i).shape().type();
			if (
				(modelType == Fit2D::GAUSSIAN || modelType == Fit2D::DISK)
				&& estType == ComponentType::POINT
			) {
				_fitskyExtractBeam(parameters, imageInfo, xIsLong, cSys);
			}
		}
		fitter.addModel(modelType, parameters, parameterMask);
	}
	Array<Float> sigma;
	Fit2D::ErrorTypes status = fitter.fit(pixels, pixelMask, sigma);
	*_getLog() << LogOrigin(_class, __func__);

	if (status == Fit2D::OK) {
		*_getLog() << LogIO::NORMAL << "Fitter was able to find a solution in "
			<< fitter.numberIterations() << " iterations." << LogIO::POST;
		converged = True;
	}
	else {
		converged = False;
		*_getLog() << LogIO::WARN << fitter.errorMessage() << LogIO::POST;
		return;
	}
	Vector<SkyComponent> result(_doZeroLevel ? nModels - 1 : nModels);
	Double facToJy;
	uInt j = 0;
	Bool doDeconvolved = _getImage()->imageInfo().hasBeam();
	GaussianBeam beam = _getImage()->imageInfo().restoringBeam(
		_chanPixNumber, _stokesPixNumber
	);
	for (uInt i = 0; i < nModels; i++) {
		if (fitter.type(i) == Fit2D::LEVEL) {
			zeroLevelOffsetSolution = fitter.availableSolution(i)[0];
			zeroLevelOffsetError = fitter.availableErrors(i)[0];
		}
		else {
			ComponentType::Shape modelType = _convertModelType(
				Fit2D::type(modelTypes(i))
			);
			Vector<Double> solution = fitter.availableSolution(i);
			Vector<Double> errors = fitter.availableErrors(i);
			ThrowIf(
				anyLT(errors, 0.0),
				"At least one calculated error is less than zero"
			);
			result[j] = ImageUtilities::encodeSkyComponent(
				*_getLog(), facToJy, allAxesSubImage, modelType,
				solution, stokes, xIsLong, deconvolveIt, beam
			);
			String error;
			Record r;
			result[j].flux().toRecord(error, r);
            try {
                _encodeSkyComponentError(
				    result[j], facToJy, allAxesSubImage.coordinates(),
				    solution, errors, stokes, xIsLong
			    );
            }
            catch (const AipsError& x) {
                ThrowCc(
                	"POTENTIAL DEFECT: Fitter converged but exception caught in post processing. "
                    "This may be a bug. Contact us with the image and the input parameters "
                    "you used and we will have a look. The exception message was "
                	+ x.getMesg()
                );
            }
			_curConvolvedList.add(result[j]);
			if (doDeconvolved) {
				_curDeconvolvedList.add(result[j].copy());
			}
			_setSum(result[j], subImage);
			_allChanNums.push_back(_curChan);
			j++;
		}
	}
	_setBeam(beam, j);
}

void ImageFitter::_setBeam(GaussianBeam& beam, uInt ngauss) {
	if (beam.isNull()) {
		return;
	}
	beam.convert("arcsec", "arcsec", "deg");
	Double ster = beam.getArea("sr");
	Double _pWidth = _pixelWidth().getValue("rad");
	Double pixelArea = _pWidth*_pWidth;
	Double pixels = ster/pixelArea;
	for (uInt i=0; i<ngauss; i++) {
		_allBeams.push_back(beam);
		_allBeamsPix.push_back(pixels);
		_allBeamsSter.push_back(ster);
	}
}


void ImageFitter::_setSum(const SkyComponent& comp, const SubImage<Float>& im) {
	const GaussianShape& g = static_cast<const GaussianShape&>(comp.shape());
	Quantum<Vector<Double> > dir = g.refDirection().getAngle();
	Quantity xcen(dir.getValue()[0], dir.getUnit());
	Quantity ycen(dir.getValue()[1], dir.getUnit());
	Quantity sMajor = g.majorAxis()/2;
	Quantity sMinor = g.minorAxis()/2;
	Quantity pa = g.positionAngle();
	const static Vector<Stokes::StokesTypes> stokes(0);
	AnnEllipse x(
		xcen, ycen, sMajor, sMinor, pa,
		im.coordinates(), im.shape(), stokes
	);
	Record r = x.getRegion()->toRecord("");
	SPCIIF tmp = SubImageFactory<Float>::createImage(
		im, "", r, "", True, False, True, False
	);
	ImageStatsCalculator statsCalc(tmp, 0,	String(""), False);
	statsCalc.setList(False);
	statsCalc.setVerbose(False);
	Record res = statsCalc.statistics();
	_allSums.push_back(Quantity(*(res.asArrayDouble("sum").begin()), _bUnit));
}


Vector<Double> ImageFitter::_singleParameterEstimate(
	Fit2D& fitter, Fit2D::Types model, const MaskedArray<Float>& pixels,
	Float minVal, Float maxVal, const IPosition& minPos,
	const IPosition& maxPos
) const {
	// position angle +x -> +y

	// Return the initial fit guess as either the model, an auto guess,
	// or some combination.
	*_getLog() << LogOrigin(_class, __func__);
	Vector<Double> parameters;
	if (model == Fit2D::GAUSSIAN || model == Fit2D::DISK) {
		//
		// Auto determine estimate
		//
		parameters
				= fitter.estimate(model, pixels.getArray(), pixels.getMask());
		//
		if (parameters.nelements() == 0) {
			// Fall back parameters
			*_getLog() << LogIO::WARN
				<< "The primary initial estimate failed.  Fallback may be poor"
				<< LogIO::POST;
			parameters.resize(6);
			IPosition shape = pixels.shape();
			if (abs(minVal) > abs(maxVal)) {
				parameters(0) = minVal; // height
				parameters(1) = Double(minPos(0)); // x cen
				parameters(2) = Double(minPos(1)); // y cen
			} else {
				parameters(0) = maxVal; // height
				parameters(1) = Double(maxPos(0)); // x cen
				parameters(2) = Double(maxPos(1)); // y cen
			}
			parameters(3) = Double(std::max(shape(0), shape(1)) / 2); // major axis
			parameters(4) = 0.9 * parameters(3); // minor axis
			parameters(5) = 0.0; // position angle
		}
		else {
			ThrowIf(
				parameters.nelements() != 6,
				"Not enough parameters returned by fitter estimate"
			);
		}
	}
	else {
		ThrowCc("Only Gaussian/Disk auto-single estimates are available");
	}
	return parameters;
}

ComponentType::Shape ImageFitter::_convertModelType(Fit2D::Types typeIn) const {
	if (typeIn == Fit2D::GAUSSIAN) {
		return ComponentType::GAUSSIAN;
	}
	else if (typeIn == Fit2D::DISK) {
		return ComponentType::DISK;
	}
	else {
		throw(AipsError("Unrecognized model type"));
	}
}

void ImageFitter::_fitskyExtractBeam(
	Vector<Double>& parameters, const ImageInfo& imageInfo,
	const Bool xIsLong, const CoordinateSystem& cSys
) const {
	// We need the restoring beam shape as well.
	GaussianBeam beam = imageInfo.restoringBeam(_chanPixNumber, _stokesPixNumber);
	Vector<Quantity> wParameters(5);
	// Because we convert at the reference
	// value for the beam, the position is
	// irrelevant
	wParameters(0).setValue(0.0);
	wParameters(1).setValue(0.0);
	wParameters(0).setUnit(String("rad"));
	wParameters(1).setUnit(String("rad"));
	wParameters(2) = beam.getMajor();
	wParameters(3) = beam.getMinor();
	wParameters(4) = beam.getPA();

	// Convert to pixels for Fit2D
	IPosition pixelAxes(2);
	pixelAxes(0) = 0;
	pixelAxes(1) = 1;
	if (!xIsLong) {
		pixelAxes(1) = 0;
		pixelAxes(0) = 1;
	}
	Bool doRef = True;
	Vector<Double> dParameters;
	ImageUtilities::worldWidthsToPixel(
		dParameters, wParameters, cSys, pixelAxes, doRef
	);
	parameters.resize(6, True);
	parameters(3) = dParameters(0);
	parameters(4) = dParameters(1);
	parameters(5) = dParameters(2);
}

void ImageFitter::_encodeSkyComponentError(
		SkyComponent& sky, Double facToJy, const CoordinateSystem& csys,
		const Vector<Double>& parameters, const Vector<Double>& errors,
		Stokes::StokesTypes stokes, Bool xIsLong) const
// Input
//   facToJy = conversion factor to Jy
//   pars(0) = peak flux  image units
//   pars(1) = x cen    abs pix
//   pars(2) = y cen    abs pix
//   pars(3) = major    pix
//   pars(4) = minor    pix
//   pars(5) = pa radians (pos +x -> +y)
//
//   error values will be zero for _fixed parameters
{
	//
	// Flux. The fractional error of the integrated and peak flux
	// is the same.  errorInt = Int * (errorPeak / Peak) * facToJy
	// TODO it is? why is that? The error in the flux density has not
	// been propogated correctly, because the (implicit) error in facToJy
	// is not being caried along. This error arises because the size (major*minor axex)
	// is not error free.
	Flux<Double> flux = sky.flux(); // Integral
	Vector<Double> valueInt;
	flux.value(valueInt);
	Vector<Double> tmp(4, 0.0);
	if (errors(0) > 0.0) {
		Double rat = (errors(0) / parameters(0)) * facToJy;
		if (stokes == Stokes::I) {
			tmp(0) = valueInt(0) * rat;
		} else if (stokes == Stokes::Q) {
			tmp(1) = valueInt(1) * rat;
		} else if (stokes == Stokes::U) {
			tmp(2) = valueInt(2) * rat;
		} else if (stokes == Stokes::V) {
			tmp(3) = valueInt(3) * rat;
		} else {
			// TODO handle stokes in addition to I,Q,U,V. For now, treat other stokes like stokes I.
			tmp(0) = valueInt(0) * rat;
		}
		flux.setErrors(tmp(0), tmp(1), tmp(2), tmp(3));
	}
	// Shape.  Only TwoSided shapes have something for me to do
	IPosition pixelAxes(2);
	pixelAxes(0) = 0;
	pixelAxes(1) = 1;
	if (!xIsLong) {
		pixelAxes(1) = 0;
		pixelAxes(0) = 1;
	}
	ComponentShape& shape = sky.shape();
	TwoSidedShape* pS = dynamic_cast<TwoSidedShape*> (&shape);
	Vector<Double> dParameters(5);
	GaussianBeam wParameters;
	static const Quantity qzero(0, "deg");
	if (pS) {
		if (errors(3) > 0.0 || errors(4) > 0.0 || errors(5) > 0.0) {
			dParameters(0) = parameters(1); // x
			dParameters(1) = parameters(2); // y
			// Use the pixel to world converter by pretending the width
			// errors are widths.  The minor error may be greater than major
			// error so beware as the widths converted will flip them about.
			// The error in p.a. is just the input error value as its
			// already angular.
			// Major

			// widths cannot be zero or exceptions will be thrown, so if either axis
			// is fixed so that its error is 0, make it a very small number instead for
			// the call to pixelWidthsToWorld
			static const Double epsilon = 1e-9;
			dParameters(2) = errors(3) == 0 ? epsilon : errors(3);
			// Minor
			dParameters(3) = errors(4) == 0 ? epsilon : errors(4);
			// PA
			dParameters(4) = parameters(5);
			// If flipped, it means pixel major axis morphed into world minor
			// Put back any zero errors as well.
			Bool flipped = ImageUtilities::pixelWidthsToWorld(
				wParameters,
				dParameters, csys, pixelAxes, False
			);
			Quantity paErr(errors(5), "rad");
			if (flipped) {
				pS->setErrors(
					errors(4) == 0 ? qzero : wParameters.getMinor(),
					errors(3) == 0 ? qzero : wParameters.getMajor(),
					paErr
				);
			}
			else {
				pS->setErrors(
					errors(3) == 0 ? qzero : wParameters.getMajor(),
					errors(4) == 0 ? qzero : wParameters.getMinor(),
					paErr
				);
			}
		}
	}
	// Position.  Use the pixel to world widths converter again.
	// Or do something simpler ?
	// X
	dParameters(2) = errors(1) == 0 ? 1e-8 : errors(1);
	// Y
	dParameters(3) = errors(2) == 0 ? 1e-8 : errors(2);
	dParameters(4) = 0.0; // Pixel errors are in X/Y directions not along major axis
	Bool flipped = ImageUtilities::pixelWidthsToWorld(
			wParameters, dParameters,
			csys, pixelAxes, False
		);
	// TSS::setRefDirErr interface has lat first

	if (flipped) {
		pS->setRefDirectionError(
				errors(2) == 0 ? qzero : wParameters.getMinor(),
				errors(1) == 0 ? qzero : wParameters.getMajor()
			);
	}
	else {
		pS->setRefDirectionError(
			errors(2) == 0 ? qzero : wParameters.getMajor(),
			errors(1) == 0 ? qzero : wParameters.getMinor()
		);
	}
}




