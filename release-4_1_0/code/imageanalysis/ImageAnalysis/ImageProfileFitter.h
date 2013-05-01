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

class ImageProfileFitter : public ImageTask {
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
	// <src>region</src> - Named region to use for fitting
	// <src>chans</src> - Zero-based channel range on which to do the fit.
	// <src>stokes</src> - Stokes plane on which to do the fit. Only a single Stokes parameter can be
	// specified.
	// <src>mask</src> - Mask (as LEL) to use as a way to specify which pixels to use </src>
	// <src>axis</src> - axis along which to do the fits. If <0, use spectral axis, and if no spectral
	// axis, use zeroth axis.
	// <src>ngauss</src> number of single gaussians to fit. Not used if <src>estimatesFile</src> or
	// <src>spectralList</src> is specified.
	// <src>estimatesFilename</src> file containing initial estimates for single gaussians.
	// <src>spectralList</src> spectral list containing initial estimates of single gaussians. Do
	// not put a polynomial in here; set that with setPolyOrder(). Only one of a non-empty <src>estimatesFilename</src>
	// or a non-empty <src>spectralList</src> can be specified.

	ImageProfileFitter(
		const ImageInterface<Float> *const &image, const String& region,
		const Record *const &regionPtr, const String& box,
		const String& chans, const String& stokes, const String& mask,
		const Int axis, const uInt ngauss, const String& estimatesFilename,
		const SpectralList& spectralList
	);

	// destructor
	~ImageProfileFitter();

	// Do the fit.
	Record fit();

	// get the fit results
	Record getResults() const;

    inline String getClass() const { return _class; };

   	inline CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_FIRST_STOKES;
   	}

    inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>(0);
    }

    // set the order of a polynomial to be simultaneously fit.
    inline void setPolyOrder(const Int p) { _polyOrder = p;}

    // set whether to do a pixel by pixel fit.
    inline void setDoMultiFit(const Bool m) { _multiFit = m; }

    // set if results should be written to the logger
    inline void setLogResults(const Bool logResults) { _logResults = logResults; }

    // set minimum number of good points required to attempt a fit
    inline void setMinGoodPoints(const uInt mgp) { _minGoodPoints = mgp; }

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

    void setGoodAmpRange(const Double min, const Double max);

    void setGoodCenterRange(const Double min, const Double max);

    void setGoodFWHMRange(const Double min, const Double max);

    // <group>
    // set standard deviation image
    void setSigma(const Array<Float>& sigma);

    void setSigma(const ImageInterface<Float> *const &sigma);

    inline void setOutputSigmaImage(const String& s) { _sigmaName = s; }
    // </group>


    const static String _CONVERGED;
    const static String _SUCCEEDED;
    const static String _VALID;

    const Array<ImageFit1D<Float> >& getFitters() const;
    // Returns the center, in pixels of the indexth fit.
    const Vector<Double> getPixelCenter( uint index ) const;
    //Converts a pixel value into a world value either in velocity, wavelength, or
    //frequency units.
    Double getWorldValue( double pixelVal, const IPosition& imPos, const String& units,
        bool velocity, bool wavelength) const;

private:

	String _residual, _model, _regionString, _xUnit,
		_centerName, _centerErrName, _fwhmName,
		_fwhmErrName, _ampName, _ampErrName,
		_integralName, _integralErrName, _sigmaName;
	Bool _logfileAppend, _fitConverged, _fitDone, _multiFit,
		_deleteImageOnDestruct, _logResults;
	Int _polyOrder, _fitAxis;
	uInt _nGaussSinglets, _nGaussMultiplets, _nLorentzSinglets,
		_minGoodPoints;
	Array<ImageFit1D<Float> > _fitters;
    // subimage contains the region of the original image
	// on which the fit is performed.
	SubImage<Float> _subImage;
	Record _results;
	SpectralList _nonPolyEstimates;
	Vector<Double> _goodAmpRange, _goodCenterRange, _goodFWHMRange;

	std::auto_ptr<TempImage<Float> > _sigma;

	const static String _class;

	const static uInt _nOthers;
	const static uInt _gsPlane;
	const static uInt _lsPlane;


	enum gaussSols {
	    AMP, CENTER, FWHM, INTEGRAL, AMPERR, CENTERERR,
	    FWHMERR, INTEGRALERR, NGSOLMATRICES
	};

    void _getOutputStruct(
        vector<ImageInputProcessor::OutputStruct>& outputs
    );

    void _checkNGaussAndPolyOrder() const;

    void _finishConstruction();

    void _setResults();

    String _radToRa(const Float ras) const;

    void _resultsToLog();

    String _getTag(const uInt i) const;

    String _elementToString(
    	const Double value, const Double error,
    	const String& unit
    ) const;

    String _pcfToString(
    	const PCFSpectralElement *const &pcf, const CoordinateSystem& csys,
    	const Vector<Double> world, const IPosition imPos, const Bool showTypeString=True,
    	const String& indent=""
    ) const;

    String _gaussianMultipletToString(
    	const GaussianMultipletSpectralElement& gm,
    	const CoordinateSystem& csys, const Vector<Double> world,
    	const IPosition imPos
    ) const;

    String _polynomialToString(
    	const PolynomialSpectralElement& poly, const CoordinateSystem& csys,
    	const Vector<Double> imPix, const Vector<Double> world
    ) const;

    static void _makeSolutionImage(
    	const String& name, const CoordinateSystem& csys,
    	const Array<Double>& values, const String& unit,
    	const Array<Bool>& mask
    );

    void _insertPCF(
    	vector<vector<Matrix<Double> > >& pcfMatrices, /* Bool& isSolutionSane,*/
    	const uInt idx, const PCFSpectralElement& pcf,
    	const uInt row, const uInt col, const IPosition& pos,
    	const Double increment/*, const uInt npix*/
    ) const;

    void _writeImages(
    	const CoordinateSystem& csys,
    	const Array<Bool>& mask, const String& yUnit
    ) const;

    /*
    // moved from ImageAnalysis
    void _fitProfile(
        const Bool fitIt=True
    );
    */

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

    void _fitProfiles(
    	std::auto_ptr<ImageInterface<Float> >& pFit,
    	std::auto_ptr<ImageInterface<Float> >& pResid,
        const Bool showProgress=False
    );

    Double _fitAxisIncrement() const;

    Double _centerWorld(
    	const PCFSpectralElement& solution, const IPosition& imPos
    ) const;

    Bool _inVelocitySpace() const;

    void _flagFitterIfNecessary(ImageFit1D<Float>& fitter) const;

    Bool _isPCFSolutionOK(const PCFSpectralElement *const &pcf) const;

    Vector< Vector<Double> > _pixelPositions;
};
}

#endif
