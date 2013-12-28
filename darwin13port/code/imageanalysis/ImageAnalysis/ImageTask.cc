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

#include <casa/IO/FilebufIO.h>
#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <imageanalysis/IO/LogFile.h>


namespace casa {

ImageTask::ImageTask(
	const shCImFloat image,
	const String& region, const Record *const &regionPtr,
	const String& box, const String& chanInp,
	const String& stokes, const String& maskInp,
    const String& outname, const Bool overwrite
) : _image(image), _log(new LogIO()), _regionPtr(regionPtr),
	_region(region), _box(box),
	_chan(chanInp), _stokesString(stokes), _mask(maskInp),
	_outname(outname), _overwrite(overwrite), _stretch(False),
	_logfileSupport(False), _logfileAppend(False),/* _logFD(0),*/
	_verbosity(NORMAL), _logfile() {
    FITSImage::registerOpenFunction();
    MIRIADImage::registerOpenFunction();
}

ImageTask::~ImageTask() {}

vector<OutputDestinationChecker::OutputStruct> ImageTask::_getOutputStruct() {
	vector<OutputDestinationChecker::OutputStruct> outputs(0);
    _outname.trim();
    if (! _outname.empty()) {
        OutputDestinationChecker::OutputStruct outputImage;
        outputImage.label = "output image";
        outputImage.outputFile = &_outname;
        outputImage.required = True;
        outputImage.replaceable = _overwrite;
        outputs.push_back(outputImage);
    }
    return outputs;
}

void ImageTask::_construct(Bool verbose) {
	String diagnostics;
	vector<OutputDestinationChecker::OutputStruct> outputs = _getOutputStruct();
	vector<OutputDestinationChecker::OutputStruct> *outputPtr = outputs.size() > 0
		? &outputs
		: 0;
	vector<Coordinate::Type> necCoords = _getNecessaryCoordinates();
	vector<Coordinate::Type> *coordsPtr = necCoords.size() > 0
		? &necCoords
		: 0;
	ThrowIf(
		_mustHaveSquareDirectionPixels()
		&& _image->coordinates().hasDirectionCoordinate()
		&& ! _image->coordinates().directionCoordinate().hasSquarePixels(),
		"This application requires that the input image must have square"
		"direction pixels, but the input image does not. Please regrid it"
		"so it does and rerun on the regridded image"
	);
	ImageInputProcessor inputProcessor;
	inputProcessor.process(
		_regionRecord, diagnostics, outputPtr,
    	_stokesString, _image.get(), _regionPtr,
    	_region, _box, _chan,
    	_getStokesControl(), _supportsMultipleRegions(),
    	coordsPtr, verbose
    );
}

void ImageTask::setRegion(const Record& region) {
    _regionRecord = region;
    _box = "";
    _chan = "";
    _stokesString = "";
    _region = "";
}

void ImageTask::_removeExistingFileIfNecessary(
	const String& filename, const Bool overwrite
) {
	File out(filename);
	if (out.exists()) {
		// remove file if it exists which prevents emission of
		// file is already open in table cache exceptions
		if (overwrite) {
			if (out.isDirectory()) {
				Directory dir(filename);
				dir.removeRecursive();
			}
			else if (out.isRegular()) {
				RegularFile reg(filename);
				reg.remove();
			}
			else if (out.isSymLink()) {
				SymLink link(filename);
				link.remove();
			}
		}
		else {
			LogIO log;
			log << LogOrigin("ImageTask", __FUNCTION__) << "File " << filename
				<< " exists but overwrite is false so it cannot be overwritten"
				<< LogIO::EXCEPTION;
		}
	}
}

void ImageTask::_removeExistingOutfileIfNecessary() const {
	_removeExistingFileIfNecessary(_outname, _overwrite);
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

void ImageTask::setLogfile(const String& lf) {
	if (! _logfileSupport) {
		*_log << "Logic Error: This task does not support writing of a log file" << LogIO::EXCEPTION;
	}
	try {
		_logfile.reset(new LogFile(lf));
		_logfile->setAppend(_logfileAppend);
	}
	catch (const AipsError& x) {}
	/*
	String mylf = lf;
	OutputDestinationChecker::OutputStruct logFile;
	logFile.label = "log file";
	logFile.outputFile = &mylf;
	logFile.required = False;
	logFile.replaceable = True;
	OutputDestinationChecker::checkOutput(logFile, *_log);
	_logfile = mylf;
	*/

}

// const String& ImageTask::_getLogfile() const {
const std::tr1::shared_ptr<LogFile> ImageTask::_getLogFile() const {
	if (! _logfileSupport) {
		*_log << "Logic Error: This task does not support writing of a log file" << LogIO::EXCEPTION;
	}
	return _logfile;
}

Bool ImageTask::_openLogfile() {
	if (_logfile.get() == 0) {
		return False;
	}
	*_log << LogOrigin(getClass(), __FUNCTION__);
	if (! _logfileSupport) {
		*_log << "Logic Error: This task does not support writing of a log file" << LogIO::EXCEPTION;
	}
	return _logfile->openFile();
	/*
	File log(_logfile);
	switch (File::FileWriteStatus status = log.getWriteStatus()) {
	case File::OVERWRITABLE:
		if (_logfileAppend) {
			_logFD = open(_logfile.c_str(), O_RDWR | O_APPEND);
		}
		// no break here to fall through to the File::CREATABLE block if logFileAppend is false
	case File::CREATABLE:
		if (status == File::CREATABLE || ! _logfileAppend) {
			// can fall throw from previous case block so status can be File::OVERWRITABLE
			String action = (status == File::OVERWRITABLE) ? "Overwrote" : "Created";
			_logFD = FiledesIO::create(_logfile.c_str());
		}
		break;
	default:
		// checks to see if the log file is not creatable or not writeable should have already been
		// done and if so _logFile set to the empty string so this method wouldn't be called in
		// those cases.
		*_log << "Programming logic error. This block should never be reached" << LogIO::EXCEPTION;
	}
	_logFileIO.reset(new FiledesIO(_logFD, _logfile.c_str()));

	return True;
	*/
}

void ImageTask::_closeLogfile() const {
	/*
	if (_logFD > 0) {
		FiledesIO::close(_logFD);
	}
	*/
	if (_logfile.get() != 0) {
		_logfile->closeFile();
	}
}

Bool ImageTask::_writeLogfile(
	const String& output, const Bool open, const Bool close
) {
	/*
	if (open) {
		if (! _openLogfile()) {
			return False;
		}
	}
	*/
	if (_logfile.get() == 0) {
		return False;
	}
	else {
		*_log << LogOrigin(getClass(), __FUNCTION__);
		if (! _logfileSupport) {
			*_log << "Logic Error: This task does not support writing of a log file" << LogIO::EXCEPTION;
		}
	}
	_logfile->write(output, open, close);
	/*
	_logFileIO->write(output.length(), output.c_str());
	if (close) {
		_closeLogfile();
	}
	*/
	return True;
}

void ImageTask::setLogfileAppend(const Bool a) {
	if (! _logfileSupport) {
		*_log << "Logic Error: This task does not support writing of a log file" << LogIO::EXCEPTION;
	}
	_logfileAppend = a;
	if (_logfile.get() != 0) {
		_logfile->setAppend(a);
	}
}

std::tr1::shared_ptr<ImageInterface<Float> > ImageTask::_prepareOutputImage(
    const ImageInterface<Float>& image,
    const Array<Float> *const values,
    const ArrayLattice<Bool> *const mask,
	const IPosition *const outShape,
	const CoordinateSystem *const coordsys
) const {
	IPosition oShape = outShape == 0 ? image.shape() : *outShape;
	CoordinateSystem csys = coordsys == 0 ? image.coordinates() : *coordsys;
	std::tr1::shared_ptr<ImageInterface<Float> > outImage(
		new TempImage<Float>(
			TiledShape(oShape), csys
		)
	);
	std::auto_ptr<ArrayLattice<Bool> > mymask;
	if (mask != 0) {
		mymask.reset(dynamic_cast<ArrayLattice<Bool> *>(mask->clone()));
	}
	else if (image.hasPixelMask()) {
		mymask.reset(new ArrayLattice<Bool>(image.pixelMask().get()));
	}
	if (mymask.get() != 0 && ! allTrue(mymask->get())) {
		dynamic_cast<TempImage<Float> *>(outImage.get())->attachMask(*mymask);
	}
	ImageUtilities::copyMiscellaneous(*outImage, image);
	if (! _getOutname().empty()) {
		_removeExistingOutfileIfNecessary();
		String emptyMask = "";
		Record empty;
		outImage = SubImageFactory<Float>::createImage(
			*outImage, _getOutname(), empty, emptyMask,
			False, False, True, False
		);
	}
	outImage->put(values == 0 ? image.get() : *values);
	return outImage;
}

}

