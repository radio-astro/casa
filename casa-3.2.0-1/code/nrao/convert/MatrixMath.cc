//# MatrixMath.cc: The AIPS++ Linear Algebra Functions
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
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <aips/LAPACK.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/BasicSL/String.h>
#include <stdio.h>
#include <math.h>

#define BLOCKSIZE 64  // optimal size as returned by lapack's ILAENV(1, ...)

                                      // the vector dot/scalar/inner product
template<class T> T innerProduct (const Vector<T> &A, const Vector<T> &B) {
                                   // check for correct dimensions
  if (A.conform(B) == False){
    throw(ArrayConformanceError("innerProduct - conform() error."));
  }
  T scalar = 0;   
  Int AOffset, BOffset;
  A.origin(AOffset);
  B.origin(BOffset);
  for (uInt i=0; i < A.nelements(); i++)
    scalar += A(i+AOffset)*B(i+BOffset);
  return scalar;
}

template <class T> Matrix<T> Rot3D(Int axis, T angle) {
    if (axis<0 || axis>2) { 
	throw (ArrayError("Rot3D(axis, angle): axis has to be 0 (x),"
			  " 1 (y) or 2 (z)."));
    }
    else {
	Matrix<T> Rot(3,3);
	Rot=0;
	T cosa=cos(angle); 
	T sina=sin(angle);

	Rot(axis,axis)=1;
	Rot((axis+1)%3,(axis+1)%3)=cosa;
	Rot((axis+2)%3,(axis+1)%3)=sina;
	Rot((axis+1)%3,(axis+2)%3)=-sina;
	Rot((axis+2)%3,(axis+2)%3)=cosa;
	return Rot;
    }
}

// Need to provide explicit Rot3D versions for Float and Double because 
// of something nasty like that Cfront bug

Matrix<Float> Rot3D(Int axis, Float angle) {
    if (axis<0 || axis>2) { 
	throw (ArrayError("Rot3D(axis, angle): axis has to be 0 (x),"
			  " 1 (y) or 2 (z)."));
    }
    else {
	Matrix<Float> Rot(3,3);
	Rot=0;
	Float cosa=cos(angle); 
	Float sina=sin(angle);

	Rot(axis,axis)=1;
	Rot((axis+1)%3,(axis+1)%3)=cosa;
	Rot((axis+2)%3,(axis+1)%3)=sina;
	Rot((axis+1)%3,(axis+2)%3)=-sina;
	Rot((axis+2)%3,(axis+2)%3)=cosa;
	return Rot;
    }
}

Matrix<Double> Rot3D(Int axis, Double angle) {
    if (axis<0 || axis>2) { 
	throw (ArrayError("Rot3D(axis, angle): axis has to be 0 (x),"
			  " 1 (y) or 2 (z)."));
    }
    else {
	Matrix<Double> Rot(3,3);
	Rot=0;
	Double cosa=cos(angle); 
	Double sina=sin(angle);

	Rot(axis,axis)=1;
	Rot((axis+1)%3,(axis+1)%3)=cosa;
	Rot((axis+2)%3,(axis+1)%3)=sina;
	Rot((axis+1)%3,(axis+2)%3)=-sina;
	Rot((axis+2)%3,(axis+2)%3)=cosa;
	return Rot;
    }
}
                                    

                                  // the 3-space cross/vector product
template <class T>
Vector<T> crossProduct (const Vector<T> &A, const Vector<T> &B) {
                                      // check for correct dimensions
  if (!A.conform(B)){
    throw (ArrayConformanceError("crossProduct - conform() error."));
  } else {
    if (A.nelements() != 3) 
      throw (ArrayConformanceError("crossProduct - Vector not in 3-space"));
  }
  Vector<T> result(3);
  Int AOffset, BOffset;
  A.origin(AOffset);
  B.origin(BOffset);
  result(0) = A(1+AOffset)*B(2+BOffset) - A(2+AOffset)*B(1+BOffset);
  result(1) = A(2+AOffset)*B(0+BOffset) - A(0+AOffset)*B(2+BOffset);
  result(2) = A(0+AOffset)*B(1+BOffset) - A(1+AOffset)*B(0+BOffset);
  return result;
}

                                 // matrix multiplication or cayley product
template <class T>
Vector<T> product (const Matrix<T> &A, const Vector<T> &x) {
  if (A.ncolumn() != x.nelements())
    throw (ArrayError("product - multiplication of" 
                                    " these matrices shapes is undefined"));
  Vector<T> result(A.nrow());
  Int ARowOffset, Dummy;
  A.origin(ARowOffset, Dummy);
  for (Int i = 0; i < A.nrow(); i++) 
    result(i) = innerProduct(A.row(i+ARowOffset), x);
  return result;
}

template <class T>
Vector<T> outerProduct (const Matrix<T> &A, const Vector<T> &x) {
  static count = 0;
  if(count==0) 
    cerr << "outerProduct(Matrix, Vector) is deprecated; use product(...)\n";
  return product(A,x);
}

template <class T> 
Matrix<T> product (const Vector<T> &x, const Matrix<T> &yT) {
  if (yT.nrow()!= 1) 
    throw (ArrayError("product - multiplication of" 
                                    " these matrices shapes is undefined"));
  Matrix<T> A(x.nelements(),1);
  A.column(0) = x;

  return product(A,yT);
}

template <class T>
Matrix<T> outerProduct (const Vector<T> &x, const Matrix<T> &yT) {
  static count = 0;
  if(count==0) 
    cerr << "outerProduct(Vector, Matrix) is deprecated; use product(...)\n";
  return product(x, yT);
}


                                      // Vector magnitude/norm
template<class T> T norm (const Vector<T> &A) {
  return sqrt(innerProduct(A,A));
}


                                 // matrix multiplication or cayley product
template <class T> 
Matrix<T> product (const Matrix<T> &A, const Matrix<T> &B) {
  if (A.ncolumn() != B.nrow())
    throw (ArrayError("product - multiplication of" 
                                    " these matrices shapes is undefined"));
  Matrix<T> result(A.nrow(), B.ncolumn(), 0, 0);
  Int ARowOffset, BColOffset, Dummy;
  A.origin(ARowOffset, Dummy);
  B.origin(Dummy, BColOffset);
  for (Int i = 0; i < A.nrow(); i++) 
    for (Int j = 0; j < B.ncolumn(); j++) {
      result(i,j) = innerProduct(A.row(i+ARowOffset), 
				 B.column(j+BColOffset));
    }
  return result;
}

template <class T>
Matrix<T> cayleyProduct (const Matrix<T> &A, const Matrix<T> &B) {
  static count = 0;
  if(count==0) 
    cerr << "cayleyProduct(Matrix, Matrix) is deprecated; use product(...)\n";
  return product(A,B);
}

template <class T> Matrix<T> transpose (const Matrix<T> &A) {
  Matrix<T> aT(A.ncolumn(), A.nrow());
  Int RowOffset, ColOffset;
  A.origin(RowOffset, ColOffset);
  for (uInt i=0; i<A.nrow(); i++)
    for (uInt j=0; j<A.ncolumn(); j++) aT(j,i) = A(i+RowOffset,j+ColOffset);
  return aT;
}

template<class T> Matrix<T> inverse(const Matrix<T> &A) {
  Matrix<T> dummy(A.copy());
  LUdecomp<T> myLU(dummy);
  return inverse(myLU);
}

template<class T> Matrix<T> inverse(const LUdecomp<T> &myLU) {
  Matrix<T> Ainverse(myLU.getLU());
  Int M = Ainverse.nrow();
  Int N = Ainverse.ncolumn();
  Bool Astoreflag;
  T *A = Ainverse.getStorage(Astoreflag);  
  Block<Int> thePivot(myLU.getPivot());
  Int info = 0; 

  getri(N, A, M, thePivot.storage(), info);  
  
  if (info!= 0){ 
   char badarg[7];
   String message;
   if (info < 0){
     sprintf(badarg, "%s", -info);
     message = "Function inverse()'s argument #";
     message += badarg;
     message += "is illegal.";
   }
   if (info > 0) {
     sprintf(badarg, "%s", info);
     message = "Function inverse()'s returned value for U(";
     message += badarg;
     message += ", "; 
     message += badarg;
     message += ") is zero i.e singular.";
   }
   throw(ArrayError(message));
 }
  Ainverse.putStorage(A,Astoreflag);
  return Ainverse;
}

template<class T> T determinant(const Matrix<T> &A){
  LUdecomp<T> myLU(A.copy());
  return determinant(myLU);
}

template<class T> T determinant(const LUdecomp<T> &myLU){
  Matrix<T> LU(myLU.getLU());
  if (LU.nrow() != LU.ncolumn())
    throw(ArrayError("The determinant of a non-square matrix is undefined."));

  Vector<T> diag(LU.diagonal());
  T det = T(0);
  uInt flops = 0;
  for (int i = 0;i < thePivot.nelements();i++) 
    if (thePivot[i] != i) flops++;
  det = product(diag);
  while(flops--) det *= T(-1);
  return det;
}

template<class T> Vector<T> solve(const Matrix<T> &A, const Vector<T> &y, 
                                  double &ferr, double &berr) {
 Matrix<T> B(y.nelements(),1);
 B.column(0)=y;
 Vector<double> Ferr(1);
 Vector<double> Berr(1);

 Matrix<T> X = solve(A, B, Ferr, Berr);
 
 ferr = Ferr(0);
 berr = Berr(0);

 return X.column(0);
}


template<class T> Vector<T> solve(const LUdecomp<T> &myLU, const Vector<T> &y, 
                                  double &ferr, double &berr) {
 Matrix<T> B(y.nelements(),1);
 B.column(0)=y;
 Vector<double> Ferr(1);
 Vector<double> Berr(1);

 Matrix<T> X = solve(myLU, B, Ferr, Berr);
 
 ferr = Ferr(0);
 berr = Berr(0);

 return X.column(0);
}

template<class T> Matrix<T> solve(const Matrix<T> &A, const Matrix<T> &B,
        Vector<double> &Ferr, Vector<double> &Berr) {

  LUdecomp<T> myLU(A.copy());
  Matrix<T> LU(myLU.getLU());
  Int M = LU.nrow();     
  Int N = LU.ncolumn();    // = B.nrow() = X.nrow()
  Int nrhs = B.ncolumn();  // nrhs = "number of right hand sides"

  Bool Aflag;
  T *Ahold = A.getStorage(Aflag);

  Bool AFflag;
  const T *AFhold = LU.getStorage(AFflag);

  Bool Bflag;
  T *Bhold = B.getStorage(Bflag);

  const Block<Int> thePivot(myLU.getPivot());

  Matrix<T> X(N,nrhs);
  X = T(0);
  Bool Xflag;
  T *Xhold = X.getStorage(Xflag);

  Ferr.resize(nrhs);
  Berr.resize(nrhs);
  Bool berrflag;
  double *Berrhold = Berr.getStorage(berrflag);
  Bool ferrflag;
  double *Ferrhold = Ferr.getStorage(ferrflag);

  Int diagnostic = 0;

  gesvx(N,nrhs,Ahold,M,AFhold,M,thePivot.storage(),Bhold,N,Xhold,
	N,Ferrhold,Berrhold,diagnostic);
  

  A.freeStorage(Ahold,Aflag);
  LU.freeStorage(AFhold,AFflag);
  B.freeStorage(Bhold,Bflag);
  X.putStorage(Xhold,Xflag);  
  Ferr.putStorage(Ferrhold,ferrflag);
  Berr.putStorage(Berrhold,berrflag);

  if (diagnostic != 0){
  
    Bool badarg;

    if(diagnostic < 0){ 
      badarg = True;
      diagnostic *= -1;
    } else {
      badarg = False;
    }
    char code[7];
    sprintf(code,"%i",diagnostic);
    String message;

    if ((diagnostic > 20000)&&badarg){
      sprintf(code,"%i",diagnostic-20000);
      message = "LAWRAP:xGESVX argument # ";
      message += code;
      message += " is bad.";
    } else if ((diagnostic > 10000)&&badarg){ 
      sprintf(code,"%i",diagnostic-10000);
      message = "LAWRAP:xGESVX argument # "; 
      message += code;
      message += " is bad.  Additionally, matrix A may be singular ";
      message += "within machine precision.";
    } else if (badarg){
      message = "LAWRAP:xGESVX argument # ";
      message += code;
      message += " is bad.";
    } else if (diagnostic > 20000){
      message = "LAWRAP:xGESVX has computational trouble.";
    } else if (diagnostic > 10000){
      message = "LAWRAP:xGESVX matrix A is singular within machine ";
      message += "precision.";
    } else {
      message = "LAWRAP:xGESVX matrix A factor U is singular at (";
      message += code;
      message += ", ";
      message += code;
      message += ").";
    }
    throw( ArrayError(message));
  }
  return X;
}

template<class T> Matrix<T> solve(const LUdecomp<T> &myLU, const Matrix<T> &B,
        Vector<double> &Ferr, Vector<double> &Berr) {
  Matrix<T> LU(myLU.getLU());
  Int M = LU.nrow();     
  Int N = LU.ncolumn();    // = B.nrow() = X.nrow()
  Int nrhs = B.ncolumn();  // nrhs = "number of right hand sides"

  Matrix<T> Perm(myLU.getPerm());
  Matrix<T> Lower(myLU.getLower());
  Matrix<T> Upper(myLU.getUpper());
  Matrix<T> LUProduct(product(Lower,Upper));
  Matrix<T> A(product(Perm,LUProduct));

  Bool Aflag;
  T *Ahold = A.getStorage(Aflag);

  Bool AFflag;
  const T *AFhold = LU.getStorage(AFflag);

  Bool Bflag;
  T *Bhold = B.getStorage(Bflag);

  const Block<Int> thePivot(myLU.getPivot());
  

  Matrix<T> X(N,nrhs);
  X = T(0);
  Bool Xflag;
  T *Xhold = X.getStorage(Xflag);

  Ferr.resize(nrhs);
  Berr.resize(nrhs);
  Bool berrflag;
  double *Berrhold = Berr.getStorage(berrflag);
  Bool ferrflag;
  double *Ferrhold = Ferr.getStorage(ferrflag);

  Int diagnostic = 0;

  gesvx(N,nrhs,Ahold,M,AFhold,M,thePivot.storage(),Bhold,N,Xhold,
	N,Ferrhold,Berrhold,diagnostic);
  

  A.freeStorage(Ahold,Aflag);
  LU.freeStorage(AFhold,AFflag);
  B.freeStorage(Bhold,Bflag);
  X.putStorage(Xhold,Xflag);  
  Ferr.putStorage(Ferrhold,ferrflag);
  Berr.putStorage(Berrhold,berrflag);

  if (diagnostic != 0){
  
    Bool badarg;

    if(diagnostic < 0){ 
      badarg = True;
      diagnostic *= -1;
    } else {
      badarg = False;
    }
    char code[7];
    sprintf(code,"%i",diagnostic);
    String message;

    if ((diagnostic > 20000)&&badarg){
      sprintf(code,"%i",diagnostic-20000);
      message = "LAWRAP:xGESVX argument # ";
      message += code;
      message += " is bad.";
    } else if ((diagnostic > 10000)&&badarg){ 
      sprintf(code,"%i",diagnostic-10000);
      message = "LAWRAP:xGESVX argument # "; 
      message += code;
      message += " is bad.  Additionally, matrix A may be singular ";
      message += "within machine precision.";
    } else if (badarg){
      message = "LAWRAP:xGESVX argument # ";
      message += code;
      message += " is bad.";
    } else if (diagnostic > 20000){
      message = "LAWRAP:xGESVX has computational trouble.";
    } else if (diagnostic > 10000){
      message = "LAWRAP:xGESVX matrix A is singular within machine ";
      message += "precision.";
    } else {
      message = "LAWRAP:xGESVX matrix A factor U is singular at (";
      message += code;
      message += ", ";
      message += code;
      message += ").";
    }
    throw( ArrayError(message));
  }
  return X;
}

void getri(int n, Int *A, Int lda, const Int *pivot, Int info){
  Int worksize = BLOCKSIZE*n;
  float *workspace = new float[worksize];
  sgetri(n,(float *)A, lda, pivot, workspace, worksize, info);
  delete[] workspace;
  return;
}
void getri(int n, float *A, Int lda, const Int *pivot, Int info){
  Int worksize = BLOCKSIZE*n;
  float *workspace = new float[worksize];
  sgetri(n, A, lda, pivot, workspace, worksize, info);
  delete[] workspace;
  return;
}
void getri(int n, double *A, Int lda, const Int *pivot, Int info){
  Int worksize = BLOCKSIZE*n;
  double *workspace = new double[worksize];
  dgetri(n, A, lda, pivot, workspace, worksize, info);
  delete[] workspace;
  return;
}
void getri(int n, Complex *A, Int lda, const Int *pivot, Int info){
  Int worksize = BLOCKSIZE*n;
  Complex *workspace = new Complex[worksize];
  cgetri(n, A, lda, pivot, workspace, worksize, info);
  delete[] workspace;
  return;
}
void getri(int n, DComplex *A, Int lda, const Int *pivot, Int info){
  Int worksize = BLOCKSIZE*n;
  DComplex *workspace = new DComplex[worksize];
  zgetri(n, A, lda, pivot, workspace, worksize, info);
  delete[] workspace;
  return;
}

void gesvx(Int n,Int nrhs,Int *A,Int lda,const Int *AF,Int ldaf,
           const Int *Piv,Int *B,Int ldb,Int *X,Int ldx,double *Ferr,
           double *Berr,Int info){
  float *R = new float[n];
  float *C = new float[n];
  Int *iwork = new Int[n];
  float *work = new float[4*n];
  ssolvx(n, nrhs,(float *)A, lda, (const float *)AF, ldaf, Piv, 
	 (float *)B, ldb, (float *)X, ldx, (float *)Ferr, (float *)Berr, 
	 info, R, C, work, iwork);
  delete[] R;
  delete[] C;
  delete[] iwork;
  delete[] work;
  return;
}
void gesvx(Int n, Int nrhs, float *A, Int lda, const float *AF, Int ldaf, 
           const Int *Piv, float *B, Int ldb, float *X, Int ldx, 
	   double *Ferr, double *Berr, Int info){
  float *R = new float[n];
  float *C = new float[n];
  Int *iwork = new Int[n];
  float *work = new float[4*n];
  ssolvx(n, nrhs, A, lda, AF, ldaf, Piv, B, ldb, X, ldx, (float *)Ferr,
         (float *)Berr, info, R, C, work, iwork);
  delete[] R;
  delete[] C;
  delete[] iwork;
  delete[] work;
  return;
}
void gesvx(Int n, Int nrhs, double *A, Int lda, const double *AF, Int ldaf, 
	   const Int *Piv, double *B, Int ldb, double *X, Int ldx, 
	   double *Ferr, double *Berr, Int info){
  double *R = new double[n];
  double *C = new double[n];
  Int *iwork = new Int[n];
  double *work = new double[4*n];
  dsolvx(n, nrhs, A, lda, AF, ldaf, Piv, B, ldb, X, ldx, Ferr, Berr, 
	 info, R, C, work, iwork);
  delete[] R;
  delete[] C;
  delete[] iwork;
  delete[] work;
  return;
}
void gesvx(Int n, Int nrhs, Complex *A, Int lda, const Complex *AF, Int ldaf,
	   const Int *Piv, Complex *B, Int ldb, Complex *X, Int ldx, 
	   double *Ferr, double *Berr, Int info){
  float *R = new float[n];
  float *C = new float[n];
  float *rwork = new float[n];
  Complex *work = new Complex[3*n];

  csolvx(n, nrhs, A, lda, AF, ldaf, Piv, B, ldb, X, ldx, (float *)Ferr,
         (float *)Berr, info, R, C, work, rwork);
  delete[] R;
  delete[] C;
  delete[] rwork;
  delete[] work;
  return;
}
void gesvx(Int n, Int nrhs, DComplex *A, Int lda, const DComplex *AF, 
	   Int ldaf, const Int *Piv, DComplex *B, Int ldb, DComplex *X, 
	   Int ldx, double *Ferr, double *Berr, Int info){
  double *R = new double[n];
  double *C = new double[n];
  double *rwork = new double[n];
  DComplex *work = new DComplex[3*n];  
  zsolvx(n, nrhs, A, lda, AF, ldaf, Piv, B, ldb, X, ldx, Ferr, Berr, 
	 info, R, C, work, rwork);
  delete[] R;
  delete[] C;
  delete[] rwork;
  delete[] work;
  return;
}

// This should probably move into aips.C.
// called by fortran initialization, may not be needed on all machines,
// in fact, this might cause problems on some machines.
void MAIN()
{
}
