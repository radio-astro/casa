//# GLPCColTblTemplates.cc: templates for mapping indexes to colors
//# Copyright (C) 2001
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
//# $Id $

#if defined(OGL)
#include <display/Display/GLPixelCanvasColorTable.h>
#include <casa/Arrays/VectorIter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// =====

	template <class T>
	void GLPixelCanvasColorTable_mapToColor(uLong * table,
	                                        uInt tableSize,
	                                        uInt mapOffset,
	                                        Array<T> & inOutArray,
	                                        Bool rangeCheck) {
		VectorIterator<T> vi(inOutArray);

		uInt n = vi.vector().nelements();
		T val;

		if (rangeCheck) {
			uInt maxc = tableSize-1;
			while (!vi.pastEnd()) {
				for (uInt i = 0; i < n; i++) {
					val = vi.vector()(i);
					vi.vector()(i) = table[mapOffset + ((val <= 0) ? 0 : (val >= maxc) ? maxc : val)];
				}
				vi.next();
			}
		} else {
			while (!vi.pastEnd()) {
				for (uInt i = 0; i < n; i++) {
					vi.vector()(i) = table[mapOffset + vi.vector()(i)];
				}
				vi.next();
			}
		}
	}

} //# NAMESPACE CASA - END

#endif // OGL
