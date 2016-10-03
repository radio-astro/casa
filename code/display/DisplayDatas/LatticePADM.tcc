//# LatticePADM.cc: Base class for drawing axis-bound lattice elements
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/IPosition.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <display/DisplayDatas/LatticePADD.h>
#include <display/DisplayDatas/LatticePADM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor
	template <class T>
	LatticePADisplayMethod<T>::LatticePADisplayMethod() :
		PrincipalAxesDM() {
	}

	template <class T>
	LatticePADisplayMethod<T>::LatticePADisplayMethod(const casacore::uInt xAxis,
	        const casacore::uInt yAxis,
	        const casacore::uInt mAxis,
	        const casacore::IPosition fixedPos,
	        LatticePADisplayData<T> *arDat) :
		PrincipalAxesDM(xAxis, yAxis, mAxis, (PrincipalAxesDD *)arDat) {

		// call setup which does things common to all PrincipalAxesDM objects
		setup(fixedPos);
	}

	template <class T>
	LatticePADisplayMethod<T>::LatticePADisplayMethod(const casacore::uInt xAxis,
	        const casacore::uInt yAxis,
	        LatticePADisplayData<T> *arDat) :
		PrincipalAxesDM(xAxis, yAxis, 0, (PrincipalAxesDD *)arDat) {

		// call setup2d which does things common to all PrincipalAxesDM objects
		setup2d();
	}

// Destructor
	template <class T>
	LatticePADisplayMethod<T>::~LatticePADisplayMethod() {
	}

// Query the shape of the lattice
	template <class T>
	casacore::IPosition LatticePADisplayMethod<T>::dataShape() {
		casacore::Lattice<T> *latt = ((LatticePADisplayData<T> *)parentDisplayData())->
		                   maskedLattice().get();
		if (!latt) {
			throw(casacore::AipsError("LatticePADisplayMethod<T>::dataShape - "
			                "no lattice is available"));
		}
		return latt->shape();
	}

	template <class T>
	casacore::Bool LatticePADisplayMethod<T>::dataGetSlice(casacore::Matrix<T>& data,
	        casacore::Matrix<casacore::Bool> &mask,
	        const casacore::IPosition& start,
	        const casacore::IPosition& sliceShape,
	        const casacore::IPosition& stride) {
		casacore::MaskedLattice<T>* latt =
		    ((LatticePADisplayData<T> *)parentDisplayData())->maskedLattice().get();
		if (!latt) {
			throw(casacore::AipsError("LatticePADisplayMethod<T>::dataGetSlice - "
			                "no lattice is available"));
		}
		return dataGetSlice (data, mask, start, sliceShape, stride, *latt);
	}

	template <class T>
	casacore::Bool LatticePADisplayMethod<T>::dataGetSlice(casacore::Matrix<T>& data,
	        casacore::Matrix<casacore::Bool> &mask,
	        const casacore::IPosition& start,
	        const casacore::IPosition& sliceShape,
	        const casacore::IPosition& stride,
	        casacore::MaskedLattice<T>& latt) {

		// It is assumed that sliceShape has already been trimmed so
		// that it doesn't dangle over the edge of the lattice

		data.resize(0,0);
		mask.resize(0,0);	// Reset Matrices so they can resize automatically.

		if (needToTranspose()) {

			casacore::Matrix<T>    tmpData(latt.getSlice(start, sliceShape, stride, true));
			casacore::Matrix<casacore::Bool> tmpMask(latt.getMaskSlice(start, sliceShape, stride, true));
			// These will be Nx1 in a degenerate axis case.
			// (When 1xN is needed, needToTranspose() will also have been true).

			data = transpose(tmpData);
			if (anyEQ(tmpMask, false)) mask = transpose(tmpMask);
			// (otherwise, 0-length mask indicates 'all good').

		} else {

			data = latt.getSlice(start, sliceShape, stride, true);
			mask = latt.getMaskSlice(start, sliceShape, stride, true);
			// These will be Nx1 in any degenerate axis case.
			// (If 1xN had been needed, needToTranspose() would
			// also have been true).
			if (!anyEQ(mask, false)) mask.resize(0,0);
			// (indicates 'all good').
		}

		return true;
	}

} //# NAMESPACE CASA - END

