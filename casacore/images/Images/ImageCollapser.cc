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

#include <images/Images/ImageCollapser.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <images/Images/ImageInputProcessor.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>

namespace casa {

    ImageCollapser::ImageCollapser(
    	String aggString, const String& imagename,
    	const String& region, const String& box,
    	const String& chanInp, const String& stokes,
    	const String& maskInp, const uInt compressionAxis,
        const String& outname, const Bool overwrite
    ) : _log(new LogIO()), _image(0), _regionString(""),
		_chan(chanInp), _stokesString(stokes), _mask(maskInp),
		_outname(outname), _compressionAxis(compressionAxis),
		_overwrite(overwrite), _aggregateFunction(0) {
        _construct(aggString, imagename, box, region);
    }

    ImageCollapser::~ImageCollapser() {
        delete _log;
        delete _image;
    }

    ImageInterface<Float>* ImageCollapser::collapse(const Bool wantReturn) const {
        *_log << LogOrigin("ImageCollapser", __FUNCTION__);
    	ImageRegion *imageRegion = 0;
    	ImageRegion *maskRegion = 0;
    	SubImage<Float> subImage = SubImage<Float>::createSubImage(
    		imageRegion, maskRegion, *_image,
    		_regionRecord, _mask, _log, False
    	);
    	delete imageRegion;
    	delete maskRegion;
    	IPosition inShape = subImage.shape();
    	IPosition outShape = inShape;
    	outShape[_compressionAxis] = 1;
    	// Set the compressed axis reference pixel and reference value
    	CoordinateSystem outCoords(subImage.coordinates());
    	Vector<Double> blc, trc;
    	IPosition pixblc(inShape.nelements(), 0);
    	IPosition pixtrc = inShape - 1;
    	if(
    		! outCoords.toWorld(blc, pixblc)
    		|| ! outCoords.toWorld(trc, pixtrc)
    	) {
    		*_log << "Could not set new coordinate values" << LogIO::EXCEPTION;
    	}

    	Vector<Double> refValues = outCoords.referenceValue();
    	refValues[_compressionAxis] = (blc[_compressionAxis] + trc[_compressionAxis])/2;
       	if (! outCoords.setReferenceValue(refValues)) {
        	*_log << "Unable to set reference value" << LogIO::EXCEPTION;
        }
    	Vector<Double> refPixels = outCoords.referencePixel();
    	refPixels[_compressionAxis] = 0;
    	if (! outCoords.setReferencePixel(refPixels)) {
    		*_log << "Unable to set reference pixel" << LogIO::EXCEPTION;
    	}
    	ImageInterface<Float> *outImage = 0;
    	if (_outname.empty()) {
    		outImage = new TempImage<Float>(outShape, outCoords);
    	}
    	else {
    		File out(_outname);
    		if (out.exists()) {
    			// remove file if it exists which prevents emission of
    			// file is already open in table cache exceptions
    			if (_overwrite) {
    				if (out.isDirectory()) {
    					Directory dir(_outname);
    					dir.removeRecursive();
    				}
    				else if (out.isRegular()) {
    					RegularFile reg(_outname);
    					reg.remove();
    				}
    				else if (out.isSymLink()) {
    					SymLink link(_outname);
    					link.remove();
    				}
    			}
    			else {
    				// The only way this block can be entered is if a file by this name
    				// has been written between the checking of inputs in the constructor
    				// call and the call of this method.
    				*_log << "File " << _outname << " exists but overwrite is false so it cannot be overwritten"
    					<< LogIO::EXCEPTION;
    			}
    		}
    		outImage = new PagedImage<Float>(outShape, outCoords, _outname);
    	}
		IPosition shape(outShape.nelements(), 1);
		shape[_compressionAxis] = inShape[_compressionAxis];
    	for (uInt i=0; i<outShape.product(); i++) {
    		IPosition start = toIPositionInArray(i, outShape);
    		Array<Float> ary = subImage.getSlice(start, shape);
    		outImage->putAt(_aggregateFunction(ary), start);
    	}
    	if (! _outname.empty()) {
    		outImage->flush();
    	}
    	if (! wantReturn) {
    		delete outImage;
    		outImage = 0;
    	}
    	return outImage;
    }

    void ImageCollapser::_construct(
        String& aggString, const String& imagename,
        const String& box, const String& regionName
    ) {
    	LogOrigin logOrigin("ImageCollapser", __FUNCTION__);
        *_log << logOrigin;
        if (aggString.empty()) {
        	*_log << "Aggregate function name is not specified and it must be."
        		<< LogIO::EXCEPTION;
        }
    	Vector<ImageInputProcessor::OutputStruct> outputs(0);
        _outname.trim();
        if (! _outname.empty()) {
        	ImageInputProcessor::OutputStruct outputImage;
        	outputImage.label = "output image";
        	outputImage.outputFile = &_outname;
        	outputImage.required = True;
        	outputImage.replaceable = _overwrite;
        	outputs.resize(1);
        	outputs[0] = outputImage;
        }
        ImageInputProcessor inputProcessor;
        String diagnostics;
        Vector<ImageInputProcessor::OutputStruct> *outputPtr = outputs.size() > 0
        	? &outputs
        	: 0;
        inputProcessor.process(
        	_image, _regionRecord, diagnostics,
        	outputPtr, imagename, 0, regionName,
        	box, _chan, _stokesString,
        	ImageInputProcessor::USE_ALL_STOKES,
        	False
        );
        *_log << logOrigin;
        if (_compressionAxis >= _image->ndim()) {
        	*_log << "Specified zero-based compression axis (" << _compressionAxis
        		<< ") must be less than the number of axes in " << _image->name()
        		<< "(" << _image->ndim() << LogIO::EXCEPTION;
        }
        _setAggregateFunction(aggString);
    }

    void ImageCollapser::_setAggregateFunction(String& aggString) {
    	aggString.downcase();
    	String aString = aggString;
    	if (aString.startsWith("a")) {
			_aggregateFunction = casa::avdev;
    	}
    	else if (aString.startsWith("ma")) {
			_aggregateFunction = casa::max;
    	}
    	else if (aString.startsWith("mea")) {
			_aggregateFunction = casa::mean;
    	}
    	else if (aString.startsWith("med")) {
			_aggregateFunction = casa::median;
    	}
    	else if (aString.startsWith("mi")) {
			_aggregateFunction = casa::min;
    	}
    	else if (aString.startsWith("r")) {
			_aggregateFunction = casa::rms;
    	}
    	else if (aString.startsWith("st")) {
			_aggregateFunction = casa::stddev;
    	}
    	else if (aString.startsWith("su")) {
			_aggregateFunction = casa::sum;
    	}
    	else if (aString.startsWith("v")) {
			_aggregateFunction = casa::variance;
    	}
    	else {
    		*_log << "Unknown aggregate function specified by " << aggString << LogIO::EXCEPTION;
    	}
    }
}

