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

#include <tr1/memory>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
template <class T> class ImageInterface;
template <class T> class Matrix;
template <class T> class Vector;
class String;
class IPosition;
class CoordinateSystem;
class ImageInfo;
class Unit;
class GaussianBeam;


// <summary>
// This class does 2D convolution of an image by a functional form
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="ImageInterface">ImageInterface</linkto>
//   <li> <linkto class="Convolver">Convolver</linkto>
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

	Image2DConvolver(
		const SPCIIT image, const Record *const &regionPtr,
	    const String& mask, const String& outname, const Bool overwrite
	);

	// Destructor
	~Image2DConvolver() {}

	// Convolve.   If the output image needs a mask and doesn't have one,
	// it will be given one if possible.  The miscInfo, imageInfo,
	// units and logger will be copied from the input to the output
	// unless you indicate not to (copyMiscellaneous).
	static void convolve(
		LogIO& os, std::tr1::shared_ptr<ImageInterface<T> > imageOut,
		const ImageInterface<T>& imageIn,
		const VectorKernel::KernelTypes kernelType,
		const IPosition& pixelAxes,
		const Vector<Quantity>& parameters,
		const Bool autoScale, const Double scale,
		const Bool copyMiscellaneous=True,
		const Bool targetres=False
	);

	SPIIT convolve();

	void setKernel(
		const String& type, const Quantity& major, const Quantity& minor,
		const Quantity& pa
	);

	void setScale(Double d) { _scale = d; }

	void setAxes(const std::pair<uInt, uInt>& axes);

	void setTargetRes(Bool b) { _targetres = b; }

	String getClass() const { const static String c = "Image2DConvolver"; return c; }

protected:

   	virtual CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_ALL_STOKES;
   	}

    virtual vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>();
    }

    virtual inline Bool _supportsMultipleRegions() const {return True;}

private:
    VectorKernel::KernelTypes _type;
    Double _scale;
    Quantity _major, _minor, _pa;
    IPosition _axes;
    Bool _targetres;

	// This class contains all static methods. Do not allow it to
	// be instantiated.
	// Constructor
	Image2DConvolver ();

	// Copy constructor.  Uses reference semantics.
	Image2DConvolver(const Image2DConvolver<T> &other);



	// Assignment operator. Uses reference semantics.
	Image2DConvolver &operator=(const Image2DConvolver<T> &other);


	static void _checkKernelParameters(
		VectorKernel::KernelTypes kernelType,
		const Vector<Quantity>& parameters
	);

	// returns the value by which pixel values will be scaled
	static T _dealWithRestoringBeam (
		LogIO& os, String& brightnessUnitOut, GaussianBeam& beamOut,
		const Array<T>& kernelArray, const T kernelVolume,
		const VectorKernel::KernelTypes kernelType,
		const Vector<Quantity>& parameters,
		const IPosition& axes, const CoordinateSystem& cSys,
		const GaussianBeam& beamIn, const Unit& brightnessUnit,
		const Bool autoscale, const Double scale, const Bool emitMessage
	);

	static T _fillKernel (
		Matrix<T>& kernelMatrix,
		VectorKernel::KernelTypes kernelType,
		const IPosition& kernelShape,
		const IPosition& axes,
		const Vector<Double>& parameters
	);

	static void _fillGaussian(
		T& maxVal, T& volume, Matrix<T>& pixels,
		T height, T xCentre, T yCentre,
		T majorAxis, T ratio, T positionAngle
	);

	static T _makeKernel(
		Array<T>& kernel,
		VectorKernel::KernelTypes kernelType,
		const Vector<Quantity>& parameters,
		const IPosition& axes,
		const ImageInterface<T>& inImage
	);

	static IPosition _shapeOfKernel(
		const VectorKernel::KernelTypes kernelType,
		const Vector<Double>& parameters,
		const uInt ndim, const IPosition& axes
	);

	static uInt _sizeOfGaussian(const Double width, const Double nSigma);

	static Vector<Quantity> _getConvolvingBeamForTargetResolution(
		const Vector<Quantity>& targetBeamParms,
		const GaussianBeam& inputBeam
	);
};

} //# NAMESPACE CASA - END

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/Image2DConvolver.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif



