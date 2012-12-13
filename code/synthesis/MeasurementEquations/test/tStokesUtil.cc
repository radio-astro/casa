//# tStokesVector.cc:  
//# Copyright (C) 1996,1999,2001
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
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main() {
  Bool anyFailures = False;
  /*  {
    Bool failed = False;
    const uInt nvec= 3;
    Vector<StokesVector> stokes(nvec);
    StokesVector iquv(0, 1, 2, 3), cs(4,4,4,4);
    for (uInt i = 0; i < nvec ; i++){
      stokes(i) = iquv;
      iquv += cs;
    }
    Array<Float> result;
    unpackStokes(result, stokes);
    Matrix<Float> expectedResult(nvec, 4);
    expectedResult(0,0) = 0; expectedResult(1,0) = 4; expectedResult(2,0) = 8;
    expectedResult(0,1) = 1; expectedResult(1,1) = 5; expectedResult(2,1) = 9;
    expectedResult(0,2) = 2; expectedResult(1,2) = 6; expectedResult(2,2) = 10;
    expectedResult(0,3) = 3; expectedResult(1,3) = 7; expectedResult(2,3) = 11;
    if (!allNearAbs(expectedResult, result, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the StokesVector ->  Float Array Conversion Test"
	 << endl;
    // Now lets repack the array
    if (!failed){
      Vector<StokesVector> newStokes;
      repackStokes(newStokes, result);
//       cout << "result:" << result << endl;
//       cout << "newstokes:" << newStokes(1) << endl;
//       cout << "stokes:" << stokes(1) << endl;
      for (uInt i = 0; i < newStokes.nelements(); i++)
	newStokes(i)-=stokes(i);
      unpackStokes(result, newStokes);
      expectedResult = 0;
      if (!allNearAbs(result, expectedResult, 1.E-5)){
	failed = True;
	cout << "Failed";
      }
      else
	cout << "Passed";
      cout << " the Float Array -> StokesVector Conversion Test"
	   << endl;
    }
    if (failed)
      anyFailures = True;
  }
  // because of explicit typecasts the g++ compiler cannot do the 
  // following StokesVector->Double test
  {
    Bool failed = False;
    const uInt nvec = 3;
    Array<StokesVector> stokes(IPosition(2,nvec,1));
    StokesVector iquv(0, 1, 2, 3), cs(4,4,4,4);
    for (uInt i = 0; i < nvec ; i++){
      stokes(IPosition(2,i,0)) = iquv;
      iquv += cs;
    }
    Array<Double> result;
    unpackStokes(result, stokes);
    Cube<Double> expectedResult(nvec, 1, 4);
    expectedResult(0,0,0) = 0; 
    expectedResult(1,0,0) = 4; 
    expectedResult(2,0,0) = 8;
    expectedResult(0,0,1) = 1; 
    expectedResult(1,0,1) = 5; 
    expectedResult(2,0,1) = 9;
    expectedResult(0,0,2) = 2; 
    expectedResult(1,0,2) = 6; 
    expectedResult(2,0,2) = 10;
    expectedResult(0,0,3) = 3; 
    expectedResult(1,0,3) = 7; 
    expectedResult(2,0,3) = 11;

    if (!allNearAbs(expectedResult, result, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the StokesVector ->  Double Array Conversion Test"
	 << endl;
    // Now lets repack the array
    if (!failed){
      Array<StokesVector> newStokes;
      repackStokes(newStokes, result);
      for (uInt i = 0; i < newStokes.nelements(); i++)
	newStokes(IPosition(2,i,0))-=stokes(IPosition(2,i,0));
      unpackStokes(result, newStokes);
      expectedResult = 0;
      if (!allNearAbs(result, expectedResult, 1.E-5)){
	failed = True;
	cout << "Failed";
      }
      else
	cout << "Passed";
      cout << " the Double Array -> StokesVector Conversion Test"
	   << endl;
    }
    if (failed)
      anyFailures = True;
  }
  {
    Bool failed = False;
    const uInt nvec = 3;
    Array<CStokesVector> stokes(IPosition(2,nvec,1));
    CStokesVector iquv(Complex(0,0.1), Complex(1,1.1), 
		       Complex(2,2.1), Complex(3,3.1));
    CStokesVector cs(Complex(4,.05),Complex(4,.05),
		     Complex(4,.05),Complex(4,.05));
    for (uInt i = 0; i < nvec ; i++){
      stokes(IPosition(2,i,0)) = iquv;
      iquv += cs;
    }
    Array<Complex> result;
    unpackStokes(result, stokes);
    Cube<Complex> expectedResult(nvec, 1, 4);
    expectedResult(0,0,0) = Complex(0,0.1); 
    expectedResult(1,0,0) = Complex(4,.15); 
    expectedResult(2,0,0) = Complex(8,.20);
    expectedResult(0,0,1) = Complex(1,1.1); 
    expectedResult(1,0,1) = Complex(5,1.15); 
    expectedResult(2,0,1) = Complex(9,1.2);
    expectedResult(0,0,2) = Complex(2,2.1); 
    expectedResult(1,0,2) = Complex(6,2.15); 
    expectedResult(2,0,2) = Complex(10,2.20);
    expectedResult(0,0,3) = Complex(3,3.1); 
    expectedResult(1,0,3) = Complex(7,3.15); 
    expectedResult(2,0,3) = Complex(11,3.2);
    if (!allNearAbs(expectedResult, result, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the CStokesVector ->  Complex Array Conversion Test"
	 << endl;
    // Now lets repack the array
    if (!failed){
      Array<CStokesVector> newStokes;
      repackStokes(newStokes, result);
      for (uInt i = 0; i < newStokes.nelements(); i++)
	newStokes(IPosition(2,i,0))-=stokes(IPosition(2,i,0));
      unpackStokes(result, newStokes);
      expectedResult = 0;
      if (!allNearAbs(result, expectedResult, 1.E-5)){
	failed = True;
	cout << "Failed";
      }
      else
	cout << "Passed";
      cout << " the Complex Array -> CStokesVector Conversion Test"
	   << endl;
    }
    if (failed)
      anyFailures = True;
  }
  // because of explicit typecasts the g++ compiler cannot do the 
  // following CStokesVector->DComplex test
  {
    Bool failed = False;
    const uInt nvec = 3;
    Vector<CStokesVector> stokes(nvec);
    CStokesVector iquv(Complex(0,0.1), Complex(1,1.1), 
		       Complex(2,2.1), Complex(3,3.1));
    CStokesVector cs(Complex(4,.05),Complex(4,.05),
		     Complex(4,.05),Complex(4,.05));
    for (uInt i = 0; i < nvec ; i++){
      stokes(i) = iquv;
      iquv += cs;
    }
    Array<DComplex> result;
    unpackStokes(result, stokes);
    Matrix<DComplex> expectedResult(nvec, 4);
    expectedResult(0,0) = DComplex(0,0.1); 
    expectedResult(1,0) = DComplex(4,.15); 
    expectedResult(2,0) = DComplex(8,.20);
    expectedResult(0,1) = DComplex(1,1.1); 
    expectedResult(1,1) = DComplex(5,1.15); 
    expectedResult(2,1) = DComplex(9,1.2);
    expectedResult(0,2) = DComplex(2,2.1); 
    expectedResult(1,2) = DComplex(6,2.15); 
    expectedResult(2,2) = DComplex(10,2.20);
    expectedResult(0,3) = DComplex(3,3.1); 
    expectedResult(1,3) = DComplex(7,3.15); 
    expectedResult(2,3) = DComplex(11,3.2);
    if (!allNearAbs(expectedResult, result, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the CStokesVector ->  DComplex Array Conversion Test"
	 << endl;
    // Now lets repack the array
    if (!failed){
      Vector<CStokesVector> newStokes;
      repackStokes(newStokes, result);
      for (uInt i = 0; i < newStokes.nelements(); i++)
	newStokes(i)-=stokes(i);
      unpackStokes(result, newStokes);
      expectedResult = 0;
      if (!allNearAbs(result, expectedResult, 1.E-5)){
	failed = True;
	cout << "Failed";
      }
      else
	cout << "Passed";
      cout << " the DComplex Array -> CStokesVector Conversion Test"
	   << endl;
    }
    if (failed)
      anyFailures = True;
  } 
  {
    Bool failed = False;
    StokesVector scalar(1.0f, 3.0f, 7.0f, 4.0f);
    Vector<Float> myArray(5);
    indgen(myArray);
    Vector<StokesVector> expectedResult(myArray.shape());
    for (uInt i = 0; i < expectedResult.nelements(); i++)
      expectedResult(i) = Float(i)*scalar;
    Array<StokesVector> result;
    result = myArray*scalar;
    // Convert the expected and actual result to arrays of Floats in order
    // to check if they are equal.
    Array<Float> fresult;
    unpackStokes(fresult, result);
    Array<Float> feresult;
    unpackStokes(feresult, expectedResult);
    if (!allNearAbs(fresult, feresult, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the Array<Float>*StokesVector test" << endl;
    result = scalar*myArray;
    unpackStokes(fresult, result);
    if (!allNearAbs(fresult, feresult, 1.E-5)){
      failed = True;
      cout << "Failed";
    }
    else
      cout << "Passed";
    cout << " the StokesVector*Array<Float> test" << endl;
    if (failed) 
      anyFailures = True;
  }
  */
  if (anyFailures) 
    return 1;
  else
    return 0;
} 
