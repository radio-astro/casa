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

#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageMetaData.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/ImageRegrid.h>
#include <scimath/Mathematics/Geometry.h>

#include <casa/BasicSL/STLIO.h>
#include <memory>
#include <stdcasa/cboost_foreach.h>

namespace casa {

const String  ImageRegridder::_class = "ImageRegridder";

ImageRegridder::ImageRegridder(
	const SPCIIF image,
	const Record *const regionRec,
	const String& maskInp, const String& outname, Bool overwrite,
	const CoordinateSystem& csysTo, const IPosition& axes,
	const IPosition& shape
) : ImageRegridderBase<Float>(
		image, regionRec, maskInp, outname,
		overwrite, csysTo, axes, shape
	), _debug(0) {}

ImageRegridder::ImageRegridder(
	const SPCIIF image, const String& outname,
	const SPCIIF templateIm, const IPosition& axes,
	const Record *const regionRec, const String& maskInp,
	Bool overwrite, const IPosition& shape
)  : ImageRegridderBase<Float>(
		image, regionRec, maskInp, outname, overwrite,
		templateIm->coordinates(), axes, shape
	),
	_debug(0) {}

ImageRegridder::~ImageRegridder() {}

SPIIF ImageRegridder::regrid() const {
	_subimage = SubImageFactory<Float>::createImage(
		*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
		this->_getDropDegen(), False, False, this->_getStretch()
	);
	auto regridByVel = False;
	const auto axes = _getAxes();
	auto hasMultipleBeams = _getImage()->imageInfo().hasMultipleBeams();
	if (
		(_getSpecAsVelocity() || hasMultipleBeams)
		&& _getImage()->coordinates().hasSpectralAxis()
		&& _getTemplateCoords().hasSpectralAxis()
	) {
		auto inputSpecAxis = _getImage()->coordinates().spectralAxisNumber(False);
		auto isInputSpecDegen = _subimage->shape()[inputSpecAxis] == 1;
		if (axes.empty()) {
			ThrowIf(
				hasMultipleBeams,
				"An image with multiple beams cannot be regridded along the spectral axis. "
				"You may wish to convolve all channels to a common resolution and retry"
			);
			if (! isInputSpecDegen && _getSpecAsVelocity()) {
				regridByVel = True;
			}
		}
		else {
			auto specAxis = this->_getImage()->coordinates().spectralAxisNumber();
			for (uInt i=0; i<axes.size(); i++) {
				if (axes[i] == specAxis) {
					ThrowIf(
						hasMultipleBeams,
						"An image with multiple beams cannot be regridded along the spectral axis. "
						"You may wish to convolve all channels to a common resolution and retry"
					);
					if (! isInputSpecDegen && _getSpecAsVelocity()) {
						regridByVel = True;
					}
					break;
				}
			}
		}
	}
	SPIIF workIm = regridByVel
		? _regridByVelocity()
		: _regrid();
	return this->_prepareOutputImage(*workIm);
}

SPIIF ImageRegridder::_regrid() const {
	/*
	SPIIF subImage = SubImageFactory<Float>::createImage(
		*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
		this->_getDropDegen(), False, False, this->_getStretch()
	);
	*/
	if (! _subimage) {
		// for when this method is called directly by regridByVelocity
		_subimage = SubImageFactory<Float>::createImage(
			*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
			this->_getDropDegen(), False, False, this->_getStretch()
		);
	}
	*this->_getLog() << LogOrigin(_class, __func__);
	ThrowIf(
		! anyTrue(_subimage->getMask()),
		"All selected pixels are masked"
	);
	const CoordinateSystem csysFrom = _subimage->coordinates();
	CoordinateSystem csysTo = _getTemplateCoords();
	csysTo.setObsInfo(csysFrom.obsInfo());
	std::set<Coordinate::Type> coordsToRegrid;
	CoordinateSystem csys = ImageRegrid<Float>::makeCoordinateSystem(
		*this->_getLog(), coordsToRegrid, csysTo, csysFrom, _getAxes(),
		_subimage->shape(), False
	);
	ThrowIf(
		csys.nPixelAxes() != _getShape().nelements(),
		"The number of pixel axes in the output shape and Coordinate System must be the same"
	);
	_checkOutputShape(*_subimage, coordsToRegrid);
	SPIIF workIm(new TempImage<Float>(_getKludgedShape(), csys));
	workIm->set(0.0);
	ImageUtilities::copyMiscellaneous(*workIm, *_subimage);
	String maskName("");
	ImageMaskAttacher::makeMask(*workIm, maskName, True, True, *this->_getLog(), True);
	ThrowIf (
		! _doImagesOverlap(_subimage, workIm),
		"There is no overlap between the (region chosen in) the input image"
		" and the output image with respect to the axes being regridded."
	);
	ImageRegrid<Float> ir;
	ir.showDebugInfo(_debug);
	ir.disableReferenceConversions(! _getDoRefChange());
	ir.regrid(
		*workIm, _getMethod(), _getAxes(), *_subimage,
		_getReplicate(), _getDecimate(), True,
		_getForceRegrid()
	);
	if (! _getOutputStokes().empty()) {
		workIm = _decimateStokes(workIm);
	}
	ThrowIf(
		workIm->hasPixelMask() && ! anyTrue(workIm->pixelMask().get()),
		"All output pixels are masked"
		+ String(
			_getDecimate() > 1 && _regriddingDirectionAxes()
			? ". You might want to try decreasing the value of decimate if you are regridding direction axes"
			: ""
		)
	);
	if (_getNReplicatedChans() > 1) {
		// spectral channel needs to be replicated _nReplicatedChans times,
		// and spectral coordinate of the template needs to be copied to the
		// output.
		IPosition finalShape = _getKludgedShape();
		Int specAxisNumber = workIm->coordinates().spectralAxisNumber(False);
		finalShape[specAxisNumber] = _getNReplicatedChans();
		SPIIF replicatedIm(new TempImage<Float>(finalShape, csys));
		Array<Float> fillerPixels = workIm->get();
		Array<Bool> fillerMask = workIm->pixelMask().get();
		Array<Float> finalPixels = replicatedIm->get();
		Array<Bool> finalMask(replicatedIm->shape());
		IPosition begin(finalPixels.ndim(), 0);
		IPosition end = finalPixels.shape();
		for (uInt i=0; i<_getNReplicatedChans(); i++) {
			begin[specAxisNumber] = i;
			end[specAxisNumber] = 0;
			Slicer slice(begin, end);
			finalPixels(slice) = fillerPixels;
			finalMask(slice) = fillerMask;
		}
		replicatedIm->put(finalPixels);
		DYNAMIC_POINTER_CAST<TempImage<Float> >(replicatedIm)->attachMask(
			ArrayLattice<Bool>(finalMask)
		);
		SpectralCoordinate spTo = _getTemplateCoords().spectralCoordinate();
		CoordinateSystem csysFinal = replicatedIm->coordinates();
		csysFinal.replaceCoordinate(spTo, csysFinal.spectralCoordinateNumber());
		replicatedIm->setCoordinateInfo(csysFinal);
		workIm = replicatedIm;
	}
	return workIm;
}

SPIIF ImageRegridder::_decimateStokes(SPIIF workIm) const {
	ImageMetaData md(workIm);
	if (_getOutputStokes().size() >= md.nStokes()) {
		return workIm;
	}
	CasacRegionManager rm(workIm->coordinates());
	String diagnostics;
	uInt nSelectedChannels = 0;
	if (_getOutputStokes().size() == 1) {
		String stokes = _getOutputStokes()[0];
		Record region = rm.fromBCS(
			diagnostics, nSelectedChannels, stokes,
			"", CasacRegionManager::USE_FIRST_STOKES,
			"", workIm->shape()
		).toRecord("");
		return SubImageFactory<Float>::createImage(
			*workIm, "", region, "", False, False, False, False
		);
	}
	else {
		// Only include the wanted stokes
		SHARED_PTR<ImageConcat<Float> > concat(
			new ImageConcat<Float>(
				workIm->coordinates().polarizationAxisNumber(False)
			)
		);
		foreach_(String stokes, _getOutputStokes()) {
			Record region = rm.fromBCS(
				diagnostics, nSelectedChannels, stokes,
				"", CasacRegionManager::USE_FIRST_STOKES,
				"", workIm->shape()
			).toRecord("");
			concat->setImage(
				*SubImageFactory<Float>::createImage(
					*workIm, "", region, "", False, False, False, False
				), True
			);
		}
		return concat;
	}
}

void ImageRegridder::_checkOutputShape(
	const SubImage<Float>& subImage,
	const std::set<Coordinate::Type>& coordsToRegrid
) const {
	const auto& csysFrom = subImage.coordinates();
	std::set<Coordinate::Type> coordsNotToRegrid;
	auto nCoordinates = csysFrom.nCoordinates();
	auto inputShape = subImage.shape();
	auto axes = _getAxes();
	auto outputAxisOrder = axes;
	for (uInt i=axes.size(); i<_getKludgedShape().size(); i++) {
		outputAxisOrder.append(IPosition(1, i));
	}
	std::set<Coordinate::Type>::const_iterator coordsToRegridEnd = coordsToRegrid.end();
	for (uInt i=0; i<nCoordinates; i++) {
		const Coordinate::Type coordType = csysFrom.coordinate(i).type();
		if (coordsToRegrid.find(coordType) == coordsToRegridEnd) {
			Vector<Int> coordAxes = csysFrom.worldAxes(i);
			foreach_(uInt oldAxis, coordAxes) {
				uInt count = 0;
				foreach_(uInt newAxis, outputAxisOrder) {
					if (
						newAxis == oldAxis
						&& inputShape[oldAxis] != _getKludgedShape()[count]
				    ) {
						*this->_getLog() << "Input axis " << oldAxis << " (coordinate type "
							<< Coordinate::typeToString(coordType) << "), which "
						    << "will not be regridded and corresponds to output axis "
							<< newAxis << ", has length " << inputShape[oldAxis] << " where as "
							<< " the specified length of the corresponding output axis is "
							<< _getKludgedShape()[count] << ". If a coordinate is not regridded, "
							<< "its input and output axes must have the same length. " << LogIO::EXCEPTION;
					}
					count++;
				}
			}
		}
	}
}

SPIIF ImageRegridder::_regridByVelocity() const {
	const CoordinateSystem csysTo = _getTemplateCoords();
	ThrowIf(
		csysTo.spectralCoordinate().frequencySystem(True)
		!= this->_getImage()->coordinates().spectralCoordinate().frequencySystem(True),
		"Image to be regridded has different frequency system from template coordinate system."
	);
	ThrowIf(
		csysTo.spectralCoordinate().restFrequency() == 0,
		"Template spectral coordinate rest frequency is 0, "
		"so cannot regrid by velocity."
	);
	ThrowIf(
		this->_getImage()->coordinates().spectralCoordinate().restFrequency() == 0,
		"Input image spectral coordinate rest frequency is 0, "
		"so cannot regrid by velocity."
	);

	std::unique_ptr<CoordinateSystem> csys(
		dynamic_cast<CoordinateSystem *>(csysTo.clone())
	);
	SpectralCoordinate templateSpecCoord = csys->spectralCoordinate();
	/*
 	SPIIF maskedClone = SubImageFactory<Float>::createImage(
 		*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
 		False, False, False, this->_getStretch()
 	);
 	*/
	std::unique_ptr<CoordinateSystem> coordClone(
		dynamic_cast<CoordinateSystem *>(_subimage->coordinates().clone())
	);

	SpectralCoordinate newSpecCoord = coordClone->spectralCoordinate();
	Double newVelRefVal = 0;
	Double newVelInc = 0;
	std::pair<Double, Double> toVelLimits;
	auto inSpecAxis = coordClone->spectralAxisNumber(False);
	for (uInt i=0; i<2; i++) {
		// i == 0 => csysTo, i == 1 => csysFrom
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
			auto newRefPix = specCoord.referencePixel()[0];
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
		ThrowIf(
			! specCoord.frequencyToVelocity(velRefVal, freqRefVal),
			"Unable to determine reference velocity"
		);
		Double vel0, vel1;
		ThrowIf(
			! specCoord.pixelToVelocity(vel0, 0.0)
			|| ! specCoord.pixelToVelocity(vel1, 1.0),
			"Unable to determine velocity increment"
		);
		if (i == 0) {
			toVelLimits.first = vel0;
			specCoord.pixelToVelocity(
				toVelLimits.second, _getShape()[inSpecAxis] - 1
			);
			if (toVelLimits.first > toVelLimits.second) {
				std::swap(toVelLimits.first, toVelLimits.second);
			}
		}
		if (i == 1) {
			std::pair<Double, Double> fromVelLimits;
			specCoord.pixelToVelocity(
				fromVelLimits.first, 0
			);
			specCoord.pixelToVelocity(
				fromVelLimits.second, _subimage->shape()[inSpecAxis] - 1
			);
			if (fromVelLimits.first > fromVelLimits.second) {
				std::swap(fromVelLimits.first, fromVelLimits.second);
			}
			ThrowIf(
				fromVelLimits.first > toVelLimits.second
				|| fromVelLimits.second < toVelLimits.first,
				"Request to regrid by velocity, but input and output velocity coordinates do not overlap"
			);
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
		// don't bother checking the return value of the replaceCoordinate call
		// as it will always be False because the replaced and replacement coordinate
		// types differ, but the coordinate will be replaced anyway.
		// Yes I find it nonintuitive and am scratching my head regarding the usefulness
		// of the return value as well. Just check that replacement coordinate is equal to
		// the coordinate we expect.
		cs->replaceCoordinate(lin, specCoordNum);
		ThrowIf(
			! lin.near(cs->linearCoordinate(specCoordNum)),
			"Replacement linear coordinate does not match "
			"original linear coordinate because "
			+ lin.errorMessage()
		);
		if (cs == csys.get()) {
			newVelRefVal = velRefVal;
			newVelInc = vel1 - vel0;
		}
		else {
			_subimage->setCoordinateInfo(*cs);
		}
	}
	// do not pass the region or the mask, the maskedClone has already had the region and
	// mask applied
	ImageRegridder regridder(
		_subimage, 0, "", this->_getOutname(),
		this->_getOverwrite(), *csys, _getAxes(), _getShape()
	);
	regridder.setConfiguration(*this);

	SPIIF outImage = regridder._regrid();

	// replace the temporary linear coordinate with the saved spectral coordinate
	std::unique_ptr<CoordinateSystem> newCoords(
		dynamic_cast<CoordinateSystem *>(outImage->coordinates().clone())
	);

	// make frequencies correct
	Double newRefFreq;
	ThrowIf(
		! newSpecCoord.velocityToFrequency(
			newRefFreq, newVelRefVal
		),
		"Unable to determine new reference frequency"
	);
	// get the new frequency increment
	Double newFreq;
	ThrowIf (
		! newSpecCoord.velocityToFrequency(
			newFreq, newVelRefVal + newVelInc
		),
		"Unable to determine new frequency increment"
	);
	ThrowIf (
		! newSpecCoord.setReferenceValue(Vector<Double>(1, newRefFreq)),
		"Unable to set new reference frequency"
	);
	ThrowIf (
		! newSpecCoord.setIncrement((Vector<Double>(1, newFreq - newRefFreq))),
		"Unable to set new frequency increment"
	);
	ThrowIf(
		! newSpecCoord.setReferencePixel(
			templateSpecCoord.referencePixel()
		), "Unable to set new reference pixel"
	);
	ThrowIf(
		! newCoords->replaceCoordinate(
			newSpecCoord,
			_subimage->coordinates().linearCoordinateNumber())
		&& ! newSpecCoord.near(newCoords->spectralCoordinate()),
		"Unable to replace coordinate for velocity regridding"
	);
	outImage->setCoordinateInfo(*newCoords);
	return outImage;
}

Bool ImageRegridder::_doImagesOverlap(
	SPCIIF image0, SPCIIF image1
) const {
	const CoordinateSystem csys0 = image0->coordinates();
	const CoordinateSystem csys1 = image1->coordinates();
	IPosition shape0 = image0->shape();
	IPosition shape1 = image1->shape();
	ImageMetaData md0(image0);
	ImageMetaData md1(image1);
	Bool overlap = False;
	if (
		csys0.hasDirectionCoordinate()
		&& csys1.hasDirectionCoordinate()
	) {
		const DirectionCoordinate dc0 = csys0.directionCoordinate();
		DirectionCoordinate dc1 = csys1.directionCoordinate();
		Vector<Int> dirShape0 = md0.directionShape();
		Vector<Int> dirShape1 = md1.directionShape();
		Vector<Double> inc0 = dc0.increment();
		Vector<Double> inc1 = dc1.increment();
		Vector<String> units0 = dc0.worldAxisUnits();
		Vector<String> units1 = dc1.worldAxisUnits();
		Bool reallyBig = False;
		Quantity extent;
		Quantity oneDeg(1, "deg");
		for (uInt i=0; i<2; ++i) {
			extent = Quantity(dirShape0[i]*abs(inc0[i]), units0[i]);
			if (extent > oneDeg) {
				reallyBig = True;
				break;
			}
			extent = Quantity(dirShape1[i]*abs(inc1[i]), units1[i]);
			if (extent > oneDeg) {
				reallyBig = True;
				break;
			}
		}
		if (reallyBig) {
			*this->_getLog() << LogOrigin("ImageRegridder", __func__)
				<< LogIO::WARN << "At least one of the images "
				<< "exceeds one degree on at one side, not checking "
				<< "for direction plane overlap." << LogIO::POST;
		}
		else {
			Bool sameFrame = dc0.directionType(True) == dc1.directionType(True);
			if (!sameFrame) {
				dc1.setReferenceConversion(dc0.directionType(True));
			}
			Vector<std::pair<Double, Double> > corners0 = _getDirectionCorners(
				dc0, dirShape0
			);
			Vector<std::pair<Double, Double> > corners1 = _getDirectionCorners(
				dc1, dirShape1
			);
			overlap = _doRectanglesIntersect(corners0, corners1);
			if (! overlap) {
				return False;
			}
		}
	}
	if (
		csys0.hasSpectralAxis()
		&& csys1.hasSpectralAxis()
	) {
		const SpectralCoordinate sp0 = csys0.spectralCoordinate();
		const SpectralCoordinate sp1 = csys1.spectralCoordinate();
		uInt nChan0 = md0.nChannels();
		uInt nChan1 = md1.nChannels();
		Double world;
		sp0.toWorld(world, 0);
		Double end00 = world;
		sp0.toWorld(world, nChan0 - 1);
		Double end01 = world;
		sp1.toWorld(world, 0);
		Double end10 = world;
		sp1.toWorld(world, nChan1 - 1);
		Double end11 = world;
		if (
			max(end00, end01) < min(end10, end11)
			|| max(end10, end11) < min(end00, end01)
		) {
			return False;
		}
	}
	return True;
}

Vector<std::pair<Double, Double> > ImageRegridder::_getDirectionCorners(
	const DirectionCoordinate& dc,
	const IPosition& directionShape
) {
	Vector<Double> world;
	Vector<Double> pixel(2, 0);
	Vector<String> units = dc.worldAxisUnits();
	dc.toWorld(world, pixel);
	Vector<std::pair<Double, Double> > corners(4);
	for (uInt i=0; i<4; i++) {
		switch(i) {
		case 0:
			// blcx, blcy
			pixel.set(0);
			break;
		case 1:
			// trcx, blcy
			pixel[0] = directionShape[0];
			pixel[1] = 0;
			break;
		case 2:
			// trcx, trcy
			pixel[0] = directionShape[0];
			pixel[1] = directionShape[1];
			break;
		case 3:
			// blcx, trcy
			pixel[0] = 0;
			pixel[1] = directionShape[1];
			break;
		default:
			ThrowCc("Logic Error: This code should never be reached");
			break;
		}
		dc.toWorld(world, pixel);
		Double x = Quantity(world[0], units[0]).getValue("rad");
		if (fabs(x) >= C::_2pi) {
			// resolve 2pi ambiguities for x (longitude) coordinate
			x = fmod(x, C::_2pi);
		}
		if (x < 0) {
			// ensure longitude is > 0
			x += C::_2pi;
		}
		corners[i].first = x;
		corners[i].second = Quantity(world[1], units[1]).getValue("rad");
	}
	return corners;
}

Bool ImageRegridder::_doRectanglesIntersect(
	const Vector<std::pair<Double, Double> >& corners0,
	const Vector<std::pair<Double, Double> >& corners1
) {
	Double minx0 = corners0[0].first;
	Double maxx0 = minx0;
	Double miny0 = corners0[0].second;
	Double maxy0 = miny0;
	Double minx1 = corners1[0].first;
	Double maxx1 = minx1;
	Double miny1 = corners1[0].second;
	Double maxy1 = miny1;
	for (uInt i=1; i<4; i++) {
		minx0 = min(minx0, corners0[i].first);
		maxx0 = max(maxx0, corners0[i].first);
		miny0 = min(miny0, corners0[i].second);
		maxy0 = max(maxy0, corners0[i].second);

		minx1 = min(minx1, corners1[i].first);
		maxx1 = max(maxx1, corners1[i].first);
		miny1 = min(miny1, corners1[i].second);
		maxy1 = max(maxy1, corners1[i].second);
	}
	if (
		minx0 > maxx1 || maxx0 < minx1
		|| miny0 > maxy1 || maxy0 < miny1
	) {
		// bounds check shows images do not intersect
		return False;
	}
	else if (
		(minx0 >= minx1 && maxx0 <= maxx1 && miny0 >= miny1 && maxy0 <= maxy1)
		|| (minx0 < minx1 && maxx0 > maxx1 && miny0 < miny1 && maxy0 > maxy1)
	) {
		// one image lies completely inside the other
		return True;
	}
	else {
		// determine intersection
		// FIXME There are more efficient algorithms. See eg
		// the Shamos-Hoey Algorithm
		// http://geomalgorithms.com/a09-_intersect-3.html#Pseudo-Code%3a%20S-H
		for (uInt i=0; i<4; i++) {
			Vector<Double> start0(2, corners0[i].first);
			start0[1] = corners0[i].second;
			Vector<Double> end0(
				2,
				i == 3 ? corners0[0].first
					: corners0[i+1].first
			);
			end0[1] = i == 3 ? corners0[0].second : corners0[i+1].second;

			for (uInt j=0; j<4; j++) {
				Vector<Double> start1(2, corners1[j].first);
				start1[1] = corners1[j].second;
				Vector<Double> end1(
					2,
					j == 3 ? corners1[0].first
						: corners1[j+1].first
				);
				end1[1] = j == 3 ? corners1[0].second : corners1[j+1].second;
				if (
					Geometry::doLineSegmentsIntersect(
						start0[0], start0[1], end0[0], end0[1],
						start1[0], start1[1], end1[0], end1[1]
					)
				) {
					return True;
					break;
				}
			}
		}
	}
	return False;
}

}


