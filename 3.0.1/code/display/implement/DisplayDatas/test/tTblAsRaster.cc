//# tTblAsRaster.cc: test rastering of tables on PostScript and X11 canvases
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
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/PSWorldCanvasApp.h>
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/TblAsRasterDD.h>
#include <display/DisplayDatas/WorldAxesDD.h>         

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
main(int argc, char **argv) {

  try {

    Input inputs(2);
    inputs.version("");
    inputs.create("table", "", "table filename to display",
		  "table to display");
    inputs.create("canvas", "x11", "drawing canvas: x11 or ps",
		  "drawing canvas");
    inputs.readArguments(argc, argv);

    if (argc < 2) {
      throw(AipsError("no table given on the command line"));
    }

    PSWorldCanvasApp *psapp = 0;
    SimpleWorldCanvasApp *x11app = 0;
    WorldCanvas *wCanvas = 0;
    PSPixelCanvas *pCanvas;
    PSDriver *psdriver;

    String filename1 = inputs.getString("table");

    // make a ps or x11 canvas
    String reqCanvas = inputs.getString("canvas");
    if (reqCanvas == String("ps")) {
      psdriver = new PSDriver("tTblAsRaster.ps", PSDriver::A4, 
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

    // read table from disk and set up display data
    TblAsRasterDD *tardd1 = 0;
    tardd1 = new TblAsRasterDD(filename1);
    if (!tardd1) {
      throw(AipsError("couldn't build the display data"));
    }

    Record rec, recOut;
    rec.define("axislabelswitch", True);
    rec.define("xgridtype", "Full grid");
    rec.define("ygridtype", "Full grid");
    tardd1->setOptions(rec, recOut);

    Colormap cmap1("Hot Metal 2");
    tardd1->setColormap(&cmap1, 1.0);

    WorldAxesDD axdd;          
    rec.define("titletext", "UV Table DisplayData First Light!"); 
    axdd.setOptions(rec, recOut);
    // add the data to the display
    wcHolder->addDisplayData((DisplayData *)tardd1);
    wcHolder->addDisplayData(&axdd);      

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
    if (tardd1) {
      delete tardd1;
    }

  } catch (const AipsError &x) {
    cerr << "Exception caught:" << endl;
    cerr << x.getMesg() << endl;
  }
  
}

