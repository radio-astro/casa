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

map<Char, AnnSymbol::Symbol> AnnSymbol::_symbolMap;

const String AnnSymbol::_class = "AnnSymbol";

AnnSymbol::AnnSymbol(
	const Quantity& x, const Quantity& y,
	const String& dirRefFrameString,
	const CoordinateSystem& csys,
	const Char symbolChar
) : AnnotationBase(SYMBOL, dirRefFrameString, csys),
	_symbolChar(symbolChar) {
	if (_symbolMap.size() == 0) {
		_initMap();
	}
	if ((_symbol = charToSymbol(symbolChar)) == UNKNOWN) {
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

AnnSymbol::AnnSymbol(
	const Quantity& x, const Quantity& y,
	const CoordinateSystem& csys,
	const Symbol symbol
) : AnnotationBase(SYMBOL, csys),
	_symbol(symbol) {
	if (_symbolMap.size() == 0) {
		_initMap();
	}
	_symbolChar = symbolToChar(_symbol);
	_inputDirection.resize(2);
	_inputDirection[0] = x;
	_inputDirection[1] = y;
	_checkAndConvertDirections(String(__FUNCTION__), _inputDirection);
}

AnnSymbol& AnnSymbol::operator= (
	const AnnSymbol& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnotationBase::operator=(other);
    _inputDirection.resize(other._inputDirection.nelements());
    _inputDirection = other._inputDirection;
    _symbol = other._symbol;
    _symbolChar = other._symbolChar;
    return *this;
}

MDirection AnnSymbol::getDirection() const {
	return _getConvertedDirections()[0];
}

AnnSymbol::Symbol AnnSymbol::getSymbol() const {
	return _symbol;
}

void AnnSymbol::_initMap() {
	_symbolMap['.'] = POINT;
	_symbolMap[','] = PIXEL;
	_symbolMap['o'] = CIRCLE;
	_symbolMap['v'] = TRIANGLE_DOWN;
	_symbolMap['^'] = TRIANGLE_UP;
	_symbolMap['<'] = TRIANGLE_LEFT;
	_symbolMap['>'] = TRIANGLE_RIGHT;
	_symbolMap['1'] = TRI_DOWN;
	_symbolMap['2'] = TRI_UP;
	_symbolMap['3'] = TRI_LEFT;
	_symbolMap['4'] = TRI_RIGHT;
	_symbolMap['s'] = SQUARE;
	_symbolMap['p'] = PENTAGON;
	_symbolMap['*'] = STAR;
	_symbolMap['h'] = HEXAGON1;
	_symbolMap['H'] = HEXAGON2;
	_symbolMap['+'] = PLUS;
	_symbolMap['x'] = X;
	_symbolMap['D'] = DIAMOND;
	_symbolMap['d'] = THIN_DIAMOND;
	_symbolMap['|'] = VLINE;
	_symbolMap['_'] = HLINE;
}

AnnSymbol::Symbol AnnSymbol::charToSymbol(
	const Char c
) {
	if (_symbolMap.find(c) != _symbolMap.end()) {
		return _symbolMap.at(c);
	}
	else {
		return UNKNOWN;
	}
}

Char AnnSymbol::symbolToChar(const AnnSymbol::Symbol s) {
	for (
		map<Char, Symbol>::const_iterator iter=_symbolMap.begin();
		iter != _symbolMap.end(); iter++
	) {
		if (s == iter->second) {
			return iter->first;
		}
	}
	ostringstream oss;
	oss << _class << "::" << __FUNCTION__
		<< ": Logic error. No corresponding character found for symbol " << s;
	throw AipsError(oss.str());
}

ostream& AnnSymbol::print(ostream &os) const {
	os << "symbol [[" << _inputDirection[0] << ", "
		<< _inputDirection[1] << "], "
		<< _symbolChar << "]";
	_printPairs(os);
	return os;
}


}


