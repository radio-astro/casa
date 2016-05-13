//# ImageMoments.cc:  generate moments from an image
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001,2002,2003
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
//# $Id: ImageMoments.tcc 20652 2009-07-06 05:04:32Z Malte.Marquarding $
//   

#include <imageanalysis/ImageAnalysis/ImageMoments.h>

#include <imageanalysis/ImageAnalysis/CasaImageBeamSet.h>
#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>
#include <imageanalysis/ImageAnalysis/ImageHistograms.h>
#include <imageanalysis/ImageAnalysis/ImageMomentsProgress.h>
#include <imageanalysis/ImageAnalysis/MomentFit.h>
#include <imageanalysis/ImageAnalysis/MomentClip.h>
#include <imageanalysis/ImageAnalysis/MomentWindow.h>
#include <imageanalysis/ImageAnalysis/SepImageConvolver.h>

namespace casa {

template <class T> 
ImageMoments<T>::ImageMoments (
    const ImageInterface<T>& image, LogIO &os,
    Bool overWriteOutput, Bool showProgressU
) : MomentsBase<T>(os, overWriteOutput, showProgressU) {
    setNewImage(image);
}

template <class T>
ImageMoments<T>::ImageMoments(const ImageMoments<T> &other)
: MomentsBase<T>(other), _image(), _progressMonitor(nullptr) {
    operator=(other);
}

template <class T>
ImageMoments<T>::ImageMoments(ImageMoments<T> &other)
: MomentsBase<T>(other), _image(0), _progressMonitor(0) {
    operator=(other);
}

template <class T> 
ImageMoments<T>::~ImageMoments () {}

template <class T>
ImageMoments<T> &ImageMoments<T>::operator=(const ImageMoments<T> &other) {
    if (this != &other) {

        // Deal with image pointer

        _image.reset(other._image->cloneII());

        // Do the rest
        _progressMonitor = other._progressMonitor;
        os_p = other.os_p;
        showProgress_p = other.showProgress_p;
        momentAxis_p = other.momentAxis_p;
        worldMomentAxis_p = other.worldMomentAxis_p;
        momentAxisDefault_p = other.momentAxisDefault_p;
        kernelTypes_p = other.kernelTypes_p.copy();
        kernelWidths_p = other.kernelWidths_p.copy();
        moments_p = other.moments_p.copy();
        selectRange_p = other.selectRange_p.copy();
        smoothAxes_p = other.smoothAxes_p.copy();
        peakSNR_p = other.peakSNR_p;
        stdDeviation_p = other.stdDeviation_p;
        yMin_p = other.yMin_p;
        yMax_p = other.yMax_p;
        smoothOut_p = other.smoothOut_p;
        goodParameterStatus_p = other.goodParameterStatus_p;
        doWindow_p = other.doWindow_p;
        doFit_p = other.doFit_p;
        doAuto_p = other.doAuto_p;
        doSmooth_p = other.doSmooth_p;
        noInclude_p = other.noInclude_p;
        noExclude_p = other.noExclude_p;
        fixedYLimits_p = other.fixedYLimits_p;
        overWriteOutput_p = other.overWriteOutput_p;
        error_p = other.error_p;
        convertToVelocity_p = other.convertToVelocity_p;
        velocityType_p = other.velocityType_p;
    }
    return *this;
}


template <class T> 
Bool ImageMoments<T>::setNewImage(const ImageInterface<T>& image) {
    T *dummy = nullptr;
    DataType imageType = whatType(dummy);

    ThrowIf(
        imageType != TpFloat && imageType != TpDouble,
        "Moments can only be evaluated for Float or Double valued "
        "images"
    );
    // Make a clone of the image
    _image.reset(image.cloneII());
    return True;
}

template <class T>
Bool ImageMoments<T>::setMomentAxis(const Int momentAxisU) {
    if (!goodParameterStatus_p) {
        throw AipsError("Internal class status is bad");
    }
    momentAxis_p = momentAxisU;
    if (momentAxis_p == momentAxisDefault_p) {
        momentAxis_p = _image->coordinates().spectralAxisNumber();
        if (momentAxis_p == -1) {
            goodParameterStatus_p = False;
            throw AipsError(
                    "There is no spectral axis in this image -- specify the axis"
            );
        }
    }
    else {
        if (momentAxis_p < 0 || momentAxis_p > Int(_image->ndim()-1)) {
            goodParameterStatus_p = False;
            throw AipsError("Illegal moment axis; out of range");
        }
        if (_image->shape()(momentAxis_p) <= 0) {
            goodParameterStatus_p = False;
            throw AipsError("Illegal moment axis; it has no pixels");
        }
    }
    if (
            momentAxis_p == _image->coordinates().spectralAxisNumber()
            && _image->imageInfo().hasMultipleBeams()
    ) {
        GaussianBeam maxBeam = CasaImageBeamSet(_image->imageInfo().getBeamSet()).getCommonBeam();
        os_p << LogIO::NORMAL << "The input image has multiple beams so each "
                << "plane will be convolved to the largest beam size " << maxBeam
                << " prior to calculating moments" << LogIO::POST;
        Image2DConvolver<Float> convolver(_image, nullptr, "", "", False);
        auto dirAxes = _image->coordinates().directionAxesNumbers();
        convolver.setAxes(std::make_pair(dirAxes[0], dirAxes[1]));
        convolver.setKernel(
                "gaussian", maxBeam.getMajor(), maxBeam.getMinor(),
                maxBeam.getPA(True)
        );
        convolver.setScale(-1);
        convolver.setTargetRes(True);
        auto imageCopy = convolver.convolve();
        // replace the input image pointer with the convolved image pointer
        // and proceed using the convolved image as if it were the input
        // image
        _image = imageCopy;
    }
    worldMomentAxis_p = _image->coordinates().pixelAxisToWorldAxis(momentAxis_p);
    return True;
}

template <class T>
Bool ImageMoments<T>::setSmoothMethod(
    const Vector<Int>& smoothAxesU,
    const Vector<Int>& kernelTypesU,
    const Vector<Quantum<Double> >& kernelWidthsU
) {
    //
    // Assign the desired smoothing parameters.
    //
    if (!goodParameterStatus_p) {
        error_p = "Internal class status is bad";
        return False;
    }


    // First check the smoothing axes

    Int i;
    if (smoothAxesU.nelements() > 0) {
        smoothAxes_p = smoothAxesU;
        for (i=0; i<Int(smoothAxes_p.nelements()); i++) {
            if (smoothAxes_p(i) < 0 || smoothAxes_p(i) > Int(_image->ndim()-1)) {
                error_p = "Illegal smoothing axis given";
                goodParameterStatus_p = False;
                return False;
            }
        }
        doSmooth_p = True;
    }
    else {
        doSmooth_p = False;
        return True;
    }

    // Now check the smoothing types

    if (kernelTypesU.nelements() > 0) {
        kernelTypes_p = kernelTypesU;
        for (i=0; i<Int(kernelTypes_p.nelements()); i++) {
            if (kernelTypes_p(i) < 0 || kernelTypes_p(i) > VectorKernel::NKERNELS-1) {
                error_p = "Illegal smoothing kernel types given";
                goodParameterStatus_p = False;
                return False;
            }
        }
    }
    else {
        error_p = "Smoothing kernel types were not given";
        goodParameterStatus_p = False;
        return False;
    }


    // Check user gave us enough smoothing types

    if (smoothAxesU.nelements() != kernelTypes_p.nelements()) {
        error_p = "Different number of smoothing axes to kernel types";
        goodParameterStatus_p = False;
        return False;
    }


    // Now the desired smoothing kernels widths.  Allow for Hanning
    // to not be given as it is always 1/4, 1/2, 1/4

    kernelWidths_p.resize(smoothAxes_p.nelements());
    Int nK = kernelWidthsU.size();
    for (i=0; i<Int(smoothAxes_p.nelements()); i++) {
        if (kernelTypes_p(i) == VectorKernel::HANNING) {

            // For Hanning, width is always 3pix

            Quantity tmp(3.0, String("pix"));
            kernelWidths_p(i) = tmp;
        }
        else if (kernelTypes_p(i) == VectorKernel::BOXCAR) {

            // For box must be odd number greater than 1

            if (i > nK-1) {
                error_p = "Not enough smoothing widths given";
                goodParameterStatus_p = False;
                return False;
            }
            else {
                kernelWidths_p(i) = kernelWidthsU(i);
            }
        }
        else if (kernelTypes_p(i) == VectorKernel::GAUSSIAN) {
            if (i > nK-1) {
                error_p = "Not enough smoothing widths given";
                goodParameterStatus_p = False;
                return False;
            }
            else {
                kernelWidths_p(i) = kernelWidthsU(i);
            }
        }
        else {
            error_p = "Internal logic error";
            goodParameterStatus_p = False;
            return False;
        }
    }
    return True;
}

template <class T>
Bool ImageMoments<T>::setSmoothMethod(
    const Vector<Int>& smoothAxesU,
    const Vector<Int>& kernelTypesU,
    const Vector<Double> & kernelWidthsPix) {
    return MomentsBase<T>::setSmoothMethod(smoothAxesU, kernelTypesU, kernelWidthsPix);
}

template <class T>
vector<SHARED_PTR<MaskedLattice<T> > > ImageMoments<T>::createMoments(
    Bool doTemp, const String& outName, Bool removeAxis
) {
    LogOrigin myOrigin("ImageMoments", __func__);
    os_p << myOrigin;
    if (!goodParameterStatus_p) {
        // FIXME goodness, why are we waiting so long to throw an exception if this
        // is the case?
        throw AipsError("Internal status of class is bad.  You have ignored errors");
    }
    // Find spectral axis
    // use a copy of the coordinate system here since, if the image has multiple beams,
    // _image will change and hence a reference to its CoordinateSystem will disappear
    // causing a seg fault.
    CoordinateSystem cSys = _image->coordinates();
    Int spectralAxis = cSys.spectralAxisNumber(False);
    if (momentAxis_p == momentAxisDefault_p) {
        this->setMomentAxis(spectralAxis);
        if (_image->shape()(momentAxis_p) <= 1) {
            goodParameterStatus_p = False;
            throw AipsError("Illegal moment axis; it has only 1 pixel");
        }
        worldMomentAxis_p = cSys.pixelAxisToWorldAxis(momentAxis_p);
    }
    convertToVelocity_p = (momentAxis_p == spectralAxis)
        && (cSys.spectralCoordinate().restFrequency() > 0);
    os_p << myOrigin;
    String momentAxisUnits = cSys.worldAxisUnits()(worldMomentAxis_p);
    os_p << LogIO::NORMAL << endl << "Moment axis type is "
        << cSys.worldAxisNames()(worldMomentAxis_p) << LogIO::POST;
    // If the moment axis is a spectral axis, indicate we want to convert to velocity
    // Check the user's requests are allowed
    _checkMethod();
    // Check that input and output image names aren't the same.
    // if there is only one output image
    if (moments_p.nelements() == 1 && !doTemp) {
        if(!outName.empty() && (outName == _image->name())) {
            throw AipsError("Input image and output image have same name");
        }
    }
    auto smoothClipMethod = False;
    auto windowMethod = False;
    auto fitMethod = False;
    auto clipMethod = False;
    //auto doPlot = plotter_p.isAttached();
    if (doSmooth_p && !doWindow_p) {
        smoothClipMethod = True;
    }
    else if (doWindow_p) {
        windowMethod = True;
    }
    else if (doFit_p) {
        fitMethod = True;
    }
    else {
        clipMethod = True;
    }
    // We only smooth the image if we are doing the smooth/clip method
    // or possibly the interactive window method.  Note that the convolution
    // routines can only handle convolution when the image fits fully in core
    // at present.
    SPIIT smoothedImage;
    if (doSmooth_p) {
        smoothedImage = _smoothImage();
    }
    // Set output images shape and coordinates.
    IPosition outImageShape;
    const auto cSysOut = this->_makeOutputCoordinates(
        outImageShape, cSys, _image->shape(),
        momentAxis_p, removeAxis
    );
    auto nMoments = moments_p.nelements();
    // Resize the vector of pointers for output images
    vector<SHARED_PTR<MaskedLattice<T> > > outPt(nMoments);
    // Loop over desired output moments
    String suffix;
    Bool goodUnits;
    Bool giveMessage = True;
    const auto imageUnits = _image->units();
    for (uInt i=0; i<nMoments; ++i) {
        // Set moment image units and assign pointer to output moments array
        // Value of goodUnits is the same for each output moment image
        Unit momentUnits;
        goodUnits = this->_setOutThings(
            suffix, momentUnits, imageUnits, momentAxisUnits,
            moments_p(i), convertToVelocity_p
        );
        // Create output image(s).    Either PagedImage or TempImage
        SPIIT imgp;
        if (!doTemp) {
            const String in = _image->name(False);
            String outFileName;
            if (moments_p.size() == 1) {
                if (outName.empty()) {
                    outFileName = in + suffix;
                }
                else {
                    outFileName = outName;
                }
            }
            else {
                if (outName.empty()) {
                    outFileName = in + suffix;
                }
                else {
                    outFileName = outName + suffix;
                }
            }
            if (!overWriteOutput_p) {
                NewFile x;
                String error;
                if (!x.valueOK(outFileName, error)) {
                    throw AipsError(error);
                }
            }
            imgp.reset(new PagedImage<T>(outImageShape, cSysOut, outFileName));
            os_p << LogIO::NORMAL << "Created " << outFileName << LogIO::POST;
        }
        else {
            imgp.reset(new TempImage<T>(TiledShape(outImageShape), cSysOut));
            os_p << LogIO::NORMAL << "Created TempImage" << LogIO::POST;
        }
        ThrowIf (! imgp, "Failed to create output file");
        imgp->setMiscInfo(_image->miscInfo());
        imgp->setImageInfo(_image->imageInfo());
        imgp->appendLog(_image->logger());
        imgp->makeMask ("mask0", True, True);

        // Set output image units if possible

        if (goodUnits) {
            imgp->setUnits(momentUnits);
        }
        else {
            if (giveMessage) {
                os_p << LogIO::NORMAL
                        << "Could not determine the units of the moment image(s) so the units " << endl;
                os_p << "will be the same as those of the input image. This may not be very useful." << LogIO::POST;
                giveMessage = False;
            }
        }
        outPt[i] = imgp;
    }
    // If the user is using the automatic, non-fitting window method, they need
    // a good assessment of the noise.  The user can input that value, but if
    // they don't, we work it out here.
    T noise;
    if (stdDeviation_p <= T(0) && ( (doWindow_p && doAuto_p) || (doFit_p && !doWindow_p && doAuto_p) ) ) {
        if (smoothedImage) {
            os_p << LogIO::NORMAL << "Evaluating noise level from smoothed image" << LogIO::POST;
            _whatIsTheNoise(noise, *smoothedImage);
        }
        else {
            os_p << LogIO::NORMAL << "Evaluating noise level from input image" << LogIO::POST;
            _whatIsTheNoise (noise, *_image);
        }
        stdDeviation_p = noise;
    }

    // Create appropriate MomentCalculator object
    os_p << LogIO::NORMAL << "Begin computation of moments" << LogIO::POST;
    shared_ptr<MomentCalcBase<T> > momentCalculator;
    if (clipMethod || smoothClipMethod) {
        momentCalculator.reset(
            new MomentClip<T>(smoothedImage, *this, os_p, outPt.size())
        );
    }
    else if (windowMethod) {
        momentCalculator.reset(
            new MomentWindow<T>(smoothedImage, *this, os_p, outPt.size())
        );
    }
    else if (fitMethod) {
        momentCalculator.reset(
            new MomentFit<T>(*this, os_p, outPt.size())
        );
    }
    // Iterate optimally through the image, compute the moments, fill the output lattices
    unique_ptr<ImageMomentsProgress> pProgressMeter;
    if (showProgress_p) {
        pProgressMeter.reset(new ImageMomentsProgress());
        if (_progressMonitor) {
            pProgressMeter->setProgressMonitor(_progressMonitor);
        }
    }
    uInt n = outPt.size();
    PtrBlock<MaskedLattice<T>* > ptrBlock(n);
    for (uInt i=0; i<n; ++i) {
        ptrBlock[i] = outPt[i].get();
    }
    LatticeApply<T>::lineMultiApply(
        ptrBlock, *_image, *momentCalculator,
        momentAxis_p, pProgressMeter.get()
    );
    if (windowMethod || fitMethod) {
        if (momentCalculator->nFailedFits() != 0) {
            os_p << LogIO::NORMAL << "There were "
                <<  momentCalculator->nFailedFits()
                << " failed fits" << LogIO::POST;
        }
    }
    for (auto& p: outPt) {
        p->flush();
    }
    return outPt;
}

template <class T> SPIIT ImageMoments<T>::_smoothImage() {
    // Smooth image.   Input masked pixels are zerod before smoothing.
    // The output smoothed image is masked as well to reflect
    // the input mask.
    auto axMax = max(smoothAxes_p) + 1;
    ThrowIf(
        axMax > Int(_image->ndim()),
        "You have specified an illegal smoothing axis"
    );
    SPIIT smoothedImage;
    if (smoothOut_p.empty()) {
        smoothedImage.reset(
            new TempImage<T>(
                _image->shape(),
                _image->coordinates()
            )
        );
    }
    else {
        // This image has already been checked in setSmoothOutName
        // to not exist
        smoothedImage.reset(
            new PagedImage<T>(
                _image->shape(),
                _image->coordinates(), smoothOut_p
            )
        );
    }
    smoothedImage->setMiscInfo(_image->miscInfo());
    // Do the convolution.  Conserve flux.
    SepImageConvolver<T> sic(*_image, os_p, True);
    auto n = smoothAxes_p.size();
    for (uInt i=0; i<n; ++i) {
        VectorKernel::KernelTypes type = VectorKernel::KernelTypes(kernelTypes_p[i]);
        sic.setKernel(
            uInt(smoothAxes_p[i]), type, kernelWidths_p[i],
            True, False, 1.0
        );
    }
    sic.convolve(*smoothedImage);
    return smoothedImage;
}

template <class T> 
void ImageMoments<T>::_whatIsTheNoise (
    T& sigma, const ImageInterface<T>& image
) {
    // Determine the noise level in the image by first making a histogram of
    // the image, then fitting a Gaussian between the 25% levels to give sigma
    // Find a histogram of the image
    ImageHistograms<T> histo(image, False);
    const uInt nBins = 100;
    histo.setNBins(nBins);
    // It is safe to use Vector rather than Array because
    // we are binning the whole image and ImageHistograms will only resize
    // these Vectors to a 1-D shape
    Vector<T> values, counts;
    ThrowIf(
        ! histo.getHistograms(values, counts),
        "Unable to make histogram of image"
    );
    // Enter into a plot/fit loop
    auto binWidth = values(1) - values(0);
    T xMin, xMax, yMin, yMax;
    xMin = values(0) - binWidth;
    xMax = values(nBins-1) + binWidth;
    Float xMinF = Float(real(xMin));
    Float xMaxF = Float(real(xMax));
    LatticeStatsBase::stretchMinMax(xMinF, xMaxF);
    IPosition yMinPos(1), yMaxPos(1);
    minMax (yMin, yMax, yMinPos, yMaxPos, counts);
    Float yMaxF = Float(real(yMax));
    yMaxF += yMaxF/20;
    auto first = True;
    auto more = True;
    while (more) {
        Int iMin = 0;
        Int iMax = 0;
        if (first) {
            first = False;
            iMax = yMaxPos(0);
            uInt i;
            for (i=yMaxPos(0); i<nBins; i++) {
                if (counts(i) < yMax/4) {
                    iMax = i;
                    break;
                }
            }
            iMin = yMinPos(0);
            for (i=yMaxPos(0); i>0; i--) {
                if (counts(i) < yMax/4) {
                    iMin = i;
                    break;
                }
            }
            // Check range is sensible
            if (iMax <= iMin || abs(iMax-iMin) < 3) {
                os_p << LogIO::NORMAL << "The image histogram is strangely shaped, fitting to all bins" << LogIO::POST;
                iMin = 0;
                iMax = nBins-1;
            }
        }
        // Now generate the distribution we want to fit.  Normalize to
        // peak 1 to help fitter.
        const uInt nPts2 = iMax - iMin + 1;
        Vector<T> xx(nPts2);
        Vector<T> yy(nPts2);
        Int i;
        for (i=iMin; i<=iMax; i++) {
            xx(i-iMin) = values(i);
            yy(i-iMin) = counts(i)/yMax;
        }
        // Create fitter
        NonLinearFitLM<T> fitter;
        Gaussian1D<AutoDiff<T> > gauss;
        fitter.setFunction(gauss);
        // Initial guess
        Vector<T> v(3);
        v(0) = 1.0;                          // height
        v(1) = values(yMaxPos(0));           // position
        v(2) = nPts2*binWidth/2;             // width
        // Fit
        fitter.setParameterValues(v);
        fitter.setMaxIter(50);
        T tol = 0.001;
        fitter.setCriteria(tol);
        Vector<T> resultSigma(nPts2);
        resultSigma = 1;
        Vector<T> solution;
        Bool fail = False;
        try {
            solution = fitter.fit(xx, yy, resultSigma);
        }
        catch (const AipsError& x) {
            fail = True;
        }
        // Return values of fit
        if (! fail && fitter.converged()) {
            sigma = T(abs(solution(2)) / sqrt(2.0));
            os_p << LogIO::NORMAL
                    << "*** The fitted standard deviation of the noise is " << sigma
                    << endl << LogIO::POST;
        }
        else {
            os_p << LogIO::WARN << "The fit to determine the noise level failed." << endl;
            os_p << "Try inputting it directly" << endl;
        }
        // Another go
        more = False;
    }
}

template <class T>
void ImageMoments<T>::setProgressMonitor( ImageMomentsProgressMonitor* monitor ) {
    _progressMonitor = monitor;
}

}

