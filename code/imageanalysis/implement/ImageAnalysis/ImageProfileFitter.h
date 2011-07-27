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

#include <images/Images/ImageFit1D.h>

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
	// <src>multiFit</src> True means do fit on each pixel in the specified region or box. False means
	// average together the pixels in the specified box or region and do a single fit to that average profile.
	// <src>residual</src> - Name of residual image to save. Blank means do not save residual image.
	// <src>model</src> - Name of the model image to save. Blank means do not save model image.
	// The output solution images are only written if multiFit is true.

	ImageProfileFitter(
		const ImageInterface<Float> *const &image, const String& region,
		const Record *const &regionPtr, const String& box,
		const String& chans, const String& stokes, const String& mask,
		const Int axis, const Bool multiFit, const String& residual,
		const String& model, const uInt ngauss, const Int polyOrder,
		const String& ampName = "", const String& ampErrName = "",
		const String& centerName = "", const String& centerErrName = "",
		const String& fwhmName = "", const String& fwhmErrName = ""
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

private:

	String _residual, _model, _regionString, _xUnit,
		_centerName, _centerErrName, _fwhmName,
		_fwhmErrName, _ampName, _ampErrName;
	Bool _logfileAppend, _fitConverged, _fitDone, _multiFit, _deleteImageOnDestruct;
	Int _polyOrder, _fitAxis;
	uInt _ngauss;
	Vector<ImageFit1D<Float> > _fitters;
    // subimage contains the region of the original image
	// on which the fit is performed.
	SubImage<Float> _subImage;
	Record _results;

	const static String _class;

	// disallow default constructor
	ImageProfileFitter();

    void _getOutputStruct(
        vector<ImageInputProcessor::OutputStruct>& outputs
    );

    void _checkNGaussAndPolyOrder() const;

    void _finishConstruction();

    void _setResults();

    String _radToRa(const Float ras) const;
    String _resultsToString() const;

    String _elementToString(
    	const Double value, const Double error,
    	const String& unit
    ) const;

    String _gaussianToString(
    	const SpectralElement& gauss, const CoordinateSystem& csys,
    	const Vector<Double> world
    ) const;

    String _polynomialToString(
    	const SpectralElement& poly, const CoordinateSystem& csys,
    	const Vector<Double> imPix, const Vector<Double> world
    ) const;

    static void _makeSolutionImage(
    	const String& name, const IPosition& shape, const CoordinateSystem& csys,
    	const Vector<Double>& values, const String& unit
    );

    // moved from ImageAnalysis
    ImageFit1D<Float> _fitProfile(
        const Record& estimate, const Bool fitIt=True,
        const String weightsImageName=""
    );

    // moved from ImageAnalysis
    Vector<ImageFit1D<Float> > _fitallprofiles(
        const String& weightsImageName = ""
    );

};
}

#endif
