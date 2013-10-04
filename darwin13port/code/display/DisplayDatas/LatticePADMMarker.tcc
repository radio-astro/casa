//# LatticePADMMarker.cc: Class for drawing axis bound lattice elements as vector fields
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
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/LatticeAsMarker.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <lattices/Lattices/MaskedLattice.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/BasicMath/Math.h>
#include <casa/Quanta/Unit.h>

#include <display/DisplayDatas/LatticePADMMarker.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	LatticePADMMarker<T>::LatticePADMMarker(const uInt xAxis,
	                                        const uInt yAxis, const uInt mAxis,
	                                        const IPosition fixedPos,
	                                        LatticePADisplayData<T>* arDat)
		: LatticePADisplayMethod<T>(xAxis, yAxis, mAxis, fixedPos, arDat)
	{}

	template <class T>
	LatticePADMMarker<T>::LatticePADMMarker(const uInt xAxis,
	                                        const uInt yAxis,
	                                        LatticePADisplayData<T>* arDat)
		: LatticePADisplayMethod<T>(xAxis, yAxis, arDat)
	{}

	template <class T>
	LatticePADMMarker<T>::~LatticePADMMarker()
	{}

	template <class T>
	uInt LatticePADMMarker<T>::dataDrawSelf(WorldCanvas *wCanvas,
	                                        const Vector<Double> &blc,
	                                        const Vector<Double> &trc,
	                                        const IPosition &start,
	                                        const IPosition &shape,
	                                        const IPosition &stride,
	                                        const Bool usePixelEdges)
//
// blc and trc are world coordinates.  they will be 2-D
// if there is no dependency on any third axis. If there
// is (e.g. ra/freq/dec) then they will be 3-D
//
// start and shape are appropriate to the SubImage if the
// user has applied an image region.
//
	{
		LogIO os(LogOrigin("LatticePADMMarker", "drawDataSelf", WHERE));
//
		uInt drawListNumber = wCanvas->newList();
		LatticeAsMarker<T>* lam = dynamic_cast<LatticeAsMarker<T>*>(parentDisplayData());

// set min and max datavalues to help out scalehandler on WC.

		Attribute dmin("dataMin", Double(lam->getDataMin()));
		Attribute dmax("dataMax", Double(lam->getDataMax()));
		wCanvas->setAttribute(dmin);
		wCanvas->setAttribute(dmax);
//
		T* dummy = NULL;
		DataType dataType = whatType(dummy);
		if ((dataType == TpComplex) || (dataType == TpDComplex)) {
			wCanvas->setComplexToRealMethod(lam->complexMode());
		}

// Get options from DD

		Float scale = lam->itsScale;
		Int incX = lam->itsIncX;
		Int incY = lam->itsIncY;
		String markerType = lam->itsMarker;

		try {

// Set options.  Resampling is irrelevant.  There needs to be a way
// to tell LatticePADD not to set up the resmapling adjust GUI
// handlers for some DDs

			wCanvas->setColor(lam->itsColor);
			wCanvas->pixelCanvas()->setLineWidth(lam->itsLineWidth);

// Get data. SHould really put the inc{X,Y} into the
// getSlice, but then CS needs to be subImaged as well.

			Matrix<T> datMatrix;
			Matrix<Bool> datMask;
			this->dataGetSlice(datMatrix, datMask, start, shape, stride);

// Get CS

			const CoordinateSystem& cSys = lam->coordinateSystem();
			const Vector<Double>& inc = cSys.increment();

// Draw markers

			wCanvas->drawMarkerMap(blc, trc, datMatrix, datMask, incX,
			                       incY, scale,  inc(0), inc(1),
			                       markerType, usePixelEdges);

		} catch (const AipsError &x) {
			wCanvas->endList();
			if (wCanvas->validList(drawListNumber)) {
				wCanvas->deleteList(drawListNumber);
			}
			throw(AipsError(x));
		}
//
		wCanvas->endList();
		return drawListNumber;
	}

} //# NAMESPACE CASA - END

