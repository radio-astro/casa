//# ComponentShape.h: Base class for component shapes
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
//# $Id: ComponentShape.h 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#ifndef ANNOTATIONS_ANNOTATIONBASE_H
#define ANNOTATIONS_ANNOTATIONBASE_H

#include <casa/aips.h>

#include <coordinates/Coordinates/CoordinateSystem.h>


namespace casa {

// <summary>Base class for annotations</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd">
// </reviewed>

// <synopsis>
// Base class for annotations

// In order to minimize maintainability, many parameters are not
// set in the constructor but can be set by mutator methods.
//
// Input directions will be converted to the reference frame of the
// input coordinate system upon construction if necessary.
// </synopsis>

class AnnotationBase {
public:

	static const String DEFAULT_LABEL;
	static const String DEFAULT_COLOR;
	static const String DEFAULT_LINESTYLE;
	static const uInt DEFAULT_LINEWIDTH;
	static const uInt DEFAULT_SYMBOLSIZE;
	static const uInt DEFAULT_SYMBOLTHICKNESS;
	static const String DEFAULT_FONT;
	static const String DEFAULT_FONTSIZE;
	static const String DEFAULT_FONTSTYLE;
	static const Bool DEFAULT_USETEX;


	enum Type {
		// annotations only
		LINE,
		VECTOR,
		TEXT,
		SYMBOL,
		// regions
		RECT_BOX,
		CENTER_BOX,
		ROTATED_BOX,
		POLYGON,
		CIRCLE,
		ANNULUS,
		ELLIPSE
	};

	virtual ~AnnotationBase();

	Type getType() const;

	// Given a string, return the corresponding annotation type or throw
	// an error if the string does not correspond to an allowed type.
	static Type typeFromString(const String& type);

	void setLabel(const String& label);

	String getLabel() const;

	void setColor(const String& color);

	String getColor() const;

	void setLineStyle(const String& lineStyle);

	String getLineStyle() const;

	void setLineWidth(const uInt linewidth);

	uInt getLineWidth() const;

	void setSymbolSize(const uInt symbolsize);

	uInt getSymbolSize() const;

	void setSymbolThickness(const uInt symbolthickness);

	uInt getSymbolThickness() const;

	void setFont(const String& font);

	String getFont() const;

	void setFontSize(const String& fontsize);

	String getFontSize() const;

	void setFontStyle(const String& fontstyle);

	String getFontStyle() const;

	void setUseTex(const Bool usetex);

	Bool isUseTex() const;

	virtual Bool isRegion() const;

	// set "pix" as valid unit. This should be called externally
	// before creating quantities which have pixel units.
	static void unitInit();


protected:
	Type _type;
	MDirection::Types _directionRefFrame;
	CoordinateSystem _csys;
	IPosition _directionAxes;
	String _label, _color, _linestyle, _font, _fontsize, _fontstyle;
	uInt _linewidth, _symbolsize, _symbolthickness;
	Bool _usetex;
	Vector<MDirection> _convertedDirections;


	AnnotationBase(
		const Type type, const String& dirRefFrameString,
		const CoordinateSystem& csys
	);

	static void _checkMixed(const String& origin, const Array<Quantity>& quantities);

	MDirection _directionFromQuantities(
		const Quantity& q0, const Quantity& q1
	);

	void _checkAndConvertDirections(const String& origin, const Matrix<Quantity>& quantities);

private:
	static Bool _doneUnitInit;


};

}

#endif
