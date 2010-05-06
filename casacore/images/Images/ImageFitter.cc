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

#include <casa/IO/FilebufIO.h>
#include <casa/IO/FiledesIO.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>

#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/SpectralModel.h>

#include <images/IO/FitterEstimatesFileParser.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageFitter.h>
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
    ) : chan(chanInp), stokesString(stokes), mask(maskInp),
		residual(residualInp),model(modelInp), logfileName(logfile),
		regionString(""), estimatesString(""), newEstimatesFileName(newEstimatesInp),
		includePixelRange(includepix), excludePixelRange(excludepix),
		estimates(), fixed(0), logfileAppend(append), fitConverged(False),
		fitDone(False), _noBeam(False), peakIntensities(), pixelPositions() {
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
    ) : chan(chanInp), stokesString(stokes), mask(maskInp),
		residual(residualInp),model(modelInp), logfileName(logfile),
		regionString(""), estimatesString(""), newEstimatesFileName(newEstimatesInp),
		includePixelRange(includepix), excludePixelRange(excludepix),
		estimates(), fixed(0), logfileAppend(append), fitConverged(False),
		fitDone(False), _noBeam(False), peakIntensities(), pixelPositions() {
        _construct(imagename, box, "", regionPtr, estimatesFilename);
    }

    ImageFitter::~ImageFitter() {
        delete itsLog;
        delete image;
    }

    ComponentList ImageFitter::fit() {
        *itsLog << LogOrigin("ImageFitter", "fit");
        Array<Float> residPixels;
        Array<Bool> residMask;
        Bool converged;

        uInt ngauss = estimates.nelements() > 0 ? estimates.nelements() : 1;
        Vector<String> models(ngauss);
        models.set("gaussian");
        ImageAnalysis myImage(image);

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
            	regionRecord, chan, stokesString, mask,
            	models, estimatesRecord, fixed,
            	includePixelRange, excludePixelRange,
            	fit, deconvolve, list, residual, model
        	);
		}
		catch (AipsError err) {
    		*itsLog << LogIO::WARN << "Fit failed to converge because of exception: "
			<< err.getMesg() << LogIO::POST;
			converged = false;
		}
        fitDone = True;
        fitConverged = converged;
        if(converged) {
        	_setSizes();
        	_setFluxes();
        	pixelPositions.resize(results.nelements());
        	for(uInt i=0; i<results.nelements(); i++) {
        		Vector<Double> x(2);
        		pixelPositions[i] = x;
        	}
        }
        String resultsString = _resultsToString();
        if (converged && ! newEstimatesFileName.empty()) {
        	_writeNewEstimatesFile();
        }
        *itsLog << LogIO::NORMAL << resultsString << LogIO::POST;
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
        itsLog = new LogIO();
        *itsLog << LogOrigin("ImageFitter", "_construct");
        if (imagename.empty()) {
            *itsLog << "imagename cannot be blank" << LogIO::EXCEPTION;
        }

        // Register the functions to create a FITSImage or MIRIADImage object.
        FITSImage::registerOpenFunction();
        MIRIADImage::registerOpenFunction();
        ImageUtilities::openImage(image, imagename, *itsLog);
        if (image == 0) {
            throw(AipsError("Unable to open image " + imagename));
        }
        _checkImageParameterValidity();
        _doRegion(box, regionName, regionPtr);
        if(estimatesFilename.empty()) {
        	*itsLog << LogIO::NORMAL << "No estimates file specified, so will attempt to find and fit one gaussian."
        		<< LogIO::POST;
        }
        else {
        	FitterEstimatesFileParser parser(estimatesFilename, *image);
        	estimates = parser.getEstimates();
        	estimatesString = parser.getContents();
        	fixed = parser.getFixed();
        	Record rec;
        	String errmsg;
        	estimates.toRecord(errmsg, rec);
        	*itsLog << LogIO::NORMAL << "File " << estimatesFilename << " has " << estimates.nelements()
        		<< " specified, so will attempt to fit that many gaussians " << LogIO::POST;
        }
    }

    void ImageFitter::_checkImageParameterValidity() {
        *itsLog << LogOrigin("ImageFitter", "_checkImageParameterValidity");
        String error;
        ImageMetaData md(*image);
        if (md.hasPolarizationAxis()) {
        	if(stokesString.empty()) {
        		if (md.nStokes() == 1) {
        			// stokes not specified, but only one stokes axis so use that polarization
        			stokesString = md.stokesAtPixel(0);
        		}
        		else {
        			*itsLog << "No stokes specified but image has multiple polarizations. Please specify"
        					<< "which stokes plane on which you want the fit performed"
        					<< LogIO::EXCEPTION;
        		}
        	}
        	if (! md.isStokesValid(stokesString)) {
        		*itsLog << "This image has no stokes " << stokesString << LogIO::EXCEPTION;
        	}
        }
        // <todo> kludge because Flux class is really only made for I, Q, U, and V stokes
        String iquv = "IQUV";
        _kludgedStokes = (iquv.index(stokesString) == String::npos) ? "I" : stokesString;
        // </todo>
        if (md.hasSpectralAxis()) {
        	if (! md.isChannelNumberValid(chan)) {
        		*itsLog << "Spectral channel number " << chan << " is not valid for this image." << LogIO::EXCEPTION;
        	}
        }
    } 

    void ImageFitter::_doRegion(const String& box, const String& region, const Record* regionPtr) {
    	// note that only one of region and regionPtr should ever be provided
    	ImageRegion imRegion;
        if (box.empty()) {
            // box not specified, check for saved region
            if (region.empty() && regionPtr == 0) {
                // neither region nor box specified, use entire 2-D plane
                IPosition imShape = image->shape();
                Vector<Int> dirNums = ImageMetaData(*image).directionAxesNumbers();
                Vector<Int> dirShape(imShape[dirNums[0]], imShape[dirNums[1]]);
                *itsLog << LogIO::NORMAL << "Neither box nor region specified, "
                    << "so entire plane of " << dirShape[0] << " x "
                    << dirShape[1] << "  will be used" << LogIO::POST;
                ostringstream boxStream;
                boxStream << "0, 0, " << dirShape[0] << ", " << dirShape[1];
                imRegion = _processBox(String(boxStream));
            }
            else if (regionPtr != 0) {
            	// region record pointer provided
            	regionRecord = *regionPtr;
            	regionString = "used provided region record";
            	return;
            }
            else {
            	// region name provided
            	Regex otherImage("(.*)+:(.*)+");
            	if (region.matches(otherImage)) {
            		String res[2];
            		casa::split(region, res, 2, ":");
            		PagedImage<Float> other(res[0]);
            		imRegion = other.getRegion(res[1]);
            	}
            	else {
            		imRegion = image->getRegion(region);
            	}
                regionString = "Used image region " + region;
            }

        }
        else if (box.freq(",") % 4 != 3) {
            *itsLog << "box not specified correctly" << LogIO::EXCEPTION;
        }
        else {
            if (region != "") {
                *itsLog << "both box and region specified, box will be used" << LogIO::WARN;
            }
            // we have been given a box by the user and it is specified correctly
            imRegion = _processBox(box);
            regionString = "Used box " + box;
        }
        regionRecord = Record(imRegion.toRecord(""));
    } 

    ImageRegion ImageFitter::_processBox(const String& box) {
        Vector<String> boxParts = stringToVector(box);
        AlwaysAssert(boxParts.size() % 4 == 0, AipsError);
        RegionManager rm;
        ImageRegion imRegion;
        for(uInt i=0; i<boxParts.size()/4; i++) {
        	uInt index = 4*i;
        	ImageRegion boxRegion = _boxRegion(
        		boxParts[index], boxParts[index+1], boxParts[index+2], boxParts[index+3]
        	);
        	imRegion = (i == 0) ? boxRegion : imRegion = *rm.doUnion(imRegion, boxRegion);
        }
        return imRegion;
    }

    ImageRegion ImageFitter::_boxRegion(String blc1, String blc2, String trc1, String trc2) {

        IPosition imShape = image->shape(); 
        Vector<Double> blc(imShape.nelements());
        Vector<Double> trc(imShape.nelements());

        for (uInt i=0; i<imShape.nelements(); i++) {
            blc[i] = 0;
            trc[i] = imShape[i] - 1;
        }
        ImageMetaData md(*image);

        Vector<Int> dirNums = md.directionAxesNumbers();


        blc[dirNums[0]] = String::toDouble(blc1);
        blc[dirNums[1]] = String::toDouble(blc2);
        trc[dirNums[0]] = String::toDouble(trc1);
        trc[dirNums[1]] = String::toDouble(trc2);

        if(md.hasSpectralAxis()) {
        	Int spectralAxisNumber = md.spectralAxisNumber();
        	blc[spectralAxisNumber] = chan;
        	trc[spectralAxisNumber] = chan;
        }

        if(md.hasPolarizationAxis()) {
        	Int polarizationAxisNumber = md.polarizationAxisNumber();
        	Int stokesPixelNumber = md.stokesPixelNumber(stokesString);
        	blc[polarizationAxisNumber] = stokesPixelNumber;
        	trc[polarizationAxisNumber] = stokesPixelNumber;
        }

        LCBox lcBox(blc, trc, imShape);
        WCBox wcBox(lcBox, image->coordinates());
        return ImageRegion(wcBox);
    }

    String ImageFitter::_resultsToString() {
    	ostringstream summary;
    	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
    	summary << "Input parameters ---" << endl;
    	summary << "       --- imagename:           " << image->name() << endl;
    	summary << "       --- region:              " << regionString << endl;
    	summary << "       --- channel:             " << chan << endl;
    	summary << "       --- stokes:              " << stokesString << endl;
    	summary << "       --- mask:                " << mask << endl;
    	summary << "       --- include pixel ragne: " << includePixelRange << endl;
    	summary << "       --- exclude pixel ragne: " << excludePixelRange << endl;
    	summary << "       --- initial estimates:   " << estimatesString << endl;

    	if (converged()) {
        	if (_noBeam) {
        		*itsLog << LogIO::WARN << "Flux density not reported because "
        				<< "there is no clean beam in image header so these quantities cannot "
        				<< "be calculated" << LogIO::POST;
        	}
    		summary << _statisticsToString() << endl;
    		for (uInt i = 0; i < results.nelements(); i++) {
    			summary << "Fit on " << image->name(True) << " component " << i << endl;
    			summary << _positionToString(i) << endl;
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

    String ImageFitter::_positionToString(const uInt compNumber)  {
    	ostringstream position;
    	MDirection mdir = results.getRefDirection(compNumber);

    	Quantity lat = mdir.getValue().getLat("rad");
    	String dec = MVAngle(lat).string(MVAngle::ANGLE_CLEAN, 8);

    	Quantity longitude = mdir.getValue().getLong("rad");
    	String ra = MVTime(longitude).string(MVTime::TIME, 9);
    	const ComponentShape* compShape = results.getShape(compNumber);

    	Quantity ddec = compShape->refDirectionErrorLat();
    	ddec.convert("rad");

    	Quantity dra = compShape->refDirectionErrorLong();

    	dra.convert("rad");

    	// choose a unified error for both axes
    	Double delta = 0;
    	if ( dra.getValue() == 0 && ddec.getValue() == 0 ) {
    		delta = 0;
    	}
    	else if ( dra.getValue() == 0 ) {
    		delta = fabs(ddec.getValue());
    	}
    	else if ( ddec.getValue() == 0 ) {
    		delta = fabs(dra.getValue());
    	}
    	else {
    		delta = sqrt( dra.getValue()*dra.getValue() + ddec.getValue()*ddec.getValue() );
    	}

		// Add error estimates to ra/dec strings if an error is given (either >0)

    	uInt precision = 1;
    	if ( delta != 0 ) {
    		dra.convert("s");

    		ddec.convert("arcsec");
    		Double drasec  = _round(dra.getValue());
    		Double ddecarcsec = _round(ddec.getValue());
    		Vector<Double> dravec(2), ddecvec(2);
    		dravec.set(drasec);
    		ddecvec.set(ddecarcsec);
    		precision = _precision(dravec,ddecvec);
    		ra = MVTime(longitude).string(MVTime::TIME, 6+precision);
    		dec =  MVAngle(lat).string(MVAngle::ANGLE, 6+precision);
    	}
    	position << "Position ---" << endl;
    	position << "       --- ra:    " << ra << " +/- " << std::fixed
    		<< setprecision(precision) << dra << " (" << dra.getValue("arcsec")
    		<< " arcsec)" << endl;
    	position << "       --- dec: " << dec << " +/- " << ddec << endl;
       	Vector<Double> world(4,0), pixel(4,0);
    	image->coordinates().toWorld(world, pixel);

        world[0] = longitude.getValue();
        world[1] = lat.getValue();
        // TODO do the pixel computations in another method
        if (image->coordinates().toPixel(pixel, world)) {
        	pixelPositions[compNumber][0] = pixel[0];
        	pixelPositions[compNumber][1] = pixel[1];

        	DirectionCoordinate dCoord = image->coordinates().directionCoordinate(
        		ImageMetaData(*image).directionCoordinateNumber()
        	);
        	Vector<Double> increment = dCoord.increment();
        	Double raPixErr = dra.getValue("rad")/increment[0];
        	Double decPixErr = ddec.getValue("rad")/increment[1];
        	Vector<Double> raPix(2), decPix(2);
         	raPix.set(_round(raPixErr));
        	decPix.set(_round(decPixErr));
        	precision = _precision(raPix, decPix);

        	position << setprecision(precision);
        	position << "       --- ra:   " << pixel[0] << " +/- " << raPixErr << " pixels" << endl;
        	position << "       --- dec:  " << pixel[1] << " +/- " << decPixErr << " pixels" << endl;
        }
        else {
        	position << "unable to determine max in pixels" << endl;
        }
    	return position.str();
    }

    void ImageFitter::_setFluxes() {
    	fluxDensities.resize(results.nelements());
    	peakIntensities.resize(results.nelements());
    	Vector<Quantity> fluxQuant;
		ImageAnalysis ia;
		ia.open(image->name());

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
    	Quantity maj = majorAxes[compNumber];
    	Quantity min = minorAxes[compNumber];
    	Quantity pa = positionAngles[compNumber];
    	Quantity emaj = (static_cast<const GaussianShape *>(compShape))->majorAxisError();
    	Quantity emin = (static_cast<const GaussianShape *>(compShape))->minorAxisError();
    	Quantity epa  = (static_cast<const GaussianShape *>(compShape))->positionAngleError();
    	Vector<Quantum<Double> > beam = image->imageInfo().restoringBeam();
    	Bool hasBeam = beam.nelements() == 3;
    	size << "Image component size";
    	if (hasBeam) {
    		size << " (convolved with beam)";
    	}
    	size << " ---" << endl;
    	size << _gaussianToString(maj, min, pa, emaj, emin, epa) << endl;
    	if (hasBeam) {
    		size << "Clean beam size ---" << endl;
    		size << _gaussianToString(beam[0], beam[1], beam[2], 0, 0, 0, False) << endl;
    		size << "Image component size (deconvolved from beam) ---" << endl;
    		// NOTE fit components change here to their deconvolved counterparts
    		Quantity femaj = emaj/maj;
    		Quantity femin = emin/min;
    		Bool fitSuccess = False;
    		Bool isPointSource = ImageUtilities::deconvolveFromBeam(maj, min, pa, fitSuccess, *itsLog, beam);
    		if(fitSuccess) {
    			if (isPointSource) {
    				size << "    Component is a point source" << endl;
    			}
    			else {
    				if (pa.getValue("deg") < 0) {
    					pa += Quantity(180, "deg");
    				}
    				emaj *= femaj;
    				emin *= femin;
    				size << _gaussianToString(maj, min, pa, emaj, emin, epa);
    			}
    		}
    		else {
    			// I doubt this block will ever be entered, but just in case
    			size << "    Deconvolved size could not be determined" << endl;
    		}
    	}
    	return size.str();
    }

    Double ImageFitter::_round(Double number) const {
    	Double sign = 1;
        if (number < 0) {
            sign = -1;
            number = -number;
        }
        Double lgr = log10(number);
        // shift number into range 32-320
        Int i = (lgr >= 0) ? int(lgr + 0.5) : int(lgr - 0.5);
        Double temp = number * pow(10.0, (2-i));
        return sign*(temp + 0.5)*pow(10.0, (i-2));
    }

    uInt ImageFitter::_precision(
    	const Vector<Double>& pair1, const Vector<Double>& pair2
    ) const {
    	Double val1 = pair1[0];
    	Double err1 = pair1[1];
    	Double value = val1;
    	Double error = fabs(err1);
    	uInt sign = 0;
    	if (pair2.size() == 0) {
				if (val1 < 0) {
                sign = 1;
                val1 = fabs(val1);
            }
    	}
    	else {
    		Double val2 = pair2[0];
    		Double err2 = pair2[1];
            if (val1 < 0 or val2 < 0) {
                sign = 1;
                val1 = fabs(val1);
                val2 = fabs(val2);
            }
            value = max(val1, val2);
            error = (err1 == 0 || err2 == 0)
            	? max(fabs(err1), fabs(err1))
            	: min(fabs(err1), fabs(err2));
    	}

    	// Here are a few general safeguards
    	// If we are dealing with a value smaller than the estimated error
    	// (e.g., 0.6 +/- 12) , the roles in formatting need to be
    	// reversed.
    	if ( value < error ) {
    		value = max(value,0.1*error);
    		// TODO be cool and figure out a way to swap without using a temporary variable
    		Double tmp = value;
    		value = error;
    		error = tmp;
    	}

		// A value of precisely 0 formats as if it were 1.  If the error is
    	// precisely 0, we print to 3 significant digits

    	if ( value == 0 ) {
    		value = 1;
    	}
    	if ( error == 0 ) {
    		error = 0.1*value;
    	}

		// Arithmetically we have to draw the limit somewhere.  It is
		// unlikely that numbers (and errors) < 1e-10 are reasonably
		// printed using this limited technique.
    	value = max(value,1e-10);
    	error = max(error,1e-8);

    	// Print only to two significant digits in the error
    	error = 0.1*error;

    	// Generate format

    	uInt before = max(int(log10(value))+1,1);     // In case value << 1 ==> log10 < 0
    	uInt after = 0;
    	//String format="%"+str(sign+before)+".0f"
    	uInt width = sign + before;
    	if ( log10(error) < 0 ) {
    		after = int(fabs(log10(error)))+1;
    		//format="%"+str(sign+before+1+after)+"."+str(after)+"f"
    		width = sign + before + after + 1;
    	}
    	return after;
    }

    String ImageFitter::_gaussianToString(
    	Quantity major, Quantity minor, Quantity posangle,
    	Quantity majorErr, Quantity minorErr, Quantity posanErr,
    	Bool includeUncertainties
    ) const {
    	// Inputs all as angle quanta
    	Vector<String> angUnits(5);
    	angUnits[0] = "deg";
    	angUnits[1] = "arcmin";
    	angUnits[2] = "arcsec";
    	angUnits[3] = "marcsec";
    	angUnits[4] = "uarcsec";
    	// First force position angle to be between 0 and 180 deg
    	if(posangle.getValue() < 0) {
    		posangle + Quantity(180, "deg");
    	}

    	String prefUnits;
    	Quantity vmax(max(fabs(major.getValue("arcsec")), fabs(minor.getValue("arcsec"))), "arcsec");

       	for (uInt i=0; i<angUnits.size(); i++) {
        	prefUnits = angUnits[i];
        	if(vmax.getValue(prefUnits) > 1) {
        		break;
        	}
        }
       	major.convert(prefUnits);
       	minor.convert(prefUnits);
       	majorErr.convert(prefUnits);
       	minorErr.convert(prefUnits);

    	Double vmaj = major.getValue();
    	Double vmin = minor.getValue();

    	// Formatting as "value +/- err" for symmetric errors

    	Double dmaj = majorErr.getValue();
    	Double dmin = minorErr.getValue();
    	// position angle is always in degrees cuz users like that
    	Double pa  = posangle.getValue("deg");
    	Double dpa = posanErr.getValue("deg");

    	Vector<Double> majVec(2), minVec(2), paVec(2);
    	majVec[0] = vmaj;
    	majVec[1] = dmaj;
    	minVec[0] = vmin;
    	minVec[1] = dmin;
    	paVec[0] = pa;
    	paVec[1] = dpa;
    	uInt precision1 = _precision(majVec, minVec);
    	uInt precision2 = _precision(paVec, Vector<Double>(0));

    	ostringstream summary;
    	summary << std::fixed << setprecision(precision1);
    	summary << "       --- major axis:     " << major.getValue();
    	if (includeUncertainties) {
    		summary << " +/- " << majorErr.getValue();
    	}
    	summary << " " << prefUnits << endl;
    	summary << "       --- minor axis:     " << minor.getValue();
    	if (includeUncertainties) {
    	    summary << " +/- " << minorErr.getValue();
    	}
    	summary << " "<< prefUnits << endl;
    	summary << setprecision(precision2);
    	summary << "       --- position angle: " << pa;
    	if (includeUncertainties) {
    		summary << " +/- " << dpa;
    	}
    	summary << " deg" << endl;
    	return summary.str();
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
        	precision = _precision(fd, Vector<Double>());
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
        precision = _precision(pi, Vector<Double>());
        fluxes << std::fixed << setprecision(precision);
        fluxes << "       --- Peak:         " << peakIntensity.getValue()
 			<< " +/- " << peakIntensityError.getValue() << " "
 			<< peakIntensity.getUnit() << endl;
        fluxes << "       --- Polarization: " << stokesString << endl;
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
    			*itsLog << LogIO::NORMAL << "Appended results to file "
    					<< logfileName << LogIO::POST;
    		}
    	case File::CREATABLE:
    		if (status == File::CREATABLE || ! logfileAppend) {
    			String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
    			Int fd = FiledesIO::create(logfileName.c_str());
    			FiledesIO fio (fd, logfileName.c_str());
    			fio.write(output.length(), output.c_str());
    			FiledesIO::close(fd);
    			*itsLog << LogIO::NORMAL << action << " file "
    					<< logfileName << " with new log file"
    					<< LogIO::POST;
    		}
    		break;
    	case File::NOT_OVERWRITABLE:
    		*itsLog << LogIO::WARN << "Unable to write to file "
    		<< logfileName << LogIO::POST;
    		break;
    	case File::NOT_CREATABLE:
    		*itsLog << LogIO::WARN << "Cannot create log file "
    		<< logfileName << LogIO::POST;
    		break;
    	}
    }

    void ImageFitter::_writeNewEstimatesFile() const {
    	ostringstream out;
    	for (uInt i=0; i<results.nelements(); i++) {
    		out << peakIntensities[i].getValue() << ", "
    				<< pixelPositions[i][0] << ", " << pixelPositions[i][1] << ", "
    				<< majorAxes[i] << ", " << minorAxes[i] << ", "
    				<< positionAngles[i] << endl;
    	}
    	String output = out.str();
    	File estimates(newEstimatesFileName);
    	switch (File::FileWriteStatus status = estimates.getWriteStatus()) {
    	case File::NOT_OVERWRITABLE:
    		*itsLog << LogIO::WARN << "Unable to write to file "
				<< newEstimatesFileName << LogIO::POST;
    		break;
    	case File::NOT_CREATABLE:
    		*itsLog << LogIO::WARN << "Cannot create estimates file "
				<< newEstimatesFileName << LogIO::POST;
    		break;
    	default:
    		String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
    		Int fd = FiledesIO::create(newEstimatesFileName.c_str());
    		FiledesIO fio(fd, logfileName.c_str());
    		fio.write(output.length(), output.c_str());
    		FiledesIO::close(fd);
    		*itsLog << LogIO::NORMAL << action << " file "
    				<< newEstimatesFileName << " with new estimates file"
    				<< LogIO::POST;
    	}
    }
}

