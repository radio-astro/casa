//# tNewCalTable.cc: Test program for NewCalTable class
//# Copyright (C) 2011
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

#include <synthesis/CalTables/RIorAParray.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
//#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

// <summary>
// Test program for RIorAParray class.
// </summary>

// Control verbosity
#define RIORAPARRAYTEST_VERBOSE True

void doTest1 (Bool verbose=False) {

  cout << "---doTest1--BEGIN--" << endl;

  {  
    Matrix<Float> fl(2,3);
    fl(0,0)=1.0;  fl(1,0)=0.0;
    fl(0,1)=1.0;  fl(1,1)=C::pi_2;
    fl(0,2)=5.0;  fl(1,2)=-C::pi+1.0e-6;
    
    cout << "fl = " << fl << endl;
    
    RIorAPArray a(fl);
    RIorAPArray b(a.c());  // convert from a

    if (verbose) {
      a.state(True);
      b.state(True);
    }


    Matrix<Float> fl1;
    fl1=b.f(False);

    if (verbose)
      cout << "new fl=" << fl1 << endl;

    Matrix<Float> fldiff;
    fldiff=fl1-fl;

    if (verbose) {
      cout << "diff= " << fldiff << endl;
    }

  }

  cout << "---doTest1---END---" << endl << endl;

}

void doTest2 (Bool verbose=False) {

  cout << "---doTest2--BEGIN--" << endl;

  {
    
    uInt n(5);
    Vector<Complex> gain(n);
    for (uInt i=0;i<n;++i) {
      Float ph=(-178.0-Float(i))*C::pi/180.0;
      gain(i)=Complex(cos(ph),sin(ph));
    }

    cout.precision(10);
    cout << "gain = " << gain << endl;

    RIorAPArray a(gain);   // ctor from Complex

    cout << endl << "a.state():" << endl;
    a.state(True);

    RIorAPArray b(a.f(True));  // ctor from Float
    cout << "b.state(True):" << endl;
    b.state(True);

    Vector<Complex> gain2(b.c());
    cout << "gain2.shape() = " << gain2.shape() << " gain2.data() = " << gain2.data() << endl;
    cout << "gain2 = " << gain2 << endl;
    cout << "gain  = " << gain << endl;

    Vector<Complex> diff;
    diff=gain2-gain;

    cout << "diff=" << diff << endl;
    cout << "amplitude(diff)=" << amplitude(diff) << endl;




    // .reform(gain.shape())

  }
  cout << "---doTest2---END---" << endl << endl;

}

void doTest3 () {

  // This demos reference semantics of Array initialization

  Vector<Float> a(6,0.0);
  Vector<Float>& b(a);   // same object as a (c++ reference)
  Vector<Float> c(a);    // new object, same data (implicit Array data reference)
  Vector<Float> d;       // new object, 
  d.reference(a);        //    ...same data  (explicit Array data reference)
  Vector<Float> e=a;     // new object, same data?!  (??)
  Vector<Float> f;       // new object,
  f=a;                   //   ...new data (Array::operater= is deep copy)

  cout << endl << "Originals: " << endl;
  cout << "a = " << a << endl;
  cout << "b = " << b << endl;
  cout << "c = " << c << endl;
  cout << "d = " << d << endl;
  cout << "e = " << e << endl;
  cout << "f = " << f << endl;

  a(0)=1.0;    // set in a, expect in b,c,d (and e? ...yes)
  b(1)=2.0;    // set in b, expect in a,c,d (and e? ...yes)
  c(2)=3.0;    // set in c, expect in a,b,d (and e? ...yes)
  d(3)=4.0;    // set in d, expect in a,b,c (and e?  ...yes)
  e(4)=999.0;  // set in e, (expect elsewhere?  ...yes: a,b,c,d)
  f(5)=5.0;    // set in f, (expect elsewhere?  ...no)

  cout << endl << "After setting: " << endl;
  cout << "a = " << a << endl;
  cout << "b = " << b << endl;
  cout << "c = " << c << endl;
  cout << "d = " << d << endl;
  cout << "e = " << e << endl;
  cout << "f = " << f << endl;

  cout << boolalpha;

  // Check object addresses:
  cout << endl << "Object addresses:  (==&a?)" << endl;
  cout << "&a = " << &a << endl;
  cout << "&b = " << &b << " " << (&b==&a) << endl;
  cout << "&c = " << &c << " " << (&c==&a) << endl;
  cout << "&d = " << &d << " " << (&d==&a) << endl;
  cout << "&e = " << &e << " " << (&e==&a) << endl;
  cout << "&f = " << &f << " " << (&f==&a) << endl;

  // Check data addresses:
  cout << endl << "Data addresses:  (==a.data()?)" << endl;
  cout << "a.data() = " << a.data() << endl;
  cout << "b.data() = " << b.data() << " " << (b.data()==a.data()) << endl;
  cout << "c.data() = " << c.data() << " " << (c.data()==a.data()) << endl;
  cout << "d.data() = " << d.data() << " " << (d.data()==a.data()) << endl;
  cout << "e.data() = " << e.data() << " " << (e.data()==a.data()) << endl;
  cout << "f.data() = " << f.data() << " " << (f.data()==a.data()) << endl;

  cout << "a.nrefs() = " << a.nrefs() << ":  " 
       << "a "
       << ( (&b!=&a && b.data()==a.data()) ? "b " : "")
       << ( (&c!=&a && c.data()==a.data()) ? "c " : "")
       << ( (&d!=&a && d.data()==a.data()) ? "d " : "")
       << ( (&e!=&a && e.data()==a.data()) ? "e " : "")
       << ( (&f!=&a && f.data()==a.data()) ? "f " : "")
       << endl;
  
}

int main ()
{
  try {

    doTest1(RIORAPARRAYTEST_VERBOSE);
    doTest2(RIORAPARRAYTEST_VERBOSE);

  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  //  if (foundError) {
  //    aexit(1);
  //  }
  cout << "OK" << endl;
  exit(0);
};
