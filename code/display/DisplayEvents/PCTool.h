//# PCTool.h: base class for PixelCanvas event-based tools
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

#ifndef TRIALDISPLAY_PCTOOL_H
#define TRIALDISPLAY_PCTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCPositionEH.h>
#include <display/DisplayEvents/PCMotionEH.h>
#include <display/DisplayEvents/PCRefreshEH.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PCTool;
	class PixelCanvas;

// <summary>
// PixelCanvas position event handler for PCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a PCPositionEH which
// passes PixelCanvas position events on to a single PCTool.
// </synopsis>

	class PCToolPosEH : public PCPositionEH {
	public:
		PCToolPosEH(PCTool *tool);
		virtual ~PCToolPosEH() {};
		virtual void operator()(const PCPositionEvent& ev);
	private:
		PCTool *itsTool;
	};

// <summary>
// PixelCanvas motion event handler for PCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a PCMotionEH which
// passes PixelCanvas motion events on to a single PCTool.
// </synopsis>

	class PCToolMotEH : public PCMotionEH {
	public:
		PCToolMotEH(PCTool *tool);
		virtual ~PCToolMotEH() {};
		virtual void operator()(const PCMotionEvent& ev);
	private:
		PCTool *itsTool;
	};

// <summary>
// PixelCanvas refresh event handler for PCTool.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a PCRefreshEH which
// passes PixelCanvas refresh events on to a single PCTool.
// </synopsis>

	class PCToolRefEH : public PCRefreshEH {
	public:
		PCToolRefEH(PCTool *tool);
		virtual ~PCToolRefEH() {};
		virtual void operator()(const PCRefreshEvent& ev);
	private:
		PCTool *itsTool;
	};

// <summary>
// Base class for PixelCanvas event-based tools.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisites>
//   <li> PCPositionEH
//   <li> PCMotionEH
//   <li> PCRefreshEH
// </prerequisites>

// <etymology>
// PCTool stands for PixelCanvas Tool.
// </etymology>

// <synopsis>
// This class is a base class upon which tools which respond to
// various events on a PixelCanvas can be built.  It wraps up
// the position, motion and refresh events so that the programmer
// sees them all coming into one class, where they can be dealt
// with in a unified manner.  PCTool is not actually abstract,
// so the programmer need only write handlers for the events in
// which they are interested.
// </synopsis>

// <example>
// </example>

// <motivation>
// The majority of tools written for the PixelCanvas will fall
// into the category that this class serves: they respond to a
// single key or mouse button, and they potentially need to
// respond to position, motion and refresh events.
// </motivation>

// <todo asof="1999/10/09">
//   <li> Nothing known
// </todo>

	class PCTool {

	public:

		// Constructor.  A PixelCanvas to attach to must be specified,
		// and normally a key to respond to as well.
		PCTool(PixelCanvas *pcanvas,
		       Display::KeySym keysym = Display::K_Pointer_Button1);

		// Destructor.
		virtual ~PCTool();

		// Switch the tool on/off.  This simply registers or unregisters the
		// event handlers.
		// <group>
		virtual void enable();
		virtual void disable();
		// </group>

		// Required operators for event handling.  These are called when
		// events occur, and distribute the events to the "user-level"
		// methods.
		// <group>
		virtual void operator()(const PCPositionEvent& ev);
		virtual void operator()(const PCMotionEvent& ev);
		virtual void operator()(const PCRefreshEvent& ev);
		// </group>

		// Functions called by the local event handling operators.  By
		// default they do nothing, so a derived class needs only implement
		// the events it cares about.
		// <group>
		virtual void keyPressed(const PCPositionEvent &ev);
		virtual void keyReleased(const PCPositionEvent &ev);
		virtual void otherKeyPressed(const PCPositionEvent &ev);
		virtual void otherKeyReleased(const PCPositionEvent &ev);
		virtual void moved(const PCMotionEvent &ev);
		virtual void refresh(const PCRefreshEvent &ev);
		// </group>

		// Get the PixelCanvas to which this Tool is attached.
		virtual PixelCanvas *pixelCanvas() const {
			return itsPixelCanvas;
		}

		// Set/get which key to catch.
		// <group>
		virtual void setKey(const Display::KeySym &keysym);
		virtual Display::KeySym getKey() const {
			return itsKeySym;
		}
		// </group>

	protected:

		// Indicate whether key is presently pressed.
		virtual Bool keyPresentlyDown(const PCMotionEvent &ev);

	private:

		// The PixelCanvas to which this is connected.
		PixelCanvas *itsPixelCanvas;

		// The key to handle.
		Display::KeySym itsKeySym;

		// The modifier mask for the key.
		Display::KeyModifier itsKeyModifier;

		// Whether the event handlers are currently registered.
		Bool itsEventHandlersRegistered;

		// The event handlers.
		PCToolPosEH *itsPositionEH;
		PCToolMotEH *itsMotionEH;
		PCToolRefEH *itsRefreshEH;

	};


} //# NAMESPACE CASA - END

#endif


