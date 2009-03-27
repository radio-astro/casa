//# DisplayShapeWithCoords.cc : Base class for DisplayShapes with Coords
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

#include <display/DisplayShapes/DisplayShapeWithCoords.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <coordinates/Coordinates/CoordinateSystem.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>
#include <display/Display/WorldCanvas.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

void DisplayShapeWithCoords::floatToPix(Record& inHere, 
					       const String& field) {
  
  if (!inHere.isDefined(field)) {
    return;
  }
  
  if (inHere.dataType(field) != TpFloat) {
    throw (AipsError("When trying to get an option record from a shape, "
		     "some of the data (field name : \'" + field + "\')"
		     " was in a bad format"));
  }
  
  Float data = inHere.asFloat(field);
  
  Quantity quant(data, "pix");
  QuantumHolder quanth(quant);

  Record sub;
  String error;

  quanth.toRecord(error, sub);
  if (error.length() != 0) {
    throw(AipsError("I couldn't quantify \'" + field +"\' because when I tried"
		    " to write to a record, I received the error: " +error));
  }
  
  inHere.removeField(field);
  inHere.defineRecord(field, sub);
  
  return ;
}

void DisplayShapeWithCoords::floatPointToPix(Record& inHere, 
					     const String& field) {


  if (!inHere.isDefined(field)) {
    return;
  }
  
  if (inHere.dataType(field) != TpArrayFloat) {
    throw (AipsError("When trying to get an option record from a shape, "
		     "some of the data (field name : \'" + field + "\')"
		     " was in a bad format"));
  }
  
  Vector<Float> data = inHere.asArrayFloat(field);

  if (data.nelements() !=2) {
    throw (AipsError("When trying to get an option record from a shape, "
		     "some of the data (field name : \'" + field + "\')"
		     " was in a bad format (it had the wrong number of"
		     " fields"));
  }
  
  Quantity quantx(data(0), "pix");
  Quantity quanty(data(1), "pix");
  
  QuantumHolder quanthx(quantx);
  QuantumHolder quanthy(quanty);

  Record sub, subx, suby;
  
  String error;

  quanthx.toRecord(error, subx);
  if (error.length() != 0) {
    throw(AipsError("I couldn't quantify \'" + field +"\' because when I tried"
		    " to write to a record (first element), I received the "
		    "error: " +error));
  }
  
  quanthy.toRecord(error, suby);
  if (error.length() != 0) {
    throw(AipsError("I couldn't quantify \'" + field +"\' because when I tried"
		    " to write to a record (second element), I received the "
		    "error: " +error));
  }
  
  inHere.removeField(field);
  
  sub.defineRecord("x", subx);
  sub.defineRecord("y", suby);
  
  inHere.defineRecord(field, sub);
  
  return ;

}

void DisplayShapeWithCoords::floatFromPix(Record &inHere, const String& field) {

  if (!inHere.isDefined(field)) {
    return;
  }

  if (inHere.dataType(field) != TpRecord) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " wasn't in the expected format"));
  }

  Record subRecord = inHere.subRecord("field");
  
  if (subRecord.nfields() != 1) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " didn't have the right number of fields (was expecting"
		    " 1 i.e. a scalar)"));
  }

  QuantumHolder qh;
  String error;

  qh.fromRecord(error, subRecord);
  if (error.length() != 0) {
    throw(AipsError("Error extracting data. When trying to extract from"
		    " \'" +field+ "\' into a record, the following"
		    " error occured: " +error));
  }


  if (qh.asQuantumFloat().getFullUnit().getName() != "pix")
    throw(AipsError("Error extracting data. I was expecting the data to "
		    "be of type \'pix\'"));

  Float data = qh.asQuantumFloat().getValue();
  
  inHere.removeField(field);
  inHere.define(field, data);

}

Vector<Float> DisplayShapeWithCoords::floatPointFromQuant(const Record& inHere,
							  const String& field,
							  String& units) 
{
  if (!inHere.isDefined(field)) {
    throw(AipsError("Field not defined (floatPointFromQuant)"));
  }
  
  if (inHere.dataType(field) != TpRecord) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " wasn't in the expected format"));
  }
  
  Record subRecord = inHere.subRecord("field");
  
  if (subRecord.nfields() != 2) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " didn't have the right number of fields (was expecting"
		    " 2 i.e. a point)"));
  }
  
  Record x = subRecord.subRecord(0);
  Record y = subRecord.subRecord(1);
  
  QuantumHolder xh,yh;
  String error;
  
  xh.fromRecord(error, x);
  if (error.length() != 0) {
    throw(AipsError("Error extracting data. When trying to make the first"
		    "element of \'" +field+ "\' into a record, the following"
		    " error occured: " +error));
  }
  yh.fromRecord(error,y);
  if (error.length() != 0) {
    throw(AipsError("Error extracting data. When trying to make the second"
		    "element of \'" +field+ "\' into a record, the following"
		    " error occured: " +error));
  }
  
  if (xh.asQuantumFloat().getFullUnit().getName() !=
      yh.asQuantumFloat().getFullUnit().getName() ) {
    throw(AipsError("Error extracting data. I was expecting the data to "
		    "be of the same type"));
  }
  units = xh.asQuantumFloat().getFullUnit().getName();
  
  Vector<Float> dataVector(2);
  dataVector(0) = xh.asQuantumFloat().getValue();
  dataVector(1) = yh.asQuantumFloat().getValue();
  
  return dataVector;
}
void DisplayShapeWithCoords::floatPointFromPix(Record& inHere, const String& field) {
  
  if (!inHere.isDefined(field)) {
    return;
  }
  
  if (inHere.dataType(field) != TpRecord) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " wasn't in the expected format"));
  }

  Record subRecord = inHere.subRecord("field");
  
  if (subRecord.nfields() != 2) {
    throw(AipsError("I couldn't extract the data from the quanta in the "
		    "record (field : \'" + field + "\') because the data"
		    " didn't have the right number of fields (was expecting"
		    " 2 i.e. a point)"));
  }

  Record x = subRecord.subRecord(0);
  Record y = subRecord.subRecord(1);
  
  QuantumHolder xh,yh;
  String error;
  
  xh.fromRecord(error, x);
  if (error.length() != 0) {
    throw(AipsError("Error extracting data. When trying to make the first"
		    "element of \'" +field+ "\' into a record, the following"
		    " error occured: " +error));
  }
  yh.fromRecord(error,y);
  if (error.length() != 0) {
    throw(AipsError("Error extracting data. When trying to make the second"
		    "element of \'" +field+ "\' into a record, the following"
		    " error occured: " +error));
  }

  if ((xh.asQuantumFloat().getFullUnit().getName() != "pix") ||
      (yh.asQuantumFloat().getFullUnit().getName() != "pix")) {
    throw(AipsError("Error extracting data. I was expecting the data to "
		    "be of type \'pix\'"));
  }

  Vector<Float> dataVector(2);
  dataVector(0) = xh.asQuantumFloat().getValue();
  dataVector(1) = yh.asQuantumFloat().getValue();

  inHere.removeField(field);
  inHere.define(field, dataVector);

}


Vector<Float> DisplayShapeWithCoords::relToScreen(const Vector<Float>& rel, 
						  PixelCanvas* pc) {
  
  if (rel(0) < 0 || rel(0) > 1 || rel(1) < 0 || rel(1) > 1 ) {
    throw(AipsError("Relative co-ordinates are only valid of they are between"
		    " 0 and 1."));
    
  }
  
  if (!pc) {
    throw(AipsError("Can't do relative to screen if I don't have a valid"
		    " pixelCanvas!"));
  }

  Vector<Float> pix(2);
  
  pix(0) = pc->width() * rel(0);
  pix(1) = pc->height() * rel(1);

  if ((     isNaN(pix(0)) || isInf(pix(0)) 
	    || isNaN(pix(1)) || isInf(pix(1)) )) {
    
    pix(0) = 0; pix(1) = 0;
  }

  return pix;
}


void DisplayShapeWithCoords::matrixFloatFromQuant(Record& inHere, 
						  const String& field,
						  const String& unit) {
  if (!inHere.isDefined(field)) {
    return;
  }
  
  if (inHere.dataType(field) != TpRecord) {
    throw (AipsError("Bad data type found when trying to make matrix "
		     "from quantum (was expecting"
		     " a record)"));
  }

  Record subRecord = inHere.subRecord(field);

  uInt numberPoints = subRecord.nfields();
  Matrix<Float> extracted(numberPoints, 2);
  
  for (uInt i=0 ; i < numberPoints ; i ++) {
    Record point = subRecord.subRecord(i);
    
    if (point.nfields() != 2) {
      throw (AipsError("Bad point found... nfields != 2"));
    }
    
    Record x = point.subRecord(0);
    Record y = point.subRecord(1);

    QuantumHolder xh, yh;
    String error;

    xh.fromRecord(error, x);
    if (error.length() != 0) {
      throw(AipsError("Error extracting data. When trying to extract from"
		      " \'" +field+ "\' into a record, the following"
		      " error occured: " +error));
    }

    yh.fromRecord(error, y);
    if (error.length() !=0) {
      throw(AipsError("Error extracting data. When trying to extract from"
		      " \'" +field+ "\' into a record, the following"
		      " error occured: " +error));
    }

    if (unit != "?") {
      if (xh.asQuantumFloat().getFullUnit().getName() != unit ||
	  yh.asQuantumFloat().getFullUnit().getName() != unit) {
	
	throw(AipsError("Unexpected units found"));
	
      }
    }
    
    extracted(i,0) = xh.asQuantumFloat().getValue();
    extracted(i,1) = yh.asQuantumFloat().getValue();
    
  }
  inHere.removeField(field);
  inHere.define(field, extracted);
  
}

void DisplayShapeWithCoords::matrixFloatToQuant(Record& inHere, 
						const String& field,
						const String& unit) {
  
  
  if (!inHere.isDefined(field)) {
    return;
  }
  
  if (inHere.dataType(field) != TpArrayFloat) {
    throw (AipsError("Bad data type returned when trying to make quantum "
		     "from matrix (was expecting"
		     " an array of Floats)"));
  } 
  
  Matrix<Float> points = inHere.asArrayFloat(field);
  
  Record subRecord, pointsRecord;
  
  for (uInt i=0; i < points.nrow(); i++) {
    Vector<Float> point = points.row(i);
    Quantity x(point(0), unit);
    Quantity y(point(1), unit);
    
    QuantumHolder xh(x);
    QuantumHolder yh(y);
    
    Record subx, suby;
    String error;
    
    xh.toRecord(error, subx);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create one of the fields needed to "
		      "return as an option record (point " + String(i) + 
		      " , element 1)"));
    }
    yh.toRecord(error, suby);
    if (error.length() != 0) {
      throw(AipsError("Couldn\'t create one of the fields needed to "
		      "return as an option record (point " + String(i) + 
		      " , element 2)"));
    }
    
    subRecord.defineRecord("x", subx);
    subRecord.defineRecord("y", suby);
    
    pointsRecord.defineRecord(String("point " + String(i)), subRecord);
    
  }
  inHere.removeField(field);
  inHere.defineRecord(field, pointsRecord);
  
  
  
}

Vector<Float> DisplayShapeWithCoords::screenToRel(const Vector<Float>& screen, 
						  PixelCanvas* pc) {

  if (!pc) {
    throw(AipsError("Can't do screen to rel if I don't have a valid"
		    " pixelCanvas!"));
  }

  Vector<Float> rel(2);

  if (screen.nelements() != 2) {
    throw(AipsError("Can't to screen to rel - I was expecting a point ("
		    "nelements == 2)"));
    
  }

  rel(0) = screen(0) / pc->width();
  rel(1) = screen(1) / pc->height();
  
  if ((     isNaN(screen(0)) || isInf(screen(0)) 
	    || isNaN(screen(1)) || isInf(screen(1)) )) {
    
    rel(0) = 0; rel(1) = 0;
  }
  
  return rel;
}

WorldCanvas* DisplayShapeWithCoords::chooseWCFromPixPoints(const 
							   Matrix<Float> 
							   points,
							   PanelDisplay* pd) {
  WorldCanvas* toReturn(0);
  Bool success = False;
  Bool pointsuccess = True;

  ListIter<WorldCanvas* >* wcs = pd->wcs();
  wcs->toStart();
  
  while(!wcs->atEnd() && !success) {
    // Test all points.
    const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());
    if (!test) {
      pointsuccess = False;
    } 
    
    for (uInt i=0 ; i < points.nrow() ; i ++) {
      
      pointsuccess = 
	pointsuccess && 
	(wcs->getRight()->inWC(Int(points(i,0)+0.5), Int(points(i,1)+0.5)) &&
	 wcs->getRight()->inDrawArea(Int(points(i,0)+0.5), 
				     Int(points(i,1)+0.5)));
      
    }
    
    if (pointsuccess) {
      toReturn = wcs->getRight();
      success = True;
    } else {
      wcs->step();
    }
  }
  
  
  // if that returns nothing, look for one just in WC..
  if (!success) {
    wcs->toStart();
    pointsuccess = True;
    
    while(!wcs->atEnd() && !success) {
      const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());
      if (!test) {
	pointsuccess = False;
      }
      
      for (uInt i=0 ; i < points.nrow() ; i ++) {
	pointsuccess = 
	  pointsuccess && 
	  (wcs->getRight()->inWC(Int(points(i,0)+0.5), Int(points(i,1)+0.5)));
      }
      
      
      if (pointsuccess) {
	toReturn = wcs->getRight();
	success = True;
      } else {
	wcs->step();
      }
      
    }
  }
  
  return toReturn;
  
}

WorldCanvas* DisplayShapeWithCoords::chooseWCFromPixPoint(const Float& xPos, 
							  const Float& yPos,
							  PanelDisplay* pd) {
  WorldCanvas* toReturn(0);
  Bool success = False;
  
  // Look for ones where the point is in WC and in draw area
  ListIter<WorldCanvas* >* wcs = pd->wcs();
  
  
  wcs->toStart();
  
  while(!wcs->atEnd() &&  !success) {
    
    if (wcs->getRight()->inWC(Int(xPos+0.5), Int(yPos+0.5)) &&
	wcs->getRight()->inDrawArea(Int(xPos+0.5), Int(yPos+0.5))) {
      
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
      if (wcs->getRight()->inWC(Int(xPos+0.5), Int(yPos+0.5))) {
	
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

WorldCanvas* DisplayShapeWithCoords::chooseWCFromWorldPoints(const Record&
							     settings,
							     const String&
							     field,
							     PanelDisplay* pd)
{
  WorldCanvas* toReturn(0);
  
  if (!settings.isDefined(field)) {
    throw(AipsError("Field (" + field +") not found when searching for"
		    " a WorldCanvas"));
  }

  if (!pd) {
    throw(AipsError("Bad paneldisplay supplied when searching for a "
		    "WorldCanvas"));
    
  }

  if (settings.dataType(field) != TpRecord) {
    throw(AipsError("Bad data type for field \'" + field + "\'. I was "
		    "expecting a record"));
    
  }

  // TODO units

  // Record
  //   points
  //       quantas

  Record subRecord;
  subRecord = settings.subRecord(field);

  Matrix<Float> worldPoints(subRecord.nfields(), 2);
  
  for (uInt i=0; i<subRecord.nfields(); i++) {
    Record point = subRecord.subRecord(i);
    if (point.nfields() != 2) {
      throw(AipsError("Badly formed point data"));
    }
    Record x = point.subRecord(0);
    Record y = point.subRecord(1);

    QuantumHolder xh, yh;
    String error;

    xh.fromRecord(error,y);
    if (error.length() != 0) {
      throw(AipsError("When looking for a WC, error extracting x value"));
    }
    yh.fromRecord(error,y);
    if (error.length() != 0) {
      throw(AipsError("When looking for a WC, error extracting y value"));
    }
    
    worldPoints(i,0) = xh.asQuantumFloat().getValue();
    worldPoints(i,1) = yh.asQuantumFloat().getValue();
      
  }
  
  Bool success = False;
  
  Vector<Double> pix(2);
  Vector<Double> wcent(2);
  Bool currentOK = True;
  ListIter<WorldCanvas* >* wcs = pd->wcs();
  wcs->toStart();
  
  while (!wcs->atEnd() && !success) {
    const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());
    if (!test) {
      currentOK = False;
    }
    
    // Check all points for current WC.
    for (uInt i =0; i < worldPoints.nrow() ; i++) {

      wcent(0) = worldPoints(i,0); wcent(1) = worldPoints(i,1);
      
      if (!wcs->getRight()->worldToPix(wcent, pix)) {
	currentOK = False;
      }
      
      if (currentOK) {
	currentOK = currentOK && 
	  wcs->getRight()->inWC(Int(pix(0)+0.5), Int(pix(1)+0.5)) &&
	  wcs->getRight()->inDrawArea(Int(pix(0)+0.5), Int(pix(1)+0.5));
      }
    }
    
    // Are all those points OK?
    if (currentOK) {
      toReturn = wcs->getRight();
      success = True;
    } else {
      wcs->step();
    }
  }
  
  // Check for just on WC (not in draw area)
  if (!success) {
    currentOK = True;
    wcs->toStart();
  
    while(!wcs->atEnd() && !success) {
      const CoordinateSystem* test = &(wcs->getRight()->coordinateSystem());
      if (!test) {
	currentOK = False;
      }
      
      // Check all points for current WC.
      for (uInt i =0; i < worldPoints.nrow() ; i++) {
	wcent(0) = worldPoints(i,0); wcent(1) = worldPoints(i,1);
	if (!wcs->getRight()->worldToPix(wcent, pix)) {
	  currentOK = False;
	}
	if (currentOK) {
	  currentOK = currentOK && 
	    wcs->getRight()->inWC(Int(pix(0)+0.5), Int(pix(1)+0.5));
	}
      }
      
      // Are all those points OK?
      if (currentOK) {
	toReturn = wcs->getRight();
	success = True;
      } else {
	wcs->step();
      }
    }
  }
  
  return toReturn;  
}


WorldCanvas* DisplayShapeWithCoords::chooseWCFromWorldPoint(const Record& 
							    settings, 
							    PanelDisplay* pd) {
  
  Bool success = False;

  WorldCanvas* toReturn(0);
  
  if (!settings.isDefined("center")) {
    throw (AipsError("When trying to choose a WC, the field \'center\' was"
		     " not defined.")); 
  }

  

  if (settings.dataType("center") != TpRecord) {
    throw (AipsError("When trying to choose a WC, the field \'center\' was"
		     " of incorrect type.")); 
    
  }
  Record subRecord = settings.subRecord("center");
  if (subRecord.nfields() != 2) {
    throw (AipsError("When trying to choose a WC, the field \'center\' had"
		     " the wrong number of fields.")); 
  }

  Record x = subRecord.subRecord(0);
  Record y = subRecord.subRecord(1);

  QuantumHolder xh,yh;
  String error;

  xh.fromRecord(error, x);
  if (error.length() != 0) {
    throw(AipsError("I couldn't get data from field \'center\' because"
		    " the first element returned an error when I tried to "
		    "make a quantum out of it"));
  }
  
  yh.fromRecord(error, y);
  if (error.length() != 0) {
    throw(AipsError("I couldn't get data from field \'center\' because"
		    " the second element returned an error when I tried to "
		    "make a quantum out of it"));
  }
  Vector<Double> wcent(2);
  wcent(0) = Double(xh.asQuantumFloat().getValue());
  wcent(1) = Double(yh.asQuantumFloat().getValue());

  String unitx = xh.asQuantumFloat().getFullUnit().getName();
  String unity = yh.asQuantumFloat().getFullUnit().getName();

  Vector<Double> pix(2);

  ListIter<WorldCanvas* >* wcs = pd->wcs();
  wcs->toStart();

  while (!wcs->atEnd() && !success) {
    // UNITS??!?!?
    if (wcs->getRight()->worldToPix(wcent, pix)) {
      if (wcs->getRight()->inWC(Int(pix(0)+0.5), Int(pix(1)+0.5)) &&
	  (wcs->getRight()->inDrawArea(Int(pix(0)+0.5), Int(pix(1)+0.5)))) {

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
    } else {
      wcs->step();
    }
  }
  
  if (!success) {
    wcs->toStart();
    while(!wcs->atEnd() && !success) {
      if (wcs->getRight()->worldToPix(wcent, pix)) {
	if (wcs->getRight()->inWC(Int(pix(0)+0.5), Int(pix(1)+0.5))) {
	  const CoordinateSystem* test = 
	    &(wcs->getRight()->coordinateSystem());
	  
	  if (test) {
	    toReturn = wcs->getRight();
	    success = True;
	  } else {
	    wcs->step();
	  }
	} else {
	  wcs->step();
	}
      } else {
	wcs->step();
      }
    }
  }

  return toReturn;
}

Matrix<Float> DisplayShapeWithCoords::relToScreen(const Matrix<Float>& rel, 
						  PixelCanvas* pc) {
  

  Matrix<Float> toReturn(rel.nrow(), rel.ncolumn());

  for (uInt i=0 ; i < rel.nrow() ; i++) {
    Vector<Float> relPoint = rel.row(i);

    Vector<Float> screenPoint = relToScreen(relPoint, pc);

    toReturn(i,0) = screenPoint(0);
    toReturn(i,1) = screenPoint(1);
  }

  return toReturn;
 
}

Matrix<Float> DisplayShapeWithCoords::screenToRel(const Matrix<Float>& screen, 
						  PixelCanvas* pc) {
  
  Matrix<Float> toReturn(screen.nrow(), screen.ncolumn());

  for (uInt i=0 ; i < screen.nrow() ; i++) {
    Vector<Float> screenPoint = screen.row(i);
    Vector<Float> relPoint = screenToRel(screenPoint, pc);
    
    toReturn(i,0) = relPoint(0);
    toReturn(i,1) = relPoint(1); 
    
  }
  
  return toReturn;
}

Bool DisplayShapeWithCoords::inWorldCanvasDrawArea(const Matrix<Float> points,
						   WorldCanvas* wc) {

  if (!wc) {
    throw(AipsError("Invalid worldcanvas passed "
		    "to inWorldCanvasDrawArea(Matrix)"));
  }
  Bool success = True;
  for (uInt i = 0 ; i < points.nrow() ; i++) {
    success = success && wc->inDrawArea(Int(points(i,0)+0.5), 
					Int(points(i,1)+0.5));
  }

  return success;
}

void DisplayShapeWithCoords::pixelToWorldPoints(Record& inHere, 
						const String& field,
						WorldCanvas* wc) {
  if (!wc) {
    throw(AipsError("Bad worldcanvas supplied to function "
		    "\'pixelToWorldPoints\'"));
  }

  if (!inHere.isDefined(field)) {
    throw(AipsError("Field requested to be converted (" + field +
		    ") not found"));
  }
  
  if (inHere.dataType(field) != TpArrayFloat) {
    throw(AipsError("Bad data type supplied in record to be converted to"
		    " world coords"));

  }

  Matrix<Float> pixels = inHere.asArrayFloat(field);
  Record subRecord;
  
  Vector<String> units = wc->coordinateSystem().worldAxisUnits();
  String unit = units(0);
  for (uInt i=0; i < units.nelements() ; i ++) {
    if (units(i) != unit) {
      throw(AipsError("Not *yet* able to handle what happened in"
		      " DisplayShapeWithCoords line 902"));
    }
  }
  
  for (uInt i=0 ; i < pixels.nrow() ; i++) {
    Vector<Double> world(2);
    Vector<Double> pix(2); 
    pix(0) = Double(pixels(i,0)); 
    pix(1) = Double(pixels(i,1)); 

    if (!wc->pixToWorld(world, pix)) {
      throw(AipsError("Couldn't covert point to world"));
    }
    
    Quantity x(world(0), unit);
    Quantity y(world(1), unit);

    QuantumHolder xh(x);
    QuantumHolder yh(y);

    Record xrec, yrec;
    String error;
    xh.toRecord(error, xrec);
    if (error.length() != 0) {
      throw(AipsError("I couldn't quantify \'" + field +"\' (first element) "
		    "because when I tried"
		    " to write to a record, I received the error: " +error));
    }
    yh.toRecord(error, yrec);
    if (error.length() != 0) {
      throw(AipsError("I couldn't quantify \'" + field +"\' (second element) "
		      "because when I tried"
		      " to write to a record, I received the error: " +error));
    }
    
    Record point;
    point.defineRecord("x", xrec);
    point.defineRecord("y", yrec);
    
    subRecord.defineRecord(String("point " + String(i)) , point);
  }

  inHere.removeField(field);
  inHere.defineRecord(field, subRecord);
}





} //# NAMESPACE CASA - END

