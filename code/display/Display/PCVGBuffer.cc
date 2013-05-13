//# PCVGBuffer.cc: buffering of PixelCanvas vector graphics
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
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
//#$Id$

#include <display/Display/PixelCanvas.h>
#include <display/Display/PCVGBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
	PCVGBuffer::PCVGBuffer(PixelCanvas * pc, uInt nVerts) :
		pc_(pc),
		mode_(AMNone),
		c_(0),
		verts_(nVerts,2u),
		blockSize_(nVerts) {
	}

	void PCVGBuffer::accumPoint(Float x, Float y) {
		// switch mode
		setMode(AMPoints);
		// check space
		checkSpace();
		// add point
		verts_(c_,0) = x;
		verts_(c_,1) = y;
		c_++;
	}

	void PCVGBuffer::accumLine(Float x1, Float y1, Float x2, Float y2) {
		// switch mode
		setMode(AMLines);
		// check space
		checkSpace();
		// add end points
		verts_(c_,0) = x1;
		verts_(c_,1) = y1;
		c_++;
		verts_(c_,0) = x2;
		verts_(c_,1) = y2;
		c_++;
	}

	void PCVGBuffer::accumPolylinePoint(Float x, Float y) {
		// switch mode
		setMode(AMPolyline);
		// check space
		checkSpace();
		// add vertex
		verts_(c_,0) = x;
		verts_(c_,1) = y;
		c_++;
	}
	void PCVGBuffer::accumPolygonPoint(Float x, Float y) {
		// switch mode
		setMode(AMPolygon);
		// check space
		checkSpace();
		// add vertex
		verts_(c_,0) = x;
		verts_(c_,1) = y;
		c_++;
	}

	void PCVGBuffer::flush() {
		if (mode_ == AMNone || c_ == 0) return;

		Matrix<Float> m = verts_(IPosition(2, 0, 0), IPosition(2, c_ - 1, 1));


		switch(mode_) {
		case AMNone:
			break;
		case AMPoints:
			pc_->drawPoints(m);
			break;
		case AMLines:
			pc_->drawLines(m);
			break;
		case AMPolyline:
			pc_->drawPolyline(m);
			break;
		case AMPolygon:
			pc_->drawPolygon(m);
			break;
		}
		c_ = 0;
		verts_.resize(blockSize_, 2);
		mode_ = AMNone;
	}

	void PCVGBuffer::setMode(AccumMode mode) {
		if (mode == mode_) return;

		// flush the buffer and switch modes
		flush();
		mode_ = mode;
	}

	void PCVGBuffer::checkSpace() {
		// check if we've reached the end of the matrix
		// (sub 1 because some buffers add two vertices...)
		if ((Int)c_ < verts_.shape()(0) - 1) {
			return;
		}
		// we have, so resize it upwards
		Matrix<Float> temp(verts_.shape());
		temp = verts_;
		verts_.resize(temp.shape()(0) + blockSize_, 2);
		verts_(IPosition(2, 0, 0), IPosition(2, temp.shape()(0)-1, 1)) = temp;
	}

// Destructor
	PCVGBuffer::~PCVGBuffer() {
	}


} //# NAMESPACE CASA - END

