//# DSWorldArrow.cc : Implementation of a world coords DSArrow
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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

#include <display/DisplayShapes/DSWorldArrow.h>
#include <display/DisplayShapes/DSPixelArrow.h>
#include <display/DisplayShapes/DSScreenArrow.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Containers/List.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Arrays/ArrayMath.h>

#include <coordinates/Coordinates/CoordinateSystem.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSWorldArrow::DSWorldArrow() :
  DSArrow(),
  itsPD(0),
  itsWC(0),
  itsWorldStart(0),
  itsWorldEnd(0) {
  
  // Default cons. We know nothing about anything.
  
}

DSWorldArrow::DSWorldArrow(const Record& settings, PanelDisplay* pd) :
  DSArrow(),
  itsPD(pd),
  itsWC(0),
  itsWorldStart(0),
  itsWorldEnd(0) {
  
  setOptions(settings);
}

DSWorldArrow::DSWorldArrow(DSScreenArrow& other, PanelDisplay* pd) :
  DSArrow(),
  itsPD(pd),
  itsWC(0),
  itsWorldStart(0),
  itsWorldEnd(0) {
  
  Record shapeSettings = other.getRawOptions();
  DSArrow::setOptions(shapeSettings);
  updateWCoords();
  
}

DSWorldArrow::DSWorldArrow(DSPixelArrow& other, PanelDisplay* pd) :
  DSArrow(),
  itsPD(pd),
  itsWC(0),
  itsWorldStart(0),
  itsWorldEnd(0) {
  
  Record shapeSettings = other.getRawOptions();
  DSArrow::setOptions(shapeSettings);
  updateWCoords();
}

DSWorldArrow::~DSWorldArrow() {

}

void DSWorldArrow::recalculateScreenPosition() {
  if (isValid()) {
    if (!itsWC) {
      if (itsWorldStart.nelements()!=2 || itsWorldEnd.nelements()!=2) {
	throw(AipsError("Can't determine WC without valid start and end"));
      }

      //Do this so we can re-use the fn we use for polys
      Record settings;
      QuantumHolder sx(itsWorldStart(0));
      QuantumHolder sy(itsWorldStart(1));
      QuantumHolder ex(itsWorldEnd(0));
      QuantumHolder ey(itsWorldEnd(1));

      Record rsx, rsy, rex, rey;
      String error;
      sx.toRecord(error, rsx);
      sy.toRecord(error, rsy);
      ex.toRecord(error, rex);
      ey.toRecord(error, rey);
      
      if (error.length() != 0) {
	throw(AipsError("Problem changing world points of arrow to record"));
      }
      Record start,end;
      
      start.defineRecord("x", rsx);
      start.defineRecord("y", rsy);
      end.defineRecord("x", rex);
      end.defineRecord("y", rey);
      
      Record points;
      points.defineRecord("start", start);
      points.defineRecord("end", end);
      settings.defineRecord("points", points);
      itsWC = chooseWCFromWorldPoints(settings, 
				      "points", 
				      itsPD);
      
      if (!itsWC) {
	throw(AipsError("DSWorldArrow couldn't update world position since "
			"it didn't have a valid worldcanvas"));
      }
      
    }
    
    Vector<Double> start(2);
    Vector<Double> end(2);
    
    start(0) = itsWorldStart(0).getValue();
    start(1) = itsWorldStart(1).getValue();
    
    end(0) = itsWorldEnd(0).getValue();
    end(1) = itsWorldEnd(1).getValue();
    
    Vector<Double> pixstart(2);
    Vector<Double> pixend(2);
    
    itsWC->worldToPix(pixstart, start);
    itsWC->worldToPix(pixend, end);
    
    Vector<Float> flPixStart(2);
    Vector<Float> flPixEnd(2);
    
    convertArray(flPixStart, pixstart);
    convertArray(flPixEnd, pixend);
    
    DSArrow::setStartPoint(flPixStart);
    DSArrow::setEndPoint(flPixEnd);
  }  
}
  
void DSWorldArrow::draw(PixelCanvas* pc) {
  if (isValid()) {
    Matrix<Float> ends = getEnds();
    Vector<Float> pixelStart = ends.row(0);
    Vector<Float> pixelEnd = ends.row(1);
    
    if (itsWC) {
      if (itsWC->inDrawArea(Int(pixelStart(0)+0.5), Int(pixelStart(1)+0.5)) &&
	  itsWC->inDrawArea(Int(pixelEnd(0)+0.5), Int(pixelEnd(1)+0.5))) {
	DSArrow::draw(pc);
      }
    }
  }

}


Bool DSWorldArrow::setOptions(const Record& settings) {
  Bool localChange = False;

  Record toSet = settings;
  
  if (settings.isDefined("coords")) {
    if (settings.asString("coords") != "world") {
      throw(AipsError("I (DSWorldArrow) was expecting an option record which"
		      " had coords == \'world\'. Please use a \'lock\' or"
		      " \'revert\' function"
		      " to change my co-ord system"));
    }
  }
  
  if (settings.isDefined("startpoint")) {
    localChange = True;
    if (settings.dataType("startpoint") == TpRecord) {
      
      Record startField = settings.subRecord("startpoint");

      if (startField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
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
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
			"extracting the \'x\' (first element) from the "
			"\'startpoint\'"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
			"extracting the \'y\' (second element) from the "
			"\'startpoint\' field caused an error: " + error));
      }
      
      
      // TODO Change this to are units compatible with current CS units? 
      
      itsWorldStart.resize(2);
      itsWorldStart(0) = xh.asQuantumDouble();
      itsWorldStart(1) = yh.asQuantumDouble();
      
      toSet.removeField("startpoint");
      
    } else {
      throw(AipsError("Bad data type for startpoint"));
    }
    // Put in a dummy start point. This will be overwritten by
    // 'recalculateScreenPosition' which occurs at the end of the fn.
    // This is to ensure that we know when we have a valid arrow 
    Vector<Float> dummy(2); dummy(0) = 0; dummy(1) = 0;
    DSArrow::setStartPoint(dummy);
  }
  
  if (settings.isDefined("endpoint")) {
    localChange = True;
    if (settings.dataType("endpoint") == TpRecord) {
      
      Record endField = settings.subRecord("endpoint");

      if (endField.nfields() != 2) {
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
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
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
			"extracting the \'x\' (first element) from the "
			"\'endpoint\'"
			" field caused an error: " + error));
      }
      
      yh.fromRecord(error,y);
      if (error.length() != 0) {
	throw(AipsError("Bad record to setoptions() of DSWorldArrow, since "
			"extracting the \'y\' (second element) from the "
			"\'endpoint\' field caused an error: " + error));
      }
      
      
      /* Change this to are units compatible with current CS units?

      if ((xh.asQuantumFloat().getFullUnit().getName() != "") ||
      (yh.asQuantumFloat().getFullUnit().getName() != "frac")) {
      throw(AipsError("Bad record to setoptions() of DSScreenArrow, since "
      "the units were incorrect for the field center ("
      "I was expecting \'frac\' units!)"));
      
      }
      */
      
      itsWorldEnd.resize(2);
      itsWorldEnd(0) = xh.asQuantumDouble();
      itsWorldEnd(1) = yh.asQuantumDouble();
      
      toSet.removeField("endpoint");
      
    } else {
      throw(AipsError("Bad data type for endpoint"));
    }
    // Put in a dummy end point. This will be overwritten by
    // 'recalculateScreenPosition' which occurs at the end of the fn.
    // This is to ensure that we know when we have a valid arrow 
    Vector<Float> dummy(2); dummy(0) = 0; dummy(1) = 0;
    DSArrow::setEndPoint(dummy);
  }

  if (localChange) {
    recalculateScreenPosition();
  }
  
  if (DSArrow::setOptions(toSet)) {
    localChange = True;
  }
  
  return localChange;
}

Record DSWorldArrow::getOptions() {
  Record toReturn;
  toReturn = DSArrow::getOptions();

  updateWCoords();
  
  if (itsWorldStart.nelements() == 2 &&
      itsWorldEnd.nelements() == 2) {

    QuantumHolder qsx(itsWorldStart(0));
    QuantumHolder qsy(itsWorldStart(1));
    QuantumHolder qex(itsWorldEnd(0));
    QuantumHolder qey(itsWorldEnd(1));

    Record start, end;
    Record sx, sy, ex, ey;

    String error;

    qsx.toRecord(error, sx);
    qsy.toRecord(error, sy);
    
    qex.toRecord(error, ex);
    qey.toRecord(error, ey);
    
    if (error.length() != 0) {
      throw(AipsError(String("Error going worldpoints -> record in Arrow "
		      "getOptions! :" + error)));
    }

    
    start.defineRecord("x", sx);
    start.defineRecord("y", sy);
    
    end.defineRecord("x", ex);
    end.defineRecord("y", ey);

    if (toReturn.isDefined("startpoint")) {
      toReturn.removeField("startpoint");
    }

    if (toReturn.isDefined("endpoint")) {
      toReturn.removeField("endpoint");
    }
    
    toReturn.defineRecord("startpoint", start);
    toReturn.defineRecord("endpoint", end);

  } 

  // Shouldn't happen (should never be defined) .. but why not
  if (toReturn.isDefined("coords")) {
    toReturn.removeField("coords");
  }
  
  toReturn.define("coords", "world");
  return toReturn;
}


WorldCanvas* DSWorldArrow::chooseWC(const Float& startXPos, 
				    const Float& startYPos,
				    const Float& endXPos, 
				    const Float& endYPos,
				    PanelDisplay* pd) {
  
  Bool success = False;
  WorldCanvas* toReturn(0);
  
  // Look for ones where the point is in WC and in draw area
  ListIter<WorldCanvas* >* wcs = pd->wcs();

  wcs->toStart();
  
  while( !wcs->atEnd() && !success) {
    if (wcs->getRight()->inWC(Int(startXPos+0.5), Int(startYPos+0.5)) &&
	wcs->getRight()->inWC(Int(endXPos+0.5), Int(endYPos+0.5)) &&
	wcs->getRight()->inDrawArea(Int(startXPos+0.5), Int(startYPos+0.5)) &&
	wcs->getRight()->inDrawArea(Int(endXPos+0.5), Int(endYPos+0.5))) {

      
      const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());


      if (test) {
	toReturn = wcs->getRight();
	success = True;
      } else {
	wcs->step();
      }
      
    } else {
      wcs->step();
    }
  }

  // if that returns nothing, look for one just in WC..
  if (!success) {
    wcs->toStart();

    while(!wcs->atEnd() && !success) {
      if (wcs->getRight()->inWC(Int(startXPos+0.5), Int(startYPos+0.5)) &&
	  wcs->getRight()->inWC(Int(endXPos+0.5), Int(endYPos+0.5))) {
	
	const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());
	if (test) {
	  toReturn = wcs->getRight();
	  success = True;
	} else {
	  wcs->step();
	}
	
      } else {
	wcs++;
      }
    }
  }

  return toReturn;
}

void DSWorldArrow::updateWCoords() {
  if (isValid()) {

    Matrix<Float> ends = getEnds();
    
    Vector<Float> pixelStart = ends.row(0);
    Vector<Float> pixelEnd = ends.row(1);

    if (!itsWC) {

      // Ok , we are valid (start + end) so lets try and find a WC, if we 
      // cant find one, then we throw 
      
      itsWC = chooseWC(pixelStart(0), pixelStart(1), pixelEnd(0), 
		       pixelEnd(1), itsPD);
      if (!itsWC) {
	throw(AipsError("No WorldCanvas available to update the world "
			"co-ordinates of arrow."));
      }
    }

    const Vector<String>& units = itsWC->coordinateSystem().worldAxisUnits();

    Vector<Double> dblPixStart(pixelStart.nelements()); 
    Vector<Double> dblPixEnd(pixelEnd.nelements());
  
    convertArray(dblPixStart, pixelStart);
    convertArray(dblPixEnd, pixelEnd);
    
    Vector<Double> worldStart;
    Vector<Double> worldEnd;
    
    Bool suc = itsWC->pixToWorld(worldStart, dblPixStart);
    suc = suc & itsWC->pixToWorld(worldEnd, dblPixEnd);
    
    if (worldStart.nelements() != 2 || worldEnd.nelements() != 2) {
      throw(AipsError("I (DSWorldArrow) am not yet sure how to handle "
		      "a conversion to world co-ords if I get returned !=2"
		      " co-ordinates. Sorry"));
    }
    
    Quantity xs(worldStart(0), units(0));
    Quantity ys(worldStart(1), units(1));
    Quantity xe(worldEnd(0), units(0));
    Quantity ye(worldEnd(1), units(1));
    

    itsWorldStart.resize(2);
    itsWorldEnd.resize(2);
    
    itsWorldStart(0) = xs;
    itsWorldStart(1) = ys;
    itsWorldEnd(0) = xe;
    itsWorldEnd(1) = ye;
    
  /*
    QuantumHolder xh(x) , yh(y);
  
  String error;

  Record subx,suby, subRecord;
  xh.toRecord(error, subx);

  if (error.length() != 0) {
    throw(AipsError("Error in DSWorldArrow when trying to save the first elem"
		    " (x) of the center as a world co-ordinate, the error "
		    "was: " + error));
  }
  
  yh.toRecord(error, suby);
  if (error.length() != 0) {
    throw(AipsError("Error in DSWorldArrow when trying to save the second elem"
		    " (y) of the center as a world co-ordinate, the error "
		    "was: " + error));
  }
  
  subRecord.define("x", subx);
  subRecord.define("y", suby);
  */
  } else {

  }
}

/*
WorldCanvas* DSWorldArrow::chooseWC(const Record& settings, PanelDisplay* pd) {

  Bool success = False;
  WorldCanvas* toReturn;

  // Look for ones where the point is in WC and in draw area
  ListIter<WorldCanvas* >* wcs = pd->wcs();
  
  // if that returns nothing, look for one just in WC..
  return success;
}

*/

void DSWorldArrow::move(const Float& dX, const Float& dY) {
  DSArrow::move(dX, dY);
  updateWCoords();
}

void DSWorldArrow::setCenter(const Float& xPos, const Float& yPos) {
  
  DSArrow::setCenter(xPos, yPos);
  updateWCoords();
  
}

void DSWorldArrow::rotate(const Float& angle) {

  DSArrow::rotate(angle);
  updateWCoords();

}

void DSWorldArrow::scale(const Float& scaleFactor) {

  DSArrow::scale(scaleFactor);
  updateWCoords();

}

void DSWorldArrow::setStartPoint(const Vector<Float>& startPoint) {
  if (!itsWC && itsPD) {

    if (validEnd()) {
      DSArrow::setStartPoint(startPoint);
      Matrix<Float> ends = getEnds();
      Vector<Float> start = ends.row(0);
      Vector<Float> end = ends.row(1);

      itsWC = chooseWC(start(0), start(1), end(0), end(1), 
		       itsPD);
      
      if (!itsWC) {
	throw(AipsError("There was an error constructing a world canvas "
			"arrow. The first time I got information about "
			"its ends, I tried to lock it to a WC, but that "
			"failed.")); 
	
      } else {
	updateWCoords();
      }

    } else {
      DSArrow::setStartPoint(startPoint);
    }
  } else {
    
    DSArrow::setStartPoint(startPoint);
    updateWCoords();

  }
}

void DSWorldArrow::setEndPoint(const Vector<Float>& endPoint) {
  if (!itsWC && itsPD) {
    
    if (validStart()) {
      DSArrow::setEndPoint(endPoint);
      Matrix<Float> ends = getEnds();
      Vector<Float> start = ends.row(0);
      Vector<Float> end = ends.row(1);

      itsWC = chooseWC(start(0), start(1), end(0), end(1), 
		       itsPD);
      if (!itsWC) {
	throw(AipsError("There was an error constructing a world canvas "
			"arrow. The first time I got information about "
			"its ends, I tried to lock it to a WC, but that "
			"failed.")); 
	
      } else {
	updateWCoords();
      }
    } else {
      DSArrow::setEndPoint(endPoint);
    }
  } else {
    
    DSArrow::setEndPoint(endPoint);
    updateWCoords();
  }
}

void DSWorldArrow::changePoint(const Vector<Float>& pos, const Int n) {
  if (n==0) {
    setStartPoint(pos);
  } else if (n==1) {
    setEndPoint(pos);
  } else throw (AipsError("DSWorldArros.cc - changePoint must have n == 0 or 1"
			  " since an arrow will never have more than 2 "
			  "points!"));
}
 
void DSWorldArrow::changePoint(const Vector<Float>& pos) {
  DSArrow::changePoint(pos);
  updateWCoords();
}









} //# NAMESPACE CASA - END

