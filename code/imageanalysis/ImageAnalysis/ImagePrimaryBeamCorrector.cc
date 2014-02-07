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
#include <images/Images/ExtendImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/TempImage.h>
#include <images/Regions/WCBox.h>
#include <lattices/Lattices/LatticeStatistics.h>
#include <lattices/Lattices/LCLELMask.h>

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

#include <unistd.h>


namespace casa {

const String ImagePrimaryBeamCorrector::_class = "ImagePrimaryBeamCorrector";
uInt _tempTableNumber = 0;

ImagePrimaryBeamCorrector::ImagePrimaryBeamCorrector(
		const SPCIIF image,
		const SPCIIF pbImage,
	const Record *const &regionPtr,
	const String& region, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite, const Float cutoff,
	const Bool useCutoff,
	const ImagePrimaryBeamCorrector::Mode mode
) : ImageTask<Float>(
		image, region, regionPtr, box, chanInp, stokes, maskInp, outname, overwrite
	), _pbImage(pbImage->cloneII()), _cutoff(cutoff),
	_mode(mode), _useCutoff(useCutoff) {
	_checkPBSanity();
	_construct();
}

ImagePrimaryBeamCorrector::ImagePrimaryBeamCorrector(
		const SPCIIF image,
	const Array<Float>& pbArray,
	const Record *const &regionPtr,
	const String& region, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite, const Float cutoff,
	const Bool useCutoff,
	const ImagePrimaryBeamCorrector::Mode mode
) : ImageTask<Float>(
		image, region, regionPtr, box, chanInp, stokes, maskInp, outname, overwrite
	), _cutoff(cutoff), _mode(mode), _useCutoff(useCutoff) {
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	IPosition imShape = _getImage()->shape();
	if (pbArray.shape().isEqual(imShape)) {
		_pbImage.reset(new TempImage<Float>(imShape, _getImage()->coordinates()));
	}
	else if (pbArray.ndim() == 2) {
		if (_getImage()->coordinates().hasDirectionCoordinate()) {
			Vector<Int> dirAxes = _getImage()->coordinates().directionAxesNumbers();
			if (
				pbArray.shape()[0] != imShape[dirAxes[0]]
				|| pbArray.shape()[1] != imShape[dirAxes[1]]
			) {
				*_getLog() << "Array shape does not equal image direction plane shape" << LogIO::EXCEPTION;
			}
			IPosition boxShape(imShape.size(), 1);
			boxShape[dirAxes[0]] = imShape[dirAxes[0]];
			boxShape[dirAxes[1]] = imShape[dirAxes[1]];
			LCBox x(IPosition(imShape.size(), 0), boxShape - 1, imShape);
			std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
			SubImage<Float> sub = SubImageFactory<Float>::createSubImage(
				*clone, x.toRecord(""), "",
				_getLog().get(), True, AxesSpecifier(False)
			);
			_pbImage.reset(new TempImage<Float>(sub.shape(), sub.coordinates()));
		}
		else {
			*_getLog() << "Image " << _getImage()->name()
				<< " does not have direction coordinate" << LogIO::EXCEPTION;
		}
	}
	else {
		*_getLog() << "Primary beam array is of wrong shape (" << pbArray.shape() << ")" << LogIO::EXCEPTION;
	}
	_pbImage->put(pbArray);
	_construct();
}

ImagePrimaryBeamCorrector::~ImagePrimaryBeamCorrector() {}

String ImagePrimaryBeamCorrector::getClass() const {
	return _class;
}

void ImagePrimaryBeamCorrector::_checkPBSanity() {
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	if (_getImage()->shape().isEqual(_pbImage->shape())) {
		const CoordinateSystem csys = _getImage()->coordinates();
		if (
			! csys.directionCoordinate().near(
				_pbImage->coordinates().directionCoordinate()
			)
		) {
			*_getLog() << "Coordinate systems of image and template are different: "
				<< csys.errorMessage() << LogIO::EXCEPTION;
		}
		else {
			// direction coordinates and image shapes are the same
			// which is sufficient to proceed
			// but set the coordinate system just in case the non-directional coordinates
			// differ (CAS-5096)
			_pbImage->setCoordinateInfo(csys);
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
			! _getImage()->coordinates().directionCoordinate().near(
				pbCopy->coordinates().directionCoordinate()
			)
		) {
			*_getLog() << "Direction coordinates of input image and primary beam "
				<< "image are different. Cannot do primary beam correction."
				<< LogIO::EXCEPTION;
		}
	}
	else {
		*_getLog() << "Input image and primary beam image have different shapes. "
			<< "Cannot do primary beam correction."
			<< LogIO::EXCEPTION;
	}
	_pbImage.reset(pbCopy);
	LatticeStatistics<Float> ls(*_pbImage);
	Float myMin, myMax;
	ls.getFullMinMax(myMin, myMax);
	if (_mode == DIVIDE && myMax > 1.0 && ! near(myMax, 1.0)) {
		*_getLog() << LogIO::WARN
			<< "Mode DIVIDE chosen but primary beam has one or more pixels "
			<< "greater than 1.0. Proceeding but you may want to check your inputs"
			<< LogIO::POST;
	}
	if (myMin < 0 && near(myMin, 0.0)) {
		*_getLog() << LogIO::WARN
			<< "Primary beam has one or more pixels less than 0."
			<< "Proceeding but you may want to check your inputs"
			<< LogIO::POST;
	}
}

vector<Coordinate::Type> ImagePrimaryBeamCorrector::_getNecessaryCoordinates() const {
	return vector<Coordinate::Type>(1, Coordinate::DIRECTION);
}

CasacRegionManager::StokesControl ImagePrimaryBeamCorrector::_getStokesControl() const {
	return CasacRegionManager::USE_ALL_STOKES;
}

ImageInterface<Float>* ImagePrimaryBeamCorrector::correct(
	const Bool wantReturn
) const {
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
    SubImage<Float> pbSubImage;
    std::auto_ptr<ImageInterface<Float> > tmpStore(0);
    ImageInterface<Float> *pbTemplate = _pbImage.get();
	if (! _getImage()->shape().isEqual(_pbImage->shape())) {
		pbTemplate = new ExtendImage<Float>(
			*_pbImage, _getImage()->shape(),
			 _getImage()->coordinates()
		);
		tmpStore.reset(pbTemplate);
	}
    SubImage<Float> subImage;
	if (_useCutoff) {
		LatticeExpr<Bool> mask = (_mode == DIVIDE)
			? *pbTemplate >= _cutoff
			: *pbTemplate <= _cutoff;
		if (pbTemplate->hasPixelMask()) {
			mask = mask && pbTemplate->pixelMask();
		}
		subImage = SubImage<Float>(*_getImage(), LattRegionHolder(LCLELMask(mask)));
		subImage = SubImageFactory<Float>::createSubImage(
		    subImage, *_getRegion(), _getMask(), _getLog().get(), False,
		    AxesSpecifier(), _getStretch()
		);
	}
	else {
		std::auto_ptr<ImageInterface<Float> > tmp(
			pbTemplate->hasPixelMask()
			? new SubImage<Float>(
				*_getImage(),
				LattRegionHolder(
					LCLELMask(LatticeExpr<Bool>(pbTemplate->pixelMask()))
				)
			)
			: _getImage()->cloneII());
		subImage = SubImageFactory<Float>::createSubImage(
		    *tmp, *_getRegion(), _getMask(), _getLog().get(),
		    False, AxesSpecifier(), _getStretch()
		);
	}
	pbSubImage = SubImageFactory<Float>::createSubImage(
    	*pbTemplate, *_getRegion(), _getMask(), _getLog().get(),
    	False, AxesSpecifier(), _getStretch()
    );
	tmpStore.reset(0);
	std::tr1::shared_ptr<ImageInterface<Float> > outImage;
	if (_getOutname().empty()) {
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
		_removeExistingOutfileIfNecessary();
		String mask = "";
	    Record empty;
		outImage.reset( SubImageFactory<Float>::createImage(
			subImage, _getOutname(), empty,
			mask, False, False, False, False
		));
	}
	LatticeExpr<Float> expr = (_mode == DIVIDE)
		? subImage/pbSubImage
		: subImage*pbSubImage;
	outImage->copyData(expr);

    if (wantReturn) {
    	return outImage->cloneII();
    }
    else {
    	return 0;
    }
}
}

