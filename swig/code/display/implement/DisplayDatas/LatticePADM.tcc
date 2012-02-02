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
LatticePADisplayMethod<T>::LatticePADisplayMethod(const uInt xAxis, 
						  const uInt yAxis,
						  const uInt mAxis,
						  const IPosition fixedPos,
						  LatticePADisplayData<T> *arDat) :
  PrincipalAxesDM(xAxis, yAxis, mAxis, (PrincipalAxesDD *)arDat) {

  // call setup which does things common to all PrincipalAxesDM objects
  setup(fixedPos);
}

template <class T>
LatticePADisplayMethod<T>::LatticePADisplayMethod(const uInt xAxis,
						  const uInt yAxis,
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
IPosition LatticePADisplayMethod<T>::dataShape() {
  Lattice<T> *latt = ((LatticePADisplayData<T> *)parentDisplayData())->
    maskedLattice();
  if (!latt) {
    throw(AipsError("LatticePADisplayMethod<T>::dataShape - "
		    "no lattice is available"));
  }
  return latt->shape();
}

template <class T>
Bool LatticePADisplayMethod<T>::dataGetSlice(Matrix<T>& data,
					     Matrix<Bool> &mask,
					     const IPosition& start,
					     const IPosition& sliceShape,
					     const IPosition& stride) 
{
  MaskedLattice<T>* latt = 
    ((LatticePADisplayData<T> *)parentDisplayData())->maskedLattice();
  if (!latt) {
    throw(AipsError("LatticePADisplayMethod<T>::dataGetSlice - "
		    "no lattice is available"));
  }
  return dataGetSlice (data, mask, start, sliceShape, stride, *latt);
}

template <class T>
Bool LatticePADisplayMethod<T>::dataGetSlice(Matrix<T>& data,
					     Matrix<Bool> &mask,
					     const IPosition& start,
					     const IPosition& sliceShape,
					     const IPosition& stride,
                                             MaskedLattice<T>& latt) {
  
  // It is assumed that sliceShape has already been trimmed so
  // that it doesn't dangle over the edge of the lattice

  data.resize(0,0);
  mask.resize(0,0);	// Reset Matrices so they can resize automatically.

  if (needToTranspose()) {
  
    Matrix<T>    tmpData(latt.getSlice(start, sliceShape, stride, True));
    Matrix<Bool> tmpMask(latt.getMaskSlice(start, sliceShape, stride, True));  
	// These will be Nx1 in a degenerate axis case.
	// (When 1xN is needed, needToTranspose() will also have been True).
    
    data = transpose(tmpData);
    if (anyEQ(tmpMask, False)) mask = transpose(tmpMask);
	// (otherwise, 0-length mask indicates 'all good').
  
  } else {
    
    data = latt.getSlice(start, sliceShape, stride, True);
    mask = latt.getMaskSlice(start, sliceShape, stride, True);
	// These will be Nx1 in any degenerate axis case.
	// (If 1xN had been needed, needToTranspose() would
	// also have been True).
    if (!anyEQ(mask, False)) mask.resize(0,0);
  				// (indicates 'all good').
  }

  return True;
}

} //# NAMESPACE CASA - END

