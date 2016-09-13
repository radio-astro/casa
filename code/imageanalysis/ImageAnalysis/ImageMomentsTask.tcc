#include <imageanalysis/ImageAnalysis/ImageMomentsTask.h>

#include <imageanalysis/ImageAnalysis/ImageMoments.h>

#include <algorithm>

namespace casa {

template<class T> ImageMomentsTask<T>::ImageMomentsTask(
	const SPCIIT image,	const casacore::Record *const region,
	const casacore::String& maskInp, const casacore::String& outname, casacore::Bool overwrite
) : ImageTask<T>(
	image, "", region, "", "", "", maskInp, outname, overwrite
), _moments(), _axis(-1), _methods(), _kernels(), _smoothAxes(),
    _kernelWidths(), _range(), _isIncludeRange(false),
    _removeAxis(true), _snr(-1), _stddev(0), _velocityType("RADIO"),
    _momentName(), _imageMomentsProgressMonitor(nullptr) {
    this->_construct();
}

template<class T> SPIIT ImageMomentsTask<T>::makeMoments() const {
    *this->_getLog() << casacore::LogOrigin(getClass(), __func__);
    // Note that the user may give the strings (method & kernels)
    // as either vectors of strings or one string with separators.
    // Hence the code below that deals with it.
    casacore::String tmpImageName;
    casacore::Record r;
    SPIIT pIm;
    try {
        SPCIIT x;
        if (this->_getImage()->imageType() != casacore::PagedImage<casacore::Float>::className()) {
            casacore::Path tmpImage = casacore::File::newUniqueName (".", "moments.scratch.image");
            tmpImageName = tmpImage.baseName();
            *this->_getLog() << casacore::LogIO::NORMAL
                << "Calculating moments of non-paged images can be notoriously slow, "
                << "so converting to a CASA temporary paged image named "
                << tmpImageName  << " first which will be written to the current "
                << "directory" << casacore::LogIO::POST;
            x = SubImageFactory<T>::createSubImageRO(
                *this->_getImage(), *this->_getRegion(), this->_getMask(),
                this->_getLog().get(), casacore::AxesSpecifier(),
                this->_getStretch()
            );
            x = SubImageFactory<T>::createImage(
                *x, tmpImageName, r, "", false,
                false, true, false
            );
        }
        else {
            x = SubImageFactory<T>::createSubImageRO(
                *this->_getImage(), *this->_getRegion(), this->_getMask(),
                this->_getLog().get(), casacore::AxesSpecifier(),
                this->_getStretch()
            );
        }
        // Create ImageMoments object
        ImageMoments<T> momentMaker(*x, *this->_getLog(), this->_getOverwrite(), true);
        if (_imageMomentsProgressMonitor) {
            momentMaker.setProgressMonitor( _imageMomentsProgressMonitor );
        }
        // Set which moments to output, NOTE we add one from what the UI
        // gives us.
        ThrowIf(
            ! momentMaker.setMoments(_moments + 1),
            momentMaker.errorMessage()
        );
        // Set moment axis
        if (_axis >= 0) {
            momentMaker.setMomentAxis(_axis);
        }
        if (x->imageInfo().hasMultipleBeams()) {
            const casacore::CoordinateSystem& csys = x->coordinates();
            if (csys.hasPolarizationCoordinate() && _axis == csys.polarizationAxisNumber()) {
                *this->_getLog() << casacore::LogIO::WARN << "This image has multiple beams and you are determining "
                    << " moments along the polarization axis. Interpret your results carefully"
                    << casacore::LogIO::POST;
            }
        }
        // Set moment methods
        if (! _methods.empty() && ! _methods[0].empty()) {
            casacore::String tmp;
            for (const auto& m : _methods) {
                tmp += m + " ";
            }
            auto intmethods = momentMaker.toMethodTypes(tmp);
            ThrowIf(
                ! momentMaker.setWinFitMethod(intmethods),
                momentMaker.errorMessage()
            );
        }
        // Set smoothing
        if (
            _kernels.size() >= 1 && _kernels[0] != "" && _smoothAxes.size() >= 1
            && _kernelWidths.size() >= 1
        ) {
            auto intkernels = casacore::VectorKernel::toKernelTypes(casacore::Vector<casacore::String>(_kernels));
            ThrowIf(
                ! momentMaker.setSmoothMethod(
                    casacore::Vector<casacore::Int>(_smoothAxes), intkernels,
                    casacore::Vector<casacore::Quantity>(_kernelWidths)
                ), momentMaker.errorMessage()
            );
        }
        // Set pixel include/exclude range
        if (! _range.empty()) {
            auto includepix = _isIncludeRange ? casacore::Vector<T>(_range) : casacore::Vector<T>();
            auto excludepix = _isIncludeRange ? casacore::Vector<T>() : casacore::Vector<T>(_range);
            momentMaker.setInExCludeRange(includepix, excludepix);
        }
        // Set SNR cutoff
        momentMaker.setSnr(_snr, _stddev);
        // Set velocity type
        if (! _velocityType.empty()) {
            casacore::MDoppler::Types velType;
            if (! casacore::MDoppler::getType(velType, _velocityType)) {
                *this->_getLog() << casacore::LogIO::WARN << "Illegal velocity type "
                    << _velocityType << ". Using RADIO" << casacore::LogIO::POST;
                velType = casacore::MDoppler::RADIO;
            }
            momentMaker.setVelocityType(velType);
        }
        // Set output names
        auto smoothout = this->_getOutname();
        ThrowIf(
            ! smoothout.empty() && ! momentMaker.setSmoothOutName(smoothout),
            momentMaker.errorMessage()
        );
        // If no file name given for one moment image, make TempImage.
        // Else casacore::PagedImage results
        casacore::Bool doTemp = _momentName.empty() && _moments.size() == 1;
        // Create moments
        auto images = momentMaker.createMoments(doTemp, _momentName, _removeAxis);
        for (auto& image: images) {
            // copy history from input to all created images
            SPIIT x = dynamic_pointer_cast<casacore::ImageInterface<T>>(image);
            this->_doHistory(x);
        }
        // Return handle of first image
        pIm = dynamic_pointer_cast<casacore::ImageInterface<T>>(images[0]);
    }
    catch (const casacore::AipsError& x) {
        _deleteTempImage(tmpImageName);
        RETHROW(x);
    }
    _deleteTempImage(tmpImageName);
    return pIm;
}

template<class T> void ImageMomentsTask<T>::_deleteTempImage(
    const casacore::String& tmpImage
) const {
    if (! tmpImage.empty()) {
        casacore::Directory dir(tmpImage);
        if (dir.exists()) {
            dir.removeRecursive(false);
        }
    }
}

template<class T> void ImageMomentsTask<T>::setAxis(casacore::Int axis) {
    ThrowIf(
        axis >= (casacore::Int)this->_getImage()->ndim(),
        "Axis " + casacore::String::toString(axis) + " doesn't exist."
        "Image has only "
        + casacore::String::toString(this->_getImage()->ndim()) + " dimensions"
    );
    _axis = axis;
}

template<class T> void ImageMomentsTask<T>::setIncludeExcludeRange(
    const vector<T>& range, casacore::Bool isInclude
) {
    ThrowIf(
       _range.size() > 2, "range cannot have more than 2 values"
    );
    _range = range;
    _isIncludeRange = isInclude;
}

template<class T> void ImageMomentsTask<T>::setMoments(
    const casacore::Vector<casacore::Int>& moments
) {
    casacore::uInt nMom = moments.nelements();
    ThrowIf(nMom == 0, "No moments requested");
    ThrowIf(
        nMom > MomentsBase<T>::NMOMENTS,
        "Too many moments specified"
    );
    _moments = moments;
}

template<class T> void ImageMomentsTask<T>::setSmoothAxes(
    const std::vector<casacore::uInt>& axes
) {
    auto mymax = *max_element(axes.begin(), axes.end());
    ThrowIf(
        mymax >= this->_getImage()->ndim(),
        "Axis " + casacore::String::toString(mymax) + " doesn't exist."
        "Image has only " + casacore::String::toString(this->_getImage()->ndim())
        + " dimensions"
    );
    _smoothAxes = axes;
}

}
