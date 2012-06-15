//# DisplayShape.cc: Abstract base class for all shapes/annotations objects
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
#include <casa/Exceptions.h>
#include <casa/iostream.h>

#include <display/DisplayShapes/DisplayShape.h>
#include <display/DisplayShapes/DSClosed.h>
#include <display/DisplayShapes/DSBasic.h> 
#include <display/DisplayShapes/DSRectangle.h>
#include <display/DisplayShapes/DSCircle.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/DParameterColorChoice.h>

#include <scimath/Mathematics.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>


namespace casa { //# NAMESPACE CASA - BEGIN

DisplayShape::DisplayShape() {
  itsColor = 
    new DParameterColorChoice("color", "Object Color", 
			      "Select the drawing color of the shape");
  setDefaultOptions();
}

DisplayShape::DisplayShape(const DisplayShape& other) :
  itsColor(other.itsColor),
  itsHandles(other.itsHandles),
  itsLocks(other.itsLocks),
  itsHasHandles(other.itsHasHandles),
  itsDrawHandles(other.itsDrawHandles),
  itsValidHandles(other.itsValidHandles),
  itsHandleColor(other.itsHandleColor),
  itsHandleShape(other.itsHandleShape),
  itsHandleSize(other.itsHandleSize) {

}

DisplayShape::~DisplayShape() {
  delete itsColor; itsColor = 0;

  if (itsHasHandles) {
    for (uInt i=0; i<itsHandles.nelements(); i++) {
      delete itsHandles[i];
      itsHandles[i] = 0;
    }
  }

}

void DisplayShape::setDefaultOptions() {  
  itsHasHandles = True;
  itsDrawHandles = False;
  itsValidHandles = False;
  itsHandleColor = "Green";
  itsHandleSize = 8;
  itsHandleShape = DisplayShape::Filled_Circle;

}

String DisplayShape::getColor() {
  return itsColor->value();
}

Float DisplayShape::toRadians(const Float& degrees) {
  return ((C::pi / 180) * degrees );
}

Float DisplayShape::toDegrees(const Float& radians) {
  return ((180 / C::pi) * radians );
}

void DisplayShape::setColor(const String& color) {
  itsColor->setValue(color);
}

Vector<Float> DisplayShape::rotatePoint(const Vector<Float>& toRotate, 
					const Float& angle,
					const Float& aboutX, 
					const Float& aboutY) {

  Float movedX = toRotate[0] - aboutX;
  Float movedY = toRotate[1] - aboutY;
  Vector<Float> toReturn(2);

  toReturn[0] = (movedX * cos(angle)) - (movedY * sin(angle));
  toReturn[1] = (movedX * sin(angle)) + (movedY * cos(angle));
  
  toReturn[0] += aboutX;
  toReturn[1] += aboutY;

  return toReturn;
}

Matrix<Float> DisplayShape::rotatePolygon(const Matrix<Float>& toRotate, 
					  const Float& angle,
					  const Float& aboutX, 
					  const Float& aboutY) {
  
  // Rotates the matrix around specified location
  Float cosa = cos(angle);
  Float sina = sin(angle);
  Matrix<Float> rotationMatrix(2,2);
  Matrix<Float> rotated(toRotate.nrow(), toRotate.ncolumn());
  rotated = translateMatrix(toRotate, -aboutX, -aboutY);
  
  // Calculate the rotated matrix
  rotationMatrix(0,0) = cosa;
  rotationMatrix(1,0) = -sina;
  rotationMatrix(0,1) = sina;
  rotationMatrix(1,1) =  cosa;

  rotated = (product(rotated, rotationMatrix));

  // Make into an array of integers and move back.
  rotated = translateMatrix(rotated, aboutX, aboutY);

  return rotated;
}

Matrix<Float> DisplayShape::translateMatrix(const Matrix<Float>& points,
			     const Float& dx, const Float& dy)
{
  Matrix<Float> translated(points.nrow(), points.ncolumn());

  for (uInt k=0;k<points.nrow();k++) {
    for (uInt l=0;l<points.ncolumn();l++) {
      if (l==0) translated(k,l) = points(k,l) + dx;
      else if (l==1) translated(k,l) = points(k,l) + dy;
    }
  }

  return translated;
}

uInt DisplayShape::nHandles() {
  return itsHandles.nelements();
}

Bool DisplayShape::inPolygon(const Matrix<Float>& points, const Float& xPos, 
			const Float& yPos) {

  const Vector<Float>& xP = points.column(0);
  const Vector<Float>& yP = points.column(1);

  AlwaysAssert(xP.nelements() == yP.nelements(), AipsError);
  Bool isInside = False;
  uInt n = xP.nelements();
  for (uInt i=0,j=n-1; i < n; j=i++) {
    if ((((yP(i)<=yPos) && (yPos<yP(j))) ||
         ((yP(j)<=yPos) && (yPos<yP(i)))) &&
        (xPos < (xP(j) - xP(i)) * (yPos - yP(i)) / (yP(j) - yP(i)) + xP(i))) {
      isInside = !isInside;
    }
  }
  return isInside;
}

void DisplayShape::rotateAbout(const Float& angle, const Float& aboutX, 
			       const Float& aboutY) {
  
  // Nothing yet. If it is ever decided that handles should be 
  // rotated with object, this should happen here.
}

void DisplayShape::draw(PixelCanvas* pc) {
  // Redraw all my little handles
  if (itsHasHandles && itsValidHandles) {
    pc->setColor(itsHandleColor);
    if (itsHandles.nelements() && itsDrawHandles) {
      for (uInt i=0; i<itsHandles.nelements(); i++) {
	
	itsHandles[i]->draw(pc);

      }
    }
  } else if (itsHasHandles && itsDrawHandles && !itsValidHandles) {

  }
}

void DisplayShape::move(const Float& dX, const Float& dY) {

  // Move all my little handles
  if (itsHasHandles && itsValidHandles) {
    if(itsHandles.nelements()) {
      for(uInt i=0; i<itsHandles.nelements(); i++) {
	itsHandles[i]->move(dX, dY);
      }
    }
  }

  // Move any locked shapes
  for (uInt i =0; i<itsLocks.nelements() ; i++) {
    itsLocks[i]->move(dX, dY);
  }
}

void DisplayShape::setHandlePositions(const Matrix<Float>& newPoints) {
  if (itsHasHandles) {
    if(itsValidHandles) {
      if (itsHandles.nelements() == newPoints.nrow()) {
	for (uInt i=0; i<itsHandles.nelements(); i++) {
	  itsHandles[i]->setCenter((newPoints(i,0)),
				   (newPoints(i,1)));
	}
       } else throw(AipsError("DisplayShape.cc - Problem while trying to "
			      "set the position of a shape's handles - "
			      "Incorrect number of points ( != number of "
			      "handles )" )) ;  
    } else {
      buildHandles(newPoints);
    }
  }
}


Bool DisplayShape::onHandles(const Float& xPos, const Float& yPos) {
  Bool onHandle = False;
  
  if (itsHasHandles && itsValidHandles) {
    if (itsHandles.nelements()) {
      for(uInt i=0; i<itsHandles.nelements() ; i++) {
	onHandle = onHandle || itsHandles[i]->inObject(xPos, yPos);
	if (onHandle) break;
      }
    }
  }
  return onHandle;
}


Bool DisplayShape::closestLine(const Matrix<Float>& points, 
			       const Float& xPos, const Float& yPos,
			       Int& firstVert, Int& secondVert, 
			       const Bool& closedPoly){

  
  Int lowesti(-1);
  Float lowestDistance(-1);
  Float u, x, y;
  uInt n(points.nrow());

  // Loop through all points. Calculate x and y in each case 
  // (x and y are the points a tangent from the
  // line to point intersects with the line.

  for(uInt i=0; i< (n - 1); i++) {
    
    u = (((xPos-points(i,0))*(points(i+1,0)-points(i,0))) + ((yPos-points(i,1))*(points(i+1,1)-points(i,1)))) / (( square( points(i+1,0) - points(i,0) ) ) + ( square(points(i+1,1)-points(i,1)) ));
    x = points(i,0) + u * ( points(i+1,0) - points(i,0) );
    y = points(i,1) + u * ( points(i+1,1) - points(i,1) );
    
    // Now check that the tangent hits the line in between the two points
    Float P2P(hypot(points(i,0)-points(i+1,0),  points(i,1)- points(i+1,1)));
    if ((P2P > hypot(points(i,0)-y, points(i,1)-y)) 
	&& (P2P > hypot(points(i+1,0)-x, points(i+1,1)-y)))
      { 
	
	// It does, so determine whether this is the closest we've got to a line yet.
	Float dist(hypot(xPos-x, yPos-y));

	// First time through?
	if (lowesti == -1) {
	  lowesti = i;
	  lowestDistance = dist;
	} else {
	  if (dist < lowestDistance) {
	    lowesti = i;
	    lowestDistance = dist;
	  }
	}

      }
  }
  
  //Check the last (closed poly) one;
  if (closedPoly) {
    Float u, x, y;

    u = (((xPos-points(n,0))*(points(0,0)-points(n,0))) + ((yPos-points(n,1))*(points(0,1)-points(n,1)))) / (( square( points(0,0) - points(n,0) ) ) + ( square(points(0,1)-points(n,1)) ));    
    x = points(n,0) + u * ( points(0,0) - points(n,0) );
    y = points(n,1) + u * ( points(0,1) - points(n,1) );
    Float dist(hypot(xPos - x,yPos - y));

    // Now check that the tangent hits the line in between the two points
    Float P2P(hypot(points(n,0)- points(0,0) , points(n,1)- points(0,1)));
    if ((P2P > hypot(points(n,0)-x, points(n,1)-y)) 
	&& (P2P > hypot(points(0,0)-x, points(0,1)- y)))
      { 
	if (lowesti == -1) {
	  lowesti = n;
	  lowestDistance = dist;
	} else {
	  if (dist < lowestDistance) {
	    lowesti = n;
	    lowestDistance = dist;
	  }
	}
      }
  }

  if (!closedPoly) {
    firstVert = lowesti;
    secondVert = lowesti + 1;
  } else if (lowesti == Int(n)) {
    firstVert = n;
    secondVert = 0;
  } else {
    firstVert = lowesti;
    secondVert = lowesti + 1;
  }

  return (lowesti != -1); // If -1, didn't find any.
}

Bool DisplayShape::closestPoints(const Matrix<Float>& points, 
				      const Float& xPos, const Float& yPos, 
				      Int& outClosest, Int& outSecond) {
  closestPoint(points, xPos, yPos, outClosest);
  
  //Now find the second closest.
  outSecond = -1;
  Float distance = -1;

  for (uInt i=0; i<points.nrow();i++) {
    if (i != uInt(outClosest)) {
      Float currentDistance = hypot(xPos - points(i,0), yPos - points(i,1));
      
      if (outSecond == -1) {
	distance = currentDistance;
	outSecond = i;
      } else {
	if (currentDistance < distance) {
	  distance = currentDistance;
	  outSecond = i;
	}
      }
   
    }
  }
  return True;

  
}

Bool DisplayShape::closestPoint(const Matrix<Float>& points, 
				      const Float& xPos, const Float& yPos, 
				      Int& out) {
  out = -1;
  Float distance = -1;

  for (uInt i=0; i<points.nrow();i++) {

    Float currentDistance = hypot(xPos - points(i,0), yPos - points(i,1));

    if (out == -1) {
      distance = currentDistance;
      out = i;
    } else {
      if (currentDistance < distance) {
	distance = currentDistance;
	out = i;
      }
    }
  }
  return True;
}
Bool DisplayShape::setOptions(const Record& settings) {
  Bool localChange = False;
  Bool handleRebuild = False;

  //Pick out all the settings I'm interested in.
  if (itsColor->fromRecord(settings)) localChange = True;
  
  if (settings.isDefined("hashandles")) 
    itsHasHandles = settings.asBool("hashandles");  
  
  if (itsHasHandles) {
  
    if (settings.isDefined("drawhandles")) 
      itsDrawHandles = settings.asBool("drawhandles");

    if (settings.isDefined("handlesize")) {
      Int newSize = settings.asInt("handlesize");
      if (itsHandleSize != newSize) {
	itsHandleSize = newSize;
	handleRebuild = True;
      }
    }

    if (settings.isDefined("handlecolor")) 
      itsHandleColor = settings.asString("handlecolor");
   
    if (settings.isDefined("handlestyle")) {
      Int temp(settings.asInt("handlestyle"));
      DisplayShape::HandleShape cast;
      cast =  static_cast<DisplayShape::HandleShape>(temp);

      if (cast != itsHandleShape) {
	itsHandleShape = cast;
	handleRebuild = True;
      }
      
    }
  }

  return localChange;
}

Record DisplayShape::getOptions() {
  
  Record rec;
  rec.define("type", "displayshape");
  rec.define("hashandles", itsHasHandles);
  
  if (itsHasHandles) rec.define("drawhandles", itsDrawHandles);
  if (itsHasHandles) rec.define("handlesize", itsHandleSize);
  if (itsHasHandles) rec.define("handlecolor", itsHandleColor);
  if (itsHasHandles) rec.define("handlestyle", itsHandleShape);
  
  itsColor->toRecord(rec);

  return rec;
}

Matrix<Float> DisplayShape::getHandleLocations() {
  Matrix<Float> toReturn;
  if (itsHasHandles && itsValidHandles) {
    toReturn.resize(itsHandles.nelements(), 2);

    for (uInt i=0; i<itsHandles.nelements(); i++) {
      Vector<Float> temp(itsHandles[i]->getCenter());
      toReturn(i,0) = temp[0];
      toReturn(i,1) = temp[1];
    }
  }

  return toReturn;

}

Bool DisplayShape::whichHandle(const Float& xPos, const Float& yPos, 
			       Int& out) {
  Bool found = False;
  Int i = -1;
  
  if (itsHasHandles && itsValidHandles) {
    if(itsHandles.nelements()) {
      while(i<Int(itsHandles.nelements()-1) && !found ) { 
        i++;
	found = found || (itsHandles[i]->inObject(xPos, yPos));
      }  
      if (found) out = i;
      return found;
    }
  }

  return False;
}

void DisplayShape::setHasHandles(const Bool& hasHandles) {
  itsHasHandles = hasHandles;

  if (!hasHandles) itsDrawHandles = False;
}

void DisplayShape::setDrawHandles(const Bool& shouldIDraw) {
  if (itsHasHandles) {
    itsDrawHandles = shouldIDraw;
  }
}

DSClosed* DisplayShape::makeHandle(const Vector<Float>& newHandlePos) {
  DSClosed* toAdd(0);

  if (itsHasHandles) {
    //Create new Handle
    if (itsHandleShape == DisplayShape::Open_Square) {
      toAdd = new DSRectangle(newHandlePos[0], newHandlePos[1], itsHandleSize, 
			      itsHandleSize, False, False);
      toAdd->setColor(itsHandleColor);
    } else if (itsHandleShape == DisplayShape::Filled_Square) {
      toAdd = new DSRectangle(newHandlePos[0], newHandlePos[1], itsHandleSize, 
			      itsHandleSize, False, False);
      toAdd->setColor(itsHandleColor);
      toAdd->setFillStyle(DSClosed::Full_Fill);
    } else if (itsHandleShape == DisplayShape::Filled_Circle) {
      toAdd = new DSCircle(newHandlePos[0], newHandlePos[1], itsHandleSize, 
			   False, False);
      toAdd->setColor(itsHandleColor);
      toAdd->setFillStyle(DSClosed::Full_Fill);
    } else {
      toAdd = new DSCircle(newHandlePos[0], newHandlePos[1], itsHandleSize, 
			   False, False);
      toAdd->setColor(itsHandleColor);
      toAdd->setFillStyle(DSClosed::Full_Fill);
    }
  }
  return toAdd;
}

//This needs some updating once other shapes are ready
// **Not yet tested for block
void DisplayShape::addHandle(const Vector<Float>& newHandlePos, 
				   const Bool& atEnd,
				   const Int position) {
  
  if (itsHasHandles) {
    //
    DSClosed* toAdd(makeHandle(newHandlePos));
    
    if (itsValidHandles) {
      PtrBlock<DSClosed *> newHandles(itsHandles.nelements() + 1);
      if (atEnd) {
	for (uInt i=0; i< itsHandles.nelements() ; i++) {
	  newHandles[i] = itsHandles[i];
	}
	
	newHandles[itsHandles.nelements()] = toAdd;
	itsHandles.resize(itsHandles.nelements() + 1);
	itsHandles = newHandles;
	
      } else {
	for (Int i=0; i<position ; i++) {    
	  newHandles[i] = itsHandles[i];
	}
	newHandles[position] = toAdd;
	for (Int i=0; i<Int(itsHandles.nelements()-position); i++) {
	  newHandles[position + 1 + i] = itsHandles[position + i];
	}
	itsHandles.resize(itsHandles.nelements() + 1);
	itsHandles = newHandles;
      }
    } else {
      itsHandles.resize(1);
      itsHandles[0] = toAdd;
      itsValidHandles = True;
    }
  }
}

Bool DisplayShape::removeHandle(const Vector<Float>& getRidOf) {

  return False;
}

Bool DisplayShape::removeHandle(const Int nHandle) {
  if (nHandle < 0 || nHandle >= Int(itsHandles.nelements())) 
    return False;
  
  itsHandles.remove(uInt(nHandle), True);
  return True;
}

void DisplayShape::setHandleShape(const DisplayShape::HandleShape& shape) {
  if (itsHasHandles) itsHandleShape = shape;
}

void DisplayShape::setHandleSize(const Int pixelSize) {
  if (itsHasHandles) itsHandleSize = pixelSize;
}

void DisplayShape::setHandleColor(const String& handleColor) {
  if (itsHasHandles) itsHandleColor = handleColor;
}

void DisplayShape::addLocked(DisplayShape* toLock) {
  itsLocks.resize(itsLocks.nelements() + 1, False, True);
  itsLocks[itsLocks.nelements() - 1] = toLock;
}

void DisplayShape::removeLocked(DisplayShape* removeLock) {
  for (uInt i=0; i<itsLocks.nelements(); i++) {
    if (itsLocks[i] == removeLock) itsLocks.remove(i, True);
  }
}

void DisplayShape::buildHandles(const Matrix<Float>& startPoints) {
  if (itsHasHandles) {
    
    for (uInt i=0; i<itsHandles.nelements(); i++) {
      delete itsHandles[i];
      itsHandles[i] = 0;
    }

    itsHandles.resize(startPoints.nrow());
    itsValidHandles = True;
    
    Vector<Float> temp(2);
    for (uInt i =0 ; i< startPoints.nrow(); i++) {
      temp[0] = startPoints(i,0);
      temp[1] = startPoints(i,1);
      itsHandles[i] = (makeHandle(temp));
    }
  }
}









} //# NAMESPACE CASA - END

