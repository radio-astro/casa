//# tKJones: test delay term
//# Copyright (C) 2013
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

#include <synthesis/MeasurementComponents/KJones.h>

// <summary>
// Test program for KJones-related classes
// </summary>

// Control verbosity
#define KJONES_TEST_VERBOSE False


Vector<Complex> appdel(Int nchan,Float f0,Float df,Float tau, Float ftau) {

    Vector<Float> ph(nchan);
    indgen(ph);
    ph*=df;
    ph+=(f0-ftau);
    ph*=tau;
    ph*=Float(C::_2pi);
    Vector<Float> fph(nchan*2);
    fph(Slice(0,nchan,2))=cos(ph);
    fph(Slice(1,nchan,2))=sin(ph);
    Vector<Complex> cph(nchan);
    RealToComplex(cph,fph);
    return cph;
}


void testDelayFFT (Bool verbose=False) {

  {

    Int nchan0(10),nchan1(20);
    Double df0(0.1),df1(0.05);
    Double rf0(90.0), rf1(91.0);
    Float tau(0.5333);

    Cube<Complex> Vobs0(1,nchan0,1), Vobs1(1,nchan1,1);
    Vobs0.set(Complex(1.0));
    Vobs1.set(Complex(1.0));

    Vector<Complex> v0(Vobs0.xyPlane(0).row(0));
    v0*=appdel(nchan0,rf0,df0,tau,90.0);

    Vector<Complex> v1(Vobs1.xyPlane(0).row(0));
    v1*=appdel(nchan1,rf1,df1,tau,90.0);

    //    cout << "phase(Vobs0) = " << phase(Vobs0)*180/C::pi << endl;
    //    cout << "phase(Vobs1) = " << phase(Vobs1)*180/C::pi << endl;

    Double pbw(64.0);

    DelayFFT sum(90.0,df1,pbw,1,1,-1,Complex(0.0));
      
    DelayFFT delfft0(rf0,df0,pbw,Vobs0);
    delfft0.FFT();
    delfft0.shift(90.0);
    sum.add(delfft0);
    delfft0.searchPeak();

    //cout << "A0 = " << amplitude(delfft0.Vpad()) << endl;
    //cout << "ph0 = " << phase(delfft0.Vpad())*(180.0/C::pi) << endl;

    DelayFFT delfft1(rf1,df1,pbw,Vobs1);
    delfft1.FFT();
    delfft1.shift(90.0);
    sum.add(delfft1);
    delfft1.searchPeak();

    sum.searchPeak();

    //cout << "A1 = " << amplitude(delfft1.Vpad()) << endl;
    //cout << "ph1 = " << phase(delfft1.Vpad())*(180.0/C::pi) << endl;

    if (verbose) {
      cout << boolalpha;
      cout << "delay0 = " << delfft0.delay()-tau << " " 
	   << delfft0.flag() << endl;
      cout << "delay1 = " << delfft1.delay()-tau << " " 
	   << delfft1.flag() << endl;
      cout << "sum    = " << sum.delay()-tau << " " 
	   << sum.flag() << endl;
    }

    AlwaysAssert( allNearAbs(delfft0.delay(),tau,1e-5), AipsError);
    AlwaysAssert( allNearAbs(delfft1.delay(),tau,1e-5), AipsError);
    AlwaysAssert( allNearAbs(sum.delay(),tau,1e-5), AipsError);


  }
}


int main ()
{
  try {

    testDelayFFT(KJONES_TEST_VERBOSE);
    
  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  cout << "OK" << endl;
  exit(0);
};
