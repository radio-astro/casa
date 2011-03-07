//# dRGBWCRaster.cc: demo RGB rastering on PostScript and X11 WorldCanvases
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

main(int argc, char **argv) {

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
      psdriver = new PSDriver("dRGBWCRaster.ps", PSDriver::A4, 
			      PSDriver::LANDSCAPE);
      psapp = new PSWorldCanvasApp(psdriver);
      wCanvas = psapp->worldCanvas();
      pCanvas = (PSPixelCanvas *)wCanvas->pixelCanvas();
      pCanvas->setResolution(100, 100);
      PSPixelCanvasColorTable *psctbl = pCanvas->PSpcctbl();
      psctbl->setColorModel(Display::RGB);
    } else if (reqCanvas == String("x11")) {
      x11app = new SimpleWorldCanvasApp(Display::RGB);
      wCanvas = x11app->worldCanvas();
    } else {
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
    } else {
      throw(AipsError("An application was not built"));
    }

    if (x11app) {
      delete x11app;
    } 
    if (psapp) {
      delete psapp;
    }

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
  wCanvas->drawImage(blpos, trpos, arr1, Display::Red);

  dataRange.set("dataMin", Double(min(arr2)));
  dataRange.set("dataMax", Double(max(arr2)));
  wCanvas->setAttributes(dataRange);
  blpos = 0.2;
  trpos(0) = 0.4;
  trpos(1) = 0.9;
  wCanvas->drawImage(blpos, trpos, arr2, Display::Green);

  dataRange.set("dataMin", Double(min(arr3)));
  dataRange.set("dataMax", Double(max(arr3)));
  wCanvas->setAttributes(dataRange);
  blpos(0) = 0.3;
  blpos(1) = 0.4;
  trpos = 1.0;
  wCanvas->drawImage(blpos, trpos, arr3, Display::Blue);

  wCanvas->flushComponentImages();
}
