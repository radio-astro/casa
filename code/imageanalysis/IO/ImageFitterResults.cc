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

#include <imageanalysis/IO/ImageFitterResults.h>

#include <casa/IO/STLIO.h>
#include <casa/OS/File.h>
#include <casa/Quanta/QLogical.h>
#include <casa/Utilities/Precision.h>

#include <images/Images/ImageInterface.h>
#include <imageanalysis/IO/LogFile.h>

#include <iomanip>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_of.hpp>
using namespace boost::assign;

namespace casa {

const String ImageFitterResults::_class = "ImageFitterResults";

vector<String> ImageFitterResults::_prefixesWithCenti = vector<String>();

vector<String> ImageFitterResults::_prefixes = vector<String>();


ImageFitterResults::ImageFitterResults(
	SPCIIF image, std::tr1::shared_ptr<LogIO> log
) : _image(image), _log(log), _bUnit(image->units().getName()) {}

ImageFitterResults::~ImageFitterResults() {}

void ImageFitterResults::writeNewEstimatesFile(const String& filename) const {
	ostringstream out;
	uInt ndim = _image->ndim();
	const CoordinateSystem csys = _image->coordinates();
	Vector<Int> dirAxesNumbers = csys.directionAxesNumbers();
	Vector<Double> world(ndim,0), pixel(ndim,0);
	csys.toWorld(world, pixel);
	*_log << LogOrigin(_class, __func__);
	uInt n = _convolvedList.nelements();
	for (uInt i=0; i<n; i++) {
		MDirection mdir = _convolvedList.getRefDirection(i);
		Quantity lat = mdir.getValue().getLat("rad");
		Quantity longitude = mdir.getValue().getLong("rad");
		world[dirAxesNumbers[0]] = longitude.getValue();
		world[dirAxesNumbers[1]] = lat.getValue();
		if (csys.toPixel(pixel, world)) {
			out << _peakIntensities[i].getValue() << ", "
					<< pixel[0] << ", " << pixel[1] << ", "
					<< _majorAxes[i] << ", " << _minorAxes[i] << ", "
					<< _positionAngles[i] << endl;
		}
		else {
			*_log << LogIO::WARN << "Unable to calculate pixel location of "
				<< "component number " << i << " so cannot write new estimates"
				<< "file" << LogIO::POST;
			return;
		}
	}
	String output = out.str();
	File estimates(filename);
	String action = (estimates.getWriteStatus() == File::OVERWRITABLE) ? "Overwrote" : "Created";
	LogFile newEstimates(filename);
	newEstimates.write(output, True, True);
	*_log << LogIO::NORMAL << action << " file "
		<< filename << " with new estimates file"
		<< LogIO::POST;
}

void ImageFitterResults::writeCompList(
	ComponentList& list, const String& compListName,
	CompListWriteControl writeControl
) const {
	if (compListName.empty()) {
		return;
	}
	switch (writeControl) {
	case NO_WRITE:
		return;
	case WRITE_NO_REPLACE:
	{
		File file(compListName);
		if (file.exists()) {
			LogOrigin logOrigin(_class, __func__);
			*_log << logOrigin;
			*_log << LogIO::WARN << "Requested persistent component list " << compListName
				<< " already exists and user does not wish to overwrite it so "
				<< "the component list will not be written" << LogIO::POST;
			return;
		}
	}
	// allow fall through
	case OVERWRITE: {
		Path path(compListName);
		list.rename(path, Table::New);
		*_log << LogIO::NORMAL << "Wrote component list table "
			<< compListName << LogIO::POST;
	}
	return;
	default:
		// should never get here
		return;
	}
}

String ImageFitterResults::resultsHeader(
	const String& chans, const Vector<uInt>& chanVec,
	const String& region, const String& mask,
	std::tr1::shared_ptr<std::pair<Float, Float> > includePixelRange,
	std::tr1::shared_ptr<std::pair<Float, Float> > excludePixelRange,
	const String& estimates
) const {
	ostringstream summary;
	ostringstream chansoss;
	if (! chans.empty()) {
		chansoss << chans;
	}
	else if (chanVec.size() == 2) {
		if (chanVec[0] == chanVec[1]) {
			chansoss << chanVec[0];
		}
		else {
			chansoss << chanVec[0] << "-" << chanVec[1];
		}
	}
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << "Input parameters ---" << endl;
	summary << "       --- imagename:           " << _image->name() << endl;
	summary << "       --- region:              " << region << endl;
	summary << "       --- channel:             " << chansoss.str() << endl;
	summary << "       --- stokes:              " << _stokes << endl;
	summary << "       --- mask:                " << mask << endl;
	summary << "       --- include pixel range: [";
	if (includePixelRange) {
		ostringstream os;
		os << *includePixelRange;
		summary << os.str();
	}
	summary << "]" << endl;
	summary << "       --- exclude pixel range: [";
	if (excludePixelRange) {
			ostringstream os;
			os << *excludePixelRange;
			summary << os.str();
		}
		summary << "]" << endl;
	if (! estimates.empty()) {
		summary << "       --- initial estimates:   Peak, X, Y, a, b, PA" << endl;
		summary << "                                " << estimates << endl;
	}
	return summary.str();
}

String ImageFitterResults::fluxToString(
	uInt compNumber, uInt chanPixNumber, uInt stokesPixNumber, Bool hasBeam
) const {
	vector<String> unitPrefix = ImageFitterResults::unitPrefixes(False);
	ostringstream fluxes;
	Quantity fluxDensity = _fluxDensities[compNumber];
	Quantity fluxDensityError = _fluxDensityErrors[compNumber];
	Vector<String> polarization = _convolvedList.getStokes(compNumber);
	Quantity intensityToFluxConversion = _bUnit.contains("/beam")
	    ? Quantity(1.0, "beam")
	    : Quantity(1.0, "pixel");

	String baseUnit = "Jy";
	Bool hasTemperatureUnits = fluxDensity.isConform("K*rad2");
	if (hasTemperatureUnits) {
		String areaUnit = "beam";
		if (_image->imageInfo().hasBeam()) {
			Double beamArea = _image->imageInfo().restoringBeam(
				chanPixNumber, stokesPixNumber
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

	if (hasBeam) {
		precision = precisionForValueErrorPairs(fd, Vector<Double>());
		fluxes << std::fixed << std::setprecision(precision);
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
	fluxes << std::fixed << std::setprecision(precision);
	fluxes << "       --- Peak:         " << peakIntensity.getValue();
	if (_fixed[compNumber].contains("f")) {
		fluxes << " " << peakIntensity.getUnit() << " (fixed)" << endl;
	}
	else {
		fluxes << " +/- " << peakIntensityError << endl;
	}
	fluxes << "       --- Polarization: " << _stokes << endl;
	return fluxes.str();
}


vector<String> ImageFitterResults::unitPrefixes(Bool includeCenti) {
	if (_prefixes.empty()) {
		_prefixesWithCenti = list_of("T")("G")("M")("k")("")("c")("m")("u")("n");
		_prefixes = list_of("T")("G")("M")("k")("")("m")("u")("n");

	}
	if (includeCenti) {
		return _prefixesWithCenti;
	}
	else {
		return _prefixes;
	}
}

}
