//# LUdecomp.cc: LU matrix decomposition
//# Copyright (C) 1994,1995
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

//# Includes
#include <aips/LUdecomp.h>
#include <aips/LAPACK.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicSL/Complex.h>
#include <stdio.h>

template<class T> LUdecomp<T>::LUdecomp(){
// data members are created, but of zero size.
}

template<class T> LUdecomp<T>::LUdecomp(Matrix<T> &A){
 Int numrows = A.nrow();     // M
 Int numcols = A.ncolumn();  // N
 LU.resize(numrows,numcols);
 LU = A;
 thePivot.resize(numrows+1, True, False);
 for(int i =0;i<numrows+1;i++) thePivot[i]=i+1;
 Bool LUstoreflag;
 T *matrixhold = LU.getStorage(LUstoreflag);
 Int diagnostic = 0;
                                                  // das meat unt bones
 getrf(numrows, numcols, matrixhold, numrows, thePivot.storage(), diagnostic);

 LU.putStorage(matrixhold,LUstoreflag);

 if (diagnostic!= 0){ 
   char badarg[7];
   String message;
   if (diagnostic < 0){
     sprintf(badarg, "%s", -diagnostic);
     message = "LUdecomp::LUdecomp() - getrf()'s argument #";
     message += badarg;
     message += "is illegal.";
   }
   if (diagnostic > 0) {
     sprintf(badarg, "%s", diagnostic);
     message = "LUdecomp::LUdecomp() - getrf()'s returned value for U(";
     message += badarg;
     message += ", "; 
     message += badarg;
     message += ") is zero i.e singular.";
   }
   throw(ArrayError(message));
 }
}


template<class T> LUdecomp<T>::LUdecomp(const LUdecomp<T> &myPLU) {
  thePivot = myPLU.getPivot();
  LU = myPLU.getLU();
}

template<class T> Matrix<T> LUdecomp<T>::getUpper() const {
  Matrix<T> upper(LU.copy());
  Int M = upper.nrow();
  Int N = upper.ncolumn();
  if (M>N) {
    M=N;
    upper.resize(N,N);
    for (int i=0;i<N;i++)
      for (int j=0;j<N;j++) upper(i,j) = LU(i,j);
  }
  for (Int i = 1; i<M; i++) 
    for (Int j = 0; (j<i)&&(j<N); j++) upper(i,j) = T(0.0);

  return upper;
}

template<class T> Matrix<T> LUdecomp<T>::getLower() const {
  Matrix<T> lower(LU.copy());
  Int M = lower.nrow();
  Int N = lower.ncolumn();
  if (M<N) {
    N=M;
    lower.resize(M,M);
    for (int i=0;i<M;i++)
      for (int j=0;j<M;j++) lower(i,j) = LU(i,j);
  }
  for (Int i=0; i<M; i++) 
    for (Int j = i; j<N; j++){
      if (i!=j) lower(i,j) = T(0.0);
      else lower(i,j) = T(1.0);
    }

  return lower;
}

template<class T> Matrix<T> LUdecomp<T>::getPerm() const {
  Int size = LU.nrow();
/*
  if (size != thePivot.nelements()) throw(ArrayError("LUdecomp::getPerm() has"
  "failed.  Data member LUdecomp::thePivot is bad."));
*/
  Matrix<T> Perm(size,size);
  if (size>1) {
    Perm = T(0);
    Perm.diagonal(0) = T(1.0);
    Vector<T> RowHold(size);

    for(Int i=0;i<size;i++){
      if (thePivot[i] != 0)
	if (thePivot[i] <= size ){
	  RowHold = Perm.row(i);
	  Perm.row(i) = Perm.row(thePivot[i]-1);
	  Perm.row(thePivot[i]-1) = RowHold;
	}/*else{
	  throw(ArrayError("LUdecomp::getPerm() has failed.  Data member "
			   "LUdecomp.thePivot is bad."));
	}*/
    }
  }else{
    Perm = T(1.0);
  }
  return Perm;
}


//bogus overloading to use compiler to call non-templated LAPACK

void getrf(Int m, Int n, Int *A, Int lda, Int *ipiv, Int info){
  sgetrf(m, n,(float*)A, lda, ipiv, info);
}
void getrf(Int m, Int n, float *A, Int lda, Int *ipiv, Int info){
  sgetrf(m, n, A, lda, ipiv, info);
}
void getrf(Int m, Int n, double *A, Int lda, Int *ipiv, Int info){
  dgetrf(m, n, A, lda, ipiv, info);
}
void getrf(Int m, Int n, Complex *A, Int lda, Int *ipiv, Int info){
  cgetrf(m, n, A, lda, ipiv, info);
}
void getrf(Int m, Int n, DComplex *A, Int lda, Int *ipiv, Int info){
  zgetrf(m, n, A, lda, ipiv, info);
}
