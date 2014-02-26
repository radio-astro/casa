#include <imageanalysis/ImageAnalysis/ImageFitter.h>

#include <casa/Containers/HashMap.h>
#include <casa/Containers/HashMapIter.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/Utilities/Precision.h>

#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/PointShape.h>
#include <components/ComponentModels/SpectralModel.h>

#include <imageanalysis/IO/FitterEstimatesFileParser.h>
#include <imageanalysis/IO/LogFile.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/PeakIntensityFluxDensityConverter.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <imageanalysis/Regions/CasacRegionManager.h>

#include <images/Regions/WCUnion.h>
#include <images/Regions/WCBox.h>
#include <lattices/Lattices/LCPixelSet.h>

#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentShape.h>

#include <components/ComponentModels/GaussianShape.h>

#include <memory>

// #define DEBUG cout << __FILE__ << " " << __LINE__ << endl;

namespace casa {

const String ImageFitter::_class = "ImageFitter";

ImageFitter::ImageFitter(
		const SPCIIF image, const String& region,
	const Record *const &regionRec,
	const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const Vector<Float>& includepix,
	const Vector<Float>& excludepix, const String& residualInp,
	const String& modelInp, const String& estimatesFilename,
	const String& newEstimatesInp, const String& compListName
) : ImageTask<Float>(
		image, region, regionRec, box,
		chanInp, stokes,
		maskInp, "", False
	), _regionString(region), _residual(residualInp),_model(modelInp),
	_estimatesString(""), _newEstimatesFileName(newEstimatesInp),
	_compListName(compListName), _bUnit(image->units().getName()),
	_includePixelRange(includepix),
	_excludePixelRange(excludepix), _estimates(), _fixed(0),
	_fitDone(False), _noBeam(False),
	_doZeroLevel(False), _zeroLevelIsFixed(False),
	_fitConverged(0), _peakIntensities(), _rms(0),
	_writeControl(NO_WRITE), _zeroLevelOffsetEstimate(0),
	_zeroLevelOffsetSolution(0), _zeroLevelOffsetError(0),
	_stokesPixNumber(-1), _chanPixNumber(-1) {
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
	LogOrigin origin(_class, __FUNCTION__);;
	*_getLog() << origin;
	Bool converged;
	SubImage<Float> templateImage;
	std::auto_ptr<TempImage<Float> > modelImage, residualImage;
	std::auto_ptr<LCMask> completePixelMask;
	if (! _residual.empty() || ! _model.empty()) {
		templateImage = _createImageTemplate();
		completePixelMask.reset(new LCMask(templateImage.shape()));
		if (! _residual.empty()) {
			residualImage.reset(
				new TempImage<Float>(
					templateImage.shape(), templateImage.coordinates()
				)
			);
		}
		if (! _model.empty()) {
			modelImage.reset(
				new TempImage<Float>(
					templateImage.shape(), templateImage.coordinates()
				)
			);
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
	Bool fit = True;
	Bool deconvolve = False;
	Bool list = True;
	String errmsg;
	ImageStatsCalculator myStats(
		_getImage(), _getRegion(), "", False
	);
	myStats.setAxes(_getImage()->coordinates().directionAxesNumbers());
	inputStats = myStats.statistics();
	Vector<String> allowFluxUnits(2, "Jy.km/s");
	allowFluxUnits[1] = "K.rad2";
	FluxRep<Double>::setAllowedUnits(allowFluxUnits);
	FluxRep<Float>::setAllowedUnits(allowFluxUnits);
	String resultsString = _resultsHeader();
	*_getLog() << LogIO::NORMAL << resultsString << LogIO::POST;
	ComponentList convolvedList, deconvolvedList;
	Bool anyConverged = False;
	Array<Float> residPixels, modelPixels;
	Double zeroLevelOffsetSolution, zeroLevelOffsetError;
	Double zeroLevelOffsetEstimate = _doZeroLevel ? _zeroLevelOffsetEstimate : 0;
	for (_curChan=_chanVec[0]; _curChan<=_chanVec[1]; _curChan++) {
		if (_chanPixNumber >= 0) {
			_chanPixNumber = _curChan;
		}
		Fit2D fitter(*_getLog());
		_setIncludeExclude(fitter);
		Array<Float> pixels, curResidPixels, curModelPixels;
		Array<Bool> pixelMask;
		_curConvolvedList = ComponentList();
		_curDeconvolvedList = ComponentList();
		try {
			_fitsky(
				fitter, pixels, pixelMask, converged,
				zeroLevelOffsetSolution, zeroLevelOffsetError,
				models,
				fit, deconvolve, list, zeroLevelOffsetEstimate
			);
		}
		catch (const AipsError& err) {
			*_getLog() << origin << LogIO::WARN << "Fit failed to converge because of exception: "
				<< err.getMesg() << LogIO::POST;
			converged = False;
		}
		*_getLog() << origin;
		anyConverged |= converged;
		if (converged) {
			convolvedList.addList(_curConvolvedList);
			deconvolvedList.addList(_curDeconvolvedList);
			if (_doZeroLevel) {
				_zeroLevelOffsetSolution.push_back(
					zeroLevelOffsetSolution
				);
				_zeroLevelOffsetError.push_back(
					zeroLevelOffsetError
				);
				zeroLevelOffsetEstimate = zeroLevelOffsetSolution;
			}
			fitter.residual(curResidPixels, curModelPixels, pixels);
			// coordinates arean't important, just need the stats for a masked lattice.
			TempImage<Float> residPlane(
				curResidPixels.shape(), CoordinateUtil::defaultCoords2D()
			);
			residPlane.put(curResidPixels);
			LCPixelSet lcResidMask(pixelMask, LCBox(pixelMask.shape()));
			residPlane.attachMask(lcResidMask);
			LatticeStatistics<Float> lStats(*residPlane.cloneML(), False);
			Array<Double> stat;
			lStats.getStatistic(stat, LatticeStatistics<Float>::RMS, True);
			_residStats.define("rms", stat[0]);
			lStats.getStatistic(stat, LatticeStatistics<Float>::SIGMA, True);
			_residStats.define("sigma", stat[0]);
		}
		else if (_doZeroLevel) {
			_zeroLevelOffsetSolution.push_back(doubleNaN());
			_zeroLevelOffsetError.push_back(doubleNaN());
		}
		if (residualImage.get() != 0 || modelImage.get() != 0) {
			IPosition arrShape = templateImage.shape();
			if (! converged) {
				pixelMask.set(False);
			}
			IPosition putLocation(templateImage.ndim(), 0);
			if (templateImage.coordinates().hasSpectralAxis()) {
				uInt spectralAxisNumber = templateImage.coordinates().spectralAxisNumber();
				arrShape[spectralAxisNumber] = 1;
				putLocation[spectralAxisNumber] = _curChan - _chanVec[0];
			}
			completePixelMask->putSlice(pixelMask, putLocation);
			if (residualImage.get() != 0) {
				if (! converged) {
					curResidPixels.resize(pixels.shape());
					curResidPixels.set(0);
				}
				residualImage->putSlice(curResidPixels, putLocation);
			}
			if (modelImage.get() != 0) {
				if (! converged) {
					curModelPixels.resize(pixels.shape());

					curModelPixels.set(0);
				}
				modelImage->putSlice(curModelPixels, putLocation);
			}
		}
		_fitDone = True;
		_fitConverged[_curChan - _chanVec[0]] = converged;
		if(converged) {
			Record estimatesRecord;
			_setFluxes();
			_setSizes();
			_setDeconvolvedSizes();
			_curConvolvedList.toRecord(errmsg, estimatesRecord);
			*_getLog() << origin;
		}
		String currentResultsString = _resultsToString();
		resultsString += currentResultsString;
		*_getLog() << LogIO::NORMAL << currentResultsString << LogIO::POST;
	}
	if (anyConverged) {
		_writeCompList(convolvedList);
	}
	else {
		*_getLog() << LogIO::WARN
			<< "No fits converged. Will not write component list"
			<< LogIO::POST;
	}
	if (residualImage.get() != 0) {
		try {
			ImageUtilities::writeImage(
				residualImage->shape(),
				residualImage->coordinates(),
				_residual,
				residualImage->get(),
				*_getLog(), completePixelMask->get(False)
			);
		}
		catch (const AipsError& x) {
			*_getLog() << LogIO::WARN << "Error writing residual image. The reported error is "
				<< x.getMesg() << LogIO::POST;
		}
	}
	if (modelImage.get() != 0) {
		try {
			ImageUtilities::writeImage(
				modelImage->shape(),
				modelImage->coordinates(),
				_model,
				modelImage->get(),
				*_getLog(), completePixelMask->get(False)
			);
		}
		catch (const AipsError& x) {
			*_getLog() << LogIO::WARN << "Error writing residual image. The reported error is "
				<< x.getMesg() << LogIO::POST;
		}
	}
	FluxRep<Double>::clearAllowedUnits();
	FluxRep<Float>::clearAllowedUnits();
	if (converged && ! _newEstimatesFileName.empty()) {
		_writeNewEstimatesFile();
	}
	_writeLogfile(resultsString);
	std::pair<ComponentList, ComponentList> lists;
	lists.first = convolvedList;
	lists.second = deconvolvedList;
	return lists;
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

void ImageFitter::setRMS(Double rms) {
	ThrowIf(rms <= 0, "rms must be positive.");
	_rms = rms;
}

void ImageFitter::_setIncludeExclude(Fit2D& fitter) const {
    *_getLog() << LogOrigin(_class, __FUNCTION__);
	Bool doInclude = (_includePixelRange.nelements() > 0);
	Bool doExclude = (_excludePixelRange.nelements() > 0);
	ThrowIf(
		doInclude && doExclude,
		"You cannot give both an include and an exclude pixel range"
	);
	if (!doInclude && !doExclude) {
		*_getLog() << LogIO::NORMAL << "Selecting all pixel values because neither "
			<< "includepix nor excludepix was specified" << LogIO::POST;
		return;
	}
	if (doInclude) {
		if (_includePixelRange.nelements() == 1) {
			fitter.setIncludeRange(
				-abs(_includePixelRange(0)), abs(_includePixelRange(0))
			);
			*_getLog() << LogIO::NORMAL << "Selecting pixels from "
				<< -abs(_includePixelRange(0)) << " to " << abs(_includePixelRange(0))
				<< LogIO::POST;
		}
		else if (_includePixelRange.nelements() > 1) {
			fitter.setIncludeRange(
				_includePixelRange(0), _includePixelRange(1)
			);
			*_getLog() << LogIO::NORMAL << "Selecting pixels from "
				<< _includePixelRange(0) << " to " << _includePixelRange(1)
				<< LogIO::POST;
		}
	}
    else {
		if (_excludePixelRange.nelements() == 1) {
			fitter.setExcludeRange(
				-abs(_excludePixelRange(0)), abs(_excludePixelRange(0))
			);
			*_getLog() << LogIO::NORMAL << "Excluding pixels from "
				<< -abs(_excludePixelRange(0)) << " to " << abs(_excludePixelRange(0))
				<< LogIO::POST;
		}
           else if (_excludePixelRange.nelements() > 1) {
			fitter.setExcludeRange(
				_excludePixelRange(0), _excludePixelRange(1)
			);
			*_getLog() << LogIO::NORMAL << "Excluding pixels from "
				<< _excludePixelRange(0) << " to " << _excludePixelRange(1)
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

vector<OutputDestinationChecker::OutputStruct> ImageFitter::_getOutputs() {
	LogOrigin logOrigin("ImageFitter", __FUNCTION__);
	*_getLog() << logOrigin;

	OutputDestinationChecker::OutputStruct residualIm;
	residualIm.label = "residual image";
	residualIm.outputFile = &_residual;
	residualIm.required = False;
	residualIm.replaceable = True;
	OutputDestinationChecker::OutputStruct modelIm;
	modelIm.label = "model image";
	modelIm.outputFile = &_model;
	modelIm.required = False;
	modelIm.replaceable = True;
	OutputDestinationChecker::OutputStruct newEstFile;
	newEstFile.label = "new estiamtes file";
	newEstFile.outputFile = &_newEstimatesFileName;
	newEstFile.required = False;
	newEstFile.replaceable = True;

	vector<OutputDestinationChecker::OutputStruct> outputs(3);
	outputs[0] = residualIm;
	outputs[1] = modelIm;
	outputs[2] = newEstFile;

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
	*_getLog() << LogOrigin(_class, __FUNCTION__);
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

String ImageFitter::_resultsHeader() const {
	ostringstream summary;
	ostringstream chansoss;
	String chans = _getChans();
	if (! chans.empty()) {
		chansoss << chans;
	}
	else if (_chanVec.size() == 2) {
		if (_chanVec[0] == _chanVec[1]) {
			chansoss << _chanVec[0];
		}
		else {
			chansoss << _chanVec[0] << "-" << _chanVec[1];
		}
	}
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << "Input parameters ---" << endl;
	summary << "       --- imagename:           " << _getImage()->name() << endl;
	summary << "       --- region:              " << _regionString << endl;
	summary << "       --- channel:             " << chansoss.str() << endl;
	summary << "       --- stokes:              " << _getStokes() << endl;
	summary << "       --- mask:                " << _getMask() << endl;
	summary << "       --- include pixel range: " << _includePixelRange << endl;
	summary << "       --- exclude pixel range: " << _excludePixelRange << endl;
	if (! _estimatesString.empty()) {
		summary << "       --- initial estimates:   Peak, X, Y, a, b, PA" << endl;
		summary << "                                " << _estimatesString << endl;
	}
	return summary.str();
}

String ImageFitter::_resultsToString() {
	ostringstream summary;
	summary << "*** Details of fit for channel number " << _curChan << endl;

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
			summary << _curConvolvedList.component(i).positionToString(&(_getImage()->coordinates())) << endl;
			summary << _sizeToString(i) << endl;
			summary << _fluxToString(i) << endl;
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
	stats << "       --- Standard deviation of input image " << inputStdDev << " " << unit << endl;
	stats << "       --- Standard deviation of residual image " << residStdDev << " " << unit << endl;
	stats << "       --- RMS of input image " << inputRMS << " " << unit << endl;
	stats << "       --- RMS of residual image " << residRMS << " " << unit << endl;
	return stats.str();
}

void ImageFitter::_setFluxes() {
	uInt ncomps = _curConvolvedList.nelements();
	_fluxDensities.resize(ncomps);
	_fluxDensityErrors.resize(ncomps);
	_peakIntensities.resize(ncomps);
	_peakIntensityErrors.resize(ncomps);
	_majorAxes.resize(ncomps);
	_minorAxes.resize(ncomps);
	Vector<Quantity> fluxQuant;
	Double rmsPeak = ! near(_rms, 0.0) ? _rms : Vector<Double>(_residStats.asArrayDouble("rms"))[0];
	Quantity rmsPeakError(rmsPeak, _bUnit);
	Quantity resArea = _getImage()->coordinates().directionCoordinate().getPixelArea();
	if (
		_bUnit.contains("/beam")
		|| (Quantity(1, _bUnit).isConform("K") && _getImage()->imageInfo().hasBeam())
	) {
        try {
            Unit unit = resArea.getUnit();
            resArea = Quantity(
                _getImage()->imageInfo().restoringBeam(
                    _chanPixNumber, _stokesPixNumber
                ).getArea(unit),
                unit
            );
        }
        catch (const AipsError&) {
			*_getLog() << LogIO::WARN
				<< "Image units are per beam but beam area could not "
				<< "be determined. Assume beam area is pixel area."
				<< LogIO::POST;
		}
	}
    PeakIntensityFluxDensityConverter converter(_getImage());
    converter.setVerbosity(ImageTask<Float>::NORMAL);
    converter.setShape(ComponentType::GAUSSIAN);
	uInt polNum = 0;
	for(uInt i=0; i<ncomps; i++) {
		_curConvolvedList.getFlux(fluxQuant, i);
		Bool fluxIsFixed = _fixed[i].contains("f")
			&& _fixed[i].contains("a")
			&& _fixed[i].contains("b");
		// TODO there is probably a better way to get the flux component we want...
		Vector<String> polarization = _curConvolvedList.getStokes(i);
		for (uInt j=0; j<polarization.size(); j++) {
			if (polarization[j] == _kludgedStokes) {
				_fluxDensities[i] = fluxQuant[j];
				if (fluxIsFixed) {
					_fluxDensityErrors[i] = 0;
				}
				else {
					std::complex<double> error = _curConvolvedList.component(i).flux().errors()[j];
					_fluxDensityErrors[i].setValue(sqrt(error.real()*error.real()
						+ error.imag()*error.imag()));
				}
				_fluxDensityErrors[i].setUnit(_fluxDensities[i].getUnit());
				polNum = j;
				break;
			}
		}
		const ComponentShape* compShape = _curConvolvedList.getShape(i);
		AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);
		_majorAxes[i] = (static_cast<const GaussianShape *>(compShape))->majorAxis();
		_minorAxes[i] = (static_cast<const GaussianShape *>(compShape))->minorAxis();
		converter.setBeam(_chanPixNumber, _stokesPixNumber);
		converter.setSize(
			Angular2DGaussian(_majorAxes[i], _minorAxes[i], Quantity(0, "deg"))
		);
		_peakIntensities[i] = converter.fluxDensityToPeakIntensity(
			_noBeam, _fluxDensities[i]
		);
		rmsPeakError.convert(_peakIntensities[i].getUnit());
		Double rmsPeakErrorValue = rmsPeakError.getValue();
		Double peakErrorFromFluxErrorValue = (
				_peakIntensities[i]*_fluxDensityErrors[i]/_fluxDensities[i]
		).getValue();
		if (_fixed[i].contains("f")) {
			_peakIntensityErrors[i].setValue(0);
		}
		else {
			_peakIntensityErrors[i].setValue(
				max(
					rmsPeakErrorValue,
					peakErrorFromFluxErrorValue
				)
			);
		}
		_peakIntensityErrors[i].setUnit(_bUnit);
		if (
			! fluxIsFixed
			&& rmsPeakErrorValue > peakErrorFromFluxErrorValue
		) {
			const GaussianShape *gaussShape = static_cast<const GaussianShape *>(compShape);
			Quantity rmsFluxError;
			if (rmsPeakError.isConform("K")) {
				Double fracMajor = (gaussShape->majorAxisError()/_majorAxes[i]).getValue("");
				Double fracMinor = (gaussShape->minorAxisError()/_minorAxes[i]).getValue("");
				Double fracPI = (rmsPeakError/_peakIntensities[i]).getValue("");
				Double fracError = sqrt(
					fracMinor*fracMinor + fracMajor*fracMajor + fracPI*fracPI
				);
				rmsFluxError = fracError*_fluxDensities[i];
			}
			else {
				Quantity compArea = gaussShape->getArea();
				rmsFluxError = rmsPeakError*compArea/resArea;
				rmsFluxError.convert(_fluxDensityErrors[i].getUnit());
			}
			_fluxDensityErrors[i].setValue(
				max(
					_fluxDensityErrors[i].getValue(),
					rmsFluxError.getValue()
				)
			);
			Vector<std::complex<double> > errors(4, std::complex<double>(0, 0));
			errors[polNum] = std::complex<double>(_fluxDensityErrors[i].getValue(), 0);
			_curConvolvedList.component(i).flux().setErrors(errors);
			if (_getImage()->imageInfo().hasBeam()) {
				_curDeconvolvedList.component(i).flux().setErrors(errors);
			}
		}
	}
}

void ImageFitter::_setSizes() {
	uInt ncomps = _curConvolvedList.nelements();
	_positionAngles.resize(ncomps);
	_majorAxisErrors.resize(ncomps);
	_minorAxisErrors.resize(ncomps);
	_positionAngleErrors.resize(ncomps);
	Double rmsPeak = ! near(_rms, 0.0) ? _rms : Vector<Double>(_residStats.asArrayDouble("rms"))[0];
	Quantity rmsPeakError(rmsPeak, _bUnit);
	Quantity xBeam;
	Quantity yBeam;
	Quantity paBeam;
	if (_getImage()->imageInfo().hasBeam()) {
		GaussianBeam beam = _getImage()->imageInfo().restoringBeam(
			_chanPixNumber, _stokesPixNumber
		);
		xBeam = beam.getMajor();
		yBeam = beam.getMinor();
		paBeam = beam.getPA();
	}
	else {
		Vector<Double> pixInc = _getImage()->coordinates().directionCoordinate().increment();
		xBeam = Quantity(pixInc[0], "rad");
		yBeam = Quantity(pixInc[1], "rad");
		paBeam = Quantity(0, "rad");
	}
	uInt n = _curConvolvedList.nelements();
	for(uInt i=0; i<n; i++) {
		const ComponentShape* compShape = _curConvolvedList.getShape(i);
		AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);
		_positionAngles[i]  = (static_cast<const GaussianShape *>(compShape))->positionAngle();
		_majorAxisErrors[i] = (static_cast<const GaussianShape *>(compShape))->majorAxisError();
		_minorAxisErrors[i] = (static_cast<const GaussianShape *>(compShape))->minorAxisError();
		_positionAngleErrors[i] = (static_cast<const GaussianShape *>(compShape))->positionAngleError();
		Double signalToNoise = fabs((_peakIntensities[i]/rmsPeakError).getValue());
		Quantity paRelToBeam = _positionAngles[i] - paBeam;
		paRelToBeam.convert("rad");
		xBeam.convert(_majorAxisErrors[i].getUnit());
		yBeam.convert(_majorAxisErrors[i].getUnit());
		Double xBeamVal = xBeam.getValue();
		Double yBeamVal = yBeam.getValue();

		Double cosPA = cos(paRelToBeam.getValue());
		Double sinPA = sin(paRelToBeam.getValue());

		// angles are measured from north (y direction).
		if (! _fixed[i].contains("a")) {
			_majorAxisErrors[i].setValue(
				max(
					_majorAxisErrors[i].getValue(),
					sqrt(
						(
							xBeamVal*sinPA * xBeamVal*sinPA
							+ yBeamVal*cosPA * yBeamVal*cosPA
						)
					)/signalToNoise
				)
			);
		}
		if (! _fixed[i].contains("b")) {
			_minorAxisErrors[i].setValue(
				max(
					_minorAxisErrors[i].getValue(),
					sqrt(
						(
							xBeamVal*cosPA * xBeamVal*cosPA
							+ yBeamVal*sinPA * yBeamVal*sinPA
						)
					)/signalToNoise
				)
			);
		}
		if (! _fixed[i].contains("p")) {
			Double posAngleRad = _positionAngles[i].getValue(Unit("rad"));
			Quantity posAngErrorFromSN = _positionAngles[i] * sqrt(
					_majorAxisErrors[i]/_majorAxes[i] * _majorAxisErrors[i]/_majorAxes[i]
					+ _minorAxisErrors[i]/_minorAxes[i] * _minorAxisErrors[i]/_minorAxes[i]
			);
			posAngErrorFromSN *= 1/(1 + posAngleRad*posAngleRad);
			posAngErrorFromSN.convert(_positionAngleErrors[i].getUnit());
			_positionAngleErrors[i].setValue(
					max(_positionAngleErrors[i].getValue(), posAngErrorFromSN.getValue())
				);
		}
		_majorAxisErrors[i].convert(_majorAxes[i].getUnit());
		_minorAxisErrors[i].convert(_minorAxes[i].getUnit());
		_positionAngleErrors[i].convert(_positionAngles[i].getUnit());
		GaussianShape* newShape = dynamic_cast<GaussianShape *>(compShape->clone());
		newShape->setErrors(
			_majorAxisErrors[i], _minorAxisErrors[i],
			_positionAngleErrors[i]
		);
		// set the position uncertainties

		Quantity latError = compShape->refDirectionErrorLat();
		Quantity longError = compShape->refDirectionErrorLong();

		paBeam.convert("rad");
		Double cosPaBeam = cos(paBeam.getValue());
		Double sinPaBeam = sin(paBeam.getValue());

		if (! _fixed[i].contains("x")) {
			Quantity longErrorFromSN = sqrt(
					xBeam*sinPaBeam*xBeam*sinPaBeam + yBeam*cosPaBeam*yBeam*cosPaBeam
				)/(2*signalToNoise);
			longErrorFromSN.convert(longError.getUnit());
			longError.setValue(
					max(longError.getValue(), longErrorFromSN.getValue())
				);
		}
		if (! _fixed[i].contains("y")) {
			Quantity latErrorFromSN = sqrt(
					xBeam*cosPaBeam*xBeam*cosPaBeam + yBeam*sinPaBeam*yBeam*sinPaBeam
				)/(2*signalToNoise);
			latErrorFromSN.convert(latError.getUnit());
			latError.setValue(
					max(latError.getValue(), latErrorFromSN.getValue())
				);
		}
		newShape->setRefDirectionError(latError, longError);
		Vector<Int> index(1, i);
		_curConvolvedList.setShape(index, *newShape);
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
			static_cast<const GaussianShape *>(
				_curDeconvolvedList.getShape(i)->clone()
			)
		);
		std::tr1::shared_ptr<PointShape> point;
		Quantity emaj = gaussShape->majorAxisError();
		Quantity emin = gaussShape->minorAxisError();
		Quantity epa  = gaussShape->positionAngleError();
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
								try {
									isPointSource = beam.deconvolve(decon, sourceIn);
								}
								catch (const AipsError& x) {
									fitSuccess = False;
								}
								if (fitSuccess) {
									Quantity errMaj = abs(bestDecon.getMajor() - decon.getMajor());
									errMaj.convert(emaj.getUnit());
									Quantity errMin = abs(bestDecon.getMinor() - decon.getMinor());
									errMin.convert(emin.getUnit());
									Quantity errPA = abs(bestDecon.getPA(True) - decon.getPA(True));
									errPA.convert("deg");
									errPA.setValue(fmod(errPA.getValue(), 180.0));
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
					bestDecon.getPA(True), True, emaj, emin, epa
				);
				gaussShape->setWidth(
					bestDecon.getMajor(), bestDecon.getMinor(),
					bestDecon.getPA(True)
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

String ImageFitter::_sizeToString(const uInt compNumber) {
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
		/*
		Quantity maj = _majorAxes[compNumber];
		Quantity minor = _minorAxes[compNumber];
		Quantity pa = _positionAngles[compNumber];
		const GaussianShape *gaussShape = static_cast<const GaussianShape *>(compShape);
		Quantity emaj = gaussShape->majorAxisError();
		Quantity emin = gaussShape->minorAxisError();
		Quantity epa  = gaussShape->positionAngleError();
		*/
		size << "Clean beam size ---" << endl;
		// CAS-4577, users want two digits, so just do it explicitly here rather than using
		// TwoSidedShape::sizeToString
		size << std::fixed << std::setprecision(2) << "       --- major axis FWHM: " << beam.getMajor() << endl;
		size << "       --- minor axis FWHM: " << beam.getMinor() << endl;
		size << "       --- position angle: " << beam.getPA(True) << endl;
		/*
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
		*/
		size << "Image component size (deconvolved from beam) ---" << endl;
		size << _deconvolvedMessages[compNumber];
		/*
		Angular2DGaussian decon;
		if(fitSuccess) {
			if (isPointSource) {
				_curDeconvolvedList.setShape(Vector<Int>(1, compNumber), PointShape());
                Angular2DGaussian largest(
                	maj + emaj,
					minor + emin,
					pa - epa
				);
				size << "    Component is a point source" << endl;
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
                	}
                	else {
                		size << "    It may be as large as " << std::setprecision(2) << lsize.getMajor()
                	        << " x " << lsize.getMinor() << endl;
                	}
                }
                else {
                	if (isPointSource1) {
                		size << "    It may be as large as " << std::setprecision(2) << decon.getMajor()
                	        << " x " << decon.getMinor() << endl;
                	}
                	else {
                		Quantity lmaj = max(decon.getMajor(), lsize.getMajor());
                		Quantity lmin = max(decon.getMinor(), lsize.getMinor());
                		size << "    It may be as large as " << std::setprecision(2) << lmaj
                		    << " x " << lmin << endl;
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
								try {
									isPointSource = beam.deconvolve(decon, sourceIn);
								}
								catch (const AipsError& x) {
									fitSuccess = False;
								}
								if (fitSuccess) {
									Quantity errMaj = abs(bestDecon.getMajor() - decon.getMajor());
									errMaj.convert(emaj.getUnit());
									Quantity errMin = abs(bestDecon.getMinor() - decon.getMinor());
									errMin.convert(emin.getUnit());
									Quantity errPA = abs(bestDecon.getPA(True) - decon.getPA(True));
									errPA.convert("deg");
									errPA.setValue(fmod(errPA.getValue(), 180.0));
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
					bestDecon.getPA(True), True, emaj, emin, epa
				);
			}
		}
		else {
			size << "    Could not deconvolve source from beam. "
				<< "Source may be (only marginally) resolved in only one direction.";
		}
		*/
	}
	return size.str();
}

String ImageFitter::_fluxToString(uInt compNumber) const {
	Vector<String> unitPrefix(8);
	unitPrefix[0] = "T";
	unitPrefix[1] = "G";
	unitPrefix[2] = "M";
	unitPrefix[3] = "k";
	unitPrefix[4] = "";
	unitPrefix[5] = "m";
	unitPrefix[6] = "u";
	unitPrefix[7] = "n";

	ostringstream fluxes;
	Quantity fluxDensity = _fluxDensities[compNumber];
	Quantity fluxDensityError = _fluxDensityErrors[compNumber];
	Vector<String> polarization = _curConvolvedList.getStokes(compNumber);
	Quantity intensityToFluxConversion = _bUnit.contains("/beam")
	    	? Quantity(1.0, "beam")
	    	: Quantity(1.0, "pixel");

	String baseUnit = "Jy";
	Bool hasTemperatureUnits = fluxDensity.isConform("K*rad2");
	if (hasTemperatureUnits) {
		String areaUnit = "beam";
		if (_getImage()->imageInfo().hasBeam()) {
			Double beamArea = _getImage()->imageInfo().restoringBeam(
				_chanPixNumber, _stokesPixNumber
			).getArea("rad2");
			fluxDensity /= Quantity(beamArea, "rad2");
			fluxDensity.setUnit("K.beam");
			fluxDensityError /= Quantity(beamArea, "rad2");
			fluxDensityError.setUnit("K.beam");
			areaUnit = "beam";
		}
		else {
			if (_minorAxes[compNumber] > Quantity(1.0, "rad")) {
				areaUnit = "rad2";
			}
			else if (_minorAxes[compNumber] > Quantity(1.0, "deg")) {
				areaUnit = "deg2";
			}
			else if (_minorAxes[compNumber] > Quantity(1.0, "arcmin")) {
				areaUnit = "arcmin2";
			}
			else if (_minorAxes[compNumber] > Quantity(1.0, "arcsec")) {
				areaUnit = "arcsec2";
			}
		}
		baseUnit = "K." + areaUnit;
		intensityToFluxConversion.setUnit(areaUnit);
	}
	String usedPrefix;
	String unit;
	for (uInt i=0; i<unitPrefix.size(); i++) {
		usedPrefix = unitPrefix[i];
		unit = usedPrefix + baseUnit;
		if (fluxDensity.getValue(unit) > 1) {
			fluxDensity.convert(unit);
			fluxDensityError.convert(unit);
			break;
		}
	}
	Vector<Double> fd(2);
	fd[0] = fluxDensity.getValue();
	fd[1] = fluxDensityError.getValue();
	Quantity peakIntensity = _peakIntensities[compNumber];
	Quantity tmpFlux = peakIntensity * intensityToFluxConversion;
	tmpFlux.convert(baseUnit);

	Quantity peakIntensityError = _peakIntensityErrors[compNumber];
	Quantity tmpFluxError = peakIntensityError * intensityToFluxConversion;
	uInt precision = 0;
	fluxes << "Flux ---" << endl;

	if (! _noBeam) {
		precision = precisionForValueErrorPairs(fd, Vector<Double>());
		fluxes << std::fixed << setprecision(precision);
		fluxes << "       --- Integrated:   " << fluxDensity.getValue();
		if (
			_fixed[compNumber].contains("f") && _fixed[compNumber].contains("a")
			&& _fixed[compNumber].contains("b")
		) {
			fluxes << " " << fluxDensity.getUnit() << " (fixed)" << endl;
		}
		else {
			fluxes << " +/- " << fluxDensityError << endl;
		}
	}
	for (uInt i=0; i<unitPrefix.size(); i++) {
		usedPrefix = unitPrefix[i];
		String unit = usedPrefix + tmpFlux.getUnit();
		if (tmpFlux.getValue(unit) > 1) {
			tmpFlux.convert(unit);
			tmpFluxError.convert(unit);
			break;
		}
	}
	//peakIntensity = tmpFlux/intensityToFluxConversion;
	peakIntensity = Quantity(
		tmpFlux.getValue(),
		tmpFlux.getUnit() + "/" + intensityToFluxConversion.getUnit()
	);
	peakIntensityError = Quantity(tmpFluxError.getValue(), peakIntensity.getUnit());
	if (hasTemperatureUnits) {
		peakIntensity.setUnit(usedPrefix + "K");
		peakIntensityError.setUnit(usedPrefix + "K");
	}
	Vector<Double> pi(2);
	pi[0] = peakIntensity.getValue();
	pi[1] = peakIntensityError.getValue();
	precision = precisionForValueErrorPairs(pi, Vector<Double>());
	fluxes << std::fixed << setprecision(precision);
	fluxes << "       --- Peak:         " << peakIntensity.getValue();
	if (_fixed[compNumber].contains("f")) {
		fluxes << " " << peakIntensity.getUnit() << " (fixed)" << endl;
	}
	else {
		fluxes << " +/- " << peakIntensityError << endl;
	}
	fluxes << "       --- Polarization: " << _getStokes() << endl;
	return fluxes.str();
}

String ImageFitter::_spectrumToString(uInt compNumber) const {
	Vector<String> unitPrefix(9);
	unitPrefix[0] = "T";
	unitPrefix[1] = "G";
	unitPrefix[2] = "M";
	unitPrefix[3] = "k";
	unitPrefix[4] = "";
	unitPrefix[5] = "c";
	unitPrefix[6] = "m";
	unitPrefix[7] = "u";
	unitPrefix[8] = "n";
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

SubImage<Float> ImageFitter::_createImageTemplate() const {
	std::auto_ptr<ImageInterface<Float> > imageClone(_getImage()->cloneII());

	return SubImageFactory<Float>::createSubImage(
		*imageClone, *_getRegion(), _getMask(), 0,
		False, AxesSpecifier(), _getStretch()
	);
}

void ImageFitter::_writeNewEstimatesFile() const {
	ostringstream out;
	uInt ndim = _getImage()->ndim();
	Vector<Int> dirAxesNumbers = _getImage()->coordinates().directionAxesNumbers();
	Vector<Double> world(ndim,0), pixel(ndim,0);
	_getImage()->coordinates().toWorld(world, pixel);

	uInt n = _curConvolvedList.nelements();
	for (uInt i=0; i<n; i++) {
		MDirection mdir = _curConvolvedList.getRefDirection(i);
		Quantity lat = mdir.getValue().getLat("rad");
		Quantity longitude = mdir.getValue().getLong("rad");
		world[dirAxesNumbers[0]] = longitude.getValue();
		world[dirAxesNumbers[1]] = lat.getValue();
		if (_getImage()->coordinates().toPixel(pixel, world)) {
			out << _peakIntensities[i].getValue() << ", "
					<< pixel[0] << ", " << pixel[1] << ", "
					<< _majorAxes[i] << ", " << _minorAxes[i] << ", "
					<< _positionAngles[i] << endl;
		}
		else {
			*_getLog() << LogIO::WARN << "Unable to calculate pixel location of "
					<< "component number " << i << " so cannot write new estimates"
					<< "file" << LogIO::POST;
			return;
		}
	}
	String output = out.str();
	File estimates(_newEstimatesFileName);
	String action = (estimates.getWriteStatus() == File::OVERWRITABLE) ? "Overwrote" : "Created";
	LogFile newEstimates(_newEstimatesFileName);
	newEstimates.write(output, True, True);
	*_getLog() << LogIO::NORMAL << action << " file "
		<< _newEstimatesFileName << " with new estimates file"
		<< LogIO::POST;
}

void ImageFitter::_writeCompList(ComponentList& list) const {
	if (! _compListName.empty()) {
		switch (_writeControl) {
		case NO_WRITE:
			return;
		case WRITE_NO_REPLACE:
		{
			File file(_compListName);
			if (file.exists()) {
				LogOrigin logOrigin(_class, __FUNCTION__);
				*_getLog() << logOrigin;
				*_getLog() << LogIO::WARN << "Requested persistent component list " << _compListName
						<< " already exists and user does not wish to overwrite it so "
						<< "the component list will not be written" << LogIO::POST;
				return;
			}
		}
		// allow fall through
		case OVERWRITE: {
			Path path(_compListName);
			list.rename(path, Table::New);
			*_getLog() << LogIO::NORMAL << "Wrote component list table " << _compListName << endl;
		}
		return;
		default:
			// should never get here
			return;
		}
	}
}


// TODO From here until the end of the file is code extracted directly
// from ImageAnalysis. It is in great need of attention.

void ImageFitter::_fitsky(
	Fit2D& fitter, Array<Float>& pixels, Array<Bool>& pixelMask,
	Bool& converged, Double& zeroLevelOffsetSolution,
   	Double& zeroLevelOffsetError,
	const Vector<String>& models,
	const Bool fitIt, const Bool deconvolveIt, const Bool list,
	const Double zeroLevelEstimate
) {
	LogOrigin origin(_class, __FUNCTION__);
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
	if (nModels == 0) {
		*_getLog() << "You have not specified any models" << LogIO::EXCEPTION;
	}
	if (nGauss > 1 && estimate.nelements() < nGauss) {
		*_getLog() << "You must specify one estimate for each model component"
			<< LogIO::EXCEPTION;
	}
	if (!fitIt && nModels > 1) {
		*_getLog() << "Parameter estimates are only available for a single Gaussian model"
			<< LogIO::EXCEPTION;
	}
	SubImage<Float> subImageTmp;
	{
		std::auto_ptr<ImageInterface<Float> > imageClone(_getImage()->cloneII());
		subImageTmp = SubImageFactory<Float>::createSubImage(
			*imageClone, *_getRegion(), _getMask(),
			(list ? _getLog().get() : 0), False,
			AxesSpecifier(True), _getStretch()
		);
	}
	SubImage<Float> allAxesSubImage;
	{
		IPosition imShape = subImageTmp.shape();
		IPosition startPos(imShape.nelements(), 0);
		// Pass in an IPosition here to the constructor
		// this will subtract 1 from each element of the IPosition imShape
		IPosition endPos(imShape - 1);
		IPosition stride(imShape.nelements(), 1);
		//const CoordinateSystem& imcsys = pImage_p->coordinates();
		const CoordinateSystem& imcsys = subImageTmp.coordinates();
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
			subImageTmp, slice, False, AxesSpecifier(True)
		);
	}
	// for some things we don't want the degenerate axes,
	// so make a subimage without them as well
	SubImage<Float> subImage = SubImage<Float>(
		allAxesSubImage, AxesSpecifier(False)
	);

    // Make sure the region is 2D and that it holds the sky.  Exception if not.
	const CoordinateSystem& cSys = subImage.coordinates();
	Bool xIsLong = CoordinateUtil::isSky(*_getLog(), cSys);
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
	if (nEstimates == 0 && nGauss > 1) {
		*_getLog() << "Can only auto estimate for a gaussian model"
			<< LogIO::EXCEPTION;
	}
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
	// Do fit
	Array<Float> sigma;
	// residMask constant so do not recalculate out_pixelmask
	Fit2D::ErrorTypes status = fitter.fit(pixels, pixelMask, sigma);
	*_getLog() << LogOrigin(_class, __FUNCTION__);

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
	for (uInt i = 0; i < models.nelements(); i++) {
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
			if (anyLT(errors, 0.0)) {
				throw AipsError(
					"At least one calculated error is less than zero"
				);
			}
			result(j) = ImageUtilities::encodeSkyComponent(
				*_getLog(), facToJy, allAxesSubImage, modelType,
				solution, stokes, xIsLong, deconvolveIt,
				_getImage()->imageInfo().restoringBeam(_chanPixNumber, _stokesPixNumber)
			);
			String error;
			Record r;
			result(j).flux().toRecord(error, r);

            try {
                _encodeSkyComponentError(
				    result(j), facToJy, allAxesSubImage,
				    solution, errors, stokes, xIsLong
			    );
            }
            catch (const AipsError& x) {
                *_getLog() << "POTENTIAL DEFECT: Fitter converged but exception caught in post processing. "
                    << "This may be a bug. Conact us with the image and the input parameters "
                    << "you used and we will have a look." << LogIO::EXCEPTION;
            }
			_curConvolvedList.add(result(j));
			if (_getImage()->imageInfo().hasBeam()) {
				_curDeconvolvedList.add(result(j).copy());
			}
			j++;
		}
	}
}

Vector<Double> ImageFitter::_singleParameterEstimate(
	Fit2D& fitter, Fit2D::Types model, const MaskedArray<Float>& pixels,
	Float minVal, Float maxVal, const IPosition& minPos,
	const IPosition& maxPos
) const {
	// position angle +x -> +y

	// Return the initial fit guess as either the model, an auto guess,
	// or some combination.
	*_getLog() << LogOrigin(_class, __FUNCTION__);
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
		} else if (parameters.nelements() != 6) {
			*_getLog() << "Not enough parameters returned by fitter estimate"
					<< LogIO::EXCEPTION;
		}
	} else {
		// points, levels etc
		*_getLog() << "Only Gaussian/Disk auto-single estimates are available"
				<< LogIO::EXCEPTION;
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
		SkyComponent& sky, Double facToJy, const ImageInterface<Float>& subIm,
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
	const CoordinateSystem& cSys = subIm.coordinates();
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
			dParameters(2) = errors(3) == 0 ? 5e-14 : errors(3);
			// Minor
			dParameters(3) = errors(4) == 0 ? 5e-14 : errors(4);
			// PA
			dParameters(4) = parameters(5);
			// If flipped, it means pixel major axis morphed into world minor
			// Put back any zero errors as well.
			Bool flipped = ImageUtilities::pixelWidthsToWorld(
				wParameters,
				dParameters, cSys, pixelAxes, False
			);
			Quantum<Double> paErr(errors(5), Unit(String("rad")));
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
			cSys, pixelAxes, False
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

}



