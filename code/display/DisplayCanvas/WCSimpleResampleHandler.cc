//# WCSimpleResampleHandler.h: Simple resampling for the WorldCanvas
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# $Id$

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <display/DisplayCanvas/WCSimpleResampleHandler.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	WCSimpleResampleHandler::WCSimpleResampleHandler() {
		itsInterp = Interpolate2D(Interpolate2D::NEAREST);
	}


	WCSimpleResampleHandler::WCSimpleResampleHandler(Interpolate2D::Method type) {
		itsInterp = Interpolate2D(type);
	}


	WCSimpleResampleHandler::WCSimpleResampleHandler (const WCSimpleResampleHandler& other)
		: WCResampleHandler(other),
		  itsInterp(other.itsInterp)
	{}



	WCSimpleResampleHandler& WCSimpleResampleHandler::operator= (const WCSimpleResampleHandler& other) {
		if (this != &other) {
			WCResampleHandler::operator=(other);
			itsInterp = other.itsInterp;
		}
		return *this;
	}

	void WCSimpleResampleHandler::setInterpolationType (Interpolate2D::Method type) {
		itsInterp = Interpolate2D(type);
	}


	void WCSimpleResampleHandler::operator() (Matrix<Float> &out,
	        const Matrix<Float> &in,
	        const Vector<Float> &inblc,
	        const Vector<Float> &intrc,
	        const Float blank) {

		// inblc, intrc are the corners of the rectangular area within*
		// the input matrix to be resampled onto the output matrix.
		// (* need not lie entirely within input; 'blank' is used to fill
		// output areas outside input extent).

		out = blank;

		Int outNX = out.shape()[0],
		    outNY = out.shape()[1];

		Double  inDeltaX = intrc[0]-inblc[0],  outDeltaX = outNX-1;
		Double  inDeltaY = intrc[1]-inblc[1],  outDeltaY = outNY-1;

		Double incrX = (outDeltaX==0.)? 0. : inDeltaX/outDeltaX;
		Double incrY = (outDeltaY==0.)? 0. : inDeltaY/outDeltaY;
		// input location increment per output pixel.

		Int i,j;
		Vector<Double> inloc(2);

		for (i=0, inloc[0]=inblc[0];   i<outNX;   i++, inloc[0]+=incrX) {
			for (j=0, inloc[1]=inblc[1];   j<outNY;   j++, inloc[1]+=incrY) {
				itsInterp.interp(out(i, j), inloc, in);
			}
		}
	}


	void WCSimpleResampleHandler::operator() (Matrix<Float> &out,
	        Matrix<Bool> &outMask,
	        const Matrix<Float> &in,
	        const Matrix<Bool> &inMask,
	        const Vector<Float> &inblc,
	        const Vector<Float> &intrc,
	        const Float blank) {

		// inblc, intrc are the corners of the rectangular area within*
		// the input matrix to be resampled onto the output matrix.
		// (* need not lie entirely within input; 'blank' is used to fill
		// output areas outside input extent).

		out = blank;
		outMask = False;

		Int outNX = out.shape()[0],
		    outNY = out.shape()[1];

		Double  inDeltaX = intrc[0]-inblc[0],  outDeltaX = outNX-1;
		Double  inDeltaY = intrc[1]-inblc[1],  outDeltaY = outNY-1;

		Double incrX = (outDeltaX==0.)? 0. : inDeltaX/outDeltaX;
		Double incrY = (outDeltaY==0.)? 0. : inDeltaY/outDeltaY;
		// input location increment per output pixel.

		Int i,j;
		Vector<Double> inloc(2);

		for (i=0, inloc[0]=inblc[0];   i<outNX;   i++, inloc[0]+=incrX) {
			for (j=0, inloc[1]=inblc[1];   j<outNY;   j++, inloc[1]+=incrY) {
				outMask(i,j) = itsInterp.interp(out(i, j), inloc, in, inMask);
			}
		}
	}


	void WCSimpleResampleHandler::operator()  (Matrix<Bool> &out,
	        const Matrix<Bool> &in,
	        const Vector<Float> &inblc,
	        const Vector<Float> &intrc,
	        const Bool blank) {

		// inblc, intrc are the corners of the rectangular area within*
		// the input matrix to be resampled onto the output matrix.
		// (* need not lie entirely within input; 'blank' is used to fill
		// output areas outside input extent).

		out = blank;

		Int outNX = out.shape()[0],
		    outNY = out.shape()[1];

		Double  inDeltaX = intrc[0]-inblc[0],  outDeltaX = outNX-1;
		Double  inDeltaY = intrc[1]-inblc[1],  outDeltaY = outNY-1;

		Double incrX = (outDeltaX==0.)? 0. : inDeltaX/outDeltaX;
		Double incrY = (outDeltaY==0.)? 0. : inDeltaY/outDeltaY;
		// input location increment per output pixel.

		Int i,j;
		Vector<Double> inloc(2);

		for (i=0, inloc[0]=inblc[0];   i<outNX;   i++, inloc[0]+=incrX) {
			for (j=0, inloc[1]=inblc[1];   j<outNY;   j++, inloc[1]+=incrY) {
				itsInterp.interp(out(i, j), inloc, in);
			}
		}
	}


#define WCSimpleResampleHandlerRESAMPLE(Type) \
throw(AipsError("WCSimpleResampleHandler for this type not implemented"));

	void WCSimpleResampleHandler::operator()(Matrix<Bool> & out, const Matrix<Bool> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Bool);
	}
	void WCSimpleResampleHandler::operator()(Matrix<uChar> & out, const Matrix<uChar> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(uChar);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Char> & out, const Matrix<Char> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Char);
	}
	void WCSimpleResampleHandler::operator()(Matrix<uShort> & out, const Matrix<uShort> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(uShort);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Short> & out, const Matrix<Short> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Short);
	}
	void WCSimpleResampleHandler::operator()(Matrix<uInt> & out, const Matrix<uInt> & in) {

		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(uInt);
	}

	void WCSimpleResampleHandler::operator()(Matrix<Int> & out, const Matrix<Int> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Int);
	}
	void WCSimpleResampleHandler::operator()(Matrix<uLong> & out, const Matrix<uLong> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(uLong);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Long> & out, const Matrix<Long> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Long);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Float> & out, const Matrix<Float> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}

		WCSimpleResampleHandlerRESAMPLE(Float);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Double> & out, const Matrix<Double> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Double);
	}
	void WCSimpleResampleHandler::operator()(Matrix<Complex> & out, const Matrix<Complex> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(Complex);
	}
	void WCSimpleResampleHandler::operator()(Matrix<DComplex> & out, const Matrix<DComplex> & in) {
		if (blc()(0) == 0.0 && blc()(1) == 0.0 && trc()(0) == 1.0 && trc()(1) == 1.0
		        && in.shape() == out.shape()) {
			out = in.copy();
			return;
		}
		WCSimpleResampleHandlerRESAMPLE(DComplex);
	}

// Destructor
	WCSimpleResampleHandler::~WCSimpleResampleHandler() {
	}


} //# NAMESPACE CASA - END

