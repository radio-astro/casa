//# WCCSNLAxisLabeller.h: non-linear axis labelling using a CS on a WC
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

#ifndef TRIALDISPLAY_WCCSNLAXISLABELLER_H
#define TRIALDISPLAY_WCCSNLAXISLABELLER_H

#include <casa/aips.h>
#include <display/Display/AttributeBuffer.h>
#include <display/DisplayCanvas/WCCSAxisLabeller.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class which provides non-linear axis labelling using a DisplayCoordinateSystem.
// </summary>
//
// <synopsis>
// This class implements the interface defined in WCCSAxisLabeller to
// provide arbitrary non-linear axis labelling, using coordinate
// transformations provided by a supplied DisplayCoordinateSystem.
//
// </synopsis>

	class WCCSNLAxisLabeller : public WCCSAxisLabeller {

	public:

		// Constructor
		WCCSNLAxisLabeller();

		// Destructor
		virtual ~WCCSNLAxisLabeller();

		// Draw axis labels for the provided WorldCanvas refresh event.
		// Note: it is the responsibility of the caller to assure that this
		// object is invalidated (or destroyed) _before_ the WorldCanvas
		// corresponding to ev gets destroyed, so that any cached axis drawing
		// which the WC is holding for us can be deleted properly.
		virtual Bool draw(const WCRefreshEvent &ev);

		// Invalidate/return any cached drawing list
		virtual void invalidate();

		// Install the DisplayCoordinateSystem to use
		virtual void setCoordinateSystem(const DisplayCoordinateSystem &coordsys);

		// Set the world min and max ranges (and their associated units)
		// to be used by the DisplayCoordinateSystem::toMix computations.
		// If not specified with this function,
		// -180->180, -90->90 for DirectionCoordinates used (rest
		// don't matter).  Specifying this correctly can make the
		// toMix computations much faster (it uses an iterative
		// solution) and avoid solution ambiguities.
		void setWorldRanges (const Vector<Double>& worldMin,
		                     const Vector<Double>& worldMax,
		                     const Vector<String>& units);

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in rec to the DisplayData; return value
		// True means a refresh is needed.  Any fields added to the
		// updatedOptions argument are options which have changed in
		// some way due to the setting of other options - ie. they
		// are context sensitive.
		virtual Bool setOptions(const Record &rec, Record &updatedOptions);

		// retrieve the current and default options and parameter types.
		virtual Record getOptions() const;

	private:

		// Is the drawlist state below valid?
		Bool itsValid;

		// our drawlist number
		uInt itsDrawListNumber;

		// attribute buffer that suits the drawlist we have made
		AttributeBuffer itsDrawStateBuffer;

		// last WorldCanvas drawn upon
		WorldCanvas *itsLastWorldCanvas;
	};


} //# NAMESPACE CASA - END

#endif
