//# Histogram2dDD.cc: 2d histogram DisplayData
//# Copyright (C) 2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/Arrays/ArrayMath.h>
#include <lattices/Lattices/LatticeHistograms.h>
#include <lattices/Lattices/LatticeStatistics.h>
#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <display/DisplayDatas/Histogram2dDD.h>
#include <display/DisplayDatas/Histogram2dDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	Histogram2dDD::Histogram2dDD(const ImageInterface<Float> *image) :
		itsBaseImage(0),
		itsMaskedLattice(0),
		itsLatticeStatistics(0),
		itsLatticeHistograms(0) {

		itsBaseImage = image->cloneII();
		itsMaskedLattice = itsBaseImage;

		// construct our statistics calculator
		itsLatticeStatistics =
		    new LatticeStatistics<Float>(*itsMaskedLattice, False);
		if (!itsLatticeStatistics) {
			throw(AipsError("Histogram2dDD::ctor - failed to create statistics"));
		}

		// now figure out a coordinate system
		Array<Float> tar;
		Vector<Double> linblc(2), lintrc(2);
		if (!itsLatticeStatistics->getConvertedStatistic(tar,LatticeStatsBase::MIN) ||
		        !tar.shape().nelements()) {
			linblc(0) = -1.0;
		} else {
			IPosition ipos(tar.ndim());
			ipos = 0;
			linblc(0) = tar(ipos);
		}
		if (!itsLatticeStatistics->getConvertedStatistic(tar,LatticeStatsBase::MAX) ||
		        !tar.shape().nelements()) {
			lintrc(0) = 1.0;
		} else {
			IPosition ipos(tar.ndim());
			ipos = 0;
			lintrc(0) = tar(ipos);
		}

		linblc(1) = 0;
		lintrc(1) = itsMaskedLattice->nelements();

		// scale lintrc(1) by inverse binwidth
		Float bwidth = (lintrc(0) - linblc(0)) / (100 - 1);
		if (bwidth != 0) {
			lintrc(1) = lintrc(1) / bwidth * 4;
		}

		DisplayCoordinateSystem newcsys;
		Vector<String> names(2);
		names(0) = "Data value";
		names(1) = "Number";
		Vector<String> units(2);
		units(0) = itsBaseImage->units().getName();
		units(1) = "_";
		//Vector<Double> refVal = linblc;
		//Vector<Double> inc = (lintrc - linblc) / 100.0;
		Matrix<Double> pc(2, 2);
		pc = 0.0;
		pc(0, 0) = pc(1, 1) = 1.0;
		//Vector<Double> refPix(2);
		//refPix = 0.0;

		Vector<Double> refVal = linblc;
		Vector<Double> inc(2);
		inc = 1.0;
		Vector<Double> refPix = linblc;

		LinearCoordinate lc(names, units, refVal, inc, pc, refPix);
		newcsys.addCoordinate(lc);

		//linblc = 0.0;
		//lintrc(0) = 5.0;
		//lintrc(1) = 100.0;
		//cerr << "linblc = " << linblc << ", lintrc = " << lintrc << endl;
		setCoordinateSystem(newcsys, linblc, lintrc);

		// construct our histogram calculator
		itsLatticeHistograms =
		    new LatticeHistograms<Float>(maskedLattice(), False, False);
		if (!itsLatticeHistograms) {
			throw(AipsError("Histogram2dDD::ctor - failed to create histograms"));
		}
		itsLatticeHistograms->setNBins(100);

	}

	Histogram2dDD::~Histogram2dDD() {
	}

	const Unit Histogram2dDD::dataUnit() {
		return itsBaseImage->units();
	}

	String Histogram2dDD::showValue(const Vector<Double> &/*world*/) {
		String retval;
		// IMPLEMENT THIS
		return retval;
	}

	void Histogram2dDD::setDefaultOptions() {
		ActiveCaching2dDD::setDefaultOptions();
	}

	Bool Histogram2dDD::setOptions(Record &rec, Record &recOut) {
		Bool ret = ActiveCaching2dDD::setOptions(rec, recOut);
		return ret;
	}

	Record Histogram2dDD::getOptions( bool scrub ) const {
		Record rec = ActiveCaching2dDD::getOptions(scrub);
		return rec;
	}

	CachingDisplayMethod *Histogram2dDD::newDisplayMethod(
	    WorldCanvas *worldCanvas,
	    AttributeBuffer *wchAttributes,
	    AttributeBuffer *ddAttributes,
	    CachingDisplayData *dd) {
		return new Histogram2dDM(worldCanvas, wchAttributes, ddAttributes, dd);
	}

	AttributeBuffer Histogram2dDD::optionsAsAttributes() {
		AttributeBuffer buffer = ActiveCaching2dDD::optionsAsAttributes();
		return buffer;
	}

	Histogram2dDD::Histogram2dDD() :
		ActiveCaching2dDD() {
		// MUST IMPLEMENT
	}

	Histogram2dDD::Histogram2dDD(const Histogram2dDD &o) : ActiveCaching2dDD(o) {
		// MUST IMPLEMENT
	}

	void Histogram2dDD::operator=(const Histogram2dDD &) {
		// MUST IMPLEMENT
	}

} //# NAMESPACE CASA - END

