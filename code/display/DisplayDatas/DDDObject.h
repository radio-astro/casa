//# DDDObject.h: base object interface for DrawingDisplayData
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

#ifndef TRIALDISPLAY_DDDOBJECT_H
#define TRIALDISPLAY_DDDOBJECT_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Matrix.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayDatas/DDDHandle.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;
	class WCPositionEvent;
	class DrawingDisplayData;


// <summary>
// Base class describing interface for objects supported by DrawingDisplayData
// </summary>

// <synopsis>
// This class defines the interface which objects that are drawable
// by the DrawingDisplayData class must provide.
//
// World units of 'pix' and 'frac' are defined.  'pix' units are in
// screen pixel units.  They are not very useful once the display
// has been zoomed.  'frac' units have range [0,0] -> [1,1]
// mapping to the display part of the pixel canvas.
// </synopsis>

	class DDDObject : public WCRefreshEH, public WCMotionEH,
		public WCPositionEH {

	public:

		enum Mode {
		    None,
		    Handle,
		    Move,
		    Rotate
		};

		// Constructor taking a Record description.  Fields in the record
		// are: <src>color</src> and <src>label</src>.
		DDDObject(const Record &description, DrawingDisplayData *owner);

		// Destructor.
		virtual ~DDDObject();

		// Draw this DrawingDisplayData object for the given reason on the
		// provided WorldCanvas.
		virtual void draw(const Display::RefreshReason &reason,
		                  WorldCanvas *worldcanvas) = 0;

		// Indicate whether the object should show its handles or not.  The
		// parent DrawingDisplayData will control this, and either ask all
		// DDDObjects to show their handles or not, via the user setting an
		// option.  This actually controls the state of whether this
		// DDDObject is editable, so the parent DrawingDisplayData could
		// also allow editing of only one DDDObject at a time.
		virtual void showHandles(const Bool show, const Bool tellOwner = True);

		// Query whether the object is showing its handles.
		virtual Bool showingHandles() {
			return itsShowHandles;
		}

		// Return a record describing this object.
		virtual Record description();

		// Update this object based on the information in the provided
		// Record.
		virtual void setDescription(const Record &rec);

		// Store a click in the buffer and look for a double-click event.
		// If one is found, then call the doubleClick function in the
		// owning DisplayData.  Returns <src>True</src> if a double click
		// was detected.
		virtual Bool storeClick(const DisplayEvent &ev);

		// Clear the click buffer.
		virtual void clearClickBuffer();

		// Return the unique id of this object.
		Int objectID() const {
			return itsObjectID;
		}

		// Event handlers.  The parent DrawingDisplayData will distribute
		// events as necessary to the various DDDObjects which comprise it.
		// <group>
		virtual void operator()(const WCRefreshEvent &ev) = 0;
		virtual void operator()(const WCPositionEvent &ev) = 0;
		virtual void operator()(const WCMotionEvent &ev) = 0;
		// </group>

	protected:

		// Return the owner of this object.
		DrawingDisplayData *owner() {
			return itsOwner;
		}

		// Return the color to use to draw this object.
		String color() const {
			return itsColor;
		}

		// Return the label of this object.
		String label() const {
			return itsLabel;
		}

		// Return the line width of this object.
		Int lineWidth() const {
			return itsLineWidth;
		}

		Bool isEditable() const {
			return itsEditable;
		}

		Bool isMovable() const {
			return itsMovable;
		}

		Bool isFixed() const {
			return !itsMovable && !itsEditable;
		}

		// (Required) default constructor.
		DDDObject();

		// (Required) copy constructor.
		DDDObject(const DDDObject &other);

		// (Required) copy assignment.
		void operator=(const DDDObject &other);

		// Translate Matrix
		void translateMatrix(Matrix<Double>& points, Double dx, Double dy);

		// Rotate Matrix
		Matrix<Double> rotateMatrix(const Matrix<Double>& points, Double angle);

		// Is point inside the polygon
		// <group>
		Bool inPolygon(const Matrix<Double>& points, Double x, Double y);
		Bool inPolygon(const Vector<Double>& xP, const Vector<Double>& yP,
		               Double x, Double y);
		// </group>

		// Convert DisplayCoordinateSystem to screen pixels
		void convertCoordinateSystem (DisplayCoordinateSystem& cSys, WorldCanvas* wcPtr) const;

		// Is the point on a handle
		Bool onHandle(const Block<DDDHandle>& handles,
		              Double x, Double y);

	private:

		// DrawingDisplayData which owns this DDDObject.
		DrawingDisplayData *itsOwner;

		// Whether the handles are showing, and therefore whether this
		// DDDObject is presently editable.
		Bool itsShowHandles;

		// Is this object editable
		Bool itsEditable;
		// Is it movable
		Bool itsMovable;

		// Timing for double clicks.
		Double itsLastClickTime, its2ndLastClickTime;

		// Color of this object.
		String itsColor;

		// Line width of the lines drawn
		Int itsLineWidth;

		// Label for this object.
		String itsLabel;

		// Unique identification for this object.
		Int itsObjectID;

	};


} //# NAMESPACE CASA - END

#endif
