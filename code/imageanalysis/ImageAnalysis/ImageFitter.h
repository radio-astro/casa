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

#ifndef IMAGES_IMAGEFITTER_H
#define IMAGES_IMAGEFITTER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <components/ComponentModels/ComponentList.h>
#include <lattices/LatticeMath/Fit2D.h>

#include <imageanalysis/IO/ImageFitterResults.h>

namespace casa {

class ImageFitter : public ImageTask<Float> {
	// <summary>
	// Top level interface to ImageAnalysis::fitsky to handle inputs, bookkeeping etc and
	// ultimately call fitsky to do fitting
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Fits components to sources in images (ImageSourceComponentFitter was deemed to be to long
	// of a name)
	// </etymology>

	// <synopsis>
	// ImageFitter is the top level interface for fitting image source components. It handles most
	// of the inputs, bookkeeping etc. It can be instantiated and its one public method, fit,
	// run from either a C++ app or python.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageFitter fitter(...)
	// fitter.fit()
	// </srcblock>
	// </example>

public:
	/*
	enum CompListWriteControl {
		NO_WRITE,
		WRITE_NO_REPLACE,
		OVERWRITE
	};
	*/

	// constructor appropriate for API calls.
	// Parameters:
	// <ul>
	// <li>imagename - the name of the input image in which to fit the models</li>
	// <li>box - A 2-D rectangular box in which to use pixels for the fitting, eg box=100,120,200,230
	// In cases where both box and region are specified, box, not region, is used.</li>
	// <li>region - Named region to use for fitting</li>
	// <li>regionPtr - A pointer to a region. Note there are unfortunately several different types of
	// region records throughout CASA. In this case, it must be a Record produced by creating a
	// region via a RegionManager method.
	// <li>chanInp - Zero-based channel number on which to do the fit. Only a single channel can be
	// specified.</li>
	// <li>stokes - Stokes plane on which to do the fit. Only a single Stokes parameter can be
	// specified.</li>
	// <li> maskInp - Mask (as LEL) to use as a way to specify which pixels to use </li>
	// <li> includepix - Pixel value range to include in the fit. includepix and excludepix
	// cannot be specified simultaneously. </li>
	// <li> excludepix - Pixel value range to exclude from fit</li>
	// <li> residualInp - Name of residual image to save. Blank means do not save residual image</li>
	// <li> modelInp - Name of the model image to save. Blank means do not save model image</li>

	// use these constructors when you already have a pointer to a valid ImageInterface object

	ImageFitter(
		const SPCIIF image, const String& region,
		const Record *const &regionRec,
		const String& box="",
		const String& chanInp="", const String& stokes="",
		const String& maskInp="",
		const String& estiamtesFilename="",
		const String& newEstimatesInp="", const String& compListName=""
	);

	// destructor
	~ImageFitter();

	// Do the fit. If componentList is specified, store the fitted components in
	// that object. The first list in the returned pair represents the convolved components.
	// The second list represents the deconvolved components. If the image has no beam,
	// the two lists will be the same.
	std::pair<ComponentList, ComponentList> fit();

	void setWriteControl(ImageFitterResults::CompListWriteControl x) { _writeControl = x; }

	inline String getClass() const {return _class;}

	// Did the fit converge for the specified channel?
	// Throw AipsError if the fit has not yet been done.
	// <src>plane</src> is relative to the first plane in the image chosen to be fit.
	Bool converged(uInt plane) const;

	// Did the fit converge?
	// Throw AipsError if the fit has not yet been done.
	// <src>plane</src> is relative to the first plane in the image chosen to be fit.
	Vector<Bool> converged() const;

	// set the zero level estimate. Implies fitting of zero level should be done. Must be
	// called before fit() to have an effect.
	void setZeroLevelEstimate(Double estimate, Bool isFixed);

	// Unset zero level (resets to zero). Implies fitting of zero level should not be done.
	// Call prior to fit().
	void unsetZeroLevelEstimate();

	// get the fitted result and error. Throws
	// an exception if the zero level was not fit for.
	void getZeroLevelSolution(vector<Double>& solution, vector<Double>& error);

	// set rms level for calculating uncertainties. If not positive, an exception is thrown.
	void setRMS(const Quantity& rms);

	void setIncludePixelRange(const std::pair<Float, Float>& r) {
		_includePixelRange.reset(new std::pair<Float, Float>(r));
	}

	void setExcludePixelRange(const std::pair<Float, Float>& r) {
		_excludePixelRange.reset(new std::pair<Float, Float>(r));
	}

	// set the output model image name
	void setModel(const String& m) { _model = m; }

	// set the output residual image name
	void setResidual(const String& r) { _residual = r; }

	// set noise correlation beam FWHM
	void setNoiseFWHM(const Quantity& q);

	// in pixel widths
	void setNoiseFWHM(Double d);

	// clear noise FWHM, if the image has no beam, use the uncorrelated noise equations.
	// If the image has a beam(s) use the correlated noise equations with theta_N =
	// the geometric mean of the beam major and minor axes.
	void clearNoiseFWHM();

	// The Record holding all the output info
	Record getOutputRecord() const {return _output; }

protected:
    virtual inline Bool _supportsMultipleRegions() {return True;}

private:
	String _regionString, _residual, _model,
		_estimatesString, _newEstimatesFileName, _compListName, _bUnit;
	std::tr1::shared_ptr<std::pair<Float, Float> > _includePixelRange, _excludePixelRange;
	ComponentList _estimates, _curConvolvedList, _curDeconvolvedList;
	Vector<String> _fixed, _deconvolvedMessages;
	Bool _fitDone, _noBeam, _doZeroLevel, _zeroLevelIsFixed, _correlatedNoise, _useBeamForNoise;
	Vector<Bool> _fitConverged;
	Vector<Quantity> _peakIntensities, _peakIntensityErrors, _fluxDensityErrors,
		_fluxDensities, _majorAxes, _majorAxisErrors, _minorAxes, _minorAxisErrors,
		_positionAngles, _positionAngleErrors;
	vector<Quantity> _allConvolvedPeakIntensities, _allConvolvedPeakIntensityErrors, _allSums;
	vector<GaussianBeam> _allBeams;
	vector<Double> _allBeamsPix, _allBeamsSter;
	Record _residStats, inputStats, _output;
	Double _rms;
	String _kludgedStokes;
	ImageFitterResults::CompListWriteControl _writeControl;
	Vector<uInt> _chanVec;
	uInt _curChan;
	Double _zeroLevelOffsetEstimate;
	vector<Double> _zeroLevelOffsetSolution, _zeroLevelOffsetError;
	Int _stokesPixNumber, _chanPixNumber;
	ImageFitterResults _results;
	std::auto_ptr<Quantity> _noiseFWHM;
	Quantity _pixWidth;

	const static String _class;

	void _fitLoop(
		Bool& anyConverged, ComponentList& convolvedList,
		ComponentList& deconvolvedList, SPIIF templateImage,
		SPIIF residualImage, SPIIF modelImage,
		/*LCMask& completePixelMask, */ String& resultsString
	);

	vector<OutputDestinationChecker::OutputStruct> _getOutputStruct();

	vector<Coordinate::Type> _getNecessaryCoordinates() const;

	CasacRegionManager::StokesControl _getStokesControl() const;

	void _finishConstruction(const String& estimatesFilename);

	//String _resultsHeader() const;

	// summarize the results in a nicely formatted string
	String _resultsToString(uInt nPixels) const;

	//summarize the size details in a nicely formatted string
	String _sizeToString(const uInt compNumber) const;

	String _spectrumToString(uInt compNumber) const;

	void _setDeconvolvedSizes();

	void _getStandardDeviations(Double& inputStdDev, Double& residStdDev) const;

	void _getRMSs(Double& inputRMS, Double& residRMS) const;

	Double _getStatistic(const String& type, const uInt index, const Record& stats) const;

	String _statisticsToString() const;

	SPIIF _createImageTemplate() const;

	void _writeCompList(ComponentList& list) const;

	void _setIncludeExclude(
	    Fit2D& fitter
	) const;

	void _fitsky(
		Fit2D& fitter, Array<Float>& pixels,
	    Array<Bool>& pixelMask, Bool& converged,
	    Double& zeroLevelOffsetSolution,
	    Double& zeroLevelOffsetError,
	    std::pair<Int, Int>& pixelOffsets,
		const Vector<String>& models, Bool fitIt,
		Bool deconvolveIt,
		Double zeroLevelEstimate
	);

	Vector<Double> _singleParameterEstimate(
		Fit2D& fitter, Fit2D::Types model,
		const MaskedArray<Float>& pixels, Float minVal,
		Float maxVal, const IPosition& minPos, const IPosition& maxPos
	) const;

	ComponentType::Shape _convertModelType(Fit2D::Types typeIn) const;

	void _fitskyExtractBeam(
		Vector<Double>& parameters, const ImageInfo& imageInfo,
		const Bool xIsLong, const CoordinateSystem& cSys
	) const;

	void _encodeSkyComponentError(
		SkyComponent& sky, Double facToJy, const ImageInterface<Float>& subIm,
		const Vector<Double>& parameters, const Vector<Double>& errors,
		Stokes::StokesTypes stokes, Bool xIsLong
	) const;

	void _doConverged(
		ComponentList& convolvedList, ComponentList& deconvolvedList,
		Double& zeroLevelOffsetEstimate, std::pair<Int, Int>& pixelOffsets,
		SPIIF& residualImage, SPIIF& modelImage,
		std::tr1::shared_ptr<TempImage<Float> >& tImage,
		std::tr1::shared_ptr<ArrayLattice<Bool> >& initMask,
		Double zeroLevelOffsetSolution, Double zeroLevelOffsetError,
		Bool hasSpectralAxis, Int spectralAxisNumber, Bool outputImages, const IPosition& planeShape,
		const Array<Float>& pixels, const Array<Bool>& pixelMask, const Fit2D& fitter,
		SPIIF templateImage
	);

	Quantity _pixelWidth();

	void _calculateErrors();

	Double _getRMS() const;

	Double _correlatedOverallSNR(
		uInt comp, Double a, Double b, Double signalToNoise
	) const;

	GaussianBeam _getCurrentBeam() const;

	void _createOutputRecord(const ComponentList& convolved, const ComponentList& decon);

	void _setSum(const SkyComponent& comp, const SubImage<Float>& im);

	void _setBeam(GaussianBeam& beam, uInt ngauss);
};
}

#endif
