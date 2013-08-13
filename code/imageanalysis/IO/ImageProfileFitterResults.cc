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

#include <imageanalysis/IO/ImageProfileFitterResults.h>

#include <casa/Utilities/Precision.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <images/Images/PagedImage.h>
#include <scimath/Mathematics/Combinatorics.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/IO/LogFile.h>

namespace casa {

const String ImageProfileFitterResults::_class = "ImageProfileFitterResults";
const String ImageProfileFitterResults::_CONVERGED = "converged";
const String ImageProfileFitterResults::_SUCCEEDED = "succeeded";
const String ImageProfileFitterResults::_VALID = "valid";

const uInt ImageProfileFitterResults::_nOthers = 2;
const uInt ImageProfileFitterResults::_gsPlane = 0;
const uInt ImageProfileFitterResults::_lsPlane = 1;


ImageProfileFitterResults::ImageProfileFitterResults(
	const std::tr1::shared_ptr<LogIO>& log, const CoordinateSystem& csysIm,
	const Array<ImageFit1D<Float> >* const &fitters, const SpectralList& nonPolyEstimates,
	const std::tr1::shared_ptr<const SubImage<Float> >& subImage, Int polyOrder, Int fitAxis,
	uInt nGaussSinglets, uInt nGaussMultiplets, uInt nLorentzSinglets,
	uInt nPLPCoeffs, uInt nLTPCoeffs,
	Bool logResults, Bool multiFit, const std::tr1::shared_ptr<LogFile>& logfile,
	const String& xUnit, const String& summaryHeader
) : _logResults(logResults), _multiFit(multiFit), _xUnit(xUnit), _summaryHeader(summaryHeader),
	_nGaussSinglets(nGaussSinglets), _nGaussMultiplets(nGaussMultiplets),
	_nLorentzSinglets(nLorentzSinglets), _nPLPCoeffs(nPLPCoeffs),_nLTPCoeffs(nLTPCoeffs),
	_fitters(fitters),
	_nonPolyEstimates(nonPolyEstimates), _subImage(subImage), _polyOrder(polyOrder),
	_fitAxis(fitAxis), _logfile(logfile), _log(log), _csysIm(csysIm) {}

ImageProfileFitterResults::~ImageProfileFitterResults() {}

Record ImageProfileFitterResults::getResults() const {
	return _results;
}

void ImageProfileFitterResults::createResults() {
	_setResults();
	_resultsToLog();
}

std::auto_ptr<vector<vector<Matrix<Double> > > > ImageProfileFitterResults::_createPCFMatrices() const {
	std::auto_ptr<vector<vector<Matrix<Double> > > > pcfMatrices(
		new vector<vector<Matrix<Double> > >(
			NGSOLMATRICES, vector<Matrix<Double> >(_nGaussMultiplets+_nOthers)
		)
	);
	uInt nSubcomps = 0;
	uInt compCount = 0;
	Double fNAN = casa::doubleNaN();

	Matrix<Double> blank;
	uInt nFitters = _fitters->size();
	for (uInt i=0; i<_nGaussMultiplets + _nOthers; i++) {
		if (i == _gsPlane) {
			// gaussian singlets go in position 0
			nSubcomps = _nGaussSinglets;
		}
		else if (i == _lsPlane) {
			// lorentzian singlets go in position 1
			nSubcomps = _nLorentzSinglets;
		}
		else {
			// gaussian multiplets go in positions 2 to 1 + _nGaussianMultiplets
			while (
				_nonPolyEstimates[compCount]->getType() != SpectralElement::GMULTIPLET
			) {
				compCount++;
			}
			nSubcomps = dynamic_cast<const GaussianMultipletSpectralElement*>(
					_nonPolyEstimates[compCount]
			)->getGaussians().size();
			compCount++;
		}
		blank.resize(nFitters, nSubcomps, False);
		blank = fNAN;
		for (uInt k=0; k<NGSOLMATRICES; k++) {
			(*pcfMatrices)[k][i] = blank.copy();
		}
	}
	return pcfMatrices;
}

Bool ImageProfileFitterResults::_setAxisTypes() {
	const CoordinateSystem& csysSub = _subImage->coordinates();

	const DirectionCoordinate *dcoord = csysSub.hasDirectionCoordinate()
		? &csysSub.directionCoordinate() : 0;
	String directionType = dcoord ? MDirection::showType(dcoord->directionType()) : "";
	const SpectralCoordinate *spcoord = csysSub.hasSpectralAxis()
		? &csysSub.spectralCoordinate() : 0;
	const StokesCoordinate *polcoord = csysSub.hasPolarizationCoordinate()
		? &csysSub.stokesCoordinate() : 0;
	Vector<String> axisNames = csysSub.worldAxisNames();
	for (uInt i=0; i<axisNames.size(); i++) {
		axisNames[i].upcase();
	}
	Bool hasLat = False;
	Bool hasLong = False;
	_axisTypes = vector<axisType>(axisNames.size(), NAXISTYPES);
	for (uInt i=0; i<axisNames.size(); i++) {
		if ((Int)i != _fitAxis) {
			if (
				dcoord
				&& (axisNames[i].startsWith("RIG") || axisNames[i].startsWith("LONG"))
			) {
				_axisTypes[i] = LONGITUDE;
				hasLat = True;
			}
			else if (
				dcoord
				&& (axisNames[i].startsWith("DEC") || axisNames[i].startsWith("LAT"))
			) {
				_axisTypes[i] = LATITUDE;
				hasLong = True;
			}
			else if (
				spcoord
				&& (axisNames[i].startsWith("FREQ") || axisNames[i].startsWith("VEL"))
			) {
				_axisTypes[i] = FREQUENCY;
			}
			else if (
				polcoord
				&& axisNames[i].startsWith("STO")
			) {
				_axisTypes[i] = POLARIZATION;
			}
		}
	}
	return hasLat && hasLong;
}

void ImageProfileFitterResults::_marshalFitResults(
	Array<Bool>& attemptedArr, Array<Bool>& successArr,
	Array<Bool>& convergedArr, Array<Bool>& validArr,
	Matrix<String>& typeMat, Array<Int>& niterArr,
	Array<Int>& nCompArr, std::auto_ptr<vector<vector<Matrix<Double> > > >& pcfMatrices,
	vector<Matrix<Double> >& plpMatrices, vector<Matrix<Double> >& ltpMatrices, Bool returnDirection,
    Vector<String>& directionInfo, Array<Bool>& mask
) {
    IPosition inTileShape = _subImage->niceCursorShape();
	TiledLineStepper stepper (_subImage->shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(*_subImage, stepper);
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters->begin();
	uInt count = 0;
	const CoordinateSystem& csysSub = _subImage->coordinates();
	const DirectionCoordinate *dcoord = csysSub.hasDirectionCoordinate()
		? &csysSub.directionCoordinate() : 0;
	String directionType = dcoord ? MDirection::showType(dcoord->directionType()) : "";
	const SpectralCoordinate *spcoord = csysSub.hasSpectralAxis()
		? &csysSub.spectralCoordinate() : 0;
	const StokesCoordinate *polcoord = csysSub.hasPolarizationCoordinate()
		? &csysSub.stokesCoordinate() : 0;
	IPosition pixel;
	Vector<Double> world;
	directionInfo.resize(returnDirection ? _fitters->size() : 0);
	String latitude, longitude;
	Double increment = fabs(_fitAxisIncrement());
	Vector<ImageFit1D<Float> >::const_iterator end = _fitters->end();
	for (
		inIter.reset();
		! inIter.atEnd() && fitter != end;
		inIter++, fitter++, count++
	) {
		IPosition idx(1, count);
		attemptedArr(idx) = fitter->getList().nelements() > 0;
		successArr(idx) = fitter->succeeded();
		convergedArr(idx) = attemptedArr(idx) && successArr(idx)
			&& fitter->converged();
		validArr(idx) = convergedArr(idx) && fitter->isValid();

		pixel = inIter.position();
		if (csysSub.toWorld(world, pixel)) {
			for (uInt i=0; i<world.size(); i++) {
				// The Coordinate::format() calls have the potential of modifying the
				// input unit so this needs to be reset at the beginning of the loop
				// rather than outside the loop
				String emptyUnit = "";
				if ((Int)i != _fitAxis) {
					if (_axisTypes[i] == LONGITUDE) {
						longitude = dcoord->format(emptyUnit, Coordinate::DEFAULT, world[i], 0, True, True);
						_worldCoords(LONGITUDE, count) = longitude;
					}
					else if (_axisTypes[i] == LATITUDE) {
						latitude = dcoord->format(emptyUnit, Coordinate::DEFAULT, world[i], 1, True, True);
						_worldCoords(LATITUDE, count) = latitude;
					}
					else if (_axisTypes[i] == FREQUENCY) {
						_worldCoords(FREQUENCY, count) = spcoord->format(emptyUnit, Coordinate::DEFAULT, world[i], 0, True, True);
					}
					else if (_axisTypes[i] == POLARIZATION) {
						_worldCoords(POLARIZATION, count) = polcoord->format(emptyUnit, Coordinate::DEFAULT, world[i], 0, True, True);
					}
				}
			}
			if (returnDirection) {
				directionInfo[count] = directionType + " "
					+ longitude + " " + latitude;
			}
		}
		else {
			LogOrigin logOrigin(_class, __FUNCTION__);
			*_log << "Could not convert pixel to world coordinate: "
				<< csysSub.errorMessage() << LogIO::EXCEPTION;
			}
		if (validArr(idx)) {
			IPosition subimPos = inIter.position();
			mask(idx) = anyTrue(inIter.getMask());
			niterArr(idx) = (Int)fitter->getNumberIterations();
			nCompArr(idx) = (Int)fitter->getList().nelements();
			SpectralList solutions = fitter->getList();
			uInt gCount = 0;
			uInt gmCount = 0;
			uInt lseCount = 0;
			for (uInt i=0; i<solutions.nelements(); i++) {
				SpectralElement::Types type = solutions[i]->getType();
				typeMat(count, i) = SpectralElement::fromType(type);
				switch (type) {
				case SpectralElement::POLYNOMIAL:
					break;
				case SpectralElement::GAUSSIAN:
					// allow fall through because gaussians and lorentzians use common code
				case SpectralElement::LORENTZIAN:
				{
					const PCFSpectralElement *pcf = dynamic_cast<
						const PCFSpectralElement*
					>(solutions[i]);
					uInt plane = _lsPlane;
					uInt col = lseCount;
					// if block because we allow case fall through
					if (type == SpectralElement::LORENTZIAN) {
						lseCount++;
					}
					else {
						plane = _gsPlane;
						col = gCount;
						gCount++;
					}
					_insertPCF(
						*pcfMatrices, plane, *pcf, count, col,
						subimPos, increment
					);
				}
				break;
				case SpectralElement::GMULTIPLET:
				{
					const GaussianMultipletSpectralElement *gm = dynamic_cast<
						const GaussianMultipletSpectralElement*
					>(solutions[i]);
					const Vector<GaussianSpectralElement> g = gm->getGaussians();
					for (uInt k=0; k<g.size(); k++) {
						_insertPCF(
							*pcfMatrices, gmCount + 2, g[k], count,
							k, subimPos, increment
						);
					}
					gmCount++;
				}
				break;
				case SpectralElement::POWERLOGPOLY:
				{
					Vector<Double> sols = solutions[i]->get();
					Vector<Double> errs = solutions[i]->getError();
					for (uInt j=0; j<_nPLPCoeffs; j++) {
						plpMatrices[SPXSOL](count, j) = sols[j];
						plpMatrices[SPXERR](count, j) = errs[j];
					}
				}
				break;
				case SpectralElement::LOGTRANSPOLY:
				{
					Vector<Double> sols = solutions[i]->get();
					Vector<Double> errs = solutions[i]->getError();
					for (uInt j=0; j<_nLTPCoeffs; j++) {
						ltpMatrices[SPXSOL](count, j) = sols[j];
						ltpMatrices[SPXERR](count, j) = errs[j];
					}
				}
				break;
				default:

					*_log << LogOrigin(_class, __FUNCTION__)
						<< "Logic Error: Unhandled Spectral Element type"
						<< LogIO::EXCEPTION;
					break;
				}
			}
		}
	}
}

void ImageProfileFitterResults::_writeLogfile(const String& str, Bool open, Bool close) {
	if (_logfile.get() != 0) {
		_logfile->write(str, open, close);
	}
}

void ImageProfileFitterResults::_setResults() {
    LogOrigin logOrigin(_class, __FUNCTION__);
    Double fNAN = casa::doubleNaN();
    uInt nComps = _nGaussSinglets + _nGaussMultiplets + _nLorentzSinglets;
    if (_polyOrder >= 0) {
    	nComps++;
    }
    if (_nPLPCoeffs > 0) {
    	nComps++;
    }
    if (_nLTPCoeffs > 0) {
    	nComps++;
    }
    uInt aSize = _fitters->size();
	Array<Bool> attemptedArr(IPosition(1, aSize), False);
	Array<Bool> successArr(IPosition(1, aSize), False);
	Array<Bool> convergedArr(IPosition(1, aSize), False);
	Array<Bool> validArr(IPosition(1, aSize), False);
	_worldCoords = Matrix<String>((Int)NAXISTYPES, aSize, "");
	Array<Int> niterArr(IPosition(1, aSize), -1);
	std::auto_ptr<vector<vector<Matrix<Double> > > > pcfMatrices = _createPCFMatrices();
	Matrix<Double> blank(aSize, max(_nPLPCoeffs, _nLTPCoeffs), fNAN);
	vector<Matrix<Double> > plpMatrices(NSPXSOLMATRICES);
	vector<Matrix<Double> > ltpMatrices(NSPXSOLMATRICES);

	for (uInt i=0; i<NSPXSOLMATRICES; i++) {
		// because assignmet of Arrays is by reference, which is horribly confusing
		if (_nPLPCoeffs > 0) {
			plpMatrices[i] = blank.copy();
		}
		if (_nLTPCoeffs > 0) {
			ltpMatrices[i] = blank.copy();
		}
	}
	Matrix<String> typeMat(aSize, nComps, "UNDEF");
	Array<Bool> mask(IPosition(1, aSize), False);
	Array<Int> nCompArr(IPosition(1, aSize), -1);
	Bool returnDirection = _setAxisTypes();
	Vector<String> directionInfo;
    _marshalFitResults(
		attemptedArr, successArr,
		convergedArr, validArr,
		typeMat, niterArr,
		nCompArr, pcfMatrices, plpMatrices, ltpMatrices,
        returnDirection, directionInfo, mask

	);
	{
		*_log << LogOrigin(_class, __FUNCTION__);
		ostringstream oss;
		oss << "Number of profiles       = " << _fitters->size();
		String str = oss.str();
		*_log << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", True, False);
		oss.str("");
		oss << "Number of fits attempted = " << ntrue(attemptedArr);
		str = oss.str();
		*_log << LogOrigin(_class, __FUNCTION__);
		*_log << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number succeeded         = " << ntrue(successArr);
		str = oss.str();
		*_log << LogOrigin(_class, __FUNCTION__);
		*_log << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number converged         = " << ntrue(convergedArr);
		str = oss.str();
		*_log << LogOrigin(_class, __FUNCTION__);
		*_log << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
		oss.str("");
		oss << "Number valid             = " << ntrue(validArr) << endl;
		str = oss.str();
		*_log << LogOrigin(_class, __FUNCTION__);
		*_log << LogIO::NORMAL << str << LogIO::POST;
		_writeLogfile(str + "\n", False, False);
	}
	Bool someConverged = anyTrue(convergedArr);
	String key;
	IPosition axes(1, _fitAxis);
	ImageCollapser collapser(
		_subImage, axes, False, ImageCollapser::ZERO, String(""), False
	);
	std::auto_ptr<TempImage<Float> > myTemplate(
		static_cast<TempImage<Float>* >(collapser.collapse(True))
	);
	IPosition shape = myTemplate->shape();
	_results.define("attempted", attemptedArr.reform(shape));
	_results.define(_SUCCEEDED, successArr.reform(shape));
	_results.define(_CONVERGED, convergedArr.reform(shape));
	_results.define(_VALID, validArr.reform(shape));
	_results.define("niter", niterArr.reform(shape));
	_results.define("ncomps", nCompArr.reform(shape));
	if (returnDirection) {
		_results.define("direction", directionInfo.reform(shape));
	}
	_results.define("xUnit", _xUnit);

	const String yUnit = _subImage->units().getName();
	_results.define("yUnit", yUnit);
	IPosition typeShape = shape;
	typeShape.resize(shape.size() + 1, True);
	typeShape[typeShape.size() - 1] = nComps;
	_results.define( "type", typeMat.reform(typeShape));

	for (uInt i=0; i<_nGaussMultiplets+_nOthers; i++) {
		if (i == _gsPlane && _nGaussSinglets == 0) {
			continue;
		}
		else if (i == _lsPlane && _nLorentzSinglets == 0) {
			continue;
		}
		Record rec;
		IPosition solArrShape = shape;
		solArrShape.resize(shape.size()+1, True);
		solArrShape[solArrShape.size()-1] = (*pcfMatrices)[AMP][i].shape()[(*pcfMatrices)[AMP][i].shape().size()-1];
		rec.define("center", (*pcfMatrices)[CENTER][i].reform(solArrShape));
		rec.define("fwhm", (*pcfMatrices)[FWHM][i].reform(solArrShape));
		rec.define("amp", (*pcfMatrices)[AMP][i].reform(solArrShape));
		rec.define("integral", (*pcfMatrices)[INTEGRAL][i].reform(solArrShape));
		rec.define("centerErr", (*pcfMatrices)[CENTERERR][i].reform(solArrShape));
		rec.define("fwhmErr", (*pcfMatrices)[FWHMERR][i].reform(solArrShape));
		rec.define("ampErr", (*pcfMatrices)[AMPERR][i].reform(solArrShape));
		rec.define("integralErr", (*pcfMatrices)[INTEGRALERR][i].reform(solArrShape));
		String description = i == _gsPlane
			? "Gaussian singlets results"
			: i == _lsPlane
			  ? "Lorentzian singlets"
			  : "Gaussian multiplet number " + String::toString(i-1) + " results";
		rec.define("description", description);
		_results.defineRecord(_getTag(i), rec);
	}
	if (_nPLPCoeffs > 0) {
		Record rec;
		IPosition solArrShape = shape;
		solArrShape.resize(shape.size()+1, True);
		solArrShape[solArrShape.size()-1] = _nPLPCoeffs;
		rec.define("solution", plpMatrices[SPXSOL].reform(solArrShape));
		rec.define("error", plpMatrices[SPXERR].reform(solArrShape));
		_results.defineRecord("plp", rec);
	}
	if (_nLTPCoeffs > 0) {
		Record rec;
		IPosition solArrShape = shape;
		solArrShape.resize(shape.size()+1, True);
		solArrShape[solArrShape.size()-1] = _nLTPCoeffs;
		rec.define("solution", ltpMatrices[SPXSOL].reform(solArrShape));
		rec.define("error", ltpMatrices[SPXERR].reform(solArrShape));
		_results.defineRecord("ltp", rec);
	}
	Bool writeSolutionImages = (
		! _centerName.empty() || ! _centerErrName.empty()
		|| ! _fwhmName.empty() || ! _fwhmErrName.empty()
		|| ! _ampName.empty() || ! _ampErrName.empty()
		|| ! _integralName.empty() || ! _integralErrName.empty()
		|| ! _plpName.empty() || ! _plpErrName.empty()
		|| ! _ltpName.empty() || ! _ltpErrName.empty()
	);
	if (
		! _multiFit && writeSolutionImages
	) {
		*_log << LogIO::WARN << "This was not a multi-pixel fit request so solution "
			<< "images will not be written" << LogIO::POST;
	}
	if (
		_multiFit && writeSolutionImages
	) {
		if (
			_nGaussSinglets + _nGaussMultiplets + _nLorentzSinglets + _nPLPCoeffs + _nLTPCoeffs == 0
		) {
			*_log << LogIO::WARN << "No functions for which solution images are supported were fit "
				<< "so no solution images will be written" << LogIO::POST;
		}
		else {
			if (someConverged) {
				_writeImages(myTemplate->coordinates(), mask, yUnit);
			}
			else {
				*_log << LogIO::WARN << "No solutions converged, solution images will not be written"
					<< LogIO::POST;
			}
		}
	}
}

String ImageProfileFitterResults::_getTag(const uInt i) const {
	return i == _gsPlane
		? "gs"
		: i == _lsPlane
		    ? "ls"
		    : "gm" + String::toString(i-2);
}

void ImageProfileFitterResults::_insertPCF(
	vector<vector<Matrix<Double> > >& pcfMatrices, /*Bool& isSolutionSane,*/
	const uInt idx, const PCFSpectralElement& pcf,
	const uInt row, const uInt col, const IPosition& pos,
	const Double increment/*, const uInt npix*/
) const {
	pcfMatrices[CENTER][idx](row, col) = _centerWorld(pcf, pos);
	pcfMatrices[FWHM][idx](row, col) = pcf.getFWHM() * increment;
	pcfMatrices[AMP][idx](row, col) = pcf.getAmpl();
	pcfMatrices[CENTERERR][idx](row, col) = pcf.getCenterErr() * increment;
	pcfMatrices[FWHMERR][idx](row, col) = pcf.getFWHMErr() * increment;
	pcfMatrices[AMPERR][idx](row, col) = pcf.getAmplErr();
	pcfMatrices[INTEGRAL][idx](row, col) = pcf.getIntegral() * increment;
	pcfMatrices[INTEGRALERR][idx](row, col) = pcf.getIntegralErr() * increment;
}

void ImageProfileFitterResults::_writeImages(
	const CoordinateSystem& xcsys,
	const Array<Bool>& mask, const String& yUnit
) const {
	// add a linear coordinate for the individual components
	Vector<Double> crpix(1, 0);
	Vector<Double> crval(1, 0);
	Vector<Double> cdelt(1, 1);
	Matrix<Double> pc(1, 1, 0);
	pc.diagonal() = 1.0;
	Vector<String> name(1, "Component Number");
	Vector<String> units(1, "");
	LinearCoordinate componentCoord(name, units, crval, cdelt, pc, crpix);
	CoordinateSystem mycsys = CoordinateSystem(xcsys);

	mycsys.addCoordinate(componentCoord);

	map<String, String> mymap;
	map<String, String> unitmap;
	mymap["center"] = _centerName;
	mymap["centerErr"] = _centerErrName;
	mymap["fwhm"] = _fwhmName;
	mymap["fwhmErr"] = _fwhmErrName;
	mymap["amp"] = _ampName;
	mymap["ampErr"] = _ampErrName;
	mymap["integral"] = _integralName;
	mymap["integralErr"] = _integralErrName;
	mymap["center"] = _centerName;

	unitmap["center"] = _xUnit;
	unitmap["centerErr"] = _xUnit;
	unitmap["fwhm"] = _xUnit;
	unitmap["fwhmErr"] = _xUnit;
	unitmap["amp"] = yUnit;
	unitmap["ampErr"] = yUnit;
	unitmap["integral"] = _xUnit + "." + yUnit;
	unitmap["integralErr"] = _xUnit + "." + yUnit;
	for (uInt i=0; i<_nGaussMultiplets+_nOthers; i++) {
		if (i == _gsPlane && _nGaussSinglets == 0) {
			continue;
		}
		else if (i == _lsPlane && _nLorentzSinglets == 0) {
			continue;
		}
		String id = _getTag(i);
		IPosition maskShape = _results.asRecord(id).asArrayDouble("amp").shape();
		Array<Bool> fMask(maskShape);
		uInt n = maskShape[maskShape.size()-1];
		maskShape[maskShape.size()-1] = 1;
		Array<Bool> reshapedMask = mask.reform(maskShape);
		AlwaysAssert(ntrue(mask) == ntrue(reshapedMask), AipsError);

		IPosition shape = fMask.shape();
		IPosition begin(shape.size(), 0);
		IPosition end = shape - 1;
		for (uInt j=0; j<n; j++) {
			begin[shape.size() - 1] = j;
			end[shape.size() - 1] = j;
			fMask(begin, end) = reshapedMask;
		}

		for (
			map<String, String>::const_iterator iter=mymap.begin();
			iter!=mymap.end(); iter++
		) {
			String imagename = iter->second;
			String suffix = i == _gsPlane
				? ""
				: i == _lsPlane
				  ? "_ls"
				  : _nGaussMultiplets <= 1
				    ? "_gm"
				    : "_gm" + String::toString(i-_nOthers);
			imagename += suffix;
			if (! iter->second.empty()) {
				_makeSolutionImage(
					imagename, mycsys,
					_results.asRecord(id).asArrayDouble(iter->first),
					unitmap.find(iter->first)->second, fMask
				);
			}
		}
	}
	if (_nPLPCoeffs > 0 && (! _plpName.empty() || ! _plpErrName.empty())) {
		IPosition maskShape = _results.asRecord("plp").asArrayDouble("solution").shape();
		Array<Bool> fMask(maskShape);
		maskShape[maskShape.size() - 1] = 1;
		Array<Bool> reshapedMask = mask.reform(maskShape);
		AlwaysAssert(ntrue(mask) == ntrue(reshapedMask), AipsError);
		if (! _plpName.empty()) {
			_makeSolutionImage(
				_plpName, mycsys,
				_results.asRecord("plp").asArrayDouble("solution"),
				"", fMask
			);
		}
		if (! _plpErrName.empty()) {
			_makeSolutionImage(
				_plpErrName, mycsys,
				_results.asRecord("plp").asArrayDouble("error"),
				"", fMask
			);
		}
	}
	if (_nLTPCoeffs > 0 && (! _ltpName.empty() || ! _ltpErrName.empty())) {
		IPosition maskShape = _results.asRecord("ltp").asArrayDouble("solution").shape();
		Array<Bool> fMask(maskShape);
		maskShape[maskShape.size() - 1] = 1;
		Array<Bool> reshapedMask = mask.reform(maskShape);
		AlwaysAssert(ntrue(mask) == ntrue(reshapedMask), AipsError);
		if (! _ltpName.empty()) {
			_makeSolutionImage(
				_ltpName, mycsys,
				_results.asRecord("ltp").asArrayDouble("solution"),
				"", fMask
			);
		}
		if (! _ltpErrName.empty()) {
			_makeSolutionImage(
				_ltpErrName, mycsys,
				_results.asRecord("ltp").asArrayDouble("error"),
				"", fMask
			);
		}
	}
}

Bool ImageProfileFitterResults::_inVelocitySpace() const {
	return _fitAxis == _subImage->coordinates().spectralAxisNumber()
		&& Quantity(1, _xUnit).isConform("m/s");
}

Double ImageProfileFitterResults::_fitAxisIncrement() const {
	if (_inVelocitySpace()) {
		Vector<Double> pixels(2);
		pixels[0] = 0;
		pixels[1] = 1;
		Vector<Double> velocities(2);
		_subImage->coordinates().spectralCoordinate().pixelToVelocity(
			velocities, pixels
		);
		return velocities[1] - velocities[0];
	}
	else {
		return _subImage->coordinates().increment()[_fitAxis];
	}
}

const Vector<Double> ImageProfileFitterResults::getPixelCenter( uint index ) const {
	Vector<Double> pos;
	if ( index < _pixelPositions.size()){
		pos = _pixelPositions[index];
	}
	return pos;
}

Double ImageProfileFitterResults::_centerWorld(
    const PCFSpectralElement& solution, const IPosition& imPos
) const {
	Vector<Double> pixel(imPos.size());
	for (uInt i=0; i<pixel.size(); i++) {
		pixel[i] = imPos[i];
	}
	Vector<Double> world(pixel.size());
	// in pixels here
	pixel[_fitAxis] = solution.getCenter();
	_subImage->coordinates().toWorld(world, pixel);
	if (_inVelocitySpace()) {
		Double velocity;
		_subImage->coordinates().spectralCoordinate().frequencyToVelocity(velocity, world(_fitAxis));
		return velocity;
	}
	else {
		return world(_fitAxis);
	}
}

void ImageProfileFitterResults::_resultsToLog() {
	if (! _logResults && _logfile.get() == 0) {
		return;
	}
	ostringstream summary;
	summary << "****** Fit performed at " << Time().toString() << "******" << endl << endl;
	summary << _summaryHeader;
	if (_nPLPCoeffs + _nLTPCoeffs == 0) {
		if (_goodAmpRange.size() == 2) {
			summary << "       --- valid amplitude range: " << _goodAmpRange << endl;
		}
		if (_goodCenterRange.size() == 2) {
			summary << "       --- valid center range: " << _goodCenterRange << endl;
		}
		if (_goodFWHMRange.size() == 2) {
			summary << "       --- valid FWHM range: " << _goodFWHMRange << endl;
		}
		summary << "       --- number of Gaussian singlets: " << _nGaussSinglets << endl;
		summary << "       --- number of Gaussian multiplets: " << _nGaussMultiplets << endl;
		if (_nGaussMultiplets > 0) {
			for (uInt i=0; i<_nGaussMultiplets; i++) {
				Array<Double> amp = _results.asRecord("gm" + String::toString(i)).asArrayDouble(AMP);
				uInt n = amp.shape()[amp.ndim()-1];
				summary << "           --- number of components in Gaussian multiplet "
					<< i << ": " << n << endl;
			}
		}
		if (_polyOrder >= 0) {
			summary << "       --- polynomial order:    " << _polyOrder << endl;
		}
		else {
			summary << "       --- no polynomial fit " << endl;
		}
	}
	if (_multiFit) {
		summary << "       --- Multiple profiles fit, one per pixel over selected region" << endl;
	}
	else {
		summary << "       --- One profile fit, averaged over several pixels if necessary" << endl;
	}
	if (_logResults) {
		*_log << LogIO::NORMAL << summary.str() << LogIO::POST;
	}
	_writeLogfile(summary.str(), False, False);
	IPosition inTileShape = _subImage->niceCursorShape();
	TiledLineStepper stepper (_subImage->shape(), inTileShape, _fitAxis);
	RO_MaskedLatticeIterator<Float> inIter(*_subImage, stepper);
	CoordinateSystem csysSub = _subImage->coordinates();
	Vector<Double> worldStart;
	if (! csysSub.toWorld(worldStart, inIter.position())) {
		*_log << csysSub.errorMessage() << LogIO::EXCEPTION;
	}
	Vector<Double> pixStart;
	if (! _csysIm.toPixel(pixStart, worldStart)) {
		*_log << _csysIm.errorMessage() << LogIO::EXCEPTION;
	}
	if (_multiFit) {
		for (uInt i=0; i<pixStart.size(); i++) {
			pixStart[i] = (Int)std::floor( pixStart[i] + 0.5);
		}
	}
	Vector<ImageFit1D<Float> >::const_iterator fitter = _fitters->begin();
	Vector<Double> imPix(pixStart.size());
	Vector<Double> world;
	IPosition subimPos;
	SpectralList solutions;
	String axisUnit = _csysIm.worldAxisUnits()[_fitAxis];
	Int pixPrecision = _multiFit ? 0 : 3;
	_pixelPositions.resize( _fitters->size());
	uint index = 0;
	Vector<ImageFit1D<Float> >::const_iterator end = _fitters->end();

	for (
		inIter.reset();
		! inIter.atEnd() && fitter != end;
		inIter++, fitter++, index++
	) {
		summary.str("");
		Int basePrecision = summary.precision(1);
		summary.precision(basePrecision);
		subimPos = inIter.position();
		if (csysSub.toWorld(world, subimPos)) {
			summary << "Fit   :" << endl;
			for (uInt i=0; i<world.size(); i++) {
				if ((Int)i != _fitAxis) {
					if (_axisTypes[i] == LONGITUDE) {
						summary << "    RA           :   "
							<< _worldCoords(LONGITUDE, index) << endl;
					}
					else if (_axisTypes[i] == LATITUDE) {
						summary << "    Dec          :  "
							<< _worldCoords(LATITUDE, index) << endl;
					}
					else if (_axisTypes[i] == FREQUENCY) {
						summary << "    Freq         : "
							<< _worldCoords(FREQUENCY, index) <<  endl;
					}
					else if (_axisTypes[i] == POLARIZATION) {
						// stokes
						summary << "    Stokes       : " << _worldCoords(POLARIZATION, index) << endl;
//							<< Stokes::name(Stokes::type((Int)world[i])) << endl;
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
		_pixelPositions[index] = imPix;
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
				summary << "    Iterations   : " << fitter->getNumberIterations() << endl;
				String valid = fitter->isValid() ? "YES" : "NO";
				summary << "    Valid        : " << valid << endl;
				if (fitter->isValid()) {
					solutions = fitter->getList();
					for (uInt i=0; i<solutions.nelements(); i++) {
						SpectralElement::Types type = solutions[i]->getType();
						if (solutions.nelements() > 1) {
							summary << "    Results for component " << i << ":" << endl;
						}
						switch(type) {
						case SpectralElement::GAUSSIAN:
							// allow fall through; gaussians and lorentzians use the same
							// method for output
						case SpectralElement::LORENTZIAN:
							{
								const PCFSpectralElement *pcf
									= dynamic_cast<const PCFSpectralElement*>(solutions[i]);
								summary << _pcfToString(
									pcf, _csysIm, world.copy(), subimPos
								);
							}
							break;
						case SpectralElement::GMULTIPLET:
							{
								const GaussianMultipletSpectralElement *gm
									= dynamic_cast<const GaussianMultipletSpectralElement*>(solutions[i]);
								summary << _gaussianMultipletToString(
									*gm, _csysIm, world.copy(), subimPos
								);
								break;
							}
						case SpectralElement::POLYNOMIAL:
							{
								const PolynomialSpectralElement *p
									= dynamic_cast<const PolynomialSpectralElement*>(solutions[i]);
								summary << _polynomialToString(*p, _csysIm, imPix, world);
							}
							break;
						case SpectralElement::POWERLOGPOLY:
							{
								const PowerLogPolynomialSpectralElement *p
									= dynamic_cast<const PowerLogPolynomialSpectralElement*>(solutions[i]);
								summary << _powerLogPolyToString(*p);
							}
							break;
						case SpectralElement::LOGTRANSPOLY:
							{
								const LogTransformedPolynomialSpectralElement *p
								= dynamic_cast<const LogTransformedPolynomialSpectralElement*>(solutions[i]);
								summary << _logTransPolyToString(*p);
							}
							break;
						default:
							*_log << LogOrigin(_class, __FUNCTION__)
								<< "Logic Error: Unhandled spectral element type"
							    << LogIO::EXCEPTION;
							break;
						}
					}
				}
			}
		}
    	if (_logResults) {
    		*_log << LogIO::NORMAL << summary.str() << endl << LogIO::POST;
    	}
    	_writeLogfile(summary.str(), False, False);
	}
	if (_logfile.get() != 0) {
		_logfile->closeFile();
	}
}

String ImageProfileFitterResults::_elementToString(
	const Double value, const Double error,
	const String& unit, Bool isFixed
) const {
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
	ostringstream out;
    out << std::fixed << setprecision(precision);
    out << qVal.getValue();
    if (isFixed && qErr.getValue() == 0) {
    	out << " (fixed)";
    }
    else {
    	out << " +/- " << qErr.getValue();
    }
    out << " " << qVal.getUnit();
    return out.str();
}

String ImageProfileFitterResults::_pcfToString(
	const PCFSpectralElement *const &pcf, const CoordinateSystem& csys,
	const Vector<Double> world, const IPosition imPos,
	const Bool showTypeString, const String& indent
) const {
	Vector<Double> myWorld = world;
    String yUnit = _subImage->units().getName();
	ostringstream summary;
	Vector<Bool> fixed = pcf->fixed();
	if (showTypeString) {
		summary << indent << "        Type     : "
			<< SpectralElement::fromType(pcf->getType()) << endl;
	}
	summary << indent << "        Peak     : "
		<< _elementToString(
			pcf->getAmpl(), pcf->getAmplErr(), yUnit, fixed[0]
		) << endl;
	Double center = _centerWorld(
	    *pcf, imPos
	);

	Double increment = fabs(_fitAxisIncrement());

	Double centerErr = pcf->getCenterErr() * increment;
	Double fwhm = pcf->getFWHM() * increment;
	Double fwhmErr = pcf->getFWHMErr() * increment;

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
	summary << indent << "        Center   : "
		<< _elementToString(
			center, centerErr, _xUnit, fixed[1]
		) << endl;
	if (convertedCenterToPix) {
		summary << indent << "                   "
			<< _elementToString(
				pCenter, pCenterErr, "pixel", fixed[1]
			) << endl;
	}
	else {
		summary << indent << "                  Could not convert world to pixel for center" << endl;
	}
	summary << indent << "        FWHM     : "
		<< _elementToString(
			fwhm, fwhmErr, _xUnit, fixed[2]
		)
		<< endl;
	if (convertedFWHMToPix) {
		summary << indent << "                   "
			<< _elementToString(pFWHM, pFWHMErr, "pixel", fixed[2])
			<< endl;
	}
	else {
		summary << indent << "                  Could not convert FWHM to pixel" << endl;
	}
	Double integral = pcf->getIntegral()*increment;
	Double integralErr = pcf->getIntegralErr()*increment;
	String integUnit = (Quantity(1.0 ,yUnit)*Quantity(1.0, _xUnit)).getUnit();
	summary << indent << "        Integral : "
		<< _elementToString(integral, integralErr, integUnit, fixed[0] && fixed[2])
		<< endl;
	return summary.str();
}

String ImageProfileFitterResults::_gaussianMultipletToString(
	const GaussianMultipletSpectralElement& gm,
	const CoordinateSystem& csys,
	const Vector<Double> world, const IPosition imPos
) const {
	Vector<GaussianSpectralElement> g = gm.getGaussians();
	ostringstream summary;
	summary << "        Type     : GAUSSIAN MULTIPLET" << endl;
	for (uInt i=0; i<g.size(); i++) {
		summary << "        Results for subcomponent "
			<< i << ":" << endl;
		summary
			<< _pcfToString(&g[i], csys, world, imPos, False, "    ")
			<< endl;
	}
	return summary.str();
}

String ImageProfileFitterResults::_polynomialToString(
	const PolynomialSpectralElement& poly, const CoordinateSystem& csys,
	const Vector<Double> imPix, const Vector<Double> world
) const {
	ostringstream summary;
	summary << "        Type: POLYNOMIAL" << endl;
	Vector<Double> parms, errs;
	poly.get(parms);
	poly.getError(errs);
	for (uInt j=0; j<parms.size(); j++) {
		String unit = _subImage->units().getName();
        if (j > 0) {
			unit = unit + "/((pixel)" + String::toString(j) + ")";
		}
		summary << "         c" << j << " : "
            << _elementToString(parms[j], errs[j], unit, False) << endl;
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
		String unit = _subImage->units().getName();
		if (j > 0 ) {
			unit = unit + "/" + "((" + _xUnit + ")" + String::toString(j) + ")";
		}
        summary << _elementToString(pCoeff[j], pCoeffErr[j], unit, False) << endl;
    }
	return summary.str();
}


String ImageProfileFitterResults::_powerLogPolyToString(
	const PowerLogPolynomialSpectralElement& plp
) const {
	ostringstream summary;
	summary << "    Type         : POWER LOGARITHMIC POLYNOMIAL" << endl;
	summary << "    Function     : c0*(x/D)**(c1";
	uInt nElements = plp.get().size();
	for (uInt i=2; i<nElements; i++) {
		summary << " + c" << i << "*ln(x/D)";
		if (i > 2) {
			summary << "**" << (i - 1);
		}
	}
	summary << ")" << endl;
	summary << "    D            : " << _plpDivisor << endl;
	Vector<Double> parms = plp.get();
	Vector<Double> errs = plp.getError();
	Vector<Bool> fixed = plp.fixed();

	for (uInt j=0; j<parms.size(); j++) {
		summary << "    c" << j << "           : "
            << _elementToString(parms[j], errs[j], "", fixed[j]) << endl;
	}
	return summary.str();
}

String ImageProfileFitterResults::_logTransPolyToString(
	const LogTransformedPolynomialSpectralElement& ltp
) const {
	ostringstream summary;
	summary << "    Type         : LOGARITHMIC TRANSFORMED POLYNOMIAL" << endl;
	summary << "    Function     : c0 + ";
	uInt nElements = ltp.get().size();
	for (uInt i=1; i<nElements; i++) {
		summary << " + c" << i << "*ln(x/D)";
		if (i > 2) {
			summary << "**" << i;
		}
	}
	summary << ")" << endl;
	summary << "    D            : " << _plpDivisor << endl;
	Vector<Double> parms = ltp.get();
	Vector<Double> errs = ltp.getError();
	Vector<Bool> fixed = ltp.fixed();

	for (uInt j=0; j<parms.size(); j++) {
		summary << "    c" << j << "           : "
            << _elementToString(parms[j], errs[j], "", fixed[j]) << endl;
	}
	return summary.str();
}


void ImageProfileFitterResults::_makeSolutionImage(
	const String& name, const CoordinateSystem& csys,
	const Array<Double>& values, const String& unit,
	const Array<Bool>& mask
) {
	IPosition shape = values.shape();
	PagedImage<Float> image(shape, csys, name);
	Vector<Float> dataCopy(values.size());
	Vector<Double>::const_iterator iter;
	// isNaN(Array<Double>&) works, isNaN(Array<Float>&) gives spurious results
	Array<Bool> nanInfMask = ! (isNaN(values) || isInf(values));

	Vector<Float>::iterator jiter = dataCopy.begin();
	for (iter=values.begin(); iter!=values.end(); iter++, jiter++) {
		*jiter = (Float)*iter;
	}
	image.put(dataCopy.reform(shape));
	Bool hasPixMask = ! allTrue(mask);
	Bool hasNanMask = ! allTrue(nanInfMask);
	if (hasNanMask || hasPixMask) {
		Array<Bool> resMask(shape);
		String maskName = image.makeUniqueRegionName(
			String("mask"), 0
		);
		image.makeMask(maskName, True, True, False);
		if (hasPixMask) {
			resMask = mask.copy().reform(shape);
			if (hasNanMask) {
				resMask = resMask && nanInfMask;
			}
		}
		else {
			resMask = nanInfMask;
		}
		(&image.pixelMask())->put(resMask);
	}
	image.setUnits(Unit(unit));
}

}
