//# DisplayShapeWithCoords.h : Base class for DisplayShapes with Coords
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
#ifndef TRIALDISPLAY_DISPLAYSHAPEWITHCOORDS_H
#define TRIALDISPLAY_DISPLAYSHAPEWITHCOORDS_H

#include <casa/aips.h>
#include <display/DisplayShapes/DisplayShape.h>

namespace casacore{

	class Record;
	template <class T> class Vector;
	template <class T> class Matrix;
}

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for DisplayShapes with Coords
// </summary>

	class PixelCanvas;
	class PanelDisplay;
	class WorldCanvas;


	class DisplayShapeWithCoords {

	public:
		virtual casacore::Record getRawOptions() = 0;

		static void floatToPix(casacore::Record& inHere, const casacore::String& field);
		static void floatPointToPix(casacore::Record& inHere, const casacore::String& field);

		static void matrixFloatToQuant(casacore::Record& inHere, const casacore::String& field,
		                               const casacore::String& unit);
		static void matrixFloatFromQuant(casacore::Record& inHere,
		                                 const casacore::String& field,
		                                 const casacore::String& onlyParseTheseUnits);

		static void floatFromPix(casacore::Record& inHere, const casacore::String& field);
		static void floatPointFromPix(casacore::Record& inHere, const casacore::String& field);

		static WorldCanvas* chooseWCFromWorldPoint(const casacore::Record& settings,
		        PanelDisplay* pd);

		static WorldCanvas* chooseWCFromWorldPoints(const casacore::Record& settings,
		        const casacore::String& fieldWithPoints,
		        PanelDisplay* pd);

		static WorldCanvas* chooseWCFromPixPoint(const casacore::Float& xPos,
		        const casacore::Float& yPos,
		        PanelDisplay* pd);

		static WorldCanvas* chooseWCFromPixPoints(const casacore::Matrix<casacore::Float> points,
		        PanelDisplay* pd);

		static casacore::Vector<casacore::Float> floatPointFromQuant(const casacore::Record &inHere,
		        const casacore::String& field,
		        casacore::String& units);

		static casacore::Vector<casacore::Float> relToScreen(const casacore::Vector<casacore::Float>& rel, PixelCanvas* pc);
		static casacore::Vector<casacore::Float> screenToRel(const casacore::Vector<casacore::Float>& screen,
		                                 PixelCanvas* pc);

		static casacore::Matrix<casacore::Float> relToScreen(const casacore::Matrix<casacore::Float>& rel, PixelCanvas* pc);
		static casacore::Matrix<casacore::Float> screenToRel(const casacore::Matrix<casacore::Float>& screen,
		                                 PixelCanvas* pc);

		static casacore::Bool inWorldCanvasDrawArea(const casacore::Matrix<casacore::Float> points,
		                                  WorldCanvas* wc);

		static void pixelToWorldPoints(casacore::Record& inHere, const casacore::String& fieldname,
		                               WorldCanvas* wc);
		virtual ~DisplayShapeWithCoords() {}

	};

} //# NAMESPACE CASA - END

#endif






