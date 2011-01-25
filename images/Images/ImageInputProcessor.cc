
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

#include <images/Images/ImageInputProcessor.h>

#include <casa/Containers/HashMap.h>
#include <casa/Utilities/Sort.h>
#include <casa/iostream.h>

#include <images/Images/FITSImage.h>
#include <images/Images/ImageMetaData.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/MIRIADImage.h>
#include <images/Regions/WCBox.h>
#include <measures/Measures/Stokes.h>

ImageInputProcessor::ImageInputProcessor()
: _log(new LogIO()), _processHasRun(False),
  _nSelectedChannels(0)
{}

ImageInputProcessor::~ImageInputProcessor() {
	delete _log;
}

void ImageInputProcessor::process(
	ImageInterface<Float>*& image, Record& regionRecord,
	String& diagnostics, Vector<OutputStruct> *outputStruct,
	String& stokes, const String& imagename, const Record* regionPtr,
	const String& regionName, const String& box,
	const String& chans,
	const RegionManager::StokesControl& stokesControl, const Bool& allowMultipleBoxes,
	const Vector<Coordinate::Type> *requiredCoordinateTypes
) {
	LogOrigin origin("ImageInputProcessor", __FUNCTION__);
    *_log << origin;
    if (imagename.empty()) {
        *_log << "imagename cannot be blank" << LogIO::EXCEPTION;
    }
    // Register the functions to create a FITSImage or MIRIADImage object.
    FITSImage::registerOpenFunction();
    MIRIADImage::registerOpenFunction();
    image = 0;
    ImageUtilities::openImage(image, imagename, *_log);
    if (image == 0) {
    	*_log << origin;
    	*_log << "Unable to open image " << imagename << LogIO::EXCEPTION;
    }
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes
	);
}

void ImageInputProcessor::process(
	Record& regionRecord, String& diagnostics,
	Vector<OutputStruct> *outputStruct, String& stokes,
	const ImageInterface<Float>*& image,
	const Record* regionPtr, const String& regionName,
	const String& box, const String& chans,
	const RegionManager::StokesControl& stokesControl, const Bool& allowMultipleBoxes,
	const Vector<Coordinate::Type> *requiredCoordinateTypes
) {
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes
	);
}

void ImageInputProcessor::_process(
    Record& regionRecord,
    String& diagnostics, Vector<OutputStruct>* outputStruct,
    String& stokes, const ImageInterface<Float>* image,
    const Record*& regionPtr,
    const String& regionName, const String& box,
    const String& chans, const RegionManager::StokesControl& stokesControl,
    const Bool& allowMultipleBoxes,
    const Vector<Coordinate::Type>* requiredCoordinateTypes
) {
	LogOrigin origin("ImageInputProcessor", __FUNCTION__);
    *_log << origin;
    if (outputStruct != 0) {
    	checkOutputs(outputStruct, *_log);
    }
    *_log << origin;
    if (requiredCoordinateTypes) {
    	for (
    		Vector<Coordinate::Type>::const_iterator iter = requiredCoordinateTypes->begin();
    		iter != requiredCoordinateTypes->end(); iter++
    	) {
    		if (image->coordinates().findCoordinate(*iter) < 0) {
    			*_log << "Image " << image->name() << " does not have required coordinate "
					<< Coordinate::typeToString(*iter) << LogIO::EXCEPTION;
    		}
    	}
    }
	ImageMetaData metaData(*image);
	_nSelectedChannels = metaData.nChannels();
	// region specification order:
	// 1: process box if given explicitly
	// 2. else process region if pointer to region record specified
	// 3. else process region if region name specified
	// 4. else no box or region specified, process region as entire
	//    positional plane anding with chans and stokes specs

	RegionManager regionMgr(image->coordinates());

    if (! box.empty()) {
    	if (box.freq(",") % 4 != 3) {
    		*_log << "box not specified correctly" << LogIO::EXCEPTION;
    	}
    	regionRecord = regionMgr.fromBCS(
    		diagnostics, _nSelectedChannels, stokes,
    		chans, stokesControl, box, image->shape()
    	).toRecord("");
    	*_log << origin;
    	*_log << LogIO::NORMAL << "Using specified box(es) " << box << LogIO::POST;
    }
    else if (regionPtr != 0) {
    	_setRegion(regionRecord, diagnostics, regionPtr);
    	*_log << origin;
    	*_log << LogIO::NORMAL << "Set region from supplied region record"
    		<< LogIO::POST;
        stokes = _stokesFromRecord(regionRecord, metaData);
    }
    else if (! regionName.empty()) {
    	_setRegion(regionRecord, diagnostics, image, regionName);
    	*_log << origin;
       	*_log << LogIO::NORMAL << "Set region from supplied region file "
        	<< regionName << LogIO::POST;
    	stokes = _stokesFromRecord(regionRecord, metaData);
    }
    else {
    	// nothing specified, use entire positional plane with spectral and polarization specs
    	Vector<Double> boxCorners(0);
    	if(metaData.hasDirectionCoordinate()) {
    		Vector<Int> dirShape = metaData.directionShape();
    		boxCorners.resize(4);
    		boxCorners[0] = 0;
    		boxCorners[1] = 0;
    		boxCorners[2] = dirShape[0] - 1;
    		boxCorners[3] = dirShape[1] - 1;
    	}
    	Vector<uInt> chanEndPts, polEndPts;
        regionRecord = regionMgr.fromBCS(
        	diagnostics, _nSelectedChannels, stokes,
        	chanEndPts, polEndPts, chans,
        	stokesControl, boxCorners, image->shape()
        ).toRecord("");
        *_log << origin;
    	*_log << LogIO::NORMAL << "No region specified. Using full positional plane."
    		<< LogIO::POST;
    	if (chans.empty()) {
    		*_log << LogIO::NORMAL << "Using all spectral channels."
    			<< LogIO::POST;
    	}
    	else {
    		*_log << LogIO::NORMAL << "Using channel range(s) "
    			<< _pairsToString(chanEndPts) << LogIO::POST;
    	}
    	if (!stokes.empty()) {
    		*_log << LogIO::NORMAL << "Using polarizations " << stokes << LogIO::POST;
    	}
    	else {
    		*_log << LogIO::NORMAL << "Using polarization range(s) "
    			<< _pairsToString(polEndPts) << LogIO::POST;
    	}
    }
    if (!allowMultipleBoxes && regionRecord.fieldNumber("regions") >= 0) {
    	*_log << "Only a single n-dimensional rectangular region is supported."
    		<< LogIO::EXCEPTION;
    }
    _processHasRun = True;
}

uInt ImageInputProcessor::nSelectedChannels() const {
	if (! _processHasRun) {
	    *_log << LogOrigin("ImageInputProcessor", __FUNCTION__);
		*_log << "Programming logic error, ImageInputProcessor::process() must be called "
			<< "before ImageInputProcessor::" << __FUNCTION__ << "()" << LogIO::EXCEPTION;
	}
	return _nSelectedChannels;
}

String ImageInputProcessor::_stokesFromRecord(const Record& region, const ImageMetaData& metaData) const {
    String stokes = "";
 	if(metaData.hasPolarizationAxis()) {
 		Int polAxis = metaData.polarizationAxisNumber();
 		uInt stokesBegin, stokesEnd;
 		Array<Double> blc, trc;
 		region.toArray("blc", blc);
 		region.toArray("trc", trc);
 		stokesBegin = (uInt)((Vector<Double>)blc)[polAxis];
 		stokesEnd = (uInt)((Vector<Double>)trc)[polAxis];
 		if (region.isDefined("oneRel") && region.asBool("oneRel")) {
 			stokesBegin--;
 			stokesEnd--;
 		}
 		for (uInt i=stokesBegin; i<=stokesEnd; i++) {
 			stokes += metaData.stokesAtPixel(i);
 		}
 	}
 	return stokes;
}

void ImageInputProcessor::_setRegion(
	Record& regionRecord, String& diagnostics,
	const Record* regionPtr
) const {
 	// region record pointer provided
 	regionRecord = *(regionPtr->clone());
 	// set stokes from the region record
 	diagnostics = "used provided region record";
}

void ImageInputProcessor::_setRegion(
	Record& regionRecord, String& diagnostics,
	const ImageInterface<Float> *image, const String& regionName
) const {
	// region name provided
	ImageRegion imRegion;
	Regex otherImage("(.*)+:(.*)+");
	try {
		if (regionName.matches(otherImage)) {
			String res[2];
			casa::split(regionName, res, 2, ":");
			PagedImage<Float> other(res[0]);
			imRegion = other.getRegion(res[1]);
		}
		else {
			imRegion = image->getRegion(regionName);
		}
	}
	catch (AipsError) {
		*_log << "Unable to open region file " << regionName << LogIO::EXCEPTION;
	}
    regionRecord = Record(imRegion.toRecord(""));
    diagnostics = "Used image region " + regionName;
}

String ImageInputProcessor::_pairsToString(const Vector<uInt>& pairs) const {
	ostringstream os;
	uInt nPairs = pairs.size()/2;
	for (uInt i=0; i<nPairs; i++) {
		os << pairs[2*i] << " - " << pairs[2*i + 1];
		if (i < nPairs - 1) {
			os << "; ";
		}
	}
	return os.str();
}

void ImageInputProcessor::checkOutputs(
	Vector<OutputStruct> *output, LogIO& log
) {
	for (
		Vector<OutputStruct>::iterator iter = output->begin();
		iter != output->end();
		iter++
	) {
		String label = iter->label;
		String name = *(iter->outputFile);
		Bool required = iter->required;
		Bool replaceable = iter->replaceable;
		if (name.empty()) {
			if (required) {
				log << label << " cannot be blank" << LogIO::EXCEPTION;
			}
			else {
				continue;
			}
		}
		LogIO::Command logLevel = required ? LogIO::SEVERE : LogIO::WARN;
		LogIO::Command logAction = required ? LogIO::EXCEPTION : LogIO::POST;
		File f(name);
		switch (f.getWriteStatus()) {
		case File::NOT_CREATABLE:
			log << logLevel << "Requested " << label << " " << name
				<< " cannot be created so will not be written" << logAction;
			*(iter->outputFile) = "";
			break;
		case File::NOT_OVERWRITABLE:
			log << logLevel << "There is already a file or directory named "
				<< name << " which cannot be overwritten so the " << label
				<< " will not be written" << logAction;
			*(iter->outputFile) = "";
			break;
		case File::OVERWRITABLE:
			if (! replaceable) {
				log << logLevel << "Replaceable flag is false and there is "
					<< "already a file or directory named " << name
					<< " so the " << label << " will not be written"
					<< logAction;
				*(iter->outputFile) = "";
			}
			break;
		default:
			continue;
		}
	}
}
 
