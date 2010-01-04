//# tCalInterpolation.cc: Test program for CalInterpolation class
//# Copyright (C) 1997,1999,2000,2001,2002,2003,2004
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

#include <calibration/CalTables/CalInterpolation.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>

#include <casa/namespace.h>
// <summary>
// Test program for CalInterpolation class.
// </summary>

Bool foundError = False;

int main (int argc, char** argv)
{
  try {
    // Create a CalInterpolation object
    CalInterpolation calIntp;

    // Excercise the data accessor methods:
    //
    // Test setAxes(), axes()
    Vector<Int> axes(2);
    axes(0) = MSC::TIME;
    axes(1) = MSC::SPECTRAL_WINDOW_ID;
    calIntp.setAxes(axes);
    if (calIntp.axes()(0) != MSC::TIME ||
	calIntp.axes()(1) != MSC::SPECTRAL_WINDOW_ID) {
      foundError = True;
      cout << "CalInterpolation::setAxes()/axes() failed" << endl;
    };

    // Test setNpoly(), nPoly()
    calIntp.setNpoly(3);
    if (calIntp.nPoly() != 3) {
      foundError = True;
      cout << "CalInterpolation::setNpoly()/nPoly() failed" << endl;
    };

    // Test setWeighting(), weighting()
    calIntp.setWeighting(CalInterpolation::WEIGHTED);
    if (calIntp.weighting() != CalInterpolation::WEIGHTED) {
      foundError = True;
      cout << "CalInterpolation::setWeighting()/weighting() failed" << endl;
    };

    // Test setWindows(), windows()
    Vector<Quantity> windows(2);
    windows(0) = Quantity(10, "s");
    windows(1) = Quantity(1, "Hz");
    calIntp.setWindows(windows);
    if (!near(calIntp.windows()(0).getValue("ms"), 10000.0) ||
	!near(calIntp.windows()(1).getValue("kHz"), 0.001)) {
      foundError = True;
      cout << "CalInterpolation::setWindows()/windows() failed" << endl;
    };

  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  if (foundError) {
    exit(1);
  }
  cout << "OK" << endl;
  exit(0);
};
