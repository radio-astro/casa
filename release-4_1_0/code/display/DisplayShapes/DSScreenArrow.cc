//# DSScreenArrow.cc : Implementation of an absolute pixel DSArrow
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

#include <display/DisplayShapes/DSScreenArrow.h>
#include <display/DisplayShapes/DSWorldArrow.h>
#include <display/DisplayShapes/DSPixelArrow.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSScreenArrow::DSScreenArrow() :
  DSArrow(),
  itsPC(0),
  itsRelativeStart(0),
  itsRelativeEnd(0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
}

DSScreenArrow::DSScreenArrow(const Record& settings, PixelCanvas* pc) :
  DSArrow(),
  itsPC(pc),
  itsRelativeStart(0),
  itsRelativeEnd(0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  setOptions(settings);

}

DSScreenArrow::DSScreenArrow(DSPixelArrow& other, PixelCanvas* pc) :
  DSArrow(),
  itsPC(pc) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  DSArrow::setOptions(other.getRawOptions());
  updateRelative();
  
}

DSScreenArrow::DSScreenArrow(DSWorldArrow& other) :
  DSArrow(),
  itsPC(0) {
  
  UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
  
  itsPC = other.panelDisplay()->pixelCanvas();
  
  DSArrow::setOptions(other.getRawOptions());
  updateRelative();
}
 
DSScreenArrow::~DSScreenArrow() {

}
  

Bool DSScreenArrow::setOptions(const Record& settings) {
  Bool localChange = False;
  Record toSet = settings;

  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "frac") {
      throw(AipsError("I (DSScreenArrow) was expecting an option record which"
		      " had coords == \'frac\'. Please use a \'lock\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (settings.isDefined("startpoint")) {
    if (settings.dataType("startpoint") == TpRecord) {
      Record startField = settings.subRecord("startpoint");
      if (startField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the "
			"field \'startpoint\' must be composed of two "
			"subfields"
			" (x, y)"));
      }
      
      Record x = startField.subRecord(0);
      Record y = startField.subRecord(1);
      
      QuantumHolder xh, yh;
      String error;
      
      xh.fromRecord(error,x);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"extracting the \'x\' (first element) from the "
			"startpoint"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"extracting the \'y\' (second element) from the "
			"startpoint"
			" field caused an error: " + error));
      }
      
      if ((xh.asQuantumFloat().getFullUnit().getName() != "frac") ||
	  (yh.asQuantumFloat().getFullUnit().getName() != "frac")) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the units were incorrect for the field startpoint ("
			"I was expecting \'frac\' units!)"));
	
      }
      
      itsRelativeStart.resize(2);
      itsRelativeStart(0) = xh.asQuantumFloat().getValue();
      itsRelativeStart(1) = yh.asQuantumFloat().getValue();
      
      Vector<Float> screen(2);
      screen = relToScreen(itsRelativeStart, itsPC);
      
      toSet.removeField("startpoint");
      DSArrow::setStartPoint(screen);
      
    } else if (settings.dataType("startpoint") == TpArrayFloat) {
      
      throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
		      "the field"
		      " \'startpoint\' must be of type quanta, not an array"));
    } else {
      
      throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
		      "the field"
		      " \'startpoint\' was of an unknown type"));
    }
  } // END if isDefined("startpoint")

  if (settings.isDefined("endpoint")) {
    if (settings.dataType("endpoint") == TpRecord) {
      Record endField = settings.subRecord("endpoint");
      if (endField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the "
			"field \'endpoint\' must be composed of two "
			"subfields"
			" (x, y)"));
      }
      
      Record x = endField.subRecord(0);
      Record y = endField.subRecord(1);
      
      QuantumHolder xh, yh;
      String error;
      
      xh.fromRecord(error,x);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"extracting the \'x\' (first element) from the "
			"endpoint"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"extracting the \'y\' (second element) from the "
			"endpoint"
			" field caused an error: " + error));
      }
      
      if ((xh.asQuantumFloat().getFullUnit().getName() != "frac") ||
	  (yh.asQuantumFloat().getFullUnit().getName() != "frac")) {

	throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
			"the units were incorrect for the field endpoint ("
			"I was expecting \'frac\' units!)"));
	
      }
      
      itsRelativeEnd.resize(2);
      itsRelativeEnd(0) = xh.asQuantumFloat().getValue();
      itsRelativeEnd(1) = yh.asQuantumFloat().getValue();
      
      Vector<Float> screen(2);
      screen = relToScreen(itsRelativeEnd, itsPC);
      
      toSet.removeField("endpoint");
      DSArrow::setEndPoint(screen);
      
    } else if (settings.dataType("endpoint") == TpArrayFloat) {
      
      throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
		      "the field"
		      " \'endpoint\' must be of type quanta, not an array"));
    } else {
      
      throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
		      "the field"
		      " \'endpoint\' was of an unknown type"));
    }
  } // END if isDefined("endpoint")


  if (DSArrow::setOptions(toSet)) {
    localChange = True;
  }

  return localChange;
}


void DSScreenArrow::recalculateScreenPosition() {
  if (!itsPC || itsRelativeStart.nelements()==0 || 
      itsRelativeEnd.nelements() == 0) {

    throw(AipsError("Can't do recalculateScreenPosition() since I don't have"
		    " a valid pixelCanvas, a relative start or a relative "
		    "end saved!"));
  }
  
  Vector<Float> start;
  Vector<Float> end;

  start = relToScreen(itsRelativeStart, itsPC);
  end = relToScreen(itsRelativeEnd, itsPC);
  
  DSArrow::setStartPoint(start);
  DSArrow::setEndPoint(end);
}

void DSScreenArrow::move(const Float& dX, const Float& dY) {
  DSArrow::move(dX, dY);
  updateRelative();
}

void DSScreenArrow::setCenter(const Float& xPos, const Float& yPos) {
  DSArrow::setCenter(xPos, yPos);
  updateRelative();
}

void DSScreenArrow::rotate(const Float& angle) {
  DSArrow::rotate(angle);
  updateRelative();
}

void DSScreenArrow::scale(const Float& scaleFactor) {
  DSArrow::scale(scaleFactor);
  updateRelative();
}

void DSScreenArrow::setStartPoint(const Vector<Float>& startPoint) {
  DSArrow::setStartPoint(startPoint);
  updateRelative();
}

void DSScreenArrow::setEndPoint(const Vector<Float>& endPoint) {
  DSArrow::setEndPoint(endPoint);
  updateRelative();
}

void DSScreenArrow::changePoint(const Vector<Float>&pos, const Int n) {
  DSArrow::changePoint(pos, n);
  updateRelative();
}

void DSScreenArrow::changePoint(const Vector<Float>& pos) {
  DSArrow::changePoint(pos);
  updateRelative();
}


void DSScreenArrow::updateRelative() {
  
  if (validStart() && validEnd()) {
    itsRelativeStart.resize(2);
    itsRelativeEnd.resize(2);
    
    Matrix<Float> ends = getEnds();
    
    Vector<Float> start = ends.row(0);
    Vector<Float> end = ends.row(1);
  
    itsRelativeStart = screenToRel(start, itsPC);
    itsRelativeEnd = screenToRel(end, itsPC);
  } else {
  }
    
}



Record DSScreenArrow::getOptions() {

  Record toReturn;
  
  toReturn = DSArrow::getOptions();
  
  if (toReturn.isDefined("startpoint")) {
    if (toReturn.dataType("startpoint") != TpArrayFloat) {
      throw (AipsError("I (DSScreenArrow) received a bad option record from "
		       "DSArrow: the field \'startpoint\' was in an unexpected"
		       " format"));
    }
    
    Vector<Float> startPoint = toReturn.asArrayFloat("startpoint");
    
    if (startPoint.nelements() !=2) {
      throw (AipsError("I (DSScreenArrow) received a bad option record from "
		       "DSArrow: the field \'startpoint\' had an unexpected"
		       " number of elements"));
    }
    
    itsRelativeStart = screenToRel(startPoint, itsPC);
    
    
    Quantity x(itsRelativeStart(0), "frac");
    Quantity y(itsRelativeStart(1), "frac");
    
    QuantumHolder xh(x); 
    QuantumHolder yh(y);
    
    Record subx, suby, sub;
    String error;
    
    xh.toRecord(error, subx); 
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'startpoint\' (1st element) "
		      "as a "
		      "quantity in "
		      "DSScreenArrow, error occured: " + error));
    }
    
    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'startpoint\' (2nd element) "
		      "as a "
		      "quantity in "
		      "DSScreenArrow, error occured: " + error));
    }
    
    sub.defineRecord("x", subx);
    sub.defineRecord("y", suby);
    
    toReturn.removeField("startpoint");
    toReturn.defineRecord("startpoint", sub);
    
  } // end if Defined("startpoint")


  if (toReturn.isDefined("endpoint")) {
    if (toReturn.dataType("endpoint") != TpArrayFloat) {
      throw (AipsError("I (DSScreenArrow) received a bad option record from "
		       "DSArrow: the field \'endpoint\' was in an unexpected"
		       " format"));
    }
    
    Vector<Float> endPoint = toReturn.asArrayFloat("endpoint");
    
    if (endPoint.nelements() !=2) {
      throw (AipsError("I (DSScreenArrow) received a bad option record from "
		       "DSArrow: the field \'endPoint\' had an unexpected"
		       " number of elements"));
    }
    
    itsRelativeEnd = screenToRel(endPoint, itsPC);
    
    
    Quantity x(itsRelativeEnd(0), "frac");
    Quantity y(itsRelativeEnd(1), "frac");
    
    QuantumHolder xh(x); 
    QuantumHolder yh(y);
    
    Record subx, suby, sub;
    String error;
    
    xh.toRecord(error, subx); 
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'endpoint\' (1st element) "
		      "as a "
		      "quantity in "
		      "DSScreenArrow, error occured: " + error));
    }
    
    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create field \'endpoint\' (2nd element) "
		      "as a "
		      "quantity in "
		      "DSScreenArrow, error occured: " + error));
    }
    
    sub.defineRecord("x", subx);
    sub.defineRecord("y", suby);
    
    toReturn.removeField("endpoint");
    toReturn.defineRecord("endpoint", sub);
    
  } // end if Defined("endpoint")

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  
  toReturn.define("coords", "frac");

  return toReturn;

}







} //# NAMESPACE CASA - END

