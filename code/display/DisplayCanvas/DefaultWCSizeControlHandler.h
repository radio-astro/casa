//# DefaultWCSizeControlHandler.h: default size control handling for WC
//# Copyright (C) 1993,1994,1995,1996,2000,2001
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

#ifndef TRIALDISPLAY_DEFAULTWCSIZECONTROLHANDLER_H
#define TRIALDISPLAY_DEFAULTWCSIZECONTROLHANDLER_H

#include <casa/aips.h>
#include <display/DisplayCanvas/WCSizeControlHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Default WorldCanvas SizeControl handler
// </summary>
//
// <etymology>
// DefaultWCSizeControlHandler - Default WorldCanvas Size Control Handler.
// </etymology>
//
// <synopsis>
// This class implements default functionality for the size control handler
// that the WorldCanvas creates for itself when the WorldCanvas constructor
// is called.
//
// Presently the default size control handler does nothing, relying on the
// <linkto class="WCResampleHandler">WCResampleHandler</linkto>
// to fit output images to the screen pixel array when images are supposed
// to be drawn.
// </synopsis>
//
// <motivation>
// Wanted to have default functionality for this to make it easier on the user
// of this class and to avoid having to check all the time in the WorldCanvas
// to see if a size control handler has been registered.
// </motivation>
//
// <example>
// none available yet.
// </example>
//
// <todo>
// <li> provide example
// <li> consider restricting interface to accepting and modifying
// a rectangle representing the largest available plotting size.
// </todo>
//

	class DefaultWCSizeControlHandler  : public WCSizeControlHandler {

	public:

		// Default Constructor Required
		DefaultWCSizeControlHandler();

		// apply modifies the world canvas parameters through the passed pointer.
		virtual Bool executeSizeControl(WorldCanvas * wc);

		// Destructor
		virtual ~DefaultWCSizeControlHandler();

	};


} //# NAMESPACE CASA - END

#endif


