//# WCCrosshairTool.h: Base class for WorldCanvas event-based crosshair tools
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

#ifndef TRIALDISPLAY_WCCROSSHAIRTOOL_H
#define TRIALDISPLAY_WCCROSSHAIRTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based crosshair tools.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li> WCTool
// </prerequisites>
//
// <etymology>
// WCCrosshairTool stands for WorldCanvas Crosshair Tool.
// </etymology>
//
// <synopsis> 
// This class adds to its base WCTool to provide a tool for placing
// and moving a crosshair on a WorldCanvas.  While WCCrosshairTool is
// not abstract, it performs no useful function.  The programmer
// should derive from this class, and override the functions
// doubleInside and doubleOutside, which are called when the user
// double-clicks the key or mouse button inside or outside the
// existing crosshair respectively.  It is up to the programmer to
// decide what double clicks inside and outside the crosshair
// correspond to, although it is recommended that a double click
// inside correspond to the main action of the tool (eg. emitting the
// current position to an outside controller), and a double click
// outside correspond to a secondary action of the tool, if indeed a
// secondary action exists.
//
// The crosshair is drawn by simply clicking at the location the
// crosshair should be placed.  Once constructed, the crosshair can be
// relocated by dragging inside the crosshair.  The crosshair is
// removed from the display when the Esc key is pressed.  
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user placing
// a crosshair, and then proceeding to some action with that crosshair.
// A nice example is emitting positions to be caught by some external
// controlling process.
// </motivation>
//
// <todo asof="1999/11/26">
// </todo>

class WCCrosshairTool : public WCTool, public DTVisible {
  
 public:
  
  // Constructor requires a WorldCanvas to operate on, and optional
  // specifications of the key to respond to, and whether the
  // crosshair should be persistent.  A persistent crosshair will
  // "stick around" after a double click action.
  WCCrosshairTool(WorldCanvas *wcanvas,
	     Display::KeySym keysym = Display::K_Pointer_Button1,
	     const Bool persistent = True);
  
  // Destructor.
  virtual ~WCCrosshairTool();
  
  // Switch the tool off: this calls the base class disable, and then
  // erases the crosshair if necessary.
  virtual void disable();

  // Functions called by the local event handling operators - these
  // handle the drawing of the crosshair.  In special conditions,
  // namely double clicking the key, they will pass control on to the
  // doubleInside and doubleOutside functions.
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  virtual void refresh(const WCRefreshEvent &ev);
  // </group>

  // Functions special to the crosshair event handling: called when
  // there is a double click inside or outside the crosshair.
  // <group>
  virtual void doubleInside() { };
  virtual void doubleOutside() { };
  // </group>

  // Functions called when the crosshair is ready and not being
  // editted, and when this status changes.
  // <group>
  virtual void crosshairReady() { };
  virtual void crosshairNotReady() { };
  // </group>

  // Retrieve the crosshair position.
  virtual void get(Int &x1, Int &y1) const ;

 private:

  // does the crosshair persist after double clicks?
  Bool itsCrosshairPersistent;

  // what radius is the crosshair? in pixels
  Int itsCrosshairRadius;

  // is the crosshair on screen?
  Bool itsOnScreen;
  
  // is some activity taking place with the crosshair?
  Bool itsActive;

  // has the crosshair been moved?
  Bool itsMoved;

  // do we have a crosshair drawn yet?
  Bool itsCrosshairExists;

  // adjustment mode
  enum AdjustMode {
    Off, 
    Move
  };
  WCCrosshairTool::AdjustMode itsAdjustMode;

  // coordinates of the crosshair: pixel and world
  // <group>
  Int itsX1, itsY1;
  Vector<Double> itsStoredWorldPosition;
  // </group>

  // set the coordinates of the crosshair
  // <group>
  virtual void set(const Int &x1, const Int &y1);
  // </group>

  // set/get only the anchor point
  // <group>
  //virtual void set(const Int &x1, const Int &y1);
  //virtual void get(Int &x1, Int &y1) const ;
  // </group>

  // preserve/restore the world coordinates
  // <group>
  virtual void preserve();
  virtual void restore();
  // </group>

  // draw the crosshair on the WorldCanvas' PixelCanvas.
  virtual void draw(const Bool drawHandles = False);
  
  // reset this drawer
  virtual void reset();

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


