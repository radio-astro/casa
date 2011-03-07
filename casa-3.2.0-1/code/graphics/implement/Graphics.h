//# Graphics.h:  classes for the visual display of data
//# Copyright (C) 1995
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

#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

namespace casa { //# NAMESPACE CASA - BEGIN

// <module>
//
// <summary>
// Classes for the visual display of data.
// </summary>

// <prerequisite>
//   <li> none yet appropriate
// </prerequisite>
//

// <reviewed reviewer="Paul Shannon" date="1995/06/02" demos="">
// </reviewed>

// <etymology>
// Nothing special.
// </etymology>
//
// <synopsis>
// This module is (as of June 1995) a work in progress.  It contains a PGPLOT
// wrapper and a test program, and nothing else.  
//
// <note role=caution>
// This module header file does not yet play the traditional role -- of
// allowing the application programmer to gain a lot of capability with
// only one <em> include </em> statement.
// </note>

// </synopsis>

// <example>
// See tVisualize.cc.
// </example>
//
// <motivation>
// The benefits of graphical display of data are many, and well-known...
// </motivation>

// <todo asof="1995/06/02">
//   <li> Replace (or improve) the PGPLOT wrapper.
//   <li> Once the (device-independent) Plot1d interface settles down,
//        move it from the trial package to here.
//   <li> We may wish to create subdirectories of the Graphics directory,
//        each containing window-system-specific implementations of 
//        the interfaces defined in the directory.
//        Site makedef's could trigger automatic linking against
//        the appropriate window-system-specific library.  
// </todo>

// </module>


} //# NAMESPACE CASA - END

#endif
