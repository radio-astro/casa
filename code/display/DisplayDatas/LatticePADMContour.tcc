//# LatticePADMContour.cc: Class for drawing axis bound lattice elements as contours
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

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <display/DisplayDatas/LatticePADMContour.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor for the case of multiple slices
	template <class T>
	LatticePADMContour<T>::LatticePADMContour(const uInt xAxis,
	        const uInt yAxis, const uInt mAxis,
	        const IPosition fixedPos,
	        LatticeAsContour<T> *arDat) :
		LatticePADisplayMethod<T>(xAxis, yAxis, mAxis, fixedPos, arDat) {
	}

// Constructor for a single slice
	template <class T>
	LatticePADMContour<T>::LatticePADMContour(const uInt xAxis,
	        const uInt yAxis,
	        LatticeAsContour<T> *arDat) :
		LatticePADisplayMethod<T>(xAxis, yAxis, arDat) {
	}

// Destructor
	template <class T>
	LatticePADMContour<T>::~LatticePADMContour() {
	}

// Actually draw the slice as an contour
	template <class T>
	uInt LatticePADMContour<T>::dataDrawSelf(WorldCanvas *wCanvas,
	        const Vector<Double> &blc,
	        const Vector<Double> &trc,
	        const IPosition &start,
	        const IPosition &shape,
	        const IPosition &stride,
	        const Bool usePixelEdges) {
		uInt drawListNumber = wCanvas->newList();
		LatticeAsContour<T> *lac = (LatticeAsContour<T> *)parentDisplayData();

		/*
		// set min and max datavalues to help out scalehandler on WC.
		Attribute dmin("dataMin", Double(lac->getDataMin()));
		Attribute dmax("dataMax", Double(lac->getDataMax()));
		wCanvas->setAttribute(dmin);
		wCanvas->setAttribute(dmax);
		*/

		// Set the complex-to-real mode on the WorldCanvas if necessary:
		T t;
		DataType dtype = whatType(&t);
		if ((dtype == TpComplex) || (dtype == TpDComplex)) {
			wCanvas->setComplexToRealMethod
			//(((LatticeAsContour<T> *)parentDisplayData())->complexMode());
			(lac->complexMode());
		}

		try {
			wCanvas->setResampleHandler(lac->resampleHandler());
			wCanvas->setColor(lac->itsColor);
			wCanvas->pixelCanvas()->setLineWidth(lac->itsLine);
			Attribute dashNeg("dashNegativeContours", Bool(lac->itsDashNeg));
			Attribute dashPos("dashPositiveContours", Bool(lac->itsDashPos));
			wCanvas->setAttribute(dashNeg);
			wCanvas->setAttribute(dashPos);
			Matrix<T> datMatrix;
			Matrix<Bool> maskMatrix;
			this->dataGetSlice(datMatrix, maskMatrix, start, shape, stride);
			Vector<Float> levels;
			levels = lac->levels();
			if (maskMatrix.nelements() == datMatrix.nelements()) {
				//cerr << "mask available, but being ignored in "
				//   << "LPADMContour::dataDrawSelf" << endl;
				wCanvas->drawContourMap(blc, trc, datMatrix, maskMatrix,
				                        levels, usePixelEdges);
			} else {
				wCanvas->drawContourMap(blc, trc, datMatrix, levels, usePixelEdges);
			}
			wCanvas->setResampleHandler(0);
		} catch (const AipsError &x) {
			wCanvas->endList();
			if (wCanvas->validList(drawListNumber)) {
				wCanvas->deleteList(drawListNumber);
			}
			throw(AipsError(x));
		}

		wCanvas->endList();
		return drawListNumber;
	}


} //# NAMESPACE CASA - END

