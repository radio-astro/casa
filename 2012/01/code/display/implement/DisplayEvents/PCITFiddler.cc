//# PCITFiddler.cc: Colormap fiddling on the PixelCanvas
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

#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/Colormap.h>
#include <display/DisplayEvents/PCITFiddler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
PCITFiddler::PCITFiddler(PixelCanvas *pcanvas,
			 const PCITFiddler::FiddleType &fiddletype,
			 Display::KeySym keysym) :
  PCInvisTool(pcanvas, keysym),
  itsFiddleType(fiddletype) {
}

// Destructor.
PCITFiddler::~PCITFiddler() {
}

// Set the fiddle type.
void PCITFiddler::setFiddleType(const PCITFiddler::FiddleType &fiddletype) {
  itsFiddleType = fiddletype;
}

// This function is called when a new position is ready.
void PCITFiddler::positionReady() {
  Float xfrac, yfrac;
  getFractional(xfrac, yfrac);
  Colormap *cmap = pixelCanvas()->colormap();
  if (!cmap) {
    return;
  }
  switch (itsFiddleType) {
  case PCITFiddler::StretchAndShift:
    {
      Float shift, slope;
      slope = tan((1.0 - yfrac) * 3.14159265 / 2.0);
      // squash up xfrac so as not to waste too much space
      // at the edges of the PixelCanvas
      Float xfracsquashed = pow(abs(xfrac - 0.5) * 2.0, 2.0 + 1.0 / slope) / 
	2.0;
      xfrac = (xfrac - 0.5 < 0.0) ? 0.5 - xfracsquashed : 0.5 + xfracsquashed;
      shift = (slope == 0.0) ? 0.5 : xfrac + (xfrac - 0.5) / slope;
      Vector<Float> params(2);
      params(0) = 0.5 - slope * shift;
      params(1) = slope;
      cmap->setShapingCoefficients(params);
      break;
    }
  case PCITFiddler::BrightnessAndContrast:
    {
      cmap->setBrightness(xfrac, False); // suppress reinstall with False
      cmap->setContrast(yfrac);
      break;
    }
  default:
    {
      throw(AipsError("Unknown fiddle type in PCITFiddler::positionReady"));
    }
  }
}

} //# NAMESPACE CASA - END

