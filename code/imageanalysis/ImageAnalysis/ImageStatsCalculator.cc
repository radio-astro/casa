//# t_subImage.cc: Test program for class _subImage
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
	), _statistics(), _oldStatsRegion(0), _oldStatsMask(0),
	_axes(), _includepix(), _excludepix(), _list(False),
	_disk(False), _robust(False), _verbose(False), _algConf(),
	_subImage(), _prefClassStatsAlg(AUTO) {
	_construct(beVerboseDuringConstruction);
	_setSupportsLogfile(True);
	_algConf.algorithm = StatisticsData::CLASSICAL;
}

ImageStatsCalculator::~ImageStatsCalculator() {}

Record ImageStatsCalculator::calculate() {
    *_getLog() << LogOrigin(_class, __func__);
	std::unique_ptr<vector<String> > messageStore( _getLogFile() ? new vector<String>() : nullptr );
	Record retval = statistics(messageStore.get());
	Bool writeFile = _openLogfile();
	if (_verbose || writeFile) {
		if (writeFile) {
			for (
				vector<String>::const_iterator iter=messageStore->begin();
				iter != messageStore->end(); ++iter
			) {
				_writeLogfile("# " + *iter, False, False);
			}
		}
		ImageCollapser<Float> collapsed(
			_subImage,
			_axes.nelements() == 0
				? IPosition::makeAxisPath(_getImage()->ndim()).asVector()
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

void ImageStatsCalculator::configureChauvenet(
	Double zscore, Int maxIterations
) {
	if (
		_algConf.algorithm != StatisticsData::CHAUVENETCRITERION
		|| ! near(_algConf.zs, zscore)
		|| maxIterations != _algConf.mi
	) {
		_algConf.algorithm = StatisticsData::CHAUVENETCRITERION;
		_algConf.zs = zscore;
		_algConf.mi = maxIterations;
		_statistics.reset();
	}
}

void ImageStatsCalculator::configureClassical(
	PreferredClassicalAlgorithm p
) {
	if (
		_algConf.algorithm != StatisticsData::CLASSICAL
		|| p != _prefClassStatsAlg
	) {
		_algConf.algorithm = StatisticsData::CLASSICAL;
		_prefClassStatsAlg = p;
		_statistics.reset();
	}
}

void ImageStatsCalculator::configureFitToHalf(
	FitToHalfStatisticsData::CENTER centerType,
	FitToHalfStatisticsData::USE_DATA useData,
	Double centerValue
) {
    if (
    	_algConf.algorithm != StatisticsData::FITTOHALF
    	|| _algConf.ct != centerType
        || _algConf.ud != useData
        || (
            centerType == FitToHalfStatisticsData::CVALUE
            && ! near(_algConf.cv, centerValue)
        )
    ) {
    	_algConf.algorithm = StatisticsData::FITTOHALF;
    	_algConf.ct = centerType;
    	_algConf.ud = useData;
    	_algConf.cv = centerValue;
        _statistics.reset();
    }
}

void ImageStatsCalculator::configureHingesFences(Double f) {
    if (
    	_algConf.algorithm != StatisticsData::HINGESFENCES
    	|| ! near(_algConf.hf, f)
    ) {
    	_algConf.algorithm = StatisticsData::HINGESFENCES;
    	_algConf.hf = f;
        _statistics.reset();
    }
}

void ImageStatsCalculator::setVerbose(Bool v) {
    if (_verbose != v) {
        _statistics.reset();
    }
	_verbose = v;
}

void ImageStatsCalculator::setDisk(Bool d) {
    if (_disk != d) {
        _statistics.reset();
    }
	_disk = d;
}

void ImageStatsCalculator::_reportDetailedStats(
	const SHARED_PTR<const ImageInterface<Float> > tempIm,
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
		++myloc;
	}
	if (freqCol >= 0) {
		idx[myloc] = freqCol;
		if (imShape[freqCol] > 1) {
			reportAxes.prepend(IPosition(1, freqCol));
		}
		++myloc;
	}
	if (decCol >= 0) {
		idx[myloc] = decCol;
		if (imShape[decCol] > 1) {
			reportAxes.prepend(IPosition(1, decCol));
		}
		++myloc;
	}
	if (raCol >= 0) {
		idx[myloc] = raCol;
		if (imShape[raCol] > 1) {
			reportAxes.prepend(IPosition(1, raCol));
		}
		myloc++;
	}
	if (otherCol.nelements() > 0) {
		for (uInt i=0; i<otherCol.nelements(); ++i) {
			idx[myloc] = otherCol[i];
			++myloc;
			if (imShape[otherCol[i]] > 1) {
				reportAxes.append(IPosition(1, otherCol[i]));
			}
		}
	}

	ostringstream oss;
	for (uInt i=0; i<reportAxes.nelements(); ++i) {
		String gg = worldAxes[reportAxes[i]];
		gg.upcase();
		uInt width = gg == "STOKES" ? 6 : gg == "FREQUENCY"?  16: 15;
		colwidth.push_back(width);
		oss << setw(width) << worldAxes[reportAxes[i]] << "  "
			<< worldAxes[reportAxes[i]] << "(Plane)" << " ";
		width = worldAxes[reportAxes[i]].size() + 8;
		colwidth.push_back(width);
	}
	Vector<Int> axesMap = reportAxes.asVector();
	GenSort<Int>::sort(axesMap);
	oss << "Npts          Sum           Mean          Rms           Std dev       Minimum       Maximum     ";
	std::map<String, uInt> chauvIters;
	if (_algConf.algorithm == StatisticsData::CHAUVENETCRITERION) {
		chauvIters = _statistics->getChauvenetNiter();
		oss << "  N Iter";
	}
	oss << endl;
	for (uInt i=0; i<7; ++i) {
		colwidth.push_back(12);
	}
	if (_algConf.algorithm == StatisticsData::CHAUVENETCRITERION) {
		colwidth.push_back(6);
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
	IPosition blc = _statistics->getBlc();
    IPosition position(tempIm->ndim());
	oss << std::scientific;
	uInt width = 13;
	Vector<Vector<String> > coords(reportAxes.size());
	for (uInt i=0; i<reportAxes.size(); ++i) {
		Vector<Double> indices = indgen(imShape[reportAxes[i]], 0.0, 1.0);
		uInt prec = reportAxes[i] == freqCol ? 9 : 5;
		ImageUtilities::pixToWorld(
			coords[i], csys, reportAxes[i], _axes,
			IPosition(imShape.nelements(),0), imShape-1, indices,prec,
			True
		);
	}
	uInt count = 0;
	for (inIter.reset(); ! inIter.atEnd(); ++inIter) {
		uInt colNum = 0;
		position = inIter.position();
		for (uInt i=0; i<reportAxes.nelements(); ++i) {
			oss << setw(colwidth[colNum]);
			oss	<< coords[i][position[reportAxes[i]]];// world[reportAxes[i]];
			++colNum;
			oss << " " << setw(colwidth[colNum])
				<< (position[reportAxes[i]] + blc[reportAxes[i]]) << " ";
			++colNum;
	    }
		csys.toWorld(world, position);
		if (axesMap.nelements() == 0) {
			arrayIndex = IPosition(1, 0);
		}
		else {
			for (uInt i=0; i<axesMap.nelements(); ++i) {
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
				<< std::setw(width) << retval.asArrayDouble("max")(arrayIndex);
			if (_algConf.algorithm == StatisticsData::CHAUVENETCRITERION) {
				ostringstream pos;
				pos << position;
				oss << std::setw(6) << " " << chauvIters[pos.str()];
				++count;
			}
			oss << endl;
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
	CountedPtr<ImageRegion> pRegionRegion, pMaskRegion;
	String mtmp = _getMask();
	if (mtmp == "false" || mtmp == "[]") {
		mtmp = "";
	}
	//SPIIF clone(_getImage()->cloneII());
    _subImage = SubImageFactory<Float>::createSubImageRO(
		pRegionRegion, pMaskRegion, *_getImage(), *_getRegion(), mtmp,
		(_verbose ? _getLog().get() : 0), AxesSpecifier(),
		_getStretch()
	);
    *_getLog() << myOrigin;
	// Find BLC of _subImage in pixels and world coords, and output the
	// information to the logger.
	// NOTE: ImageStatitics can't do this because it only gets the _subImage
	//       not a region and the full image.
	IPosition shape = _subImage->shape();
	IPosition blc(_subImage->ndim(), 0);
	IPosition trc(shape - 1);
	if (pRegionRegion) {
		LatticeRegion latRegion = pRegionRegion->toLatticeRegion(
			_getImage()->coordinates(), _getImage()->shape()
		);
		Slicer sl = latRegion.slicer();
		blc = sl.start();
		trc = sl.end();
	}
	// for precision
	CoordinateSystem csys = _getImage()->coordinates();
	//Bool hasDirectionCoordinate = (csys.findCoordinate(Coordinate::DIRECTION) >= 0);
	Int precis = -1;
	if (csys.hasDirectionCoordinate()) {
		DirectionCoordinate dirCoord = csys.directionCoordinate(0);
		Vector<String> dirUnits = dirCoord.worldAxisUnits();
		Vector<Double> dirIncs = dirCoord.increment();
		for (uInt i=0; i< dirUnits.size(); ++i) {
			Quantity inc(dirIncs[i], dirUnits[i]);
			inc.convert("s");
			Int newPrecis = abs(int(floor(log10(inc.getValue()))));
			precis = (newPrecis > 2 && newPrecis > precis) ? newPrecis : precis;
		}
	}

	String blcf, trcf;
	blcf = CoordinateUtil::formatCoordinate(blc, csys, precis);
	trcf = CoordinateUtil::formatCoordinate(trc, csys, precis);


    if (_statistics.get() == NULL) {
        _statistics.reset(
		    _verbose
			? new ImageStatistics<Float> (*_subImage, *_getLog(), True, _disk)
			: new ImageStatistics<Float> (*_subImage, True, _disk)
		);
	}
	else {
		_statistics->resetError();
		if (
		    _haveRegionsChanged(
				pRegionRegion.get(), pMaskRegion.get(),
				_oldStatsRegion.get(), _oldStatsMask.get()
			)
		) {
			_statistics->setNewImage(*_subImage);
		}
	}
    // prevent the table of stats we no longer use from being logged
    _statistics->setListStats(False);
    String myAlg = _configureAlgorithm();
	if (_list) {
		*_getLog() << myOrigin << LogIO::NORMAL;
		String algInfo = "Statistics calculated using "
			+ myAlg + " algorithm";
		*_getLog() << algInfo << LogIO::POST;
		if (messageStore) {
			messageStore->push_back(algInfo + "\n");
		}
		// Only write to the logger if the user wants it displayed.
		Vector<String> x(5);
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
		for (uInt i=0; i<x.size(); ++i) {
			*_getLog() << x[i] << LogIO::POST;
			if (messageStore != 0) {
				messageStore->push_back(x[i] + "\n");
			}
		}
	}
	if (messageStore != NULL) {
		_statistics->recordMessages(True);
	}
	_statistics->setPrecision(precis);
	_statistics->setBlc(blc);

	// Assign old regions to current regions
	_oldStatsMask.reset(0);

	_oldStatsRegion = pRegionRegion;
	_oldStatsMask = pMaskRegion;
	//_oldStatsStorageForce = _disk;
	// Set cursor axes
	*_getLog() << myOrigin;
	ThrowIf(! _statistics->setAxes(_axes), _statistics->errorMessage());
	ThrowIf(
		!_statistics->setInExCludeRange(_includepix, _excludepix, False),
		_statistics->errorMessage()
	);
	// Tell what to list
	ThrowIf(
		!_statistics->setList(_list),
		_statistics->errorMessage()
	);
	// Recover statistics
	Array<Double> npts, sum, sumsquared, min, max, mean, sigma;
	Array<Double> rms, fluxDensity, med, medAbsDevMed, quartile, q1, q3;
	Bool ok = True;
	Bool doFlux = True;
	if (_getImage()->imageInfo().hasMultipleBeams()) {
		if (csys.hasSpectralAxis() || csys.hasPolarizationCoordinate()) {
			Int spAxis = csys.spectralAxisNumber();
			Int poAxis = csys.polarizationAxisNumber();
			for (Int i=0; i<(Int)_axes.size(); ++i) {
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
	if (_robust) {
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
	if (_robust) {
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
		if (minPos.nelements() > 0) {
			statsout.define("minpos", (blc + minPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + minPos, csys, precis);
			statsout.define("minposf", tmp);
		}
		if (maxPos.nelements() > 0) {
			statsout.define("maxpos", (blc + maxPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + maxPos, csys, precis);
			statsout.define("maxposf", tmp);
		}
	}
	if (_list) {
		_statistics->showRobust(_robust);
		ThrowIf(
			!_statistics->display(),
			_statistics->errorMessage()
		);
	}
	if (messageStore != 0) {
		vector<String> messages = _statistics->getMessages();
		for (
			vector<String>::const_iterator iter=messages.begin();
			iter!=messages.end(); ++iter
		) {
			messageStore->push_back(*iter + "\n");
		}
		_statistics->clearMessages();
	}
	return statsout;
}

void ImageStatsCalculator::setRobust(Bool b) {
    _robust = b;
}

String ImageStatsCalculator::_configureAlgorithm() {
	String myAlg;
	switch (_algConf.algorithm) {
	case StatisticsData::CHAUVENETCRITERION:
		_statistics->configureChauvenet(_algConf.zs, _algConf.mi);
		myAlg = "Chauvenet Criterion/Z-score";
		break;
	case StatisticsData::CLASSICAL:
		switch (_prefClassStatsAlg) {
		case AUTO:
			_statistics->configureClassical();
			break;
		case TILED_APPLY:
			_statistics->configureClassical(0, 0, 1, 1);
			break;
		case STATS_FRAMEWORK:
			_statistics->configureClassical(1, 1, 0, 0);
			break;
		default:
			ThrowCc("Unhandled classical stats type");
		}
		myAlg = "Classic";
		break;
	case StatisticsData::FITTOHALF:
		_statistics->configureFitToHalf(_algConf.ct, _algConf.ud, _algConf.cv);
		myAlg = "Fit-to-Half";
		break;
	case StatisticsData::HINGESFENCES:
		_statistics->configureHingesFences(_algConf.hf);
		myAlg = "Hinges-Fences";
		break;
	default:
		ThrowCc(
			"Logic Error: Unhandled statistics algorithm "
			+ String::toString(_algConf.algorithm)
		);
	}
	return myAlg;
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

