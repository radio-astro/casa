//# dProfile2dDD.cc: demo of Profile2dDD
//# Copyright (C) 2000,2003
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
#include <casa/Containers/Record.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/X11PixelCanvas.h>
#include <display/Display/SimplePixelCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/Colormap.h>
#include <display/DisplayEvents/CrosshairEvent.h>


//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/Profile2dDD.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/LatticeAsMarker.h>

#include <casa/namespace.h>
int main(int argc, char **argv) {

  try {
    Input inputs(1);
    inputs.version("");
    inputs.create("in", "", "Input file name");
    inputs.readArguments(argc, argv);
    const String in = inputs.getString("in");
    SimplePixelCanvasApp *x11app = 0;
    x11app = new SimplePixelCanvasApp;

    // make an X11PixelCanvas
    PixelCanvas *pcanvas = 0;
    pcanvas = x11app->pixelCanvas();
    //PanelDisplay *pdisp1 = 0;
    //PanelDisplay *pdisp2 = 0;
    WorldCanvas *pdisp1 = 0;
    WorldCanvas *pdisp2 = 0;
    WorldCanvasHolder *wch1 = 0;
    WorldCanvasHolder *wch2 = 0;
    if (!pcanvas) {
      throw(AipsError("Couldn't construct PixelCanvas"));
    }
    // manage it with a PanelDisplay
    //pdisp1 = new PanelDisplay(pcanvas,1,1,0.0,0.0,0.8,1.0);
    //pdisp2 = new PanelDisplay(pcanvas,1,1,0.8,0.0,0.2,1.0);
    pdisp1 = new WorldCanvas(pcanvas,0.0,0.0,0.5,1.0);
    pdisp2 = new WorldCanvas(pcanvas,0.5,0.0,0.5,1.0);
    wch1 = new WorldCanvasHolder(pdisp1);
    wch2 = new WorldCanvasHolder(pdisp2);

    // make a some DisplayDatas
    ImageInterface<Float>* pImage = 0;
    cout << "Trying to load AIPS++ Image \"" << in << "\"" << endl;
    pImage = new PagedImage<Float>(in, TableLock::UserNoReadLocking);
    uInt nDim = pImage->ndim();
    cout << "Image has " << nDim << " dimensions" << endl;
    if (nDim < 3) {
      throw(AipsError("image has less than three dimensions"));
    }
    DisplayData *lar = 0;
    IPosition fixedPos(nDim);
    fixedPos = 0;
    lar = new LatticeAsRaster<Float>(pImage, 0, 1, 2, fixedPos);
    //    Profile2dDD *pDD = new Profile2dDD(dynamic_cast<LatticePADisplayData<Float>*>(lar));
    cout << "Creating Profile2dDD... ";
    Profile2dDD *pDD = new Profile2dDD();
    cout << "OK" << endl;
    cout << "Attaching Display Data... ";
    pDD->attachDD(static_cast<LatticePADisplayData<Float>*>(lar));
    cout << "OK" << endl;
    if (!lar) {
      throw(AipsError("couldn't build the display data"));
    }
    // Test setOptions
    cout << "Testing setOptions... ";
    Record rec, recout;
    rec.define("xgridcolor", "white");	
    rec.define("ygridcolor", "white");
    rec.define("profilecolor", "yellow");
    rec.define("profilelinewidth", 0.5);
    rec.define("profilelinestyle", "LSSolid");
    rec.define("profiledatamin", -0.04);
    rec.define("profiledatamax", 0.04);
    rec.define("spectralunit", "Hz");
    //     rec.define("profileautoscale", True);
    pDD->setOptions(rec, recout);
    cout << "OK!" << endl;
    // Test getOptions
    cout << "Testing getOptions... ";
    rec = pDD->getOptions();
    if (rec.nfields() > 0)
      cout << "OK" << endl;
    //    for (uInt i=0; i < rec.nfields(); i++) {
    //      cout << "rec.name(" << i << ") = " << rec.name(i) << endl;
    //    }
    
    // set some colormaps
    Colormap cmap1("Hot Metal 1");
    lar->setColormap(&cmap1, 1.0);

    CrosshairEvent ev(pdisp1, 0, 0, "up");

    wch1->addDisplayData(lar);
    wch2->addDisplayData(pDD);
    lar->handleEvent(ev);
    // run the PS application
    if (x11app) {
      x11app->run();
    } else {
      throw(AipsError("An application was not built"));
    }
    delete pDD;
    delete lar;
    delete pImage;
    delete pcanvas;
    delete x11app;
  } catch (const AipsError &x) {
    cerr << "Exception caught:" << endl;
    cerr << x.getMesg() << endl;
  }
  
}
