//# X11Util.h: X utilities to assist the X11 classes
//# Copyright (C) 1999,2000,2001
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

#ifndef GRAPHICS_X11UTIL_H
#define GRAPHICS_X11UTIL_H


#include <casa/aips.h>
#include <casa/iosfwd.h>

#include <graphics/X11/X_enter.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Xmu/Xmu.h>
#include <X11/Xmu/StdCmap.h>
#include <graphics/X11/X_exit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// X Utilities to assist the X11 classes
// </summary>
//
// <use visibility=export>
//
// <reviewed>
// nope
// </reviewed>
//  
// <prerequisite>
// <li> Knowledge of X internals
// </prerequisite>
//
// <etymology>
// Name subject to change
// </etymology>
//
// <synopsis>
//
//
//
// </synopsis>
//
// <motivation>
// Offload some complex functions to utility library, making it a
// bit easier to manage the X11 code.
// </motivation>
//
// <group name="X11 Utility Functions">

// returns true if nColors can be allocated
// on the colormap of the display using or not using
// a contiguous color method.
Bool X11TestColormapAlloc(::XDisplay * display,
			  XColormap colormap,
			  Bool contig,
			  uInt nColors);

// returns the number of colors presently
// available on the passed colormap, and display,
// possibly using contiguous mapping.
uInt X11QueryColorsAvailable(::XDisplay * display,
			     XColormap colormap,
			     Bool contig);

// returns a default display name
// First, search for the DISPLAY environment
// Else it returns ":0"
const char * X11DefaultDisplayName();

// returns an opened display on the default name as above
::XDisplay * X11DefaultDisplay();

// returns a default screen opened on the default display
Screen * X11DefaultScreen();

// function returns best visual in given class.  Best is determined
// by largest depth
Visual * X11BestVisual(Screen * screen, uInt c_class);

// returns depth of given visual.  The safest way to get the right
// depth is to match it with XGetVisualInfo and return its depth
uInt X11DepthOfVisual(::XDisplay * display, Visual * visual);

// maps a visualId to a visual
Visual * X11VisualIdToVisual(::XDisplay * display, uInt visualId);

// gets XVisualInfo for the given Visual
XVisualInfo X11VisualInfoFromVisual(::XDisplay *display, Visual *visual);

// print info on a standard colormap
ostream & operator << (ostream & os, const XStandardColormap & mapInfo);
      
// print visual information
ostream & operator << (ostream & os, Visual * visual);

// print screen information
ostream & operator << (ostream & os, Screen * screen);

// Initialize a Standard Colormap.  If the function succeeds,
// The passed mapInfo structure will be filled with information
// about the color cube.
Bool X11InitializeStandardColormap(Screen * screen,
				   Visual * visual,
				   Atom property,
				   XStandardColormap ** mapInfo);

// Return the widget that is the top-level widget of this widget
Widget X11TopLevelWidget(Widget w);

// Return the shell widget that contains this widget
Widget X11ContainingShell(Widget w);
// </group>
// function to print colormap information
// ostream & operator << (ostream & os, Colormap cmap)

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <graphics/X11/X11Util.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
