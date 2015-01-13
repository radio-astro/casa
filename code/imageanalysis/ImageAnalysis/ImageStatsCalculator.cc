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

#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>

#include <casa/BasicSL/String.h>
#include <images/Images/ImageUtilities.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <stdcasa/cboost_foreach.h>

#include <iomanip>

namespace casa {

const String ImageStatsCalculator::_class = "ImageStatsCalculator";


ImageStatsCalculator::ImageStatsCalculator(
	const SPCIIF image,
	const Record *const &regionPtr,
	const String& maskInp,
	Bool beVerboseDuringConstruction
) : ImageTask<Float>(
		image, "", regionPtr, "", "",
		"", maskInp, "", False
	), _statistics(0), _oldStatsRegion(0), _oldStatsMask(0),
	_oldStatsStorageForce(False),
	_axes(),
	_includepix(), _excludepix(),
	_list(False), _force(False),
	_disk(False), _robust(False), _verbose(False), _algorithm(StatisticsData::CLASSICAL), _algConf() {
	_construct(beVerboseDuringConstruction);
	_setSupportsLogfile(True);
}

ImageStatsCalculator::~ImageStatsCalculator() {}

Record ImageStatsCalculator::calculate() {
	*_getLog() << LogOrigin(_class, __func__);
	std::auto_ptr<vector<String> > messageStore(
		_getLogFile() == 0 ? 0 : new vector<String>()
	);
	Record retval = statistics(messageStore.get());
	Bool writeFile = _openLogfile();
	if (_verbose || writeFile) {
		if (writeFile) {
			for (
				vector<String>::const_iterator iter=messageStore->begin();
				iter != messageStore->end(); iter++
			) {
				_writeLogfile("# " + *iter, False, False);
			}
		}
		SPIIF clone(_getImage()->cloneII());
		ImageCollapser<Float> collapsed(
			clone,
			_axes.nelements() == 0
				? IPosition::makeAxisPath(clone->ndim()).asVector()
				: _axes,
			False, ImageCollapserData::ZERO, "", False
		);
		SPIIF tempIm;
		try {
			tempIm = collapsed.collapse();
		}
		catch (const AipsError& x) {
			*_getLog() << LogIO::WARN << "Unable to collapse image "
				<< "so detailed per plane statistics reporting is not "
				<< "possible. The exception message was " << x.getMesg()
				<< LogIO::POST;
		}
		if (tempIm) {
			_reportDetailedStats(tempIm, retval);
		}
	}
	return retval;
}

void ImageStatsCalculator::configureHingesFences(Double f) {
	_algConf.define("hf", f);
}

void ImageStatsCalculator::setAlgorithm(
	StatisticsData::ALGORITHM algorithm
) {
	_algorithm = algorithm;
}

void ImageStatsCalculator::_reportDetailedStats(
	const std::tr1::shared_ptr<const ImageInterface<Float> > tempIm,
	const Record& retval
) {
	CoordinateSystem csys = tempIm->coordinates();
	Vector<String> worldAxes = csys.worldAxisNames();
	IPosition imShape = tempIm->shape();
	vector<uInt> colwidth;
	Int stokesCol = -1;
	Int freqCol = -1;
	Int raCol = -1;
	Int decCol = -1;
	IPosition otherCol;

	for (Int i=worldAxes.size()-1; i>=0; i--) {
		String gg = worldAxes[i];
		gg.upcase();
		if (gg == "RIGHT ASCENSION") {
			raCol = i;
		}
		else if (gg == "DECLINATION") {
			decCol = i;
		}
		else if (gg == "FREQUENCY") {
			freqCol = i;
		}
		else if (gg == "STOKES") {
			stokesCol = i;
		}
		else {
			otherCol.append(IPosition(1, i));
		}
	}
	IPosition idx(worldAxes.size(), 0);
	uInt myloc = 0;
	IPosition reportAxes;
	if (stokesCol >= 0) {
		idx[myloc] = stokesCol;
		if (imShape[stokesCol] > 1) {
			reportAxes.prepend(IPosition(1, stokesCol));
		}
		myloc++;
	}
	if (freqCol >= 0) {
		idx[myloc] = freqCol;
		if (imShape[freqCol] > 1) {
			reportAxes.prepend(IPosition(1, freqCol));
		}
		myloc++;
	}
	if (decCol >= 0) {
		idx[myloc] = decCol;
		if (imShape[decCol] > 1) {
			reportAxes.prepend(IPosition(1, decCol));
		}
		myloc++;
	}
	if (raCol >= 0) {
		idx[myloc] = raCol;
		if (imShape[raCol] > 1) {
			reportAxes.prepend(IPosition(1, raCol));
		}
		myloc++;
	}
	if (otherCol.nelements() > 0) {
		for (uInt i=0; i<otherCol.nelements(); i++) {
			idx[myloc] = otherCol[i];
			myloc++;
			if (imShape[otherCol[i]] > 1) {
				reportAxes.append(IPosition(1, otherCol[i]));
			}
		}
	}

	ostringstream oss;
	oss << "# ";
	for (uInt i=0; i<reportAxes.nelements(); i++) {
		String gg = worldAxes[reportAxes[i]];
		gg.upcase();
		uInt width = gg == "STOKES" ? 6 : gg == "FREQUENCY"?  16: 15;
		colwidth.push_back(width);
		oss << setw(width) << worldAxes[reportAxes[i]] << "  "
			<< worldAxes[reportAxes[i]] << "(Plane)" << " ";
		width = worldAxes[reportAxes[i]].size() + 9;
		colwidth.push_back(width);
	}
	Vector<Int> axesMap = reportAxes.asVector();
	GenSort<Int>::sort(axesMap);
	oss << "Npts          Sum           Mean          Rms           Std dev       Minimum       Maximum     " << endl;
	for (uInt i=0; i<7; i++) {
		colwidth.push_back(12);
	}
	TileStepper ts(
		tempIm->niceCursorShape(),
		IPosition(tempIm->ndim(), 1), idx
	);
	RO_MaskedLatticeIterator<Float> inIter(
		*tempIm, ts
	);
	Vector<Double> world;
	IPosition arrayIndex(axesMap.nelements(), 0);
	IPosition position(tempIm->ndim(), 0);
	oss << std::scientific;
	uInt width = 13;
	Vector<Vector<String> > coords(reportAxes.size());
	for (uInt i=0; i<reportAxes.size(); i++) {
		Vector<Double> indices(imShape[reportAxes[i]]);
		indgen(indices);
		uInt prec = reportAxes[i] == freqCol ? 9 : 5;
		ImageUtilities::pixToWorld(
			coords[i], csys, reportAxes[i], _axes,
			IPosition(imShape.nelements(),0), imShape-1, indices,prec,
			True
		);
	}
	for (inIter.reset(); ! inIter.atEnd(); inIter++) {
		uInt colNum = 0;
		position = inIter.position();
		for (uInt i=0; i<reportAxes.nelements(); i++) {
			oss << setw(colwidth[colNum]);
			oss	<< coords[i][position[reportAxes[i]]];// world[reportAxes[i]];
			colNum++;
			oss << " " << setw(colwidth[colNum])
				<< position[reportAxes[i]] << " ";
			colNum++;
	    }
		csys.toWorld(world, position);
		if (axesMap.nelements() == 0) {
			arrayIndex = IPosition(1, 0);
		}
		else {
			for (uInt i=0; i<axesMap.nelements(); i++) {
				arrayIndex[i] = position[axesMap[i]];
			}
		}
		if (retval.asArrayDouble("npts").size() == 0) {
			oss << "NO VALID POINTS FOR WHICH TO DETERMINE STATISTICS" << endl;
		}
		else {
			oss << std::setw(width) << retval.asArrayDouble("npts")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("sum")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("mean")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("rms")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("sigma")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("min")(arrayIndex) << " "
				<< std::setw(width) << retval.asArrayDouble("max")(arrayIndex) << endl;
		}
		if (_verbose) {
			*_getLog() << LogIO::NORMAL << oss.str() << LogIO::POST;
		}
		_writeLogfile(oss.str(), False, False);
		oss.str("");
	}
	_closeLogfile();
}

Record ImageStatsCalculator::statistics(
	vector<String> *const &messageStore
) {
	LogOrigin myOrigin(_class, __func__);
	*_getLog() << myOrigin;
	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;
	String mtmp = _getMask();
	if (mtmp == "false" || mtmp == "[]") {
		mtmp = "";
	}
	SPIIF clone(_getImage()->cloneII());
	Record regionRec = *_getRegion();
	SubImage<Float> subImage = SubImageFactory<Float>::createSubImage(
		pRegionRegion, pMaskRegion, *clone, regionRec, mtmp,
		(_verbose ? _getLog().get() : 0), False, AxesSpecifier(),
		_getStretch()
	);
	*_getLog() << myOrigin;

	// Find BLC of subimage in pixels and world coords, and output the
	// information to the logger.
	// NOTE: ImageStatitics can't do this because it only gets the subimage
	//       not a region and the full image.
	IPosition blc(subImage.ndim(), 0);
	IPosition trc(subImage.shape() - 1);
	if (pRegionRegion != 0) {
		LatticeRegion latRegion = pRegionRegion->toLatticeRegion(
				_getImage()->coordinates(), _getImage()->shape());
		Slicer sl = latRegion.slicer();
		blc = sl.start();
		trc = sl.end();
	}
	// for precision
	CoordinateSystem csys = _getImage()->coordinates();
	Bool hasDirectionCoordinate = (csys.findCoordinate(Coordinate::DIRECTION) >= 0);
	Int precis = -1;
	if (hasDirectionCoordinate) {
		DirectionCoordinate dirCoord = csys.directionCoordinate(0);
		Vector<String> dirUnits = dirCoord.worldAxisUnits();
		Vector<Double> dirIncs = dirCoord.increment();
		for (uInt i=0; i< dirUnits.size(); i++) {
			Quantity inc(dirIncs[i], dirUnits[i]);
			inc.convert("s");
			Int newPrecis = abs(int(floor(log10(inc.getValue()))));
			precis = (newPrecis > 2 && newPrecis > precis) ? newPrecis : precis;
		}
	}

	String blcf, trcf;
	blcf = CoordinateUtil::formatCoordinate(blc, csys, precis);
	trcf = CoordinateUtil::formatCoordinate(trc, csys, precis);

	if (_list) {
		// Only write to the logger if the user wants it displayed.
		Vector<String> x(5);
		*_getLog() << myOrigin << LogIO::NORMAL;
		ostringstream y;
		x[0] = "Regions --- ";
		y << "         -- bottom-left corner (pixel) [blc]:  " << blc;
		x[1] = y.str();
		y.str("");
		y << "         -- top-right corner (pixel) [trc]:    " << trc;
		x[2] = y.str();
		y.str("");
		y << "         -- bottom-left corner (world) [blcf]: " << blcf;
		x[3] = y.str();
		y.str("");
		y << "         -- top-right corner (world) [trcf]:   " << trcf;
		x[4] = y.str();
		for (uInt i=0; i<x.size(); i++) {
			*_getLog() << x[i] << LogIO::POST;
			if (messageStore != 0) {
				messageStore->push_back(x[i] + "\n");
			}
		}
	}

	// Make new statistics object only if we need to.    This code is getting
	// a bit silly. I should rework it somewhen.
	Bool forceNewStorage = _force;
	if (_statistics.get() != 0 && _disk != _oldStatsStorageForce) {
		forceNewStorage = True;
	}
	if (forceNewStorage) {
		_statistics.reset(
			_verbose
			? new ImageStatistics<Float> (subImage, *_getLog(), False, _disk)
			: new ImageStatistics<Float> (subImage, False, _disk)
		);
	}
	else {
		if (_statistics.get() == 0) {
			// We are here if this is the first time or the image has
			// changed (_statistics is deleted then)

			_statistics.reset(
				_verbose
				? new ImageStatistics<Float> (subImage, *_getLog(), False, _disk)
				: new ImageStatistics<Float> (subImage, False, _disk)
			);
			/*
			Array<Double> debugMax;
			_statistics->getStatistic(debugMax, LatticeStatsBase::MAX);
			*/
		}
		else {
			// We already have a statistics object.  We only have to set
			// the new image (which will force the accumulation image
			// to be recomputed) if the region has changed.  If the image itself
			// changed, _statistics will already have been set to 0

			Bool reMake = (_verbose && !_statistics->hasLogger())
								|| (!_verbose && _statistics->hasLogger());
			if (reMake) {
				_statistics.reset(
					_verbose
					? new ImageStatistics<Float> (subImage, *_getLog(), True, _disk)
					: new ImageStatistics<Float> (subImage, True, _disk)
				);

			}
			else {
				_statistics->resetError();
				if (
					_haveRegionsChanged(
						pRegionRegion, pMaskRegion,
						_oldStatsRegion.get(), _oldStatsMask.get()
					)
				) {
					_statistics->setNewImage(subImage);
				}
			}
		}
	}
	_statistics->setAlgorithm(_algorithm);
	if (_algorithm == StatisticsData::HINGESFENCES && _algConf.isDefined(("hf"))) {
		_statistics->configureHingesFences(_algConf.asDouble("hf"));
	}
	if (messageStore != NULL) {
		_statistics->recordMessages(True);
	}
	_statistics->setPrecision(precis);
	_statistics->setBlc(blc);

	// Assign old regions to current regions
	_oldStatsMask.reset(0);

	_oldStatsRegion.reset(pRegionRegion);
	_oldStatsMask.reset(pMaskRegion);
	_oldStatsStorageForce = _disk;
	// Set cursor axes
	*_getLog() << LogOrigin(_class, __func__);
	ThrowIf(! _statistics->setAxes(_axes), _statistics->errorMessage());
	ThrowIf(
		!_statistics->setInExCludeRange(_includepix, _excludepix, False),
		_statistics->errorMessage()
	);

	// Tell what to list
	if (!_statistics->setList(_list)) {
		*_getLog() << _statistics->errorMessage() << LogIO::EXCEPTION;
	}

	// Recover statistics
	Array<Double> npts, sum, sumsquared, min, max, mean, sigma;
	Array<Double> rms, fluxDensity, med, medAbsDevMed, quartile, q1, q3;
	Bool ok = True;

	Bool trobust(_robust);
	Bool doFlux = True;
	if (_getImage()->imageInfo().hasMultipleBeams()) {
		if (csys.hasSpectralAxis() || csys.hasPolarizationCoordinate()) {
			Int spAxis = csys.spectralAxisNumber();
			Int poAxis = csys.polarizationAxisNumber();
			for (Int i=0; i<(Int)_axes.size(); i++) {
				if (_axes[i] == spAxis || _axes[i] == poAxis) {
					*_getLog() << LogIO::WARN << "At least one cursor axis contains multiple beams. "
							<< "You should thus use care in interpreting these statistics. Flux densities "
							<< "will not be computed." << LogIO::POST;
					doFlux = False;
					break;
				}
			}
		}
	}
	if (trobust) {
		ok = _statistics->getStatistic(med, LatticeStatsBase::MEDIAN)
			&& _statistics->getStatistic(
				medAbsDevMed, LatticeStatsBase::MEDABSDEVMED
			)
			&& _statistics->getStatistic(
				quartile, LatticeStatsBase::QUARTILE
			)
			&& _statistics->getStatistic(
				q1, LatticeStatsBase::Q1
			)
			&& _statistics->getStatistic(
				q3, LatticeStatsBase::Q3
			);
	}
	if (ok) {
		ok = _statistics->getStatistic(npts, LatticeStatsBase::NPTS)
			&& _statistics->getStatistic(sum, LatticeStatsBase::SUM)
			&& _statistics->getStatistic(sumsquared, LatticeStatsBase::SUMSQ)
			&& _statistics->getStatistic(min, LatticeStatsBase::MIN)
			&& _statistics->getStatistic(max, LatticeStatsBase::MAX)
			&& _statistics->getStatistic(mean, LatticeStatsBase::MEAN)
			&& _statistics->getStatistic(sigma, LatticeStatsBase::SIGMA)
			&& _statistics->getStatistic(rms, LatticeStatsBase::RMS);
	}
	ThrowIf(! ok, _statistics->errorMessage());
	Record statsout;
	statsout.define("npts", npts);
	statsout.define("sum", sum);
	statsout.define("sumsq", sumsquared);
	statsout.define("min", min);
	statsout.define("max", max);
	statsout.define("mean", mean);
	if (trobust) {
		statsout.define("median", med);
		statsout.define("medabsdevmed", medAbsDevMed);
		statsout.define("quartile", quartile);
		statsout.define("q1", q1);
		statsout.define("q3", q3);
	}
	statsout.define("sigma", sigma);
	statsout.define("rms", rms);
	if (
		doFlux
		&& _statistics->getStatistic(
			fluxDensity, LatticeStatsBase::FLUX
		)
	) {
		statsout.define("flux", fluxDensity);
	}
	statsout.define("blc", blc.asVector());
	statsout.define("blcf", blcf);

	statsout.define("trc", trc.asVector());
	statsout.define("trcf", trcf);

	String tmp;
	IPosition minPos, maxPos;
	if (_statistics->getMinMaxPos(minPos, maxPos)) {
		if (minPos.nelements() > 0 && maxPos.nelements() > 0) {
			statsout.define("minpos", (blc + minPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + minPos, csys, precis);
			statsout.define("minposf", tmp);
			statsout.define("maxpos", (blc + maxPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + maxPos, csys, precis);
			statsout.define("maxposf", tmp);
		}
	}
	if (_list) {
		_statistics->showRobust(trobust);
		ThrowIf(
			!_statistics->display(),
			_statistics->errorMessage()
		);
	}
	if (messageStore != 0) {
		vector<String> messages = _statistics->getMessages();
		for (
				vector<String>::const_iterator iter=messages.begin();
				iter!=messages.end(); iter++
		) {
			messageStore->push_back(*iter + "\n");
		}
		_statistics->clearMessages();
	}
	return statsout;
}

Bool ImageStatsCalculator::_haveRegionsChanged(
	ImageRegion* pNewRegion,
	ImageRegion* pNewMask, ImageRegion* pOldRegion,
	ImageRegion* pOldMask
) {
	Bool regionChanged = (
			pNewRegion != 0 && pOldRegion != 0
			&& (*pNewRegion) != (*pOldRegion)
		)
		|| (pNewRegion == 0 && pOldRegion != 0)
		|| (pNewRegion != 0 && pOldRegion == 0
	);
	Bool maskChanged = (
			pNewMask != 0 && pOldMask != 0
			&& (*pNewMask) != (*pOldMask)
		)
		|| (pNewMask == 0 && pOldMask != 0)
		|| (pNewMask != 0 && pOldMask == 0
	);
	return (regionChanged || maskChanged);
}

}

