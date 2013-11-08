//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LCLELMask.h>

#include <casa/namespace.h>

void testExtend() {
    CoordinateSystem cSys = CoordinateUtil::defaultCoords4D();
    PagedImage<Float> pa(IPosition(4,10,11,4, 13), cSys, "tSubImage_tmp.pa");
    CoordinateSystem maskCsys = CoordinateUtil::defaultCoords3D();
    PagedImage<Float> maskImage(IPosition(3, 10, 11, 1), maskCsys, "mask_tmp.pa");
    maskImage.flush();
    Array<Float> arr(pa.shape());
    indgen(arr);
    pa.put (arr);
    Array<Float> maskarr(maskImage.shape());
    indgen(maskarr);
    maskImage.put (maskarr);
    Record empty;
    String maskExpr = "mask_tmp.pa > 10";
    SubImage<Float> myim2 = SubImageFactory<Float>::createSubImage(
    	pa, empty, maskExpr, new LogIO(), False,
    	AxesSpecifier(), True
    );
    Array<Bool> got = myim2.getMask();
    LatticeExprNode x(ArrayLattice<Float>(maskImage.get()) > 10);
    LatticeExpr<Bool> kk(x);
    LCLELMask lel(kk);
    ExtendLattice<Bool> z(lel, pa.shape(), IPosition(1, 2), IPosition(1, 3));
    AlwaysAssert(got.shape().isEqual(z.shape()), AipsError);
    for (uInt i=0; i<z.shape().product(); i++) {
    	AlwaysAssert(z.get().data()[i] == got.data()[i], AipsError);
    }
}

int main () {
  try {
    // test extending mask
    testExtend();
  }
  catch (AipsError x) {
    cerr << "Caught exception: " << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 

  cout << "OK" << endl;
  return 0;
}
