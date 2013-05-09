//# PCTestPattern.h: PixelCanvas test pattern display refresh event handler
//# Copyright (C) 2000
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

#ifndef TRIALDISPLAY_PCTESTPATTERN_H
#define TRIALDISPLAY_PCTESTPATTERN_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCRefreshEH.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// PixelCanvas test pattern display refresh event handler.
// </summary>
//
// <etymology>
// "PCTestPattern" refers to a PixelCanvas Test Pattern.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <motivation>
// In a number of places, notably test programs, it is useful to
// be able to see a test pattern on the PixelCanvas which displays
// currently registered Colormaps for Indexed ColorTables, or
// views of the color cube for RGB / HSV ColorTables.
// </motivation>
//
// <example>
// </example>
//
// <todo>
// </todo>

	class PCTestPattern : public PCRefreshEH {

	public:

		// Default (and only) constructor.
		PCTestPattern();

		// Destructor.
		~PCTestPattern();

		// Refresh event handling operator.  This implementation draws
		// the test pattern/s on the PixelCanvas stored in <src>ev</src>.
		void operator()(const PCRefreshEvent &ev);

	protected:

		// (Required) copy constructor.
		PCTestPattern(const PCTestPattern &other);

		// (Required) copy assignment.
		PCTestPattern &operator=(const PCTestPattern &other);

	private:

		Bool itsFirstTime;
		uInt itsWidth, itsHeight;
		uInt itsListLength;
		uInt *itsImList;

	};


} //# NAMESPACE CASA - END

#endif


