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
  Vector<Double> wx;
  Vector<Double> wy;
  Vector<Double> px;
  Vector<Double> py;
  getWorldCoords(wx, wy);
  getLinearCoords(px, py);
  //Int nt;
  //x.shape(nt);
  //cout << "nt=" << nt << endl;
  //for (Int i = 0; i << nt; i++) {
  //  cout << "x=" << x(i) << " y=" << y(i) << endl;
  //} 

  emit wcNotify( coordType, px, py, wx, wy );
}

void QtPolyTool::setCoordType(const String& t) {
    QtMWCTool::setCoordType(t);
    updateRegion( );
}

//////////////rect tool//////////////////
QtRectTool::QtRectTool(PanelDisplay *pd) : QtRTRegion(pd) 
{
  setObjectName("RectTool"); 
}

void QtRectTool::updateRegion() {
  if (!itsCurrentWC) return;

  //qDebug() << "rect ready" << coordType.chars();
  Vector<Double> wx;
  Vector<Double> wy;
  Vector<Double> px;
  Vector<Double> py;
  getWorldCoords(wx, wy);
  getLinearCoords(px, py);
  Vector<Double> wxx(2);
  Vector<Double> wyy(2);
  wxx(0) = wx(0); wxx(1) = wy(0);
  wyy(0) = wx(1); wyy(1) = wy(1);
  Vector<Double> pxx(2);
  Vector<Double> pyy(2);
  pxx(0) = px(0); pxx(1) = py(0);
  pyy(0) = px(1); pyy(1) = py(1);
  //cout << "(" << xx(0) << "," << yy(0) << ")"
  //     << " (" << xx(1) << "," << yy(1) << ")" << endl;
  emit wcNotify( coordType, pxx, pyy, wxx, wyy );
}

void QtRectTool::setCoordType(const String& t) {
  QtMWCTool::setCoordType(t);
  updateRegion( );
}

//////////////cross tool//////////////////

QtCrossTool::QtCrossTool() : MWCCrosshairTool()
{
  setObjectName("CrossTool"); 
}

void QtCrossTool::crosshairReady(const String &evtype) {
  if (!itsCurrentWC) return;

  //qDebug() << "cross ready" << coordType.chars();
  Double wx, wy;
  Double px, py;
  getWorld(wx, wy);
  getLin(px, py);
  Vector<Double> wxx(1);
  Vector<Double> wyy(1);
  wxx[0]=wx; 
  wyy[0]=wy;
  Vector<Double> pxx(1);
  Vector<Double> pyy(1);
  pxx[0]=px; 
  pyy[0]=py;
  emit wcNotify( coordType, pxx, pyy, wxx, wyy );
}

void QtCrossTool::setCoordType(const String& t) {
  QtMWCTool::setCoordType(t);
  crosshairReady("");
}


}


