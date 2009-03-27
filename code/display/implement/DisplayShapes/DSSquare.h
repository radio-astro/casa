//# DSSquare.h: Square implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSSQUARE_H
#define TRIALDISPLAY_DSSQUARE_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSRectangle.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a square. 
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSLine">DSLine</linkto>
// <li> <linkto class="DSPoly">DSPoly</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSSquare is a method of managing the drawing of a square onto a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSSquare simple extends DSRectangle, and takes much of its functionality 
// from there. It also overrides some functions to ensure that during 
// manipulation of the shape, it remains a square.
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
// To enable the drawing of squares onto a pixel canvas
// </motivation>
//
// <example>
// </example>


class DSSquare : public DSRectangle { 

public: 
  
  // Default constructor
  DSSquare();

  // Constructor. Accepts the centre (xPos, yPos), the size (height/width
  // in pixels), and whether or not it will ever have handles and if
  // so, whether to draw them now.
  DSSquare(const Float& xPos, const Float& yPos, const Float& size, 
	   const Bool& handles = False, const Bool& drawHandles = False);

  // Copy cons.
  DSSquare(const DSSquare& other);

  // Destructor
  virtual ~DSSquare();

  // Copy constructor


  // Set the size of the square in pixels
  virtual void setSize(const Float& size);

  // Change the points, while maintaining the shape as a square
  // <group>
  virtual void changePoint(const Vector<Float>& pos);
  virtual void changePoint(const Vector<Float>& pos, const Int nPoint);
  // </group>

  // Get and set options
  // <group>
  virtual Bool setOptions(const Record& settings);
  virtual Record getOptions();
  // </group>

private:

};

} //# NAMESPACE CASA - END

#endif





