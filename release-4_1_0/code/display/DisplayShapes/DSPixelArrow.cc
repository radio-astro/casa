//# DSPixelArrow.cc : Implementation of DSArrow with abs. pixel coords
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

#include <display/DisplayShapes/DSPixelArrow.h>
#include <display/DisplayShapes/DSScreenArrow.h>
#include <display/DisplayShapes/DSWorldArrow.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSPixelArrow::DSPixelArrow() :
DSArrow() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

}

DSPixelArrow::DSPixelArrow(const Record& settings) :
  DSArrow() {
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  setOptions(settings);
}

DSPixelArrow::DSPixelArrow(DSScreenArrow& other) :
  DSArrow() {
  
  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");
  
  DSArrow::setOptions(other.getRawOptions());

}

DSPixelArrow::DSPixelArrow(DSWorldArrow& other) :
  DSArrow() {

  UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

  DSArrow::setOptions(other.getRawOptions());
  
}

DSPixelArrow::~DSPixelArrow() {

}

Record DSPixelArrow::getOptions() {
  Record toReturn;
  
  toReturn = DSArrow::getOptions();
  
  if (toReturn.isDefined("startpoint")) {
    if (toReturn.dataType("startpoint") != TpArrayFloat) {
      throw (AipsError("I (DSPixelArrow) received a bad option record from "
		       "DSArrow: the field \'startpoint\' was in an unexpected"
		       " format"));
    }
    
    Vector<Float> start = toReturn.asArrayFloat("startpoint");

    if (start.nelements() !=2) {
      throw (AipsError("I (DSPixelArrow) received a bad option record from "
		       "DSArrow: the field \'startpoint\' had an unexpected"
		       " number of elements"));
    }

    Quantity x(start(0), "pix");
    Quantity y(start(1), "pix");

    QuantumHolder xh(x); 
    QuantumHolder yh(y);
    
    Record subx, suby, sub;
    String error;
    
    xh.toRecord(error, subx); 
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'startpoint\' (1st element) "
		      "as a "
		      "quantity in "
		      "DSPixelArrow, error occured: " + error));
    }

    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'startpoint\' (2nd element) "
		      "as a "
		      "quantity in "
		      "DSPixelArrow, error occured: " + error));
    }
    
    sub.defineRecord("x", subx);
    sub.defineRecord("y", suby);

    toReturn.removeField("startpoint");
    toReturn.defineRecord("startpoint", sub);
    
  }
  
  if (toReturn.isDefined("endpoint")) {
    if (toReturn.dataType("endpoint") != TpArrayFloat) {
      throw (AipsError("I (DSPixelArrow) received a bad option record from "
		       "DSArrow: the field \'endpoint\' was in an unexpected"
		       " format"));
    }
    
    Vector<Float> end = toReturn.asArrayFloat("endpoint");

    if (end.nelements() !=2) {
      throw (AipsError("I (DSPixelArrow) received a bad option record from "
		       "DSArrow: the field \'endpoint\' had an unexpected"
		       " number of elements"));
    }

    Quantity x(end(0), "pix");
    Quantity y(end(1), "pix");

    QuantumHolder xh(x); 
    QuantumHolder yh(y);
    
    Record subx, suby, sub;
    String error;
    
    xh.toRecord(error, subx); 
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'endpoint\' (1st element) "
		      "as a "
		      "quantity in "
		      "DSPixelArrow, error occured: " + error));
    }

    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'endpoint\' (2nd element) "
		      "as a "
		      "quantity in "
		      "DSPixelArrow, error occured: " + error));
    }
    
    sub.defineRecord("x", subx);
    sub.defineRecord("y", suby);

    toReturn.removeField("endpoint");
    toReturn.defineRecord("endpoint", sub);
    
  }
  

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  toReturn.define("coords", "pix");
  
  return toReturn;
}

Bool DSPixelArrow::setOptions(const Record& settings) {
  
  Bool localChange = False;
  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "pix") {
      throw(AipsError("I (DSPixelArrow) was expecting an option record which"
		      " had coords == \'pix\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (settings.isDefined("startpoint")) {
    
    if (settings.dataType("startpoint") == TpRecord) {
      
      Record startField = settings.subRecord("startpoint");
      if (startField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"the "
			"field \'startpoint\' must be composed of "
			"two subfields"
			" (x, y)"));
      }

      Record x = startField.subRecord(0);
      Record y = startField.subRecord(1);
      
      QuantumHolder xh, yh;
      String error;
      
      xh.fromRecord(error,x);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"extracting the \'x\' (first element) from the "
			"startpoint"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"extracting the \'y\' (second element) from the "
			"startpoint"
			" field caused an error: " + error));
      }
      
      if ((xh.asQuantumFloat().getFullUnit().getName() != "pix") ||
	  (yh.asQuantumFloat().getFullUnit().getName() != "pix")) {

	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the units were incorrect for the field center ("
			"I was expecting \'pix\' units!)"));
	
      }

      
      Vector<Float> pixStart(2);
      pixStart(0) = xh.asQuantumFloat().getValue();
      pixStart(1) = yh.asQuantumFloat().getValue();
      
      toSet.removeField("startpoint");
      toSet.define("startpoint", pixStart);
      
    } else if (settings.dataType("startpoint") == TpArrayFloat) {
      
      throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
		      "the field"
		      " \'startpoint\' must be of type quanta, not an array"));
    } else {
      
      throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
		      "the field"
		      " \'startpoint\' was of an unknown type"));
    }
    
  } // END isDefined("startpoint")

  if (settings.isDefined("endpoint")) {
    
    if (settings.dataType("endpoint") == TpRecord) {
      
      Record endField = settings.subRecord("endpoint");
      if (endField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"the "
			"field \'endpoint\' must be composed of "
			"two subfields"
			" (x, y)"));
      }
      
      Record x = endField.subRecord(0);
      Record y = endField.subRecord(1);
      
      QuantumHolder xh, yh;
      String error;
      
      xh.fromRecord(error,x);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"extracting the \'x\' (first element) from the "
			"endpoint"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
			"extracting the \'y\' (second element) from the "
			"endpoint"
			" field caused an error: " + error));
      }
      
      if ((xh.asQuantumFloat().getFullUnit().getName() != "pix") ||
	  (yh.asQuantumFloat().getFullUnit().getName() != "pix")) {
	
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the units were incorrect for the field endpoint ("
			"I was expecting \'pix\' units!)"));
	
      }

      
      Vector<Float> pixEnd(2);
      pixEnd(0) = xh.asQuantumFloat().getValue();
      pixEnd(1) = yh.asQuantumFloat().getValue();
      
      toSet.removeField("endpoint");
      toSet.define("endpoint", pixEnd);
      
    } else if (settings.dataType("endpoint") == TpArrayFloat) {
      
      throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
		      "the field"
		      " \'endpoint\' must be of type quanta, not an array"));
    } else {
      
      throw(AipsError("Bad record to setoptions() of DSPixelArrow, since "
		      "the field"
		      " \'endpoint\' was of an unknown type"));
    }
    
  } // END isDefined("endpoint")


  if (DSArrow::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}





} //# NAMESPACE CASA - END

