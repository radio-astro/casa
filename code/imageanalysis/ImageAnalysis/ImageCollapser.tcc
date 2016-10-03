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

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/STLIO.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/LatticeMath/LatticeMathUtil.h>

#include <memory>

namespace casa {

template<class T> map<casacore::uInt, T ( *)(const casacore::Array<T>&)> ImageCollapser<T>::_funcMap;

template<class T> ImageCollapser<T>::ImageCollapser(
    const casacore::String & aggString, const SPCIIT image,
    const casacore::Record * const regionRec,
    const casacore::String & maskInp, const casacore::IPosition & axes,
    casacore::Bool invertAxesSelection,
    const casacore::String & outname, casacore::Bool overwrite
) : ImageTask<T>(
        image, "", regionRec, "", "", "",
        maskInp, outname, overwrite
    ),
    _invertAxesSelection(invertAxesSelection),
    _axes(axes), _aggType(ImageCollapserData::UNKNOWN)
{
    _aggType = ImageCollapserData::aggregateType(aggString);
    this->_construct();
    _finishConstruction();
}

template<class T> ImageCollapser<T>::ImageCollapser(
    const SPCIIT image,
    const casacore::IPosition & axes, const casacore::Bool invertAxesSelection,
    const ImageCollapserData::AggregateType aggregateType,
    const casacore::String & outname, const casacore::Bool overwrite
) : ImageTask<T>(
        image, "", 0, "", "", "",
        "", outname, overwrite
    ),
    _invertAxesSelection(invertAxesSelection),
    _axes(axes), _aggType(aggregateType)
{
    ThrowIf (
        _aggType == ImageCollapserData::UNKNOWN,
        "UNKNOWN aggregateType not allowed"
    );
    ThrowIf(
        ! image,
        "Cannot use a null image pointer with this constructor"
    );
    this->_construct();
    _finishConstruction();
}

template<class T> SPIIT ImageCollapser<T>::collapse() const
{
    SPCIIT subImage = SubImageFactory<T>::createSubImageRO(
                          *this->_getImage(), *this->_getRegion(), this->_getMask(),
                          this->_getLog().get(), casacore::AxesSpecifier(), this->_getStretch()
                      );
    *this->_getLog() << casacore::LogOrigin(getClass(), __func__);
    ThrowIf(
        ImageMask::isAllMaskFalse(*subImage),
        "All selected pixels are masked"
    );
    casacore::CoordinateSystem outCoords = subImage->coordinates();
    casacore::Bool hasDir = outCoords.hasDirectionCoordinate();
    casacore::IPosition inShape = subImage->shape();
    if (_aggType == ImageCollapserData::FLUX) {
        _checkFlux(subImage);
        /*
        casacore::String cant = " Cannot do flux density calculation";
        ThrowIf(
            ! hasDir,
            "Image has no direction coordinate." + cant
        );
        ThrowIf(
            subImage->units().getName().contains("beam") && ! subImage->imageInfo().hasBeam(),
            "Image has no beam." + cant
        );
        casacore::Vector<casacore::Int> dirAxes = outCoords.directionAxesNumbers();
        for (casacore::uInt i=0; i<_axes.nelements(); i++) {
            casacore::Int axis = _axes[i];
            ThrowIf(
                ! anyTrue(dirAxes == axis)
                && inShape[axis] > 1,
                "Specified axis " + casacore::String::toString(axis)
                + " is not a direction axis but has length > 1." + cant
            );
        }
        */
    }
    // Set the compressed axis reference pixel and reference value
    casacore::Vector<casacore::Double> blc, trc;
    casacore::IPosition pixblc(inShape.nelements(), 0);
    casacore::IPosition pixtrc = inShape - 1;
    ThrowIf(
        ! outCoords.toWorld(blc, pixblc)
        || ! outCoords.toWorld(trc, pixtrc),
        "Could not set new coordinate values"
    );
    casacore::Vector<casacore::Double> refValues = outCoords.referenceValue();
    casacore::Vector<casacore::Double> refPixels = outCoords.referencePixel();
    casacore::IPosition outShape = inShape;
    casacore::IPosition shape(outShape.nelements(), 1);
    for (
        casacore::IPosition::const_iterator iter = _axes.begin();
        iter != _axes.end(); iter++
    ) {
        casacore::uInt i = *iter;
        refValues[i] = (blc[i] + trc[i]) / 2;
        refPixels[i] = 0;
        outShape[i] = 1;
        shape[i] = inShape[i];
    }
    ThrowIf(
        ! outCoords.setReferenceValue(refValues),
        "Unable to set reference value"
    );
    ThrowIf(
        ! outCoords.setReferencePixel(refPixels),
        "Unable to set reference pixel"
    );
    casacore::TempImage<T> tmpIm(outShape, outCoords);
    if (_aggType == ImageCollapserData::ZERO) {
        casacore::Array<T> zeros(outShape, 0.0);
        tmpIm.put(zeros);
    } else if (_aggType == ImageCollapserData::MEDIAN) {
        _doMedian(subImage, tmpIm);
    } else {
        _doOtherStats(tmpIm, subImage, shape, outShape);
    }
    casacore::Bool copied = subImage->imageInfo().hasMultipleBeams()
                  ? _doMultipleBeams(tmpIm, subImage, hasDir, outCoords)
                  : false;
    if (! copied) {
        casacore::ImageUtilities::copyMiscellaneous(tmpIm, *subImage, true);
    }
    if (_aggType == ImageCollapserData::FLUX) {
        // get the flux units right
        auto sbunit = subImage->units().getName();
        casacore::String unit;
        if (sbunit.contains("K")) {
            casacore::String areaUnit = "arcsec2";
            unit = sbunit + "." + areaUnit;
        } else {
            unit = "Jy";
            if (sbunit.contains("/beam")) {
                casacore::uInt iBeam = sbunit.find("/beam");
                unit = sbunit.substr(0, iBeam) + sbunit.substr(iBeam + 5);
            }
        }
        tmpIm.setUnits(unit);
    }
    return this->_prepareOutputImage(tmpIm);
}

template<class T> void ImageCollapser<T>::_checkFlux(
    SPCIIT subImage
) const
{
    casacore::String cant = " Cannot do flux density calculation";
    const casacore::CoordinateSystem & outCoords = subImage->coordinates();
    ThrowIf(
        ! outCoords.hasDirectionCoordinate(),
        "Image has no direction coordinate." + cant
    );
    ThrowIf(
        subImage->units().getName().contains("beam") && ! subImage->imageInfo().hasBeam(),
        "Image has no beam." + cant
    );
    casacore::Vector<casacore::Int> dirAxes = outCoords.directionAxesNumbers();
    for (casacore::uInt i = 0; i < _axes.nelements(); i++) {
        casacore::Int axis = _axes[i];
        ThrowIf(
            ! anyTrue(dirAxes == axis)
            && subImage->shape()[axis] > 1,
            "Specified axis " + casacore::String::toString(axis)
            + " is not a direction axis but has length > 1." + cant
        );
    }
}

template<class T> casacore::Bool ImageCollapser<T>::_doMultipleBeams(
    casacore::TempImage<T>& tmpIm, SPCIIT subImage, casacore::Bool hasDir,
    const casacore::CoordinateSystem & outCoords
) const
{
    casacore::Int naxes = _axes.size();
    casacore::Bool dirAxesOnlyCollapse = hasDir && naxes == 2;
    if (dirAxesOnlyCollapse) {
        casacore::Vector<casacore::Int>dirAxes = outCoords.directionAxesNumbers();
        dirAxesOnlyCollapse = (_axes[0] == dirAxes[0] && _axes[1] == dirAxes[1])
                              || (_axes[1] == dirAxes[0] && _axes[0] == dirAxes[1]);
    }
    if (! dirAxesOnlyCollapse) {
        // check for degeneracy of spectral or polarization axes
        casacore::Int specAxis = outCoords.spectralAxisNumber(false);
        casacore::Int polAxis = outCoords.polarizationAxisNumber(false);
        dirAxesOnlyCollapse = true;
        casacore::IPosition shape = subImage->shape();
        for (casacore::Int i = 0; i < naxes; i++) {
            casacore::Int axis = _axes[i];
            if (
                (axis == specAxis || axis == polAxis)
                && shape[axis] > 1
            ) {
                dirAxesOnlyCollapse = false;
                break;
            }
        }
    }
    if (! dirAxesOnlyCollapse) {
        *this->_getLog() << casacore::LogIO::WARN << "casacore::Input image has per plane beams "
                         << "but the collapse is not done exclusively along the direction axes. "
                         << "The output image will arbitrarily have a single beam which "
                         << "is the first beam available in the subimage."
                         << "Thus, the image planes will not be convolved to a common "
                         << "restoring beam before collapsing. If, however, this is desired, "
                         << "then run the task imsmooth or the tool method ia.convolve2d() first, "
                         << "and use the output image of that as the input for collapsing."
                         << casacore::LogIO::POST;
        casacore::ImageUtilities::copyMiscellaneous(tmpIm, *subImage, false);
        casacore::ImageInfo info = subImage->imageInfo();
        vector<casacore::Vector<casacore::Quantity> > out;
        casacore::GaussianBeam beam = *(info.getBeamSet().getBeams().begin());
        info.removeRestoringBeam();
        info.setRestoringBeam(beam);
        tmpIm.setImageInfo(info);
        return true;
    }
    return false;
}

template<class T> void ImageCollapser<T>::_doOtherStats(
    casacore::TempImage<T>& tmpIm, SPCIIT subImage,
    const casacore::IPosition & shape, const casacore::IPosition & outShape
) const
{
    casacore::Bool lowPerf = _aggType == ImageCollapserData::FLUX;
    if (! lowPerf) {
        lowPerf = ! ImageMask::ImageMask::isAllMaskTrue(*subImage);
    }
    T npixPerBeam = 1;
    if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
        casacore::ImageInfo info = subImage->imageInfo();
        if (! info.hasBeam()) {
            *this->_getLog() << casacore::LogIO::WARN
                             << "Image has no beam, will use sqrtsum method"
                             << casacore::LogIO::POST;
        } else if (info.hasMultipleBeams()) {
            *this->_getLog() << casacore::LogIO::WARN
                             << "casacore::Function sqrtsum_npix_beam does not support multiple beams, will"
                             << "use sqrtsum method instead"
                             << casacore::LogIO::POST;
        } else {
            npixPerBeam = info.getBeamAreaInPixels(
                              -1, -1, subImage->coordinates().directionCoordinate()
                          );
        }
    }
    if (lowPerf) {
        _doLowPerf(tmpIm, subImage, npixPerBeam);
    } else {
        // no mask, can use higher performance method
        T (*function)(const casacore::Array<T>&) = _getFuncMap().find(_aggType)->second;
        // FIXME this can exhaust memory for large images
        casacore::Array<T> data = subImage->get(false);
        casacore::Int64 nelements = outShape.product();
        for (casacore::uInt i = 0; i < nelements; i++) {
            casacore::IPosition start = toIPositionInArray(i, outShape);
            casacore::IPosition end = start + shape - 1;
            casacore::Slicer s(start, end, casacore::Slicer::endIsLast);
            tmpIm.putAt(function(data(s)), start);
        }
        if (
            _aggType == ImageCollapserData::SQRTSUM
            || _aggType == ImageCollapserData::SQRTSUM_NPIX
            || _aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM
        ) {
            // FIXME memory check
            casacore::Array<T> arr = tmpIm.get();
            _zeroNegatives(arr);
            arr = sqrt(arr);
            if (_aggType == ImageCollapserData::SQRTSUM_NPIX) {
                T npts = subImage->shape().product() / nelements;
                arr /= npts;

            } else if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
                arr /= npixPerBeam;
            }
            tmpIm.put(arr);
        }
    }
}

template<class T> void ImageCollapser<T>::_doLowPerf(
    casacore::TempImage<T>& tmpIm, SPCIIT subImage, T npixPerBeam
) const
{
    // flux or mask with one or more false values, must use lower performance methods
    casacore::LatticeStatsBase::StatisticsTypes lattStatType = _getStatsType();
    casacore::Array<T> data;
    casacore::Array<casacore::Bool> mask;
    if (_aggType == ImageCollapserData::FLUX) {
        casacore::ImageStatistics<T> stats(*subImage, false);
        stats.setAxes(_axes.asVector());
        if (
            ! stats.getConvertedStatistic(
                data, lattStatType, false
            )
        ) {
            ostringstream oss;
            oss << "Unable to calculate flux density: "
                << stats.getMessages();
            ThrowCc(oss.str());
        }
        mask.resize(data.shape());
        mask.set(true);
    } else {
        casacore::LatticeMathUtil::collapse(
            data, mask, _axes, *subImage, false,
            true, true, lattStatType
        );
        if (
            _aggType == ImageCollapserData::SQRTSUM
            || _aggType == ImageCollapserData::SQRTSUM_NPIX
            || _aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM
        ) {
            _zeroNegatives(data);
            data = sqrt(data);
            if (_aggType == ImageCollapserData::SQRTSUM_NPIX) {
                casacore::Array<T> npts = data.copy();
                casacore::LatticeMathUtil::collapse(
                    npts, mask, _axes, *subImage, false,
                    true, true, casacore::LatticeStatsBase::NPTS
                );
                data /= npts;
            } else if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
                data /= npixPerBeam;
            }
        }
    }
    casacore::Array<T> dataCopy = (_axes.size() <= 1)
                        ? data : data.addDegenerate(_axes.size() - 1);
    casacore::IPosition newOrder(tmpIm.ndim(), -1);
    casacore::uInt nAltered = _axes.size();
    casacore::uInt nUnaltered = tmpIm.ndim() - nAltered;
    casacore::uInt alteredCount = nUnaltered;
    casacore::uInt unAlteredCount = 0;
    for (casacore::uInt i = 0; i < tmpIm.ndim(); i++) {
        for (casacore::uInt j = 0; j < _axes.size(); j++) {
            if (i == _axes[j]) {
                newOrder[i] = alteredCount;
                alteredCount++;
                break;
            }
        }
        if (newOrder[i] < 0) {
            newOrder[i] = unAlteredCount;
            unAlteredCount++;
        }
    }
    tmpIm.put(reorderArray(dataCopy, newOrder));
    if (! allTrue(mask)) {
        casacore::Array<casacore::Bool> maskCopy = (
                                   _axes.size() <= 1) ? mask
                               : mask.addDegenerate(_axes.size() - 1
                                                   );
        casacore::Array<casacore::Bool> mCopy = reorderArray(maskCopy, newOrder);
        _attachOutputMask(tmpIm, mCopy);
    }
}

template<class T> casacore::LatticeStatsBase::StatisticsTypes ImageCollapser<T>::_getStatsType() const
{
    casacore::LatticeStatsBase::StatisticsTypes lattStatType = casacore::LatticeStatsBase::NACCUM;
    switch (_aggType) {
    case ImageCollapserData::FLUX:
        lattStatType = casacore::LatticeStatsBase::FLUX;
        break;
    case ImageCollapserData::MAX:
        lattStatType = casacore::LatticeStatsBase::MAX;
        break;
    case ImageCollapserData::MEAN:
        lattStatType = casacore::LatticeStatsBase::MEAN;
        break;
    case ImageCollapserData::MIN:
        lattStatType = casacore::LatticeStatsBase::MIN;
        break;
    case ImageCollapserData::NPTS:
        lattStatType = casacore::LatticeStatsBase::NPTS;
        break;
    case ImageCollapserData::RMS:
        lattStatType = casacore::LatticeStatsBase::RMS;
        break;
    case ImageCollapserData::STDDEV:
        lattStatType = casacore::LatticeStatsBase::SIGMA;
        break;
    case ImageCollapserData::SQRTSUM:
    case ImageCollapserData::SQRTSUM_NPIX:
    case ImageCollapserData::SQRTSUM_NPIX_BEAM:
    case ImageCollapserData::SUM:
        lattStatType = casacore::LatticeStatsBase::SUM;
        break;
    case ImageCollapserData::VARIANCE:
        lattStatType = casacore::LatticeStatsBase::VARIANCE;
        break;
    case ImageCollapserData::MEDIAN:
    case ImageCollapserData::ZERO:
    case ImageCollapserData::UNKNOWN:
    default:
        ThrowCc(
            "Logic error. Should never have gotten the the bottom of the switch statement"
        );
        break;
    }
    return lattStatType;
}


template<class T> void ImageCollapser<T>::_zeroNegatives(casacore::Array<T>& arr)
{
    typename casacore::Array<T>::iterator iter = arr.begin();
    if (isComplex(whatType(&(*iter))) || allGE(arr, (T)0)) {
        return;
    }
    typename casacore::Array<T>::iterator end = arr.end();
    while (iter != end) {
        if (*iter < 0) {
            *iter = 0;
        }
        iter++;
    }
}

template<class T> void ImageCollapser<T>::_finishConstruction()
{
    for (
        casacore::IPosition::const_iterator iter = _axes.begin();
        iter != _axes.end(); iter++
    ) {
        ThrowIf(
            *iter >= this->_getImage()->ndim(),
            "Specified zero-based axis (" + casacore::String::toString(*iter)
            + ") must be less than the number of axes in " + this->_getImage()->name()
            + "(" + casacore::String::toString(this->_getImage()->ndim()) + ")"
        );
    }
    _invert();
}

template<class T> void ImageCollapser<T>::_invert()
{
    if (_invertAxesSelection) {
        casacore::IPosition x = casacore::IPosition::otherAxes(this->_getImage()->ndim(), _axes);
        _axes.resize(x.size());
        _axes = x;
    }
}

template<class T> void ImageCollapser<T>::_doMedian(
    SPCIIT image, casacore::TempImage<T>& outImage
) const
{
    casacore::IPosition cursorShape(image->ndim(), 1);
    for (casacore::uInt i = 0; i < cursorShape.size(); i++) {
        for (casacore::uInt j = 0; j < _axes.size(); j++) {
            if (_axes[j] == i) {
                cursorShape[i] = image->shape()[i];
                break;
            }
        }
    }
    casacore::LatticeStepper stepper(image->shape(), cursorShape);
    // FIXME check memory
    casacore::Array<T> ary = image->get(false);
    // FIXME check memory
    casacore::Array<casacore::Bool> mask = image->getMask();
    if (image->hasPixelMask()) {
        // FIXME check memory
        mask = mask && image->pixelMask().get(false);
    }
    std::unique_ptr<casacore::Array<casacore::Bool> > outMask;
    casacore::Bool hasMaskedPixels = ! allTrue(mask);
    for (stepper.reset(); !stepper.atEnd(); stepper++) {
        casacore::Slicer slicer(stepper.position(), stepper.endPosition(), casacore::Slicer::endIsLast);
        vector<T> data = ary(slicer).tovector();
        if (hasMaskedPixels) {
            casacore::Vector<casacore::Bool> maskSlice(mask(slicer).tovector());
            if (! anyTrue(maskSlice)) {
                if (! outMask) {
                    outMask.reset(new casacore::Array<casacore::Bool>(outImage.shape(), true));
                }
                (*outMask)(stepper.position()) = false;
                //kk.resize(0);
                data.resize(0);
            } else if (! allTrue(maskSlice)) {
                //vector<T> data = kk.tovector();
                typename vector<T>::iterator diter = data.begin();
                casacore::Vector<casacore::Bool>::iterator miter = maskSlice.begin();
                while (diter != data.end()) {
                    if (! *miter) {
                        data.erase(diter);
                        if (diter == data.end()) {
                            break;
                        }
                    } else {
                        diter++;
                    }
                    miter++;
                }
            }
        }
        casacore::uInt s = data.size();
        if (s > 0) {
            sort(data.begin(), data.end());
        }
        outImage.putAt(
            s == 0
            ? 0
            : s % 2 == 1
            ? data[s / 2]
            : (data[s / 2] + data[s / 2 - 1]) / 2,
            stepper.position()
        );
    }
    if (outMask.get() != 0) {
        // FIXME check memory
        _attachOutputMask(outImage, *outMask.get());
    }
}

template<class T> void ImageCollapser<T>::_attachOutputMask(
    casacore::TempImage<T>& outImage,
    const casacore::Array<casacore::Bool>& outMask
) const
{
    if (this->_getOutname().empty()) {
        outImage.attachMask(casacore::ArrayLattice<casacore::Bool>(outMask));
    } else {
        casacore::String maskName = outImage.makeUniqueRegionName(
                              casacore::String("mask"), 0
                          );
        outImage.makeMask(maskName, true, true, true, true);
        (&outImage.pixelMask())->put(outMask);
    }
}

template<class T> const map<casacore::uInt, T ( *)(const casacore::Array<T>&)>& ImageCollapser<T>::_getFuncMap()
{
    if (_funcMap.size() == 0) {
        _funcMap[(casacore::uInt)ImageCollapserData::MAX] = casacore::max;
        _funcMap[(casacore::uInt)ImageCollapserData::MEAN] = casacore::mean;
        _funcMap[(casacore::uInt)ImageCollapserData::MEDIAN] = casacore::median;
        _funcMap[(casacore::uInt)ImageCollapserData::MIN] = casacore::min;
        _funcMap[(casacore::uInt)ImageCollapserData::RMS] = casacore::rms;
        _funcMap[(casacore::uInt)ImageCollapserData::SQRTSUM] = casacore::sum;
        _funcMap[(casacore::uInt)ImageCollapserData::SQRTSUM_NPIX] = casacore::sum;
        _funcMap[(casacore::uInt)ImageCollapserData::SQRTSUM_NPIX_BEAM] = casacore::sum;
        _funcMap[(casacore::uInt)ImageCollapserData::STDDEV] = casacore::stddev;
        _funcMap[(casacore::uInt)ImageCollapserData::SUM] = casacore::sum;
        _funcMap[(casacore::uInt)ImageCollapserData::VARIANCE] = casacore::variance;
    }
    return _funcMap;
}

}
