//# Copyright (C) 1996,1997,1998,1999,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
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
//# $Id:  $

#include <imageanalysis/IO/FitterEstimatesFileParser.h>

#include <casa/aips.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/Utilities/Regex.h>
#include <casa/Containers/Record.h>
#include <components/ComponentModels/ConstantSpectrum.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/GaussianShape.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageStatistics.h>


namespace casa { //# NAMESPACE CASA - BEGIN

FitterEstimatesFileParser::FitterEstimatesFileParser (
		const String& filename,
		const ImageInterface<Float>& image
	) : _componentList(), _fixedValues(0), _log(new LogIO()),
		_peakValues(0), _xposValues(0), _yposValues(0),
		_majValues(0), _minValues(0), _paValues(0), _contents("") {

	RegularFile myFile(filename);
	_log->origin(LogOrigin("FitterEstimatesFileParser","constructor"));

	if (! myFile.exists()) {
		*_log << LogIO::NORMAL << "Estimates file " << filename << " does not exist"
			<< LogIO::EXCEPTION;
	}
	if (! myFile.isReadable()) {
		*_log << LogIO::NORMAL << "Estimates file " << filename << " is not readable"
			<< LogIO::EXCEPTION;
	}
	_parseFile(myFile);
	_createComponentList(image);

}

FitterEstimatesFileParser::~FitterEstimatesFileParser() {}

ComponentList FitterEstimatesFileParser::getEstimates() const {
	return _componentList;
}

Vector<String> FitterEstimatesFileParser::getFixed() const {
	return _fixedValues;
}

String FitterEstimatesFileParser::getContents() const {
	return _contents;
}

void FitterEstimatesFileParser::_parseFile(
	const RegularFile& myFile
) {
	_log->origin(LogOrigin("FitterEstimatesFileParser",__func__));

	RegularFileIO fileIO(myFile);
	// I doubt a genuine estimates file will ever have this many characters
	Int bufSize = 4096;
	char *buffer = new char[bufSize];
	int nRead;

	while ((nRead = fileIO.read(bufSize, buffer, False)) == bufSize) {
		*_log << LogIO::NORMAL << "read: " << nRead << LogIO::POST;
		String chunk(buffer, bufSize);

		_contents += chunk;
	}
	// get the last chunk
	String chunk(buffer, nRead);
	_contents += chunk;

	Vector<String> lines = stringToVector(_contents, '\n');
	Regex blankLine("^[ \n\t\r\v\f]+$",1000);
	uInt componentIndex = 0;
	Vector<String>::iterator end = lines.end();
	String filename = myFile.path().dirName() + "/" + myFile.path().baseName();
	for(Vector<String>::iterator iter=lines.begin(); iter!=end; iter++) {
		if (iter->empty() || iter->firstchar() == '#' ||  iter->matches(blankLine)) {
			// ignore comments and blank lines
			continue;
		}
		uInt commaCount = iter->freq(',');
		ThrowIf(
			commaCount < 5 || commaCount > 6,
			"bad format for line " + *iter
		);
		Vector<String> parts = stringToVector(*iter);
		for (Vector<String>::iterator viter = parts.begin(); viter != parts.end(); viter++) {
			viter->trim();
		}
		String peak = parts[0];
		String xpos = parts[1];
		String ypos = parts[2];
		String maj = parts[3];
		String min = parts[4];
		String pa = parts[5];

		String fixedMask;
		_peakValues.resize(componentIndex + 1, True);
		_xposValues.resize(componentIndex + 1, True);
		_yposValues.resize(componentIndex + 1, True);
		_majValues.resize(componentIndex + 1, True);
		_minValues.resize(componentIndex + 1, True);
		_paValues.resize(componentIndex + 1, True);
		_fixedValues.resize(componentIndex + 1, True);

		ThrowIf(
			! peak.matches(RXdouble),
			"File " + filename + ", line " + *iter
				+ ": peak value " + peak + " is not numeric"
		);
		_peakValues(componentIndex) = String::toDouble(peak);

		if (! xpos.matches(RXdouble) ) {
			*_log << "File " << filename << ", line " << *iter
				<< ": x position value " << xpos << " is not numeric"
				<< LogIO::EXCEPTION;
		}
		_xposValues(componentIndex) = String::toDouble(xpos);

		if (! ypos.matches(RXdouble) ) {
			*_log << "File " << filename << ", line " << *iter
				<< ": y position value " << ypos << " is not numeric"
				<< LogIO::EXCEPTION;
		}
		_yposValues(componentIndex) = String::toDouble(ypos);

		Quantity majQuantity;
		ThrowIf(
			! readQuantity(majQuantity, maj),
			"File " + filename + ", line " + *iter
				+ ": Major axis value " + maj + " is not a quantity"
		);
		_majValues(componentIndex) = majQuantity;

		Quantity minQuantity;
		ThrowIf(
			! readQuantity(minQuantity, min),
			"File " + filename + ", line " + *iter
				+ ": Major axis value " + min + " is not a quantity"
		);
		_minValues(componentIndex) = minQuantity;

		Quantity paQuantity;
		ThrowIf(
			! readQuantity(paQuantity, pa),
			"File " + filename + ", line " + *iter
				+ ": Position angle value " + pa + " is not a quantity"
		);
		_paValues(componentIndex) = paQuantity;

		if (parts.size() == 7) {
			fixedMask = parts[6];
			for (
				String::iterator siter = fixedMask.begin();
				siter != fixedMask.end(); siter++
			) {
				if (
					*siter != 'a' && *siter != 'b' && *siter != 'f'
					&& *siter != 'p' && *siter != 'x' && *siter != 'y'
				) {
					*_log << "fixed parameter ID " << String(*siter) << " is not recognized"
						<< LogIO::EXCEPTION;
				}
			}
			_fixedValues(componentIndex) = fixedMask;
		}
		_fixedValues(componentIndex) = fixedMask;
		componentIndex++;
	}
	ThrowIf(componentIndex == 0, "No valid estmates were found in file " + filename);
}

void FitterEstimatesFileParser::_createComponentList(
	const ImageInterface<Float>& image
) {
	ConstantSpectrum spectrum;
    const CoordinateSystem csys = image.coordinates();
    Vector<Double> pos(2,0);
    Vector<Int> dirAxesNums = csys.directionAxesNumbers();
    Vector<Double> world;
    Int dirCoordNumber = csys.directionCoordinateNumber();
    const DirectionCoordinate& dirCoord = csys.directionCoordinate(
     	dirCoordNumber
    );
    MDirection::Types mtype = dirCoord.directionType();
    // SkyComponents require the flux density but users and the fitting
	// code really want to specify peak intensities. So we must convert
	// here. To do that, we need to know the brightness units of the image.

	Quantity resArea;
	Quantity intensityToFluxConversion(1.0, "beam");

	// does the image have a restoring beam?
    if (image.imageInfo().hasBeam()) {
        if (image.imageInfo().hasMultipleBeams()) {
            *_log << LogIO::WARN << "This image has multiple beams. The first will be "
                << "used to determine flux density estimates." << LogIO::POST;
        }
        resArea = Quantity(
            image.imageInfo().getBeamSet().getBeams().begin()->getArea("sr"),
            "sr"
        );
    }
    else {
		// if no restoring beam, let's hope the the brightness units are
		// in [prefix]Jy/pixel and let's find the pixel size.
    	resArea = dirCoord.getPixelArea();
	}
    Vector<String> units = csys.directionCoordinate().worldAxisUnits();
    String raUnit = units[0];
    String decUnit = units[1];
	for(uInt i=0; i<_peakValues.size(); i++) {
		pos[dirAxesNums[0]] = _xposValues[i];
		pos[dirAxesNums[1]] = _yposValues[i];
        csys.directionCoordinate().toWorld(world, pos);
        Quantity ra(world[0], raUnit);
        Quantity dec(world[1], decUnit);
        MDirection mdir(ra, dec, mtype);

        GaussianShape gaussShape(
        	mdir, _majValues[i], _minValues[i], _paValues[i]
        );
		Unit brightnessUnit = image.units();
		// Estimate the flux density
		Quantity fluxQuantity = Quantity(_peakValues[i], brightnessUnit) * intensityToFluxConversion;
		fluxQuantity.convert("Jy");
		fluxQuantity = fluxQuantity*gaussShape.getArea()/resArea;
		// convert to Jy again to get rid of the superfluous sr/(sr)
		fluxQuantity.convert("Jy");
		// Just fill the Stokes which aren't being fit with the same value as
		// the Stokes that is. Doesn't matter that the other three are bogus
		// for the purposes of this, since we only fit one stokes at a time
		Vector<Double> fluxStokes(4);

		for(uInt j=0; j<4; j++) {
			fluxStokes[j] = fluxQuantity.getValue();
		}
		Quantum<Vector<Double> > fluxVector(fluxStokes, fluxQuantity.getUnit());
		Flux<Double> flux(fluxVector);
        SkyComponent skyComp(flux, gaussShape, spectrum);
        _componentList.add(skyComp);
	}

}
} //# NAMESPACE CASA - END

