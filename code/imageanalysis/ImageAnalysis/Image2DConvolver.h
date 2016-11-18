//# Image2DConvolver.h: 2D convolution of an image
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# $Id: Image2DConvolver.h 20229 2008-01-29 15:19:06Z gervandiepen $

#ifndef IMAGES_IMAGE2DCONVOLVER_H
#define IMAGES_IMAGE2DCONVOLVER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Array.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <casa/Quanta/Quantum.h>

namespace casacore{

template <class T> class ImageInterface;
template <class T> class Matrix;
template <class T> class Vector;
class String;
class IPosition;
class CoordinateSystem;
class ImageInfo;
class Unit;
class GaussianBeam;
}

namespace casa {


// <summary>
// This class does 2D convolution of an image by a functional form
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="casacore::ImageInterface">casacore::ImageInterface</linkto>
//   <li> <linkto class="casacore::Convolver">casacore::Convolver</linkto>
// </prerequisite>

// <etymology>
// This class handles 2D convolution of images 
// </etymology>

// <synopsis>
// This class convolves an image by a specified 2D function.
// </synopsis>

// <example>
// <srcBlock>
// </srcBlock>
// </example>

// <motivation>
// Convolution is a standard image processing requirement.  The
// class object has no state.
// The convolution is done via FFT.  Thus input pixels which
// are masked are set to 0 before the convolution.  The mask
// is transferred to the output image.  No additional scaling
// of the output image values is done.
// 
// </motivation>

// <todo asof="2001/08/28">
//   <li> 
// </todo>

template <class T> class Image2DConvolver : public ImageTask<T> {
public:

    const static casacore::String CLASS_NAME;

    Image2DConvolver() = delete;

    Image2DConvolver(
        const SPCIIT image, const casacore::Record *const &regionPtr,
        const casacore::String& mask, const casacore::String& outname, const casacore::Bool overwrite
    );
    
    Image2DConvolver(const Image2DConvolver<T> &other) = delete;

    ~Image2DConvolver() {}

    Image2DConvolver &operator=(const Image2DConvolver<T> &other) = delete;

    SPIIT convolve();

    // type is a string that starts with "g" (gaussian), "b" (boxcar), or "h" (hanning),
    // and is case insensitive
    void setKernel(
        const casacore::String& type, const casacore::Quantity& major, const casacore::Quantity& minor,
        const casacore::Quantity& pa
    );

    void setScale(casacore::Double d) { _scale = d; }

    void setAxes(const std::pair<casacore::uInt, casacore::uInt>& axes);

    void setTargetRes(casacore::Bool b) { _targetres = b; }

    casacore::String getClass() const { return CLASS_NAME; }

    // if true, do not log certain warning messages which would normally
    // be logged during convolution
    void setSuppressWarnings(casacore::Bool b) { _suppressWarnings = b; }

protected:

       CasacRegionManager::StokesControl _getStokesControl() const {
           return CasacRegionManager::USE_ALL_STOKES;
       }

    vector<casacore::Coordinate::Type> _getNecessaryCoordinates() const {
        return vector<casacore::Coordinate::Type>();
    }

    inline casacore::Bool _supportsMultipleRegions() const {return true;}

private:
    casacore::VectorKernel::KernelTypes _type;
    casacore::Double _scale;
    casacore::Quantity _major, _minor, _pa;
    casacore::IPosition _axes;
    casacore::Bool _targetres, _suppressWarnings;

    void _checkKernelParameters(
        casacore::VectorKernel::KernelTypes kernelType,
        const casacore::Vector<casacore::Quantity>& parameters
    ) const;

    void _convolve(
        casacore::LogIO& os, SPIIT imageOut, const casacore::ImageInterface<T>& imageIn,
        casacore::VectorKernel::KernelTypes kernelType
    ) const;

    // returns the value by which pixel values will be scaled
    T _dealWithRestoringBeam (
        casacore::LogIO& os, casacore::String& brightnessUnitOut, casacore::GaussianBeam& beamOut,
        const casacore::Array<T>& kernelArray, const T kernelVolume,
        const casacore::VectorKernel::KernelTypes kernelType,
        const casacore::Vector<casacore::Quantity>& parameters,
        const casacore::CoordinateSystem& cSys,
        const casacore::GaussianBeam& beamIn, const casacore::Unit& brightnessUnit,
        casacore::Bool emitMessage
    ) const;

    void _doMultipleBeams(
        LogIO& os, ImageInfo& iiOut, T& kernelVolume, SPIIT imageOut, String& brightnessUnitOut,
        GaussianBeam& beamOut, Double factor1, const ImageInterface<T>& imageIn,
        const vector<Quantity>& originalParms, vector<Quantity>& kernelParms,
        Array<T>& kernel, VectorKernel::KernelTypes kernelType, Bool logFactors,
        Double pixelArea
    ) const;

    T _fillKernel (
        casacore::Matrix<T>& kernelMatrix,
        casacore::VectorKernel::KernelTypes kernelType,
        const casacore::IPosition& kernelShape,
        const casacore::Vector<casacore::Double>& parameters
    ) const;

    void _fillGaussian(
        T& maxVal, T& volume, casacore::Matrix<T>& pixels,
        T height, T xCentre, T yCentre,
        T majorAxis, T ratio, T positionAngle
    ) const;

    T _makeKernel(
        casacore::Array<T>& kernel,
        casacore::VectorKernel::KernelTypes kernelType,
        const std::vector<casacore::Quantity>& parameters,
        const casacore::ImageInterface<T>& inImage
    ) const;

    casacore::IPosition _shapeOfKernel(
        const casacore::VectorKernel::KernelTypes kernelType,
        const casacore::Vector<casacore::Double>& parameters,
        const casacore::uInt ndim
    ) const;

    casacore::uInt _sizeOfGaussian(const casacore::Double width, const casacore::Double nSigma) const;

    std::vector<casacore::Quantity> _getConvolvingBeamForTargetResolution(
        const std::vector<casacore::Quantity>& targetBeamParms,
        const casacore::GaussianBeam& inputBeam
    ) const;
};

}

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/Image2DConvolver.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif



