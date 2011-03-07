//# WCSizeControlHandler.h: base class for control of WorldCanvas size
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000
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

#ifndef TRIALDISPLAY_WCSIZECONTROLHANDLER_H
#define TRIALDISPLAY_WCSIZECONTROLHANDLER_H

#include <casa/aips.h>
namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;

// <summary>
// Base class defining the interface to controlling the WorldCanvas size.
// </summary>
//
// <prerequisite>
// <li> <linkto class="WorldCanvas">WorldCanvas</linkto>
// </prerequisite>
//
// <etymology>
// WCSizeControlHandler : WorldCanvas size control handler
// </etymology>
//
// <synopsis>
// The intention of this class is to provide a means of allowing
// a group of display objects to determine among themselves the
// optimum size of the display for the rendering required.
//
// The WCSizeControlHandler could, for example, be programmed to
// fudge the display size to make it more convenient to display 
// images of a certain size.
// </synopsis>
//
// <motivation>
// Allow for client programmer to control the size of the
// <linkto class="WorldCanvas">WorldCanvas</linkto>.
// </motivation>
//
// <example>
// none available yet.
// </example>
//
// <todo>
// <li> Decide on implementation details
// <li> Get a concrete class implemented
// </todo>
//

class WCSizeControlHandler {

 public:

  // Default Constructor Required
  WCSizeControlHandler();

  // apply function called by world canvas
  // during refresh
  virtual Bool executeSizeControl(WorldCanvas *wc) = 0;

  // Destructor
  virtual ~WCSizeControlHandler();

};


} //# NAMESPACE CASA - END

#endif

