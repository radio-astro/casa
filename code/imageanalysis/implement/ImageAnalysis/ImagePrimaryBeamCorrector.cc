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

#include <imageanalysis/ImageAnalysis/ImagePrimaryBeamCorrector.h>

#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>
#include <images/Regions/WCBox.h>
#include <lattices/Lattices/LCLELMask.h>

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

#include <memory>
#include <unistd.h>


namespace casa {

const String ImagePrimaryBeamCorrector::_class = "ImagePrimaryBeamCorrector";
uInt _tempTableNumber = 0;

ImagePrimaryBeamCorrector::ImagePrimaryBeamCorrector(
	const ImageInterface<Float> *const &image,
	const ImageInterface<Float> *const &pbImage,
	const Record *const &regionPtr,
	const String& region, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite, const Float cutoff,
	const Bool useCutoff,
	const ImagePrimaryBeamCorrector::Mode mode
) : _log(new LogIO()), _image(image), _pbImage(pbImage->cloneII()),
	_chan(chanInp), _stokesString(stokes), _mask(maskInp),
	_outname(outname), _overwrite(overwrite), _cutoff(cutoff),
	_mode(mode), _useCutoff(useCutoff) {
	_checkPBSanity();
	_construct(box, region, regionPtr);
}

ImagePrimaryBeamCorrector::ImagePrimaryBeamCorrector(
	const ImageInterface<Float> *const &image,
	const Array<Float>& pbArray,
	const Record *const &regionPtr,
	const String& region, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite, const Float cutoff,
	const Bool useCutoff,
	const ImagePrimaryBeamCorrector::Mode mode
) : _log(new LogIO()), _image(image), _pbImage(0),
	_chan(chanInp), _stokesString(stokes), _mask(maskInp),
	_outname(outname), _overwrite(overwrite), _cutoff(cutoff),
	_mode(mode), _useCutoff(useCutoff) {
	if (pbArray.shape().isEqual(_image->shape())) {
		_pbImage.reset(new TempImage<Float>(_image->shape(), _image->coordinates()));
	}
	else if (pbArray.ndim() == 2) {
		if (_image->coordinates().hasDirectionCoordinate()) {
			Vector<Int> dirAxes = _image->coordinates().directionAxesNumbers();
			if (
				pbArray.shape()[0] !=_image->shape()[dirAxes[0]]
				|| pbArray.shape()[1] !=_image->shape()[dirAxes[1]]
			) {
				*_log << "Array shape does not equal image direction plane shape" << LogIO::EXCEPTION;
			}
			IPosition boxShape(_image->ndim(), 1);
			boxShape[dirAxes[0]] = _image->shape()[dirAxes[0]];
			boxShape[dirAxes[1]] = _image->shape()[dirAxes[1]];
			LCBox x(IPosition(_image->ndim(), 0), boxShape - 1, _image->shape());
			std::auto_ptr<ImageInterface<Float> > clone(_image->cloneII());
			SubImage<Float> sub = SubImage<Float>::createSubImage(
				*clone, x.toRecord(""), "",
				_log.get(), True, AxesSpecifier(False)
			);
			_pbImage.reset(new TempImage<Float>(sub.shape(), sub.coordinates()));
		}
		else {
			*_log << "Image " << _image->name()
				<< " does not have direction coordinate" << LogIO::EXCEPTION;
		}
	}
	else {
		*_log << "Primary beam array is of wrong shape" << LogIO::EXCEPTION;
	}
	_pbImage->put(pbArray);
	_construct(box, region, regionPtr);
}

ImagePrimaryBeamCorrector::~ImagePrimaryBeamCorrector() {}

void ImagePrimaryBeamCorrector::_checkPBSanity() {
	*_log << LogOrigin(_class, __FUNCTION__);
	if (_image->shape().isEqual(_pbImage->shape())) {
		if (! _image->coordinates().near(_pbImage->coordinates())) {
			*_log << "Coordinate systems of image and template are different"
				<< LogIO::EXCEPTION;
		}
		else {
			// coordinate systems and shapes are the same
			// which is sufficient to proceed
			return;
		}
	}
	Vector<Int> dcn = _pbImage->coordinates().pixelAxes(
		_pbImage->coordinates().directionCoordinateNumber()
	);
	ImageInterface<Float> *pbCopy(
		(_pbImage->ndim() == 2)
		? _pbImage.get()
		: new SubImage<Float>(
			*_pbImage,
			AxesSpecifier(IPosition(dcn))
		)
	);
	if (pbCopy->ndim() == 2) {
		if (
			! _image->coordinates().directionCoordinate().near(
				pbCopy->coordinates().directionCoordinate()
			)
		) {
			*_log << "Direction coordinates of input image and primary beam "
				<< "image are different. Cannot do primary beam correction."
				<< LogIO::EXCEPTION;
		}
	}
	else {
		*_log << "Input image and primary beam image have different shapes. "
			<< "Cannot do primary beam correction."
			<< LogIO::EXCEPTION;
	}
	_pbImage.reset(pbCopy);
	LatticeStatistics<Float> ls(*_pbImage);
	Float myMin, myMax;
	ls.getFullMinMax(myMin, myMax);
	if (_mode == DIVIDE && myMax > 1.0 && ! near(myMax, 1.0)) {
		*_log << LogIO::WARN
			<< "Mode DIVIDE chosen but primary beam has one or more pixels "
			<< "greater than 1.0. Proceeding but you may want to check your inputs"
			<< LogIO::POST;
	}
	if (myMin < 0 && near(myMin, 0.0)) {
		*_log << LogIO::WARN
			<< "Primary beam has one or more pixels less than 0."
			<< "Proceeding but you may want to check your inputs"
			<< LogIO::POST;
	}
}

vector<ImageInputProcessor::OutputStruct> ImagePrimaryBeamCorrector::_getOutputStruct() {
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

void ImagePrimaryBeamCorrector::_construct(
		const String& box, const String& region, const Record * const regionPtr
) {
	String diagnostics;
	vector<ImageInputProcessor::OutputStruct> outputs = _getOutputStruct();
	vector<ImageInputProcessor::OutputStruct> *outputPtr = outputs.size() > 0
		? &outputs
		: 0;
	vector<Coordinate::Type> direction(1, Coordinate::DIRECTION);
	ImageInputProcessor inputProcessor;
	inputProcessor.process(
		_regionRecord, diagnostics, outputPtr,
    	_stokesString, _image, regionPtr,
    	region, box, _chan,
    	CasacRegionManager::USE_ALL_STOKES, False,
    	&direction
    );
}

ImageInterface<Float>* ImagePrimaryBeamCorrector::correct(
	const Bool wantReturn
) const {
	*_log << LogOrigin(_class, __FUNCTION__);
    SubImage<Float> pbSubImage;
    std::auto_ptr<ImageInterface<Float> > tmpStore(0);
    ImageInterface<Float> *pbTemplate = _pbImage.get();
	if (! _image->shape().isEqual(_pbImage->shape())) {
		Vector<Int> dcn = _image->coordinates().pixelAxes(
			_image->coordinates().directionCoordinateNumber()
		);
		IPosition extendAxes(_image->ndim()-2);
		uInt j = 0;
		for (Int i=0; i<(Int)_image->ndim(); i++) {
			if (i != dcn[0] && i != dcn[1]) {
				extendAxes[j] = i;
				j++;
			}
		}
		ExtendLattice<Float> pbLattice(
			*_pbImage, _image->shape(),
			 extendAxes, IPosition(0)
		);
		pbTemplate = new TempImage<Float>(
			TiledShape(_image->shape()), _image->coordinates()
		);
		tmpStore.reset(pbTemplate);
		pbTemplate->copyData(pbLattice);
		if (pbLattice.hasPixelMask()) {
			dynamic_cast<TempImage<Float> *>(pbTemplate)->attachMask(pbLattice.pixelMask());
		}
	}
    SubImage<Float> subImage;
	if (_useCutoff) {
		LatticeExpr<Bool> mask = (_mode == DIVIDE)
			? *pbTemplate >= _cutoff
			: *pbTemplate <= _cutoff;
		if (pbTemplate->hasPixelMask()) {
			mask = mask && pbTemplate->pixelMask();
		}
		subImage = SubImage<Float>(*_image, LattRegionHolder(LCLELMask(mask)));
		subImage = SubImage<Float>::createSubImage(
		    subImage, _regionRecord, _mask, _log.get(), False
		);
	}
	else {
		std::auto_ptr<ImageInterface<Float> > tmp(
			pbTemplate->hasPixelMask()
			? new SubImage<Float>(
				*_image,
				LattRegionHolder(
					LCLELMask(LatticeExpr<Bool>(pbTemplate->pixelMask()))
				)
			)
			: _image->cloneII());
		subImage = SubImage<Float>::createSubImage(
		    *tmp, _regionRecord, _mask, _log.get(), False
		);
	}
	pbSubImage = SubImage<Float>::createSubImage(
    	*pbTemplate, _regionRecord, _mask, _log.get(), False
    );
	tmpStore.reset(0);
	std::auto_ptr<ImageInterface<Float> > outImage(0);
	if (_outname.empty()) {
		outImage.reset(
			new TempImage<Float>(
				TiledShape(subImage.shape()), subImage.coordinates()
			)
		);
		if (subImage.hasPixelMask()) {
			dynamic_cast<TempImage<Float> *>(outImage.get())->attachMask(subImage.pixelMask());
		}
		if (pbSubImage.hasPixelMask()) {
			LatticeExpr<Bool> mask(pbSubImage.pixelMask());
			if (outImage->hasPixelMask()) {
				mask = mask && outImage->pixelMask();
			}
			dynamic_cast<TempImage<Float> *>(outImage.get())->attachMask(
				mask
			);
		}
		ImageUtilities::copyMiscellaneous(*outImage, subImage);

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
				*_log << "File " << _outname
					<< " exists but overwrite is false so it cannot be overwritten"
					<< LogIO::EXCEPTION;
			}
		}
		ImageAnalysis ia(&subImage);
		String mask = "";
	    Record empty;
		outImage.reset(
			ia.subimage(_outname, empty, mask, False, False)
		);
	}
	LatticeExpr<Float> expr = (_mode == DIVIDE)
		? subImage/pbSubImage
		: subImage*pbSubImage;
	outImage->copyData(expr);
    if (wantReturn) {
    	ImageInterface<Float> *ret = outImage.get();
    	// release the pointer for return without deleting its object
    	outImage.release();
    	return ret;
    }
    else {
    	return 0;
    }
}
}

