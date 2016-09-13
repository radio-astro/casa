//# DDDEllipse.h: declaration of ellipse DDDObject
//# Copyright (C) 1999,2000,2001,2002
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


#ifndef TRIALDISPLAY_DDDELLIPSE_H
#define TRIALDISPLAY_DDDELLIPSE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/DisplayDatas/DDDObject.h>
#include <display/DisplayDatas/DDDHandle.h>

namespace casacore{

	class LogIO;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class DisplayEvent;

// <summary>
// Implementation of an ellipse/rectangle object for DrawingDisplayData class.
// </summary>

// <synopsis>
// This class implements the interface defined by DDDObject, to provide
// an ellipse or rectangle object for registration with DrawingDisplayData objects.
// The widths are all full widths.
// </synopsis>
//
// <todo>
// Create a drawEllipse function on the world canvas and move
// intelligence from here to there.
// </todo>
//

	class DDDEllipse : public DDDObject {

	public:

		// Constructor taking a casacore::Record description.  Fields in the record,
		// on top of what is consumed by the DDDObject constructor, are:
		// Required : <src>center</src> (vector of quanta length 2),
		// <src>major, minor, positionangle</src> (all quanta).
		//
		// Units 'frac' ([0->1]) are available for the center only.
		// Units 'pix' (absolute 0-rel image pixels) are available for the
		// major and minor only.
		//
		// Other, not required fields are <src>outline & editable</src>
		// which default to <src>true</src>.  When <src>editable=false</src>
		// the object can be moved but not reshaped, when <src>true</src>
		// it can be moved and reshaped.   When   <src>outline=false</src>
		// the ellipse is filled.   If the field <src>rectangle=true</src>
		// a rectangle is drawn. If it does not exist or is false, an ellipse
		// is drawn.
		//
		// Finally, field <src>doreference</src> which defaults to <src>false</src>
		// is used to control the location at which conversions to and from
		// pixel coordinates are done.  If <src>true</src> then all conversions
		// are done at the reference value. Otherwise, the conversions are
		// done at the actual location of the shape.  For example, with
		// a casacore::DirectionCoordinate plane, setting <src>doreference=true</src>
		// will cause the shape to rotate as you move it about the display
		// as the position angle tracks the local North.
		DDDEllipse(const casacore::Record &description, DrawingDisplayData *owner);

		// Destructor.
		virtual ~DDDEllipse();

		// Draw this object for the given reason on the provided
		// WorldCanvas.
		virtual void draw(const Display::RefreshReason &reason,
		                  WorldCanvas *worldcanvas);

		// Return a record describing this object.  Presently returns
		// only construction description.
		virtual casacore::Record description();

		// Update this object based on the information in the provided
		// Record.
		virtual void setDescription(const casacore::Record &rec);

		// Event handlers.  The parent DrawingDisplayData will distribute
		// events as necessary to the various DDDObjects which comprise it.
		// <group>
		virtual void operator()(const WCRefreshEvent &ev);
		virtual void operator()(const WCPositionEvent &ev);
		virtual void operator()(const WCMotionEvent &ev);
		// </group>

	protected:

		// (Required) default constructor.
		DDDEllipse();

		// (Required) copy constructor.
		DDDEllipse(const DDDEllipse &other);

		// (Required) copy assignment.
		void operator=(const DDDEllipse &other);

	private:

// Fill style
		casacore::Bool itsOutline;

// Define center x, center y, major, minor, pa in world
		casacore::Vector<casacore::Quantum<casacore::Double> > itsWorldParameters;

// Define center x, center y, major, minor, pa (radians) in screen pixel
		casacore::Vector<casacore::Double> itsPixelCenter, itsPixelShape;

// Rotated Rectangle Corners to draw in screen pixel
		casacore::Matrix<casacore::Double> itsCorners;
		casacore::Matrix<casacore::Double> itsPixelOffsets;

// casacore::List of handles

		casacore::Block<DDDHandle> itsHandles;

// Mode.

		DDDObject::Mode itsMode;

// Store for movement bases.

		casacore::Double itsBaseMoveX, itsBaseMoveY;

// Logger

		casacore::LogIO itsLogger;

// DisplayCoordinateSystem in screen pixel coordinates
		DisplayCoordinateSystem itsCoordinateSystem;

// pointer to world canvas (shallow copy)
		WorldCanvas* itsWorldCanvasPtr;

// are we drawing a rectangle or an ellipse ?
		casacore::Bool itsRectangle;

// Fractional indicators
		casacore::Bool itsFracX, itsFracY;

// Do world/pixel conversions at reference location ?
		casacore::Bool itsDoRef;

// COnvert parameters from world to pixel
		void convertToPixel ();

// Update private world parameters from current pixel values
		void updateWorldValues ();

// Decode record into private data

		void decode(const casacore::RecordInterface& description, casacore::Bool required);

// Encode private data into record
		void encode(casacore::RecordInterface& description) const;

// compute corners of rectangle
		void createCorners();

		casacore::Matrix<casacore::Int> toIntPixel(const casacore::Matrix<casacore::Double>& points);

// Debugging routines

		void listWorld();
		void listPixel();

	};


} //# NAMESPACE CASA - END

#endif
