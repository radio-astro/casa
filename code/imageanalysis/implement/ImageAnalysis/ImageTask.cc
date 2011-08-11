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

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

namespace casa {

ImageTask::ImageTask(
	const ImageInterface<Float> *const &image,
	const String& region, const Record *const &regionPtr,
	const String& box, const String& chanInp,
	const String& stokes, const String& maskInp,
    const String& outname, const Bool overwrite
) : _log(new LogIO()), _image(image), _regionPtr(regionPtr),
	_region(region), _box(box),
	_chan(chanInp), _stokesString(stokes), _mask(maskInp),
	_outname(outname), _overwrite(overwrite) {
    FITSImage::registerOpenFunction();
    MIRIADImage::registerOpenFunction();
}

ImageTask::~ImageTask() {}

vector<ImageInputProcessor::OutputStruct> ImageTask::_getOutputStruct() {
	vector<ImageInputProcessor::OutputStruct> outputs(0);
    _outname.trim();
    if (! _outname.empty()) {
        ImageInputProcessor::OutputStruct outputImage;
        outputImage.label = "output image";
        outputImage.outputFile = &_outname;
        outputImage.required = True;
        outputImage.replaceable = _overwrite;
        outputs.push_back(outputImage);
    }
    return outputs;
}

void ImageTask::_construct() {
	String diagnostics;
	vector<ImageInputProcessor::OutputStruct> outputs = _getOutputStruct();
	vector<ImageInputProcessor::OutputStruct> *outputPtr = outputs.size() > 0
		? &outputs
		: 0;
	vector<Coordinate::Type> necCoords = _getNecessaryCoordinates();
	vector<Coordinate::Type> *coordsPtr = necCoords.size() > 0
		? &necCoords
		: 0;
	ImageInputProcessor inputProcessor;
	inputProcessor.process(
		_regionRecord, diagnostics, outputPtr,
    	_stokesString, _image, _regionPtr,
    	_region, _box, _chan,
    	_getStokesControl(), False,
    	coordsPtr
    );
}

void ImageTask::_removeExistingOutfileIfNecessary() const {
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
			*_log << "File " << _outname
				<< " exists but overwrite is false so it cannot be overwritten"
				<< LogIO::EXCEPTION;
		}
	}
}

String ImageTask::_summaryHeader() const {
	String region = _box.empty() ? _region : "";
	ostringstream os;
	os << "Input parameters ---" << endl;
	os << "       --- imagename:           " << _image->name() << endl;
	os << "       --- region:              " << region << endl;
	os << "       --- box:                 " << _box << endl;
	os << "       --- channels:            " << _chan << endl;
	os << "       --- stokes:              " << _stokesString << endl;
	os << "       --- mask:                " << _mask << endl;
	return os.str();
}


}

