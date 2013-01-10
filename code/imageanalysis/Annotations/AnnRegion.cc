#include <imageanalysis/Annotations/AnnRegion.h>

#include <casa/Exceptions/Error.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <images/Regions/WCExtension.h>
#include <images/Regions/WCUnion.h>
#include <measures/Measures/VelocityMachine.h>
#include <tables/Tables/TableRecord.h>

#include <iomanip>

namespace casa {

const String AnnRegion::_class = "AnnRegion";

AnnRegion::AnnRegion(
	const Type shape, const String& dirRefFrameString,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Quantity& beginFreq,
	const Quantity& endFreq,
	const String& freqRefFrameString,
	const String& dopplerString,
	const Quantity& restfreq,
	const Vector<Stokes::StokesTypes> stokes,
	const Bool annotationOnly
) : AnnotationBase(shape, dirRefFrameString, csys), _isAnnotationOnly(annotationOnly),
	_convertedFreqLimits(Vector<MFrequency>(0)), _stokes(stokes),
	_isDifference(False), _constructing(True), _imShape(imShape),
	 _spectralPixelRange(vector<Double>(0)) {
	_init();
	_setFrequencyLimits(
		beginFreq, endFreq, freqRefFrameString,
		dopplerString, restfreq
	);
	// right before returning
	_constructing = False;
}

AnnRegion::AnnRegion(
	const Type shape,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) :	AnnotationBase(shape, csys),
	_convertedFreqLimits(Vector<MFrequency>(0)),
	_beginFreq(Quantity(0, "Hz")), _endFreq(Quantity(0, "Hz")),
	_restFreq(Quantity(0, "Hz")), _stokes(stokes),
	_isDifference(False), _constructing(True), _imShape(imShape),
	 _spectralPixelRange(vector<Double>(0)) {
	_init();
	// right before returning
	_constructing = False;
}

AnnRegion::~AnnRegion() {}

AnnRegion& AnnRegion::operator= (const AnnRegion& other) {
    if (this == &other) {
    	return *this;
    }
    AnnotationBase::operator= (other);
    _isAnnotationOnly = other._isAnnotationOnly;
    _convertedFreqLimits.resize(other._convertedFreqLimits.nelements());
    _convertedFreqLimits = other._convertedFreqLimits;
    _imageRegion = other._imageRegion;
    _beginFreq = other._beginFreq;
    _endFreq = other._endFreq;
    _restFreq = other._restFreq;
    _stokes.resize(other._stokes.nelements());
    _stokes = other._stokes;
    _freqRefFrame = other._freqRefFrame;
    _dopplerType = other._dopplerType;
    _isDifference = other._isDifference;
    _directionRegion = other._directionRegion;
    _constructing = other._constructing;
    _imShape = other._imShape;
    _spectralPixelRange = other._spectralPixelRange;
    return *this;
}

void AnnRegion::_setFrequencyLimits(
	const Quantity& beginFreq,
	const Quantity& endFreq,
	const String& freqRefFrame,
	const String& dopplerString,
	const Quantity& restfreq
) {
	String preamble(_class + ": " + String(__FUNCTION__) + ": ");
    if (beginFreq.getValue() == 0 && endFreq.getValue() == 0) {
        return;
    }
	if (! getCsys().hasSpectralAxis()) {
		return;
	}
    if ( beginFreq.getUnit().empty() && endFreq.getUnit().empty()) {
        throw AipsError(
            preamble + "Neither frequency specified has units. Both must"
        );
    }
	if (! beginFreq.getUnit().empty() && endFreq.getUnit().empty()) {
		throw AipsError(
			preamble + "beginning frequency specified but ending frequency not. "
			+ "Both must specified or both must be unspecified."
		);
	}
	if (beginFreq.getUnit().empty() && ! endFreq.getUnit().empty()) {
		throw AipsError(
			preamble + "ending frequency specified but beginning frequency not. "
			+ "Both must specified or both must be unspecified."
		);
	}
	if (! beginFreq.getUnit().empty()) {
		if (! beginFreq.isConform(endFreq)) {
			throw AipsError(
				preamble + "Beginning freq units (" + beginFreq.getUnit()
				+ ") do not conform to ending freq units (" + endFreq.getUnit()
				+ ") but they must."
			);
		}

		if (
			! beginFreq.isConform("Hz")
			&& ! beginFreq.isConform("m/s")
			&& ! beginFreq.isConform("pix")
		) {
			throw AipsError(
				preamble
				+ "Invalid frequency unit " + beginFreq.getUnit()
			);
		}
		if (beginFreq.isConform("m/s") && restfreq.getValue() <= 0) {
			throw AipsError(
				preamble
				+ "Beginning and ending velocities supplied but no restfreq specified"
			);
		}
		if (freqRefFrame.empty()) {
			_freqRefFrame = getCsys().spectralCoordinate().frequencySystem();
		}
		else if (! MFrequency::getType(_freqRefFrame, freqRefFrame)) {
			throw AipsError(
				preamble
				+ "Unknown frequency frame code "
				+ freqRefFrame
			);
		}
		else {
			_setParam(AnnotationBase::FRAME, freqRefFrame);
		}
		if (dopplerString.empty()) {
			_dopplerType = getCsys().spectralCoordinate().velocityDoppler();
		}
		else if (! MDoppler::getType(_dopplerType, dopplerString)) {
			throw AipsError(
				preamble + "Unknown doppler code " + dopplerString
			);
		}
		else {
			_setParam(AnnotationBase::VELTYPE, dopplerString);
		}
		_beginFreq = beginFreq;
		_endFreq = endFreq;
		_restFreq = restfreq;
		_setParam(AnnotationBase::RANGE, _printFreqRange());
		_setParam(AnnotationBase::RESTFREQ, _printFreq(_restFreq));

		_checkAndConvertFrequencies();
		if (! _constructing) {
			// have to re-extend the direction region because of new freq range.
			// but not during object construction
			_extend();
		}
	}
}

void AnnRegion::setAnnotationOnly(const Bool isAnnotationOnly) {
	_isAnnotationOnly = isAnnotationOnly;
}

Bool AnnRegion::isAnnotationOnly() const {
	return _isAnnotationOnly;
}

void AnnRegion::setDifference(const Bool difference) {
	_isDifference = difference;
}

Bool AnnRegion::isDifference() const {
	return _isDifference;
}

Vector<MFrequency> AnnRegion::getFrequencyLimits() const {
	return _convertedFreqLimits;
}

Vector<Stokes::StokesTypes> AnnRegion::getStokes() const {
	return _stokes;
}

TableRecord AnnRegion::asRecord() const {
	return _imageRegion.toRecord("");
}

ImageRegion AnnRegion::asImageRegion() const {
	return _imageRegion;
}

WCRegion*  AnnRegion::getRegion() const {
	return _imageRegion.asWCRegionPtr()->cloneRegion();
}

Bool AnnRegion::isRegion() const {
	return True;
}

void AnnRegion::_init() {
	if (_imShape.nelements() != getCsys().nPixelAxes()) {
		ostringstream oss;
		oss << _class << "::" << __FUNCTION__ << ": Number of coordinate axes ("
			<< getCsys().nPixelAxes() << ") differs from number of dimensions in image shape ("
			<< _imShape.nelements() << ")";
		throw AipsError(oss.str());
	}
	uInt size = _stokes.size();

	if (size > 0) {
		ostringstream os;
		os << "[";
		for (uInt i=0; i< size; i++) {
			os << Stokes::name(_stokes[i]);
			if (i != _stokes.size() - 1) {
				os << ", ";
			}
		}
		os << "]";
		_setParam(AnnotationBase::CORR, os.str());
	}
}

void AnnRegion::_setDirectionRegion(const ImageRegion& region) {
	_directionRegion = region;
}

vector<Double> AnnRegion::getSpectralPixelRange() const {
	return _spectralPixelRange;
}

void AnnRegion::_extend() {
	Int stokesAxis = -1;
	Int spectralAxis = -1;
	Vector<Quantity> freqRange;
	uInt nBoxes = 0;
    if (getCsys().hasSpectralAxis() && _convertedFreqLimits.size() == 2) {
		SpectralCoordinate spcoord = getCsys().spectralCoordinate();
		String unit = spcoord.worldAxisUnits()[0];
		_spectralPixelRange.resize(2);
		spcoord.toPixel(_spectralPixelRange[0], _convertedFreqLimits[0]);
		spcoord.toPixel(_spectralPixelRange[1], _convertedFreqLimits[1]);
        freqRange.resize(2);
        freqRange[0] = _convertedFreqLimits[0].get(unit);
        freqRange[1] = _convertedFreqLimits[1].get(unit);
		if (_spectralPixelRange[1] <= _spectralPixelRange[0]) {
			std::swap(_spectralPixelRange[0], _spectralPixelRange[1]);
			std::swap(freqRange[0], freqRange[1]);
		}
		spectralAxis = getCsys().spectralAxisNumber();
		nBoxes = 1;
	}
	vector<Stokes::StokesTypes> stokesRanges;
	if (
		getCsys().hasPolarizationCoordinate() && _stokes.size() > 0
		&& (stokesAxis = getCsys().polarizationAxisNumber()) >= 0
	) {
		vector<uInt> stokesNumbers(2*_stokes.size());
		for (uInt i=0; i<_stokes.size(); i++) {
			stokesNumbers[2*i] = (uInt)_stokes[i];
			stokesNumbers[2*i + 1] = stokesNumbers[2*i];
		}
		vector<uInt> orderedRanges = CasacRegionManager::consolidateAndOrderRanges(
			stokesNumbers
		);
		for (uInt i=0; i<orderedRanges.size(); i++) {
			stokesRanges.push_back(Stokes::type(orderedRanges[i]));
		}
		nBoxes = stokesRanges.size()/2;
	}
	if (nBoxes == 0) {
		_imageRegion = _directionRegion;
	}
	else {
		uInt nExtendAxes = 0;
		if (spectralAxis >= 0) {
			nExtendAxes++;
		}
		if (stokesAxis >= 0) {
			nExtendAxes++;
		}

		IPosition pixelAxes(nExtendAxes);
		uInt n = 0;
		// spectral axis must be first to be consistent with _makeExtensionBox()
		if (spectralAxis > 0) {
			pixelAxes[n] = spectralAxis;
			n++;
		}
		if (stokesAxis > 0) {
			pixelAxes[n] = stokesAxis;
			n++;
		}
		if (nBoxes == 1) {
			WCBox wbox = _makeExtensionBox(freqRange, stokesRanges, pixelAxes);
			_imageRegion = ImageRegion(WCExtension(_directionRegion, wbox));
		}
		else {
			PtrBlock<const WCRegion*> regions(nBoxes);
			for (uInt i=0; i<nBoxes; i++) {
				Vector<Stokes::StokesTypes> stokesRange(2);
				stokesRange[0] = stokesRanges[2*i];
				stokesRange[1] = stokesRanges[2*i + 1];
				WCBox wbox = _makeExtensionBox(freqRange, stokesRange, pixelAxes);
				regions[i] = new WCExtension(_directionRegion, wbox);
			}
			_imageRegion = ImageRegion(WCUnion(True, regions));
		}
	}
	try {
		_imageRegion.asWCRegionPtr()->toLCRegion(getCsys(), _imShape);
	}
	catch (const AipsError& x) {
		throw (ToLCRegionConversionError(x.getMesg()));
	}
}

WCBox AnnRegion::_makeExtensionBox(
	const Vector<Quantity>& freqRange,
	const Vector<Stokes::StokesTypes>& stokesRange,
	const IPosition& pixelAxes
) const {
	uInt n = 0;
	Vector<Quantity> blc(pixelAxes.size());
	Vector<Quantity> trc(pixelAxes.size());
	Vector<Int> absRel(pixelAxes.size(), RegionType::Abs);
	if (freqRange.size() == 2) {
		blc[n] = freqRange[0];
		trc[n] = freqRange[1];
		n++;
	}
	if (stokesRange.size() == 2) {
		blc[n] = Quantity(stokesRange[0], "");
		trc[n] = Quantity(stokesRange[1], "");
	}
	WCBox wbox(blc, trc, pixelAxes, getCsys(), absRel);
	return wbox;
}

void AnnRegion::_checkAndConvertFrequencies() {
    MFrequency::Types cFrameType = getCsys().spectralCoordinate().frequencySystem(False);
	MDoppler::Types cDopplerType = getCsys().spectralCoordinate().velocityDoppler();
	_convertedFreqLimits.resize(2);
    for (Int i=0; i<2; i++) {
		Quantity qFreq = i == 0 ? _beginFreq : _endFreq;
		if (qFreq.getUnit() == "pix") {
			Int spectralAxisNumber = getCsys().spectralAxisNumber();
			Vector<Double> pixel = getCsys().referencePixel();
			pixel[spectralAxisNumber] = qFreq.getValue();
			Vector<Double> world;
			getCsys().toWorld(world, pixel);
			String unit = getCsys().worldAxisUnits()[spectralAxisNumber];
			if (_freqRefFrame != cFrameType) {
				LogIO log;
				log << LogOrigin(String(__FUNCTION__)) << LogIO::WARN
					<< ": Frequency range given in pixels but supplied frequency ref frame ("
					<< MFrequency::showType(_freqRefFrame) << ") differs from that of "
					<< "the provided coordinate system (" << MFrequency::showType(cFrameType)
					<< "). The provided frequency range will therefore be assumed to already "
					<< "be in the coordinate system frequency reference frame and no conversion "
					<< "will be done" << LogIO::POST;
			}
			if (_dopplerType != cDopplerType) {
				LogIO log;
				log << LogOrigin(String(__FUNCTION__)) << LogIO::WARN
					<< ": Frequency range given in pixels but supplied doppler type ("
					<< MDoppler::showType(_dopplerType) << ") differs from that of "
					<< "the provided coordinate system (" << MDoppler::showType(cDopplerType)
					<< "). The provided frequency range will therefore be assumed to already "
					<< "be in the coordinate system doppler and no conversion "
					<< "will be done" << LogIO::POST;
			}
			_freqRefFrame = cFrameType;
			_dopplerType = cDopplerType;
			_convertedFreqLimits[i] = MFrequency(
				Quantity(world[spectralAxisNumber], unit),
				_freqRefFrame
			);
		}
		else if (qFreq.isConform("m/s")) {
			MFrequency::Ref freqRef(_freqRefFrame);
			MDoppler::Ref velRef(_dopplerType);
			VelocityMachine vm(freqRef, Unit("GHz"),
				MVFrequency(_restFreq),
				velRef, qFreq.getUnit()
			);
			qFreq = vm(qFreq);
			_convertedFreqLimits[i] = MFrequency(qFreq, _freqRefFrame);
			if (_dopplerType != cDopplerType) {
				MDoppler dopplerConversion = MDoppler::Convert(_dopplerType, cDopplerType)();
				_convertedFreqLimits[i] = MFrequency::fromDoppler(
					dopplerConversion,
					_convertedFreqLimits[i].get("Hz"), cFrameType
				);
			}
		}
		else if ( qFreq.isConform("Hz")) {
			_convertedFreqLimits[i] = MFrequency(qFreq, _freqRefFrame);
		}
		else {
			throw AipsError("Logic error. Bad spectral unit "
				+ qFreq.getUnit()
				+ " somehow made it to a place where it shouldn't have"
			);
		}
		if (_freqRefFrame != cFrameType) {
			Vector<Double> refDirection = getCsys().directionCoordinate().referenceValue();
			Vector<String> directionUnits = getCsys().directionCoordinate().worldAxisUnits();
			MDirection refDir(
				Quantity(refDirection[0], directionUnits[0]),
				Quantity(refDirection[1], directionUnits[1]),
				getCsys().directionCoordinate().directionType()
			);
			MFrequency::Ref inFrame(_freqRefFrame, MeasFrame(refDir));
			MFrequency::Ref outFrame(cFrameType, MeasFrame(refDir));
			MFrequency::Convert converter(inFrame, outFrame);
			_convertedFreqLimits[i] = converter(_convertedFreqLimits[i]);
		}
	}
}

Quantity AnnRegion::_lengthToAngle(
	const Quantity& quantity, const uInt pixelAxis
) const {
	if(quantity.getUnit() == "pix") {
		return getCsys().toWorldLength(quantity.getValue(), pixelAxis);
	}
	else if (! quantity.isConform("rad")) {
		throw AipsError (
			"Quantity " + String::toString(quantity)
			+ " is not an angular measure nor is it in pixel units."
		);
	}
	else {
		return quantity;
	}
}

void AnnRegion::_printPrefix(ostream& os) const {
	if (isAnnotationOnly()) {
		os << "ann ";
	}
	else if (isDifference()) {
		os << "- ";
	}
}

String AnnRegion::_printFreqRange() const {
	ostringstream os;
	os << "["
		<< _printFreq(_beginFreq) << ", "
		<< _printFreq(_endFreq) << "]";
	return os.str();
}

String AnnRegion::_printFreq(const Quantity& freq) {
	if (freq.getUnit() == "pix") {
		return _printPixel(freq.getValue());
	}
	ostringstream os;
	os << std::fixed;
	if (freq.isConform("km/s")) {
		os << std::setprecision(4) << freq.getValue("km/s") << "km/s";
	}
	else {
		os << std::setprecision(3) << freq.getValue("MHz") << "MHz";
	}
	return os.str();


}


}

