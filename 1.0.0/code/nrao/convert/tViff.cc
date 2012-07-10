//# tViff.cc: Test program for Viff class
//# Copyright (C) 1993,1994,1995,1999
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
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <aips/Viff.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Constants.h>
#include <iostream.h>
#include <assert.h>

main()
{
    IPosition shape(3);
    shape = 3;

    Array<float> a(shape);
    a = 0.0;
    shape.resize(7);
    shape = 2;
    Array<float> big(shape);
    big = 1.0;

    Viff viffer;
    assert (viffer.put(a) == True);
    assert (viffer.put(big) == False);
    assert (viffer.put(a) == True);

    Int nx, ny;
    Matrix<float> m(256,256);
    m.shape(nx,ny);
    // Make a wedge in X
    // Unfortunately different rows correspond to different "x" and
    // different columns correspond to different "y"
    for (Int i=0; i < nx; i++)
	    m.row(i) = float(i);

    assert(viffer.put(m) == True);
    assert(viffer.write("xgradient.viff") == True);
    // Make a wedge in Y
    for (i=0; i < nx; i++)
	    m.column(i) = float(i);

    assert(viffer.put(m) == True);
    assert(viffer.write("ygradient.viff") == True);

    // Check get from the structure.
    Matrix<float> m2;
    assert(viffer.get(m2));

    // Check that we can read back the same stuff that we write.
    m2 = 11;
    assert(viffer.put(m2));
    assert(viffer.write("test.viff"));

    Viff viffer2; // just to be sure we're reading it
    assert(viffer2.read("test.viff"));
    Matrix<float> m3;
    assert(viffer2.get(m3));
    assert(m3 == float(11));

{
    // Create an x vs. y plot
    Vector<float> x(1000);
    indgen(x);
    x *= float(2.0*C::pi/x.nelements());
    Vector<float> y = sin(x);
    assert(viffer.put(y));
    assert(viffer.putLocations(x));
    assert(viffer.write("YvsX.viff"));
}

{
    // This shows how to put multiple plots in a file -
    Int npoints = 1000;
    Int nplots = 5;
    float phase_shift = 2.0*C::pi/nplots;

    // X coordinates - must be the same for all plots?
    Vector<float> x(npoints);
    indgen(x);
    x *= float(2.0*C::pi/npoints);

    // Y coordinates - 1 per band, shift
    Cube<float> y(npoints, 1, nplots);
    for (Int i=0; i < nplots; i++) {
	y.xyPlane(i).column(0) = sin(x + i*phase_shift);
    }
    assert(viffer.put(y));
    assert(viffer.putLocations(x));
    assert(viffer.write("YvsX2.viff"));
}
{
    // Try putting a complex viff
    Vector<Complex> y(1000);
    for (Int i=0; i < 1000; i++) {
	y(i) = Complex(i, -i);
    }
    assert(viffer.put(y));
    assert(viffer.write("complex.viff"));
    assert(viffer.read("complex.viff"));
    Vector<Complex> y2(1000);
    assert(viffer.get(y2));
    assert(y2 == y);
    
}
    cout << "You might want to try editimage/xprism3 on xgradient.viff and " <<
	"ygradient.viff\n";
    cout << "You might want to try xprism2 on complex.viff, YvsX.viff and YvsX2.viff\n";
    cout << "OK\n";
    return 0;
}
