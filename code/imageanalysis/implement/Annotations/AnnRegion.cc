//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

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

namespace casa {

AnnRegion::AnnRegion(
	const Type shape, const String& dirRefFrameString,
	const CoordinateSystem& csys,
	const Quantity& beginFreq,
	const Quantity& endFreq,
	const String& freqRefFrameString,
	const String& dopplerString,
	const Quantity& restfreq,
	const Vector<Stokes::StokesTypes> stokes,
	const Bool annotationOnly
) : AnnotationBase(shape, dirRefFrameString, csys), _isAnnotationOnly(annotationOnly),
	_convertedFreqLimits(Vector<MFrequency>(0)),
	_beginFreq(beginFreq), _endFreq(endFreq), _restFreq(restfreq),
	_stokes(stokes), _wcRegion(0), _isDifference(False) {
	String preamble(String(__FUNCTION__) + ": ");
	if (_csys.hasSpectralAxis()) {
		if (! _beginFreq.getUnit().empty() > 0 && _endFreq.getUnit().empty()) {
			throw AipsError(
				preamble + "beginning frequency specified but ending frequency not. "
				+ "Both must either be specified or not specified."
			);
		}
		if (_beginFreq.getUnit().empty() && ! _endFreq.getUnit().empty()) {
			throw AipsError(
				preamble + "ending frequency specified but beginning frequency not. "
				+ "Both must either be specified or not specified."
			);
		}
		if (! _beginFreq.getUnit().empty()) {
			if (! _beginFreq.isConform(_endFreq)) {
				throw AipsError(
					preamble + "Beginning freq units (" + _beginFreq.getUnit()
					+ ") do not conform to ending freq units (" + _endFreq.getUnit()
					+ ") but they must."
				);
			}

			if (
				! _beginFreq.isConform("Hz")
				&& ! _beginFreq.isConform("m/s")
				&& ! _beginFreq.isConform("pix")
			) {
				throw AipsError(
					preamble
					+ "Invalid frequency unit " + beginFreq.getUnit()
				);
			}
			if (_beginFreq.isConform("m/s") && _restFreq.getValue() <= 0) {
				throw AipsError(
					preamble
					+ "Beginning and ending velocities supplied but no restfreq specified"
				);
			}
			if (! MFrequency::getType(_freqRefFrame, freqRefFrameString)) {
				throw AipsError(
					preamble
					+ "Unknown frequency frame code "
					+ freqRefFrameString
				);
			}
			if (! MDoppler::getType(_dopplerType, dopplerString)) {
				throw AipsError(
					preamble
					+ "Unknown doppler code "
					+ dopplerString
				);
			}
			_checkAndConvertFrequencies();
		}
	}
}

AnnRegion::~AnnRegion() {}

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

void AnnRegion::_extend(const ImageRegion& region) {
	Int stokesAxis = -1;
	Int spectralAxis = -1;
	Vector<Quantity> freqRange;
	uInt nBoxes = 0;
	if (_csys.hasSpectralAxis() && _convertedFreqLimits.size() == 2) {
		Quantity begin = _convertedFreqLimits[0].get("Hz");
		Quantity end = _convertedFreqLimits[1].get("Hz");
		freqRange.resize(2);
		freqRange[0] = begin;
		freqRange[1] = end;
		spectralAxis = _csys.spectralAxisNumber();
		nBoxes = 1;
	}
	vector<Stokes::StokesTypes> stokesRanges;
	if (_csys.hasPolarizationAxis() && _stokes.size() > 0) {
		StokesCoordinate stokesCoord = _csys.stokesCoordinate(
			_csys.polarizationCoordinateNumber()
		);
		vector<uInt> stokesNumbers(2*_stokes.size());
		for (uInt i=0; i<_stokes.size(); i++) {
			stokesNumbers[2*i] = (uInt)_stokes[i];
			stokesNumbers[2*i + 1] = stokesNumbers[2*i];
		}
		vector<uInt> orderedRanges = CasacRegionManager::consolidateAndOrderRanges(
			stokesNumbers
		);
		// stokesRanges.resize(orderedRanges.size());
		for (uInt i=0; i<orderedRanges.size(); i++) {
			stokesRanges.push_back(Stokes::type(orderedRanges[i]));
		}
		stokesAxis = _csys.polarizationAxisNumber();
		nBoxes = stokesRanges.size()/2;
	}
	if (nBoxes == 0) {
		_imageRegion = region;
		return;
	}
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
        _imageRegion = ImageRegion(WCExtension(region, wbox));
        return;
	}
	PtrBlock<const WCRegion*> regions(nBoxes);
	for (uInt i=0; i<nBoxes; i++) {
		Vector<Stokes::StokesTypes> stokesRange(2);
		stokesRange[0] = stokesRanges[2*i];
		stokesRange[1] = stokesRanges[2*i + 1];
		WCBox wbox = _makeExtensionBox(freqRange, stokesRange, pixelAxes);
		regions[i] = new WCExtension(region, wbox);
	}
    _imageRegion = ImageRegion(WCUnion(False, regions));
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
	WCBox wbox(blc, trc, pixelAxes, _csys, absRel);
	return wbox;
}


void AnnRegion::_checkAndConvertFrequencies() {
	MFrequency::Types cFrameType = _csys.spectralCoordinate().frequencySystem(False);
	MDoppler::Types cDopplerType = _csys.spectralCoordinate().velocityDoppler();
	_convertedFreqLimits.resize(2);
	for (Int i=0; i<2; i++) {
		Quantity qFreq = i == 0 ? _beginFreq : _endFreq;
		if (qFreq.getUnit() == "pix") {
			Int spectralAxisNumber = _csys.spectralAxisNumber();
			Vector<Double> pixel = _csys.referencePixel();
			pixel[spectralAxisNumber] = qFreq.getValue();
			Vector<Double> world;
			_csys.toWorld(world, pixel);
			String unit = _csys.worldAxisUnits()[spectralAxisNumber];
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
			return;
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
			Vector<Double> refDirection = _csys.directionCoordinate().referenceValue();
			Vector<String> directionUnits = _csys.directionCoordinate().worldAxisUnits();
			MDirection refDir(
				Quantity(refDirection[0], directionUnits[0]),
				Quantity(refDirection[1], directionUnits[1]),
				_csys.directionCoordinate().directionType()
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
		return _csys.toWorldLength(quantity.getValue(), pixelAxis);
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


}

