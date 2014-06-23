//# DSScreenPolyLine.h : Implementation of a relative screen pos. DSPolyLine
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

#ifndef TRIALDISPLAY_DSSCREENPOLYLINE_H
#define TRIALDISPLAY_DSSCREENPOLYLINE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <display/Display/PanelDisplay.h>

#include <display/DisplayShapes/DSPolyLine.h>
#include <display/DisplayShapes/DisplayShapeWithCoords.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a relative screen pos. DSPolyLine
// </summary>

	class PixelCanvas;
	class DSPixelPolyLine;
	class DSWorldPolyLine;

	class DSScreenPolyLine : public DSPolyLine , public DisplayShapeWithCoords {

	public:
		DSScreenPolyLine();
		DSScreenPolyLine(const Record& settings, PixelCanvas* pc);
		DSScreenPolyLine(DSPixelPolyLine& other, PixelCanvas* pc);
		DSScreenPolyLine(DSWorldPolyLine& other);

		virtual ~DSScreenPolyLine();

		virtual void recalculateScreenPosition();

		virtual void setCenter(const Float& xPos, const Float& yPos);
		virtual void move(const Float& dX, const Float& dY);
		virtual void scale(const Float& scaleFactor);
		virtual void rotate(const Float& angle);

		virtual void changePoint(const Vector<Float>&pos, const Int n);
		virtual void changePoint(const Vector<Float>& pos);
		virtual void addPoint(const Vector<Float>& newPos);
		virtual void setPoints(const Matrix<Float>& points);

		virtual Bool setOptions(const Record& settings);
		virtual Record getOptions();

		virtual Record getRawOptions() {
			return DSPolyLine::getOptions();
		}

	private:
		PixelCanvas* itsPC;
		Matrix<Float> itsRelativePoints;

		void updateRelative();

	};

} //# NAMESPACE CASA - END

#endif




