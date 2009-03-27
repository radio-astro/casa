//# Copyright (C) 2005
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

#include <display/QtPlotter/QtMWCTools.qo.h>
//#include <display/DisplayEvents/MWCRTRegion.h>
//#include <display/DisplayEvents/MWCCrosshairTool.h>
//#include <display/DisplayEvents/MWCPTRegion.h>

#include <display/Display/PanelDisplay.h>
#include <display/QtViewer/QtMouseTools.qo.h>

#include <casa/BasicSL/String.h>
//#include <casa/Utilities/Assert.h>
//#include <casa/BasicMath/Math.h>
//#include <display/Display/WorldCanvas.h>
//#include <display/Display/WorldCanvasHolder.h>

namespace casa {

//////////////poly tool//////////////////
QtPolyTool::QtPolyTool(PanelDisplay *pd) : QtPTRegion(pd)
{
  setObjectName("PolyTool"); 
}

void QtPolyTool::updateRegion() {
  if (!itsCurrentWC) return;

  //qDebug() << "ploy ready" << coordType.chars();
  Vector<Double> x;
  Vector<Double> y;
  if (coordType == "world")
     getWorldCoords(x, y);
  else
     getLinearCoords(x, y);
  //Int nt;
  //x.shape(nt);
  //cout << "nt=" << nt << endl;
  //for (Int i = 0; i << nt; i++) {
  //  cout << "x=" << x(i) << " y=" << y(i) << endl;
  //} 

  emit wcNotify(coordType, x, y);
}

//////////////rect tool//////////////////
QtRectTool::QtRectTool(PanelDisplay *pd) : QtRTRegion(pd) 
{
  setObjectName("RectTool"); 
}

void QtRectTool::updateRegion() {
  if (!itsCurrentWC) return;

  //qDebug() << "rect ready" << coordType.chars();
  Vector<Double> x;
  Vector<Double> y;
  if (coordType == "world")
     getWorldCoords(x, y);
  else
     getLinearCoords(x, y);
  Vector<Double> xx(2);
  Vector<Double> yy(2);
  xx(0) = x(0); xx(1) = y(0);
  yy(0) = x(1); yy(1) = y(1);
  //cout << "(" << xx(0) << "," << yy(0) << ")"
  //     << " (" << xx(1) << "," << yy(1) << ")" << endl;
  emit wcNotify(coordType, xx, yy);
}

//////////////cross tool//////////////////

QtCrossTool::QtCrossTool() : MWCCrosshairTool()
{
  setObjectName("CrossTool"); 
}

void QtCrossTool::crosshairReady(const String &evtype) {
  if (!itsCurrentWC) return;

  //qDebug() << "cross ready" << coordType.chars();
  Double x, y;
  if (coordType == "world")
    getWorld(x, y);
  else
     getLin(x, y);
  Vector<Double> xx(1);
  Vector<Double> yy(1);
  xx[0]=x; 
  yy[0]=y;
  emit wcNotify(coordType, xx, yy);
}


}


