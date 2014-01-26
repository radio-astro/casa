
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

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

#include <casa/Containers/HashMap.h>
#include <casa/Utilities/Sort.h>
#include <casa/iostream.h>

#include <images/Images/FITSImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/MIRIADImage.h>
#include <images/Regions/WCBox.h>
#include <imageanalysis/ImageAnalysis/ImageMetaData.h>

#include <measures/Measures/Stokes.h>

namespace casa {

ImageInputProcessor::ImageInputProcessor()
: _log(new LogIO()), _processHasRun(False),
  _nSelectedChannels(0)
{}

ImageInputProcessor::~ImageInputProcessor() {
	delete _log;
}

void ImageInputProcessor::process(
	ImageInterface<Float>*& image, Record& regionRecord,
	String& diagnostics, vector<OutputDestinationChecker::OutputStruct> *outputStruct,
	String& stokes, const String& imagename, const Record* regionPtr,
	const String& regionName, const String& box,
	const String& chans,
	const CasacRegionManager::StokesControl& stokesControl, const Bool& allowMultipleBoxes,
	const vector<Coordinate::Type> *const &requiredCoordinateTypes, Bool verbose
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
    ImageUtilities::openImage(image, imagename);
    if (image == 0) {
    	*_log << origin;
    	*_log << "Unable to open image " << imagename << LogIO::EXCEPTION;
    }
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes, verbose
	);
}

void ImageInputProcessor::process(
	Record& regionRecord, String& diagnostics,
	vector<OutputDestinationChecker::OutputStruct> * const outputStruct, String& stokes,
	const ImageInterface<Float> *const &image,
	const Record* regionPtr, const String& regionName,
	const String& box, const String& chans,
	const CasacRegionManager::StokesControl& stokesControl, const Bool& allowMultipleBoxes,
	const vector<Coordinate::Type> *const &requiredCoordinateTypes, Bool verbose
) {
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes, verbose
	);
}

void ImageInputProcessor::_process(
    Record& regionRecord,
    String& diagnostics, vector<OutputDestinationChecker::OutputStruct> * const outputStruct,
    String& stokes, const ImageInterface<Float> *const &image,
    const Record *const &regionPtr,
    const String& regionName, const String& box,
    const String& chans, const CasacRegionManager::StokesControl& stokesControl,
    const Bool& allowMultipleBoxes,
    const std::vector<Coordinate::Type> *const &requiredCoordinateTypes,
    Bool verbose
) {
	LogOrigin origin("ImageInputProcessor", __FUNCTION__);
    *_log << origin;
    if (outputStruct != 0) {
        OutputDestinationChecker::checkOutputs(outputStruct, *_log);

    }
    *_log << origin;
    if (requiredCoordinateTypes) {
    	for (
    		vector<Coordinate::Type>::const_iterator iter = requiredCoordinateTypes->begin();
    		iter != requiredCoordinateTypes->end(); iter++
    	) {
    		if (image->coordinates().findCoordinate(*iter) < 0) {
    			*_log << "Image " << image->name() << " does not have required coordinate "
					<< Coordinate::typeToString(*iter) << LogIO::EXCEPTION;
    		}
    	}
    }
	ImageMetaData<Float> md(image);
	_nSelectedChannels = md.nChannels();

	CasacRegionManager regionMgr(image->coordinates());
	regionRecord = regionMgr.fromBCS(
		diagnostics, _nSelectedChannels, stokes,
		regionPtr, regionName, chans,
		stokesControl, box, image->shape(), image->name(),
		verbose
	);
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

String ImageInputProcessor::_stokesFromRecord(
	const Record& region, const CoordinateSystem& csys
) const {
	// FIXME This implementation is incorrect for complex, recursive records
    String stokes = "";
 	if(csys.hasPolarizationCoordinate()) {
 		Int polAxis = csys.polarizationAxisNumber();
 		uInt stokesBegin, stokesEnd;
 		ImageRegion *imreg = ImageRegion::fromRecord(region, "");
 		Array<Float> blc, trc;
 		Bool oneRelAccountedFor = False;
 		if (imreg->isLCSlicer()) {
 			blc = imreg->asLCSlicer().blc();
 			trc = imreg->asLCSlicer().trc();
	 		stokesBegin = (uInt)((Vector<Float>)blc)[polAxis];
	 		stokesEnd = (uInt)((Vector<Float>)trc)[polAxis];
	 		oneRelAccountedFor = True;
 		}
 		else if (RegionManager::isPixelRegion(*(ImageRegion::fromRecord(region, "")))) {
			region.toArray("blc", blc);
			region.toArray("trc", trc);
	 		stokesBegin = (uInt)((Vector<Float>)blc)[polAxis];
	 		stokesEnd = (uInt)((Vector<Float>)trc)[polAxis];
		}
		else {
			Record blcRec = region.asRecord("blc");
			Record trcRec = region.asRecord("trc");
			stokesBegin = (Int)blcRec.asRecord(
				String("*" + String::toString(polAxis - 1))
			).asDouble("value");
			stokesEnd = (Int)trcRec.asRecord(
				String("*" + String::toString(polAxis - 1))
			).asDouble("value");
		}

 		if (! oneRelAccountedFor && region.isDefined("oneRel") && region.asBool("oneRel")) {
 			stokesBegin--;
 			stokesEnd--;
 		}
 		for (uInt i=stokesBegin; i<=stokesEnd; i++) {
 			stokes += csys.stokesAtPixel(i);
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
	File myFile(regionName);
	if (myFile.exists()) {
		Record *rec = RegionManager::readImageFile(regionName, "");
		regionRecord = *rec;
		delete rec;
		diagnostics = "Region read from file " + regionName;
	}
	else {

		ImageRegion imRegion;
		Regex otherImage("(.*)+:(.*)+");
		try {
			String imagename;
			if (regionName.matches(otherImage)) {
				String res[2];
				casa::split(regionName, res, 2, ":");
				imagename = res[0];
				PagedImage<Float> other(imagename);
				imRegion = other.getRegion(res[1]);
			}
			else {
				imRegion = image->getRegion(regionName);
				imagename = image->name();
			}
		    regionRecord = Record(imRegion.toRecord(""));
		    diagnostics = "Used region " + regionName + " from image "
		    		+ imagename + " table description";
		}
		catch (AipsError) {
			*_log << "Unable to open region file or region table description " << regionName << LogIO::EXCEPTION;
		}
	}
}

String ImageInputProcessor::_pairsToString(const std::vector<uInt>& pairs) const {
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

}
 
