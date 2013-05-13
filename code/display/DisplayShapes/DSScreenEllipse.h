//# DSScreenEllipse.h : Implementation of a relative screen pos. DSEllipse
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id:
#ifndef TRIALDISPLAY_DSSCREENELLIPSE_H
#define TRIALDISPLAY_DSSCREENELLIPSE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <display/DisplayShapes/DSEllipse.h>
#include <display/DisplayShapes/DisplayShapeWithCoords.h>

// <summary>
// Implementation of a relative screen pos. DSEllipse
// </summary>

namespace casa { //# NAMESPACE CASA - BEGIN

	class DSPixelEllipse;
	class DSWorldEllipse;
	class PixelCanvas;

	class DSScreenEllipse : public DSEllipse, public DisplayShapeWithCoords {

	public:

		DSScreenEllipse();
		DSScreenEllipse(const Record& setting, PixelCanvas* pc);
		DSScreenEllipse(DSPixelEllipse& other, PixelCanvas* pc);
		DSScreenEllipse(DSWorldEllipse& other);

		virtual ~DSScreenEllipse();

		virtual void recalculateScreenPosition();
		virtual Bool setOptions(const Record& settings);
		virtual Record getOptions();

		virtual void move(const Float& dX, const Float& dy);
		virtual void setCenter(const Float& xPos, const Float& yPos);
		virtual void rotate(const Float& angle);
		virtual void changePoint(const Vector<Float>&pos, const Int n);
		virtual void changePoint(const Vector<Float>& pos);
		// PLUS OTHERS! :)  change point etc

		virtual Record getRawOptions() {
			return DSEllipse::getOptions();
		}

		virtual void updateRelative();

	private:
		PixelCanvas* itsPC;
		Vector<Float> itsRelativeCenter;

		Float itsRelWidth;
		Float itsRelHeight;

	};

} //# NAMESPACE CASA - END

#endif



