
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

template<class T> void ImageInputProcessor::process(
	SPIIT image, Record& regionRecord,
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
    ImageInterface<Float> *imagePtr = 0;
    ImageUtilities::openImage(imagePtr, imagename);
    if (imagePtr == 0) {
    	*_log << origin;
    	*_log << "Unable to open image " << imagename << LogIO::EXCEPTION;
    }
    image.reset(imagePtr);
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes, verbose
	);
}

template<class T> void ImageInputProcessor::process(
	Record& regionRecord, String& diagnostics,
	std::vector<OutputDestinationChecker::OutputStruct> * const outputStruct, String& stokes,
	SPCIIT image,
	const Record* regionPtr, const String& regionName,
	const String& box, const String& chans,
	const CasacRegionManager::StokesControl& stokesControl, const Bool& allowMultipleBoxes,
	const std::vector<Coordinate::Type> *const &requiredCoordinateTypes, Bool verbose
) {
	_process(
		regionRecord, diagnostics, outputStruct, stokes,
		image, regionPtr, regionName, box, chans, stokesControl,
		allowMultipleBoxes, requiredCoordinateTypes, verbose
	);
}

template<class T> void ImageInputProcessor::_process(
    Record& regionRecord,
    String& diagnostics,
    std::vector<OutputDestinationChecker::OutputStruct> * const outputStruct,
    String& stokes, SPCIIT image,
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
	ImageMetaData md(image);
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

template<class T> void ImageInputProcessor::_setRegion(
	Record& regionRecord, String& diagnostics,
	const ImageInterface<T> *image, const String& regionName
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
		catch (const AipsError& x) {
			ThrowCc(
				"Unable to open region file or region table description "
				+ regionName
			);
		}
	}
}
}
 
