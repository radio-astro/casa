//# tVisCal.cc: Tests the VisCal framework
//# Copyright (C) 1995,1999,2000,2001
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

#include <casa/iostream.h>

#include<casa/Arrays/ArrayMath.h>
#include <scimath/Fitting/LinearFit.h>
#include <scimath/Functionals/CompiledFunction.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <casa/BasicMath/Math.h>
#include <casa/Arrays/MatrixMath.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casacore;


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(GlinXYphJonesTest, Basic) {

  // NB: This test merely models GlinXphJones::solveOneVB---it doesn't (yet)
  //     directly test it.
  
  Int nTime(2);  // 2 points in each fit
  Int nChan(180); // 180 "channels"

  Vector<Double> ph0(nChan);
  indgen(ph0);
  ph0+=0.5;    // Every 1 deg, starting at 0.5
  ph0*=(C::pi/180.0);  // in rad


  Matrix<Double> x(nTime,nChan,0.0),y(nTime,nChan,0.0),sig(nTime,nChan,1.0);
  Vector<Bool> mask(nChan,True);

  // Fill data for fit
  for (Int ich=0;ich<nChan;++ich) {
    x(1,ich)=cos(ph0[ich]);
    y(1,ich)=sin(ph0[ich]);
  }


  Vector<Double> Xph(nChan,0.0);
  Vector<Complex> Cph(nChan,1.0);
  Float currAmb(1.0);

  // Solve for slope in each channel
  for (Int ich=0;ich<nChan;++ich) {

    LinearFit<Double> phfitter;
    Polynomial<AutoDiff<Double> > line(1);
    phfitter.setFunction(line);

    Vector<Double> solnA;
    solnA.assign(phfitter.fit(x.column(ich),y.column(ich),sig.column(ich),&mask));
    //cout << ich << " " << solnA[1] << " " << solnA[1]*x(1,ich)/y(1,ich) << endl;
    Vector<Double> solnB;
    solnB.assign(phfitter.fit(y.column(ich),x.column(ich),sig.column(ich),&mask));
    //cout << ich << " " 
    //<< " " << solnA[1] << " " << solnA[1]*x(1,ich)/y(1,ich) 
    //<< " " << solnA[1]*solnB[1] 
    //<< " " << atan(solnA[1])-atan(1.0/solnB[1]) << endl;

    Bool shallow(abs(solnA(1))<abs(solnB(1)));

    if (shallow) 
      Xph(ich)=atan(solnA(1));
    else 
      Xph(ich)=atan(1.0/solnB(1));
      
    Cph(ich)=currAmb*Complex(DComplex(cos(Xph(ich)),sin(Xph(ich))));


    Float dang(0.0);
    if (ich>0) {
      // If Xph changes by more than pi/2, probably a ambig jump...
      dang=abs(arg(Cph(ich)/Cph(ich-1)));
      if (dang > (C::pi/2.)) {
	Cph(ich)*=-1.0;   // fix this one
	currAmb*=-1.0;    // reverse currAmb, so curr amb is carried forward
	//      cout << "  Found XY phase ambiguity jump at chan=" << ich << " in spw=" << currSpw();
      }
    }

    // Revise Xph, w/ any amb removed
    Xph(ich)=arg(Cph(ich));

    /*
    cout << boolalpha;
    cout << "ich=" << ich 
      	 << " shallow=" << shallow
	 << " Cph=" << Cph(ich) << " dang=" << dang*180/C::pi << " amb=" << currAmb
	 << " Xph=" << Xph(ich)*(180/C::pi) << "=" << arg(Cph(ich))*(180/C::pi) << " ph0=" << ph0(ich)*(180/C::pi) 
	 << " diff=" <<          (ph0(ich)-(arg(Cph(ich))))*(180/C::pi) << " " << Xph(ich)-ph0(ich)
	 << endl;
    */
  }

  Vector<Double> diff;
  diff=Xph-ph0;

  ASSERT_TRUE(allLT(abs(diff),1e-6));
  
}
