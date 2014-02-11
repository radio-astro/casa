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

template <class T> ImageTask<T>::ImageTask(
	const SPCIIT image,
	const String& region, const Record *const &regionPtr,
	const String& box, const String& chanInp,
	const String& stokes, const String& maskInp,
    const String& outname, const Bool overwrite
) : _image(image), _log(new LogIO()), _regionPtr(regionPtr),
	_region(region), _box(box),
	_chan(chanInp), _stokesString(stokes), _mask(maskInp),
	_outname(outname), _overwrite(overwrite), _stretch(False),
	_logfileSupport(False), _logfileAppend(False),
	_verbosity(NORMAL), _logfile() {
    FITSImage::registerOpenFunction();
    MIRIADImage::registerOpenFunction();
}

template <class T> ImageTask<T>::~ImageTask() {}

template <class T> std::vector<OutputDestinationChecker::OutputStruct> ImageTask<T>::_getOutputStruct() {
	std::vector<OutputDestinationChecker::OutputStruct> outputs;
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

template <class T> void ImageTask<T>::_construct(Bool verbose) {

	ThrowIf(
		! _supportsMultipleBeams() && _image->imageInfo().hasMultipleBeams(),
		"This application does not support images with multiple "
		"beams. Please convolve your image with a single beam "
		"and run this application using that image"
	);
	String diagnostics;
	std::vector<OutputDestinationChecker::OutputStruct> outputs = _getOutputStruct();
	std::vector<OutputDestinationChecker::OutputStruct> *outputPtr = outputs.size() > 0
		? &outputs
		: 0;
	std::vector<Coordinate::Type> necCoords = _getNecessaryCoordinates();
	std::vector<Coordinate::Type> *coordsPtr = necCoords.size() > 0
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
    	_stokesString, _image, _regionPtr,
    	_region, _box, _chan,
    	_getStokesControl(), _supportsMultipleRegions(),
    	coordsPtr, verbose
    );
}

template <class T> void ImageTask<T>::setRegion(const Record& region) {
	ThrowIf(
		! _supportsMultipleRegions() && region.isDefined("regions"),
		"This application does not support multiple region selection"
	);
    _regionRecord = region;
    _box = "";
    _chan = "";
    _stokesString = "";
    _region = "";
}

template <class T> void ImageTask<T>::_removeExistingFileIfNecessary(
	const String& filename, Bool overwrite
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

template <class T> void ImageTask<T>::_removeExistingOutfileIfNecessary() const {
	_removeExistingFileIfNecessary(_outname, _overwrite);
}

template <class T> String ImageTask<T>::_summaryHeader() const {
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

template <class T> void ImageTask<T>::setLogfile(const String& lf) {
	ThrowIf(
		! _logfileSupport,
		"Logic Error: This task does not support writing of a log file"
	);
	try {
		_logfile.reset(new LogFile(lf));
		_logfile->setAppend(_logfileAppend);
	}
	catch (const AipsError& x) {}
}

template <class T> const std::tr1::shared_ptr<LogFile> ImageTask<T>::_getLogFile() const {
	ThrowIf(
		! _logfileSupport,
		"Logic Error: This task does not support writing of a log file"
	);
	return _logfile;
}

template <class T> Bool ImageTask<T>::_openLogfile() {
	if (_logfile.get() == 0) {
		return False;
	}
	*_log << LogOrigin(getClass(), __FUNCTION__);
	ThrowIf(
		! _logfileSupport,
		"Logic Error: This task does not support writing of a log file"
	);
	return _logfile->openFile();
}

template <class T> void ImageTask<T>::_closeLogfile() const {
	if (_logfile) {
		_logfile->closeFile();
	}
}

template<class T> Bool ImageTask<T>::_writeLogfile(
	const String& output, const Bool open, const Bool close
) {
	ThrowIf(
		! _logfileSupport,
		"Logic Error: This task does not support writing of a log file"
	);
	if (! _logfile) {
		return False;
	}
	return _logfile->write(output, open, close);
}

template <class T> void ImageTask<T>::setLogfileAppend(Bool a) {
	ThrowIf(
		! _logfileSupport,
		"Logic Error: This task does not support writing of a log file"
	);
	_logfileAppend = a;
	if (_logfile) {
		_logfile->setAppend(a);
	}
}

template <class T> ImageInterface<T>*  ImageTask<T>::_prepareOutputImage(
    const ImageInterface<T>& image, const Array<T> *const values,
    const ArrayLattice<Bool> *const mask,
    const IPosition *const outShape, const CoordinateSystem *const coordsys
) const {
	IPosition oShape = outShape == 0 ? image.shape() : *outShape;
	CoordinateSystem csys = coordsys == 0 ? image.coordinates() : *coordsys;
    std::auto_ptr<ImageInterface<T> > outImage(
		new TempImage<T>(
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
		dynamic_cast<TempImage<T> *>(outImage.get())->attachMask(*mymask);
	}
	ImageUtilities::copyMiscellaneous(*outImage, image);
	if (! _getOutname().empty()) {
		_removeExistingOutfileIfNecessary();
		String emptyMask = "";
		Record empty;
        PagedImage<T> *tmp = dynamic_cast<PagedImage<T> *>(SubImageFactory<T>::createImage(
                    *outImage, _getOutname(), empty, emptyMask,
                    False, False, True, False));
        ThrowIf(tmp == 0, "Failed dynamic cast")
		outImage.reset(tmp);
	}
	outImage->put(values == 0 ? image.get() : *values);
	return outImage.release();
}

}

