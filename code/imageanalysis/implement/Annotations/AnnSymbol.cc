//# GaussianShape.cc:
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

#include <imageanalysis/Annotations/AnnSymbol.h>

namespace casa {

AnnSymbol::AnnSymbol(
	const Quantity& x, const Quantity& y,
	const String& dirRefFrameString,
	const CoordinateSystem& csys,
	const Char symbolChar
) : AnnotationBase(SYMBOL, dirRefFrameString, csys),
	_symbolChar(symbolChar) {
	if ((_symbol = charToSymbol(symbolChar)) == UNKOWN) {
		throw AipsError(
			String(symbolChar)
				+ " does not correspond to a known symbol"
		);
	}

	_inputDirection.resize(2);
	_inputDirection[0] = x;
	_inputDirection[1] = y;
	_checkAndConvertDirections(String(__FUNCTION__), _inputDirection);

}

MDirection AnnSymbol::getDirection() const {
	return _convertedDirections[0];
}

AnnSymbol::Symbol AnnSymbol::getSymbol() const {
	return _symbol;
}

AnnSymbol::Symbol AnnSymbol::charToSymbol(
	const Char c
) {
	switch(c) {
	case '.': return POINT;
	case ',': return PIXEL;
	case 'o': return CIRCLE;
	case 'v': return TRIANGLE_DOWN;
	case '^': return TRIANGLE_UP;
	case '<': return TRIANGLE_LEFT;
	case '>': return TRIANGLE_RIGHT;
	case '1': return TRI_DOWN;
	case '2': return TRI_UP;
	case '3': return TRI_LEFT;
	case '4': return TRI_RIGHT;
	case 's': return SQUARE;
	case 'p': return PENTAGON;
	case '*': return STAR;
	case 'h': return HEXAGON1;
	case 'H': return HEXAGON2;
	case '+': return PLUS;
	case 'x': return X;
	case 'D': return DIAMOND;
	case 'd': return THIN_DIAMOND;
	case '|': return VLINE;
	case '_': return HLINE;
	default: return UNKOWN;
	}
}

ostream& AnnSymbol::print(ostream &os) const {
	os << "symbol [[" << _inputDirection[0] << ", "
		<< _inputDirection[1] << "], "
		<< _symbolChar << "]";
	_printPairs(os);
	return os;
}


}


