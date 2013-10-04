//# DSPixelArrow.h : Implementation of an absolute pixel DSArrow
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id:

#ifndef TRIALDISPLAY_DSPIXELARROW_H
#define TRIALDISPLAY_DSPIXELARROW_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <display/DisplayShapes/DSArrow.h>
#include <display/DisplayShapes/DisplayShapeWithCoords.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of an absolute pixel DSArrow
// </summary>

	class PanelDisplay;
	class DSWorldArrow;
	class DSScreenArrow;

	class DSPixelArrow : public DSArrow , public DisplayShapeWithCoords {

	public:

		DSPixelArrow();
		DSPixelArrow(const Record& settings);
		DSPixelArrow(DSScreenArrow& other);
		DSPixelArrow(DSWorldArrow& other);

		virtual ~DSPixelArrow();

		virtual Bool setOptions(const Record& settings);
		virtual Record getOptions();

		virtual Record getRawOptions() {
			return DSArrow::getOptions();
		}

	private:

	};


} //# NAMESPACE CASA - END

#endif
