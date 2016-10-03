//# DSMarker.h: Marker implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSMARKER_H
#define TRIALDISPLAY_DSMARKER_H

#include <casa/aips.h>
#include <display/DisplayShapes/DSBasic.h>
#include <display/Display/DisplayEnums.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;
	class DSPCMarker;
	class DSWCMarker;

	template <class T> class DParameterRange;

// <summary>
// Implementation of a marker.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSBasic">DSBasic</linkto>
// <li> <linkto class="Display">Display</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSMarker is a method of managind the drawing of a marker onto a pixel
// canvas.
// </etymology>
//
// <synopsis>
// DSArrow simply extends from DSLine, and adds the management of a polygon
// to it. The polygon is used to represent the arrow head, and various options
// regarding its apppearance can be changed.
//
// There are generally two ways to make DisplayShape(s); To create them in
// "one hit" by providing arguments to the constructor, or by using the
// default constructor and then the "setOptions" method. A simple interface
// for all classes inheriting from the
// <linkto class="DisplayShape">DisplayShape</linkto> class is provided by
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// </synopsis>
//
// <motivation>
// A desire to have markers, similar to those used by PGPlot available as an
// annotation object. This wrapper is designed to make the management of
// drawing markers as primitives easier.
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>


	class DSMarker : public DSBasic {

	public:

		// Constructors and destructor. See
		// <linkto class="Display">Display</linkto> for a list of avaiable
		// markers. Currently, all pixelHeights are changed to an even number,
		// as odd sized markers tend to look funny. The pixelSize corresponds
		// to the height of the marker in pixels.
		// <group>
		DSMarker();
		DSMarker(const casacore::Float& xPos, const casacore::Float& yPos,
		         const Display::Marker& marker = Display::Square,
		         const casacore::uInt pixelSize = 10);
		DSMarker(const casacore::Record& settings);

		DSMarker(const DSPCMarker& other);
		DSMarker(const DSWCMarker& other);
		DSMarker(const DSMarker& other);

		virtual ~DSMarker();
		// </group>

		// Standard DisplayShape functions.
		// <group>
		virtual void move(const casacore::Float& dX, const casacore::Float& dY);
		virtual void setCenter(const casacore::Float& xPos, const casacore::Float& yPos);
		virtual casacore::Vector<casacore::Float> getCenter();
		virtual void scale(const casacore::Float& scaleFactor);
		virtual void draw(PixelCanvas* pc);
		virtual casacore::Bool inObject(const casacore::Float& xPos, const casacore::Float& yPos);
		// </group>

		virtual void setSize(const casacore::uInt newSize);

		// Get and set options.
		// <group>
		virtual casacore::Bool setOptions(const casacore::Record& settings);
		virtual casacore::Record getOptions();
		// </group>

		// Overload, since we never want to click on a marker (Well actually
		// we might, but as yet we have no use, and sometimes people think that
		// have clicked on a marker when actually they are on a handle. This
		// makes it appear as though the marker isn't moving.
		virtual casacore::Bool whichHandle(const casacore::Float& /*xPos*/, const casacore::Float& /*yPos*/,
		                         casacore::Int& /*active*/) {
			return false;
		}

		// These functions do not apply for DSMarkers. They are here to ensure
		// no strange behavior
		// <group>
		virtual void rotate(const casacore::Float& /*angle*/) {};
		virtual void changePoint(const casacore::Vector<casacore::Float>& /*newPos*/) {};
		virtual void changePoint(const casacore::Vector<casacore::Float>& /*newPoint*/,
		                         const casacore::Int /*nPoint*/) {};
		// </group>

		//  virtual void recalculateScreenPosition();

	private:
		// Set the default options. Called by default constructor.
		virtual void setDefaultOptions();

		// Update location of single handle
		void updateHandle();

		casacore::Vector<casacore::Float> itsCenter;
		casacore::Matrix<casacore::Float> itsHandle;
		casacore::Bool itsBuiltHandle;
		DParameterRange<casacore::Int>* itsSize;

		Display::Marker itsMarkerStyle;
		casacore::Bool itsValid;

	};

} //# NAMESPACE CASA - END

#endif

