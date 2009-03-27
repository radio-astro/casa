//# DSPixelEllipse.cc : Implementation of an absolute pixel DSEllipse
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

#include <display/DisplayShapes/DSPixelEllipse.h>
#include <display/DisplayShapes/DSWorldEllipse.h>
#include <display/DisplayShapes/DSScreenEllipse.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSPixelEllipse::DSPixelEllipse() :
  DSEllipse() {

  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
}

DSPixelEllipse::DSPixelEllipse(const Record& settings) :
  DSEllipse() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  setOptions(settings);
}

DSPixelEllipse::DSPixelEllipse(DSScreenEllipse& other) :
  DSEllipse() {
  
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

  DSEllipse::setOptions(other.getRawOptions());
}

DSPixelEllipse::DSPixelEllipse(DSWorldEllipse& other) : 
  DSEllipse() {

  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  DSEllipse::setOptions(other.getRawOptions());

}


DSPixelEllipse::~DSPixelEllipse() {
  
}

Record DSPixelEllipse::getOptions() {
  Record toReturn;
  
  toReturn = DSEllipse::getOptions();
  
  DisplayShapeWithCoords::floatPointToPix(toReturn, "center");
  DisplayShapeWithCoords::floatToPix(toReturn, "majoraxis");
  DisplayShapeWithCoords::floatToPix(toReturn, "minoraxis");
  

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }

  toReturn.define("coords", "pix");
  
  return toReturn;
}

Bool DSPixelEllipse::setOptions(const Record& settings) {
  
  Bool localChange = False;
  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "pix") {
      throw(AipsError("I (DSPixelArrow) was expecting an option record which"
		      " had coords == \'pix\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  floatPointFromPix(toSet, "center");
  floatFromPix(toSet, "majoraxis");
  floatFromPix(toSet, "minoraxis");

  if (DSEllipse::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}



} //# NAMESPACE CASA - END

