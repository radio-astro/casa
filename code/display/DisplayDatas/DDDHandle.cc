//# DDDHandle.h: a class drawing handles on an DDDObject
//# Copyright (C) 1999,2000,2001
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

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/BasicMath/Math.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <display/DisplayDatas/DDDHandle.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	DDDHandle::DDDHandle() {
		itsCorners.resize(4,2);
		itsSize = MEDIUM;
		Vector<Double> point(2);
		point(0) = 0.0;
		point(1) = 0.0;
		createHandle(point);
	}

	DDDHandle::DDDHandle(Double xpos, Double ypos, DDDHandle::Size size) {
		itsCorners.resize(4,2);
		itsSize = size;
		Vector<Double> point(2);
		point(0) = xpos;
		point(1) = ypos;
		createHandle(point);
	}

	DDDHandle::~DDDHandle() {
	}

	void DDDHandle::createHandle(const Vector<Double>& point) {
		itsCorners(0,0) = point(0)-(Double)itsSize;
		itsCorners(0,1) = point(1)-(Double)itsSize-1.0;
		itsCorners(1,0) = point(0)+(Double)itsSize+1.0;
		itsCorners(1,1) = point(1)+(Double)itsSize;
	}

	void DDDHandle::createHandle(Double x, Double y) {
		itsCorners(0,0) = x-(Double)itsSize;
		itsCorners(0,1) = y-(Double)itsSize-1.0;
		itsCorners(1,0) = x+(Double)itsSize+1.0;
		itsCorners(1,1) = y+(Double)itsSize;
	}

	void DDDHandle::resize(const DDDHandle::Size& size) {
		itsCorners(0,0) += (Double)(itsSize-size);
		itsCorners(0,1) += (Double)(itsSize-size);
		itsCorners(1,0) -= (Double)(itsSize-size);
		itsCorners(1,1) -= (Double)(itsSize-size);
		itsSize = size;
	}

	void DDDHandle::move(Double dx, Double dy) {
		itsCorners(0,0) += dx;
		itsCorners(0,1) += dy;
		itsCorners(1,0) += dx;
		itsCorners(1,1) += dy;
	}

	Double DDDHandle::blcX() const {
		return itsCorners(0,0);
	}
	Double DDDHandle::blcY() const {
		return itsCorners(0,1);
	}
	Double DDDHandle::trcX() const {
		return itsCorners(1,0);
	}
	Double DDDHandle::trcY() const {
		return itsCorners(1,1);
	}
	Bool DDDHandle::underCursor(Double posx, Double posy) const {
		if ( (posx >= min(blcX(),trcX())) && (posx <= max(blcX(),trcX())) &&
		        (posy >= min(blcY(),trcY())) && (posy <= max(blcY(),trcY())) ) {
			return True;
		} else {
			return False;
		}
	}


	DDDHandle::DDDHandle(const DDDHandle &) {
	}

	void DDDHandle::operator=(const DDDHandle &) {
	}

} //# NAMESPACE CASA - END

