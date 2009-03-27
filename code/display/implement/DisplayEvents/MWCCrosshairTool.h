//# MWCCrosshairTool.h: Base class for WorldCanvas event-based crosshair tools
//# Copyright (C) 1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_MWCCROSSHAIRTOOL_H
#define TRIALDISPLAY_MWCCROSSHAIRTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;

// <summary>
// Base class for MultiWorldCanvas event-based crosshair tools.
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
// MWCCrosshairTool stands for MultiWorldCanvas Crosshair Tool.
// </etymology>
//
// <synopsis> 
// This class adds to its base MWCTool to provide a tool for placing
// and moving a crosshair on a WorldCanvas.  While MWCCrosshairTool is
// not abstract, it performs no useful function.  The programmer
// should derive from this class, and override the crosshairReady function,
// which is called when the mouse button is pressed, moved, or released
// within a WC draw area where events are handled.  The get() function then
// retrieves the crosshair position in screen pixels.
//
// The crosshair will track the mouse as long as it is within the WC draw
// area and the button is pressed.
// The persistent parameter determines whether the crosshair remains visible
// once the button is released.
// The crosshair is removed when it is dragged off the draw area or the
// Esc key is pressed within the WC.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user causing
// some action by placing or moving a crosshair.
// Emitted positions are to be caught by some external process or method.
// </motivation>
//
// <todo asof="1999/11/26">
// </todo>

class MWCCrosshairTool : public MultiWCTool, public DTVisible {
  
 public:
  
  // Constructor specifies the button to respond to, and whether the 
  // crosshair should remain on screen after the button is released.
  // Base class methods must also be called to register event handling
  // for the desired WorldCanvas[es].
  MWCCrosshairTool(Display::KeySym keysym = Display::K_Pointer_Button1,
		   const Bool persistent = True);
  
  // Destructor.
  virtual ~MWCCrosshairTool();
  
  // Retrieve the crosshair position in pixels.  A derived crosshairReady()
  // routine would use this.
  virtual void get(Int &x, Int &y) const ;

  //Rectrive the crosshair position in Lin
  virtual void getLin(Double &x, Double &y) const;

  //Rectrive the crosshair position in World
  virtual void getWorld(Double &x, Double &y) const;

  // Switch the tool off: this calls the base class disable to turn off
  // event handling, and then erases the crosshair if necessary.
  virtual void disable();

  // set crosshair cursor type
  virtual void setCross(Bool cross=False);

  // Reset to non-showing, non-active crosshair.
  // Refreshes if necessary to erase (unless skipRefresh==True).
  // (Does not unregister from WCs or disable future event handling).
  virtual void reset(Bool skipRefresh=False);

  // handle events, via new-style interface.  Currently just for reset event.
  virtual void handleEvent(DisplayEvent& ev);


 protected:

  // Functions called by the base class mouse/kbd event handling operators--
  // and normally only those.  This is the input that controls the crosshair's
  // action.  When the crosshair is ready (positioned on the draw area)
  // the crosshairReady() routine is called.
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  // </group>

  // draw the crosshair on the object's currently active WC.
  // Only to be called by the base class refresh event handler.  Derived
  // objects should use refresh() if they need to redraw, but even that
  // is normally handled automatically.
  virtual void draw(const WCRefreshEvent&);

  // Called when the crosshair position has been chosen.  Override to
  // handle the crosshair-position-ready 'event'.
  // evtype is "down" "move" or "up" depending on the state of the
  // mouse leading to this event.
  virtual void crosshairReady(const String& evtype) { };

 private:

  // Set the current position from pixel coordinates.
  // To do: reset the tool when the WC CS (linToWorld) transformation
  // changes.  (There is a WorldCoordinateChange RefreshReason, but it is
  // not currently used when WC CS/Coordinatehandlers are set/changed).
  virtual void set(Int x, Int y);

  // the last crosshair position.
  // (zooms will change pixel but not linear coordinates; therefore this
  // position is stored in the latter).
  Vector<Double> itsPos;
  Vector<Double> itsWorld;

  // should the crosshair remain visible after its button is released?
  Bool itsPersist;

  // what is the crosshair radius? (screen pixels)
  Int itsRadius;

  // was the crosshair drawing visible after last refresh cycle?
  Bool itsShowing;
  
  // should the crosshair be drawn when X,Y are in the zoom window?
  Bool itsShow;

  // draw crosshair 
  Bool itsCross; 

  // is the crosshair's button down?  (True when the
  // tool's button was pressed in a WC where it is handling events,
  // and has not yet been released, or the tool reset).
  Bool itsBtnDn;

};


} //# NAMESPACE CASA - END

#endif


