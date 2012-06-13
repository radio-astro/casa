//# DisplayShapeWithCoords.h : Base class for DisplayShapes with Coords
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
#ifndef TRIALDISPLAY_DISPLAYSHAPEWITHCOORDS_H
#define TRIALDISPLAY_DISPLAYSHAPEWITHCOORDS_H

#include <casa/aips.h>
#include <display/DisplayShapes/DisplayShape.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for DisplayShapes with Coords
// </summary>

class Record;
class PixelCanvas;
class PanelDisplay;
class WorldCanvas;

template <class T> class Vector;
template <class T> class Matrix;

class DisplayShapeWithCoords {
  
public:
  virtual Record getRawOptions() = 0;

  static void floatToPix(Record& inHere, const String& field);
  static void floatPointToPix(Record& inHere, const String& field);

  static void matrixFloatToQuant(Record& inHere, const String& field,
				 const String& unit);
  static void matrixFloatFromQuant(Record& inHere, 
				   const String& field,
				   const String& onlyParseTheseUnits);

  static void floatFromPix(Record& inHere, const String& field);
  static void floatPointFromPix(Record& inHere, const String& field);

  static WorldCanvas* chooseWCFromWorldPoint(const Record& settings, 
					     PanelDisplay* pd);

  static WorldCanvas* chooseWCFromWorldPoints(const Record& settings,
					      const String& fieldWithPoints,
					      PanelDisplay* pd);

  static WorldCanvas* chooseWCFromPixPoint(const Float& xPos, 
					   const Float& yPos,
					   PanelDisplay* pd);

  static WorldCanvas* chooseWCFromPixPoints(const Matrix<Float> points,
					    PanelDisplay* pd);
  
  static Vector<Float> floatPointFromQuant(const Record &inHere, 
					   const String& field,
					   String& units);

  static Vector<Float> relToScreen(const Vector<Float>& rel, PixelCanvas* pc);
  static Vector<Float> screenToRel(const Vector<Float>& screen, 
				   PixelCanvas* pc);

  static Matrix<Float> relToScreen(const Matrix<Float>& rel, PixelCanvas* pc);
  static Matrix<Float> screenToRel(const Matrix<Float>& screen, 
				   PixelCanvas* pc);
  
  static Bool inWorldCanvasDrawArea(const Matrix<Float> points, 
				    WorldCanvas* wc);

  static void pixelToWorldPoints(Record& inHere, const String& fieldname,
				 WorldCanvas* wc);
  
};

} //# NAMESPACE CASA - END

#endif






