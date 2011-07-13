//# Copyright (C) 1998,1999,2000
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

#include <imageanalysis/Annotations/AnnVector.h>

namespace casa {

AnnVector::AnnVector(
	const Quantity& xStart,
	const Quantity& yStart,
	const Quantity& xEnd,
	const Quantity& yEnd,
	const String& dirRefFrameString,
	const CoordinateSystem& csys
) : AnnotationBase(VECTOR, dirRefFrameString, csys),
	_inputPoints(Matrix<Quantity>(2, 2)) {

	_inputPoints(0, 0) = xStart;
	_inputPoints(1, 0) = yStart;
	_inputPoints(0, 1) = xEnd;
	_inputPoints(1, 1) = yEnd;
	_checkAndConvertDirections(String(__FUNCTION__), _inputPoints);
}

Vector<MDirection> AnnVector::getEndPoints() const {
	return _convertedDirections;
}

ostream& AnnVector::print(ostream &os) const {
	os << "vector [[" << _inputPoints(0, 0) << ", "
		<< _inputPoints(1, 0) << "], ["
		<< _inputPoints(0, 1) << ", "
		<< _inputPoints(1, 1) << "]]";
	_printPairs(os);
	return os;
}


}


