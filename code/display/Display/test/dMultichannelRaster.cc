//# dMultichannelRaster.cc: demo multichannel rastering on WorldCanvases
//# Copyright (C) 1999,2000,2001
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
#if defined(OGL)
#include <GL/gl.h>
#include <display/Display/SimpleWorldGLCanvasApp.h>
#include <display/Display/GLPixelCanvas.h>
#endif
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCRefreshEvent.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
class MyRefresh : public WCRefreshEH {
public:
	MyRefresh() { };
	virtual void operator()(const WCRefreshEvent &ev);
};

Display::ColorModel g_colormodel;

int main(int argc, char **argv) {

	try {

		Input inputs(1);
		inputs.version("");
#if defined(OGL)
		inputs.create("canvas", "x11", "drawing canvas: x11, ps or gl",
		              "drawing canvas");
#else
		inputs.create("canvas", "x11", "drawing canvas: x11 or ps",
		              "drawing canvas");
#endif
		inputs.create("count", "2", "number of rasters to draw (1 or 2)",
		              "# rasters");
		inputs.create("mode", "rgb", "drawing mode: rgb or hsv",
		              "drawing mode");
		inputs.readArguments(argc, argv);

		PSWorldCanvasApp *psapp = 0;
		SimpleWorldCanvasApp *x11app = 0;
#if defined(OGL)
		SimpleWorldGLCanvasApp *glapp = 0;
#endif
		WorldCanvas *wCanvas = 0;
		PSPixelCanvas *pCanvas;
		PSDriver *psdriver;

		String reqMode = inputs.getString("mode");
		if (reqMode == "hsv") {
			g_colormodel = Display::HSV;
		} else if (reqMode == "rgb") {
			g_colormodel = Display::RGB;
		} else {
			throw(AipsError("Unknown drawing mode"));
		}

		// make a ps, x11 or gl canvas
		String reqCanvas = inputs.getString("canvas");
		if (reqCanvas == String("ps")) {
			psdriver = new PSDriver("dMultichannelRaster.ps", PSDriver::A4,
			                        PSDriver::LANDSCAPE);
			psapp = new PSWorldCanvasApp(psdriver);
			wCanvas = psapp->worldCanvas();
			pCanvas = (PSPixelCanvas *)wCanvas->pixelCanvas();
			pCanvas->setResolution(100, 100);
			PSPixelCanvasColorTable *psctbl = pCanvas->PSpcctbl();
			psctbl->setColorModel(g_colormodel);
		} else if (reqCanvas == String("x11")) {
			x11app = new SimpleWorldCanvasApp(g_colormodel);
			wCanvas = x11app->worldCanvas();
		}
#if defined(OGL)
		else if (reqCanvas == String("gl")) {
			glapp = new SimpleWorldGLCanvasApp(argv[0], g_colormodel);
			wCanvas = glapp->worldCanvas();
		}
#endif
		else {
			throw(AipsError("Unknown canvas type"));
		}

		if (!wCanvas) {
			throw(AipsError("Couldn't construct WorldCanvas"));
		}

		MyRefresh refresher;
		wCanvas->addRefreshEventHandler(refresher);

		// run the PS application
		if (psapp) {
			psapp->run();
		} else if (x11app) {
			x11app->run();
		}
#if defined(OGL)
		else if (glapp) {
			glapp->run();
		}
#endif
		else {
			throw(AipsError("An application was not built"));
		}

		if (x11app) {
			delete x11app;
		}
		if (psapp) {
			delete psapp;
		}
#if defined(OGL)
		if(glapp)
			delete glapp;
#endif
	} catch (const AipsError &x) {
		cerr << "Exception caught:" << endl;
		cerr << x.getMesg() << endl;
	}

}


void MyRefresh::operator()(const WCRefreshEvent &ev) {

	if (ev.reason() == Display::BackCopiedToFront) {
		return;
	}

	WorldCanvas *wCanvas = ev.worldCanvas();
	WCPowerScaleHandler handler;
	wCanvas->setDataScaleHandler(&handler);

	Matrix<Float> arr1(100, 100), arr2(100, 100), arr3(100, 100);
	for (Int i = 0; i < arr1.shape()(0); i++) {
		for (Int j = 0; j < arr1.shape()(1); j++) {
			arr1(i, j) = Float(i);
			arr2(i, j) = Float(j);
			arr3(i, j) = Float(i + j);
		}
	}

	Vector<Double> blpos(2), trpos(2);
	blpos = 0.0;
	trpos = 0.75;

	AttributeBuffer dataRange;
	dataRange.add("dataMin", Double(min(arr1)));
	dataRange.add("dataMax", Double(max(arr1)));
	wCanvas->setAttributes(dataRange);
	if (g_colormodel == Display::RGB) {
		wCanvas->drawImage(blpos, trpos, arr1, Display::Red);
	} else if (g_colormodel == Display::HSV) {
		wCanvas->drawImage(blpos, trpos, arr1, Display::Hue);
	}

	dataRange.set("dataMin", Double(min(arr2)));
	dataRange.set("dataMax", Double(max(arr2)));
	wCanvas->setAttributes(dataRange);
	blpos = 0.2;
	trpos(0) = 0.4;
	trpos(1) = 0.9;
	if (g_colormodel == Display::RGB) {
		wCanvas->drawImage(blpos, trpos, arr2, Display::Green);
	} else if (g_colormodel == Display::HSV) {
		wCanvas->drawImage(blpos, trpos, arr2, Display::Saturation);
	}

	dataRange.set("dataMin", Double(min(arr3)));
	dataRange.set("dataMax", Double(max(arr3)));
	wCanvas->setAttributes(dataRange);
	blpos(0) = 0.1;
	blpos(1) = 0.1;
	trpos = 0.8;
	if (g_colormodel == Display::RGB) {
		wCanvas->drawImage(blpos, trpos, arr3, Display::Blue);
	} else if (g_colormodel == Display::HSV) {
		wCanvas->drawImage(blpos, trpos, arr3, Display::Value);
	}

	wCanvas->flushComponentImages();
}
