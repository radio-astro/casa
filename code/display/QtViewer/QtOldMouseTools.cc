//# QtOldMouseTools.cc: Qt versions of display library mouse tools.
//# Copyright (C) 2005
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
//#


#include <display/QtViewer/QtOldMouseTools.qo.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>

namespace casa { //# NAMESPACE CASA - BEGIN




	void QtOldRTRegion::regionReady() {
		Record mouseRegion;
		WorldCanvasHolder* wch = 0;

		if(getMouseRegion(mouseRegion, wch)) {
			emit mouseRegionReady(mouseRegion, wch);
		}
	}



	Bool QtOldRTRegion::getMouseRegion(Record& mouseRegion,
	                                   WorldCanvasHolder*& wch) {
		// Retrieve the current polygon mouse region record and WCH, if any.
		// Retrieve the current rectangular mouse region record and WCH, if any.
		// (If nothing is ready, returns False).

		// Here are fields of a typical mouseRegion record
		// returned by this routine:
		//
		// type: String "box"
		// zindex: Int 17
		// pixel: subRecord {
		//   blc: Double array with shape [2]    [243, 132]
		//   trc: Double array with shape [2]    [357, 248]  }
		// linear: subRecord {
		//   blc: Double array with shape [2]    [155.052, 61.1335]
		//   trc: Double array with shape [2]    [256.965, 164.749]  }
		// (-->following field may be missing if undefined)
		// world: subRecord {
		//   blc: Double array with shape [2]    [4.66672, 1.22307]
		//   trc: Double array with shape [2]    [4.6609, 1.22508]
		//   units: String array with shape [2]  ["rad", "rad"]  }

		mouseRegion = Record();	// Initialize to empty Record.


		if(!rectangleDefined() || itsCurrentWC==0) return False;
		wch = pd_->wcHolder(itsCurrentWC);
		// Only reason pd_ is 'needed' by this tool (it shouldn't need it):
		// locating the important coordinate state 'zindex' on wch
		// (inaccessible from WC), instead of on WC, was a blunder....
		if(wch==0) return False;


		mouseRegion.define("type", "box");


		Int zindex = 0;
		if (wch->restrictionBuffer()->exists("zIndex")) {
			wch->restrictionBuffer()->getValue("zIndex", zindex);
		}

		mouseRegion.define("zindex", zindex);


		Record pixel, linear, world;

		static Vector<Double> pix(2), lin(2), wld(2);
		Int x1, y1, x2, y2;

		get(x1, y1, x2, y2);

		pix(0) = min(x1, x2);
		pix(1) = min(y1, y2);
		if(!itsCurrentWC->pixToLin(lin, pix)) return False;	// (unlikely)
		Bool wldOk = itsCurrentWC->linToWorld(wld, lin);

		pixel.define("blc", pix);
		linear.define("blc", lin);
		if(wldOk) world.define("blc", wld);

		pix(0) = max(x1, x2);
		pix(1) = max(y1, y2);
		if(!itsCurrentWC->pixToLin(lin, pix)) return False;	// (unlikely)
		if(wldOk) wldOk = itsCurrentWC->linToWorld(wld, lin);

		pixel.define("trc", pix);
		linear.define("trc", lin);
		if(wldOk) {
			world.define("trc", wld);
			world.define("units", itsCurrentWC->worldAxisUnits());
		}

		mouseRegion.defineRecord("pixel", pixel);
		mouseRegion.defineRecord("linear", linear);
		if(wldOk) mouseRegion.defineRecord("world", world);
		// Receiver may be able to use the mouse region even if, e.g.,
		// it is outside world coordinate boundaries (a modest
		// handwave toward support of all-sky images).

		return True;
	}


//void QtOldRTRegion::handleEvent(DisplayEvent& ev) {
//  cout << "QtOldRTRegion::handleEvent" << endl;
//}

//void QtOldRTRegion::keyPressed(const WCPositionEvent &ev) {
//  cout << "QtOldRTRegion pressed========" << endl;
//}

//void QtOldRTRegion::rectangleReady() {
//  cout << "QtOldRTRegion rectangle ready" << endl;
//}

	void QtOldRTRegion::clicked(Int /*x*/, Int /*y*/) {
		//this has same implementation as doubleClicked
		//we use single 'clicked' for activation
		return;
	}

	void QtOldRTRegion::doubleClicked(Int x, Int y) {
		//cout << "QtOldPTRegion rectangle " << x << " " << y <<  endl;
		if (itsCurrentWC==0)
			return ;
		WorldCanvasHolder* wch = pd_->wcHolder(itsCurrentWC);
		if (wch == 0)
			return ;

		Vector<Double> pix(2), lin(2), wld(2);
		pix(0) = x;
		pix(1) = y;
		//cout << "pix=(" << pix(0) << ", " << pix(1) << ")" <<  endl;
		if (!itsCurrentWC->pixToLin(lin, pix))
			return ;

		//cout << "lin=(" << lin(0) << ", " << lin(1) << ")" <<  endl;
		Bool wldOk = itsCurrentWC->linToWorld(wld, lin);
		//cout << "wldOk=" << wldOk << endl;
		//cout << "wld=(" << wld(0) << ", " << wld(1) << ")" <<  endl;
		Vector<String> unit;
		unit = itsCurrentWC->worldAxisUnits();
		//cout << "units=(" << unit(0) << ", " << unit(1) << ")" <<  endl;
		unit.resize(2, true);
		//cout << "units=" << unit <<  endl;

		if (!wldOk)
			return;

		Record clickPoint;
		clickPoint.define("type", "click");
		clickPoint.define("tool", "Rectangle");
		Record pixel, linear, world;
		pixel.define("pix", pix);
		linear.define("lin", lin);
		world.define("wld", wld);
		world.define("units", unit);
		clickPoint.defineRecord("pixel", pixel);
		clickPoint.defineRecord("linear", linear);
		clickPoint.defineRecord("world", world);

		emit echoClicked(clickPoint);
	}


	void QtOldELRegion::regionReady() {
		Record mouseRegion;
		WorldCanvasHolder* wch = 0;
		if(getMouseRegion(mouseRegion, wch)) {
			emit mouseRegionReady(mouseRegion, wch);
		}
	}



	Bool QtOldELRegion::getMouseRegion(Record& mouseRegion,
	                                   WorldCanvasHolder*& wch) {
		// Retrieve the current rectangular region record and WCH, if any.
		// Construct from the rectangular coordinates the ellipse therein
		// (If nothing is ready, returns False).

		// Here are fields of a typical polygon mouseRegion record as emitted
		// in the mouseRegionReady signal by this routine; the example polygon
		// here has only four points, a typical polygon filled with section
		// points of an ellipse has typically more .
		//
		// type: String "polygon"
		// zindex: Int 23
		// pixel: subRecord {
		//   x: Double array with shape [4]  [229, 306, 312, 257]
		//   y: Double array with shape [4]  [236, 249, 201, 198]  }
		// linear: subRecord {
		//   x: Double array with shape [4]  [142.536, 211.372, 216.736, 167.567]
		//   y: Double array with shape [4]  [154.03, 165.642, 122.767, 120.087]  }
		// (-->following field may be missing if undefined)
		// world: subRecord {
		//   x: Double array with shape [4]  [4.66745, 4.66351, 4.66321, 4.66602]
		//   y: Double array with shape [4]  [1.22487, 1.2251, 1.22427, 1.22421]
		//   units: String array with shape [2]  ["rad", "rad"]  }   (<--x, y)

		mouseRegion = Record();	// Initialize to empty Record.


		if(!ellipseDefined() || itsCurrentWC==0) return False;
		wch = pd_->wcHolder(itsCurrentWC);
		// Only reason pd_ is 'needed' by this tool (it shouldn't need it):
		// locating the important coordinate state 'zindex' on wch
		// (inaccessible from WC), instead of on WC, was a blunder....
		if(wch==0) return False;

		mouseRegion.define("type", "polygon");

		Int zindex = 0;
		if (wch->restrictionBuffer()->exists("zIndex")) {
			wch->restrictionBuffer()->getValue("zIndex", zindex);
		}

		mouseRegion.define("zindex", zindex);

		// get the pix-coordinates
		Int x1, y1, x2, y2;
		get(x1, y1, x2, y2);


		Double theta = 0.0; // pangle * (Float)C::degree;
		Double ang, tmp, x_tmp, y_tmp;
		Double smajor = fabs((Double)x1 - (Double)x2)/2.0;
		Double sminor = fabs((Double)y1 - (Double)y2)/2.0;
		Double xcen   = ((Double)x1 + (Double)x2)/2.0;
		Double ycen   = ((Double)y1 + (Double)y2)/2.0;
		Vector<Double> px, py, lx, ly, wx, wy,
		       pix(2), lin(2), wld(2);
		Bool wldOk=True;

		// determine the number of segments
		uInt nsegments = uInt( min(5000., 2 * C::pi * max(abs(smajor), abs(sminor))+ 0.5));
		if (nsegments < 4) nsegments = 4;

		// size the vectors
		px.resize(nsegments);
		py.resize(nsegments);
		lx.resize(nsegments);
		ly.resize(nsegments);
		wx.resize(nsegments);
		wy.resize(nsegments);

		// go over all segments
		for (uInt i = 0; i < nsegments; i++) {

			// compute one segment point
			ang = (Double)i / (Double)(nsegments - 1) * C::circle;
			x_tmp = smajor * cos(ang);
			y_tmp = sminor * sin(ang);
			ang = theta;
			tmp   = xcen + ( x_tmp * cos(ang) - y_tmp * sin(ang) );
			y_tmp = ycen + ( x_tmp * sin(ang) + y_tmp * cos(ang) );
			x_tmp = tmp;

			// transfer values to the pix-polygon
			pix[0] = px[i] = x_tmp;	// (px, py are Double)
			pix[1] = py[i] = y_tmp;

			// compute the pixel coordinates
			if(!itsCurrentWC->pixToLin(lin, pix)) return False;	  // (unlikely).

			// compute the world coordinates
			if(wldOk) wldOk = wldOk && itsCurrentWC->linToWorld(wld, lin);

			// transfer values to the lin-polygon
			lx[i] = lin[0];
			ly[i] = lin[1];

			// if possible, transfer to
			// the world-polygon
			if(wldOk) {
				wx[i] = wld[0];
				wy[i] = wld[1];
			}

		}

		// create some empty records
		Record pixel, linear, world;

		// load the records
		pixel.define("x", px);
		pixel.define("y", py);
		linear.define("x", lx);
		linear.define("y", ly);
		if(wldOk) {
			world.define("x", wx);
			world.define("y", wy);
			world.define("units", itsCurrentWC->worldAxisUnits());
		}

		// load the output records
		mouseRegion.defineRecord("pixel", pixel);
		mouseRegion.defineRecord("linear", linear);
		if(wldOk) mouseRegion.defineRecord("world", world);

		return True;
	}


	void QtOldELRegion::clicked(Int /*x*/, Int /*y*/) {
		//this has same implementation as doubleClicked
		//we use single 'clicked' for activation
		return;
	}

	void QtOldELRegion::doubleClicked(Int x, Int y) {
		//cout << "QtOldELRegion ellipse " << x << " " << y <<  endl;
		if (itsCurrentWC==0)
			return ;
		WorldCanvasHolder* wch = pd_->wcHolder(itsCurrentWC);
		if (wch == 0)
			return ;

		Vector<Double> pix(2), lin(2), wld(2);
		pix(0) = x;
		pix(1) = y;
		//cout << "pix=(" << pix(0) << ", " << pix(1) << ")" <<  endl;
		if (!itsCurrentWC->pixToLin(lin, pix))
			return ;

		//cout << "lin=(" << lin(0) << ", " << lin(1) << ")" <<  endl;
		Bool wldOk = itsCurrentWC->linToWorld(wld, lin);
		//cout << "wldOk=" << wldOk << endl;
		//cout << "wld=(" << wld(0) << ", " << wld(1) << ")" <<  endl;
		Vector<String> unit;
		unit = itsCurrentWC->worldAxisUnits();
		//cout << "units=(" << unit(0) << ", " << unit(1) << ")" <<  endl;
		unit.resize(2, true);
		//cout << "units=" << unit <<  endl;

		if (!wldOk)
			return;

		Record clickPoint;
		clickPoint.define("type", "click");
		clickPoint.define("tool", "Ellipse");
		Record pixel, linear, world;
		pixel.define("pix", pix);
		linear.define("lin", lin);
		world.define("wld", wld);
		world.define("units", unit);
		clickPoint.defineRecord("pixel", pixel);
		clickPoint.defineRecord("linear", linear);
		clickPoint.defineRecord("world", world);

		emit echoClicked(clickPoint);
	}


	void QtOldPTRegion::regionReady() {

		updateRegion();	// (Useful for profile signalling; will begin
		// poly profiling with initial poly definition).

		Record mouseRegion;
		WorldCanvasHolder* wch = 0;

		if(getMouseRegion(mouseRegion, wch)) {
			emit mouseRegionReady(mouseRegion, wch);
		}
	}



	Bool QtOldPTRegion::getMouseRegion(Record& mouseRegion,
	                                   WorldCanvasHolder*& wch) {
		// Retrieve the current polygon mouse region record and WCH, if any.
		// (If nothing is ready, returns False).

		// Here are fields of a typical mouseRegion record as emitted
		// in the mouseRegionReady signal by this routine; this poly has 4 points:
		//
		// type: String "polygon"
		// zindex: Int 23
		// pixel: subRecord {
		//   x: Double array with shape [4]  [229, 306, 312, 257]
		//   y: Double array with shape [4]  [236, 249, 201, 198]  }
		// linear: subRecord {
		//   x: Double array with shape [4]  [142.536, 211.372, 216.736, 167.567]
		//   y: Double array with shape [4]  [154.03, 165.642, 122.767, 120.087]  }
		// (-->following field may be missing if undefined)
		// world: subRecord {
		//   x: Double array with shape [4]  [4.66745, 4.66351, 4.66321, 4.66602]
		//   y: Double array with shape [4]  [1.22487, 1.2251, 1.22427, 1.22421]
		//   units: String array with shape [2]  ["rad", "rad"]  }   (<--x, y)


		mouseRegion = Record();	// Initialize to empty Record.


		if(!polygonDefined() || itsCurrentWC==0) return False;
		wch = pd_->wcHolder(itsCurrentWC);
		// Only reason pd_ is 'needed' by this tool (it shouldn't need it):
		// locating the important coordinate state 'zindex' on wch
		// (inaccessible from WC), instead of on WC, was a blunder....
		if(wch==0) return False;

		Vector<Int> x, y;
		get(x, y);
		Int nPts = x.nelements();
		if(nPts<3 || nPts!=Int(y.nelements())) return False;
		// (no polygon ready).


		mouseRegion.define("type", "polygon");


		Int zindex = 0;
		if (wch->restrictionBuffer()->exists("zIndex")) {
			wch->restrictionBuffer()->getValue("zIndex", zindex);
		}

		mouseRegion.define("zindex", zindex);


		Vector<Double> px(nPts), py(nPts), lx(nPts), ly(nPts), wx(nPts), wy(nPts),
		       pix(2), lin(2), wld(2);
		Bool wldOk=True;

		for(Int i=0; i<nPts; i++) {

			pix[0] = px[i] = x[i];	// (px, py are Double)
			pix[1] = py[i] = y[i];

			if(!itsCurrentWC->pixToLin(lin, pix)) return False;	  // (unlikely).

			if(wldOk) wldOk = wldOk && itsCurrentWC->linToWorld(wld, lin);

			lx[i] = lin[0];
			ly[i] = lin[1];

			if(wldOk) {
				wx[i] = wld[0];
				wy[i] = wld[1];
			}
		}


		Record pixel, linear, world;

		pixel.define("x", px);
		pixel.define("y", py);
		linear.define("x", lx);
		linear.define("y", ly);
		if(wldOk) {
			world.define("x", wx);
			world.define("y", wy);
			world.define("units", itsCurrentWC->worldAxisUnits());
		}

		mouseRegion.defineRecord("pixel", pixel);
		mouseRegion.defineRecord("linear", linear);
		if(wldOk) mouseRegion.defineRecord("world", world);
		// Receiver may be able to use the mouse region even if, e.g.,
		// it is outside world coordinate boundaries (a modest
		// handwave toward support of all-sky images).

		//cout << "mouseRegion=" << mouseRegion << endl;
		return True;
	}

//void QtOldPTRegion::handleEvent(DisplayEvent& ev) {
//  cout << "QtOldPTRegion::handleEvent" << endl;
//}

//void QtOldPTRegion::keyPressed(const WCPositionEvent &ev) {
//  cout << "QtOldPTRegion pressed======" << endl;
//}

//void QtOldPTRegion::polygonReady() {
//  cout << "QtOldPTRegion polygon ready" << endl;
//}

	void QtOldPTRegion::clicked(Int /*x*/, Int /*y*/) {
		//this has the same implementation as doubleClicked
		//we use sigle 'clicked' for activation
	}
	void QtOldPTRegion::doubleClicked(Int x, Int y) {
		//cout << "QtOldPTRegion polygon " << x << " " << y <<  endl;
		if (itsCurrentWC==0)
			return ;
		WorldCanvasHolder* wch = pd_->wcHolder(itsCurrentWC);
		if (wch == 0)
			return ;

		Vector<Double> pix(2), lin(2), wld(2);
		pix(0) = x;
		pix(1) = y;
		//cout << "pix=(" << pix(0) << ", " << pix(1) << ")" <<  endl;
		if (!itsCurrentWC->pixToLin(lin, pix))
			return ;

		//cout << "lin=(" << lin(0) << ", " << lin(1) << ")" <<  endl;
		Bool wldOk = itsCurrentWC->linToWorld(wld, lin);
		//cout << "wldOk=" << wldOk << endl;
		//cout << "wld=(" << wld(0) << ", " << wld(1) << ")" <<  endl;
		Vector<String> units;
		units = itsCurrentWC->worldAxisUnits();
		//cout << "units=(" << units(0) << ", " << units(1) << ")" <<  endl;
		units.resize(2, true);

		if (!wldOk)
			return;

		Record clickPoint;
		clickPoint.define("type", "click");
		clickPoint.define("tool", "Polygon");
		Record pixel, linear, world;
		pixel.define("pix", pix);
		linear.define("lin", lin);
		world.define("wld", wld);
		world.define("units", units);
		clickPoint.defineRecord("pixel", pixel);
		clickPoint.defineRecord("linear", linear);
		clickPoint.defineRecord("world", world);

		emit echoClicked(clickPoint);
	}

} //# NAMESPACE CASA - END

