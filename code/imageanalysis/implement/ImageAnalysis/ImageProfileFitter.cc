
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

#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>

#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Precision.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/Combinatorics.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/IO/ProfileFitterEstimatesFileParser.h>

#include <memory>

namespace casa {

const Double ImageProfileFitter::integralConst = casa::sqrt(C::pi/4/casa::log(2));

const String ImageProfileFitter::_class = "ImageProfileFitter";

ImageProfileFitter::ImageProfileFitter(
	const ImageInterface<Float> *const &image, const String& region,
	const Record *const &regionPtr,	const String& box,
	const String& chans, const String& stokes,
	const String& mask, const Int axis, const Bool multiFit,
	const String& residual, const String& model, const uInt ngauss,
	const Int polyOrder, const String& estimatesFilename, const String& ampName,
	const String& ampErrName, const String& centerName,
	const String& centerErrName, const String& fwhmName,
	const String& fwhmErrName, const String& integralName,
	const String& integralErrName, const uInt minGoodPoints
) : ImageTask(
		image, region, regionPtr, box, chans, stokes,
		mask, "", False
	),
	_residual(residual), _model(model), _xUnit(""),
	_centerName(centerName), _centerErrName(centerErrName),
	_fwhmName(fwhmName), _fwhmErrName(fwhmErrName),
	_ampName(ampName), _ampErrName(ampErrName),
	_integralName(integralName), _integralErrName(integralErrName),
	_multiFit(multiFit), _deleteImageOnDestruct(False), _logResults(True),
	_polyOrder(polyOrder), _fitAxis(axis), _ngauss(ngauss),
	_minGoodPoints(minGoodPoints), _results(Record()),
	_estimates(SpectralList()) {
    _checkNGaussAndPolyOrder();
    if (! estimatesFilename.empty()) {
    	ProfileFitterEstimatesFileParser parser(estimatesFilename);
    	_estimates = parser.getEstimates();
    	_ngauss = _estimates.nelements();
    	*_getLog() << LogOrigin(_class, __FUNCTION__);
    	*_getLog() << LogIO::NORMAL << "Number of gaussians to fit found to be "
    		<< _ngauss << " in estimates file " << estimatesFilename
    		<< LogIO::POST;
    }
    _construct();
    _finishConstruction();
}

ImageProfileFitter::~ImageProfileFitter() {}

Record ImageProfileFitter::fit() {
    LogOrigin logOrigin(_class, __FUNCTION__);
    *_getLog() << logOrigin;
    {
    	std::auto_ptr<ImageInterface<Float> > clone(
    		_getImage()->cloneII()
    	);
    	std::auto_ptr<Record> regionClone(
    		dynamic_cast<Record *>(_getRegion()->clone())
    	);
    	_subImage = SubImage<Float>::createSubImage(
    		*clone,
    		*(ImageRegion::tweakedRegionRecord(regionClone.get())),
    		_getMask(), 0, False, AxesSpecifier(), _getStretch()
    	);
    }
	String weightsImageName = "";
	try {
		if (_multiFit) {
			// FIXME need to be able to specify the weights image
			_fitters = _fitallprofiles(weightsImageName);
		    *_getLog() << logOrigin;
		}
		else {
			ImageFit1D<Float> fitter = _fitProfile();
		    *_getLog() << logOrigin;
			IPosition axes(1, _fitAxis);
			ImageCollapser collapser(
				&_subImage, axes, True,
				ImageCollapser::MEAN, "", True
			);
			std::auto_ptr<ImageInterface<Float> > x(
				collapser.collapse(True)
			);
			_subImage = SubImage<Float>::createSubImage(
				*x, Record(), "", _getLog().get(),
				False, AxesSpecifier(), False
			);
			_fitters.resize(IPosition(1,1));
			_fitters(IPosition(1, 0)) = fitter;
		}
	}
	catch (AipsError exc) {
		*_getLog() << "Exception during fit: " << exc.getMesg()
			<< LogIO::EXCEPTION;
	}
	_setResults();
    *_getLog() << logOrigin;
    if (_logResults) {
    	_resultsToLog();
    }
	return _results;
}

Record ImageProfileFitter::getResults() const {
	return _results;
}

void ImageProfileFitter::_getOutputStruct(
    vector<ImageInputProcessor::OutputStruct>& outputs
) {
	outputs.resize(0);
    if (! _model.empty()) {
    	ImageInputProcessor::OutputStruct modelImage;
    	modelImage.label = "model image";
    	modelImage.outputFile = &_model;
    	modelImage.required = True;
    	modelImage.replaceable = False;
    	outputs.push_back(modelImage);
    }
    if (! _residual.empty()) {
    	ImageInputProcessor::OutputStruct residImage;
    	residImage.label = "residual image";
    	residImage.outputFile = &_residual;
    	residImage.required = True;
    	residImage.replaceable = False;
    	outputs.push_back(residImage);
    }
}

void ImageProfileFitter::_checkNGaussAndPolyOrder() const {
	LogOrigin logOrigin(_class, __FUNCTION__);
	if (_ngauss == 0 && _polyOrder < 0) {
		*_getLog() << "Number of gaussians is 0 and polynomial order is less than zero. "
			<< "According to these inputs there is nothing to fit."
			<< LogIO::EXCEPTION;
	}
}

void ImageProfileFitter::_finishConstruction() {
    if (_fitAxis >= (Int)_getImage()->ndim()) {
    	*_getLog() << "Specified fit axis " << _fitAxis
    		<< " must be less than the number of image axes ("
    		<< _getImage()->ndim() << ")" << LogIO::EXCEPTION;
    }
    if (_fitAxis < 0) {
		if (! _getImage()->coordinates().hasSpectralAxis()) {
			_fitAxis = 0;
			*_getLog() << LogIO::WARN << "No spectral coordinate found in image, "
                << "using axis 0 as fit axis" << LogIO::POST;
		}
		else {
            _fitAxis = _getImage()->coordinates().spectralAxisNumber();
			*_getLog() << LogIO::NORMAL << "Using spectral axis (axis " << _fitAxis
				<< ") as fit axis" << LogIO::POST;
		}
	}
}

void ImageProfileFitter::_setResults() {
    Double fNAN = casa::doubleNaN();
	uInt nComps = _polyOrder < 0 ? _ngauss : _ngauss + 1;
	Array<Bool> attemptedArr(IPosition(1, _fitters.size()), False);
	Array<Bool> convergedArr(IPosition(1, _fitters.size()), False);
	Array<Int> niterArr(IPosition(1, _fitters.size()), -1);
    Matrix<Double> centerMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> fwhmMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> ampMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> integralMat(_fitters.size(), nComps, fNAN);

	Matrix<Double> centerErrMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> fwhmErrMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> ampErrMat(_fitters.size(), nComps, fNAN);
	Matrix<Double> integralErrMat(_fitters.size(), nComps, fNAN);

	Matrix<String> typeMat(_fitters.size(), nComps, "UNDEF");
	Array<Bool> mask(IPosition(1, _fitters.size()), False);
	Array<Int> nCompArr(IPosition(1, _fitters.size()), -1);
	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters.begin();
	SpectralList solutions;
	uInt count = 0;

	const CoordinateSystem subimCsys = _subImage.coordinates();
	Vector<Double> world;
	Double increment = fabs(_fitAxisIncrement());

	for (
		inIter.reset();
		! inIter.atEnd() && fitter != _fitters.end();
		inIter++, fitter++, count++
	) {
		IPosition idx(1, count);
		attemptedArr(idx) = fitter->getList().nelements() > 0;
		convergedArr(idx) = attemptedArr(idx) && fitter->converged();
		if (fitter->converged()) {
			IPosition subimPos = inIter.position();
			mask(idx) = anyTrue(inIter.getMask());
			niterArr(idx) = (Int)fitter->getNumberIterations();
			nCompArr(idx) = (Int)fitter->getList().nelements();
			solutions = fitter->getList();
			for (uInt i=0; i<solutions.nelements(); i++) {
				typeMat(count, i) = SpectralElement::fromType(solutions[i]->getType());
				if (solutions[i]->getType() == SpectralElement::GAUSSIAN) {
					const GaussianSpectralElement *g = dynamic_cast<
							const GaussianSpectralElement*
						>(solutions[i]);
					centerMat(count, i) = _centerWorld(*g, subimPos);
					fwhmMat(count, i) = g->getFWHM() * increment;
					ampMat(count, i) = g->getAmpl();
					centerErrMat(count, i) = g->getCenterErr() * increment;
					fwhmErrMat(count, i) = g->getFWHMErr() * increment;
					ampErrMat(count, i) = g->getAmplErr();
					integralMat(count, i) = integralConst * ampMat(count, i) * fwhmMat(count, i);
					Double ampFErr = ampErrMat(count, i)/ampMat(count, i);
					Double fwhmFErr = fwhmErrMat(count, i)/fwhmMat(count, i);
					integralErrMat(count, i) = integralMat(count, i)
						* sqrt(ampFErr*ampFErr + fwhmFErr*fwhmFErr);
				}
			}
		}
	}
	Bool someConverged = anyTrue(convergedArr);
	String key;
	IPosition axes(1, _fitAxis);
	ImageCollapser collapser(
		&_subImage, axes, False, ImageCollapser::ZERO, String(""), False
	);
	std::auto_ptr<TempImage<Float> > myTemplate(
		static_cast<TempImage<Float>* >(collapser.collapse(True))
	);
	IPosition shape = myTemplate->shape();
	_results.define("attempted", attemptedArr.reform(shape));
	_results.define("converged", convergedArr.reform(shape));
	_results.define("niter", niterArr.reform(shape));
	_results.define("ncomps", nCompArr.reform(shape));
	_results.define("xUnit", _xUnit);
	_results.define("yUnit", _getImage()->units().getName());
	CoordinateSystem csys = myTemplate->coordinates();
	myTemplate.reset(0);
	uInt gaussCount = 0;
	if (
		! _multiFit && (
			! _centerName.empty() || ! _centerErrName.empty()
			|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
			|| ! _ampName.empty() || ! _ampErrName.empty()
			|| ! _integralName.empty() || ! _integralErrName.empty()
		)
	) {
		*_getLog() << LogIO::WARN << "This was not a multi-pixel fit request so solution "
			<< "images will not be written" << LogIO::POST;
	}
	if (
		_multiFit && ! someConverged && (
			! _centerName.empty() || ! _centerErrName.empty()
			|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
			|| ! _ampName.empty() || ! _ampErrName.empty()
			|| ! _integralName.empty() || ! _integralErrName.empty()
		)
	) {
		*_getLog() << LogIO::WARN << "No solutions converged, solution images will not be written"
			<< LogIO::POST;
	}
	// because resize with copyValues=True doesn't give the expected result
	Array<Bool> fMask = mask.reform(shape);
	for (uInt i=0; i<nComps; i++) {
		String num = String::toString(i);
		key = "center" + num;
		_results.define(key, centerMat.column(i).reform(shape));
		key = "fwhm" + num;
		_results.define(key, fwhmMat.column(i).reform(shape));
		key = "amp" + num;
		_results.define(key, ampMat.column(i).reform(shape));
		key = "integral" + num;
		_results.define(key, integralMat.column(i).reform(shape));
		key = "centerErr" + num;
		_results.define(key, centerErrMat.column(i).reform(shape));
		key = "fwhmErr" + num;
		_results.define(key, fwhmErrMat.column(i).reform(shape));
		key = "ampErr" + num;
		_results.define(key, ampErrMat.column(i).reform(shape));
		key = "integralErr" + num;
		_results.define(key, integralErrMat.column(i).reform(shape));
		key = "type" + num;
		_results.define(key, typeMat.column(i).reform(shape));
		if (
			_multiFit && someConverged
			&& solutions[i]->getType() == SpectralElement::GAUSSIAN
		) {
			String gnum = String::toString(gaussCount);
			String mUnit = _xUnit;
			map<String, String> mymap;
			mymap["center"] = _centerName;
			mymap["centerErr"] = _centerErrName;
			mymap["fwhm"] = _fwhmName;
			mymap["fwhmErr"] = _fwhmErrName;
			mymap["amp"] = _ampName;
			mymap["ampErr"] = _ampErrName;
			mymap["integral"] = _integralName;
			mymap["integralErr"] = _integralErrName;
			for (
				map<String, String>::const_iterator iter=mymap.begin();
				iter!=mymap.end(); iter++
			) {
				if (! iter->second.empty()) {
					_makeSolutionImage(
						iter->second + "_" + gnum, csys,
						_results.asArrayDouble(iter->first + num),
						mUnit, fMask
					);
				}
			}
			gaussCount++;
		}
	}
}

Bool ImageProfileFitter::_inVelocitySpace() const {
	return _fitAxis == _subImage.coordinates().spectralAxisNumber()
		&& Quantity(1, _xUnit).isConform("m/s");
}

Double ImageProfileFitter::_fitAxisIncrement() const {
	if (_inVelocitySpace()) {
		Vector<Double> pixels(2);
		pixels[0] = 0;
		pixels[1] = 1;
		Vector<Double> velocities(2);
		_subImage.coordinates().spectralCoordinate().pixelToVelocity(
			velocities, pixels
		);
		return velocities[1] - velocities[0];
	}
	else {
		return _subImage.coordinates().increment()[_fitAxis];
	}

}

Double ImageProfileFitter::_centerWorld(
    const GaussianSpectralElement& solution, const IPosition& imPos
) const {
	Vector<Double> pixel(imPos.size());
	for (uInt i=0; i<pixel.size(); i++) {
		pixel[i] = imPos[i];
	}
	Vector<Double> world(pixel.size());
	// in pixels here
	pixel[_fitAxis] = solution.getCenter();
	_subImage.coordinates().toWorld(world, pixel);
	if (_inVelocitySpace()) {
		Double velocity;
		_subImage.coordinates().spectralCoordinate().frequencyToVelocity(velocity, world(_fitAxis));
		return velocity;
	}
	else {
		return world(_fitAxis);
	}
}


String ImageProfileFitter::_radToRa(Float ras) const {
	Int h, m;
	Float rah = ras * 12 / C::pi;
	h = (int)floor(rah);
	Float ram = (rah - h) * 60;
	m = (int)floor(ram);
	ras = (ram - m) * 60;
	ras = (int)(1000 * ras) / 1000.;
	String raStr = (h < 10) ? "0" : "";
	raStr.append(String::toString(h)).append(String(":"))
        .append(String((m < 10) ? "0" : ""))
        .append(String::toString(m)).append(String(":")) 
        .append(String((ras < 10) ? "0" : ""))
        .append(String::toString(ras));
	return raStr;
}

void ImageProfileFitter::_resultsToLog() const {
	ostringstream summary;
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << _summaryHeader();
	summary << "       --- polynomial order:    " << _polyOrder << endl;
	summary << "       --- number of Gaussians: " << _ngauss << endl;

	if (_multiFit) {
		summary << "       --- Multiple profiles fit, one per pixel over selected region" << endl;
	}
	else {
		summary << "       --- One profile fit, averaged over several pixels if necessary" << endl;
	}
	*_getLog() << LogIO::NORMAL << summary.str() << LogIO::POST;
	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	CoordinateSystem csysSub = _subImage.coordinates();
	CoordinateSystem csys = _getImage()->coordinates();
	Vector<Double> worldStart;
	if (! csysSub.toWorld(worldStart, inIter.position())) {
		*_getLog() << csysSub.errorMessage() << LogIO::EXCEPTION;
	}
	CoordinateSystem csysIm = _getImage()->coordinates();
	Vector<Double> pixStart;
	if (! csysIm.toPixel(pixStart, worldStart)) {
		*_getLog() << csysIm.errorMessage() << LogIO::EXCEPTION;
	}
	if (_multiFit) {
		for (uInt i=0; i<pixStart.size(); i++) {
			pixStart[i] = (Int)std::floor( pixStart[i] + 0.5);
		}
	}
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters.begin();
	Vector<String> axesNames = csysSub.worldAxisNames();
	Vector<Double> imPix(pixStart.size());
	Vector<Double> world;
	IPosition subimPos;
	SpectralList solutions;
	String axisUnit = csysIm.worldAxisUnits()[_fitAxis];
	Int pixPrecision = _multiFit ? 0 : 3;
	Int basePrecision = summary.precision(1);
	summary.precision(basePrecision);
	for (
		Vector<String>::iterator iter = axesNames.begin();
		iter != axesNames.end(); iter++
	) {
		iter->upcase();
	}
	for (
		inIter.reset();
		! inIter.atEnd() && fitter != _fitters.end();
		inIter++, fitter++
	) {
		summary.str("");
		subimPos = inIter.position();
		if (csysSub.toWorld(world, subimPos)) {
			summary << "Fit   :" << endl;
			for (uInt i=0; i<world.size(); i++) {
				if ((Int)i != _fitAxis) {
					if (axesNames[i].startsWith("RIG")) {
						// right ascension
						summary << "    RA           :   "
							<< _radToRa(world[i]) << endl;
					}
					else if (axesNames[i].startsWith("DEC")) {
						// declination
						summary << "    Dec          : "
							<< MVAngle(world[i]).string(MVAngle::ANGLE_CLEAN, 8) << endl;
					}
					else if (axesNames[i].startsWith("FREQ")) {
						// frequency
						summary << "    Freq         : "
							<< world[i]
							<< csysSub.spectralCoordinate().formatUnit() << endl;
					}
					else if (axesNames[i].startsWith("STO")) {
						// stokes
						summary << "    Stokes       : "
							<< Stokes::name(Stokes::type((Int)world[i])) << endl;
					}
				}
			}
		}
		else {
			*_getLog() << csysSub.errorMessage() << LogIO::EXCEPTION;
		}
		for (uInt i=0; i<pixStart.size(); i++) {
			imPix[i] = pixStart[i] + subimPos[i];
		}
		summary.setf(ios::fixed);
		summary << setprecision(pixPrecision) << "    Pixel        : [";
		for (uInt i=0; i<imPix.size(); i++) {
			if (i == (uInt)_fitAxis) {
				summary << " *";
			}
			else {
				summary << imPix[i];
			}
			if (i != imPix.size()-1) {
				summary << ", ";
			}
		}
		summary << "]" << setprecision(basePrecision) << endl;
		summary.unsetf(ios::fixed);
		Bool attempted = fitter->getList().nelements() > 0;
		summary << "    Attempted    : "
			<< (attempted ? "YES" : "NO") << endl;
		if (attempted) {
			String converged = fitter->converged() ? "YES" : "NO";
			summary << "    Converged    : " << converged << endl;
			if (fitter->converged()) {
				solutions = fitter->getList();
				summary << "    Iterations   : " << fitter->getNumberIterations() << endl;
				for (uInt i=0; i<solutions.nelements(); i++) {
					summary << "    Results for component " << i << ":" << endl;
					if (solutions[i]->getType() == SpectralElement::GAUSSIAN) {
						const GaussianSpectralElement *g = dynamic_cast<const GaussianSpectralElement*>(solutions[i]);
						summary << _gaussianToString(
							*g, csys, world.copy(), subimPos
						);
					}
					else if (solutions[i]->getType() == SpectralElement::POLYNOMIAL) {
						const PolynomialSpectralElement *p = dynamic_cast<const PolynomialSpectralElement*>(solutions[i]);
						summary << _polynomialToString(*p, csys, imPix, world);
					}
				}
			}
		}
		*_getLog() << LogIO::NORMAL << summary.str() << endl << LogIO::POST;
	}
}

String ImageProfileFitter::_elementToString(
	const Double value, const Double error,
	const String& unit
) const {
	ostringstream out;

	Unit myUnit(unit);
	Vector<String> unitPrefix;
	String outUnit;
	Quantity qVal(value, unit);
	Quantity qErr(error, unit);

	if (myUnit.getValue() == UnitVal::ANGLE) {
		Vector<String> angUnits(5);
		angUnits[0] = "deg";
		angUnits[1] = "arcmin";
		angUnits[2] = "arcsec";
		angUnits[3] = "marcsec";
		angUnits[4] = "uarcsec";
	    for (uInt i=0; i<angUnits.size(); i++) {
	    	outUnit = angUnits[i];
	    	if (fabs(qVal.getValue(outUnit)) > 1) {
	    		qVal.convert(outUnit);
	    		qErr.convert(outUnit);
	    		break;
	    	}
	    }
	}
	else if (unit.empty() || Quantity(1, myUnit).isConform(Quantity(1, "m/s"))) {
		// do nothing
	}
    else {
		Vector<String> unitPrefix(10);
		unitPrefix[0] = "T";
		unitPrefix[1] = "G";
		unitPrefix[2] = "M";
		unitPrefix[3] = "k";
		unitPrefix[4] = "";
		unitPrefix[5] = "m";
		unitPrefix[6] = "u";
		unitPrefix[7] = "n";
		unitPrefix[8] = "p";
		unitPrefix[9] = "f";

		for (uInt i=0; i<unitPrefix.size(); i++) {
			outUnit = unitPrefix[i] + unit;
			if (fabs(qVal.getValue(outUnit)) > 1) {
				qVal.convert(outUnit);
				qErr.convert(outUnit);
				break;
			}
		}
	}
    Vector<Double> valErr(2);
    valErr[0] = qVal.getValue();
    valErr[1] = qErr.getValue();

    uInt precision = precisionForValueErrorPairs(valErr, Vector<Double>());
    out << std::fixed << setprecision(precision);
    out << qVal.getValue() << " +/- " << qErr.getValue()
    	<< " " << qVal.getUnit();
    return out.str();
}

String ImageProfileFitter::_gaussianToString(
	const GaussianSpectralElement& gauss, const CoordinateSystem& csys,
	const Vector<Double> world, const IPosition imPos
) const {
	Vector<Double> myWorld = world;
    String yUnit = _getImage()->units().getName();
	ostringstream summary;
	summary << "        Type     : GAUSSIAN" << endl;
	summary << "        Peak     : "
		<< _elementToString(
			gauss.getAmpl(), gauss.getAmplErr(), yUnit
		)
		<< endl;
	// Double center = gauss.getCenter();
	Double center = _centerWorld(
	    gauss, imPos
	);
	Double increment = fabs(_fitAxisIncrement());

	Double centerErr = gauss.getCenterErr() * increment;
	Double fwhm = gauss.getFWHM() * increment;
	Double fwhmErr = gauss.getFWHMErr() * increment;

	Double pCenter = 0;
	Double pCenterErr = 0;
	Double pFWHM = 0;
	Double pFWHMErr = 0;
	Int specCoordIndex = csys.findCoordinate(Coordinate::SPECTRAL);
	Bool convertedCenterToPix = True;
	Bool convertedFWHMToPix = True;

    if (
    	specCoordIndex >= 0
    	&& _fitAxis == csys.pixelAxes(specCoordIndex)[0]
    	&& ! csys.spectralCoordinate(specCoordIndex).velocityUnit().empty()
    ) {
    	if (csys.spectralCoordinate(specCoordIndex).velocityToPixel(pCenter, center)) {
    		Double nextVel;
    		csys.spectralCoordinate(specCoordIndex).pixelToVelocity(nextVel, pCenter+1);
    		Double velInc = fabs(center - nextVel);
    		pCenterErr = centerErr/velInc;
    		pFWHM = fwhm/velInc;
    		pFWHMErr = fwhmErr/velInc;
    	}
    	else {
    		convertedCenterToPix = False;
    		convertedFWHMToPix = False;
    	}
    }
    else {
    	Vector<Double> pixel(myWorld.size());
    	myWorld[_fitAxis] = center;
    	Double delta = csys.increment()[_fitAxis];
    	if (csys.toPixel(pixel, myWorld)) {
    		pCenter = pixel[_fitAxis];
    		pCenterErr = centerErr/delta;
    	}
    	else {
    		convertedCenterToPix = False;
    	}
    	pFWHM = fwhm/delta;
    	pFWHMErr = fwhmErr/delta;
    }
	summary << "        Center   : "
		<< _elementToString(
			center, centerErr, _xUnit
		)
		<< endl;
	if (convertedCenterToPix) {
		summary << "                   "
			<< _elementToString(
				pCenter, pCenterErr, "pixel"
			)
			<< endl;
	}
	else {
		summary << "                  Could not convert world to pixel for center" << endl;
	}
	summary << "        FWHM     : "
		<< _elementToString(
			fwhm, fwhmErr, _xUnit
		)
		<< endl;
	if (convertedFWHMToPix) {
		summary << "                   " << _elementToString(
			pFWHM, pFWHMErr, "pixel"
		)
		<< endl;
	}
	else {
		summary << "                  Could not convert FWHM to pixel" << endl;
	}
	Double integral = integralConst * gauss.getAmpl() * fwhm;
	Double ampFErr = gauss.getAmplErr()/gauss.getAmpl();
	Double fwhmFErr = fwhmErr/fwhm;
	Double integralErr = integral * sqrt(ampFErr*ampFErr + fwhmFErr*fwhmFErr);
	String integUnit = (Quantity(1.0 ,yUnit)*Quantity(1.0, _xUnit)).getUnit();
	summary << "        Integral : "
		<< _elementToString(
			integral, integralErr, integUnit
		)
		<< endl;
	return summary.str();
}

String ImageProfileFitter::_polynomialToString(
	const PolynomialSpectralElement& poly, const CoordinateSystem& csys,
	const Vector<Double> imPix, const Vector<Double> world
) const {
	ostringstream summary;
	summary << "        Type: POLYNOMIAL" << endl;
	Vector<Double> parms, errs;
	poly.get(parms);
	poly.getError(errs);
	for (uInt j=0; j<parms.size(); j++) {
		String unit = _getImage()->units().getName();
        if (j > 0) {
			unit = unit + "/((pixel)" + String::toString(j) + ")";
		}
		summary << "         c" << j << " : "
            << _elementToString(parms[j], errs[j], unit) << endl;
	}
    // coefficients in pixel coordinates
    Double x0;
    Double deltaX = _fitAxisIncrement();

    if (Quantity(1,_xUnit).isConform(Quantity(1, "m/s"))) {
        csys.spectralCoordinate(csys.findCoordinate(Coordinate::SPECTRAL)).pixelToVelocity(x0, 0);
    }
    else {
        Vector<Double> p0 = imPix;
        p0[_fitAxis] = 0;
        Vector<Double> world0 = world;
        csys.toWorld(world0, p0);
        x0 = world0[_fitAxis];
       // deltaX = csys.increment()[_fitAxis];
    }
    Vector<Double> pCoeff(_polyOrder + 1, 0);
    Vector<Double> pCoeffErr(_polyOrder + 1, 0);
    for (Int j=0; j<=_polyOrder; j++) {
        Double sumsq = 0;
        for (Int k=j; k<=_polyOrder; k++) {
        	/*
            Double multiplier = Combinatorics::choose(k, j)
                * casa::pow(x0, Float(k-j))
                * casa::pow(deltaX, Float(j));
            */
            Double multiplier = Combinatorics::choose(k, k-j)
				* casa::pow(x0, Float(k - j))
				* casa::pow(1/deltaX, Float(k));
            if ((k-j) % 2 == 1) {
            	multiplier *= -1;
            }


            pCoeff[j] += multiplier * parms[k];
            Double errCoeff = multiplier * errs[k];
            sumsq += errCoeff * errCoeff;
        }
        pCoeffErr[j] = casa::sqrt(sumsq);
        summary << "         c" << j << " : ";
		String unit = _getImage()->units().getName();
		if (j > 0 ) {
			unit = unit + "/" + "((" + _xUnit + ")" + String::toString(j) + ")";
		}
        summary << _elementToString(pCoeff[j], pCoeffErr[j], unit) << endl;
    }
	return summary.str();
}

void ImageProfileFitter::_makeSolutionImage(
	const String& name, const CoordinateSystem& csys,
	const Array<Double>& values, const String& unit,
	const Array<Bool>& mask
) {
	IPosition shape = values.shape();
	PagedImage<Float> image(shape, csys, name);
	Vector<Float> dataCopy(values.size());
	Vector<Double>::const_iterator iter;
	// isNaN(Array<Double>&) works, isNaN(Array<Float>&) gives spurious results
	Array<Bool> nanMask = ! isNaN(values);

	Vector<Float>::iterator jiter = dataCopy.begin();
	for (iter=values.begin(); iter!=values.end(); iter++, jiter++) {
		*jiter = (Float)*iter;
	}
	image.put(dataCopy.reform(shape));
	Bool hasPixMask = ! allTrue(mask);
	Bool hasNanMask = ! allTrue(nanMask);
	if (hasNanMask || hasPixMask) {
		Array<Bool> resMask(shape);
		String maskName = image.makeUniqueRegionName(
			String("mask"), 0
		);
		image.makeMask(maskName, True, True, False);
		if (hasPixMask) {
			resMask = mask.copy().reform(shape);
			if (hasNanMask) {
				resMask = resMask && nanMask;
			}
		}
		else {
			resMask = nanMask;
		}
		(&image.pixelMask())->put(resMask);
	}
	image.setUnits(Unit(unit));
}

// moved from ImageAnalysis
ImageFit1D<Float> ImageProfileFitter::_fitProfile(
	const Bool fitIt, const String weightsImageName
) {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	PtrHolder<ImageInterface<Float> > weightsImagePtrHolder;
	ImageInterface<Float> *pWeights = 0;
	if (! weightsImageName.empty()) {
		PagedImage<Float> weightsImage(weightsImageName);
		if (! weightsImage.shape().conform(_getImage()->shape())) {
			*_getLog() << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		std::auto_ptr<ImageRegion> pR(
			ImageRegion::fromRecord(
				_getLog().get(), weightsImage.coordinates(),
				weightsImage.shape(), *_getRegion()
			)
		);
		weightsImagePtrHolder.set(
			new SubImage<Float> (weightsImage, *pR, False)
		);
		pWeights = weightsImagePtrHolder.ptr();
	}

	// Set default axis
	const uInt nDim = _subImage.ndim();
	CoordinateSystem cSys = _subImage.coordinates();
	String doppler = "";
	ImageUtilities::getUnitAndDoppler(
		_xUnit, doppler, _fitAxis, cSys
	);

	// SpectralElement fromRecord handles each numbered elements
	// field (type, parameters, errors). It does not yet handle
	// the 'fixed' field (see below)

	Bool xAbs = True;
	// Figure out the abcissa type specifying what abcissa domain the fitter
	// is operating in.  Convert the CoordinateSystem to this domain
	// and set it back in the image
	String errMsg;
	ImageFit1D<Float>::AbcissaType abcissaType;
	/*
	Bool ok = ImageFit1D<Float>::setAbcissaState(
		errMsg, abcissaType, cSys, _xUnit, doppler, _fitAxis
	);
	*/
	Bool ok = ImageFit1D<Float>::setAbcissaState(
		errMsg, abcissaType, cSys, "pix", doppler, _fitAxis
	);
	_subImage.setCoordinateInfo(cSys);

	if (!ok) {
		*_getLog() << LogIO::WARN << errMsg << LogIO::POST;
	}

	ImageFit1D<Float> fitter;
	if (pWeights) {
		fitter.setImage(_subImage, *pWeights, _fitAxis);
	} else {
		fitter.setImage(_subImage, _fitAxis);
	}
	// Set data region averaging data in region.  We could also set the
	// ImageRegion from that passed in to this function rather than making
	// a SubImage. But the way I have done it, the 'mask' keyword is
	// handled automatically there.
	Slicer sl(IPosition(nDim, 0), _subImage.shape(), Slicer::endIsLength);
	LCSlicer lslice(sl);
	ImageRegion region(lslice);
	if (! fitter.setData(region, abcissaType, xAbs)) {
		*_getLog() << fitter.errorMessage() << LogIO::EXCEPTION;
	}

	// Now we do one of three things:
	// 1) make a fit and evaluate
	// 2) evaluate a model
	// 3) make an estimate and evaluate
	Vector<Float> model(0), residual(0);

	if (fitIt) {
		if (_estimates.nelements() > 0) {
			//_convertEstimates();
			fitter.setElements(_estimates);
		} else {
			// Set auto estimate
			if (! fitter.setGaussianElements(_ngauss)) {
				*_getLog() << LogIO::WARN << fitter.errorMessage() << LogIO::POST;
			}
		}
		if (_polyOrder >= 0) {
			// Add baseline
			PolynomialSpectralElement polyEl(_polyOrder);
			fitter.addElement(polyEl);
		}
		if (! fitter.fit()) {
			*_getLog() << LogIO::WARN << "Fit failed to converge" << LogIO::POST;
		}
		if (! _model.empty()) {
			model = fitter.getFit();
			ImageCollapser collapser(
				&_subImage, IPosition(1, _fitAxis), True,
				ImageCollapser::ZERO, _model, True
			);
			std::auto_ptr<PagedImage<Float> > modelImage(
				static_cast<PagedImage<Float>*>(
					collapser.collapse(True)
				)
			);
			modelImage->put(model.reform(modelImage->shape()));
			modelImage->flush();
		}
		if (! _residual.empty()) {
			residual = fitter.getResidual(-1, True);
            ImageCollapser collapser(
				&_subImage, IPosition(1, _fitAxis), True,
				ImageCollapser::ZERO, _residual, True
			);
			std::auto_ptr<PagedImage<Float> > residualImage(
				static_cast<PagedImage<Float>*>(
					collapser.collapse(True)
				)
			);
			residualImage->put(residual.reform(residualImage->shape()));
			residualImage->flush();
		}
	}
	else {
		if (_estimates.nelements() > 0) {
			fitter.setElements(_estimates); // Set list
			model = fitter.getEstimate(); // Evaluate list
			residual = fitter.getResidual(-1, False);
		}
		else {
			if (fitter.setGaussianElements(_ngauss)) { // Auto estimate
				model = fitter.getEstimate(); // Evaluate
				residual = fitter.getResidual(-1, False);
			}
			else {
				*_getLog() << LogIO::SEVERE << fitter.errorMessage()
					<< LogIO::POST;
			}
		}
	}
	return fitter;
}

Array<ImageFit1D<Float> > ImageProfileFitter::_fitallprofiles(
	const String& weightsImageName
) {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	IPosition imageShape = _subImage.shape();
	PtrHolder<ImageInterface<Float> > weightsImage;
	std::auto_ptr<TempImage<Float> > pWeights(0);
	if (! weightsImageName.empty()) {
		PagedImage<Float> sigmaImage(weightsImageName);
		if (!sigmaImage.shape().conform(_getImage()->shape())) {
			*_getLog() << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		std::auto_ptr<ImageRegion> pR(
			ImageRegion::fromRecord(
				_getLog().get(), sigmaImage.coordinates(),
				sigmaImage.shape(), *_getRegion()
			)
		);
		weightsImage.set(new SubImage<Float> (sigmaImage, *pR, False));
		//pWeights = weightsImage.ptr();
	}
	// Set default axis
	CoordinateSystem cSys = _subImage.coordinates();
	Int pAxis = CoordinateUtil::findSpectralAxis(cSys);
	Int axis2 = _fitAxis;
	if (axis2 < 0) {
		if (pAxis != -1) {
			axis2 = pAxis;
		}
		else {
			axis2 = _subImage.ndim() - 1;
		}
	}
	// Create output images with a mask
	PtrHolder<ImageInterface<Float> > fitImage, residImage;
	ImageInterface<Float>* pFit = 0;
	ImageInterface<Float>* pResid = 0;
	if (
		ImageAnalysis::makeExternalImage(
			fitImage, _model, cSys, imageShape, _subImage,
			*_getLog(), True, False, True
		)
	) {
		pFit = fitImage.ptr();
	}
	if (
		ImageAnalysis::makeExternalImage(
			residImage, _residual, cSys, imageShape,
			_subImage, *_getLog(), True, False, True
		)
	) {
		pResid = residImage.ptr();
	}
	// Do fits
	// FIXME give users the option to show a progress bar
	Bool showProgress = False;
	if (_estimates.nelements() > 0) {
		String doppler = "";
		ImageUtilities::getUnitAndDoppler(
			_xUnit, doppler, _fitAxis, cSys
		);
	}
	return _fitProfiles(
		pFit, pResid,
		pWeights.get(),
		showProgress
	);
}

// moved from ImageUtilities
Array<ImageFit1D<Float> > ImageProfileFitter::_fitProfiles(
	ImageInterface<Float>*& pFit, ImageInterface<Float>*& pResid,
    const ImageInterface<Float> *const &weightsImage,
    const Bool showProgress
) {

	IPosition inShape = _subImage.shape();
	if (pFit) {
		AlwaysAssert(inShape.isEqual(pFit->shape()), AipsError);
	}
	if (pResid) {
		AlwaysAssert(inShape.isEqual(pResid->shape()), AipsError);
	}

	// Check axis

	const uInt nDim = _subImage.ndim();

	// Progress Meter

	std::auto_ptr<ProgressMeter> pProgressMeter(0);
	if (showProgress) {
		Double nMin = 0.0;
		Double nMax = 1.0;
		for (uInt i=0; i<inShape.nelements(); i++) {
			if ((Int)i != _fitAxis) {
				nMax *= inShape(i);
			}
		}
		ostringstream oss;
		oss << "Fit profiles on axis " << _fitAxis+1;
		pProgressMeter.reset(
			new ProgressMeter(
				nMin, nMax, String(oss),
				String("Fits"),
				String(""), String(""),
				True, max(1,Int(nMax/20))
			)
		);
	}

	Lattice<Bool>* pFitMask = 0;
	if (pFit && pFit->hasPixelMask() && pFit->pixelMask().isWritable()) {
		pFitMask = &(pFit->pixelMask());
	}
	Lattice<Bool>* pResidMask = 0;
	if (pResid && pResid->hasPixelMask() && pResid->pixelMask().isWritable()) {
		pResidMask = &(pResid->pixelMask());
	}
	//
	IPosition sliceShape(nDim, 1);
	sliceShape(_fitAxis) = inShape(_fitAxis);
	Array<Float> failData(sliceShape);
	failData = 0.0;
	Array<Bool> failMask(sliceShape);
	failMask = False;
	Array<Float> resultData(sliceShape);
	Array<Bool> resultMask(sliceShape);

	// Since we write the fits out to images, fitting in pixel space is fine
	// FIXME I don't understand the above comment. The fit results (peak, center, fwhm)
	// certainly
	// are not written out in this method yet but they are what astronomers want.
	// I'm switching to image units.
	// OK so the fitter has problems with polynomials if the abscissa values are much
	// different from unity so for now callers should be careful of polynomials or
	// prohibit them althoughter. In the future I'll probably switch back to pixel
	// units here and force callers to deal with putting results in astronomer
	// friendly units.
	// typename ImageFit1D<T>::AbcissaType abcissaType = ImageFit1D<T>::PIXEL;

    String doppler = "";
	CoordinateSystem csys = _subImage.coordinates();
	ImageUtilities::getUnitAndDoppler(
		_xUnit, doppler, _fitAxis, csys
	);
	String errMsg;
	ImageFit1D<Float>::AbcissaType abcissaType;

	if (
		! ImageFit1D<Float>::setAbcissaState(
			errMsg, abcissaType, csys, "pix", doppler, _fitAxis
		)
	) {
		throw AipsError(errMsg);
	}
	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);

	uInt nFail = 0;
	uInt nConv = 0;
	uInt nProfiles = 0;
	uInt nFit = 0;
	IPosition fitterShape = inShape;
	fitterShape[_fitAxis] = 1;
	Array<ImageFit1D<Float> > fitters(fitterShape);
	Array<ImageFit1D<Float> *> goodFits(fitterShape);
	Int nPoints = fitterShape.product();
	uInt count = 0;
	vector<IPosition> goodPos(0);
	goodFits.set(0);
	Bool checkMinPts = _minGoodPoints > 0 && _subImage.isMasked();
	SpectralList newEstimates = _estimates;
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	for (inIter.reset(); !inIter.atEnd(); inIter++, nProfiles++) {
		if (count % 1000 == 0 && count > 0) {
			*_getLog() << LogIO::NORMAL << "Fitting profile number "
				<< count << " of " << nPoints << LogIO::POST;
		}
		const IPosition& curPos = inIter.position();
		if (checkMinPts) {
			IPosition sliceShape = inShape;
			sliceShape = 1;
			sliceShape[_fitAxis] = inShape[_fitAxis];
			if (
				ntrue(_subImage.getMaskSlice(curPos, sliceShape, True))
				< _minGoodPoints
			) {
				// not enough good points, just add a dummy fitter
				// and go to the next position
				// fitters[index] = ImageFit1D<T>();
				fitters(curPos) = ImageFit1D<Float>();
				continue;
			}
		}

		ImageFit1D<Float> fitter = (weightsImage == 0)
			? ImageFit1D<Float>(_subImage, _fitAxis)
			: ImageFit1D<Float>(_subImage, *weightsImage, _fitAxis);

		fitter.errorMessage();
		if (! fitter.setData (curPos, abcissaType, True)) {
			*_getLog() << "Unable to set data" << LogIO::EXCEPTION;
		}
		if (! fitter.setGaussianElements (_ngauss)) {
			*_getLog() << "Unable to set gaussian elements"
				<< LogIO::EXCEPTION;
		}
		if (_estimates.nelements() > 0) {
			// user supplied initial estimates
			if (goodPos.size() > 0) {
				IPosition nearest;
				Int minDist2 = 0;
				for (
					IPosition::const_iterator iter=fitterShape.begin();
					iter!=fitterShape.end(); iter++
				) {
					minDist2 += *iter * *iter;
				}
				for (
					vector<IPosition>::const_reverse_iterator iter=goodPos.rbegin();
					iter != goodPos.rend(); iter++
				) {
					IPosition diff = curPos - *iter;
					Int dist2 = 0;
					Bool larger = False;
					for (
						IPosition::const_iterator ipositer=diff.begin();
						ipositer!=diff.end(); ipositer++
					) {
						dist2 += *ipositer * *ipositer;
						if(dist2 >= minDist2) {
							larger = True;
							break;
						}
					}
					if (! larger) {
						minDist2 = dist2;
						nearest = *iter;
						if (minDist2 == 1) {
							// can't get any nearer than this
							break;
						}
					}
				}
				SpectralList goodList = fitters(nearest).getList();
				uInt count = 0;
				for (uInt i=0; i<newEstimates.nelements(); i++) {
					if (newEstimates[i]->getType() != SpectralElement::GAUSSIAN) {
						continue;
					}
					while (
						count<goodList.nelements()
						&& goodList[count]->getType() != SpectralElement::GAUSSIAN
					) {
						count++;
					}
					if (count >= goodList.nelements()) {
						break;
					}
					auto_ptr<GaussianSpectralElement> myel(
						dynamic_cast<GaussianSpectralElement*>(newEstimates[i]->clone())
					);
					const GaussianSpectralElement *goodListGauss = dynamic_cast<
							const GaussianSpectralElement*
						>(goodList[count]);
					myel->setAmpl(goodListGauss->getAmpl());
					myel->setCenter(goodListGauss->getCenter());
					myel->setFWHM(goodListGauss->getFWHM());
					newEstimates.set(*myel, i);
				}
			}
			fitter.setElements(newEstimates);
		}
		if (_polyOrder >= 0) {
			PolynomialSpectralElement polyEl(_polyOrder);
			fitter.addElement (polyEl);
		}
		nFit++;
		Bool ok = False;
		try {
			if (ok = fitter.fit()) {               // ok == False means no convergence
				if (_estimates.nelements() > 0) {
					goodFits(curPos) = &fitter;
					goodPos.push_back(curPos);
				}
			}
			else {
				nConv++;
			}
		} catch (AipsError x) {
			ok = False;                       // Some other error
			nFail++;
		}
		fitters(curPos) = fitter;
		// Evaluate and fill
		if (ok) {
			Array<Bool> resultMask = fitter.getTotalMask().reform(sliceShape);
			if (pFit) {
				Array<Float> resultData = fitter.getFit().reform(sliceShape);
				pFit->putSlice (resultData, curPos);
				if (pFitMask) {
					pFitMask->putSlice(resultMask, curPos);
				}
			}
			if (pResid) {
				Array<Float> resultData = fitter.getResidual().reform(sliceShape);
				pResid->putSlice (resultData, curPos);
				if (pResidMask) pResidMask->putSlice(resultMask, curPos);
			}
		}
		else {
			if (pFit) {
				pFit->putSlice (failData, curPos);
				if (pFitMask) {
					pFitMask->putSlice(failMask, curPos);
				}
			}
			if (pResid) {
				pResid->putSlice (failData, curPos);
				if (pResidMask) {
					pResidMask->putSlice(failMask, curPos);
				}
			}
		}
		if (showProgress) {
			pProgressMeter->update(Double(nProfiles));
		}
		count++;
	}
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	*_getLog() << LogIO::NORMAL << "Number of profiles       = " << nProfiles << LogIO::POST;
	*_getLog() << LogIO::NORMAL << "Number of fits attempted = " << nFit << LogIO::POST;
	*_getLog() << LogIO::NORMAL << "Number converged         = " << nFit - nConv - nFail << LogIO::POST;
	*_getLog() << LogIO::NORMAL << "Number not converged     = " << nConv << LogIO::POST;
	*_getLog() << LogIO::NORMAL << "Number failed            = " << nFail << LogIO::POST;
	return fitters;
}

} //# NAMESPACE CASA - END

