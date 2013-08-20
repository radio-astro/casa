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

#include <imageanalysis/ImageAnalysis/ImageRegridder.h>

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageRegrid.h>
#include <images/Regions/WCBox.h>

/*
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>

#include <lattices/Lattices/LatticeUtilities.h>
*/
#include <memory>

namespace casa {

const String ImageRegridder::_class = "ImageRegridder";

ImageRegridder::ImageRegridder(
	const ImageTask::shCImFloat image,
	const Record *const regionRec,
	const String& maskInp, const String& outname, Bool overwrite,
	const CoordinateSystem& csysTo, const IPosition& axes,
	const IPosition& shape, Bool dropdeg
) : ImageTask(
		image, "", regionRec, "", "", "",
		maskInp, outname, overwrite
	),
	_csysTo(csysTo), _axes(axes), _shape(shape), _dropdeg(dropdeg),
	_specAsVelocity(False), _doRefChange(True), _replicate(False),
	_forceRegrid(False), _debug(0), _decimate(10),
	_method(Interpolate2D::LINEAR) {
	_construct();
	_finishConstruction();
}

ImageRegridder::ImageRegridder(
	const ImageTask::shCImFloat image, const String& outname,
	const ImageTask::shCImFloat templateIm, const IPosition& axes,
	const Record *const regionRec, const String& maskInp,
	Bool overwrite, Bool dropdeg, const IPosition& shape
)  : ImageTask(
		image, "", regionRec, "", "", "",
		maskInp, outname, overwrite
	),
	_csysTo(templateIm->coordinates()), _axes(axes), _shape(shape),
	_dropdeg(dropdeg), _specAsVelocity(False), _doRefChange(True),
	_replicate(False), _forceRegrid(False), _debug(0), _decimate(10),
	_method(Interpolate2D::LINEAR)
{
	_construct();
	_finishConstruction();
}

ImageRegridder::~ImageRegridder() {}

ImageInterface<Float>* ImageRegridder::regrid(const Bool wantReturn) const {
	Bool regridByVel = False;
	if (
		_specAsVelocity && _getImage()->coordinates().hasSpectralAxis()
		&& _csysTo.hasSpectralAxis()
	) {
		if (_axes.size() == 0) {
			regridByVel = True;
		}
		else {
			Int specAxis = _getImage()->coordinates().spectralAxisNumber();
			for (uInt i=0; i<_axes.size(); i++) {
				if (_axes[i] == specAxis) {
					regridByVel = True;
					break;
				}
			}
		}
	}
	TempImage<Float> workIm;
	if (regridByVel) {
		workIm = _regridByVelocity();
	}
	else {
		workIm = _regrid();
	}
	std::tr1::shared_ptr<ImageInterface<Float> > outImage = _prepareOutputImage(workIm);
	if (wantReturn) {
		return outImage->cloneII();
	}
	else {
		return 0;
	}
}

TempImage<Float> ImageRegridder::_regrid() const {
	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
	SubImage<Float> subImage = SubImageFactory<Float>::createSubImage(
		*clone, *_getRegion(), _getMask(), _getLog().get(),
		False, AxesSpecifier(! _dropdeg), _getStretch()
	);
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	if (! anyTrue(subImage.getMask())) {
		*_getLog() << "All selected pixels are masked" << LogIO::EXCEPTION;
	}
	clone.reset(0);
	const CoordinateSystem csysFrom = subImage.coordinates();
	CoordinateSystem csysTo = _csysTo;
	csysTo.setObsInfo(csysFrom.obsInfo());
	std::set<Coordinate::Type> coordsToRegrid;
	CoordinateSystem csys = ImageRegrid<Float>::makeCoordinateSystem(
		*_getLog(), coordsToRegrid, csysTo, csysFrom, _axes, subImage.shape()
	);
	if (csys.nPixelAxes() != _shape.nelements()) {
		*_getLog()
			<< "The number of pixel axes in the output shape and Coordinate System must be the same"
			<< LogIO::EXCEPTION;
	}
	TempImage<Float> workIm(_shape, csys);
	workIm.set(0.0);
	ImageUtilities::copyMiscellaneous(workIm, subImage);
	String maskName("");
	ImageMaskAttacher<Float>::makeMask(workIm, maskName, True, True, *_getLog(), True);
	try {		// check that there is overlap between input and output
		IPosition shapeIn = subImage.shape();
		Vector<Double> blc, trc;
		Bool ok = csysFrom.toWorld(blc, IPosition(shapeIn.size(), 0));
		if (ok) {
			ok = csysFrom.toWorld(trc, IPosition(shapeIn-1));
		}
		if (ok) {
			Vector<String> units = csysFrom.worldAxisUnits();
			Vector<Quantity> blcQ(blc.size()), trcQ(trc.size());
			Vector<Int> absrel(blc.size(), RegionType::Abs);
			for (uInt i=0; i<shapeIn.size(); i++) {
				blcQ[i] = Quantity(blc[i], units[i]);
				trcQ[i] = Quantity(trc[i], units[i]);
			}
			WCBox box(blcQ, trcQ, csysFrom, absrel);
			SubImage<Float> overlap = SubImageFactory<Float>::createSubImage(
				workIm, box.toRecord(""), "", _getLog().get(),
				False, AxesSpecifier(), False
			);
		}
	}
	catch (const AipsError& x) {
		throw AipsError(
			"There is no overlap between the (region chosen in) the input image"
			" and the output image with respect to the axes being regridded."
		);
	}

	ImageRegrid<Float> ir;
	ir.showDebugInfo(_debug);
	ir.disableReferenceConversions(!_doRefChange);
	ir.regrid(
		workIm, _method, _axes, subImage,
		_replicate, _decimate, True,
		_forceRegrid
	);
	ThrowIf(
		workIm.hasPixelMask() && ! anyTrue(workIm.pixelMask().get()),
		"All output pixels are masked."
	);
	return workIm;
}

TempImage<Float> ImageRegridder::_regridByVelocity() const {
	if (
		_csysTo.spectralCoordinate().frequencySystem(True)
		!= _getImage()->coordinates().spectralCoordinate().frequencySystem(True)
	) {
		*_getLog() << "Image to be regridded has different frequency system from template."
			<< LogIO::EXCEPTION;
	}
	std::auto_ptr<CoordinateSystem> csys(
		dynamic_cast<CoordinateSystem *>(_csysTo.clone())
	);
	SpectralCoordinate templateSpecCoord = csys->spectralCoordinate();
	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
 	std::tr1::shared_ptr<SubImage<Float> > maskedClone(
		new SubImage<Float>(
			SubImageFactory<Float>::createSubImage(
				*clone, *_getRegion(), _getMask(), 0, False,
				AxesSpecifier(), _getStretch()
			)
		)
	);
 	clone.reset(0);
	std::auto_ptr<CoordinateSystem> coordClone(
		dynamic_cast<CoordinateSystem *>(maskedClone->coordinates().clone())
	);

	// SpectralCoordinate templateSpecCoord = csys->spectralCoordinate();
	SpectralCoordinate newSpecCoord = coordClone->spectralCoordinate();

	Double newVelRefVal = 0;
	Double newVelInc = 0;
	for (uInt i=0; i<2; i++) {
		CoordinateSystem *cs = i == 0 ? csys.get() : coordClone.get();
		// create and replace the coordinate system's spectral coordinate with
		// a linear coordinate which describes the velocity axis. In this way
		// we can regrid by velocity.
		Int specCoordNum = cs->spectralCoordinateNumber();
		SpectralCoordinate specCoord = cs->spectralCoordinate();
		if (
			specCoord.frequencySystem(False) != specCoord.frequencySystem(True)
		) {
			// the underlying conversion system is different from the overlying one, so this
			// is pretty confusing. We want the underlying one also be the overlying one before
			// we regrid.
			Vector<Double> newRefVal;
			Double newRefPix = specCoord.referencePixel()[0];
			specCoord.toWorld(newRefVal, Vector<Double>(1, newRefPix));
			Vector<Double> newVal;
			specCoord.toWorld(newVal, Vector<Double>(1, newRefPix+1));

			specCoord = SpectralCoordinate(
				specCoord.frequencySystem(True), newRefVal[0],
				newVal[0] - newRefVal[0], newRefPix, specCoord.restFrequency()
			);
			if (cs == coordClone.get()) {
				newSpecCoord = specCoord;
			}
		}
		Double freqRefVal = specCoord.referenceValue()[0];
		Double velRefVal;
		if (! specCoord.frequencyToVelocity(velRefVal, freqRefVal)) {
			*_getLog() << "Unable to determine reference velocity" << LogIO::EXCEPTION;
		}
		Double vel0, vel1;
		if (
			! specCoord.pixelToVelocity(vel0, 0.0)
			|| ! specCoord.pixelToVelocity(vel1, 1.0)
		) {
			*_getLog() << "Unable to determine velocity increment" << LogIO::EXCEPTION;
		}
		Matrix<Double> pc(1, 1, 0);
		pc.diagonal() = 1.0;
		LinearCoordinate lin(
			Vector<String>(1, "velocity"),
			specCoord.worldAxisUnits(),
			Vector<Double>(1, velRefVal),
			Vector<Double>(1, vel1 - vel0),
			pc, specCoord.referencePixel()
		);
		if (
			! cs->replaceCoordinate(lin, specCoordNum)
			&& ! lin.near(cs->linearCoordinate(specCoordNum))) {
				*_getLog() << "Unable to replace spectral with linear coordinate"
				<< LogIO::EXCEPTION;
		}
		if (cs == csys.get()) {
			newVelRefVal = velRefVal;
			newVelInc = vel1 - vel0;
		}
		else {
			maskedClone->setCoordinateInfo(*cs);
		}
	}
	// do not pass the region or the mask, the maskedClone has already had the region and
	// mask applied
	ImageRegridder regridder(
		maskedClone, 0, "", _getOutname(),
		_getOverwrite(), *csys, _axes, _shape, _dropdeg
	);
	regridder.setMethod(_method);
	regridder.setDecimate(_decimate);
	regridder.setReplicate(_replicate);
	regridder.setDoRefChange(_doRefChange);
	regridder.setForceRegrid(_forceRegrid);
	regridder.setStretch(_getStretch());
	regridder.setSpecAsVelocity(False);

	TempImage<Float> outImage = regridder._regrid();

	// replace the temporary linear coordinate with the saved spectral coordinate
	std::auto_ptr<CoordinateSystem> newCoords(
		dynamic_cast<CoordinateSystem *>(outImage.coordinates().clone())
	);

	// make frequencies correct
	Double newRefFreq;
	if (
		! newSpecCoord.velocityToFrequency(
			newRefFreq, newVelRefVal
		)
	) {
		*_getLog() << "Unable to determine new reference frequency"
			<< LogIO::EXCEPTION;
	}
	// get the new frequency increment
	Double newFreq;
	if (
		! newSpecCoord.velocityToFrequency(
			newFreq, newVelRefVal + newVelInc
		)
	) {
		*_getLog() << "Unable to determine new frequency increment" << LogIO::EXCEPTION;
	}
	if (! newSpecCoord.setReferenceValue(Vector<Double>(1, newRefFreq))) {
		*_getLog() << "Unable to set new reference frequency" << LogIO::EXCEPTION;
	}
	if (! newSpecCoord.setIncrement((Vector<Double>(1, newFreq - newRefFreq)))) {
		*_getLog() << "Unable to set new frequency increment" << LogIO::EXCEPTION;
	}
	if (
		! newSpecCoord.setReferencePixel(
			templateSpecCoord.referencePixel()
		)
	) {
		*_getLog() << "Unable to set new reference pixel" << LogIO::EXCEPTION;
	}
	if (
		! newCoords->replaceCoordinate(
			newSpecCoord,
			maskedClone->coordinates().linearCoordinateNumber())
		&& ! newSpecCoord.near(newCoords->spectralCoordinate())
	) {
		*_getLog() << "Unable to replace coordinate for velocity regridding"
			<< LogIO::EXCEPTION;
	}
	outImage.setCoordinateInfo(*newCoords);
	return outImage;
}

void ImageRegridder::_finishConstruction() {
	if (_shape.empty() || (_shape.size() == 1 && _shape[0] <= 0)) {
		IPosition imShape = _getImage()->shape();
		_shape.resize(imShape.size());
		_shape = imShape;
		if (_dropdeg) {
			IPosition tmp = _shape.nonDegenerate();
			_shape.resize(tmp.size());
			_shape = tmp;
		}
	}
}

}


