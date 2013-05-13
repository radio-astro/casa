//# PCVGBuffer.h: buffering vector graphics for the PixelCanvases
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
//#$Id$

#ifndef TRIALDISPLAY_PCVGBUFFER_H
#define TRIALDISPLAY_PCVGBUFFER_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

// <summary>
//
// </summary>
//
// <use visibility=export>
//
// <reviewed>
// </reviewed>
//
// <prerequisite>
// <li> none
// </prerequisite>
//
// <etymology>
// The name of PCVGBuffer comes from ...
// </etymology>
//
// <synopsis>
// Display Class File
// </synopsis>
//
// <motivation>
// </motivation>
//
// <example>
// none available.
// </example>
//
// <todo>
// <li> write and test!
// </todo>
//

	class PCVGBuffer {

	public:

		enum AccumMode { AMNone,
		                 AMPoints,
		                 AMLines,
		                 AMPolyline,
		                 AMPolygon
		               };

		// Default Constructor Required
		PCVGBuffer(PixelCanvas * pc, uInt nVerts = 1024);

		// User Constructor
		PCVGBuffer();

		// add point
		void accumPoint(Float x, Float y);

		// add line
		void accumLine(Float x1, Float y1, Float x2, Float y2);

		// add point to polyline
		void accumPolylinePoint(Float x, Float y);

		// add point to polygon
		void accumPolygonPoint(Float x, Float y);

		// flush the buffer
		void flush();

		// current mode
		AccumMode mode() const {
			return mode_;
		}

		// change modes
		void setMode(AccumMode m);

		// check size and make more if needed
		void checkSpace();

		// Destructor
		virtual ~PCVGBuffer();

	private:

		PixelCanvas * pc_;

		AccumMode mode_;
		uInt c_;
		Matrix<Float> verts_;
		uInt blockSize_;

	};


} //# NAMESPACE CASA - END

#endif
