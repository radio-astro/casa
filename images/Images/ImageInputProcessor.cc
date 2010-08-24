
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
#include <images/Regions/RegionManager.h>
#include <measures/Measures/Stokes.h>

const String ImageInputProcessor::ALL = "ALL";

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
	const StokesControl& stokesControl, const casa::Bool& allowMultipleBoxes,
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
	const StokesControl& stokesControl, const Bool& allowMultipleBoxes,
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
    const String& chans, const StokesControl& stokesControl,
    const Bool& allowMultipleBoxes,
    const Vector<Coordinate::Type>* requiredCoordinateTypes
) {
	LogOrigin origin("ImageInputProcessor", __FUNCTION__);
    *_log << origin;
    if (outputStruct != 0) {
    	_checkOutputs(outputStruct);
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

    if (! box.empty()) {
    	if (box.freq(",") % 4 != 3) {
    		*_log << "box not specified correctly" << LogIO::EXCEPTION;
    	}
    	Vector<uInt> chanEndPts = _setSpectralRanges(chans, metaData);
    	Vector<uInt> polEndPts = _setPolarizationRanges(
    		stokes, metaData, image->name(), stokesControl
    	);
    	Vector<Double> boxCorners = _setBoxCorners(box);
    	_setRegion(
    		regionRecord, diagnostics, boxCorners,
    		chanEndPts, polEndPts, metaData, image
    	);
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
    	Vector<uInt> chanEndPts = _setSpectralRanges(chans, metaData);
    	Vector<uInt> polEndPts = _setPolarizationRanges(
    		stokes, metaData, image->name(), stokesControl
    	);
    	_setRegion(
    		regionRecord, diagnostics, boxCorners,
    		chanEndPts, polEndPts, metaData, image
    	);
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
    cout << "*** stokes " << stokes << endl;
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


Vector<uInt> ImageInputProcessor::_setSpectralRanges(
	String specification, const ImageMetaData& metaData
) {
	Vector<uInt> ranges(0);
    if (! metaData.hasSpectralAxis()) {
    	return ranges;
    }

	specification.trim();
	specification.upcase();
	uInt nchan = metaData.nChannels();
	if (specification.empty() || specification == ALL) {
		ranges.resize(2);
		ranges[0] = 0;
		ranges[1] = nchan - 1;
		return ranges;
	}

    // First split on commas
    Vector<String> parts = stringToVector(specification, ',');
    ranges.resize(2*parts.size());
	Regex regexuInt("^[0-9]+$");
	Regex regexRange("^[0-9]+[ \n\t\r\v\f]*-[ \n\t\r\v\f]*[0-9]+$");
	Regex regexLT("^<.*$");
	Regex regexLTEq("^<=.*$");
	Regex regexGT("^>.*$");
	Regex regexGTEq("^>=.*$");

    for (uInt i=0; i<parts.size(); i++) {
    	parts[i].trim();
    	uInt min = 0;
    	uInt max = 0;
    	if (parts[i].matches(regexuInt)) {
    		// just one channel
    		min = String::toInt(parts[i]);
			max = min;
    	}
    	else if(parts[i].matches(regexRange)) {
    		// a range of channels
    		Vector<String> values = stringToVector(parts[i], '-');
    		if (values.size() != 2) {
    			*_log << "Incorrect specification for channel range "
    				<< parts[i] << LogIO::EXCEPTION;
    		}
    		values[0].trim();
    		values[1].trim();
    		for(uInt j=0; j < 2; j++) {
    			if (! values[j].matches(regexuInt)) {
    				*_log << "For channel specification " << values[j]
    				    << " is not a non-negative integer in "
    					<< parts[i] << LogIO::EXCEPTION;
    			}
    		}
    		min = String::toInt(values[0]);
    		max = String::toInt(values[1]);
    	}
    	else if (parts[i].matches(regexLT)) {
    		String maxs = parts[i].matches(regexLTEq) ? parts[i].substr(2) : parts[i].substr(1);
    		maxs.trim();
    		if (! maxs.matches(regexuInt)) {
    			*_log << "In channel specification, " << maxs
    				<< " is not a non-negative integer in " << parts[i]
    				<< LogIO::EXCEPTION;
    		}
    		min = 0;
    		max = String::toInt(maxs);
    		if (! parts[i].matches(regexLTEq)) {
    			if (max == 0) {
    				*_log << "In channel specification, max channel cannot "
    					<< "be less than zero in " + parts[i];
    			}
    			else {
    				max--;
    			}
    		}
    	}
    	else if (parts[i].matches(regexGT)) {
       		String mins = parts[i].matches(regexGTEq)
       			? parts[i].substr(2)
       			: parts[i].substr(1);
       		mins.trim();
        	if (! mins.matches(regexuInt)) {
        		*_log << " In channel specification, " << mins
        			<< " is not an integer in " << parts[i]
        		    << LogIO::EXCEPTION;
        	}
        	max = nchan - 1;
        	min = String::toInt(mins);
        	if(! parts[i].matches(regexGTEq)) {
        		min++;
        	}
        	if (min > nchan - 1) {
        		*_log << "Min channel cannot be greater than the (zero-based) number of channels ("
        			<< nchan - 1 << ") in the image" << LogIO::EXCEPTION;
        	}
    	}
    	else {
    		*_log << "Invalid channel specification in " << parts[i]
    		    << " of spec " << specification << LogIO::EXCEPTION;
    	}
    	if (min > max) {
    		*_log << "Min channel " << min << " cannot be greater than max channel "
    			<< max << " in " << parts[i] << LogIO::EXCEPTION;
    	}
    	else if (max >= nchan) {
    		*_log << "Zero-based max channel " << max
    			<< " must be less than the total number of channels ("
    			<< nchan << ") in the channel specification " << parts[i] << LogIO::EXCEPTION;
    	}
    	ranges[2*i] = min;
    	ranges[2*i + 1] = max;

    }
    Vector<uInt> consolidatedRanges = _consolidateAndOrderRanges(ranges);
    _nSelectedChannels = 0;
    for (uInt i=0; i<consolidatedRanges.size()/2; i++) {
    	_nSelectedChannels += consolidatedRanges[2*i + 1] - consolidatedRanges[2*i] + 1;
    }
    return consolidatedRanges;
}

Vector<uInt> ImageInputProcessor::_consolidateAndOrderRanges(
	const Vector<uInt>& ranges
) const {
	uInt arrSize = ranges.size()/2;
	uInt arrMin[arrSize];
	uInt arrMax[arrSize];
	for (uInt i=0; i<arrSize; i++) {
		arrMin[i] = ranges[2*i];
		arrMax[i] = ranges[2*i + 1];
	}
	Sort sort;
	sort.sortKey (arrMin, TpUInt);
	sort.sortKey (arrMax, TpUInt, 0, Sort::Descending);
	Vector<uInt> inxvec;
	Vector<uInt> consol(0);
	sort.sort(inxvec, arrSize);
	for (uInt i=0; i<arrSize; i++) {
		uInt idx = inxvec(i);
		uInt size = consol.size();
		uInt min = arrMin[idx];
		uInt max = arrMax[idx];
		uInt lastMax = (i == 0) ? 0 : consol[size-1];
		if (i==0) {
			consol.resize(2, True);
			consol[0] = min;
			consol[1] = max;
		}
		else if (
			// overlaps previous range, so extend
			(min < lastMax && max > lastMax)
			// or contiguous with previous range, so extend
			|| min == lastMax + 1
		) {
			consol[size-1] = max;
		}

		else if (min > lastMax + 1) {
			// non overlap of and not contiguous with previous range,
			// so create new end point pair
			uInt newSize = consol.size()+2;
			consol.resize(newSize, True);
			consol[newSize-2] = min;
			consol[newSize-1] = max;
		}
	}
	return consol;
}

Vector<uInt> ImageInputProcessor::_setPolarizationRanges(
	String& specification, const ImageMetaData& metaData,
	const String& imageName, const StokesControl& stokesControl
) const {
    Vector<uInt> ranges(0);
    if (! metaData.hasPolarizationAxis()) {
    	return ranges;
    }
	specification.trim();
	specification.upcase();
	if (specification == ALL) {
		ranges.resize(2);
		ranges[0] = 0;
		ranges[1] = metaData.nStokes() - 1;
		return ranges;
	}
	if (specification.empty()) {
		ranges.resize(2);
		ranges[0] = 0;
		switch (stokesControl) {
			case USE_FIRST_STOKES:
				ranges[1] = 0;
				specification = metaData.stokesAtPixel(0);
				break;
			case USE_ALL_STOKES:
				ranges[1] = metaData.nStokes() - 1;
				specification = "ALL";
				break;
			default:
				// bug if we get here
				*_log << "Logic error, unhandled stokes control" << LogIO::EXCEPTION;
		};

		return ranges;
	}
    // First split on commas. Commas seem to have been supported at some point
	// in the past for polarization specification.
    Vector<String> parts = stringToVector(specification, ',');

    Vector<String> polNames = Stokes::allNames(False);
    uInt nNames = polNames.size();
    Vector<uInt> nameLengths(nNames);
    for (uInt i=0; i<nNames; i++) {
    	nameLengths[i] = polNames[i].length();
    }
    uInt *lengthData = nameLengths.data();
    Vector<uInt> idx(nNames);
    Sort sorter;
    sorter.sortKey(lengthData, TpUInt, 0, Sort::Descending);
    sorter.sort(idx, nNames);

    Vector<String> sortedNames(nNames);
    for (uInt i=0; i<nNames; i++) {
    	sortedNames[i] = polNames[idx[i]];
    	sortedNames[i].upcase();
    }

    for (uInt i=0; i<parts.size(); i++) {
    	String part = parts[i];
    	Vector<String>::iterator iter = sortedNames.begin();
    	while (iter != sortedNames.end() && ! part.empty()) {
    		if (part.startsWith(*iter)) {
    			Int stokesPix = metaData.stokesPixelNumber(*iter);
    			if (stokesPix >= 0) {
    				uInt newSize = ranges.size() + 2;
    				ranges.resize(newSize, True);
    				ranges[newSize-2] = stokesPix;
    				ranges[newSize-1] = stokesPix;
    				// consume the string
    				part = part.substr(iter->length());
    				if (! part.empty()) {
    					// reset the iterator to start over at the beginning of the list for
    					// the next specified polarization
    					iter = sortedNames.begin();
    				}
    			}
    			else {
    				*_log << "Polarization " << *iter << " specified in "
						<< parts[i] << " does not exist in " << imageName
						<< LogIO::EXCEPTION;
    			}
    		}
    		else {
    			iter++;
    		}
    	}
    	if (! part.empty()) {
    		*_log << "(Sub)String " << part << " in stokes specification part " << parts[i]
    		     << " does not match a known polarization." << LogIO::EXCEPTION;
    	}
    }
    return _consolidateAndOrderRanges(ranges);
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

Vector<Double> ImageInputProcessor::_setBoxCorners(const String& box) const {
	Vector<String> boxParts = stringToVector(box);
	AlwaysAssert(boxParts.size() % 4 == 0, AipsError);
	Vector<Double> corners(boxParts.size());
	for(uInt i=0; i<boxParts.size()/4; i++) {
	    uInt index = 4*i;
	    for (uInt j=0; j<4; j++) {
	    	boxParts[index + j].trim();
	    	if (! boxParts[index + j].matches(RXdouble)) {
	    		*_log << "Box spec contains non numeric characters and so is invalid"
	    			<< LogIO::EXCEPTION;
	    	}
	    	corners[index + j] = String::toDouble(boxParts[index + j]);
	    }
	}
	return corners;
}

void ImageInputProcessor::_setRegion(
	Record& regionRecord, String& diagnostics,
	const Vector<Double>& boxCorners, const Vector<uInt>& chanEndPts,
	const Vector<uInt>& polEndPts, const ImageMetaData& md,
	const ImageInterface<Float> *image
) const {
	String method(__FUNCTION__);
	method += "_1";
	LogOrigin origin("ImageInputProcessor", method);
	*_log << origin;
    IPosition imShape = image->shape();
    Vector<Double> blc(imShape.nelements());
    Vector<Double> trc(imShape.nelements());
    CoordinateSystem csys = image->coordinates();
    Vector<Int> directionAxisNumbers = md.directionAxesNumbers();
    Int spectralAxisNumber = md.spectralAxisNumber();
    Int polarizationAxisNumber = md.polarizationAxisNumber();

    Vector<Double> xCorners(boxCorners.size()/2);
    Vector<Double> yCorners(xCorners.size());
    for (uInt i=0; i<xCorners.size(); i++) {
    	Double x = boxCorners[2*i];
    	Double y = boxCorners[2*i + 1];

    	if (x < 0 || y < 0 ) {
    		*_log << "blc in box spec is less than 0" << LogIO::EXCEPTION;
    	}
    	if (
    		x >= imShape[directionAxisNumbers[0]]
    	    || y >= imShape[directionAxisNumbers[1]]
    	) {
    		*_log << "trc in box spec is greater than or equal to number "
    			<< "of direction pixels in the image" << LogIO::EXCEPTION;
    	}
    	xCorners[i] = x;
    	yCorners[i] = y;
    }

    RegionManager rm;
    ImageRegion imRegion;
    Vector<Double> polEndPtsDouble(polEndPts.size());
    for (uInt i=0; i<polEndPts.size(); i++) {
    	polEndPtsDouble[i] = (Double)polEndPts[i];
    }

    Vector<Double> chanEndPtsDouble(chanEndPts.size());
    for (uInt i=0; i<chanEndPts.size(); i++) {
    	chanEndPtsDouble[i] = (Double)chanEndPts[i];
    }

    HashMap<uInt, Vector<Double> > axisCornerMap;
    for (Int axisNumber=0; axisNumber<(Int)image->ndim(); axisNumber++) {
    	if (directionAxisNumbers.size() > 1 && axisNumber == directionAxisNumbers[0]) {
			axisCornerMap(axisNumber) = xCorners;
    	}
    	else if (directionAxisNumbers.size() > 1 && axisNumber == directionAxisNumbers[1]) {
    		axisCornerMap(axisNumber) = yCorners;
    	}
    	else if (axisNumber == spectralAxisNumber) {
    		axisCornerMap(axisNumber) = chanEndPtsDouble;
    	}
    	else if (axisNumber == polarizationAxisNumber) {
    		axisCornerMap(axisNumber) = polEndPtsDouble;
    	}
    	else {
    		*_log << "Unhandled image axis number " << axisNumber
    			<< LogIO::EXCEPTION;
    	}
    }

    uInt nRegions = 1;
    if (md.hasDirectionCoordinate()) {
    	nRegions *= boxCorners.size()/4;
    }
    if (md.hasPolarizationAxis()) {
    	nRegions *= polEndPts.size()/2;
    }
    if (md.hasSpectralAxis()) {
    	nRegions *= chanEndPts.size()/2;
    }

    for (uInt i=0; i<nRegions; i++) {
    	for (uInt axisNumber=0; axisNumber<image->ndim(); axisNumber++) {
    		blc(axisNumber) = axisCornerMap(axisNumber)[2*i];
    		trc(axisNumber) = axisCornerMap(axisNumber)[2*i + 1];
    	}
		LCBox lcBox(blc, trc, imShape);
		WCBox wcBox(lcBox, csys);
		ImageRegion thisRegion(wcBox);
		imRegion = (i == 0)
			? thisRegion
			: imRegion = *rm.doUnion(imRegion, thisRegion);
    }

    ostringstream os;

    os << "Used image region from " << endl;
    if (md.hasDirectionCoordinate()) {
    	os << "    position box corners: ";
    	for (uInt i=0; i<boxCorners.size()/4; i++) {
    		os << boxCorners[4*i] << ", " << boxCorners[4*i + 1]
    		   << ", " << boxCorners[4*i + 2] << ", " << boxCorners[4*i + 3];
    		if (i < boxCorners.size()/4 - 1) {
    			os << "; ";
    		}
    	}

    }
    if (md.hasSpectralAxis()) {
    	os << "    spectral channel ranges: " << _pairsToString(chanEndPts);
    }
    if (md.hasPolarizationAxis()) {
    	os << "    polarization pixel ranges: " << _pairsToString(polEndPts);
    }
    regionRecord = Record(imRegion.toRecord(""));

    //_printRecordFields(regionRecord);

    Int fieldNumber = regionRecord.fieldNumber("nr");
    if (fieldNumber >= 0) {
    	Int nr;
    	regionRecord.get(regionRecord.fieldNumber("nr"), nr);
    }
    diagnostics = os.str();
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

String ImageInputProcessor::_cornersToString(const Vector<Double>& corners) const {
	ostringstream os;
	uInt nCorners = corners.size()/4;
	for (uInt i=0; i<nCorners; i++) {
		os << "blc: (" << corners[4*i] << ", " << corners[4*i + 1] << "), trc: "
			<< corners[4*i + 2] << ", " << corners[4*i + 3] << ")";
		if (i < nCorners - 1) {
			os << endl;
		}
	}
	return os.str();
}

void ImageInputProcessor::_checkOutputs(
	Vector<OutputStruct> *output
) const {
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
				*_log << label << " cannot be blank" << LogIO::EXCEPTION;
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
			*_log << logLevel << "Requested " << label << " " << name
				<< " cannot be created so will not be written" << logAction;
			*(iter->outputFile) = "";
			break;
		case File::NOT_OVERWRITABLE:
			*_log << logLevel << "There is already a file or directory named "
				<< name << " which cannot be overwritten so the " << label
				<< " will not be written" << logAction;
			*(iter->outputFile) = "";
			break;
		case File::OVERWRITABLE:
			if (! replaceable) {
				*_log << logLevel << "Replaceable flag is false and there is "
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


