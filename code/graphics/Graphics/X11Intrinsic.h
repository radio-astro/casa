//# X11Intrinsic.h: include file to be used instead of X11/Intrinsic.h
//# Copyright (C) 1996,2000
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

#ifndef GRAPHICS_X11INTRINSIC_H
#define GRAPHICS_X11INTRINSIC_H

// <X11/Intrinsic.h> #defines True, False, Bool, and String. After inclusion
// of <X11/Intrinsic.h>, usage of other portions of AIPS++, e.g. the String
// class, may fail to compile. This file circumvents this name conflict. This
// file should be used in preference to <X11/Intrinsics.h> where there is the
// possibility of a conflict with between the X11 usage of these names
// and any non-X11 usage of these names.


#ifdef String
#undef String
#endif
#define String XWinString
#ifdef Bool
#undef Bool
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif
#include <X11/Intrinsic.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Make sure some definitions ok</summary>
// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="" demos="">
// </reviewed>
// <group name=definitions>

#undef True
#undef False
#undef String
#undef Bool
// </group>


} //# NAMESPACE CASA - END

#endif
