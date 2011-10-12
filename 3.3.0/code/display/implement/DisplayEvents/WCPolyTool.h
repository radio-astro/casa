//# WCPolyTool.h: Base class for WorldCanvas event-based polygon tools
//# Copyright (C) 1999,2000
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
//# $Id$

#ifndef TRIALDISPLAY_WCPOLYTOOL_H
#define TRIALDISPLAY_WCPOLYTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based polygon tools
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li><linkto>WCTool</linkto>
// </prerequisites>
//
// <etymology>
// WCPolyTool stands for WorldCanvas Polygon Tool
// </etymology>
//
// <synopsis>
// This class adds to its base WCTool to provide a tool for drawing,
// reshaping and moving polygons on a WorldCanvas.  While WCPolyTool
// is not abstract, it performs no useful function.  The programmer
// should derive from this class and override the functions doubleInside
// and doubleOutside at the very least.  These are called when the user
// double-clicks a particular key or mouse button inside or outside an
// existing polygon respectively.  It is up to the programmer to decide
// what these events mean, but it is recommended that an internal double-
// click correspond to the main action of the tool, eg. emitting the 
// polygon vertices to the application, and that an external double-click
// correspond to a secondary action of the tool, if indeed there are
// additional actions suitable to the tool.
//
// The polygon is drawn by clicking at each of the vertices, and
// clicking on the last or first vertices to complete the polygon.
// Once drawn, the vertices can be moved by dragging their handles,
// and the entire polygon relocated by dragging inside the polygon.
// The polygon is removed from the display when the Esc key is
// pressed.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user drawing
// a polygon and using the polygon in some operation.
// </motivation>
//
// <todo asof="1999/02/24">
//   <li> Add time constraint to double click detection
// </todo>

class WCPolyTool : public WCTool, public DTVisible {

 public:

  // Constructor
  WCPolyTool(WorldCanvas *wcanvas,
	     Display::KeySym keysym = Display::K_Pointer_Button1,
	     const Bool persistent = False);

  // Destructor
  virtual ~WCPolyTool();

  // Switch the tool off - this calls the base class disable,
  // and then erases the polygon if it's around
  virtual void disable();

  // Functions called by the local event handling operators -
  // these handle the drawing of the polygon.  In special 
  // conditions, namely double clicking the key, they will
  // pass control on to the doubleInside and doubleOutside
  // functions
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  virtual void refresh(const WCRefreshEvent &ev);
  // </group>

  // Functions special to the polygon event handling - called when
  // there is a double click inside/outside the polygon
  // <group>
  virtual void doubleInside() { };
  virtual void doubleOutside() { };
  // </group>

  // Functions called when a polygon is ready and not being 
  // editted, and when this status changes
  // <group>
  virtual void polygonReady() { };
  virtual void polygonNotReady() { };
  // </group>  

  // Retrive the polygon vertices
  virtual void get(Vector<Int> &x, Vector<Int> &y);

 private:

  // do the polygons persist after double clicks?
  Bool itsPolygonPersistent;

  // is the polygon on screen?
  Bool itsOnScreen;

  // are we actively drawing?
  Bool itsActive;

  // have we moved?
  Bool itsMoved;

  // do we have a polygon yet?
  Bool itsPolygonExists;

  // adjustment mode
  enum AdjustMode {
    Off,
    Move,
    Handle
  };
  WCPolyTool::AdjustMode itsAdjustMode;

  // Number of points
  uInt itsNPoints;

  // Polygon points
  // ... pixels
  Vector<Int> itsX, itsY;
  // ... world
  Vector<Double> itsStoredWorldX, itsStoredWorldY;

  // Point operations
  // <group>
  void pushPoint(uInt x1, uInt y1);
  void popPoint();
  // </group>

  // preserve/restore the world coordinates
  // <group>
  void preserve();
  void restore();
  // </group>

  // draw the polygon on a PixelCanvas
  void draw(Bool drawHandles = False);
  
  // reset this drawer
  void reset();

  // size in pixels of the handles
  uInt itsHandleSize;
  
  // are we within the specified handle?
  Bool inHandle(const uInt &pt, const uInt &x, const uInt &y) const;

  // which handle to modify?
  uInt itsSelectedHandle;

  // are we inside the polygon?
  Bool inPolygon(const uInt &x, const uInt &y) const;

  // position that move started from
  Int itsBaseMoveX, itsBaseMoveY;

  // position of last press event
  Int itsLastPressX, itsLastPressY;
  Int its2ndLastPressX, its2ndLastPressY;

  // position of last release event
  Int itsLastReleaseX, itsLastReleaseY;

  // store the times of the last two presses here:
  Double itsLastPressTime, its2ndLastPressTime;

};


} //# NAMESPACE CASA - END

#endif


