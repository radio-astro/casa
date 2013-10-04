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

#ifndef ANNOTATIONS_ANNTEXT_H
#define ANNOTATIONS_ANNTEXT_H

#include <imageanalysis/Annotations/AnnotationBase.h>

namespace casa {

// <summary>Represents a text annotation</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd">
// </reviewed>

// <synopsis>

// Represents an ascii text annotation
// </synopsis>

class AnnText: public AnnotationBase {
public:

	AnnText(
		const Quantity& xPos, const Quantity& yPos,
		const String& dirRefFrameString,
		const CoordinateSystem& csys,
		const String& text,
		const Quantity& beginFreq,
		const Quantity& endFreq,
		const String& freqRefFrame,
		const String& dopplerString,
		const Quantity& restfreq,
		const Vector<Stokes::StokesTypes>& stokes
	);

	AnnText(
		const Quantity& xPos, const Quantity& yPos,
		const CoordinateSystem& csys,
		const String& text,
		const Vector<Stokes::StokesTypes>& stokes
	);

	// implicit copy constructor and destructor are fine

	AnnText& operator=(const AnnText& other);

	// left most point of text string
	MDirection getDirection() const;

	String getText() const;

	virtual ostream& print(ostream &os) const;

private:
	AnnotationBase::Direction _inputDirection;
	String _text;

	void _init(const Quantity& x, const Quantity& y);

};

}

#endif
