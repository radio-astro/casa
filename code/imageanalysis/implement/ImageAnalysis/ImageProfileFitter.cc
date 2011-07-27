
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

#include <memory>

namespace casa {

const String ImageProfileFitter::_class = "ImageProfileFitter";

/*
ImageProfileFitter::ImageProfileFitter(
    const String& imagename, const String& region, const String& box,
    const String& chans, const String& stokes,
    const String& mask, const Int axis, const Bool multiFit,
    const String& residual, const String& model, const uInt ngauss,
    const Int polyOrder, const String& ampName,
	const String& ampErrName, const String& centerName,
	const String& centerErrName, const String& fwhmName,
	const String& fwhmErrName
) : _log(new LogIO()), _getImage()(0),
	_regionName(region), _box(box), _chans(chans), _stokes(stokes),
	_getMask()(mask), _residual(residual),
	_model(model), _regionString(""), _xUnit(""),
	_centerName(centerName), _centerErrName(centerErrName),
	_fwhmName(fwhmName), _fwhmErrName(fwhmErrName),
	_ampName(ampName), _ampErrName(ampErrName),
	_multiFit(multiFit), _deleteImageOnDestruct(True),
	_polyOrder(polyOrder), _fitAxis(axis), _ngauss(ngauss),
	_results(Record()) {
    _construct(imagename);
}
*/
ImageProfileFitter::ImageProfileFitter(
	const ImageInterface<Float> *const &image, const String& region,
	const Record *const &regionPtr,	const String& box,
	const String& chans, const String& stokes,
	const String& mask, const Int axis, const Bool multiFit,
	const String& residual, const String& model, const uInt ngauss,
	const Int polyOrder, const String& ampName,
	const String& ampErrName, const String& centerName,
	const String& centerErrName, const String& fwhmName,
	const String& fwhmErrName
) : ImageTask(
		image, region, regionPtr, box, chans, stokes,
		mask, "", False
	),
	_residual(residual), _model(model), _xUnit(""),
	_centerName(centerName), _centerErrName(centerErrName),
	_fwhmName(fwhmName), _fwhmErrName(fwhmErrName),
	_ampName(ampName), _ampErrName(ampErrName),
	_multiFit(multiFit), _deleteImageOnDestruct(False),
	_polyOrder(polyOrder), _fitAxis(axis), _ngauss(ngauss),
	_results(Record()) {
    _checkNGaussAndPolyOrder();
    _construct();
    _finishConstruction();
}

ImageProfileFitter::~ImageProfileFitter() {}

Record ImageProfileFitter::fit() {
    LogOrigin logOrigin(_class, __FUNCTION__);
    *_log << logOrigin;

	Record estimate;

	String weightsImageName = "";
	try {
		if (_multiFit) {
			// FIXME need to be able to specify the weights image
			_fitters = _fitallprofiles(
				*_getRegion(), _subImage, _xUnit, _fitAxis, _getMask(),
				_ngauss, _polyOrder, weightsImageName,
				_model, _residual
			);
		}
		else {
			ImageFit1D<Float> fitter = _fitProfile(
				*_getRegion(), _subImage, _xUnit, _fitAxis, _getMask(),
				estimate, _ngauss, _polyOrder, _model,
				_residual
			);
			Vector<uInt> axes(1, _fitAxis);
			ImageCollapser collapser(
				&_subImage, axes, True,
				ImageCollapser::MEAN, "", True
			);
			ImageInterface<Float> *x = static_cast<SubImage<Float>*>(collapser.collapse(True));
            _subImage = (SubImage<Float>)(*x);
			delete x;
			_fitters.resize(1);
			_fitters[0] = fitter;
		}
	}
	catch (AipsError exc) {
		*_log << "Exception during fit: " << exc.getMesg()
			<< LogIO::EXCEPTION;
	}
	_setResults();
	*_log << LogIO::NORMAL << _resultsToString() << LogIO::POST;
	return _results;
}

Record ImageProfileFitter::getResults() const {
	return _results;
}

/*
void ImageProfileFitter::_construct(const String& imagename) {
	LogOrigin logOrigin(_class, __FUNCTION__);
    *_log << logOrigin;

    _checkNGaussAndPolyOrder();
    ImageInputProcessor inputProcessor;
    vector<ImageInputProcessor::OutputStruct>  outputStruct;
    _getOutputStruct(outputStruct);
    vector<ImageInputProcessor::OutputStruct>* outputPtr = outputStruct.size() == 0
        ? 0
        : &outputStruct;
    String diagnostics;
    inputProcessor.process(
    	_getImage(), *_getRegion(), diagnostics,
    	outputPtr, _stokes, imagename, 0,
    	_regionName, _box, _chans,
    	CasacRegionManager::USE_FIRST_STOKES,
    	False, 0
    );
    _finishConstruction();
}

void ImageProfileFitter::_construct(const ImageInterface<Float>* image) {
	LogOrigin logOrigin(_class, __FUNCTION__);
    *_log << logOrigin;
    _checkNGaussAndPolyOrder();
    _finishConstruction();
}
*/

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
		*_log << "Number of gaussians is 0 and polynomial order is less than zero. "
			<< "According to these inputs there is nothing to fit."
			<< LogIO::EXCEPTION;
	}
}

void ImageProfileFitter::_finishConstruction() {
    if (_fitAxis >= (Int)_getImage()->ndim()) {
    	*_log << "Specified fit axis " << _fitAxis
    		<< " must be less than the number of image axes ("
    		<< _getImage()->ndim() << ")" << LogIO::EXCEPTION;
    }
    if (_fitAxis < 0) {
		if (! _getImage()->coordinates().hasSpectralAxis()) {
			_fitAxis = 0;
			*_log << LogIO::WARN << "No spectral coordinate found in image, "
                << "using axis 0 as fit axis" << LogIO::POST;
		}
		else {
            _fitAxis = _getImage()->coordinates().spectralAxisNumber();
			*_log << LogIO::NORMAL << "Using spectral axis (axis " << _fitAxis
				<< ") as fit axis" << LogIO::POST;
		}
	}
}

void ImageProfileFitter::_setResults() {
	uInt nComps = _polyOrder < 0 ? _ngauss : _ngauss + 1;
	Vector<Bool> convergedArr(_fitters.size());
	Vector<Int> niterArr(_fitters.size(), 0);
    Matrix<Double> centerMat(_fitters.size(), nComps, -1);
	Matrix<Double> fwhmMat(_fitters.size(), nComps, -1);
	Matrix<Double> ampMat(_fitters.size(), nComps, -1);
	Matrix<Double> centerErrMat(_fitters.size(), nComps, -1);
	Matrix<Double> fwhmErrMat(_fitters.size(), nComps, -1);
	Matrix<Double> ampErrMat(_fitters.size(), nComps, -1);
	Matrix<String> typeMat(_fitters.size(), nComps, "");

	Vector<Int> nCompArr(_fitters.size(), 0);

	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters.begin();
	SpectralList solutions;

	uInt count = 0;
	for (
		inIter.reset();
		! inIter.atEnd() && fitter != _fitters.end();
		inIter++, fitter++, count++
	) {
		convergedArr[count] = fitter->converged();
		if (fitter->converged()) {
			niterArr[count] = (Int)fitter->getNumberIterations();
			nCompArr[count] = (Int)fitter->getList().nelements();
			solutions = fitter->getList();
			for (uInt i=0; i<solutions.nelements(); i++) {
				typeMat(count, i) = SpectralElement::fromType(solutions[i].getType());
				if (solutions[i].getType() == SpectralElement::GAUSSIAN) {
					centerMat(count, i) = solutions[i].getCenter();
					fwhmMat(count, i) = solutions[i].getFWHM();
					ampMat(count, i) = solutions[i].getFWHM();
					ampMat(count, i) = solutions[i].getAmpl();
					centerErrMat(count, i) = solutions[i].getCenterErr();
					fwhmErrMat(count, i) = solutions[i].getFWHMErr();
					ampErrMat(count, i) = solutions[i].getAmplErr();
				}
			}
		}
	}
	_results.define("converged", convergedArr);
	_results.define("niter", niterArr);
	_results.define("ncomps", nCompArr);
	_results.define("xUnit", _xUnit);
	_results.define("yUnit", _getImage()->units().getName());

	String key;
	TempImage<Float> *tmp = static_cast<TempImage<Float>* >(_getImage()->cloneII());
	Vector<uInt> axes(1, _fitAxis);
	ImageCollapser collapser(
		tmp, axes, False, ImageCollapser::ZERO, String(""), False
	);
	TempImage<Float> *myTemplate = static_cast<TempImage<Float>* >(collapser.collapse(True));
	delete tmp;
	IPosition shape = myTemplate->shape();
	CoordinateSystem csys = myTemplate->coordinates();
	delete myTemplate;
	uInt gaussCount = 0;
	if (
		! _multiFit && (
			! _centerName.empty() || ! _centerErrName.empty()
			|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
			|| ! _ampName.empty() || ! _ampErrName.empty()
		)
	) {
		*_log << LogIO::WARN << "This was not a multi-pixel fit request so solution "
			<< "images will not be written" << LogIO::POST;
	}

	for (uInt i=0; i<nComps; i++) {
		String num = String::toString(i);
		if (_multiFit && solutions[i].getType() == SpectralElement::GAUSSIAN) {
			String gnum = String::toString(gaussCount);
			String mUnit = _xUnit;
			if (!_centerName.empty()) {
				_makeSolutionImage(
					_centerName + "_" + gnum, shape,
					csys, centerMat.column(i), mUnit
				);
			}
			if (!_centerErrName.empty()) {
				_makeSolutionImage(
					_centerErrName + "_" + gnum, shape,
					csys, centerErrMat.column(i), mUnit
				);
			}
			if (!_fwhmName.empty()) {
				_makeSolutionImage(
					_fwhmName + "_" + gnum, shape,
					csys, fwhmMat.column(i), mUnit
				);
			}
			if (!_fwhmErrName.empty()) {
				_makeSolutionImage(
					_fwhmErrName + "_" + gnum, shape,
					csys, fwhmErrMat.column(i), mUnit
				);
			}
			mUnit = _getImage()->units().getName();
			if (!_ampName.empty()) {
				_makeSolutionImage(
					_ampName + "_" + gnum, shape,
					csys, ampMat.column(i), mUnit
				);
			}
			if (!_ampErrName.empty()) {
				_makeSolutionImage(
					_ampErrName + "_" + gnum, shape,
					csys, ampErrMat.column(i), mUnit
				);
			}
			gaussCount++;
		}
		key = "center" + num;
		_results.define(key, centerMat.column(i));
		key = "fwhm" + num;
		_results.define(key, fwhmMat.column(i));
		key = "amp" + num;
		_results.define(key, ampMat.column(i));
		key = "centerErr" + num;
		_results.define(key, centerErrMat.column(i));
		key = "fwhmErr" + num;
		_results.define(key, fwhmErrMat.column(i));
		key = "ampErr" + num;
		_results.define(key, ampErrMat.column(i));
		key = "type" + num;
		_results.define(key, typeMat.column(i));
	}
}

String ImageProfileFitter::_radToRa(Float ras) const{

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


String ImageProfileFitter::_resultsToString() const {
	ostringstream summary;
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << _summaryHeader();
	summary << "       --- polynomial order:    " << _polyOrder << endl;
	summary << "       --- number of Gaussians: " << _ngauss << endl;

	if (_multiFit) {
		summary << "       --- Multiple spectra fit, one per pixel over selected region" << endl;
	}
	else {
		summary << "       --- One spectrum fit, averaged over several pixels if necessary" << endl;
	}

	IPosition inTileShape = _subImage.niceCursorShape();
	TiledLineStepper stepper (_subImage.shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(_subImage, stepper);
	CoordinateSystem csysSub = _subImage.coordinates();
	CoordinateSystem csys = _getImage()->coordinates();
	Vector<Double> worldStart;
	if (! csysSub.toWorld(worldStart, inIter.position())) {
		*_log << csysSub.errorMessage() << LogIO::EXCEPTION;
	}
	CoordinateSystem csysIm = _getImage()->coordinates();
	Vector<Double> pixStart;
	if (! csysIm.toPixel(pixStart, worldStart)) {
		*_log << csysIm.errorMessage() << LogIO::EXCEPTION;
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
		subimPos = inIter.position();
		if (csysSub.toWorld(world, subimPos)) {
			summary << "Fit centered at:" << endl;
			for (uInt i=0; i<world.size(); i++) {
				if ((Int)i != _fitAxis) {
                                        //summary << "ax=" << axesNames[i] 
                                        //        << " world=" << world[i] 
                                        //        << endl;
					if (axesNames[i].startsWith("RIG")) {
						// right ascension
						summary << "    RA         :   "
						//	<< MVTime(world[i]).string(MVTime::TIME, 9) << endl;
							<< _radToRa(world[i]) << endl;
					}
					else if (axesNames[i].startsWith("DEC")) {
						// declination
						summary << "    Dec        : "
							<< MVAngle(world[i]).string(MVAngle::ANGLE_CLEAN, 8) << endl;
					}
					else if (axesNames[i].startsWith("FREQ")) {
						// frequency
						summary << "    Freq       : "
							<< world[i]
							<< csysSub.spectralCoordinate(i).formatUnit() << endl;
					}
					else if (axesNames[i].startsWith("STO")) {
						// stokes
						summary << "    Stokes     : "
							<< Stokes::name(Stokes::type((Int)world[i])) << endl;
					}
				}
			}
		}
		else {
			*_log << csysSub.errorMessage() << LogIO::EXCEPTION;
		}
		for (uInt i=0; i<pixStart.size(); i++) {
			imPix[i] = pixStart[i] + subimPos[i];
		}

		summary.setf(ios::fixed);
		summary << setprecision(pixPrecision) << "    Pixel      : [";
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
		String converged = fitter->converged() ? "YES" : "NO";
		summary << "    Converged  : " << converged << endl;
		if (fitter->converged()) {
			solutions = fitter->getList();
			summary << "    Iterations : " << fitter->getNumberIterations() << endl;
			for (uInt i=0; i<solutions.nelements(); i++) {
				summary << "    Results for component " << i << ":" << endl;
				if (solutions[i].getType() == SpectralElement::GAUSSIAN) {
					summary << _gaussianToString(
						solutions[i], csys, world.copy()
					);
				}
				else if (solutions[i].getType() == SpectralElement::POLYNOMIAL) {
					summary << _polynomialToString(solutions[i], csys, imPix, world);
				}
			}
		}
		summary << endl;
	}
	return summary.str();
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
	const SpectralElement& gauss, const CoordinateSystem& csys,
	const Vector<Double> world
) const {
	Vector<Double> myWorld = world;
    String yUnit = _getImage()->units().getName();
	ostringstream summary;
	summary << "        Type   : GAUSSIAN" << endl;
	summary << "        Peak   : "
		<< _elementToString(
			gauss.getAmpl(), gauss.getAmplErr(), yUnit
		)
		<< endl;
	Double center = gauss.getCenter();
	Double centerErr = gauss.getCenterErr();
	Double fwhm = gauss.getFWHM();
	Double fwhmErr = gauss.getFWHMErr();

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
    ) {;
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
	summary << "        Center : "
		<< _elementToString(
			center, centerErr, _xUnit
		)
		<< endl;
	if (convertedCenterToPix) {
		summary << "                 "
			<< _elementToString(
				pCenter, pCenterErr, "pixel"
			)
			<< endl;
	}
	else {
		summary << "                Could not convert world to pixel for center" << endl;
	}
	summary << "        FWHM   : "
		<< _elementToString(
			fwhm, fwhmErr, _xUnit
		)
		<< endl;
	if (convertedFWHMToPix) {
		summary << "                 " << _elementToString(
			pFWHM, pFWHMErr, "pixel"
		)
		<< endl;
	}
	else {
		summary << "                Could not convert FWHM to pixel" << endl;
	}
	return summary.str();
}

String ImageProfileFitter::_polynomialToString(
	const SpectralElement& poly, const CoordinateSystem& csys,
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
          String denom = _xUnit.find("/") != String::npos
                ? "(" + _xUnit + ")"
                : _xUnit;
			unit = unit + "/" + denom + "^" + String::toString(j);
		}
		summary << "         c" << j << " : "
            << _elementToString(parms[j], errs[j], unit) << endl;
	}
    // coefficents in pixels
    Double x0, deltaX;
    if (Quantity(1,_xUnit).isConform(Quantity(1, "m/s"))) {
        Double x1;
        csys.spectralCoordinate(csys.findCoordinate(Coordinate::SPECTRAL)).pixelToVelocity(x0, 0);
        csys.spectralCoordinate(csys.findCoordinate(Coordinate::SPECTRAL)).pixelToVelocity(x1, 1);
        deltaX = x1 - x0;
    }
    else {
        Vector<Double> p0 = imPix;
        p0[_fitAxis] = 0;
        Vector<Double> world0 = world;
        csys.toWorld(world0, p0);
        x0 = world0[_fitAxis];
        deltaX = csys.increment()[_fitAxis];
    }
    Vector<Double> pCoeff(_polyOrder + 1, 0);
    Vector<Double> pCoeffErr(_polyOrder + 1, 0);
    for (Int j=0; j<=_polyOrder; j++) {
        Double sumsq = 0;
        for (Int k=j; k<=_polyOrder; k++) {
            Double multiplier = Combinatorics::choose(k, j)
                * casa::pow(x0, Float(k-j))
                * casa::pow(deltaX, Float(j));

            pCoeff[j] += multiplier * parms[k];
            Double errCoeff = multiplier * errs[k];
            sumsq += errCoeff * errCoeff;
        }
        pCoeffErr[j] = casa::sqrt(sumsq);
        summary << "         c" << j << " : ";
		String unit = _getImage()->units().getName() + "/(pixel)^" + String::toString(j);
        summary << _elementToString(pCoeff[j], pCoeffErr[j], unit) << endl;
    }
	return summary.str();
}

void ImageProfileFitter::_makeSolutionImage(
	const String& name, const IPosition& shape, const CoordinateSystem& csys,
	const Vector<Double>& values, const String& unit
) {
	Vector<Float> tmpVec(values.size());
	for (uInt i=0; i<tmpVec.size(); i++) {
		tmpVec[i] = values[i];
	}
	PagedImage<Float> image( shape, csys, name);
	image.put(tmpVec.reform(shape));
	image.setUnits(Unit(unit));
}

// moved from ImageAnalysis
ImageFit1D<Float> ImageProfileFitter::_fitProfile(
	const Record& regionRecord, SubImage<Float>& subImage,
	String& xUnit, const uInt axis,
	const String& mask, const Record& estimate,
	const uInt ngauss, const Int poly,
	const String& modelName, const String& residualName,
	const Bool fitIt, const String weightsImageName
) {
	*_log << LogOrigin(_class, __FUNCTION__);
	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
	std::auto_ptr<Record> regionClone(dynamic_cast<Record*>(
		_getRegion()->clone())
	);
	subImage = SubImage<Float>::createSubImage(
		*clone, *(
			ImageRegion::tweakedRegionRecord(
				regionClone.get()
			)
		), mask, 0, False
	);
	IPosition imageShape = subImage.shape();

	PtrHolder<ImageInterface<Float> > weightsImagePtrHolder;
	ImageInterface<Float> *pWeights = 0;
	if (! weightsImageName.empty()) {
		PagedImage<Float> weightsImage(weightsImageName);
		if (! weightsImage.shape().conform(_getImage()->shape())) {
			*_log << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		ImageRegion* pR = ImageRegion::fromRecord(
			_log.get(), weightsImage.coordinates(),
			weightsImage.shape(), regionRecord
		);
		weightsImagePtrHolder.set(new SubImage<Float> (weightsImage, *pR, False));
		pWeights = weightsImagePtrHolder.ptr();
		delete pR;
	}

	// Set default axis
	const uInt nDim = subImage.ndim();
	CoordinateSystem cSys = subImage.coordinates();
	String doppler = "";
	ImageUtilities::getUnitAndDoppler(
		xUnit, doppler, axis, cSys
	);

	// Fish out request units from input estimate record
	// Fields  are
	//  xunit
	//  doppler
	//  xabs
	//  yunit
	//  elements
	//   i
	//     parameters
	//     errors
	//     fixed

	// SpectralElement fromRecord handles each numbered elements
	// field (type, parameters, errors). It does not yet handle
	// the 'fixed' field (see below)

	Bool xAbs = estimate.isDefined("xabs")
		? estimate.asBool("xabs")
		: True;
	// Figure out the abcissa type specifying what abcissa domain the fitter
	// is operating in.  Convert the CoordinateSystem to this domain
	// and set it back in the image
	String errMsg;
	ImageFit1D<Float>::AbcissaType abcissaType;
	Bool ok = ImageFit1D<Float>::setAbcissaState(
		errMsg, abcissaType, cSys, xUnit, doppler, axis
	);
	subImage.setCoordinateInfo(cSys);

	if (!ok) {
		*_log << LogIO::WARN << errMsg << LogIO::POST;
	}

	ImageFit1D<Float> fitter;
	if (pWeights) {
		fitter.setImage(subImage, *pWeights, axis);
	} else {
		fitter.setImage(subImage, axis);
	}
	// Set data region averaging data in region.  We could also set the
	// ImageRegion from that passed in to this function rather than making
	// a SubImage. But the way I have done it, the 'mask' keyword is
	// handled automatically there.
	Slicer sl(IPosition(nDim, 0), imageShape, Slicer::endIsLength);
	LCSlicer sl2(sl);
	ImageRegion region(sl2);
	if (! fitter.setData(region, abcissaType, xAbs)) {
		*_log << fitter.errorMessage() << LogIO::EXCEPTION;
	}

	// If we have the "elements" field, decode it into a list
	SpectralList list;
	if (estimate.isDefined("elements")) {
		if (!list.fromRecord(errMsg, estimate.asRecord("elements"))) {
			*_log << errMsg << LogIO::EXCEPTION;
		}

		// Handle the 'fixed' record here. This is a work around until we
		// redo this stuff properly in SpectralList and friends.
		Record tmpRec = estimate.asRecord("elements");
		const uInt nRec = tmpRec.nfields();
		AlwaysAssert(nRec == list.nelements(), AipsError);

		for (uInt i = 0; i < nRec; i++) {
			Record tmpRec2 = tmpRec.asRecord(i);
			Vector<Bool> fixed;
			if (tmpRec2.isDefined("fixed")) {
				fixed = tmpRec2.asArrayBool("fixed");
				SpectralElement& el = list[i];
				el.fix(fixed);
			}
		}
	}

	// Now we do one of three things:
	// 1) make a fit and evaluate
	// 2) evaluate a model
	// 3) make an estimate and evaluate
	Vector<Float> model(0), residual(0);
	Bool addExtras = False;
	Record recOut;

	if (fitIt) {
		if (list.nelements() > 0) {
			// Strip off any polynomial
			SpectralList list2;
			for (uInt i = 0; i < list.nelements(); i++) {
				if (list[i].getType() == SpectralElement::GAUSSIAN) {
					list2.add(list[i]);
				}
			}
			// Set estimate
			fitter.setElements(list2);
		} else {
			// Set auto estimate
			if (! fitter.setGaussianElements(ngauss)) {
				*_log << LogIO::WARN << fitter.errorMessage() << LogIO::POST;
			}
		}
		if (poly >= 0) {
			// Add baseline
			SpectralElement polyEl(poly);
			fitter.addElement(polyEl);
		}
		if (! fitter.fit()) {
			*_log << LogIO::WARN << "Fit failed to converge" << LogIO::POST;
		}
		if (! modelName.empty()) {
			model = fitter.getFit();
			ImageCollapser collapser(
				&subImage, Vector<uInt>(1, axis), True,
				ImageCollapser::ZERO, modelName, True
			);
			PagedImage<Float> *modelImage = static_cast<PagedImage<Float>*>(
												collapser.collapse(True)
											);
			modelImage->put(model.reform(modelImage->shape()));
			modelImage->flush();
			delete modelImage;
		}
		if (! residualName.empty()) {
			residual = fitter.getResidual(-1, True);
            ImageCollapser collapser(
				&subImage, Vector<uInt>(1, axis), True,
				ImageCollapser::ZERO, residualName, True
			);
			std::auto_ptr<PagedImage<Float> > residualImage(
				static_cast<PagedImage<Float>*>(
					collapser.collapse(True)
				)
			);
			residualImage->put(residual.reform(residualImage->shape()));
			residualImage->flush();
		}
		const SpectralList& fitList = fitter.getList(True);
		fitList.toRecord(recOut);
		addExtras = True;
	}
	else {
		if (list.nelements() > 0) {
			fitter.setElements(list); // Set list
			model = fitter.getEstimate(); // Evaluate list
			residual = fitter.getResidual(-1, False);
		}
		else {
			if (fitter.setGaussianElements(ngauss)) { // Auto estimate
				model = fitter.getEstimate(); // Evaluate
				residual = fitter.getResidual(-1, False);
				const SpectralList& list = fitter.getList(False);
				list.toRecord(recOut);
				addExtras = True;
			}
			else {
				*_log << LogIO::SEVERE << fitter.errorMessage()
					<< LogIO::POST;
			}
		}
	}
	return fitter;
}

Vector<ImageFit1D<Float> > ImageProfileFitter::_fitallprofiles(
	const Record &region, SubImage<Float> &subImage, String &xUnit, const Int axis,
	const String& mask, const Int nGauss, const Int poly,
	const String& weightsImageName, const String& modelImageName,
	const String& residImageName
) const {

	*_log << LogOrigin(_class, __FUNCTION__);

	Int baseline(poly);
	if (!(nGauss > 0 || baseline >= 0)) {
		*_log
			<< "You must specify a number of gaussians and/or a polynomial order to fit"
			<< LogIO::EXCEPTION;
	}

	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
	std::auto_ptr<Record> regionClone(dynamic_cast<Record *>(region.clone()));
	subImage = SubImage<Float>::createSubImage(
		*clone,
		*(ImageRegion::tweakedRegionRecord(regionClone.get())),
		mask, 0, True
	);
	IPosition imageShape = subImage.shape();
	PtrHolder<ImageInterface<Float> > weightsImage;
	//ImageInterface<Float>* pWeights = 0;
	if (! weightsImageName.empty()) {
		PagedImage<Float> sigmaImage(weightsImageName);
		if (!sigmaImage.shape().conform(_getImage()->shape())) {
			*_log << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		std::auto_ptr<ImageRegion> pR(
			ImageRegion::fromRecord(
				_log.get(), sigmaImage.coordinates(),
				sigmaImage.shape(), region
			)
		);
		weightsImage.set(new SubImage<Float> (sigmaImage, *pR, False));
		//pWeights = weightsImage.ptr();
	}
	// Set default axis
	CoordinateSystem cSys = subImage.coordinates();
	Int pAxis = CoordinateUtil::findSpectralAxis(cSys);
	Int axis2 = axis;
	if (axis2 < 0) {
		if (pAxis != -1) {
			axis2 = pAxis;
		} else {
			axis2 = subImage.ndim() - 1;
		}
	}
	// Create output images with a mask
	PtrHolder<ImageInterface<Float> > fitImage, residImage;
	ImageInterface<Float>* pFit = 0;
	ImageInterface<Float>* pResid = 0;
	if (ImageAnalysis::makeExternalImage(fitImage, modelImageName, cSys, imageShape, subImage,
			*_log, True, False, True))
		pFit = fitImage.ptr();
	if (ImageAnalysis::makeExternalImage(residImage, residImageName, cSys, imageShape,
			subImage, *_log, True, False, True))
		pResid = residImage.ptr();
	// Do fits
	// FIXME give users the option to show a progress bar
	Bool showProgress = False;
	uInt axis3(axis2);
	uInt nGauss2 = max(0, nGauss);
	return ImageUtilities::fitProfiles(
		pFit, pResid, xUnit, subImage,
		axis3, nGauss2, baseline, showProgress
	);

}

}

