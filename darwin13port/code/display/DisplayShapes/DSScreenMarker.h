//# DSScreenMarker.h : Implementation of relative screen pos. DSMarker
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
#ifndef TRIALDISPLAY_DSSCREENMARKER_H
#define TRIALDISPLAY_DSSCREENMARKER_H


#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <display/DisplayShapes/DSMarker.h>
#include <display/DisplayShapes/DisplayShapeWithCoords.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of relative screen pos. DSMarker
// </summary>

	class PixelCanvas;
	class DSPixelMarker;
	class DSWorldMarker;

	class DSScreenMarker : public DSMarker, public DisplayShapeWithCoords {

	public:

		DSScreenMarker();
		DSScreenMarker(const Record& settings, PixelCanvas* pc);
		DSScreenMarker(DSPixelMarker& other, PixelCanvas* pc);
		DSScreenMarker(DSWorldMarker& other);

		virtual ~DSScreenMarker();

		virtual void recalculateScreenPosition();
		virtual Bool setOptions(const Record& settings);
		virtual Record getOptions();

		// These are so we can monitor if the marker is moved and update our
		// relative position
		virtual void move(const Float& dX, const Float& dY);
		virtual void setCenter(const Float& xPos, const Float& yPos);

		virtual Record getRawOptions() {
			return DSMarker::getOptions();
		}
	private:

		PixelCanvas* itsPC;
		Vector<Float> itsRelativeCenter;


		virtual void updateRC();
	};

} //# NAMESPACE CASA - END

#endif
