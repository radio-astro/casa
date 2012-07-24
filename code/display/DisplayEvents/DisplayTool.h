//# DisplayTool.h: base class for event-based tools in the display classes
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

#ifndef TRIALDISPLAY_DISPLAYTOOL_H
#define TRIALDISPLAY_DISPLAYTOOL_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for event-based tools in the display classes.
// </summary>

// <use visibility=local>

// <reviewed reviewer="Ralph Marson" date="2000/04/18" tests="tDisplayTool">
// </reviewed>

// <etymology>
// "DisplayTool" is a base interface for "Tools" to be used in "Display"
// applications.
// <etymology>

// <synopsis>
// This class is a simple base class which provides a suitable base upon
// which to build interactive user tools for the display classes.  It
// simply provides a facility to setup a key to catch and respond to, and
// an interface to enable and disable the tool.
// </synopsis>

// <example>
// A function to provide information about a DisplayTool might look like:
// <srcblock>
// void MyClass::printToolKey(const DisplayTool &tool) const {
//     cout << tool.getKey() << endl;
// }
// </srcblock>
// </example>

// <motivation>
// This class unites all key handling for display tools.
// </motivation>

// <todo asof="2000/01/17">
// Nothing known.
// </todo>

class DisplayTool {

 public:

  // Destructor.
  virtual ~DisplayTool();

  // Switch the tool on/off.  This simply registers or unregisters the
  // event handlers.
  // <group>
  virtual void enable() = 0;
  virtual void disable() = 0;
  // </group>
  
  // Set/get which key to catch.
  // <group>
  virtual void setKey(const Display::KeySym &keysym);
  virtual Display::KeySym getKey() const;
  // </group>

 protected:

  // Constructor taking a key to which this tool will initially be
  // attached, typically one of the pointer buttons.
  explicit DisplayTool(const Display::KeySym &keysym = 
		       Display::K_Pointer_Button1);

  // Copy constructor - construct a new DisplayTool from
  // <src>other</src>, using copy semantics.
  DisplayTool(const DisplayTool &other);

  // Copy assignment using copy semantics.
  DisplayTool &operator=(const DisplayTool &other);

  // Return the modifier mask.  Some keys, specifically the mouse
  // (pointer) buttons, have associated with them a bit mask which is
  // used to determine if that key is currently held down.  This
  // function returns the modifier for the key of this tool, and
  // returns 0 if the key does not have a modifier bit mask, eg. it is
  // an alphanumeric key on the keyboard.
  Display::KeyModifier keyModifiers()
    { return itsKeyModifier; }

 private:

  // The key to handle.
  Display::KeySym itsKeySym;

  // The modifier mask for the key.
  Display::KeyModifier itsKeyModifier;

  // Support function to choose a key modifier for the tool key.
  void chooseKeyModifier();

};


} //# NAMESPACE CASA - END

#endif
