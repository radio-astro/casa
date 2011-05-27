//# Copyright (C) 1998,1999,2000,2001
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

#ifndef ANNOTATIONS_ANNVECTOR_H
#define ANNOTATIONS_ANNVECTOR_H

#include <images/Annotations/AnnotationBase.h>

#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>

namespace casa {

// <summary>Represents a vector annotation which has a start and ending point.</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd">
// </reviewed>

// <synopsis>

// Represents an ascii vector annotation
// </synopsis>

class AnnVector: public AnnotationBase {
public:

	AnnVector(
		const Quantity& xStart,
		const Quantity& yStart,
		const Quantity& xEnd,
		const Quantity& yEnd,
		const String& dirRefFrameString,
		const CoordinateSystem& csys
	);

	// get the end point directions, transformed to
	// the input coordinate system if necessary.
	// The first element will be the starting point,
	// the second the ending point.
	Vector<MDirection> getEndPoints() const;

protected:
	Matrix<Quantity> _inputPoints;

private:
	AnnVector();

};

}

#endif
