//# AxesDisplayMethod.cc: axis label drawing for AxesDisplayData
//# Copyright (C) 1999,2000,2001,2002
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
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/AxesDisplayData.h>
#include <display/DisplayDatas/AxesDisplayMethod.h>
#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

AxesDisplayMethod::AxesDisplayMethod(WorldCanvas *worldCanvas, 
				     AttributeBuffer *wchAttributes,
				     AttributeBuffer *ddAttributes,
				     CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}

AxesDisplayMethod::~AxesDisplayMethod() {
  cleanup();
}

Bool AxesDisplayMethod::drawIntoList(Display::RefreshReason reason,
				    WorldCanvasHolder &wcHolder) {
  // Locate the WorldCanvas to draw upon
  WorldCanvas *wc = wcHolder.worldCanvas();

  // increment the window setting by 1 to account for 0->1 offset
  // Could use an aipsrc variable here to control this.
  Float xmin = 0.0, xmax = 1.0, ymin = 0.0, ymax = 1.0;
  cpgqwin(&xmin, &xmax, &ymin, &ymax);
  cpgswin(xmin + 1, xmax + 1, ymin + 1, ymax + 1);

  AxesDisplayData *pdd = dynamic_cast<AxesDisplayData *>(parentDisplayData());

  wc->setLineWidth(pdd->lineWidth());
  cpgsch(pdd->charSize());
  if (pdd->charFont() == "roman") {
    cpgscf(2);
  } else if (pdd->charFont() == "italic") {
    cpgscf(3); 
  } else if (pdd->charFont() == "script") {
    cpgscf(4);
  } else {
    cpgscf(1);
  }
    
  wc->setColor(pdd->titleTextColor());
  cpglab("", "", pdd->titleText().chars());

  wc->setColor(pdd->xAxisColor());
  cpglab(pdd->xAxisText(wc).chars(), "", "");
  String xboxstr("BCN"); // draw top and bottom borders, numeric labels
  if (pdd->xGridType() == "Tick marks") {
    xboxstr += "TS"; // draw ticks and sub-ticks
  } else if (pdd->xGridType() == "Full grid") {
    xboxstr += "G"; // draw grid
  }
  cpgbox(xboxstr.chars(), 0.0, 0, "", 0.0, 0);

  wc->setColor(pdd->yAxisColor());
  cpglab("", pdd->yAxisText(wc).chars(), "");
  String yboxstr("BCNV"); // draw top and bottom borders, numeric labels
  if (pdd->yGridType() == "Tick marks") {
    yboxstr += "TS"; // draw ticks and sub-ticks
  } else if (pdd->yGridType() == "Full grid") {
    yboxstr += "G"; // draw grid
  }
  cpgbox("", 0.0, 0, yboxstr.chars(), 0.0, 0);

  // now redraw just the box in the requested color
  wc->setColor(pdd->outlineColor());
  cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

  cpgswin(xmin, xmax, ymin, ymax);
  return True;
}

void AxesDisplayMethod::cleanup() {
}

AxesDisplayMethod::AxesDisplayMethod() {
}

AxesDisplayMethod::AxesDisplayMethod(const AxesDisplayMethod &other) :
  CachingDisplayMethod(other) {
}

void AxesDisplayMethod::operator=(const AxesDisplayMethod &) {
}

} //# NAMESPACE CASA - END

