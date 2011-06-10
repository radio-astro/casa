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

#include <imageanalysis/Annotations/AnnotationBase.h>

#include <casa/Exceptions/Error.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/MCDirection.h>

namespace casa {
const String AnnotationBase::DEFAULT_LABEL = "";
const String AnnotationBase::DEFAULT_COLOR = "green";
const String AnnotationBase::DEFAULT_LINESTYLE = "solid";
const uInt AnnotationBase::DEFAULT_LINEWIDTH = 1;
const uInt AnnotationBase::DEFAULT_SYMBOLSIZE = 1;
const uInt AnnotationBase::DEFAULT_SYMBOLTHICKNESS = 1;
const String AnnotationBase::DEFAULT_FONT = "Helvetica";
const String AnnotationBase::DEFAULT_FONTSIZE = "10pt";
const String AnnotationBase::DEFAULT_FONTSTYLE = "bold";
const Bool AnnotationBase::DEFAULT_USETEX = False;

Bool AnnotationBase::_doneUnitInit = False;

AnnotationBase::AnnotationBase(
	const Type type, const String& dirRefFrameString,
	const CoordinateSystem& csys
)
: _type(type), _csys(csys), _label(DEFAULT_LABEL), _color(DEFAULT_COLOR),
  _linestyle(DEFAULT_LINESTYLE), _font(DEFAULT_FONT),
  _fontsize(DEFAULT_FONTSIZE), _fontstyle(DEFAULT_FONTSTYLE),
  _linewidth(DEFAULT_LINEWIDTH), _symbolsize(DEFAULT_SYMBOLSIZE),
  _symbolthickness(DEFAULT_SYMBOLTHICKNESS), _usetex(DEFAULT_USETEX),
  _globals(map<Keyword, Bool>()), _params(map<Keyword, String>()),
  _printGlobals(False) {
	String preamble = String(__FUNCTION__) + ": ";
	if (!csys.hasDirectionCoordinate()) {
		throw AipsError(
			preamble + "Coordinate system has no direction coordinate"
		);
	}

	if (! MDirection::getType(_directionRefFrame, dirRefFrameString)) {
		throw AipsError(
			preamble + "Unknown coordinate frame "
			+ dirRefFrameString
		);
	}
	if (
		_directionRefFrame != _csys.directionCoordinate().directionType(False)
		&& _directionRefFrame != MDirection::B1950
		&& _directionRefFrame != MDirection::B1950_VLA
		&& _directionRefFrame != MDirection::BMEAN
		&& _directionRefFrame != MDirection::DEFAULT
		&& _directionRefFrame != MDirection::ECLIPTIC
		&& _directionRefFrame != MDirection::GALACTIC
		&& _directionRefFrame != MDirection::J2000
		&& _directionRefFrame != MDirection::SUPERGAL
	) {
		throw AipsError(preamble
			+ "Unsupported coordinate frame for annotations "
			+ dirRefFrameString
		);
	}
	_params[COORD] = dirRefFrameString;
	_directionAxes = IPosition(_csys.directionAxesNumbers());
	for(uInt i=0; i<N_KEYS; i++) {
		_globals[(Keyword)i] = False;
	}
	_initParams();
}

AnnotationBase::~AnnotationBase() {}

void AnnotationBase::_initParams() {
	_params[LINEWIDTH] = String::toString(_linewidth);
	_params[LINESTYLE] = _linestyle;
	_params[SYMSIZE] = String::toString(_symbolsize);
	_params[SYMTHICK] = String::toString(_symbolthickness);
	_params[COLOR] = _color;
	_params[FONT] = _font;
	_params[FONTSIZE] = _fontsize;
	_params[FONTSTYLE] = _fontstyle;
	_params[USETEX] = _usetex ? "true" : "false";
	if (! _label.empty()) {
		_params[LABEL] = _label;
	}
}

void AnnotationBase::unitInit() {
	if (! _doneUnitInit) {
		UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
		_doneUnitInit = True;
	}
}

AnnotationBase::Type AnnotationBase::getType() const {
	return _type;
}

AnnotationBase::Type AnnotationBase::typeFromString(
	const String& type
) {
	String cAnnType = type;
	cAnnType.downcase();
	cAnnType.trim();
	if (cAnnType == "line") {
		return LINE;
	}
	else if (cAnnType == "vector") {
		return VECTOR;
	}
	else if (cAnnType == "text") {
		return TEXT;
	}
	else if (cAnnType == "symbol") {
		return SYMBOL;
	}
	if (cAnnType == "rectangularbox" || cAnnType == "box") {
		return RECT_BOX;
	}
	else if (cAnnType == "centerbox") {
		return CENTER_BOX;
	}
	else if (cAnnType == "rotatedbox" || cAnnType == "rotbox") {
		return ROTATED_BOX;
	}
	else if (cAnnType == "polygon" || cAnnType == "poly") {
		return POLYGON;
	}
	else if (cAnnType == "circle") {
		return CIRCLE;
	}
	else if (cAnnType == "annulus") {
		return ANNULUS;
	}
	else if (cAnnType == "ellipse") {
		return ELLIPSE;
	}
	else {
		throw AipsError(type + " is not a supported annotation type");
	}
}

void AnnotationBase::setLabel(const String& s) {
	_label = s;
	if (_label.empty()) {
		if (_params.find(LABEL) == _params.end()) {
			_params[LABEL] = _label;
		}
		else {
			_params.erase(LABEL);
		}
	}
}

String AnnotationBase::getLabel() const {
	return _label;
}

void AnnotationBase::setColor(const String& s) {
	_color = s;
	_params[COLOR] = _color;
}

String AnnotationBase::getColor() const {
	return _color;
}

void AnnotationBase::setLineStyle(const String& s) {
	_linestyle = s;
	_params[LINESTYLE] = _linestyle;
}

String AnnotationBase::getLineStyle() const {
	return _linestyle;
}

void AnnotationBase::setLineWidth(const uInt s) {
	_linewidth = s;
	_params[LINEWIDTH] = String::toString(_linewidth);
}

uInt AnnotationBase::getLineWidth() const {
	return _linewidth;
}

void AnnotationBase::setSymbolSize(const uInt s) {
	_symbolsize = s;
	_params[SYMSIZE] = String::toString(_symbolsize);

}

uInt AnnotationBase::getSymbolSize() const {
	return _symbolsize;
}

void AnnotationBase::setSymbolThickness(const uInt s) {
	_symbolthickness = s;
	_params[SYMTHICK] = String::toString(_symbolthickness);
}

uInt AnnotationBase::getSymbolThickness() const {
	return _symbolthickness;
}

void AnnotationBase::setFont(const String& s) {
	_font = s;
	_params[FONT] = _font;
}

String AnnotationBase::getFont() const {
	return _font;
}

void AnnotationBase::setFontSize(const String& s) {
	_fontsize = s;
	_params[FONTSIZE] = String::toString(_fontsize);
}

String AnnotationBase::getFontSize() const {
	return _fontsize;
}

void AnnotationBase::setFontStyle(const String& s) {
	_fontstyle = s;
	_params[FONTSTYLE] = _fontstyle;
}

String AnnotationBase::getFontStyle() const {
	return _fontstyle;
}

void AnnotationBase::setUseTex(const Bool s) {
	_usetex = s;
	_params[USETEX] = _usetex ? "true" : "false";
}

Bool AnnotationBase::isUseTex() const {
	return _usetex;
}

Bool AnnotationBase::isRegion() const {
	return False;
}

void AnnotationBase::setGlobals(
	const Vector<Keyword>& globalKeys
) {
	for (
		Vector<Keyword>::const_iterator iter=globalKeys.begin();
		iter != globalKeys.end(); iter++) {
			_globals[*iter] = True;
	}
}

String AnnotationBase::keywordToString(
	const Keyword key
) {
	switch(key) {
	case COORD: return "coord";
	case RANGE: return "range";
	case FRAME: return "frame";
	case CORR: return "corr";
	case VELTYPE: return "veltype";
	case RESTFREQ: return "restfreq";
	case LINEWIDTH: return "linewidth";
	case LINESTYLE: return "linestyle";
	case SYMSIZE: return "symsize";
	case SYMTHICK: return "symthick";
	case COLOR: return "color";
	case FONT: return "font";
	case FONTSIZE: return "fontsize";
	case FONTSTYLE: return "fontstyle";
	case USETEX: return "usetex";
	case LABEL: return "label";
	case UNKNOWN:
	case N_KEYS:
	default:
		throw AipsError("Not string representation");
	}
}


ostream& AnnotationBase::print(
	ostream& os, const map<Keyword, String>& params
) {
	if (params.size() == 0) {
		return os;
	}
	map<Keyword, String>::const_iterator bb = params.end();
	bb--;
	for (
		map<Keyword, String>::const_iterator iter=params.begin();
		iter!=params.end(); iter++
	) {
		os << keywordToString((Keyword)iter->first)
			<< "=" << iter->second;
		if (iter != bb) {
			os << ", ";
		}
	}
	return os;
}

void AnnotationBase::_printPairs(ostream &os) const {
	map<Keyword, String> x = _params;
	if (! _printGlobals) {
		for (
			map<Keyword, String>::const_iterator iter = _params.begin();
			iter != _params.end(); iter++
		) {
			Keyword k = iter->first;
			if (_globals.find(k) != _globals.end() && _globals.at(k)) {
				x.erase(k);
			}
		}
	}
	if (x.size() > 0) {
		os << " " << x;
	}
}

void AnnotationBase::_checkMixed(
	const String& origin, const Array<Quantity>& quantities
) {
	Bool isWorld = False;
	Bool isPixel = False;
	Bool isMixed = False;
	Quantity qArg;
	for (
		Array<Quantity>::const_iterator iter = quantities.begin();
		iter != quantities.end(); iter++
	) {
		Bool pix = iter->getUnit() == "pix";
		Bool world = ! pix;
		isWorld = isWorld || world;
		isPixel = isPixel || pix;
		if (isPixel && isWorld) {
			isMixed = True;
			break;
		}
	}
	if (isMixed) {
		throw AipsError(
			origin
			+ ": Mixed world and pixel coordinates not supported"
		);
	}
}

MDirection AnnotationBase::_directionFromQuantities(
	const Quantity& q0, const Quantity& q1
) {
	ostringstream oss;
	oss << q0 << ", " << q1;
	Quantity d0 = q0;
	Quantity d1 = q1;

	String value = oss.str();
	if (q0.getUnit() == "pix") {
		// both quantities are in pix, this check should
		// have been done prior to calling this method
		Vector<Double> pixel(_csys.nPixelAxes(), 0);
		pixel[_directionAxes[0]] = q0.getValue();
		pixel[_directionAxes[1]] = q1.getValue();
		Vector<Double> world;
		_csys.toWorld(world, pixel);
		Vector<String> axesUnits = _csys.worldAxisUnits();
		d0 = Quantity(world[_directionAxes[0]], axesUnits[_directionAxes[0]]);
		d1 = Quantity(world[_directionAxes[1]], axesUnits[_directionAxes[1]]);
		MDirection::Types csysDirectionType = _csys.directionCoordinate().directionType(False);
		if (_directionRefFrame != csysDirectionType) {
			LogIO log;
			log << LogOrigin(String(__FUNCTION__)) << LogIO::WARN
				<< ": Direction quantities specified in pixels but specified direction reference "
				<< "frame (" << MDirection::showType(_directionRefFrame) << ") is different from "
				<< "the reference frame (" << MDirection::showType(csysDirectionType)
				<< ") of the coordinate system. The reference frame of the coordinate system "
				<< "will be used and the direction coordinates will not be transformed"
				<< LogIO::POST;
		}
		_directionRefFrame = csysDirectionType;
	}
	try {
		return MDirection(d0, d1, _directionRefFrame);
	}
	catch (AipsError x) {
		throw AipsError(
			String(__FUNCTION__) + "Error converting direction ("
			+ value + ") to MDirection: " + x.getMesg()
		);
	}
}

void AnnotationBase::_checkAndConvertDirections(
	const String& origin, const Matrix<Quantity>& quantities
) {
	_checkMixed(origin, quantities);
	MDirection::Types csysDirectionRefFrame = _csys.directionCoordinate().directionType(False);
	Bool needsConverting = _directionRefFrame != csysDirectionRefFrame;
	_convertedDirections.resize(quantities.shape()[1]);
	for (uInt i=0; i<_convertedDirections.size(); i++) {
		_convertedDirections[i] = _directionFromQuantities(quantities(0, i), quantities(1, i));
		if (needsConverting) {
			_convertedDirections[i] = MDirection::Convert(_convertedDirections[i], csysDirectionRefFrame)();
		}
	}
}

}


