//# dPanelDisplay.cc: demo panelling of arrays on X11PixelCanvas
//# Copyright (C) 2000
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
#include <display/Display/SimplePixelCanvasApp.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/Display/PanelDisplay.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
main(int argc, char **argv) {

  try {

    Input inputs(1);
    inputs.version("");
    inputs.create("count", "2", "number of rasters to draw (1 or 2)",
		  "# rasters");
    inputs.readArguments(argc, argv);

    SimplePixelCanvasApp *x11app = 0;
    PixelCanvas *pcanvas;

    // make an X11PixelCanvas
    x11app = new SimplePixelCanvasApp;
    pcanvas = x11app->pixelCanvas();

    if (!pcanvas) {
      throw(AipsError("Couldn't construct PixelCanvas"));
    }

    // manage it with a PanelDisplay
    PanelDisplay *pdisplay = 0;
    pdisplay = new PanelDisplay(pcanvas);

    // make a some DisplayDatas
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

    // set some colormaps
    Colormap cmap1("Hot Metal 2");
    lardd1->setColormap(&cmap1, 1.0);
    Colormap cmap2("RGB 1");
    lardd2->setColormap(&cmap2, 1.0);
	       
    // add the data to the PanelDisplay
    pdisplay->addDisplayData(*lardd1);
    if (inputs.getInt("count") == 2) {
      pdisplay->addDisplayData(*lardd2);
    }

    // run the PS application
    if (x11app) {
      x11app->run();
    } else {
      throw(AipsError("An application was not built"));
    }
    
    if (pdisplay) {
      delete pdisplay;
    }
    if (x11app) {
      delete x11app;
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
  }
  
}

