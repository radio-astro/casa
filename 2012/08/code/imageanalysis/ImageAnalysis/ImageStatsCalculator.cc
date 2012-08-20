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

#include <iomanip>

#include <memory>

namespace casa {

const String ImageStatsCalculator::_class = "ImageStatsCalculator";


ImageStatsCalculator::ImageStatsCalculator(
	ImageAnalysis *const &ia,
	const String& region, const Record *const &regionPtr,
	const String& box, const String& chanInp,
	const String& stokes, const String& maskInp
) : ImageTask(
		ia->getImage(), region, regionPtr, box, chanInp,
		stokes, maskInp, "", False
	), _ia(ia), _axes(Vector<Int>(0)),_plotStats(Vector<String>(0)),
	_includepix(Vector<Float>(0)), _excludepix(Vector<Float>(0)),
	_plotter(""), _nx(1), _ny(1), _list(True), _force(False),
	_disk(False), _robust(False), _verbose(True) {
	_construct();
	_setSupportsLogfile(True);
}

ImageStatsCalculator::~ImageStatsCalculator() {}

Record ImageStatsCalculator::calculate() {
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	Record retval;
	Record region = *_getRegion();
	std::auto_ptr<vector<String> > messageStore(
		_getLogfile().empty() ? 0 : new vector<String>()
	);
	if (
		! _ia->statistics(
			retval, _axes, region, _getMask(),
			_plotStats, _includepix, _excludepix, _plotter,
			_nx, _ny, _list, _force, _disk, _robust, False,
			_getStretch(), messageStore.get()
		)
	) {
		*_getLog() << "Unable to determine statistics" << LogIO::EXCEPTION;
	}
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
		ImageCollapser collapsed(
			_getImage(),
			_axes.nelements() == 0
				? IPosition::makeAxisPath(_getImage()->ndim()).asVector()
				: _axes,
			False, ImageCollapser::ZERO, "", False
		);
		std::auto_ptr<ImageInterface<Float> > tempIm(
			collapsed.collapse(True)
		);
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
	return retval;
}



}

