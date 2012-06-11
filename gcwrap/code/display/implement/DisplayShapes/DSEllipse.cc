//# DSEllipse.cc: Ellipse implementation for "DisplayShapes"
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

#include <display/Display/PixelCanvas.h>
#include <display/DisplayShapes/DSEllipse.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSEllipse::DSEllipse(const Bool& onlyShowOneHandle) :
  DSClosed() {
  itsOneHandle = onlyShowOneHandle;
  setDefaultOptions();
}

DSEllipse::DSEllipse(const DSEllipse& other) :
  DSClosed(other),
  itsCenter(other.itsCenter),
  itsAngle(other.itsAngle),
  itsMajorAxis(other.itsMajorAxis),
  itsMinorAxis(other.itsMinorAxis),
  itsValid(other.itsValid),
  itsOneHandle(other.itsOneHandle),
  itsHandleLocation(other.itsHandleLocation) {
  
}

DSEllipse::DSEllipse(const Float& xPos, const Float& yPos, const Float& major, 
		     const Float& minor, const Bool& hasHandles, 
		     const Bool& drawHandles, const Bool& onlyOne) :
  DSClosed() {
  
  itsCenter.resize(2);
  itsCenter[0] = xPos;
  itsCenter[1] = yPos;
  itsMajorAxis = major;
  itsMinorAxis = minor;
  itsAngle = 0;
  itsOneHandle = onlyOne;
  setHasHandles(hasHandles);
  setDrawHandles(drawHandles);
  if (hasHandles) {
    calculateHandlePositions();
    buildHandles(itsHandleLocation);
  }
  itsValid = True;
}

DSEllipse::~DSEllipse() {

}

void DSEllipse::draw(PixelCanvas* pix) {

  if (itsValid) {
    
    pix->setLineWidth(getLineWidth());
    pix->setColor(getColor());
    
    if (getFillStyle() == DSClosed::No_Fill) { 
      pix->drawEllipse(itsCenter[0], itsCenter[1], Float(itsMajorAxis/2), 
		       Float(itsMinorAxis/2), itsAngle, True);
    } else if (getFillStyle() == DSClosed::Full_Fill) {
      pix->drawEllipse(itsCenter[0], itsCenter[1], Float(itsMajorAxis/2), 
		       Float(itsMinorAxis/2),itsAngle, False);
    } else {
      pix->drawEllipse(itsCenter[0], itsCenter[1], Float(itsMajorAxis/2), 
		       Float(itsMinorAxis/2), itsAngle, True);
      //cerr << "Fill style NYI sorry" << endl;    
    } 
    DSClosed::draw(pix);

  }

}

void DSEllipse::scale(const Float& scaleFactor) {
  if ((scaleFactor * getMinorAxis()) > 3)
    setMinorAxis(scaleFactor * getMinorAxis());
  if ((scaleFactor * getMajorAxis()) > 3)
    setMajorAxis(scaleFactor * getMajorAxis());
  if (itsValid) {  
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }
}

void DSEllipse::calculateHandlePositions() {
  if (!itsOneHandle) itsHandleLocation.resize(2,2);
  else itsHandleLocation.resize(1,2);

  itsHandleLocation(0,0) = 
    itsCenter[0] + (cos(itsAngle * (C::pi / 180)) * (itsMajorAxis/2));
  itsHandleLocation(0,1) = 
    itsCenter[1] + (sin(itsAngle * (C::pi / 180)) * (itsMajorAxis/2));    

  if (!itsOneHandle) {
    itsHandleLocation(1,0) = itsCenter[0] + 
      (cos((itsAngle* (C::pi / 180)) + (C::pi / 2)) * (itsMinorAxis/2));   
    itsHandleLocation(1,1) = itsCenter[1] + 
      (sin((itsAngle * (C::pi / 180) )+ (C::pi / 2)) * (itsMinorAxis/2));   
  }
}

void DSEllipse::move(const Float& dX, const Float& dY) {
  if (itsValid) {
    itsCenter[0] += dX;
    itsCenter[1] += dY;
    
    DSClosed::move(dX, dY);
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }
}

void DSEllipse::changePoint(const Vector<Float>& newPoint) {
  // This will really move the handle.
  if (itsValid) {
    Int whichOne(-1);
    closestPoint(itsHandleLocation, newPoint[0], newPoint[1], whichOne);
    changePoint(newPoint, whichOne);
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }
}

void DSEllipse::changePoint(const Vector<Float>& newPoint, const Int whichOne) {
  if (itsValid) {
    Vector<Float> cent(getCenter());
    
    if (whichOne == 0) {
      setMajorAxis(hypot(newPoint[0] - cent[0], newPoint[1]- cent[1]));
    } else if (whichOne == 1) {
      setMinorAxis(hypot(newPoint[0] - cent[0], newPoint[1]-cent[1]));
    } else {
      //cerr << "DSEllipse.cc - Debug - Oops. That shouldn't have happened." 
      //<< endl; 
      throw (AipsError("DSEllipse.cc - impossible changePoint called"));
    }
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
    
  }
}


Bool DSEllipse::inObject(const Float& xPos, const Float& yPos) {

  if (itsValid) {
    //Bool inside = False;
    
    //Not ideal, but ok:
    Vector <Float> cent(getCenter());
    
    Matrix<Float> points(4,2);
    
    points(0, 0) = cent[0] + (cos(itsAngle * (C::pi / 180)) * 
			      (itsMajorAxis/2));
    
    points(0,1) = cent[1] - (cos((itsAngle * (C::pi / 180)) + 
				 (C::pi / 2)) * (itsMajorAxis/2)); 
    
    points(1,0) = cent[0] - sin(itsAngle * (C::pi / 180)) * (itsMinorAxis/2);
    points(1,1) = cent[1] + cos(itsAngle * (C::pi / 180)) * (itsMinorAxis/2);
    
    points(2,0) = cent[0] - (cos(itsAngle * (C::pi / 180)) * 
			     (itsMajorAxis/2));
    
    points(2,1) = cent[1] + (cos((itsAngle * (C::pi / 180)) + 
				 (C::pi / 2)) * (itsMajorAxis/2)); 
    
    points(3,0) = cent[0] + sin(itsAngle * (C::pi / 180)) * (itsMinorAxis/2);
    points(3,1) = cent[1] - cos(itsAngle * (C::pi / 180)) * (itsMinorAxis/2);
    
    return inPolygon(points, xPos, yPos) ; // inside;
  }
  return False;
}

Bool DSEllipse::setOptions(const Record& settings) {
  Bool localChange = False;
  if (settings.isDefined("angle")) {
    itsAngle = settings.asFloat("angle");
    localChange = True;
  }
  if (settings.isDefined("majoraxis")) {
    itsMajorAxis = settings.asFloat("majoraxis");
    localChange = True;
  }
  if (settings.isDefined("minoraxis")) {
    itsMinorAxis = settings.asFloat("minoraxis");
    localChange = True;
  }

  if (settings.isDefined("center")) {
    Vector<Float> cent(settings.asArrayFloat("center"));
    setCenter(cent[0], cent[1]);
    localChange = True;
  }

  if (DSClosed::setOptions(settings)) localChange = True;

  if (itsHandleLocation.nelements() == 0 && 
      DisplayShape::getOptions().asBool("hashandles")) {

    calculateHandlePositions();
    buildHandles(itsHandleLocation);
  } else if (localChange) {
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }
  return localChange;
}

Record DSEllipse::getOptions() {
  Record rec(DSClosed::getOptions());
  
  rec.define("type", "ellipse");
  if (itsValid) rec.define("center", getCenter());
  rec.define("angle" , itsAngle);
  rec.define("majoraxis", itsMajorAxis);
  rec.define("minoraxis", itsMinorAxis);

  return rec;
  
}

void DSEllipse::rotate(const Float& angle) {

  itsAngle += angle;
  
  if (itsValid) {
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
    DSClosed::rotateAbout(angle, (getCenter())[0], (getCenter())[1]);
  }

}

void DSEllipse::rotateAbout(const Float& angle, const Float& aboutX, 
			    const Float& aboutY) {
  DSClosed::rotateAbout(angle, aboutX, aboutY);
}


void DSEllipse::setCenter(const Float& xPos, const Float& yPos) {
  itsCenter[0] = xPos;
  itsCenter[1] = yPos;

  if (itsValid) {
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  } else {
    calculateHandlePositions();
    buildHandles(itsHandleLocation);
    itsValid = True;
  }
}

void DSEllipse::setMajorAxis(const Float& newMajor) {
  itsMajorAxis = newMajor;
  if (itsValid) {
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }
}
void DSEllipse::setMinorAxis(const Float& newMinor) {
  itsMinorAxis = newMinor;

  if (itsValid) {
    calculateHandlePositions();
    setHandlePositions(itsHandleLocation);
  }

}
Float DSEllipse::getMinorAxis() {
  return itsMinorAxis;
}
Float DSEllipse::getMajorAxis() {
  return itsMajorAxis;
}

Vector<Float> DSEllipse::getCenter() {

  Vector<Float> z(2); z[0] = 0; z[1]= 0;
  if (itsValid) return itsCenter;

  return z;
}

void DSEllipse::setDefaultOptions() {
  itsCenter.resize(2);
  itsAngle = 0;
  itsMajorAxis = 0;
  itsMinorAxis = 0;
  setHasHandles(True);
  setDrawHandles(True);
  itsValid = False;
}






} //# NAMESPACE CASA - END

