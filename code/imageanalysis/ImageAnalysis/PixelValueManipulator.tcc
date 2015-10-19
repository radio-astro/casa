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

#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>

// #include <casa/Arrays/ArrayMath.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>

namespace casa {

template<class T> const String PixelValueManipulator<T>::_className = "PixelValueManipulator";

template<class T> PixelValueManipulator<T>::PixelValueManipulator(
	const SPCIIT image,
	const Record *const regionRec,
	const String& mask
) : ImageTask<T>(
	image, "", regionRec, "", "", "",
	mask, "", False
), _axes() {
	this->_construct();
}

template<class T> void PixelValueManipulator<T>::setAxes(
	const IPosition& axes, Bool invert
) {
	uInt ndim = this->_getImage()->ndim();
	ThrowIf(
		axes.nelements() > ndim,
		"Too many axes, image only has "
		+ String::toString(ndim)
		+ " dimensions"
	);
	if (! axes.empty()) {
		Vector<Int> t = axes.asVector();
		ThrowIf(
			max(t) >= (Int)ndim,
			"image does not have axis " + String::toString(max(t))
		);
		ThrowIf(
			min(t) < 0, "Axis cannot be negative"
		);
	}
	_axes = invert
		? IPosition::otherAxes(ndim, axes)
		: axes;
}

template<class T> Record PixelValueManipulator<T>::get() const {
	SPCIIT subImage = SubImageFactory<T>::createSubImageRO(
		*this->_getImage(), *this->_getRegion(), this->_getMask(),
		(this->_getVerbosity() > ImageTask<T>::QUIET ? this->_getLog().get() : 0),
		AxesSpecifier(), this->_getStretch()
	);
    if (! _axes.empty()) {
		ImageCollapser<T> collapser(
			subImage, _axes, False, ImageCollapserData::MEAN,
			"", False
		);
		subImage = collapser.collapse();
	}
    Array<T> values = subImage->get(this->_getDropDegen());
    Array<Bool> mask = Array<Bool>(values.shape(), True);
    if (subImage->isMasked()) {
        mask = mask && subImage->getMask(this->_getDropDegen());
    }
    Record ret;
    ret.define("values", values);
    ret.define("mask", mask);
    return ret;
}

template<class T> Record PixelValueManipulator<T>::getProfile(
	uInt axis, ImageCollapserData::AggregateType function,
	const String& unit, PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& frame
) {
	return getProfile(
		axis, ImageCollapserData::minMatchMap()->find((uInt)function)->second,
		unit,  specType, restFreq,  frame
	);
}

template<class T> Record PixelValueManipulator<T>::getProfile(
	uInt axis, const String& function,
	const String& unit, PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& frame
) {
	ImageCollapser<T> collapser(
		function, this->_getImage(), this->_getRegion(),
		this->_getMask(), IPosition(1, axis), True, "", ""
	);
	collapser.setStretch(this->_getStretch());
	SPIIT collapsed = collapser.collapse();
	Record ret;
	Array<T> values = collapsed->get(True);
	ret.define("values", values);
	Array<Bool> mask(values.shape(), True);
	if (collapsed->isMasked()) {
		mask = mask && collapsed->getMask(True);
	}
	if (collapsed->hasPixelMask()) {
		mask = mask && collapsed->pixelMask().get(True);
	}
	ret.define("mask", mask);
	ret.define("yUnit", collapsed->units().getName());
	String tunit = unit;
	tunit.downcase();
	if (tunit.startsWith("pix")) {
		Vector<Double> pix(collapsed->ndim(), 0);
		Vector<Double> outputRef = collapsed->coordinates().toWorld(pix);
		Vector<Double> inputRef = this->_getImage()->coordinates().referenceValue();
		inputRef[axis] = outputRef[axis];
		Vector<Double> inputPixel = this->_getImage()->coordinates().toPixel(inputRef);
		Double count = floor(inputPixel[axis] + 0.5);
		uInt length = values.shape()[0];
		Vector<Double> coords = indgen(length, count, 1.0);
		ret.define("coords", coords);
		ret.define("xUnit", "pixel");
	}
	else {
		ret.merge(
			_doWorld(
				collapsed, unit, specType,
				restFreq, frame, axis
			)
		);
	}
	if (this->_getLogFile()) {
		auto axisName = this->_getImage()->coordinates().worldAxisNames()[axis];
		auto cAxis = axisName;
		cAxis.downcase();
		Quantity xunit(1, ret.asString("xUnit"));
		if (
			cAxis.startsWith("freq")
			&& xunit.isConform(Unit("m/s"))
		) {
			axisName = "Velocity";
		}
		auto imageName = this->_getImage()->name();
		ostringstream oss;
		oss << "#title: " << axisName
			<< " profile - " << imageName << endl;
		if (! _regionName.empty()) {
			oss << "#region : " << _regionName << endl;
		}
		oss << "#xUnit " << xunit.getUnit() << endl;
		oss << "#yUnit " << ret.asString("yUnit") << endl;
		oss << "# " << imageName << endl << endl;
		Vector<T> data;
		ret.get("values", data);
        Vector<Bool> mask;
		ret.get("mask", mask);
		Vector<Double> xvals = ret.asArrayDouble("coords");
		auto diter = begin(data);
		auto dend = end(data);
		auto citer = begin(xvals);
        auto miter = begin(mask);
		for ( ; diter != dend; ++diter, ++citer, ++miter) {
            if (*miter) {
			    oss << fixed << setprecision(7) << *citer
				    << " " << setw(10) << *diter << endl;
            }
		}
		this->_writeLogfile(oss.str());
	}

	return ret;
}

template<class T> Record PixelValueManipulator<T>::_doWorld(
	SPIIT collapsed, const String& unit,
	PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& frame,
	uInt axis
) const {
	// drop degenerate axes
	SPIIT tmp = SubImageFactory<T>::createImage(
		*collapsed, "", Record(), "", AxesSpecifier(IPosition(1, axis)),
		False, False, False
	);
	const CoordinateSystem csys = tmp->coordinates();
	Quantity t(0, unit);
	String axisUnit = csys.worldAxisUnits()[0];
	if (! unit.empty()) {
		_checkUnit(unit, csys, specType);
	}
	uInt length = tmp->shape()[0];
	Vector<Double> coords(length);
	Matrix<Double> pixel;
	Matrix<Double> world;
	Vector<Bool> failures;
	if (
		! frame.empty() && csys.hasSpectralAxis()
		&& MFrequency::typeFromString(frame)
		!= csys.spectralCoordinate().frequencySystem(True)
	) {
		// We need to use the original coordinate system because we need
		// the direction coordinate to be able to set the spectral
		// conversion frame
		CoordinateSystem mycsys(this->_getImage()->coordinates());
		pixel.resize(IPosition(2, mycsys.nPixelAxes(), length));
		pixel.set(0);
		pixel.row(axis) = indgen(length, 0.0, 1.0);
		SpectralCoordinate spCoord = mycsys.spectralCoordinate();
		mycsys.setSpectralConversion(frame);
		ThrowIf (!
			mycsys.toWorldMany(world, pixel, failures),
			"Unable to convert spectral coordinates to " + frame
		);
		coords = world.row(axis);
	}
	else {
		pixel.resize(IPosition(2, 1, length));
		pixel.set(0);
		pixel.row(0) = indgen(length, 0.0, 1.0);
		ThrowIf(
			! csys.toWorldMany(world, pixel, failures),
			"Unable to convert to world coordinates"
		);
		coords = world.row(0);
	}
	if (! unit.empty() && unit != axisUnit) {
		if (t.isConform(axisUnit)) {
			Quantum<Vector<Double> > q(coords, axisUnit);
			coords = q.getValue(unit);
		}
		else {
			_doNoncomformantUnit(
				coords, csys, unit, specType, restFreq, axisUnit
			);
		}
	}
	Record ret;
	ret.define("coords", coords);
	ret.define("xUnit", unit.empty() ? axisUnit : unit);
	return ret;
}

template<class T> void PixelValueManipulator<T>::_doNoncomformantUnit(
	Vector<Double>& coords, const CoordinateSystem& csys,
	const String& unit, PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& axisUnit
) const {
	ThrowIf(
		! csys.hasSpectralAxis(),
		"Units must be conformant with" + axisUnit
	);
	SpectralCoordinate sp = csys.spectralCoordinate();
	if (restFreq) {
		Double value = restFreq->getValue(axisUnit);
		sp.setRestFrequency(value, False);
		sp.selectRestFrequency(value);
	}
	Quantity t(0, unit);
	if (t.isConform("m/s")) {
		MDoppler::Types doppler;
		if (
			specType == PixelValueManipulatorData::DEFAULT
			|| specType == PixelValueManipulatorData::RELATIVISTIC
		) {
			doppler = MDoppler::RELATIVISTIC;
		}
		else if (specType == PixelValueManipulatorData::RADIO_VELOCITY) {
			doppler = MDoppler::RADIO;
		}
		else if (specType == PixelValueManipulatorData::OPTICAL_VELOCITY) {
			doppler = MDoppler::OPTICAL;
		}
		else {
			ThrowCc("Spectral type not compatible with velocity units");
		}
		sp.setVelocity(unit ,doppler);
		sp.frequencyToVelocity(coords, coords);
	}
	else {
		// unit must be conformant with meters
		sp.setWavelengthUnit(unit);
		if (
			specType == PixelValueManipulatorData::DEFAULT
			|| specType == PixelValueManipulatorData::WAVELENGTH
		) {
			sp.frequencyToWavelength(coords, coords);
		}
		else if (specType == PixelValueManipulatorData::AIR_WAVELENGTH) {
			sp.frequencyToAirWavelength(coords, coords);
		}
	}
}

template<class T> void PixelValueManipulator<T>::_checkUnit(
	const String& unit, const CoordinateSystem& csys,
	PixelValueManipulatorData::SpectralType specType
) const {
	Quantity t(0, unit);
	String axisUnit = csys.worldAxisUnits()[0];
	if (! t.isConform(axisUnit)) {
		if (csys.hasSpectralAxis()) {
			ThrowIf(
				! (t.isConform("m/s") || t.isConform("m")),
				"Invalid spectral conversion unit " + unit
			);
			ThrowIf(
				t.isConform("m/s")
				&& (
					specType == PixelValueManipulatorData::WAVELENGTH
					|| specType == PixelValueManipulatorData::AIR_WAVELENGTH
				),
				"Inconsistent spectral type used for velocity units"
			);
			ThrowIf(
				t.isConform("m")
				&& (
					specType == PixelValueManipulatorData::OPTICAL_VELOCITY
					|| specType == PixelValueManipulatorData::RADIO_VELOCITY
				),
				"Inconsistent spectral type used for wavelength units"
			);
		}
		else {
			ThrowCc(
				"Unit " + unit
				+ " does not conform to corresponding axis unit "
				+ axisUnit
			);
		}
	}
}

template<class T> void PixelValueManipulator<T>::put(
	SPIIT image, const Array<T>& pixelsArray, const Vector<Int>& blc,
	const Vector<Int>& inc, Bool list, Bool locking, Bool replicate
) {
	IPosition imageShape = image->shape();
	uInt ndim = imageShape.nelements();
	ThrowIf(
		pixelsArray.ndim() > ndim,
		"Pixel array cannot have more dimensions than the image!"
	);

	// Verify blc value. Fill in values for blc and inc.  trc set to shape-1
	IPosition iblc = IPosition(blc);
	IPosition itrc = imageShape - 1;
	IPosition iinc(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
	LCBox::verify(iblc, itrc, iinc, imageShape);

	// Create two slicers; one describing the region defined by blc + shape-1
	// with extra axes given length 1. The other we extend with the shape
	IPosition len = pixelsArray.shape();
	len.resize(ndim, True);
	for (uInt i = pixelsArray.shape().nelements(); i < ndim; i++) {
		len(i) = 1;
		itrc(i) = imageShape(i) - 1;
	}
	Slicer sl(iblc, len, iinc, Slicer::endIsLength);
	ThrowIf(
		sl.end() + 1 > imageShape,
		"Pixels array, including inc, extends beyond edge of image."
	);
	Slicer sl2(iblc, itrc, iinc, Slicer::endIsLast);

	if (list) {
		LogIO log;
		log << LogOrigin("PixelValueManipulator", __func__)
			<< LogIO::NORMAL << "Selected bounding box " << sl.start()
			<< " to " << sl.end() << LogIO::POST;
	}

	// Put the pixels
	if (pixelsArray.ndim() == ndim) {
		// _setCache(pixelsArray.shape());
		if (replicate) {
			LatticeUtilities::replicate(*image, sl2, pixelsArray);
		}
		else {
			image->putSlice(pixelsArray, iblc, iinc);
		}
	}
	else {
		// Pad with extra degenerate axes if necessary (since it is somewhat
		// costly).
		Array<T> pixelsref(
			pixelsArray.addDegenerate(
				ndim - pixelsArray.ndim()
			)
		);
		// _setCache(pixelsref.shape());
		if (replicate) {
			LatticeUtilities::replicate(*image, sl2, pixelsref);
		}
		else {
			image->putSlice(pixelsref, iblc, iinc);
		}
	}
	// Ensure that we reconstruct the  histograms objects
	// now that the data have changed
	if (locking) {
		image->unlock();
	}
}

template<class T> Bool PixelValueManipulator<T>::putRegion(
    SPIIT image, const Array<T>& pixels,
    const Array<Bool>& mask, Record& region, Bool list,
    Bool usemask, Bool replicateArray
) {
    unique_ptr<LogIO> _log(new LogIO());
    *_log << LogOrigin(_className, __func__);

    // used to verify array dimension
    uInt img_ndim = image->shape().asVector().nelements();

    // Checks on pixels dimensions
    Vector<Int> p_shape = pixels.shape().asVector();
    uInt p_ndim = p_shape.size();
    if (p_ndim > img_ndim) {
        *_log << "Pixels array has more axes than the image"
                << LogIO::EXCEPTION;
        return False;
    }
    //    if (p_ndim == 0) {
    //  *_log << "The pixels array is empty" << LogIO::EXCEPTION;
    //}
    for (uInt i = 0; i < p_ndim; i++) {
        if (p_shape(i) <= 0) {
            *_log << "The shape of the pixels array is invalid"
                    << LogIO::EXCEPTION;
            return False;
        }
    }

    // Checks on pixelmask dimensions
    Vector<Int> m_shape = mask.shape().asVector();
    uInt m_ndim = m_shape.size();
    if (m_ndim > img_ndim) {
        *_log << "Mask array has more axes than the image"
                << LogIO::EXCEPTION;
    }
    //if (m_ndim == 0) {
    //  *_log << "The pixelmask array is empty" << LogIO::EXCEPTION;
    //  return False;
    //}
    for (uInt i = 0; i < m_ndim; i++) {
        if (m_shape(i) <= 0) {
            *_log << "The shape of the pixelmask array is invalid"
                    << LogIO::EXCEPTION;
            return False;
        }
    }

    // Warning, an empty Array comes through the Glish tasking system
    // as shape = [0], ndim = 1, nelements = 0
    IPosition dataShape;
    uInt dataDim = 0;
    uInt pixelElements = pixels.nelements();
    uInt maskElements = mask.nelements();

    //
    if (pixelElements != 0 && maskElements != 0) {
        if (!pixels.shape().isEqual(mask.shape())) {
            *_log << "Pixels and mask arrays have different shapes"
                    << LogIO::EXCEPTION;
        }
        if (pixelElements != 0) {
            dataShape = pixels.shape();
            dataDim = pixels.ndim();
        } else {
            dataShape = mask.shape();
            dataDim = mask.ndim();
        }
    } else if (pixelElements != 0) {
        dataShape = pixels.shape();
        dataDim = pixels.ndim();
    } else if (maskElements != 0) {
        dataShape = mask.shape();
        dataDim = mask.ndim();
    } else {
        *_log << "Pixels and mask arrays are both zero length"
                << LogIO::EXCEPTION;
    }

    // Make region.  If the region extends beyond the image, it is
    // truncated here.

    const ImageRegion* pRegion = ImageRegion::fromRecord(
        (list ? _log.get() : 0), image->coordinates(), image->shape(),
        region
    );
    LatticeRegion latRegion = pRegion->toLatticeRegion(image->coordinates(),
            image->shape());
    // The pixels array must be same shape as the bounding box of the
    // region for as many axes as there are in the pixels array.  We
    // pad with degenerate axes for missing axes. If the region
    // dangled over the edge, it will have been truncated and the
    // array will no longer be the correct shape and we get an error.
    // We could go to the trouble of fishing out the bit that doesn't
    // fall off the edge.
    for (uInt i = 0; i < dataDim; i++) {
        if (dataShape(i) != latRegion.shape()(i)) {
            if (!(i == dataDim - 1 && dataShape(i) == 1)) {
                ostringstream oss;
                oss << "Data array shape (" << dataShape
                        << ") including inc, does not"
                        << " match the shape of the region bounding box ("
                        << latRegion.shape() << ")" << endl;
                *_log << String(oss) << LogIO::EXCEPTION;
            }
        }
    }

    // If our image doesn't have a mask, try and make it one.
    if (maskElements > 0) {
        if (! image->hasPixelMask()) {
            String maskName("");
            ImageMaskAttacher::makeMask(*image, maskName, True, True, *_log, list);
        }
    }
    Bool useMask2 = usemask;
    if (!image->isMasked())
        useMask2 = False;

    // Put the mask first
    if (maskElements > 0 && image->hasPixelMask()) {
        Lattice<Bool>& maskOut = image->pixelMask();
        if (maskOut.isWritable()) {
            if (dataDim == img_ndim) {
                if (replicateArray) {
                    LatticeUtilities::replicate(maskOut, latRegion.slicer(),
                            mask);
                } else {
                    maskOut.putSlice(mask, latRegion.slicer().start());
                }
            } else {
                *_log << LogIO::NORMAL
                        << "Padding mask array with degenerate axes"
                        << LogIO::POST;
                Array<Bool> maskref(mask.addDegenerate(img_ndim - mask.ndim()));
                if (replicateArray) {
                    LatticeUtilities::replicate(maskOut, latRegion.slicer(),
                            maskref);
                } else {
                    maskOut.putSlice(maskref, latRegion.slicer().start());
                }
            }
        } else {
            *_log
                    << "The mask is not writable. Probably an ImageExpr or SubImage"
                    << LogIO::EXCEPTION;
        }
    }

    // Get the mask and data from disk if we need it
    IPosition pixelsShape = pixels.shape();
    Array<Bool> oldMask;
    Array<Float> oldData;
    Bool deleteOldMask, deleteOldData, deleteNewData;
    const Bool* pOldMask = 0;
    const Float* pOldData = 0;
    const Float* pNewData = 0;
    if (pixelElements > 0 && useMask2) {
        if (pixels.ndim() != img_ndim) {
            pixelsShape.append(IPosition(img_ndim - pixels.ndim(), 1));
        }
        oldData = image->getSlice(latRegion.slicer().start(), pixelsShape,
                False);
        oldMask = image->getMaskSlice(latRegion.slicer().start(),
                pixelsShape, False);
        pOldData = oldData.getStorage(deleteOldData); // From disk
        pOldMask = oldMask.getStorage(deleteOldMask); // From disk
        pNewData = pixels.getStorage(deleteNewData); // From user
    }

    // Put the pixels
    if (dataDim == img_ndim) {
        if (pixelElements > 0) {
            if (useMask2) {
                Bool deleteNewData2;
                Array<Float> pixels2(pixelsShape);
                Float* pNewData2 = pixels2.getStorage(deleteNewData2);
                for (uInt i = 0; i < pixels2.nelements(); i++) {
                    pNewData2[i] = pNewData[i]; // Value user gives
                    if (!pOldMask[i])
                        pNewData2[i] = pOldData[i]; // Value on disk
                }
                pixels2.putStorage(pNewData2, deleteNewData2);
                if (replicateArray) {
                    LatticeUtilities::replicate(*image, latRegion.slicer(),
                            pixels2);
                } else {
                    image->putSlice(pixels2, latRegion.slicer().start());
                }
            } else {
                if (replicateArray) {
                    LatticeUtilities::replicate(*image, latRegion.slicer(),
                            pixels);
                } else {
                    image->putSlice(pixels, latRegion.slicer().start());
                }
            }
        }
    } else {
        if (pixelElements > 0) {
            *_log << LogIO::NORMAL
                    << "Padding pixels array with degenerate axes"
                    << LogIO::POST;
            //
            if (useMask2) {
                Bool deleteNewData2;
                Array<Float> pixels2(pixelsShape);
                Float* pNewData2 = pixels2.getStorage(deleteNewData2);
                for (uInt i = 0; i < pixels2.nelements(); i++) {
                    pNewData2[i] = pNewData[i]; // Value user gives
                    if (!pOldMask[i])
                        pNewData2[i] = pOldData[i]; // Value on disk
                }
                pixels2.putStorage(pNewData2, deleteNewData2);
                if (replicateArray) {
                    LatticeUtilities::replicate(*image, latRegion.slicer(),
                            pixels2);
                } else {
                    image->putSlice(pixels2, latRegion.slicer().start());
                }
            } else {
                Array<Float> pixelsref(pixels.addDegenerate(img_ndim
                        - pixels.ndim()));
                if (replicateArray) {
                    LatticeUtilities::replicate(*image, latRegion.slicer(),
                            pixelsref);
                } else {
                    image->putSlice(pixelsref, latRegion.slicer().start());
                }
            }
        }
    }

    if (pOldMask != 0)
        oldMask.freeStorage(pOldMask, deleteOldMask);
    if (pOldData != 0)
        oldData.freeStorage(pOldData, deleteOldData);
    if (pNewData != 0)
        pixels.freeStorage(pNewData, deleteNewData);
    delete pRegion;

    return True;
}

}


