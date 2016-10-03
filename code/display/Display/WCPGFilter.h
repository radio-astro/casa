//# WCPGFilter.h: interface between WorldCanvas and its PGPLOT driver
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_WCPGFILTER_H
#define TRIALDISPLAY_WCPGFILTER_H

#include <casa/aips.h>

namespace casacore{

	template <class T> class Matrix;
	template <class T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;

// <summary>
// Interface between the WorldCanvas and its PGPLOT driver.
// </summary>

// <use visibility=export>

// <reviewed>
// </reviewed>

// <prerequisite>
// <li> none
// </prerequisite>

// <etymology>
// "WCPGFilter" is a contraction and concatenation of "WorldCanvas,"
// "PGPLOT" and "Filter", and is a simple filtering interface between
// the WorldCanvas and its PGPLOT driver.
// </etymology>

// <synopsis>
// </synopsis>

// <motivation>
// </motivation>

// <example>
// </example>

// <todo>
// </todo>

	class WCPGFilter {

	public:

		// Constructor taking a pointer to a WorldCanvas.
		WCPGFilter(WorldCanvas *wc);

		// Destructor.
		virtual ~WCPGFilter();

		// Reference counting: ref, unref and refCount.
		// <group>
		virtual void ref() {
			itsRefCount++;
		}
		virtual void unref() {
			itsRefCount--;
		}
		virtual casacore::Int refCount() const {
			return itsRefCount;
		}
		// </group>

		// Return the PGPLOT device id.
		virtual casacore::Int pgid() const {
			return casacore::Int(itsPgPlotID);
		}

		// Realign the PGPLOT domain to the WorldCanvas domain.  If linear
		// is false, then the pgplot canvas is setup to match the world
		// coordinates, but of course is only valid where the world
		// coordinates are linear to first approximation.
		casacore::Bool realign(const casacore::Bool &linear = true);

		// Contour using pgcont function.
		void cont(const casacore::Matrix<casacore::Float> &m,
		          const casacore::Vector<casacore::Float> &levels,
		          const casacore::Matrix<casacore::Float> &tr);

		// Contour using pgconb function. (contour with blanks)
		void conb(const casacore::Matrix<casacore::Float> &m,
		          const casacore::Vector<casacore::Float> &levels,
		          const casacore::Matrix<casacore::Float> &tr,
		          const casacore::Float blank);

	private:

		// Store for the WorldCanvas this filters for.
		WorldCanvas *itsWorldCanvas;

		// Store the PGPLOT device id here.
		casacore::uInt itsPgPlotID;

		// Reference counting.
		casacore::Int itsRefCount;

		// Split contours into positive and negative
		void splitContours (casacore::Vector<casacore::Float>& neg, casacore::Vector<casacore::Float>& pos,
		                    const casacore::Vector<casacore::Float>& levels);


	};


} //# NAMESPACE CASA - END

#endif

