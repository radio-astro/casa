//# DSLine.cc: Line implementation for "DisplayShapes"
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

#include <display/DisplayShapes/DSLine.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSLine::DSLine() :
  DSPolyLine() {
 
  itsValidStart = False;
  itsValidEnd = False;
}

DSLine::DSLine(const DSLine& other) :
  DSPolyLine(other),
  itsValidStart(other.itsValidStart),
  itsValidEnd(other.itsValidEnd),
  itsStart(other.itsStart),
  itsEnd(other.itsEnd) {
}

DSLine::~DSLine() {

}
DSLine::DSLine(const Vector<Float>& startPos, const Vector<Float>& endPos, 
	       const Bool& handles, const Bool& drawHandles) : 

  DSPolyLine() {

  // Valid:
  itsValidStart = True;
  itsValidEnd = True;
  itsStart = startPos;
  itsEnd = endPos;

  setHasHandles(handles);
  setDrawHandles(drawHandles);
  make();
}

void DSLine::make() {
  if (isValid()) {
    buildHandles(asPolyLine(itsStart, itsEnd));
    setPoints(asPolyLine(itsStart, itsEnd));
  } else {
    //cerr << "DSLine.cc - Debug - Create called too early!" << endl;
  }
}

Matrix<Float> DSLine::getEnds() {
  return(DSPolyLine::getPoints());
}

Bool DSLine::isValid() {
  return (itsValidStart && itsValidEnd);
}

void DSLine::setCenter(const Float& xPos, const Float& yPos) {
  if (isValid()) {
    DSPolyLine::setCenter(xPos, yPos);
  } else {
    itsStart(0) = xPos; itsStart(1) = yPos;
    itsEnd(0) = xPos; itsEnd(1) = yPos;
    itsValidStart = True;
    itsValidEnd = True;
    make();
  }
}

void DSLine::setStartPoint(const Vector<Float>& start) {
  if (isValid()) DSPolyLine::changePoint(start, 0);
  else {
    itsStart = start;
    if (!itsValidStart && itsValidEnd) {
      itsValidStart = True;
      make();
    } else itsValidStart = True;
  }
}

Record DSLine::getOptions() {
  Record rec = DSPolyLine::getOptions();

  if (rec.isDefined("type"))
    rec.removeField("type");
  rec.define("type", "line");

  Vector<Float> start; Vector<Float> end;
  
  if (rec.isDefined("polylinepoints")) {
    rec.removeField("polylinepoints");
    
    Matrix<Float> temp(getEnds());

    if (temp.nrow() >= 1) start = temp.row(0);
    if (temp.nrow() >= 2) end = temp.row(1);
    
    if (itsValidStart && temp.nrow() >= 1)
      rec.define("startpoint", start);
    if (itsValidEnd && temp.nrow() >= 2)
      rec.define("endpoint", end);
    
  }

  return rec;
}

Bool DSLine::setOptions(const Record& newSettings) {
  Bool localChange = False;

  if (newSettings.isDefined("startpoint")) {
    setStartPoint(newSettings.asArrayFloat("startpoint"));
    localChange = True;
  }
  
  if (newSettings.isDefined("endpoint")) {
    setEndPoint(newSettings.asArrayFloat("endpoint"));
    localChange = True;
  }

  if (DSPolyLine::setOptions(newSettings)) localChange = True;
  return localChange;
}

void DSLine::setEndPoint(const Vector<Float>& end) {
  itsEnd = end;
  if (isValid()) DSPolyLine::changePoint(end, 1);
  else if (itsValidStart && !itsValidEnd) {
    itsValidEnd = True;
    make();
  } else itsValidEnd = True;
}

Matrix<Float> DSLine::asPolyLine(const Vector<Float>& start, 
				 const Vector<Float>& end) {

  if (start.nelements() != 2 && end.nelements() != 2) {
    throw (AipsError("DSLine.cc ::asPolyLine - Error making a polyLine from two points"));
  }
  
  Matrix<Float> temp(2,2);
  for (uInt i=0;i<2;i++) {
    temp(0, i) = start[i];
    temp(1,i) = end[i];
  }
  return temp;
}









} //# NAMESPACE CASA - END

