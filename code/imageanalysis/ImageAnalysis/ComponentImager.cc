//# ComponentImager.cc:  this defines ComponentImager which modifies images by ComponentLists
//# Copyright (C) 1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: ComponentImager.cc 18855 2005-07-21 08:03:40Z nkilleen $

#include <imageanalysis/ImageAnalysis/ComponentImager.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/Stokes.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/SpectralModel.h>
#include <components/ComponentModels/Flux.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageInfo.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

void ComponentImager::project(ImageInterface<Float>& image, const ComponentList& list) 
{
	const CoordinateSystem& coords = image.coordinates();
	const IPosition imageShape = image.shape();
	LogIO os(LogOrigin("ComponentImager", "project"));

	// I currently REQUIRE that:
	// * The list has at least one element.
	// * The image has at least one pixel.
	// * The image has one direction coordinate (only).
	// * The direction coordinate has two pixel and two world axes.
	// * Polarization and frequency coordinates are optional, however at most one
	//   each of these coordinates can exist.
	// * If there is a Stokes axis it can only contain Stokes::I,Q,U,V pols.
	// * No other coordinate types, like LinearCoordinate, are used.
	ThrowIf(
		! coords.hasDirectionCoordinate(), "Image does not have a direction coordinate"
	);
	// FIXME: the variable definitions are reveresed; the first axis is the longitude axis,
	// the second axis is the latitude axis.
	uInt latAxis, longAxis;
	{
		const Vector<Int> dirAxes = coords.directionAxesNumbers();
		latAxis = dirAxes(0);
		longAxis = dirAxes(1);
	}
	DirectionCoordinate dirCoord = coords.directionCoordinate();
	dirCoord.setWorldAxisUnits(Vector<String>(2, "rad"));

	// Make sure get conversion frame, not just the native one

	MDirection::Types dirFrame;
	dirCoord.getReferenceConversion(dirFrame);
	const MeasRef<MDirection> dirRef(dirFrame);

	MVAngle pixelLatSize, pixelLongSize;
	{
		const Vector<Double> inc = dirCoord.increment();
		pixelLatSize = MVAngle(abs(inc(0)));
		pixelLongSize = MVAngle(abs(inc(1)));
	}

	// Check if there is a Stokes Axes and if so which polarizations. Otherwise
	// only grid the I polarisation.
	uInt nStokes = 0;
	if (coords.hasPolarizationCoordinate()) {
		StokesCoordinate stCoord = coords.stokesCoordinate();
		Vector<Int> types = stCoord.stokes();
		nStokes = types.nelements();
		for (uInt p = 0; p < nStokes; p++) {
			Stokes::StokesTypes type = Stokes::type(types[p]);
			ThrowIf(
				type != Stokes::I && type != Stokes::Q
				&& type != Stokes::U && type != Stokes::V,
				"Unsupported stokes type " + Stokes::name(type)
			);
		}
	}

	// Check if there is a frequency axis and if so get the all the frequencies
	// as a Vector<MVFrequency>. Otherwise assume the reference frequency is the
	// same as the reference frequency of the first component in the list.

	MeasRef<MFrequency> freqRef;
	uInt nFreqs = 1;
	Int freqAxis = -1;
	Vector<MVFrequency> freqValues(nFreqs);
	if (coords.hasSpectralAxis()) {
		freqAxis = coords.spectralAxisNumber(False);
		nFreqs = (uInt)imageShape[freqAxis];
		freqValues.resize(nFreqs);
		SpectralCoordinate specCoord = coords.spectralCoordinate();
		specCoord.setWorldAxisUnits(Vector<String>(1, "Hz"));

		// Create Frequency MeasFrame; this will enable conversions between
		// spectral frames (e.g. the CS frame might be TOPO and the CL
		// frame LSRK)

		MFrequency::Types specConv;
		MEpoch epochConv;
		MPosition posConv;
		MDirection dirConv;
		specCoord.getReferenceConversion(specConv,  epochConv, posConv, dirConv);
		MeasFrame measFrame(epochConv, posConv, dirConv);
		freqRef = MeasRef<MFrequency>(specConv, measFrame);
		Double thisFreq;
		for (uInt f = 0; f < nFreqs; f++) {
			// Includes any frame conversion
			ThrowIf (
				!specCoord.toWorld(thisFreq, f),
				"cannot convert a frequency value"
			);
			freqValues(f) = MVFrequency(thisFreq);
		}
	}
	else {
		const MFrequency& defaultFreq =
				list.component(0).spectrum().refFrequency();
		freqRef = defaultFreq.getRef();
		freqValues(0) = defaultFreq.getValue();
	}

	// Find out what the units are. Currently allowed units are anything
	// dimensionally equivalent to Jy/pixel or Jy/beam. If the former then the
	// pixel size at the centre of the image is assumed to hold throughout the
	// image. If the latter then the beam is fished out of the header and a
	// 'beam' unit defined. If the units are not defined or are not one of the
	// above they are assumed to be Jy/pixel and a warning message is sent to the
	// logger.

	Unit fluxUnits;
	{
		Unit imageUnit = image.units();
		const String& imageUnitName = imageUnit.getName();
		UnitMap::putUser(
			"pixel", UnitVal(pixelLatSize.radian() *
			pixelLongSize.radian(), "rad.rad")
		);
		// redefine is required to reset Unit Cache
		const Unit pixel("pixel");
		if (imageUnitName.contains("pixel")) {
			// Get the new definition of the imageUnit which uses the new
			// definition of the pixels unit.
			imageUnit = image.units();
			fluxUnits.setValue(imageUnit.getValue() * pixel.getValue());
			fluxUnits.setName(imageUnitName + String(".") + pixel.getName());
		}
		else if (imageUnitName.contains("beam")) {
			const ImageInfo imageInfo = image.imageInfo();
			// FIXME this needs to support multiple beams
			const GaussianBeam beam = imageInfo.restoringBeam();
			if (beam.isNull()) {
				os << LogIO::WARN
					<< "No beam defined even though the image units contain a beam"
					<< endl << "Assuming the beam is one pixel" << LogIO::POST;
				UnitMap::putUser("beam", pixel.getValue());
			}
			else {
				const Quantum<Double> beamArea = beam.getArea("sr");
				UnitMap::putUser(
					"beam", UnitVal(beamArea.getValue(),
					beamArea.getFullUnit().getName()))
				;
			}
			const Unit beamUnit("beam");
			const UnitVal fudgeFactor(
				pixel.getValue().getFac()/
				beamUnit.getValue().getFac()
			);

			// Get the new definition of the imageUnit which uses the new
			// definition of the beam unit.  The re-use of the Unit constructor
			// from the String forces the new Unit definitions to take effect
			imageUnit = Unit(image.units().getName());
			fluxUnits.setValue(imageUnit.getValue() *
					beamUnit.getValue() * fudgeFactor);
			fluxUnits.setName(imageUnitName + String(".") + beamUnit.getName());
		}
		// 20101013 the code above for Jy/pixel doesn't work, since Unit doesn't
		// understand that Jy/pixel.pixel == Jy.
		const Unit jy("Jy");
		os << "Adding components to image with units [" << fluxUnits.getName() << "]" << LogIO::POST;
		if (fluxUnits.getName()=="Jy/pixel.pixel") {
			fluxUnits=jy;
		}
		if (fluxUnits != jy) {
			os << LogIO::WARN
					<< "Image units [" << fluxUnits.getName() << "] are not dimensionally equivalent to "
					<< "Jy/pixel or Jy/beam " << endl
					<< "Ignoring the specified units and proceeding on the assumption"
					<< " they are Jy/pixel" << LogIO::POST;
			fluxUnits = jy;
		}
	}
	// Setup an iterator to step through the image in chunks that can fit into
	// memory. Go to a bit of effort to make the chunk size as large as
	// possible but still minimize the number of tiles in the cache.
	IPosition chunkShape = imageShape;
	{
		const IPosition tileShape = image.niceCursorShape();
		chunkShape(latAxis) = tileShape(latAxis);
		chunkShape(longAxis) = tileShape(longAxis);
	}
	IPosition pixelShape = imageShape;
	pixelShape(latAxis) = pixelShape(longAxis) = 1;
	LatticeStepper pixelStepper(imageShape, pixelShape, LatticeStepper::RESIZE);
	LatticeIterator<Float> chunkIter(image, chunkShape);
	const uInt nDirs = chunkShape(latAxis) * chunkShape(longAxis);
	Cube<Double> pixelVals(4, nDirs, nFreqs);
	Vector<MVDirection> dirVals(nDirs);
	Vector<Bool> coordIsGood(nDirs);
	const uInt naxis = imageShape.nelements();
	Vector<Double> pixelDir(2);
	uInt d;
	IPosition pixelPosition(naxis, 0);

	// Does the image have a writable mask ?  Output pixel values are
	// only modified if the mask==T  and the coordinate conversions
	// succeeded.  The mask==F on output if the coordinate conversion
	// fails (usually means a pixel is outside of the valid CoordinateSystem)
	//
	Bool doMask = False;
	if (image.isMasked() && image.hasPixelMask()) {
		if (image.pixelMask().isWritable()) {
			doMask = True;
		} else {
			os << LogIO::WARN
					<< "The image is masked, but it cannot be written to" << LogIO::POST;
		}
	}
	Lattice<Bool>* pixelMaskPtr = 0;
	if (doMask) pixelMaskPtr = &image.pixelMask();
	PtrHolder<Array<Bool> > maskPtr;
	Int polAxis = coords.polarizationAxisNumber(False);
	for (chunkIter.reset(); !chunkIter.atEnd(); chunkIter++) {

		// Iterate through sky plane of cursor and do coordinate conversions

		const IPosition& blc = chunkIter.position();
		const IPosition& trc = chunkIter.endPosition();
		d = 0;
		pixelDir(1) = blc(longAxis);
		coordIsGood = True;
		while (pixelDir(1) <= trc(longAxis)) {
			pixelDir(0) = blc(latAxis);
			while (pixelDir(0) <= trc(latAxis)) {
				if (!dirCoord.toWorld(dirVals(d), pixelDir)) {
					// These pixels will be masked
					coordIsGood(d) = False;
				}
				d++;
				pixelDir(0)++;
			}
			pixelDir(1)++;
		}

		// Sample model, converting the values in the components
		// to the specified direction and spectral frames
		list.sample(pixelVals, fluxUnits, dirVals, dirRef, pixelLatSize,
				pixelLongSize, freqValues, freqRef);

		// Modify data by model for this chunk of data
		Array<Float>& imageChunk = chunkIter.rwCursor();

		// Get input mask values if available

		if (doMask) {
			maskPtr.set(
				new Array<Bool>(
					image.getMaskSlice(chunkIter.position(),
					chunkIter.cursorShape(), False)
				)
			);
		}
		d = 0;
		pixelPosition(longAxis) = 0;
		coordIsGood = True;
		while (pixelPosition(longAxis) < chunkShape(longAxis)) {
			pixelPosition(latAxis) = 0;
			while (pixelPosition(latAxis) < chunkShape(latAxis)) {
				if (coordIsGood(d)) {
					for (uInt f = 0; f < nFreqs; f++) {
						if (freqAxis >= 0) pixelPosition(freqAxis) = f;
						for (uInt s = 0; s < nStokes; s++) {
							if (polAxis >= 0) pixelPosition(polAxis) = s;
							if (! doMask || (doMask && (*maskPtr)(pixelPosition))) {
								imageChunk(pixelPosition) += pixelVals(s, d, f);
							}
						}
					}
				}
				else if (doMask) {
					(*maskPtr)(pixelPosition) = False;
				}
				d++;
				pixelPosition(latAxis)++;
			}
			pixelPosition(longAxis)++;
		}

		// Update output mask in appropriate fashion

		if (doMask) {
			pixelMaskPtr->putSlice(*maskPtr, chunkIter.position());
		}
	}
}
} //# NAMESPACE CASA - END

