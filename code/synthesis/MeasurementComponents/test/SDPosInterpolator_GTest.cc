//# SDPosInterpolator_GTest.cc: this defines unit tests of
//# SDPosInterpolator using google test framework
//#
//# Copyright (C) 2016
//# National Astronomical Observatory of Japan
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
#include <iostream>
#include <list>
#include <cassert>
#include <cmath>

#include <gtest/gtest.h>

#include <casa/Arrays/Vector.h>
#include <synthesis/MeasurementComponents/SDPosInterpolator.h>
#include <tables/Tables/Table.h>

using namespace casa;
using namespace std;

class SDPosInterpolatorTest : public ::testing::Test {
protected:
  SDPosInterpolatorTest() : verbose_(false) {}

  virtual void SetUp() {
    // Initialize sakura
    //LIBSAKURA_SYMBOL(Status) status = LIBSAKURA_SYMBOL(Initialize)(nullptr, nullptr);
    //ASSERT_EQ(LIBSAKURA_SYMBOL(Status_kOK), status);
  }

  virtual void TearDown() {
    // Clean-up sakura
    //LIBSAKURA_SYMBOL(CleanUp)();
  }

  bool verbose_;

  // Resize data vectors
  void resizeData(const Int nant, const Int ndata, const Int ndim,
                  Vector<Vector<Double> >& in_time,
                  Vector<Vector<Vector<Double> > >& in_dir,
                  Vector<Vector<Vector<Vector<Double> > > >& out_coeff,
                  Vector<Vector<Vector<Vector<Double> > > >& ref_coeff) {
    in_time.resize(nant);
    in_time(0).resize(ndata);
    
    in_dir.resize(nant);
    in_dir(0).resize(ndata);
    for (Int i = 0; i < ndata; ++i) {
      in_dir(0)(i).resize(ndim);
    }

    out_coeff.resize(nant);
    ref_coeff.resize(nant);

    Int const nsection = ndata - 1;
    out_coeff(0).resize(nsection);
    ref_coeff(0).resize(nsection);
    for (Int i = 0; i < nsection; ++i) {
      out_coeff(0)(i).resize(ndim);
      ref_coeff(0)(i).resize(ndim);
      for (Int j = 0; j < ndim; ++j) {
        out_coeff(0)(i)(j).resize(4);
        ref_coeff(0)(i)(j).resize(4);
      }
    }
  }

  // Set input data value (y=x*x)
  void setData(Int ndata) {
    for (Int i = 0; i < ndata; ++i) {
      in_time(0)(i) = static_cast<Double>(i);
      in_dir(0)(i)(0) = static_cast<Double>(i);
      in_dir(0)(i)(1) = static_cast<Double>(i*i);
    }
  }
  
  // Check if the expected and actual values are enough close to each other
  void checkAlmostEqual(Double expected, Double actual, Double tolerance) {
    Double deviation = fabs(actual - expected);
    Double val = max(fabs(actual), fabs(expected)) * tolerance + tolerance;
    ASSERT_LE(deviation, val);
  }

  Double tol = 1.0e-6;
  Int const nant = 1;
  Int const ndim = 2;
  Vector<Vector<Double> > in_time;
  Vector<Vector<Vector<Double> > > in_dir;
  Vector<Vector<Vector<Vector<Double> > > > out_coeff;
  Vector<Vector<Vector<Vector<Double> > > > ref_coeff;
};

/*
 * Testing Spline interpolation for 4 data
 */
TEST_F(SDPosInterpolatorTest, SplineInterpolation4Points) {
  /*
    index i : 0, 1, 2, 3
    input time data (ti): 0, 1, 2, 3
    input direction data (xi) : 0, 1, 2, 3
    input direction data (yi) : 0, 1, 4, 9 (=xi*xi)
    answer spline coefficients (x) : (x = ax(t-ti)^3 + bx(t-ti)^2 + cx(t-ti) + dx)
      at the 0th section (0<=t<1) : ax =    0, bx =   0, cx =    1, dx = 0
      at the 1st section (1<=t<2) : ax =    0, bx =   0, cx =    1, dx = 1
      at the 2nd section (2<=t<3) : ax =    0, bx =   0, cx =    1, dx = 2
    answer spline coefficients (y) : (y = ay(t-ti)^3 + by(t-ti)^2 + cy(t-ti) + dy)
      at the 0th section (0<=t<1) : ay =  2/5, by =   0, cy =  3/5, dy = 0
      at the 1st section (1<=t<2) : ay =    0, by = 6/5, cy =  9/5, dy = 1
      at the 2nd section (2<=t<3) : ay = -2/5, by = 6/5, cy = 21/5, dy = 4
                                    
   */
  Int const ndata = 4;
  Int const nsection = ndata - 1;
  resizeData(nant, ndata, ndim, in_time, in_dir, out_coeff, ref_coeff);
  setData(ndata);
  
  SDPosInterpolator interp = SDPosInterpolator(in_time, in_dir);
  out_coeff = interp.getSplineCoeff();

  for (Int i = 0; i < nant; ++i) {
    //(x) : identical for all sections
    for (Int j = 0; j < nsection; ++j) {
      ref_coeff(i)(j)(0)(0) = static_cast<Double>(j);
      ref_coeff(i)(j)(0)(1) = 1.0;
      ref_coeff(i)(j)(0)(2) = 0.0;
      ref_coeff(i)(j)(0)(3) = 0.0;
    }
    //0th section (y)
    ref_coeff(i)(0)(1)(0) = 0.0;
    ref_coeff(i)(0)(1)(1) = 3.0/5.0;
    ref_coeff(i)(0)(1)(2) = 0.0;
    ref_coeff(i)(0)(1)(3) = 2.0/5.0;
    //1st section (y)
    ref_coeff(i)(1)(1)(0) = 1.0;
    ref_coeff(i)(1)(1)(1) = 9.0/5.0;
    ref_coeff(i)(1)(1)(2) = 6.0/5.0;
    ref_coeff(i)(1)(1)(3) = 0.0;
    //2nd section (y)
    ref_coeff(i)(2)(1)(0) = 4.0;
    ref_coeff(i)(2)(1)(1) = 21.0/5.0;
    ref_coeff(i)(2)(1)(2) = 6.0/5.0;
    ref_coeff(i)(2)(1)(3) = -2.0/5.0;
  }
  

  for (Int i = 0; i < nant; ++i) {
    for (Int j = 0; j < nsection; ++j) {
      for (Int k = 0; k < ndim; ++k) {
        for (Int l = 0; l < 4; ++l) {
          checkAlmostEqual(ref_coeff(i)(j)(k)(l), out_coeff(i)(j)(k)(l), tol);
        }
      }
    }
  }
  
}

/*
 * Testing Spline interpolation for 5 data
 */
TEST_F(SDPosInterpolatorTest, SplineInterpolation5Points) {
  /*
    index i : 0, 1, 2, 3, 4
    input time data (ti): 0, 1, 2, 3, 4
    input direction data (xi) : 0, 1, 2, 3, 4
    input direction data (yi) : 0, 1, 4, 9, 16 (=xi*xi)
    answer spline coefficients (x) : (x = ax(t-ti)^3 + bx(t-ti)^2 + cx(t-ti) + dx)
      at the 0th section (0<=t<1) : ax =    0, bx =   0, cx =    1, dx = 0
      at the 1st section (1<=t<2) : ax =    0, bx =   0, cx =    1, dx = 1
      at the 2nd section (2<=t<3) : ax =    0, bx =   0, cx =    1, dx = 2
      at the 3rd section (3<=t<4) : ax =    0, bx =   0, cx =    1, dx = 3
    answer spline coefficients (y) : (y = ay(t-ti)^3 + by(t-ti)^2 + cy(t-ti) + dy)
      at the 0th section (0<=t<1) : ay =  3/7, by =   0, cy =  4/7, dy = 0
      at the 1st section (1<=t<2) : ay = -1/7, by = 9/7, cy = 13/7, dy = 1
      at the 2nd section (2<=t<3) : ay =  1/7, by = 6/7, cy =    4, dy = 4
      at the 3rd section (3<=t<4) : ay = -3/7, by = 9/7, cy = 43/7, dy = 9
   */
  Int const ndata = 5;
  Int const nsection = ndata - 1;
  resizeData(nant, ndata, ndim, in_time, in_dir, out_coeff, ref_coeff);
  setData(ndata);
  
  SDPosInterpolator interp = SDPosInterpolator(in_time, in_dir);
  out_coeff = interp.getSplineCoeff();

  for (Int i = 0; i < nant; ++i) {
    //(x) : identical for all sections
    for (Int j = 0; j < nsection; ++j) {
      ref_coeff(i)(j)(0)(0) = static_cast<Double>(j);
      ref_coeff(i)(j)(0)(1) = 1.0;
      ref_coeff(i)(j)(0)(2) = 0.0;
      ref_coeff(i)(j)(0)(3) = 0.0;
    }
    //0th section (y)
    ref_coeff(i)(0)(1)(0) = 0.0;
    ref_coeff(i)(0)(1)(1) = 4.0/7.0;
    ref_coeff(i)(0)(1)(2) = 0.0;
    ref_coeff(i)(0)(1)(3) = 3.0/7.0;
    //1st section (y)
    ref_coeff(i)(1)(1)(0) = 1.0;
    ref_coeff(i)(1)(1)(1) = 13.0/7.0;
    ref_coeff(i)(1)(1)(2) = 9.0/7.0;
    ref_coeff(i)(1)(1)(3) = -1.0/7.0;
    //2nd section (y)
    ref_coeff(i)(2)(1)(0) = 4.0;
    ref_coeff(i)(2)(1)(1) = 4.0;
    ref_coeff(i)(2)(1)(2) = 6.0/7.0;
    ref_coeff(i)(2)(1)(3) = 1.0/7.0;
    //3rd section (y)
    ref_coeff(i)(3)(1)(0) = 9.0;
    ref_coeff(i)(3)(1)(1) = 43.0/7.0;
    ref_coeff(i)(3)(1)(2) = 9.0/7.0;
    ref_coeff(i)(3)(1)(3) = -3.0/7.0;
  }

  // verify the results
  for (Int i = 0; i < nant; ++i) {
    for (Int j = 0; j < nsection; ++j) {
      for (Int k = 0; k < ndim; ++k) {
        for (Int l = 0; l < 4; ++l) {
          checkAlmostEqual(ref_coeff(i)(j)(k)(l), out_coeff(i)(j)(k)(l), tol);
        }
      }
    }
  }
  
}

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "SDPosInterpolatorTest test " << endl;
    return RUN_ALL_TESTS();
}
