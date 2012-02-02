//# MatrixMath.h: The AIPS++ Linear Algebra Functions
//# Copyright (C) 1994,1995,1999,2000
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

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>

#define AIPS_ARRAY_INDEX_CHECK

//# Forward Declarations
template <class T> class Vector;
template <class T> class Matrix;
template <class T> class LUdecomp;

// <thrown> 
//   <item> ArrayError
//   <item> BlasError
// </thrown>

//<div><title> The AIPS++ Linear Algebra Functions </title>

// 
// The scalar/dot/inner product of two equal length vectors.
//
template <class T> T innerProduct (const Vector<T> &x, const Vector<T> &y);

//
// The magnitude/norm of a vector.
//
template <class T> T norm (const Vector<T> &x);

//
// Create a 3D rotation matrix (3x3).
// Axis is 0,1,2 for x,y,z; angle is in radians.
//
template <class T> Matrix<T> Rot3D(Int axis, T angle);

//
// The vector/cross product of two 3-space vectors.
//
template <class T> 
   Vector<T> crossProduct (const Vector<T> &x, const Vector<T> &y);

//
// The matrix/outer product of a vector and a transposed vector. 
// <note> The function's second argument is actually a transposed vector
// stored as the only row in a 1xN matrix.
// </note>
//
template <class T>
   Matrix<T> product (const Vector<T> &x, const Matrix<T> &yT);
template <class T>
   Matrix<T> outerProduct (const Vector<T> &x, const Matrix<T> &yT);

//
// The vector/outer product of an MxN matrix and an N-length vector.
//
template <class T>
   Vector<T> product (const Matrix<T> &A, const Vector<T> &x);
template <class T>
   Vector<T> outerProduct (const Matrix<T> &A, const Vector<T> &x);

//
// The matrix multiplication or cayley product of an MxN matrix and
// an NxP matrix.
//
template <class T> 
   Matrix<T> product (const Matrix<T> &A, const Matrix<T> &B);
template <class T> 
   Matrix<T> cayleyProduct (const Matrix<T> &A, const Matrix<T> &B);

//
// The NxM transpose of an MxN matrix.
//
template <class T> Matrix<T> transpose (const Matrix<T> &A);

//
//<div> complex space function specifications
//

//
// The complex conjugate of the complex matrix A.
//
Matrix<Complex> conjugate (const Matrix<Complex> &A);

//
// The complex conjugate of the double precision complex matrix A.
//
Matrix<DComplex> conjugate (const Matrix<DComplex> &A);

//
// The conjugate/transpose or adjoint of the complex matrix A.
//
Matrix<Complex> adjoint (const Matrix<Complex> &A);

//
// The conjugate/transpose or adjoint of the double precision complex matrix A.
//
Matrix<DComplex> adjoint (const Matrix<DComplex> &A);

//</div>

// Requires linking of the LAPACK libraries. The LAPACK libraries
// can be attained by FTPing to the netlib directory at 
// Research.ATT.com or by contacting the Numerical Algorithms Group.
// +grp
//
// The inverse of a matrix.  <note>The LU decomposition of the matrix
// is a hidden calculation. </note>
//
template <class T> Matrix<T> inverse (const Matrix<T> &A);

//
// The inverse of an LUdecomp which is the Lower/upper 
// decomposition of a matrix A.
//
template <class T> Matrix<T> inverse (const LUdecomp<T> &LU);

//
// The determinant of a matrix. <note>The LU decomposition of the matrix
// is a hidden calculation. </note>
//
template <class T> T determinant (const Matrix<T> &A);

//
// the determinant (A) of an LUdecomp which is the lower/upper 
// decomposition of a matrix A.
//
template <class T> T determinant (const LUdecomp<T> &LU);


//  
// Given a matrix "A", and given some vector "y" which is the right hand 
// side of the equation "Ax=y", then "solve(A, y, error1, error2)" 
// returns the computed vector "x". (for further details, see the LAPACK 
// man page for "sgesvx".)
//
// solve(LUdecomp<T>, Vector<T>, ...) arguments are (in order):
// 1) Matrix<T> -  (input)  the matrix "A" that is being modeled by the 
//                          equation "Ax=y".
// 2) Vector<T> -  (input)  the vector "y" that is being modeled by the
//                          equation "Ax=y".
// 3) double    -  (output) the error bound "forwardError" on the returned 
//    vector x, i.e
//                                            |max(x - xtrue)|
//                         forwardError  >    ----------------
//                                                |max(x)|
//
// 4) double    -  (output) the error bound "backwardError" on the input 
//    vector "y". i.e. 
//                        backwardError  >    |Ax-y| 
//
//<note>The LU decomposition of the matrix is a hidden calculation. </note>
//
template <class T> 
   Vector<T> solve (const Matrix<T> &A, const Vector<T> &y, double &ferr,
                    double &berr);

//
// Given an LUdecomp "myLU" which is the LU decomposition of some
// matrix "A", and given some vector "y" which is the right hand side of
// the equation "Ax=y", then "solve(myLU, y, error1, error2)" returns the 
// computed vector "x". (for further details, see the LAPACK man page for
// "sgesvx".)
//
// solve(LUdecomp<T>, Vector<T>, ...) arguments are (in order):
// 1) LUdecomp<T> - (input) the LU decomposition of the matrix "A" that
//                          is being modeled by the equation "Ax=y".
// 2) Vector<T> -  (input)  the vector "y" that is being modeled by the
//                          equation "Ax=y".
// 3) double    -  (output) the error bound "forwardError" on the returned 
//    vector x, i.e
//                                            |max(x - xtrue)|
//                         forwardError  >    ----------------
//                                                |max(x)|
//
// 4) double    - (output) the error bound "backwardError" on the input 
//    vector "y". i.e.
//                        backwardError  >    |Ax-y| 
//
template <class T> 
   Vector<T> solve (const LUdecomp<T> &myLU, const Vector<T> &y, double &ferr,
                    double &berr);

//
// Given a matrix "A", and given a matrix "B" whose columns are the stored 
// set of vectors "y1", "y2", ..."yN" which are independent right hand sides 
// to the equation "Ax", e.g. "Ax=y", then "solve(A, B, Error1, Error2)" 
// returns the computed matrix "X", whose columns are the stored set of 
// vectors "x1", "x2", ..."xN" which satisfy the equation "Ax=y" for each 
// respective "y". (for further details, see the LAPACK man page for
// "sgesvx".)
//
// solve(LUdecomp<T>, Matrix<T>,...) arguments are (in order):
// 1) Matrix<T> - (input) the matrix "A" that is being modeled by the 
//    equation "Ax=y".
// 2) Matrix<T> - (input)  MxN matrix "B" of N possible M-length 
//    vectors "y1", "y2",..."yN" stored as columns in a single matrix "B" 
//    where "A*x1=y1, A*x2=y2,..."A*xN=yN" => "AX=B".
// 3) Vector<double> - (output) an N-length vector "Ferr" with error bounds 
//    for returned matrix X, i.e. 
//                                 max(X.column(i)-Xtrue.column(i))
//                     Ferr(i) >   --------------------------------
//                                         max(X.column(i)) 
// 
// 4) Vector<double> - (output) an N-length vector "Berr" with error bounds 
//    for input matrix "B", i.e
//                              Berr(i) > |max(A*X.column(i)-B.column(i))| 
//
//<note>The LU decomposition of the matrix is a hidden calculation. </note>
//
template <class T> 
   Matrix<T> solve (const Matrix<T> &A, const Matrix<T> &B, 
                    Vector<double> &Ferr, Vector<double> &Berr);

//
// Given an LUdecomp "myLU" which is the LU decomposition of some
// matrix "A", and given a matrix "B" whose columns are the stored set of 
// vectors "y1", "y2", ..."yN" which are independent right hand sides to 
// the equation "Ax", e.g. "Ax=y", then "solve(myLU, B, Error1, Error2)" 
// returns the computed matrix "X", whose columns are the stored set of 
// vectors "x1", "x2", ..."xN" which satisfy the equation "Ax=y" for each 
// respective "y". (for further details, see the LAPACK man page for
// "sgesvx".)
//
// solve(LUdecomp<T>, Matrix<T>,...) arguments are (in order):
// 1) LUdecomp<T> - (input) the LU decomposition of the matrix "A" that
//                          is being modeled by the equation "Ax=y".
// 2) Matrix<T>      - (input)  MxN matrix "B" of N possible M-length 
//    vectors "y1", "y2",..."yN" stored as columns in a single matrix "B" 
//    where "A*x1=y1, A*x2=y2,..."A*xN=yN" => "AX=B".
// 3) Vector<double> - (output) an N-length vector "Ferr" with error bounds 
//    for returned matrix X, i.e. 
//                                 max(X.column(i)-Xtrue.column(i))
//                     Ferr(i) >   --------------------------------
//                                         max(X.column(i)) 
// 
// 4) Vector<double> - (output) an N-length vector "Berr" with error bounds 
//    for input matrix "B", i.e
//                              Berr(i) > |max(A*X.column(i)-B.column(i))| 
//
template <class T> 
   Matrix<T> solve (const LUdecomp<T> &myLU, const Matrix<T> &B, 
                    Vector<double> &Ferr, Vector<double> &Berr);

// -grp
// </div>

#endif
