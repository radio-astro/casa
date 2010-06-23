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

#include <casa/Containers/HashMap.h>
#include <casa/Containers/HashMapIter.h>

#include <casa/IO/FilebufIO.h>
#include <casa/IO/FiledesIO.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/Utilities/Precision.h>

#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/SpectralModel.h>

#include <images/IO/FitterEstimatesFileParser.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageFitter.h>
#include <images/Images/ImageInputProcessor.h>
#include <images/Images/ImageMetaData.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Regions/RegionManager.h>

#include <images/Regions/WCUnion.h>
#include <images/Regions/WCBox.h>
#include <lattices/Lattices/LCRegion.h>

#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentShape.h>

#include <components/ComponentModels/GaussianShape.h>

namespace casa {

    ImageFitter::ImageFitter(
        const String& imagename, const String& region, const String& box,
        const uInt chanInp, const String& stokes,
        const String& maskInp, const Vector<Float>& includepix,
        const Vector<Float>& excludepix, const String& residualInp,
        const String& modelInp, const String& estimatesFilename,
        const String& logfile, const Bool& append,
        const String& newEstimatesInp
    ) : _log(new LogIO()), _image(0), _chan(chanInp), _stokesString(stokes),
		mask(maskInp), residual(residualInp),model(modelInp), logfileName(logfile),
		regionString(""), estimatesString(""), newEstimatesFileName(newEstimatesInp),
		includePixelRange(includepix), excludePixelRange(excludepix),
		estimates(), fixed(0), logfileAppend(append), fitConverged(False),
		fitDone(False), _noBeam(False), peakIntensities() {
        _construct(imagename, box, region, 0, estimatesFilename);
    }

    ImageFitter::ImageFitter(
        const String& imagename, const Record* regionPtr, const String& box,
        const uInt chanInp, const String& stokes,
        const String& maskInp, const Vector<Float>& includepix,
        const Vector<Float>& excludepix, const String& residualInp,
        const String& modelInp, const String& estimatesFilename,
        const String& logfile, const Bool& append,
        const String& newEstimatesInp
    ) : _log(new LogIO()), _image(0), _chan(chanInp), _stokesString(stokes),
		mask(maskInp), residual(residualInp),model(modelInp), logfileName(logfile),
		regionString(""), estimatesString(""), newEstimatesFileName(newEstimatesInp),
		includePixelRange(includepix), excludePixelRange(excludepix),
		estimates(), fixed(0), logfileAppend(append), fitConverged(False),
		fitDone(False), _noBeam(False), peakIntensities() {
        _construct(imagename, box, "", regionPtr, estimatesFilename);
    }

    ImageFitter::~ImageFitter() {
        delete _log;
        delete _image;
    }

    ComponentList ImageFitter::fit() {
        *_log << LogOrigin("ImageFitter", "fit");
        Array<Float> residPixels;
        Array<Bool> residMask;
        Bool converged;
        uInt ngauss = estimates.nelements() > 0 ? estimates.nelements() : 1;
        Vector<String> models(ngauss);
        models.set("gaussian");
        ImageAnalysis myImage(_image);
        Bool fit = True;
        Bool deconvolve = False;
        Bool list = True;
        String errmsg;
        Record estimatesRecord;

        estimates.toRecord(errmsg, estimatesRecord);
		try {
        	results = myImage.fitsky(
            	residPixels, residMask, converged,
            	inputStats, residStats, chiSquared,
            	_regionRecord, _chan, _stokesString, mask,
            	models, estimatesRecord, fixed,
            	includePixelRange, excludePixelRange,
            	fit, deconvolve, list, residual, model
        	);
		}
		catch (AipsError err) {
    		*_log << LogIO::WARN << "Fit failed to converge because of exception: "
			<< err.getMesg() << LogIO::POST;
			converged = false;
		}
        fitDone = True;
        fitConverged = converged;
        if(converged) {
        	_setSizes();
        	_setFluxes();
        }
        String resultsString = _resultsToString();
        if (converged && ! newEstimatesFileName.empty()) {
        	_writeNewEstimatesFile();
        }
        *_log << LogIO::NORMAL << resultsString << LogIO::POST;
        if (! logfileName.empty()) {
        	_writeLogfile(resultsString);
        }
        return results;
    }

    Bool ImageFitter::converged() const {
    	if (!fitDone) {
    		throw AipsError("fit has not yet been performed");
    	}
    	return fitConverged;
    }

	void ImageFitter::_getStandardDeviations(Double& inputStdDev, Double& residStdDev) const {
		inputStdDev = _getStatistic("sigma", inputStats);
		residStdDev = _getStatistic("sigma", residStats);
	}

	void ImageFitter::_getRMSs(Double& inputRMS, Double& residRMS) const {
		inputRMS = _getStatistic("rms", inputStats);
		residRMS = _getStatistic("rms", residStats);
	}

	Double ImageFitter::_getStatistic(const String& type, const Record& stats) const {
		// FIXME ? I cannot figure out a better way of accessing an array element (neither
		// array[0] nor array(0) work) without having to convert it to a vector or creating
		// an IPosition object.
		// I would also think I could get the value of a record element by simply referencing
		// its key, without having to get a field number, but I cannot figure out how to do that
		// either.

		Array<Double> statArray;
		stats.get(stats.fieldNumber(type), statArray);
		vector<Double> statVec;
		statArray.tovector(statVec);
		return statVec[0];
	}

    void ImageFitter::_construct(
        const String& imagename, const String& box, const String& regionName,
        const Record* regionPtr, const String& estimatesFilename
    ) {
    	LogOrigin logOrigin("ImageFitter", __FUNCTION__);
        *_log << logOrigin;

		ImageInputProcessor inputProcessor;
        String diagnostics;
        inputProcessor.process(
        	_image, _regionRecord, diagnostics,
        	imagename, regionPtr, regionName, box,
        	String::toString(_chan), _stokesString
        );
        *_log << logOrigin;

        // <todo> kludge because Flux class is really only made for I, Q, U, and V stokes
        String iquv = "IQUV";
        _kludgedStokes = (iquv.index(_stokesString) == String::npos) ? "I" : _stokesString;
        // </todo>
        //_doRegion(box, regionName, regionPtr);
        if(estimatesFilename.empty()) {
        	*_log << LogIO::NORMAL << "No estimates file specified, so will attempt to find and fit one gaussian."
        		<< LogIO::POST;
        }
        else {
        	FitterEstimatesFileParser parser(estimatesFilename, *_image);
        	estimates = parser.getEstimates();
        	estimatesString = parser.getContents();
        	fixed = parser.getFixed();
        	Record rec;
        	String errmsg;
        	estimates.toRecord(errmsg, rec);
        	*_log << LogIO::NORMAL << "File " << estimatesFilename << " has " << estimates.nelements()
        		<< " specified, so will attempt to fit that many gaussians " << LogIO::POST;
        }
        if (! residual.empty() || ! model.empty()) {
        	HashMap<String, String> map, map2;
        	map("residual") = residual;
        	map("model") = model;
        	map2 = map;
        	ConstHashMapIter<String, String> iter(map);
        	for (iter.toStart(); ! iter.atEnd(); iter++) {
        		String key = iter.getKey();
        		String name = iter.getVal();
        		if (! name.empty()) {
        			File f(name);
        			switch (f.getWriteStatus()) {
        			case File::NOT_CREATABLE:
        				*_log << LogIO::WARN << "Requested " << key << " image "
							<< name << " cannot be created so will not be written" << LogIO::POST;
        				map2(key) = "";
        				break;
        			case File::NOT_OVERWRITABLE:
        				*_log << LogIO::WARN << "Requested " << key
							<< " image: There is already a file or directory named "
							<< name << " which cannot be overwritten so the " << iter.getKey()
							<< " image will not be written" << LogIO::POST;
        				map2(key) = "";
        				break;
        			default:
        				continue;
        			}
            	}
            }
        	residual = map2("residual");
        	model = map2("model");
        }
    }

    String ImageFitter::_resultsToString() {
    	ostringstream summary;
    	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
    	summary << "Input parameters ---" << endl;
    	summary << "       --- imagename:           " << _image->name() << endl;
    	summary << "       --- region:              " << regionString << endl;
    	summary << "       --- channel:             " << _chan << endl;
    	summary << "       --- stokes:              " << _stokesString << endl;
    	summary << "       --- mask:                " << mask << endl;
    	summary << "       --- include pixel ragne: " << includePixelRange << endl;
    	summary << "       --- exclude pixel ragne: " << excludePixelRange << endl;
    	summary << "       --- initial estimates:   " << estimatesString << endl;

    	if (converged()) {
        	if (_noBeam) {
        		*_log << LogIO::WARN << "Flux density not reported because "
        				<< "there is no clean beam in image header so these quantities cannot "
        				<< "be calculated" << LogIO::POST;
        	}
    		summary << _statisticsToString() << endl;
    		for (uInt i = 0; i < results.nelements(); i++) {
    			summary << "Fit on " << _image->name(True) << " component " << i << endl;
    			summary << results.component(i).positionToString(&(_image->coordinates())) << endl;
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
    	// TODO It is not clear how this chi squred value is calculated and atm it does not
    	// appear to be useful, so don't report it. In the future, investigate more deeply
    	// how it is calculated and see if a useful value for reporting can be derived from
    	// it.
    	// stats << "       --- Chi-squared of fit " << chiSquared << endl;
    	stats << "Input and residual image statistics (to be used as a rough guide only as to goodness of fit)" << endl;
    	Double inputStdDev, residStdDev, inputRMS, residRMS;
    	_getStandardDeviations(inputStdDev, residStdDev);
    	_getRMSs(inputRMS, residRMS);
    	String unit = fluxDensities[0].getUnit();
    	stats << "       --- Standard deviation of input image " << inputStdDev << " " << unit << endl;
    	stats << "       --- Standard deviation of residual image " << residStdDev << " " << unit << endl;
    	stats << "       --- RMS of input image " << inputRMS << " " << unit << endl;
    	stats << "       --- RMS of residual image " << residRMS << " " << unit << endl;
    	return stats.str();
    }

    void ImageFitter::_setFluxes() {
    	fluxDensities.resize(results.nelements());
    	peakIntensities.resize(results.nelements());
    	Vector<Quantity> fluxQuant;
		ImageAnalysis ia;
		ia.open(_image->name());

    	for(uInt i=0; i<results.nelements(); i++) {
    		results.getFlux(fluxQuant, i);
    		// TODO there is probably a better way to get the flux component we want...
    		Vector<String> polarization = results.getStokes(i);
    		for (uInt j=0; j<polarization.size(); j++) {
    			if (polarization[j] == _kludgedStokes) {
    				fluxDensities[i] = fluxQuant[j];
    				break;
    			}
    		}
    		const ComponentShape* compShape = results.getShape(i);
    		AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);
            peakIntensities[i] = ia.convertflux(
    	        _noBeam, fluxDensities[i], majorAxes[i], minorAxes[i], "Gaussian", True, True
            );
    	}
    }

    void ImageFitter::_setSizes() {
    	uInt ncomps = results.nelements();
    	majorAxes.resize(ncomps);
    	minorAxes.resize(ncomps);
    	positionAngles.resize(ncomps);
    	for(uInt i=0; i<results.nelements(); i++) {
    		const ComponentShape* compShape = results.getShape(i);
    		AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);
    		majorAxes[i] = (static_cast<const GaussianShape *>(compShape))->majorAxis();
    		minorAxes[i] = (static_cast<const GaussianShape *>(compShape))->minorAxis();
    		positionAngles[i]  = (static_cast<const GaussianShape *>(compShape))->positionAngle();
    	}
    }

    String ImageFitter::_sizeToString(const uInt compNumber) const  {
    	ostringstream size;
    	const ComponentShape* compShape = results.getShape(compNumber);
    	AlwaysAssert(compShape->type() == ComponentType::GAUSSIAN, AipsError);

    	Vector<Quantum<Double> > beam = _image->imageInfo().restoringBeam();
    	Bool hasBeam = beam.nelements() == 3;
    	size << "Image component size";
    	if (hasBeam) {
    		size << " (convolved with beam)";
    	}
    	size << " ---" << endl;
    	size << compShape->sizeToString() << endl;
    	if (hasBeam) {
    		Quantity maj = majorAxes[compNumber];
    		Quantity min = minorAxes[compNumber];
    		Quantity pa = positionAngles[compNumber];
    		const GaussianShape *gaussShape = static_cast<const GaussianShape *>(compShape);
    		Quantity emaj = gaussShape->majorAxisError();
    		Quantity emin = gaussShape->minorAxisError();
    		Quantity epa  = gaussShape->positionAngleError();

    		size << "Clean beam size ---" << endl;
    		size << TwoSidedShape::sizeToString(beam[0], beam[1], beam[2], False) << endl;
    		size << "Image component size (deconvolved from beam) ---" << endl;
    		// NOTE fit components change here to their deconvolved counterparts
    		Quantity femaj = emaj/maj;
    		Quantity femin = emin/min;
    		Bool fitSuccess = False;
    		Bool isPointSource = ImageUtilities::deconvolveFromBeam(maj, min, pa, fitSuccess, *_log, beam);
    		if(fitSuccess) {
    			if (isPointSource) {
    				size << "    Component is a point source" << endl;
    			}
    			else {
    				if (pa.getValue("deg") < 0) {
    					pa += Quantity(180, "deg");
    				}
    				emaj = femaj * maj;
    				emin = femin * min;
    				size << TwoSidedShape::sizeToString(maj, min, pa, True, emaj, emin, epa);
    			}
    		}
    		else {
    			// I doubt this block will ever be entered, but just in case
    			size << "    Deconvolved size could not be determined" << endl;
    		}
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
    	Quantity fluxDensity = fluxDensities[compNumber];
       	Quantity fluxDensityError;
		Vector<String> polarization = results.getStokes(compNumber);
		for (uInt i=0; i<polarization.nelements(); i++) {
            if (polarization[i] == _kludgedStokes) {
            	complex<double> error = results.component(compNumber).flux().errors()[i];
            	fluxDensityError.setValue(sqrt(error.real()*error.real() + error.imag()*error.imag()));
            	fluxDensityError.setUnit(fluxDensity.getUnit());
            	break;
            }
		}
        String unit;
        for (uInt i=0; i<unitPrefix.size(); i++) {
        	unit = unitPrefix[i] + "Jy";
        	if (fluxDensity.getValue(unit) > 1) {
        		fluxDensity.convert(unit);
        		fluxDensityError.convert(unit);
        		break;
        	}
        }
        Vector<Double> fd(2);
        fd[0] = fluxDensity.getValue();
        fd[1] = fluxDensityError.getValue();
    	Quantity peakIntensity = peakIntensities[compNumber];
    	Quantity intensityToFluxConversion = peakIntensity.getUnit().contains("/beam")
    		? Quantity(1.0, "beam")
    		: Quantity(1.0, "pixel");

    	Quantity tmpFlux = peakIntensity * intensityToFluxConversion;
    	tmpFlux.convert("Jy");

        Quantity peakIntensityError = peakIntensity*fluxDensityError/fluxDensity;
        Quantity tmpFluxError = peakIntensityError * intensityToFluxConversion;

        uInt precision = 0;
        fluxes << "Flux ---" << endl;

        if (! _noBeam) {
        	precision = precisionForValueErrorPairs(fd, Vector<Double>());
        	fluxes << std::fixed << setprecision(precision);
        	fluxes << "       --- Integrated:   " << fluxDensity.getValue()
					<< " +/- " << fluxDensityError.getValue() << " "
					<< fluxDensity.getUnit() << endl;
        }

        for (uInt i=0; i<unitPrefix.size(); i++) {
        	String unit = unitPrefix[i] + tmpFlux.getUnit();
         	if (tmpFlux.getValue(unit) > 1) {
         		tmpFlux.convert(unit);
         		tmpFluxError.convert(unit);
         		break;
         	}
        }
        //String newUnit = tmpFlux.getUnit() + "/" + intensityToFluxConversion.getUnit();
        peakIntensity = Quantity(tmpFlux.getValue(), tmpFlux.getUnit() + "/" + intensityToFluxConversion.getUnit());
        peakIntensityError = Quantity(tmpFluxError.getValue(), peakIntensity.getUnit());


        Vector<Double> pi(2);
        pi[0] = peakIntensity.getValue();
        pi[1] = peakIntensityError.getValue();
        precision = precisionForValueErrorPairs(pi, Vector<Double>());
        fluxes << std::fixed << setprecision(precision);
        fluxes << "       --- Peak:         " << peakIntensity.getValue()
 			<< " +/- " << peakIntensityError.getValue() << " "
 			<< peakIntensity.getUnit() << endl;
        fluxes << "       --- Polarization: " << _stokesString << endl;
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
    	const SpectralModel& spectrum = results.component(compNumber).spectrum();
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
    	spec << "      --- frequency:        " << frequency << " (" << wavelength << ")" << endl;
    	return spec.str();
    }

    void ImageFitter::_writeLogfile(const String& output) const {
    	File log(logfileName);
    	switch (File::FileWriteStatus status = log.getWriteStatus()) {
    	case File::OVERWRITABLE:
    		if (logfileAppend) {
    			Int fd = open(logfileName.c_str(), O_RDWR | O_APPEND);
    			FiledesIO fio(fd, logfileName.c_str());
    			fio.write(output.length(), output.c_str());
    			FiledesIO::close(fd);
    			*_log << LogIO::NORMAL << "Appended results to file "
    					<< logfileName << LogIO::POST;
    		}
    	case File::CREATABLE:
    		if (status == File::CREATABLE || ! logfileAppend) {
    			String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
    			Int fd = FiledesIO::create(logfileName.c_str());
    			FiledesIO fio (fd, logfileName.c_str());
    			fio.write(output.length(), output.c_str());
    			FiledesIO::close(fd);
    			*_log << LogIO::NORMAL << action << " file "
    					<< logfileName << " with new log file"
    					<< LogIO::POST;
    		}
    		break;
    	case File::NOT_OVERWRITABLE:
    		*_log << LogIO::WARN << "Unable to write to file "
    		<< logfileName << LogIO::POST;
    		break;
    	case File::NOT_CREATABLE:
    		*_log << LogIO::WARN << "Cannot create log file "
    		<< logfileName << LogIO::POST;
    		break;
    	}
    }

    void ImageFitter::_writeNewEstimatesFile() const {
    	ostringstream out;
    	for (uInt i=0; i<results.nelements(); i++) {

        	MDirection mdir = results.getRefDirection(i);
           	Quantity lat = mdir.getValue().getLat("rad");
            Quantity longitude = mdir.getValue().getLong("rad");
    		Vector<Double> world(4,0), pixel(4,0);
    		_image->coordinates().toWorld(world, pixel);
    		world[0] = longitude.getValue();
    		world[1] = lat.getValue();
    		if (_image->coordinates().toPixel(pixel, world)) {
    			out << peakIntensities[i].getValue() << ", "
    				<< pixel[0] << ", " << pixel[1] << ", "
    				<< majorAxes[i] << ", " << minorAxes[i] << ", "
    				<< positionAngles[i] << endl;
    		}
    		else {
    			*_log << LogIO::WARN << "Unable to calculate pixel location of "
    				<< "component number " << i << " so cannot write new estimates"
    				<< "file" << LogIO::POST;
    			return;
    		}
    	}
    	String output = out.str();
    	File estimates(newEstimatesFileName);
    	switch (File::FileWriteStatus status = estimates.getWriteStatus()) {
    	case File::NOT_OVERWRITABLE:
    		*_log << LogIO::WARN << "Unable to write to file "
				<< newEstimatesFileName << LogIO::POST;
    		break;
    	case File::NOT_CREATABLE:
    		*_log << LogIO::WARN << "Cannot create estimates file "
				<< newEstimatesFileName << LogIO::POST;
    		break;
    	default:
    		String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
    		Int fd = FiledesIO::create(newEstimatesFileName.c_str());
    		FiledesIO fio(fd, logfileName.c_str());
    		fio.write(output.length(), output.c_str());
    		FiledesIO::close(fd);
    		*_log << LogIO::NORMAL << action << " file "
    				<< newEstimatesFileName << " with new estimates file"
    				<< LogIO::POST;
    	}
    }
}

