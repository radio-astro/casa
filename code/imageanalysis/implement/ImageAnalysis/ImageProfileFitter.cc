
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

ImageProfileFitter::ImageProfileFitter(
	const ImageInterface<Float> *const &image, const String& region,
	const Record *const &regionPtr,	const String& box,
	const String& chans, const String& stokes,
	const String& mask, const Int axis, const Bool multiFit,
	const String& residual, const String& model, const uInt ngauss,
	const Int polyOrder, const String& ampName,
	const String& ampErrName, const String& centerName,
	const String& centerErrName, const String& fwhmName,
	const String& fwhmErrName, const uInt minGoodPoints
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
	_minGoodPoints(minGoodPoints), _results(Record()) {
    _checkNGaussAndPolyOrder();
    _construct();
    _finishConstruction();
}

ImageProfileFitter::~ImageProfileFitter() {}

Record ImageProfileFitter::fit() {
    LogOrigin logOrigin(_class, __FUNCTION__);
    *_log << logOrigin;
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
    		_getMask(), 0, False
    	);
    }
	Record estimate;
	String weightsImageName = "";
	try {
		if (_multiFit) {
			// FIXME need to be able to specify the weights image
			_fitters = _fitallprofiles(weightsImageName);
		    *_log << logOrigin;
		}
		else {
			ImageFit1D<Float> fitter = _fitProfile(estimate);
		    *_log << logOrigin;
			IPosition axes(1, _fitAxis);
			ImageCollapser collapser(
				&_subImage, axes, True,
				ImageCollapser::MEAN, "", True
			);
			std::auto_ptr<ImageInterface<Float> > x(
				collapser.collapse(True)
			);
			_subImage = SubImage<Float>::createSubImage(
				*x, Record(), "", _log.get(),
				False, AxesSpecifier(), False
			);
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
	Array<Bool> attemptedArr(IPosition(1, _fitters.size()), False);
	Array<Bool> convergedArr(IPosition(1, _fitters.size()), False);
	Array<Int> niterArr(IPosition(1, _fitters.size()), -1);
    Matrix<Double> centerMat(_fitters.size(), nComps, NAN);
	Matrix<Double> fwhmMat(_fitters.size(), nComps, NAN);
	Matrix<Double> ampMat(_fitters.size(), nComps, NAN);
	Matrix<Double> centerErrMat(_fitters.size(), nComps, NAN);
	Matrix<Double> fwhmErrMat(_fitters.size(), nComps, NAN);
	Matrix<Double> ampErrMat(_fitters.size(), nComps, NAN);
	Matrix<String> typeMat(_fitters.size(), nComps, "UNDEF");
	Array<Bool> mask(IPosition(1, _fitters.size()), False);
	Array<Int> nCompArr(IPosition(1, _fitters.size()), -1);
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
		IPosition idx(1, count);
		attemptedArr(idx) = fitter->getList().nelements() > 0;
		convergedArr(idx) = attemptedArr(idx) && fitter->converged();
		if (fitter->converged()) {
			mask(idx) = anyTrue(inIter.getMask());
			niterArr(idx) = (Int)fitter->getNumberIterations();
			nCompArr(idx) = (Int)fitter->getList().nelements();
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
		)
	) {
		*_log << LogIO::WARN << "This was not a multi-pixel fit request so solution "
			<< "images will not be written" << LogIO::POST;
	}
	if (
		_multiFit && ! someConverged && (
			! _centerName.empty() || ! _centerErrName.empty()
			|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
			|| ! _ampName.empty() || ! _ampErrName.empty()
		)
	) {
		*_log << LogIO::WARN << "No solutions converged, solution images will not be written"
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
		key = "centerErr" + num;
		_results.define(key, centerErrMat.column(i).reform(shape));
		key = "fwhmErr" + num;
		_results.define(key, fwhmErrMat.column(i).reform(shape));
		key = "ampErr" + num;
		_results.define(key, ampErrMat.column(i).reform(shape));
		key = "type" + num;
		_results.define(key, typeMat.column(i).reform(shape));
		if (
			_multiFit && someConverged
			&& solutions[i].getType() == SpectralElement::GAUSSIAN
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
					if (axesNames[i].startsWith("RIG")) {
						// right ascension
						summary << "    RA         :   "
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
		Bool attempted = fitter->getList().nelements() > 0;
		summary << "    Attempted  : "
			<< (attempted ? "YES" : "NO") << endl;
		if (attempted) {
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
	const String& name, const CoordinateSystem& csys,
	const Array<Double>& values, const String& unit,
	const Array<Bool>& mask
) {
	IPosition shape = values.shape();
	PagedImage<Float> image(shape, csys, name);
	Vector<Float> dataCopy(values.size());
	Vector<Double>::const_iterator iter;
	Vector<Float>::iterator jiter = dataCopy.begin();
	for (iter=values.begin(); iter!=values.end(); iter++, jiter++) {
		*jiter = (Float)*iter;
	}
	image.put(dataCopy.reform(shape));
	Array<Bool> nanMask = ! isNaN(image.get());
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
	const Record& estimate,
	const Bool fitIt, const String weightsImageName
) {
	*_log << LogOrigin(_class, __FUNCTION__);
	PtrHolder<ImageInterface<Float> > weightsImagePtrHolder;
	ImageInterface<Float> *pWeights = 0;
	if (! weightsImageName.empty()) {
		PagedImage<Float> weightsImage(weightsImageName);
		if (! weightsImage.shape().conform(_getImage()->shape())) {
			*_log << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		std::auto_ptr<ImageRegion> pR(
			ImageRegion::fromRecord(
				_log.get(), weightsImage.coordinates(),
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
		errMsg, abcissaType, cSys, _xUnit, doppler, _fitAxis
	);
	_subImage.setCoordinateInfo(cSys);

	if (!ok) {
		*_log << LogIO::WARN << errMsg << LogIO::POST;
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
			if (! fitter.setGaussianElements(_ngauss)) {
				*_log << LogIO::WARN << fitter.errorMessage() << LogIO::POST;
			}
		}
		if (_polyOrder >= 0) {
			// Add baseline
			SpectralElement polyEl(_polyOrder);
			fitter.addElement(polyEl);
		}
		if (! fitter.fit()) {
			*_log << LogIO::WARN << "Fit failed to converge" << LogIO::POST;
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
			if (fitter.setGaussianElements(_ngauss)) { // Auto estimate
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
	const String& weightsImageName
) {
	*_log << LogOrigin(_class, __FUNCTION__);
	Int baseline = _polyOrder;
	IPosition imageShape = _subImage.shape();
	PtrHolder<ImageInterface<Float> > weightsImage;
	std::auto_ptr<TempImage<Float> > pWeights(0);
	if (! weightsImageName.empty()) {
		PagedImage<Float> sigmaImage(weightsImageName);
		if (!sigmaImage.shape().conform(_getImage()->shape())) {
			*_log << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		std::auto_ptr<ImageRegion> pR(
			ImageRegion::fromRecord(
				_log.get(), sigmaImage.coordinates(),
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
		} else {
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
		*_log, True, False, True
		)
	) {
		pFit = fitImage.ptr();
	}
	if (
		ImageAnalysis::makeExternalImage(
			residImage, _residual, cSys, imageShape,
			_subImage, *_log, True, False, True
		)
	) {
		pResid = residImage.ptr();
	}
	// Do fits
	// FIXME give users the option to show a progress bar
	Bool showProgress = False;
	uInt axis3(axis2);
	uInt nGauss2 = max((uInt)0, _ngauss);
	return ImageUtilities::fitProfiles(
		pFit, pResid, _xUnit, _subImage, axis3,
		nGauss2, baseline, pWeights.get(),
		showProgress, _minGoodPoints
	);
}

}
