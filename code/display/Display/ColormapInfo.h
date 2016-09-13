//# ColormapInfo.h: store information about the dynamic mapping of a Colormap
//# Copyright (C) 1994,1995,1996,1997,1998,1999
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

#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class Colormap;

#ifndef TRIALDISPLAY_COLORMAPINFO_H
#define TRIALDISPLAY_COLORMAPINFO_H

// <summary>
// ColormapInfo stores information about the dynamic mapping of a Colormap.
// </summary>
//
// <synopsis>
// </synopsis>
//
// <motivation>
// </motivation>
//
// <example>
// </example>

	class ColormapInfo {

	public:

		// (Required) default constructor.
		ColormapInfo();

		// Constructor taking a pointer to a Colormap, and the initial
		// weight, offset and size of the Colormap.
		ColormapInfo(const Colormap *colormap, const casacore::Float &weight,
		             const casacore::uInt &offset, const casacore::uInt &size);

		// Yield a pointer to the Colormap about which this stores
		// information.
		const Colormap *colormap() const {
			return itsColormap;
		}

		// Return the current weight for this mapping.
		casacore::Float weight() const {
			return itsWeight;
		}

		// Return the current offset for this mapping.
		casacore::uInt offset() const {
			return itsOffset;
		}

		// Return the current size for this mapping.
		casacore::uInt size() const {
			return itsSize;
		}

		// Set the weight for this mapping.
		void setWeight(const casacore::Float &weight);

		// Set the offset for this mapping.
		void setOffset(const casacore::uInt &offset);

		// Set the size for this mapping.
		void setSize(const casacore::uInt &size);

		// Increment and decrement the reference count for this mapping.
		// <group>
		void ref();
		void unref();
		// </group>

		// Return the current reference count.
		casacore::uInt refCount() const {
			return itsRefCount;
		}

	private:

		// Store the Colormap of this mapping here.
		const Colormap *itsColormap;

		// Store the weight of the mapping here.
		casacore::Float itsWeight;

		// Store the offset of the mapping here.
		casacore::uInt itsOffset;

		// Store the size of the mapping here.
		casacore::uInt itsSize;

		// Store the reference count of the mapping here.
		casacore::Int itsRefCount;

	};


} //# NAMESPACE CASA - END

#endif
