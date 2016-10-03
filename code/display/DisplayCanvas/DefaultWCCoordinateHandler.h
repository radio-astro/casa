//# DefaultWCCoordinateHandler.h: default coordinate handling for WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_DEFAULTWCCOORDINATEHANDLER_H
#define TRIALDISPLAY_DEFAULTWCCOORDINATEHANDLER_H

#include <casa/aips.h>
#include <display/DisplayCanvas/WCCoordinateHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class to provide default coordinate handling for WorldCanvases.
// </summary>
//
// <prerequisite>
// <li> <linkto class="WCCoordinateHandler">WCCoordinateHandler</linkto>
// <li> <linkto class="casacore::CoordinateSystem">casacore::CoordinateSystem</linkto>
// </prerequisite>
//
// <etymology>
// DefaultWCCoordinateHandler stands for Default WorldCanvas casacore::Coordinate Handler
// </etymology>
//
// <synopsis>
// Implements identity transformation function pairs between World Coordinates
// and linear coordinates. It assumes 2 axes only.
// </synopsis>
//
// <motivation>
// Recognized that 95% of the time the WorldCanvas will be used in
// conjunction with a world coordinate system of some kind.  So it makes
// more sense to assume that there is always a coordinate handler available
// and use it rather than to have to check to see if there is a handler
// registered with the WorldCanvas or not each time you have to do a
// transformation.
//
// The WorldCanvas creates this default handler for its own purpose.
// </motivation>
//

	class DefaultWCCoordinateHandler : public WCCoordinateHandler {

	public:

		// Default Constructor Required
		DefaultWCCoordinateHandler();

		// just copy lin to world and vice versa.  The input vectors must be of
		// length 2. The output vectors are resized as needed.  The output matrices
		// must be the correct shape on input.
		// <group>
		virtual casacore::Bool linToWorld(casacore::Vector<casacore::Double> & world, const casacore::Vector<casacore::Double> & lin);
		virtual casacore::Bool linToWorld(casacore::Matrix<casacore::Double> & world,
		                        casacore::Vector<casacore::Bool> & failures,
		                        const casacore::Matrix<casacore::Double> & lin);
		virtual casacore::Bool worldToLin(casacore::Vector<casacore::Double> & lin, const casacore::Vector<casacore::Double> & world);
		virtual casacore::Bool worldToLin(casacore::Matrix<casacore::Double> & lin,
		                        casacore::Vector<casacore::Bool> & failures,
		                        const casacore::Matrix<casacore::Double> & world);
		// </group>

		// return the number of world axes. Always 2.
		virtual casacore::uInt nWorldAxes() const {
			return 2;
		}

		// Routines that give the axes names and the units. Both return "Pixel".
		// <group>
		virtual casacore::Vector<casacore::String> worldAxisNames() const;
		virtual casacore::Vector<casacore::String> worldAxisUnits() const;
		// </group>

		std::string errorMessage( ) const { return ""; }

		// Destructor
		virtual ~DefaultWCCoordinateHandler();

	};


} //# NAMESPACE CASA - END

#endif


