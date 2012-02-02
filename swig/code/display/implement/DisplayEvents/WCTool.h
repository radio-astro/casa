//# WCTool.h: base class for WorldCanvas event-based tools
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

#ifndef TRIALDISPLAY_WCTOOL_H
#define TRIALDISPLAY_WCTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/DisplayTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WCTool;
class WorldCanvas;
class PixelCanvas;

// <summary>
// WorldCanvas position event handler for WCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a WCPositionEH which 
// passes WorldCanvas position events on to a single WCTool.
// </synopsis>

class WCToolPosEH : public WCPositionEH {
 public:
  WCToolPosEH(WCTool *tool);
  virtual ~WCToolPosEH(){};
  virtual void operator()(const WCPositionEvent& ev);
 private:
  WCTool *itsTool;
};

// <summary>
// WorldCanvas motion event handler for WCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a WCMotionEH which 
// passes WorldCanvas motion events on to a single WCTool.
// </synopsis>

class WCToolMotEH : public WCMotionEH {
 public:
  WCToolMotEH(WCTool *tool);
  virtual ~WCToolMotEH(){};
  virtual void operator()(const WCMotionEvent& ev);
 private:
  WCTool *itsTool;
};

// <summary>
// WorldCanvas refresh event handler for WCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a WCRefreshEH which 
// passes WorldCanvas refresh events on to a single WCTool.
// </synopsis>

class WCToolRefEH : public WCRefreshEH {
 public:
  WCToolRefEH(WCTool *tool);
  virtual ~WCToolRefEH(){};
  virtual void operator()(const WCRefreshEvent& ev);
 private:
  WCTool *itsTool;
};

// <summary>
// Base class for WorldCanvas event-based tools.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisites>
//   <li> WCPositionEH
//   <li> WCMotionEH
//   <li> WCRefreshEH
// </prerequisites>

// <etymology>
// WCTool stands for WorldCanvas Tool
// </etymology>

// <synopsis>
// This class is a base class upon which tools which respond to 
// various events on a WorldCanvas can be built.  It wraps up
// the position, motion and refresh events so that the programmer
// sees them all coming into one class, where they can be dealt
// with in a unified manner.  WCTool is not actually abstract,
// so the programmer need only write handlers for the events in
// which they are interested.
// </synopsis>

// <example>
// </example>

// <motivation>
// The majority of tools written for the WorldCanvas will fall 
// into the category that this class serves: they respond to a 
// single key or mouse button, and they potentially need to 
// respond to position, motion and refresh events.
// </motivation>

// <todo asof="1999/10/18">
//   <li> Nothing known
// </todo>

class WCTool : public DisplayTool {
  
 public:
  
  // Constructor taking a pointer to a WorldCanvas to which this tool
  // will attach, and a primary key to respond to.
  WCTool(WorldCanvas *wcanvas,
	 const Display::KeySym &keysym = Display::K_Pointer_Button1);
  
  // Destructor.
  virtual ~WCTool();
  
  // Switch the tool on/off - this simply registers or unregisters
  // the event handlers
  // <group>
  virtual void enable();
  virtual void disable();
  // </group>
  
  // Required operators for event handling - these are called when
  // an events occur, and distribute the events to the "user-level"
  // methods
  // <group>
  virtual void operator()(const WCPositionEvent& ev);
  virtual void operator()(const WCMotionEvent& ev);
  virtual void operator()(const WCRefreshEvent& ev);
  // </group>

  // Functions called by the local event handling operators -
  // by default they do nothing, so a derived class needs only
  // implement the events it cares about
  // <group>
  virtual void keyPressed(const WCPositionEvent &ev);
  virtual void keyReleased(const WCPositionEvent &ev);
  virtual void otherKeyPressed(const WCPositionEvent &ev);
  virtual void otherKeyReleased(const WCPositionEvent &ev);
  virtual void moved(const WCMotionEvent &ev);
  virtual void refresh(const WCRefreshEvent &ev);
  // </group>

  // Get the WorldCanvas that this Tool is attached to
  virtual WorldCanvas *worldCanvas() const 
    { return itsWorldCanvas; }

  // Get the PixelCanvas that this Tool is attached to via its WorldCanvas
  virtual PixelCanvas *pixelCanvas() const
    { return itsPixelCanvas; }

 protected:

  // (Required) default constructor.
  WCTool();

  // (Required) copy constructor.
  WCTool(const WCTool &other);

  // (Required) copy assignment.
  WCTool &operator=(const WCTool &other);  

 private:

  // The WorldCanvas to which this is connected
  WorldCanvas *itsWorldCanvas;

  // The PixelCanvas to which this is connected via the WorldCanvas
  PixelCanvas *itsPixelCanvas;

  // whether the event handlers are registered
  Bool itsEventHandlersRegistered;

  // event handlers:
  WCToolPosEH *itsPositionEH;
  WCToolMotEH *itsMotionEH;
  WCToolRefEH *itsRefreshEH;

};


} //# NAMESPACE CASA - END

#endif


