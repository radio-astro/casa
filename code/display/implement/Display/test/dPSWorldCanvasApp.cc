//# tPSWorldCanvasApp.h: test of PSWorldCanvasApp class
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
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <display/Display/PSWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCRefreshEvent.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
class AipsLogoWCEH : public WCRefreshEH {
 public:
  AipsLogoWCEH();
  void operator()(const WCRefreshEvent &ev);
 private:
  uInt itsCount;
  Int itsXOffset, itsYOffset;
  Int itsXSize, itsYSize;
  Float itsYInterval;
  Vector<String> itsColors;
};

AipsLogoWCEH::AipsLogoWCEH() {
  itsCount = 0;
  itsColors.resize(6);
  itsColors(0) = "#ccffff";
  itsColors(1) = "#00ccff";
  itsColors(2) = "#0099ff";
  itsColors(3) = "#0033ff";
  itsColors(4) = "#0019cc";
  itsColors(5) = "#000099";
}

void AipsLogoWCEH::operator()(const WCRefreshEvent &ev) {
  WorldCanvas *wcanvas = ev.worldCanvas();
  PixelCanvas *pcanvas = wcanvas->pixelCanvas();
  Int xoffset = wcanvas->canvasXOffset();
  Int yoffset = wcanvas->canvasYOffset();
  Int xsize = wcanvas->canvasXSize();
  Int ysize = wcanvas->canvasYSize();
  if ((itsCount == 0) || (xoffset != itsXOffset) ||
      (yoffset != itsYOffset) || (xsize != itsXSize) ||
      (ysize != itsYSize)) {
    // first time or geometry has changed.
    itsCount = 0;
    itsXOffset = xoffset;
    itsYOffset = yoffset;
    itsXSize = xsize;
    itsYSize = ysize;
    itsYInterval = Float(itsYSize) / Float(itsColors.nelements());
    wcanvas->clear();
  }
  if (itsCount < itsColors.nelements()) {
    Display::DrawBuffer drawb = pcanvas->drawBuffer();
    pcanvas->setDrawBuffer(Display::FrontAndBackBuffer);
    pcanvas->setColor(itsColors(itsCount));
    pcanvas->drawFilledRectangle(itsXOffset, itsYOffset + 
				 Int(Float(itsCount) * itsYInterval),
				 itsXOffset + itsXSize,
				 itsYOffset + 
				 Int(Float(itsCount + 1) * itsYInterval) - 2);
    pcanvas->setDrawBuffer(drawb);
    itsCount++;
    this->operator()(ev);
  } else {
//    pcanvas->setFont("-b&h-lucidabright-demibold-r-normal-*-60-400-100-*-p-*-iso8859-1");
    pcanvas->setFont("Times-Roman");
    pcanvas->setColor("grey");
    pcanvas->drawText(itsXOffset + itsXSize / 2 + 2, 
		      itsYOffset + itsYSize / 2 - 3, "AIPS++");
    pcanvas->setColor("red");
    pcanvas->drawText(itsXOffset + itsXSize / 2, itsYOffset + itsYSize / 2,
		      "AIPS++");
    itsCount = 0;
  }
}

static const char *fname = "test.ps";

int main(int, char **) {
  try {
    fstream os(fname, ios::out);    // Where to set PS output.
    // PostScript driver using "LETTER" format. ('EPS').
    PSDriver *ps = new PSDriver(os, PSDriver::LETTER);
    PSWorldCanvasApp *psApp = new PSWorldCanvasApp(ps);
    WorldCanvas *wCanvas = psApp->worldCanvas();
    AipsLogoWCEH *logoRefresh = new AipsLogoWCEH();
//    itsWorldCanvas->addRefreshEventHandler(*logoRefresh);
    wCanvas->addRefreshEventHandler(*logoRefresh);
    psApp->run();
    //** maybe need to delete psApp to flush the PS buffer into
    //** the output file...
    delete psApp;
    delete ps;		// Neither PSPixelCanvas nor PSPixelCanvasColortable
			// delete their PSDriver.
    os.close();
  } catch (const AipsError &x) {
    cerr << "Exception: " << x.getMesg() << endl;
    return(1);
  } 

  cout << "ok" << endl;
  // It's best to use cerr rather than cout since the default for PSDriver
  // is to write to cout.
  return(0);
}
