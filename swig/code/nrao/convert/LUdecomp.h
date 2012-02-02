//# LUdecomp.h: LU matrix decomposition
//# Copyright (C) 1994,1995,1999
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

#ifndef NRAO_LUDECOMP_H
#define NRAO_LUDECOMP_H
 
//# Includes
#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Matrix.h>

#define AIPS_ARRAY_INDEX_CHECK

// On suns, at least, this needs to link with:  lapack.a blas.a
// Since this links fortran libraries, check also whether "underscores"
// are needed for FORTRAN on your machine, and whether a FORTRAN MAIN is 
// needed to initialize the fortran libraries (as it is on suns).

// This is needed on Sun machines, likely not on IBM. Should be set in
// makedefs.

// LUdecomp objects are a 2-D template matrix of numeric type which
// contains (in packed form) a Lower Triangular (L) and Upper Triangular (U)
// factorization of some numeric type matrix (A) and a block of integer type
// which contains (in packed form) the Permutation Matrix (P), i.e. A=PLU.  
// The data members are filled by the LUdecomp constructors calling LAPACK's 
// Fortran xGETRF subroutine.

template<class T> class LUdecomp
{
public:
  LUdecomp<T>();                      // 0-length Matrices
  LUdecomp<T>(Matrix<T> &);
  LUdecomp<T>(const LUdecomp<T> &);
  // The copy constructor uses reference semantics.
 
  Matrix<T> getUpper() const;
  Matrix<T> getLower() const;
  Matrix<T> getPerm() const;

  Matrix<T> getLU() const {return LU.copy();}
  Block<Int> getPivot() const {return thePivot;}

private:
    Matrix<T> LU;
    Block<Int> thePivot;
};

#endif
