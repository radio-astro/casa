//# Histogram2dDM.cc: drawing for 2d Histogram DisplayDatas
//# Copyright (C) 2000,2001
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
#include <lattices/LatticeMath/LatticeHistograms.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/Histogram2dDM.h>
#include <display/DisplayDatas/Histogram2dDD.h>
#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	Histogram2dDM::Histogram2dDM(WorldCanvas *worldCanvas,
	                             AttributeBuffer *wchAttributes,
	                             AttributeBuffer *ddAttributes,
	                             CachingDisplayData *dd) :
		CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
	}

	Histogram2dDM::~Histogram2dDM() {
		cleanup();
	}

	Bool Histogram2dDM::drawIntoList(Display::RefreshReason /*reason*/,
	                                 WorldCanvasHolder &wcHolder) {
		// Locate the WorldCanvas to draw upon
		WorldCanvas *wc = wcHolder.worldCanvas();

		Histogram2dDD *parent = dynamic_cast<Histogram2dDD *>
		                        (parentDisplayData());
		if (!parent) {
			throw(AipsError("invalid parent of Histogram2dDM"));
		}

		// determine low and high limits from canvas setup
		//Vector<Double> world(2), lin(2);
		Vector<Float> include(2);
		//lin(0) = wc->linXMin();
		//lin(1) = wc->linYMin();
		//wc->linToWorld(world, lin);
		//include(0) = static_cast<Float>(world(0));
		//lin(0) = wc->linXMax();
		//lin(1) = wc->linYMax();
		//wc->linToWorld(world, lin);
		//include(1) = static_cast<Float>(world(0));
		include(0) = wc->linXMin();
		include(1) = wc->linXMax();

		cerr << "setting include range to " << include << endl;
		// set histogram domain
		parent->itsLatticeHistograms->setIncludeRange(include);

		// re-evaluate histogram
		Array<Float> values, counts;
		cerr << "asking for histogram/s" << endl;
		if (!parent->itsLatticeHistograms->getHistograms(values, counts)) {
			throw(AipsError("could not calculate histogram"));
		}

		if ((values.shape().nelements() != 1) ||
		        (counts.shape().nelements() != 1)) {
			throw(AipsError("incorrect shape for histogram results"));
		}

		// scale the result by the inverse of the binwidth, so that
		// zooming works properly.
		Float bwidth = (include(1) - include(0)) /
		               static_cast<Float>(counts.nelements() - 1);
		if (bwidth != 0) {
			counts = counts / bwidth;
		}

		// draw the histogram!
		uInt nbins = values.shape()(0);
		Bool delete1, delete2;
		const Float *x, *data;
		x = values.getStorage(delete1);
		data = counts.getStorage(delete2);
		cpgbin(nbins, x, data, 1);
		values.freeStorage(x, delete1);
		counts.freeStorage(data, delete2);
		return True;
	}

	void Histogram2dDM::cleanup() {
	}

	Histogram2dDM::Histogram2dDM() {
	}

	Histogram2dDM::Histogram2dDM(const Histogram2dDM &other) :
		CachingDisplayMethod(other) {
	}

	void Histogram2dDM::operator=(const Histogram2dDM &) {
	}


} //# NAMESPACE CASA - END

