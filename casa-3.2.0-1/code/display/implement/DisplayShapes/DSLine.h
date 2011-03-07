//# DSLine.h: Line implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSLINE_H
#define TRIALDISPLAY_DSLINE_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSPolyLine.h>
#include <casa/Arrays/Matrix.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template <class T> class Vector;

// <summary>
// Implementation of a line.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSPolyLine">DSPoly</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSLine is a method of managing the drawing of a line onto a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSLine simply extends from DSPolyLine, and adds specific functions to a 
// line with only two points (e.g. setStartPoint) 
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
// The need for a basic line drawing tool. 
// </motivation>
//
// <example>
// <srcblock>
// Vector<Float> startPoint(2); startPoint[0] = 100; startPoint[1] = 100;
// Vector<Float> endPoint(2);   endPoint[0] = 200;   endPoint[1] = 200;
//
// DSLine* myLine = new DSLine(startPoint, endPoint, True, True);
//
// myLine->move(10,10);
//
// Vector<Float> newStart(2); newStart[0] = 50; newStart[1] = 50;
// myLine->setStartPoint(newStart);
//
// Record newLineOpt;
// newLineOpt.define("linewidth", 3);
//
// myLine->setOptions(newLineOpt);
// myLine->draw(myPixelCanvas);
// etc..
// </srcblock>
// </example>

class DSLine : public DSPolyLine {

public:

  // Constructors and Destructors
  // <group>
  DSLine();
  DSLine(const DSLine &other);
  DSLine(const Vector<Float>& startPos, const Vector<Float>& endPos, 
	 const Bool& handles = True, const Bool& drawHandles = True);
  virtual ~DSLine();
  // </group>

  // This does nothing, it's so arrow and other inheriting classes can 
  // take note of new centers
  virtual void setCenter(const Float& xPos, const Float& yPos);

  // Does this line have a valid start and a valid end?
  virtual Bool isValid();

  // Line specific functions for ease of use
  // <group>
  virtual void setStartPoint(const Vector<Float>& start);
  virtual void setEndPoint(const Vector<Float>& end);
  // </group>

  // Set and get options
  // <group>
  virtual Record getOptions();
  virtual Bool setOptions(const Record& newSettings);
  // </group>

private:


  Bool itsValidStart, itsValidEnd;

  // These are to hold the points while line is being made (line is invalid).
  Vector<Float> itsStart;
  Vector<Float> itsEnd;

protected: 
  
  virtual Bool validStart() {
    return itsValidStart;
  }

  virtual Bool validEnd() {
    return itsValidEnd;
  }

  virtual void make();
  // General utility functions. 
  // <group>
  virtual Matrix<Float> getEnds();
  virtual Matrix<Float> asPolyLine(const Vector<Float>& startPos, const Vector<Float>& endPos);
  // </group>
};


} //# NAMESPACE CASA - END

#endif



