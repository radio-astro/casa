//# DSCircle.cc: Circle implementation for "DisplayShapes"
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id: 

#include <casa/aips.h>

#include <display/DisplayShapes/DSCircle.h>
#include <casa/BasicMath/Math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSCircle::DSCircle() :
  DSEllipse(True) {
  setDefaultOptions();
}


DSCircle::DSCircle(const Float& xPos, const Float& yPos, const Float& radius, 
		   const Bool& hasHandles,
		   const Bool& drawHandles) :
  DSEllipse(xPos, yPos, radius, radius, False, False, True) {
  
  setHasHandles(hasHandles);
  if (hasHandles) {
    setDrawHandles(drawHandles);
    Matrix<Float> hp(1,2);
    hp(0,0) = xPos + radius;
    hp(0,1) = yPos;
    buildHandles(hp);
  }

}

DSCircle::~DSCircle() {

}

DSCircle::DSCircle(const DSCircle& other) :
  DSEllipse(other) {
}

void DSCircle::changePoint(const Vector<Float> &newPos, const Int nPoint) {
  changePoint(newPos);
}

void DSCircle::changePoint(const Vector<Float> &newPos) {
  Vector<Float> cent(getCenter());
  Float dist(hypot(newPos[0] - cent[0], newPos[1] - cent[1]));
  setRadius(dist);
  Matrix<Float> hp(1,2);
  hp(0,0) = cent[0] + getMinorAxis();
  hp(0,1) = cent[1];
  setHandlePositions(hp);
}


void DSCircle::setRadius(const Float& radius) {
  DSEllipse::setMajorAxis(radius);
  DSEllipse::setMinorAxis(radius);
}


Bool DSCircle::inObject(const Float& xPos, const Float& yPos) {
  Bool inside = False;
  Vector<Float> cent(getCenter());
  inside = (hypot(xPos - cent[0], yPos - cent[1]) < getMinorAxis());
  return inside;
}

void DSCircle::setCenter(const Float& xPos, const Float& yPos) {
  DSEllipse::setCenter(xPos, yPos);
}

Bool DSCircle::setOptions(const Record& settings) {
  Bool localChange = False;

  if (settings.isDefined("radius")) {
    DSEllipse::setMinorAxis(settings.asFloat("radius"));
    DSEllipse::setMajorAxis(settings.asFloat("radius"));
  }

  if (DSEllipse::setOptions(settings)) localChange = True;
  return localChange;
}

Record DSCircle::getOptions() {
  Record rec(DSEllipse::getOptions());
  
  rec.define("type", "circle");
  rec.define("radius", getMinorAxis());

  return rec;  
}
void DSCircle::setDefaultOptions() {

}









} //# NAMESPACE CASA - END

