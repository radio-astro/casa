//# tLUdecomp.cc: Test program for LU matrix decomposition
//# Copyright (C) 1994,1995,2000
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <aips/LUdecomp.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/BasicMath/Random.h>
#include <aips/LAPACK.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <math.h>
#include <iostream.h>

// ========================temp fix (system dependent)===================
#if defined (AIPS_SOLARIS)
extern "C" int gettimeofday(struct timeval *tp);
#endif
 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

#define RAND_MAX 32767
//#define DEBUG 
// ========================end temp fix ===============================


uInt getTime()
{
#if defined (AIPS_SOLARIS)
  struct timeval tp;
  gettimeofday(&tp);
#else
  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp, &tzp);
#endif
  return tp.tv_sec;
}

#define LDA_MAX 132

float norm1(const Matrix<float> &A)
{
  // Determine the norm of A.
            
  float out = 0;
  float value;
  int M = A.nrow();
  int N = A.ncolumn();
  for(int i=0;i<N;i++) {  
    value = 0;
    for (int j=0;j<M;j++) value += fabs(A(j,i)); 
    out = (value>out)? value:out;
  }
  return out;
}

main()
{
  Int lda = LDA_MAX;
  Block<Int> MNval(6);
  MNval[0] = 1;
  MNval[1] = 2;
  MNval[2] = 3;
  MNval[3] = 5;
  MNval[4] = 10;
  MNval[5] = 16;
  
  // The number of Matrix "types" to use.
  
  const uInt nmatrix = 8;

  //    The number of right hand sides to use.
  
  const uInt nrhs = 2;
  
  //    Initialize constants.
  
  const float THRESH = 30.0;
  const float EPS = FLT_EPSILON;
  const float SMALL = FLT_MIN;
  const float LARGE = 1.0 / SMALL;
  const float BADC2 = 1.19209E+06;
  const float BADC1 = sqrt( BADC2 );
  
  Int nrun = 0;
  Int nfail = 0;
  
  try{

    // general matrices		

    //  Make a place to hold test results.
	    
    Block<float> resid(5,float(0));

    Int M, N, NT = 0;

    // Do for each value of M in MNval

    int numMN = MNval.nelements();

    for (Int im = 0; im < numMN; im++){         
      M = MNval[im];
      lda = (M>=1) ? M:1 ;

      // Do for each value of N in MNval

      for (Int in = 0; in < numMN; in++){       
	N = MNval[in];

	uInt nmat = nmatrix;

	// don't test 1x1 matrices

	if((M==1) && (N==1)) nmat = 0;

	// Do for each type of matrix

	for (Int imat = 1;imat <= nmat; imat++){ 
	  
	  // Skip types 5, 6, or 7 if the matrix size is too small.
	  
	  int zerot = ((imat>=5)&&(imat<=7));
	  if ((zerot==0) || (N >= imat-5)){       
	    
	    // Generate a general M x N matrix.

	    Matrix<float> A(M,N);

	    // Set the lower and upper bandwidths.
	    // Set the condition number and norm.

	    Int KL, KU;
	    float cndnum, anorm;
	    float anorm0 = 0;
	    float anormi = 0;
	    float xnorm, bnorm, value;

	    switch(imat) {
	    case 1:
	      KL = 0;
	      KU = 0;
	      cndnum = 2.0;
	      anorm = 1.0;
	      break;
	    case 2:
	      KL = 0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = 2.0;
	      anorm = 1.0;
	      break;
	    case 3:
	      KL = (M-1>= 0)? M-1:0;
	      KU = 0;
	      cndnum = 2.0;
	      anorm = 1.0;
	      break;
	    case 4:
	      KL = (M-1>=0)? M-1:0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = 2.0;
	      anorm = 1.0;
	      break;
	    case 5:
	      KL = (M-1>=0)? M-1:0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = BADC1;
	      anorm = 1.0;
	      break;
	    case 6:
	      KL = (M-1>=0)? M-1:0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = BADC2;
	      anorm = 1.0;
	      break;
	    case 7:
	      KL = (M-1>=0)? M-1:0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = 2.0;
	      anorm = SMALL;
	      break;
	    case 8:
	      KL = (M-1>=0)? M-1:0;
	      KU = (N-1>=0)? N-1:0;
	      cndnum = 2.0;
	      anorm = LARGE;
	      break;
	    }

	    if(N<=1) cndnum = 1.0;
	    
	    // Store A's pointer for FORTRAN call
	    
	    Bool aflag;
	    float *a = A.getStorage(aflag);	
	    
	    Int INFO;
	    
	    sblda( M, N, cndnum, anorm, KL, KU, a, INFO); 
	    
	    // Check error code from SBLDA.

	    if(INFO!=0) {
	      char errcode[7];
	      sprintf(errcode,"%i",INFO);
	      throw(ArrayError(String("SBLDA failed. INFO = ") + 
			       String(errcode)));
	    }	    

	    // Restore to C++ object
	    
	    A.putStorage(a, aflag);

	    if(aips_debug)
	      { //kludge
		cout << "imat = "<< imat << " M = "<< M <<" N = " << N <<endl;

		for (int i =0;i<M;i++) {
		  for(int j=0;j<N;j++) cout << A(i,j) << " ";
		  cout << endl;
		}
	      }

	    // Determine the norm of A.
	    
	    anorm0 = norm1(A);

	    // Compute the LU factorization of the matrix.
	    
	    LUdecomp<float> LU(A);
	    

//    TEST 1 - Reconstruct matrix from factors and compute residual.

	    // Compute the product L*U and write AFAC with the result.
	    
	    Matrix<float> lower(LU.getLower());
	    if(aips_debug)
	      { //kludge
		cout << "Lower matrix =" << endl;
		for (int i =0;i<lower.nrow();i++) {
		  for(int j=0;j<lower.ncolumn();j++) cout << lower(i,j) << " ";
		  cout << endl;
		}
	      }

	    Matrix<float> upper(LU.getUpper());
	    if(aips_debug)
	      { //kludge
		cout << "Upper matrix =" << endl;
		for (int i =0;i<upper.nrow();i++) {
		  for(int j=0;j<upper.ncolumn();j++) cout << upper(i,j) << " ";
		  cout << endl;
		}
	      }
	    Matrix<float> Perm(LU.getPerm());
	    if(aips_debug)
	      { //kludge
		cout << "Permutation matrix =" << endl;
		for (int i =0;i<Perm.nrow();i++) {
		  for(int j=0;j<Perm.nrow();j++) cout << Perm(i,j) << " ";
		  cout << endl;
		}
	      }
	    Matrix<float> Afac(M,N);
	    if (Perm.nelements() < 2){
	      Afac = product(lower,upper);
	    }else{
	      Matrix<float> LUProduct(product(lower,upper));
	      Afac = product(Perm,LUProduct);
	    }
	    
	    // Compute the difference P*L*U - A  and store in AFAC.
	    
	    Afac -= A;
	    
	    // Compute norm( L*U - A ) / ( N * norm(A) * EPS )

	    resid[0] = norm1(Afac);

	    if ( anorm0 <= 0 ) {
	      if( resid[0] != 0 ) resid[0] = 1.0 / EPS;
	    } else {
	      resid[0] /= ((float)N * anorm0  * EPS);
	    }

	    NT = 1;
	    
//    TEST 2 - Form the inverse if the factorization was successful
//             and compute the residual.


	    if( resid[0] <= THRESH && M==N ){

	       float rcond0;

	       Matrix<float> Ainv(inverse(LU));
	      
	       // Compute the 1-norm condition number of A.
	       
	       float ainvnm0 = norm1(Ainv);
	       
	       // Set RESID[1] = 1/EPS if ANORM0 = 0 or AINVNM0 = 0.
	      
	       if( anorm0<=0 || ainvnm0<=0 ){
		 rcond0 = 0;
		 resid[1] = 1.0 / EPS;
	       } else { 
		 rcond0 = 1.0 / (anorm0 * ainvnm0);
		
		 // Compute I - A * AINV and store in WORK

		 Matrix<float> Ident(N,N);
		 Ident = 0;
		 Ident.diagonal(0) = 1.0;

		 Matrix<float> work(Ident - product(A,Ainv));
		 
		 // Compute norm(I - A*AINV)/(N*norm(A)*norm(AINV)*EPS)

		 resid[1] = norm1(work) *  rcond0 / (EPS * (float)N );
	       }
	      	    
//+    TEST 3 - Solve and compute residual for A * X = B.

	       Matrix<float> Xact(N,nrhs);
	       // Initialize X to NRHS random vectors 		
		
	       //	    RNG *gen;  // Random Number Generator
	       //	    DiscreteUniform spewnum(-1,1,gen);
		
// =======================Temp fix========================================
	       srand(getTime());
// ======================end temp fix====================================

	       for (int i=0;i<N;i++) {
		 for(int j=0;j<nrhs;j++) {
		   Xact(i,j) = (rand() * 2.0 / (float)RAND_MAX) - 1.0;
		   // =(float)spewnum();
		 }
	       }


	       //     Multiply X by op( A ) 
	       // notes: B is MxNRHS, X is NxNRHS & A is MxN

	       Matrix<float> B(M,nrhs);

	       B = product(A,Xact);
		
	       Vector<double> ferr, berr;
	       Matrix<float> X(solve(LU, B, ferr, berr));

	       // Set RESID = 1/EPS if ANORM0 = 0.
		
	       if( anorm0<=0 ){
		 resid[2] = 1.0 / EPS;
	       } else {
		  
		 // Compute  B - A*X  and store in WORK

		 Matrix<float> Work(B - product(A,X));
		  
		 // Compute the maximum over the number of right hand sides 
		 // of norm(B - A*X) / ( norm(A) * norm(X) * EPS ) .
		    
		 resid[2] = 0;
		    
		 for (Int j = 0; j<nrhs; j++){
		   bnorm = xnorm = 0;
		   for (Int k = 0; k<N; k++){
		     bnorm += fabs(Work(k,j));
		     xnorm += fabs(X(k,j));
		   }
		   if( xnorm <= 0 ){
		     resid[2] = 1.0 / EPS;
		   } else {
		     value =  bnorm / (anorm0 * xnorm * EPS) ;
		     resid[2] = ( resid[2] >= value )? resid[2]: value;
		   }
		 }
	       }
	       
//+    TEST 4 - Check solution from generated exact solution.

	       // Set RESID[3] = 1/EPS if RCOND0 is invalid.
		  
	       if( rcond0 < 0 ) {
		 resid[3] = 1.0 / EPS;
	       } else {
		    
		 // Compute the maximum of              norm(X - XACT)
		 //                                  --------------------
		 //                                  ( norm(XACT) * EPS )
		 // over all the vectors X and XACT .
		    
		 resid[3] = 0; 
		 float diffnm;
		 for(Int j=0;j<nrhs;j++){
		   xnorm = fabs(max(Xact.column(j)));
		   diffnm = 0;
		   for(int i = 1;i< N;i++){
		     value = fabs( X(i,j)-Xact(i,j) );
		     diffnm = (diffnm>=value)?diffnm:value;
		   }
		   if( xnorm<=0 ) {
		     if( diffnm>0 ) resid[3] = 1.0 / EPS;
		   } else {
		     value = ( diffnm * rcond0 )/ xnorm ;
		     resid[3] = (resid[3]>=value)?resid[3]:value;
		   }
		 }
		 if( resid[3]*EPS < 1.0 ) resid[3] /= EPS;
	       }

//+    TEST 5 - Get an estimate of RCOND = 1/CNDNUM.

	       float rcond;
		
	       // Place C++ data into FORTRAN format
		
	       Bool luflag;
	       Matrix<float> LUhold(LU.getLU());
	       float *afac = LUhold.getStorage(luflag);

	       Int norm =1;
	       cndnm( norm, N, afac, lda, anorm0, rcond, INFO );
		  
	       LUhold.freeStorage(afac,luflag);

	       // Check error code from CNDNM.
		  
	       if( INFO!=0 ){
		 cerr << "Routine cndnm is freaking out." << endl;
	       }
		  
	       if( rcond>0 ) {
		 if( rcond0>0 ){
		   float max = (rcond>=rcond0)? rcond:rcond0;
		   float min = (rcond<=rcond0)? rcond:rcond0;
		   resid[4] = (max / min) - (1.0 - EPS );
		 }else{
		   resid[4] = rcond / EPS;
		 }
	       } else {
		 if( rcond0>0 ){
		   resid[4] = rcond0 / EPS;
		 } else {
		   resid[4] = 0;
		 }
	       }
	       NT = 5;
	     }
	  }
	  // Print information about the tests that did not pass
	  // the threshold.
		
	  for(Int k = 0;k< NT;k++){
	    if( resid[k]>= THRESH ){
	      if(nfail==0) {  // print header
		cerr << " Matrix Types: " << endl;
		cerr << "    1. Diagonal                         "
		  << "5. Random, cndnum = sqrt(0.1/EPS)" << endl;
		cerr << "    2. Upper triangular                 "
		  << "6. Random, cndnum = 0.1/EPS" << endl;
		cerr << "    3. Lower triangular                 "
		  << "7. Scaled near underflow" << endl;
		cerr << "    4. Random, cndnum = 2               "
		  << "8. Scaled near overflow" << endl;
		cerr << " Test Ratios: " << endl;
		cerr << "    1 : norm( L * U - A )  "
		  << "/ ( N * norm(A) * EPS )" << endl;
		cerr << "    2 : norm( I - A*AINV ) / ( N * norm(A) * "
		  << "norm(AINV) * EPS )" << endl;
		cerr << "    3 : norm( B - A * X )  / ( norm(A)*norm(X) "
		  << "* EPS )" << endl;
		cerr << "    4 : norm( X - XACT )   / ( norm(XACT) * "
		  << "cndnum * EPS )" << endl;
		cerr << "    5 : RCOND * cndnum - 1.0" << endl;
		cerr << " Messages: " << endl;
	      }
	      cerr << "     M = " << M << ", N = " << N << ", type " << imat 
		<< ", test " << k+1 << ", ratio = " << resid[k] << endl;
	      nfail++;
	    }
	  }
	  nrun += NT;
	} 
      }
    }
    
    //    Print a summary of the results.

    if(nfail>0){
      cerr << " " << nfail << " out of " << nrun 
	<< " tests failed to pass the threshold." << endl;
    } else {
      cerr << " All tests passed the threshold (" << nrun 
	<< " tests run)" << endl;
    }

    return 0;
    
  } catch (AipsError x) {
    cout << "Caught exception: " << x.getMesg() << endl;
  } 
  
} //end
