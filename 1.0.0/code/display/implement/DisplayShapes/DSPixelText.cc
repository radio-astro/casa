//# DSPixelText.cc : Implementation of an absolute pixel DSText
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

#include <display/DisplayShapes/DSPixelText.h>
#include <display/DisplayShapes/DSScreenText.h>
#include <display/DisplayShapes/DSWorldText.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSPixelText::DSPixelText() :
DSText() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
}

DSPixelText::DSPixelText(const Record& settings) :
  DSText() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  setOptions(settings);
}

DSPixelText::DSPixelText(DSScreenText& other) :
  DSText() {

  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  DSText::setOptions(other.getRawOptions());
  //copy cons
}

DSPixelText::DSPixelText(DSWorldText& other) :
  DSText() {

  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  DSText::setOptions(other.getRawOptions());

}

DSPixelText::~DSPixelText() {

}

Record DSPixelText::getOptions() {
  Record toReturn;
  
  toReturn = DSText::getOptions();
  
  if (toReturn.isDefined("center")) {
    if (toReturn.dataType("center") != TpArrayFloat) {
      throw (AipsError("I (DSPixelText) received a bad option record from "
		       "DSText: the field \'center\' was in an unexpected"
		       " format"));
    }

    Vector<Float> center = toReturn.asArrayFloat("center");

    if (center.nelements() !=2) {
      throw (AipsError("I (DSPixelText) received a bad option record from "
		       "DSText: the field \'center\' had an unexpected"
		       " number of elements"));
    }

    Quantity x(center(0), "pix");
    Quantity y(center(1), "pix");

    QuantumHolder xh(x); 
    QuantumHolder yh(y);

    Record subx, suby, sub;
    String error;

    xh.toRecord(error, subx); 
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'center\' (1st element) as a "
		      "quantity in "
		      "DSPixelText, error occured: " + error));
    }

    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'center\' (2nd element) as a "
		      "quantity in "
		      "DSPixelText, error occured: " + error));
    }
    
    sub.defineRecord("x", subx);
    sub.defineRecord("y", suby);

    toReturn.removeField("center");
    toReturn.defineRecord("center", sub);
    
  }
  
  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  toReturn.define("coords", "pix");
  
  return toReturn;
}

Bool DSPixelText::setOptions(const Record& settings) {
  
  Bool localChange = False;
  Record toSet = settings;

  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "pix") {
      throw(AipsError("I (DSPixelText) was expecting an option record which"
		      " had coords == \'pix\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (settings.isDefined("center")) {

    if (settings.dataType("center") == TpRecord) {
      
      Record centerField = settings.subRecord("center");
      if (centerField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSPixelText, since "
			"the "
			"field \'center\' must be composed of two subfields"
			" (x, y)"));
      }

      Record x = centerField.subRecord(0);
      Record y = centerField.subRecord(1);
      
      QuantumHolder xh, yh;
      String error;
      
      xh.fromRecord(error,x);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelText, since "
			"extracting the \'x\' (first element) from the center"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelText, since "
			"extracting the \'y\' (second element) from the center"
			" field caused an error: " + error));
      }
      
      if ((xh.asQuantumFloat().getFullUnit().getName() != "pix") ||
	  (yh.asQuantumFloat().getFullUnit().getName() != "pix")) {
	throw(AipsError("Bad record to setoptions() of DSScreenText, since "
			"the units were incorrect for the field center ("
			"I was expecting \'pix\' units!)"));
	
      }

      Float pixX = xh.asQuantumFloat().getValue();
      Float pixY = yh.asQuantumFloat().getValue();
      
      Vector<Float> pixCent(2);
      pixCent(0) = pixX;
      pixCent(1) = pixY;

      toSet.removeField("center");
      toSet.define("center", pixCent);
      
      localChange = DSText::setOptions(toSet);

    } else if (settings.dataType("center") == TpArrayFloat) {
      
      throw(AipsError("Bad record to setoptions() of DSPixelText, since "
		      "the field"
		      " \'center\' must be of type quanta, not an array"));
    } else {
      
      throw(AipsError("Bad record to setoptions() of DSPixelText, since "
		      "the field"
		      " \'center\' was of an unknown type"));
    }
    
  }

  if (DSText::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}




} //# NAMESPACE CASA - END

