//# DSPixelPolyLine.cc : Implementation of an absolute pixel DSPolyLine
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

#include <display/DisplayShapes/DSPixelPolyLine.h>
#include <display/DisplayShapes/DSScreenPolyLine.h>
#include <display/DisplayShapes/DSWorldPolyLine.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSPixelPolyLine::DSPixelPolyLine() :
  DSPolyLine() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

}

DSPixelPolyLine::DSPixelPolyLine(const Record& settings) :
  DSPolyLine() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  setOptions(settings);
}

DSPixelPolyLine::DSPixelPolyLine(DSScreenPolyLine& other) :
  DSPolyLine() {
  
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  DSPolyLine::setOptions(other.getRawOptions());
  
  
}

DSPixelPolyLine::DSPixelPolyLine(DSWorldPolyLine& other) :
  DSPolyLine() {
  
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  DSPolyLine::setOptions(other.getRawOptions());
  
}

DSPixelPolyLine::~DSPixelPolyLine() {

}

Record DSPixelPolyLine::getOptions() {
  Record toReturn;
  
  toReturn = DSPolyLine::getOptions();
  
  matrixFloatToQuant(toReturn, "polylinepoints", "pix");
  
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  
  toReturn.define("coords", "pix");
  
  return toReturn;
}

Bool DSPixelPolyLine::setOptions(const Record& settings) {
  
  Bool localChange = False;
  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "pix") {
      throw(AipsError("I (DSPixelPolyLine) was expecting an "
		      "option record which"
		      " had coords == \'pix\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (DSPolyLine::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}





} //# NAMESPACE CASA - END

