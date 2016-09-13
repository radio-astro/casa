//# WCCoordinateHandler.h: interface for coordinate handling on WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000
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

#ifndef TRIALDISPLAY_WCCOORDINATEHANDLER_H
#define TRIALDISPLAY_WCCOORDINATEHANDLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T> class Matrix;

// <summary>
// Base class defining a set of transformations for WorldCanvas coordinates.
// </summary>
//
// <prerequisite>
// <li> Understanding of non-linear World Coordinate Systems
// </prerequisite>
//
// <etymology>
// WCCoordinateHandler : WorldCanvas Coordinate Handler
// </etymology>
//
// <synopsis>
// The WCCoordinateHandler class implements the concept of a coordinate
// transformation pair that can be plugged in to the
// <linkto class="WorldCanvas">WorldCanvas</linkto>.  It is designed for
// derivation by the programmer and can thereby be arbitrarily implemented.
//
// This class defines an interface between the WorldCanvas, which uses
// coordinate transformations for various reasons, and the derived class
// that implements these coordinate transformations.
//
// The designer of the derived class must write the Vector&ltDouble&gt versions
// of linToWorld and worldToLin as described in detail below.  It is also
// recommended that the Matrix&ltDouble&gt versions of linToWorld and worldToLin
// be written to maximize efficiency.
// </synopsis>
//
// <motivation>
// <ul>
// <li>Needed to plug in a user-programmable coordinate transformation functions into
// the world canvas in a convenient way.
// <li> Wanted to bundle these transformations
// into a class to follow OO methods.
// </motivation>
//
// <example>
// <linkto class="DefaultWCCoordinateHandler">DefaultWCCoordinateHandler</linkto>
// </example>
//

	class WCCoordinateHandler {

	public:

		// Default Constructor Required
		WCCoordinateHandler();

		// transform the lin point (2-dimensional) into a world coordinate
		// (N-dimensional), returning False if the coordinate could not be
		// transformed.  Derived classes should override.
		virtual Bool linToWorld(Vector<Double> & world,
		                        const Vector<Double> & lin) = 0;

		// Batch transformation where each row of lin is a coordinate that
		// is transformed into each row of world.  On input, if the i'th
		// position in the failures vector is true, the i'th transformation
		// is not attempted.  If the j'th transformation fails, the j'th
		// position in the failures vector will be set.  The return value is
		// True only when the failures vector has no position set to True;
		// This function is implemented in this base class by making
		// repeated calls to the Vector<Double> version of linToWorld.
		virtual Bool linToWorld(Matrix<Double> & world,
		                        Vector<Bool> & failures,
		                        const Matrix<Double> & lin);

		// transform the world point (N-dimensional) into a lin coordinate
		// (2-dimensional), returning False if the coordinate could not be
		// transformed.  Derived classes should override.
		virtual Bool worldToLin(Vector<Double> & lin,
		                        const Vector<Double> & world) = 0;

		// Batch transformation where each row of world is a coordinate that
		// is transformed into each row of lin.  On input, if the i'th
		// position in the failures vector is true, the i'th transformation
		// is not attempted.  If the j'th transformation fails, the j'th
		// position in the failures vector will be set.  The return value is
		// True only when the failures vector has no position set to True;
		// This function is implemented in this base class by making
		// repeated calls to the Vector<Double> version of worldToLin.
		virtual Bool worldToLin(Matrix<Double> & lin,
		                        Vector<Bool> & failures,
		                        const Matrix<Double> & world);

		virtual std::string errorMessage( ) const = 0;

		// Return the number of axes in the world coordinates
		virtual uInt nWorldAxes() const = 0;

		// Routines that give the axes names and the unit.
		// <group>
		/* virtual Vector<String> worldAxisNames() = 0; */
		/* virtual Vector<String> worldAxisUnits() = 0; */
		// </group>

		// Destructor
		virtual ~WCCoordinateHandler();

	};


} //# NAMESPACE CASA - END

#endif
