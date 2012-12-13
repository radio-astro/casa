//# DSPolyLine.h: PolyLine implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSPOLYLINE_H
#define TRIALDISPLAY_DSPOLYLINE_H

#include <casa/aips.h> 
#include <casa/Arrays/Matrix.h> 
#include <casa/Arrays/Vector.h>
#include <display/DisplayShapes/DSBasic.h> 


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a polyline.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSPolyLine is a method of managing the drawing of a polyline onto 
// a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSPolyLine manages a collection of points, which when called it draws. 
// It also manages actions like the addition and removal of points from/to 
// the line. 
//
// There are generally two ways to make DisplayShape(s); To create them in 
// "one hit" by providing arguments to the constructor, or by using the 
// default constructor and then the "setOptions" method. A simple interface 
// for all classes inheriting from the 
// <linkto class="DisplayShape">DisplayShape</linkto> class is provided by 
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// </synopsis>
//
// <motivation>
// The need to draw, move and manipulate a polyline.
// </motivation>
//
// <example>
// <srcblock>
//
// </srcblock>
// </example>


class DSPolyLine : public DSBasic {

public: 
  
  // Constructors and Destructors
  // <group>
  DSPolyLine();
  DSPolyLine(const DSPolyLine& other);
  DSPolyLine(const Matrix<Float>& points, const Bool& handles, 
	     const Bool& drawHandles);
  virtual ~DSPolyLine();
  // </group>

  // General DisplayShape functions 
  // <group>
  virtual void draw(PixelCanvas* pc);
  virtual Bool inObject(const Float& xPos, const Float& yPos);
  virtual void move(const Float& dX, const Float& dY);  
  virtual Vector<Float> getCenter();
  virtual void setCenter(const Float& xPos, const Float& yPos);
  virtual void scale(const Float& scaleFactor);
  virtual void rotate(const Float& angle);
  virtual void rotateAbout(const Float& angle, const Float& aboutX, 
			   const Float& aboutY);
  // </group>

  // Point manipulation functions. The changePoint with only one argument 
  // moves the closest point to the location specified.
  // <group>
  virtual void changePoint(const Vector<Float>& pos); 
  virtual void changePoint(const Vector<Float>& pos, const Int n); 
  virtual void addPoint(const Vector<Float>& newPos);
  virtual void setPoints(const Matrix<Float>& points);
  // </group>
  
  // Return the number of points.
  virtual uInt nPoints();
  
  // Get and set this shapes options. 
  // <group>
  virtual Bool setOptions(const Record& newSettings);
  virtual Record getOptions();
  // </group>

protected:
  // Return all my points.
  virtual Matrix<Float> getPoints();

private:
  Matrix<Float> itsPoints;
  Vector<Float> itsCenter;
  Bool itsValidCenter;
  // Used when default constructor called.
  virtual void setDefaultOptions();
};

} //# NAMESPACE CASA - END

#endif




