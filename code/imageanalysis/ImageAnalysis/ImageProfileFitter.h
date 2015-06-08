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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGES_IMAGEPROFILEFITTER_H
#define IMAGES_IMAGEPROFILEFITTER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <components/SpectralComponents/GaussianMultipletSpectralElement.h>
#include <imageanalysis/ImageAnalysis/ImageFit1D.h>
#include <images/Images/TempImage.h>

#include <casa/namespace.h>

namespace casa {

class ProfileFitResults;

class ImageProfileFitter : public ImageTask<Float> {
	// <summary>
	// Top level interface for one-dimensional profile fits.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Fits one-dimensional profiles.
	// </etymology>

	// <synopsis>
	// Top level interface for one-dimensional profile fits.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageProfileFitter fitter(...)
	// fitter.fit()
	// </srcblock>
	// </example>

public:
	// constructor appropriate for API calls.
	// Parameters:
	// <src>image</src> - the input image in which to fit the models
	// <src>box</src> - A 2-D rectangular box in direction space in which to use pixels for the fitting, eg box=100,120,200,230
	// In cases where both box and region are specified, box, not region, is used.
	// <src>region</src> - Named region to use for fitting. "" => Don't use a named region
	// <src>regPtr</src> - Pointer to a region record. 0 => don't use a region record.
	// <src>chans</src> - Zero-based channel range on which to do the fit.
	// <src>stokes</src> - Stokes plane on which to do the fit. Only a single Stokes parameter can be
	// specified.
	// Only a maximum of one of region, regionPtr, or box/stokes/chans should be specified.
	// <src>mask</src> - Mask (as LEL) to use as a way to specify which pixels to use </src>
	// <src>axis</src> - axis along which to do the fits. If <0, use spectral axis, and if no spectral
	// axis, use zeroth axis.
	// <src>ngauss</src> number of single gaussians to fit.
	// <src>estimatesFilename</src> file containing initial estimates for single gaussians.
	// <src>spectralList</src> spectral list containing initial estimates of single gaussians. Do
	// not put a polynomial in here; set that with setPolyOrder().

	// Fit only Gaussian singlets and an optional polynomial. In this case, the
	// code guestimates initial estimates for the specified number of Gaussian
	// singlets. If you only wish to fit a polynomial, you must use this
	// constructor and you must set <src>ngauss</src> to zero. After construction,
	// you must call setPolyOrder().
	ImageProfileFitter(
		const SPCIIF image, const String& region,
		const Record *const &regionPtr, const String& box,
		const String& chans, const String& stokes, const String& mask,
		const Int axis, const uInt ngauss, Bool overwrite=False
	);

	// Fit only Gaussian singlets and an optional polynomial. In this case, the number
	// of Gaussian singlets is deduced from the specified estimates file.
	ImageProfileFitter(
		const SPCIIF image, const String& region,
		const Record *const &regionPtr, const String& box,
		const String& chans, const String& stokes, const String& mask,
		const Int axis, const String& estimatesFilename,
		Bool overwrite=False
	);

	// Fit any permitted combination of spectral components and an optional polynomial.
	// All components to be fit (except a possible polynomial) must be represented
	// with initial estimates in <src>spectralList</src>.
	ImageProfileFitter(
		const SPCIIF image, const String& region,
		const Record *const &regionPtr, const String& box,
		const String& chans, const String& stokes, const String& mask,
		const Int axis, const SpectralList& spectralList, Bool overwrite=False
	);

	// destructor
	~ImageProfileFitter();

	// Do the fit. If doDetailedResults is False, an empty Record is returned.
	Record fit(Bool doDetailedResults=True);

	// get the fit results. If fit was run with doDetailedResults=False, an empty Record is returned
	Record getResults() const;

    inline String getClass() const { return _class; };

    // set the order of a polynomial to be simultaneously fit.
    void setPolyOrder(Int p);

    // set whether to do a pixel by pixel fit.
    inline void setDoMultiFit(const Bool m) { _multiFit = m; }

    // set if results should be written to the logger
    inline void setLogResults(const Bool logResults) { _logResults = logResults; }

    // set minimum number of good points required to attempt a fit
    inline void setMinGoodPoints(const uInt mgp) {
    	ThrowIf(mgp == 0, "Number of good points has to be > 0");
    	_minGoodPoints = mgp;
    }

    // <group>
    // Solution images. Only written if _multifit is True
    // model image name
    inline void setModel(const String& model) { _model = model; }
    // residual image name
    inline void setResidual(const String& residual) { _residual = residual; }
    // gaussian amplitude image name
    inline void setAmpName(const String& s) { _ampName = s; }
    // gaussian amplitude error image name
    inline void setAmpErrName(const String& s) { _ampErrName = s; }
    // gaussian center image name
    inline void setCenterName(const String& s) { _centerName = s; }
    // gaussian center error image name
    inline void setCenterErrName(const String& s) { _centerErrName = s; }
    // gaussian fwhm image name
    inline void setFWHMName(const String& s) { _fwhmName = s; }
    // gaussian fwhm error image name
    inline void setFWHMErrName(const String& s) { _fwhmErrName = s; }
    // gaussian integral image name
    inline void setIntegralName(const String& s) { _integralName = s; }
    // gaussian integral error image name
    inline void setIntegralErrName(const String& s) { _integralErrName = s; }
    // </group>

    // set the name of the power logarithmic polynomial image.
    inline void setPLPName(const String& s) { _plpName = s; }

    // set the name of the power logarithmic polynomial image.
    inline void setPLPErrName(const String& s) { _plpErrName = s; }

    // set the name of the logarithmic transformed polynomial image.
    inline void setLTPName(const String& s) { _ltpName = s; }

    // set the name of the logarithmic transformed polynomial image.
    inline void setLTPErrName(const String& s) { _ltpErrName = s; }

    // set the range over which PFC amplitude solutions are valid
    void setGoodAmpRange(const Double min, const Double max);

    // set the range over which PFC center solutions are valid
    void setGoodCenterRange(const Double min, const Double max);

    // set the range over which PFC FWHM solutions are valid
    void setGoodFWHMRange(const Double min, const Double max);

    // <group>
    // set standard deviation image
    void setSigma(const Array<Float>& sigma);

    void setSigma(const ImageInterface<Float> *const &sigma);

    inline void setOutputSigmaImage(const String& s) { _sigmaName = s; }
    // </group>

    const Array<SHARED_PTR<ProfileFitResults> >& getFitters() const;

    //Converts a pixel value into a world value either in velocity, wavelength, or
    //frequency units.  If the tabular index >= 0, it uses the tabular index for conversion
    //with the specified MFrequency type, otherwise, it uses the spectral axis for
    //conversion.
    Double getWorldValue(
    	Double pixelVal, const IPosition& imPos, const String& units,
        Bool velocity, Bool wavelength, Int tabularIndex=-1,
        MFrequency::Types type=MFrequency::DEFAULT
    ) const;

    void setAbscissaDivisor(Double d);

    void setAbscissaDivisor(const Quantity& q);

    // for backward compatibility with ia.continuumsub. If no residual
    // image has been provided, a TempImage is created.
    void createResidualImage(Bool b) { _createResid = b; }

    SPIIF getResidual() const {
    	return _residImage;
    }

    // set the planes along the fit axis that are considered good for performing
    // the fits. An empty vector implies that all planes are considered "good".
    void setGoodPlanes(const std::set<uInt>& planes) { _goodPlanes = planes; }

protected:

    inline CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_FIRST_STOKES;
   	}

    inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>(0);
    }

    inline Bool _supportsMultipleRegions() const {return True;}

private:
	String _residual, _model, _xUnit,
		_centerName, _centerErrName, _fwhmName,
		_fwhmErrName, _ampName, _ampErrName,
		_integralName, _integralErrName, _plpName, _plpErrName,
		_ltpName, _ltpErrName, _sigmaName, _abscissaDivisorForDisplay;
	Bool _logfileAppend, _fitConverged, _fitDone, _multiFit,
		_deleteImageOnDestruct, _logResults, _isSpectralIndex,
		_createResid, _overwrite, _storeFits;
	Int _polyOrder, _fitAxis;
	uInt _nGaussSinglets, _nGaussMultiplets, _nLorentzSinglets,
		_nPLPCoeffs, _nLTPCoeffs, _minGoodPoints, _nProfiles, _nAttempted,
		_nSucceeded, _nConverged, _nValid;
	Array<SHARED_PTR<ProfileFitResults> > _fitters;
    // subimage contains the region of the original image
	// on which the fit is performed.
	SHARED_PTR<const SubImage<Float> > _subImage;
	Record _results;
	SpectralList _nonPolyEstimates;
	PtrHolder<std::pair<Double, Double> > _goodAmpRange, _goodCenterRange, _goodFWHMRange;
	Matrix<String> _worldCoords;
	SHARED_PTR<TempImage<Float> > _sigma;
	Double _abscissaDivisor;
	SPIIF _modelImage, _residImage;
	// planes along _fitAxis to use in fits, empty => use all planes
	// originally used to support continuum subtraction
	std::set<uInt> _goodPlanes;

	const static String _class;

    void _getOutputStruct(
        vector<OutputDestinationChecker::OutputStruct>& outputs
    );

    void _checkNGaussAndPolyOrder() const;

    void _finishConstruction();

    // moved from ImageAnalysis
    void _fitallprofiles();

    // Fit all profiles in image.  The output images must be already
    // created; if the pointer is 0, that image won't be filled.
    // The mask from the input image is transferred to the output
    // images.    If the weights image is pointer is non-zero, the
    // values from it will be used to weight the data points in the
    // fit.  You can fit some combination of gaussians and a polynomial
    // (-1 means no polynomial).  Initial estimates are not required.
    // Fits are done in image space to provide astronomer friendly results,
    // but pixel space is better for the fitter when fitting polynomials.
    // Thus, atm, callers should be aware that fitting polynomials may
    // fail even when the data lie exactly on a polynomial curve.
    // This will probably be fixed in the future by doing the fits
    // in pixel space here and requiring the caller to deal with converting
    // to something astronomer friendly if it so desires.

    void _fitProfiles(const Bool showProgress);

    Bool _inVelocitySpace() const;

    void _flagFitterIfNecessary(ImageFit1D<Float>& fitter) const;

    Bool _isPCFSolutionOK(const PCFSpectralElement *const &pcf) const;

    void _loopOverFits(
    	SPCIIF fitData, Bool showProgress,
    	SHARED_PTR<ProgressMeter> progressMeter, Bool checkMinPts,
    	const Array<Bool>& fitMask, ImageFit1D<Float>::AbcissaType abcissaType,
    	const IPosition& fitterShape, const IPosition& sliceShape,
    	const std::set<uInt> goodPlanes
    );

    void _setAbscissaDivisorIfNecessary(const Vector<Double>& abscissaValues);

    Bool _setFitterElements(
    	ImageFit1D<Float>& fitter, SpectralList& newEstimates,
    	const PtrHolder<const PolynomialSpectralElement>& polyEl,
    	const vector<IPosition>& goodPos,
    	const IPosition& fitterShape, const IPosition& curPos,
    	uInt nOrigComps
    ) const;

    void _updateModelAndResidual(
    	Bool fitOK,	const ImageFit1D<Float>& fitter,
    	const IPosition& sliceShape, const IPosition& curPos,
    	Lattice<Bool>* const &pFitMask, Lattice<Bool>* const &pResidMask
    ) const;

    // only implemented for the simplest cases so far
    uInt _nUnknowns() const;

};
}

#endif
