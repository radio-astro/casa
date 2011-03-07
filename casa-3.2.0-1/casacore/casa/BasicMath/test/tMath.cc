//# tMath.cc:
//# Copyright (C) 1999,2000,2001
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
//# $Id: tMath.cc 18093 2004-11-30 17:51:10Z ddebonis $

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

#include <casa/BasicMath/Math.h>

#include <casa/namespace.h>
int main() {
  try {
    {
      Float x;
      setNaN(x);
      AlwaysAssert(isNaN(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Double x = floatNaN();
      AlwaysAssert(isNaN(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Float x = doubleNaN();
      AlwaysAssert(isNaN(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Double x;
      setNaN(x);
      AlwaysAssert(isNaN(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Float x;
      setInf(x);
      AlwaysAssert(isInf(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Double x = floatInf();
      AlwaysAssert(isInf(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Float x = doubleInf();
      AlwaysAssert(isInf(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
      Double x;
      setInf(x);
      AlwaysAssert(isInf(x), AipsError);
      AlwaysAssert(!isFinite(x), AipsError);
    }
    {
        Double epsilon = 1e-13;
    	Double x = 321.444;
        AlwaysAssert(roundDouble(x, 3, 1) == 300, AipsError);
        AlwaysAssert(roundDouble(x, 3, 2) == 320, AipsError);
        AlwaysAssert(roundDouble(x, 3, 3) == 321, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 3, 4) - 321.4) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 3, 5) - 321.44) < epsilon, AipsError);

        AlwaysAssert(roundDouble(x, 1, 1, 3.4) == 300, AipsError);
        AlwaysAssert(roundDouble(x, 2, 1, 3.4) == 320, AipsError);
        AlwaysAssert(roundDouble(x, 3, 1, 3.4) == 321, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 4, 1, 3.4) - 321.4) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 5, 1, 3.4) - 321.44) < epsilon, AipsError);

        x = 21.45554;
        AlwaysAssert(roundDouble(x, 1, 2) == 20, AipsError);
        AlwaysAssert(roundDouble(x, 2, 2) == 21, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 3, 2) - 21.5) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 4, 2) - 21.46) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 5, 2) - 21.456) < epsilon, AipsError);

        AlwaysAssert(roundDouble(x, 1, 1, 2.1) == 20, AipsError);
        AlwaysAssert(roundDouble(x, 1, 2, 2.1) == 21, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 1, 3, 2.1) - 21.5) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 1, 4, 2.1) - 21.46) < epsilon, AipsError);
        AlwaysAssert(fabs(roundDouble(x, 1, 5, 2.1) - 21.456) < epsilon, AipsError);

    	x = -11.324;

    	AlwaysAssert(roundDouble(x, 1, 2) == -10, AipsError);
    	AlwaysAssert(roundDouble(x, 2, 2) == -11, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 3, 2) - -11.3) < epsilon, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 4, 2) - -11.32) < epsilon, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 5, 2) - -11.324) < epsilon, AipsError);
     	AlwaysAssert(roundDouble(x, 1, 1, 1.1) == -10, AipsError);
    	AlwaysAssert(roundDouble(x, 1, 2, 1.1) == -11, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 1, 3, 1.1) - -11.3) < epsilon, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 1, 4, 1.1) - -11.32) < epsilon, AipsError);
    	AlwaysAssert(fabs(roundDouble(x, 1, 5, 1.1) - -11.324) < epsilon, AipsError);
    	x = -4502034;

        AlwaysAssert(roundDouble(x, 3, 1) == -5000000, AipsError);
     	AlwaysAssert(roundDouble(x, 3, 2) == -4500000, AipsError);
     	AlwaysAssert(roundDouble(x, 3, 3) == -4500000, AipsError);
     	AlwaysAssert(roundDouble(x, 3, 4) == -4502000, AipsError);
     	AlwaysAssert(roundDouble(x, 3, 5) == -4502000, AipsError);
     	AlwaysAssert(roundDouble(x, 1, 2, 5) == -5000000, AipsError);
     	AlwaysAssert(roundDouble(x, 2, 2, 5) == -4500000, AipsError);
     	AlwaysAssert(roundDouble(x, 3, 2, 5) == -4500000, AipsError);
     	AlwaysAssert(roundDouble(x, 4, 2, 5) == -4502000, AipsError);
     	AlwaysAssert(roundDouble(x, 5, 2, 5) == -4502000, AipsError);
    	x = -0.0123456;
    	AlwaysAssert(fabs(roundDouble(x, 1, 2) - -0.01) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 2, 2) - -0.012) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 3, 2) - -0.0123) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 2) - -0.01235) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 5, 2) - -0.012346) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 1, 1.2) - -0.01) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 2, 1.2) - -0.012) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 3, 1.2) - -0.0123) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 4, 1.2) - -0.01235) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 5, 1.2) - -0.012346) < epsilon, AipsError);
     	x = 0.0123456;
    	AlwaysAssert(fabs(roundDouble(x, 1, 2) - 0.01) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 2, 2) - 0.012) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 3, 2) - 0.0123) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 2) - 0.01235) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 5, 2) - 0.012346) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 1, 1.2) - 0.01) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 2, 1.2) - 0.012) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 3, 1.2) - 0.0123) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 4, 1.2) - 0.01235) < epsilon, AipsError);
     	AlwaysAssert(fabs(roundDouble(x, 4, 5, 1.2) - 0.012346) < epsilon, AipsError);
    	x = 0;
    	AlwaysAssert(roundDouble(x, 3, 2) == 0, AipsError);




    }
  }
  catch (AipsError x) {
    cerr << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 
  cout << "OK" << endl;
  return 0;
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 tMath"
// End: 
