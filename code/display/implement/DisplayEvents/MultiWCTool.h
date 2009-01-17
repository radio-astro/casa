//# MultiWCTool.h: base class for MultiWorldCanvas event-based tools
//# Copyright (C) 2000,2001,2002
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

#ifndef TRIALDISPLAY_MULTIWCTOOL_H
#define TRIALDISPLAY_MULTIWCTOOL_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/DisplayTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MultiWCTool;
class WorldCanvas;
class PixelCanvas;
class PanelDisplay;

// <summary>
// Base class for MultiWorldCanvas event-based tools.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li> WCPositionEH
//   <li> WCMotionEH
//   <li> WCRefreshEH
// </prerequisites>
//
// <etymology>
// MultiWCTool stands for MultiWorldCanvas Tool
// </etymology>
//
// <synopsis>
// This class is a base class upon which tools which respond to 
// various events on a WorldCanvas can be built.  It wraps up
// the position, motion and refresh events so that the programmer
// sees them all coming into one class, where they can be dealt
// with in a unified manner.  MultiWCTool is not actually abstract,
// so the programmer need only write handlers for the events in
// which they are interested.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// The majority of tools written for the WorldCanvas will fall 
// into the category that this class serves: they respond to a 
// single key or mouse button, and they potentially need to 
// respond to position, motion and refresh events.
// </motivation>
//
// <todo asof="2000/07/12">
//   <li> Nothing known
// </todo>

class MultiWCTool : public DisplayTool,
		    public WCPositionEH,
		    public WCMotionEH,
		    public WCRefreshEH {
  
 public:
  
  // Constructor taking the primary key to which this tool will
  // respond.
  MultiWCTool(const Display::KeySym &keysym = Display::K_Pointer_Button1);
  
  // Destructor.
  virtual ~MultiWCTool();

  // Add/remove a WorldCanvas from the control of this tool.
  // <group>
  virtual void addWorldCanvas(WorldCanvas &worldcanvas);
  virtual void removeWorldCanvas(WorldCanvas &worldcanvas);
  // </group>
  
  // Add/Remove a list of WorldCanvases from PanelDisplay
  // <group>
  virtual void addWorldCanvases(PanelDisplay* pdisp);
  virtual void removeWorldCanvases(PanelDisplay* pdisp);
  // </group>

  // Switch the tool on/off - this simply registers or unregisters
  // the event handlers on the WorldCanvases.
  // <group>
  virtual void enable();
  virtual void disable();
  // </group>
  
  // Required operators for event handling - these are called when
  // events occur, and distribute the events to the "user-level"
  // methods
  // <group>
  virtual void operator()(const WCPositionEvent& ev);
  virtual void operator()(const WCMotionEvent& ev);
  virtual void operator()(const WCRefreshEvent& ev);
  // </group>

  // Derived classes should implement this to return to the non-showing,
  // non-active state.  It should not unregister the tool from WCs or
  // disable event handling.  If skipRefresh is false and the tool was showing,
  // it also calls refresh() to erase.  (The caller should set skipRefresh=True
  // (only) if it will handle refresh itself).
  virtual void reset(Bool skipRefresh=False) {  }

 protected:

  // Functions called by the local event handling operators -
  // by default they do nothing, so a derived class needs only
  // implement the events it cares about
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  virtual void otherKeyReleased(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  virtual void updateRegion() {}
  // </group>

  // Draw whatever should be drawn (if anything) on current WC.
  // Should only be called by refresh event handler.
  virtual void draw(const WCRefreshEvent &ev);

  // Copy back-to-front buffer (erasing all MWCTool drawings),
  // then cause this (and all MWCTools on current WC's PC)
  // to draw (or not draw) themselves, according to their
  // current state.  Mouse and kbd event handlers within the tools
  // now call this in response to drawing state changes, rather than
  // calling draw() directly, so that only valid tool drawings are
  // displayed, even when more than one is active.  NB: the meaning of
  // this routine has changed to be more in line with the rest of the DL
  // (i.e., it now causes, rather than responds to, refresh events).
  virtual void refresh();

  // An iterator for the WorldCanvases.
  mutable ListIter<WorldCanvas *> *itsWCListIter;

  // Cause subsequent drawing commands to be clipped to the current WC
  // (or its drawing area).  Be sure to reset when finished drawing; 
  // clipping will apply to drawing on entire PC.
  // <group>
  virtual void setClipToDrawArea();
  virtual void setClipToWC();
  virtual void resetClip();
  // </group>

  // WC being (or to be) drawn on (may be 0 initially).  WC where latest
  // relevant input event was received.
  WorldCanvas *itsCurrentWC;

 private:

  // copy, default constructors (do not use)
  // <group>
  MultiWCTool();
  MultiWCTool(const MultiWCTool &other);
  MultiWCTool &operator=(const MultiWCTool &other);  
  // </group>

  // The WorldCanvases to which this tool is connected.
  List<WorldCanvas *> itsWCList;

  // whether the event handlers are registered
  Bool itsEventHandlersRegistered;

};


} //# NAMESPACE CASA - END

#endif
