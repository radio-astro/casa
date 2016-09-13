//# tArrayAndDrawing.cc: test Array and Drawing displaydatas
//# Copyright (C) 1999,2000
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

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/PSWorldCanvasApp.h>
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayEvents/WCRTZoomer.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
int main(int argc, char **argv) {

	try {

		Input inputs(1);
		inputs.version("");
		inputs.create("canvas", "x11", "drawing canvas: x11 or ps",
		              "drawing canvas");
		inputs.create("count", "2", "number of rasters to draw (1 or 2)",
		              "# rasters");
		inputs.readArguments(argc, argv);

		PSWorldCanvasApp *psapp = 0;
		SimpleWorldCanvasApp *x11app = 0;
		WorldCanvas *wCanvas = 0;
		PSPixelCanvas *pCanvas;
		PSDriver *psdriver;

		// make a ps or x11 canvas
		String reqCanvas = inputs.getString("canvas");
		if (reqCanvas == String("ps")) {
			psdriver = new PSDriver("tArrayAsRaster.ps", PSDriver::A4,
			                        PSDriver::LANDSCAPE);
			psapp = new PSWorldCanvasApp(psdriver);
			wCanvas = psapp->worldCanvas();
			pCanvas = (PSPixelCanvas *)wCanvas->pixelCanvas();
			pCanvas->setResolution(100, 100);
		} else if (reqCanvas == String("x11")) {
			x11app = new SimpleWorldCanvasApp;
			wCanvas = x11app->worldCanvas();
		} else {
			throw(AipsError("Unknown canvas type"));
		}

		if (!wCanvas) {
			throw(AipsError("Couldn't construct WorldCanvas"));
		}

		// manage it with a WorldCanvasHolder
		WorldCanvasHolder *wcHolder = 0;
		wcHolder = new WorldCanvasHolder(wCanvas);
		if (!wcHolder) {
			throw(AipsError("Couldn't construct WorldCanvasHolder"));
		}

		IPosition ipos1(2, 100, 100);
		Array<Float> array1(ipos1);
		for (uInt i = 0; i < uInt(ipos1(0)); i++) {
			for (uInt j = 0; j < uInt(ipos1(1)); j++) {
				array1(IPosition(2, i, j)) = Float(i);
			}
		}
		LatticeAsRaster<Float> *lardd1 = 0;
		lardd1 = new LatticeAsRaster<Float>(&array1, 0, 1);
		if (!lardd1) {
			throw(AipsError("couldn't build the display data"));
		}

		IPosition ipos2(2, 30, 70);
		Array<Float> array2(ipos2);
		for (uInt i = 0; i < uInt(ipos2(0)); i++) {
			for (uInt j = 0; j < uInt(ipos2(1)); j++) {
				array2(IPosition(2, i, j)) = Float(j);
			}
		}
		LatticeAsRaster<Float> *lardd2 = 0;
		lardd2 = new LatticeAsRaster<Float>(&array2, 0, 1);
		if (!lardd2) {
			throw(AipsError("couldn't build the display data"));
		}

		Record rec, recOut;
		rec.define("axislabelswitch", True);
		rec.define("xgridtype", "Full grid");
		rec.define("ygridtype", "Full grid");
		lardd1->setOptions(rec, recOut);

		Colormap cmap1("Hot Metal 2");
		lardd1->setColormap(&cmap1, 1.0);
		Colormap cmap2("RGB 1");
		lardd2->setColormap(&cmap2, 1.0);

		// add the data to the display
		wcHolder->addDisplayData((DisplayData *)lardd1);
		if (inputs.getInt("count") == 2) {
			wcHolder->addDisplayData((DisplayData *)lardd2);
		}

		// make a DrawingDisplayData
		DrawingDisplayData ddd;
		Record recrec;
		Vector<Double> blc(2), trc(2);
		blc(0) = 22.5;
		blc(1) = 32.5;
		trc(0) = 67.5;
		trc(1) = 45.0;
		recrec.define("type", "rectangle");
		recrec.define("color", "black");
		recrec.define("id", 45);
		recrec.define("label", "NGC 45");
		recrec.define("blc", blc);
		recrec.define("trc", trc);
		ddd.addObject(recrec);

		recrec.define("color", "green");
		recrec.define("id", 47);
		recrec.define("label", "NGC 47");
		recrec.define("blc", blc / 2.0);
		recrec.define("trc", trc / 1.6);
		ddd.addObject(recrec);

		ddd.removeObject(45);

		Record drawingOptions;
		drawingOptions.define("labelposition", "centre");
		ddd.setOptions(drawingOptions, recOut);

		// register it
		wcHolder->addDisplayData((DisplayData *)&ddd);

		// add a zoomer
		WCRTZoomer zoomer(wCanvas, Display::K_Pointer_Button3);

		// run the PS application
		if (psapp) {
			psapp->run();
		} else if (x11app) {
			x11app->run();
		} else {
			throw(AipsError("An application was not built"));
		}

		if (wcHolder) {
			delete wcHolder;
		}
		if (x11app) {
			delete x11app;
		}
		if (psapp) {
			delete psapp;
		}
		if (lardd2) {
			delete lardd2;
		}
		if (lardd1) {
			delete lardd1;
		}

	} catch (const AipsError &x) {
		cerr << "Exception caught:" << endl;
		cerr << x.getMesg() << endl;
		return 1;
	}
	return 0;
}

