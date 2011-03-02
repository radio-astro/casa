//# PSWorldCanvasApp.cc: simple PostScript WorldCanvas application
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

//# aips includes:
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

//# trial includes:

//# display library includes:
#include <display/Display/PSPixelCanvasColorTable.h>
#include <display/Display/PSPixelCanvas.h>

//# this include:
#include <display/Display/PSWorldCanvasApp.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// default constructor
PSWorldCanvasApp::PSWorldCanvasApp(PSDriver *ps) {

  ps_ = ps;
  if(ps == NULL)
	throw(AipsError("PSWorldCanvasApp passed NULL PSDriver."));

  PSPixelCanvasColorTable *psTable = 0;
  try {
    psTable = new PSPixelCanvasColorTable(ps);
  } catch (AipsError x) {
    psTable = 0;
  } 

  if (psTable == 0) {
    throw(AipsError("Unable to build PSPixelCanvasColorTable"));
  }

  itsPixelCanvas = new PSPixelCanvas( ps_, psTable);
  itsWorldCanvas = new WorldCanvas(itsPixelCanvas);
}

// default destructor
PSWorldCanvasApp::~PSWorldCanvasApp() {
  if (itsWorldCanvas) {
    delete itsWorldCanvas;
    itsWorldCanvas = 0;
  }
  if (itsPixelCanvas) {
    delete itsPixelCanvas;
    itsPixelCanvas = 0;
  }
}

void PSWorldCanvasApp::run() {
  //** not sure what multiple refreshes will mean on a PSPixelCanvas,
  //** I think that PSPixelCanvas::clear() should clear the page,
  //** and we will add another command, like PixelCanvas::page() 
  //** which will actually do a clear for X11PixelCanvases, but
  //** will start a new page for PSPixelCanvases.  For the moment,
  //** I'll leave it at clear, assuming that this WON'T start a 
  //** new page in your driver...
  itsPixelCanvas->clear();
  // It currently does a showpage which is ignored if the PSDriver thinks
  // it is in EPS mode. There is an untested ifdef to draw a filled
  // rectangle the size of the bounding box. If not in EPS mode, the
  // bounding box is 0 size.

  //** now do a full refresh, and I guess we need to have some 
  //** command to close the output.  Perhaps just deleting the 
  //** PixelCanvas is sufficient, so when "this" is destroyed,
  //** the file is closed...
  itsPixelCanvas->refresh();
}

} //# NAMESPACE CASA - END

