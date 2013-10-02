//# NBody.h: class which provides brute force n-body simulation and display
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_NBODY_H
#define TRIALDISPLAY_NBODY_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Containers/Record.h>
#include <display/DisplayEvents/WCPositionEvent.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayDatas/DisplayData.h>
#include <casa/Containers/List.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;
	class WCResampleHandler;
	class WorldCanvasHolder;
	class AttributeBuffer;
	class Attribute;
	class Unit;
	class IPosition;

// <summary>
// Simple class which provides brute force n-body simulation and display.
// </summary>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>

// This class provides a simple object which can (by brute force)
// calculated the internal gravitational forces acting on a
// self-gravitating set of bodies.
// </synopsis>

	class NBody : public DisplayData {

	public:

		// Constructor.
		NBody();

		// Destructor.
		virtual ~NBody();

		// Coordinate transformations, called by the WorldCanvasHolder.
		// <group>
		virtual Bool linToWorld(Vector<Double>& world, const Vector<Double>& lin);
		virtual Bool worldToLin(Vector<Double>& lin, const Vector<Double>& world);
		// </group>

		// irrelevant in this part of the tree
		virtual void setActiveImage(uInt /*zindex*/) {
			return;
		};

		// Routines which yield information on the axis names and units, and
		// some miscellaneous information.
		// <group>
		virtual Vector<String> worldAxisNames() const;
		virtual Vector<String> worldAxisUnits() const;
		virtual const Unit dataUnit() ;
		virtual const RecordInterface& miscInfo() ;
		// </group>

		// Return the number of elements in this DisplayData.
		virtual const uInt nelements(const WorldCanvasHolder& wcHolder) const;
		virtual const uInt nelements() const;

		// Add a restriction for item <src>itemNum</src> of this
		// DisplayData.
		// <group>
		virtual void addElementRestrictions(const uInt itemNum,
		                                    AttributeBuffer& other);
		virtual void addElementRestriction(const uInt itemNum,
		                                   Attribute& newRestriction,
		                                   Bool permanent);
		// </group>

		// Set a restriction for item <src>itemNum</src> of this
		// DisplayData.
		// <group>
		virtual void setElementRestrictions(const uInt itemNum,
		                                    AttributeBuffer& other);
		virtual void setElementRestriction(const uInt itemNum,
		                                   Attribute& newRestriction);
		// </group>

		// Remove a restriction from item <src>itemNum</src>.
		virtual void removeElementRestriction(const uInt itemNum,
		                                      const String& name);

		// Clear all restrictions of item <src>itemNum</src> (except the
		// ones that are permanent of course).
		virtual void clearElementRestrictions(const uInt itemNum);

		// Check if a restriction for item <src>itemNum</src> with name
		// <src>name</src> exists.
		virtual Bool existElementRestriction(const uInt itemNum,
		                                     const String& name);

		// Get a handle to the buffer of restrictions for item
		// <src>itemNum</src>.  Throws an exception of type AipsError if
		// <src>itemNum</src> is out of range.
		virtual AttributeBuffer *elementRestrictionBuffer(const uInt itemNum);

		// Report the minimum and maximum data values for this DisplayData.
		// <group>
		virtual Double getDataMin();
		virtual Double getDataMax();
		// </group>

		// sizeControlFunction, called by the WorldCanvasHolder.
		virtual Bool sizeControl(WorldCanvasHolder& wcHolder,
		                         AttributeBuffer& holderBuf);

		// Position event handler, called by the WorldCanvasHolder.
		virtual void positionEH(const WCPositionEvent& ev);

		// Motion event handler, called by the WorldCanvasHolder.
		virtual void motionEH(const WCMotionEvent& ev);

		// Refresh event handler, called by the WorldCanvasHolder.
		virtual void refreshEH(const WCRefreshEvent& ev);

		// Clean up, ie. remove cached images etc.
		virtual void cleanup();

		// required functions with null implementations
		virtual String showPosition(const Vector<Double> &, const Bool &) {
			return String("");
		}
		virtual String showValue(const Vector<Double> &) {
			return String("");
		}
		virtual Display::DisplayDataType classType() {
			return Display::Vector;
		}

	private:

		void setupStars();
		void cleanupStars();
		void drawImage(WorldCanvas& wCanvas);
		void setScale();

		// General restrictions  (already declared in DisplayData.h)
		//  AttributeBuffer restrictions;
		//True if nothing should be drawn by the NBody
		Bool turnedOff;

		// The min and max to use for drawing
		Double dataMin;
		Double dataMax;

		Double itsTimeStep;
		Double itsDampingFactor;
		Int itsNumSteps;
		Int itsXSize;
		Int itsYSize;

		// internal bookkeeping:
		// buffer for stroing sizecontrol done by this NBody
		AttributeBuffer sizeControlBuf;


		// Check if this NBody did the sizeControl
		Bool iDidSizeControl(WorldCanvas& wCanvas);

		void drawMovie(WorldCanvas& wCanvas, Int numSteps, Double timeStep,
		               Double dampingFactor);

		Record miscInfoDummy;

		List<void *> itsGalaxyList;
		ListIter<void *> *itsGalaxyListIter;

	};


} //# NAMESPACE CASA - END

#endif
