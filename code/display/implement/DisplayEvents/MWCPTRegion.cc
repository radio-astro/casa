//# MWCPTRegion.cc: MultiWorldCanvas event-based polygon region drawer
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

// aips includes:
#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>

// display includes:
#include <display/Display/WorldCanvas.h>

// this include:
#include <display/DisplayEvents/MWCPTRegion.h>

namespace casa { //# NAMESPACE CASA - BEGIN

MWCPTRegion::MWCPTRegion(Display::KeySym keysym) :
  MWCPolyTool(keysym, True) {
}

MWCPTRegion::~MWCPTRegion() {
}

void MWCPTRegion::doubleInside() {
  //cout << "doubleInside --->before region Readay" << endl;
  regionReady();
  //cout << "<------after region ready" << endl;
}

void MWCPTRegion::getLinearCoords(Vector<Double> &x, Vector<Double> &y) {
  Vector<Int> xpix;
  Vector<Int> ypix;
  get(xpix, ypix);
  Int xs, ys, z;
  xpix.shape(xs);
  ypix.shape(ys);
  z = min(xs, ys);
  x.resize(z);
  y.resize(z);
  //cout << "xs=" << xs << " ys=" << ys << " z=" << z << endl;

  Vector<Double> xp(2), xl(2);
  for (Int i = 0; i < z; i++) {
     xp(0) = xpix(i);
     xp(1) = ypix(i);
     itsCurrentWC->pixToLin(xl, xp);
     x(i) = xl(0);
     y(i) = xl(1);
     //cout << "x=" << x(i) << " y=" << y(i) << endl;
  }
  //cout << "getLinear" << endl;
}

void MWCPTRegion::getWorldCoords(Vector<Double> &x, Vector<Double> &y) {
  Vector<Int> xpix;
  Vector<Int> ypix;
  get(xpix, ypix);
  Int xs, ys, z;
  xpix.shape(xs);
  ypix.shape(ys);
  z = min(xs, ys);
  x.resize(z);
  y.resize(z);
  //cout << "xs=" << xs << " ys=" << ys << " z=" << z << endl;

  Vector<Double> xp(2), xl(2);
  for (Int i = 0; i < z; i++) {
     xp(0) = xpix(i);
     xp(1) = ypix(i);
     itsCurrentWC->pixToWorld(xl, xp);
     x(i) = xl(0);
     y(i) = xl(1);
     //cout << "x=" << x(i) << " y=" << y(i) << endl;
  }
  //cout << "getWorld" << endl;
}

} //# NAMESPACE CASA - END

