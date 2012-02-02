//# MWCPolyTool.h: Base class for MultiWorldCanvas event-based polyline tools
//# Copyright (C) 2003
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

#ifndef TRIALDISPLAY_MWCPOLYLINETOOL_H
#define TRIALDISPLAY_MWCPOLYLINETOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based polyline tools
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
// MWCPolylineTool stands for Multi-WorldCanvas Polyline Tool
// </etymology>
//
// <synopsis>
// This class adds to its base MWCTool to provide a tool for drawing,
// reshaping and moving polylines on a WorldCanvas.  While MWCPolylineTool
// is not abstract, it performs no useful function.  The programmer
// should derive from this class and override the functions doubleInside
// and doubleOutside at the very least.  These are called when the user
// double-clicks a particular key or mouse button inside or outside an
// existing polyline respectively.  It is up to the programmer to decide
// what these events mean, but it is recommended that an internal double-
// click correspond to the main action of the tool, eg. emitting the 
// polyline vertices to the application, and that an external double-click
// correspond to a secondary action of the tool, if indeed there are
// additional actions suitable to the tool.
//
// The polyline is drawn by clicking at each of the vertices, and
// clicking again on the last to complete the polyline.
// Once drawn, the vertices can be moved by dragging their handles,
// and the entire polyline relocated by dragging inside the polyline.
// The polyline is removed from the display when the Esc key is
// pressed.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user drawing
// a polyline and using the polyline in some operation.
// </motivation>
//
// <todo asof="2003/12/15">
//   <li> Add time constraint to double click detection
// </todo>

class MWCPolylineTool : public MultiWCTool, public DTVisible {

public:

  // Constructor
  MWCPolylineTool(Display::KeySym keysym = Display::K_Pointer_Button1,
		  const Bool persistent = False);

  // Destructor
  virtual ~MWCPolylineTool();

  // Switch the tool off - this calls the base class disable,
  // and then erases the polyline if it's around
  virtual void disable();

  // reset to non-existent, non-active polyline.
  // Refreshes if necessary to erase (unless skipRefresh==True).
  // (Does not unregister from WCs or disable future event handling).
  virtual void reset(Bool skipRefresh=False);
  
protected:

  // Functions called by the base class event handling operators--and
  // normally only those.  This is the input that controls the polyline's
  // appearance and action.  When the polyline is ready and double-click
  // is received, the doubleInside/Outside routine is invoked.
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  // </group>

  // draw the polyline (if any) on the object's currently active WC.
  // Only to be called by the base class refresh event handler.  Derived
  // objects should use refresh() if they need to redraw, but even that
  // is normally handled automatically by this class.
  virtual void draw(const WCRefreshEvent&);

  // Output callback functions--to be overridden in derived class as needed.
  // Called when there is a double click inside/outside the polyline
  // <group>
  virtual void doubleInside() { };
  virtual void doubleOutside() { };
  // </group>

  // Function called when the polyline is ready and not being 
  // edited.  (Useful for e.g. slicing).
  virtual void polylineReady() { };

  // Retrieve polyline vertices, or a single vertex, in screen pixels.
  // Valid results during the callback functions; to be used by them,
  // as well as internally.
  // <group>
  virtual void get(Vector<Int> &x, Vector<Int> &y) const;
  virtual void get(Int &x, Int &y, const Int pt) const;
  void getLinear(Vector<Float> &x, Vector<Float> &y) const;
  // </group>

 private:

  // Set the polyline vertices. itsNPoints should already be set, and
  // x and y must contain (at least) this many points.
  virtual void set(const Vector<Int> &x, const Vector<Int> &y);

  // replace a single vertex.
  virtual void set(const Int x, const Int y, const Int pt);
  // push/pop last vertex
  // <group>
  void pushPoint(Int x1, Int y1);
  void popPoint();
  // </group>

  // are we inside the polyline?
  Bool inPolyline(const Int &x, const Int &y) const;

  // are we within the specified handle?
  Bool inHandle(const Int &pt, const Int &x, const Int &y) const;


  // should the polyline remain on screen after double clicks?
  Bool itsPolylinePersistent;

  // state of the polyline tool
  enum AdjustMode {
    Off,	// Nothing exists yet
    Def,	// defining initial polyline
    Ready,	// polyline finished, no current activity
    Move,	// moving entire polyline
    Resize };	// moving single vertex whose handle was pressed
  MWCPolylineTool::AdjustMode itsMode;

  // set True on double-click, if the polyline is persistent.
  // set False when the polyline is moved, resized or reset.
  // If True, a click outside the polyline will erase it and begin
  // definition of a new one.
  Bool itsEmitted;

  // Number of points
  Int itsNPoints;

  // Polyline points (linear).  Not to be used directly.
  // use get, set, push, pop instead, which take pixel coordinate arguments.
  // It's done this way so that zooms work on the figures.
  Vector<Double> itsX, itsY;

  // size in pixels of the handles
  Int itsHandleSize;
  
  // vertex being moved
  Int itsSelectedHandle;

  // position that move started from
  Int itsBaseMoveX, itsBaseMoveY;

  // times of the last two presses
  Double itsLastPressTime, its2ndLastPressTime;

};


} //# NAMESPACE CASA - END

#endif


