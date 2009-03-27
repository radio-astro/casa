//# DSScreenPoly.cc : Implementation of a relative screen pos. DSPoly
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

#include <display/DisplayShapes/DSScreenPoly.h>
#include <display/DisplayShapes/DSWorldPoly.h>
#include <display/DisplayShapes/DSPixelPoly.h>

#include <display/DisplayShapes/DSScreenPolyLine.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSScreenPoly::DSScreenPoly() :
  DSPoly(),
  itsPC(0),
  itsRelativePoints(0,0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
}

DSScreenPoly::DSScreenPoly(const Record& settings, PixelCanvas* pc) :
  DSPoly(),
  itsPC(pc),
  itsRelativePoints(0,0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  setOptions(settings);
  
}

DSScreenPoly::DSScreenPoly(DSPixelPoly& other, PixelCanvas* pc) :
  DSPoly(),
  itsPC(pc),
  itsRelativePoints(0,0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  DSPoly::setOptions(other.getRawOptions());
  updateRelative();
}

DSScreenPoly::DSScreenPoly(DSWorldPoly& other) :
  DSPoly(),
  itsPC(0),
  itsRelativePoints(0,0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  itsPC = other.panelDisplay()->pixelCanvas();
  
  DSPoly::setOptions(other.getRawOptions());
  updateRelative();
}

DSScreenPoly::DSScreenPoly(DSScreenPolyLine& other, PixelCanvas* pc) :
  DSPoly(),
  itsPC(pc),
  itsRelativePoints(0,0) 
{

  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  Record otherOptions = other.getOptions();

  if (otherOptions.isDefined("polylinepoints")) {

    Record points = otherOptions.subRecord("polylinepoints");

    otherOptions.removeField("polylinepoints");
    otherOptions.defineRecord("polygonpoints", points);
  }

  setOptions(otherOptions);
  updateRelative();
}

DSScreenPoly::~DSScreenPoly() {

}
  

Bool DSScreenPoly::setOptions(const Record& settings) {
  Bool localChange = False;
  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "frac") {
      throw(AipsError("I (DSScreenPoly) was expecting an option "
		      "record which"
		      " had coords != \'frac\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  matrixFloatFromQuant(toSet, "polygonpoints", "frac");


  if (toSet.isDefined("polygonpoints")) {
    Matrix<Float> relative = toSet.asArrayFloat("polygonpoints");
    Matrix<Float> screen = relToScreen(relative, itsPC);
    
    toSet.removeField("polygonpoints");
    toSet.define("polygonpoints", screen);

  }

  if (DSPoly::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}



void DSScreenPoly::recalculateScreenPosition() {
  if (itsRelativePoints.nelements() > 0) {
    if (!itsPC) {
      throw(AipsError("No pixel canvas available to update screen pos for"
		      " ScreenPoly"));
    }
    
    if (itsRelativePoints.ncolumn() != 2) {
      throw(AipsError("Bad format for relative Points Matrix"));
    }
    
    Matrix<Float> pixel = relToScreen(itsRelativePoints, itsPC);
    setPoints(pixel);
  }
}

void DSScreenPoly::move(const Float& dX, const Float& dY) {
  DSPoly::move(dX, dY);
  updateRelative();
}

void DSScreenPoly::setCenter(const Float& xPos, const Float& yPos) {
  DSPoly::setCenter(xPos, yPos);
  updateRelative();
}

void DSScreenPoly::rotate(const Float& angle) {
  DSPoly::rotate(angle);
  updateRelative();
}

void DSScreenPoly::scale(const Float& scaleFactor) {
  DSPoly::scale(scaleFactor);
  updateRelative();
}

void DSScreenPoly::changePoint(const Vector<Float>&pos, const Int n) {
  DSPoly::changePoint(pos, n);
  updateRelative();
}

void DSScreenPoly::changePoint(const Vector<Float>& pos) {
  DSPoly::changePoint(pos);
  updateRelative();
}


void DSScreenPoly::updateRelative() {
  Matrix<Float> pixel = getPoints();
  
  if (pixel.nelements() > 0) {
    if (!itsPC) {
      throw(AipsError("No pixel canvas available to update screen pos for"
		      " ScreenPoly"));
    }
    
    if (pixel.ncolumn() != 2) {
      throw(AipsError("Bad format for pixel Points Matrix returned "
		      "from shape"));
    }
    
    itsRelativePoints.resize(pixel.nrow(), pixel.ncolumn());
    itsRelativePoints = screenToRel(pixel, itsPC);
  }
}

void DSScreenPoly::addPoint(const Vector<Float>& newPos) {
  DSPoly::addPoint(newPos);
  updateRelative();
}
void DSScreenPoly::setPoints(const Matrix<Float>& points) {
  DSPoly::setPoints(points);
  updateRelative();
}


Record DSScreenPoly::getOptions() {

  Record toReturn;
  
  toReturn = DSPoly::getOptions();

  if (toReturn.isDefined("polygonpoints")) {
    Matrix<Float> pixel = toReturn.asArrayFloat("polygonpoints");
    Matrix<Float> relative = screenToRel(pixel, itsPC);
    toReturn.removeField("polygonpoints");
    toReturn.define("polygonpoints", relative);

    matrixFloatToQuant(toReturn, "polygonpoints", "frac");
  }

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }

  toReturn.define("coords", "frac");

  return toReturn;

}



} //# NAMESPACE CASA - END

