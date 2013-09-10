//# Copyright (C) 1993,1994,1995,1997,1998,1999,2000,2001
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

#include <scimath/Mathematics/Geometry.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>

#include <casa/namespace.h>

int main() {
    try {
        Double x = 1;
        Double y = 0;
        Quantity theta(30, "deg");
        Quantity cumulative(30, "deg");
        for (uInt i=0; i<12; i++) {
        	std::pair<Double, Double> rotated = Geometry::rotate2D(x, y, theta);
        	x = rotated.first;
        	y = rotated.second;
        	AlwaysAssert(nearAbs(x, cos(cumulative.getValue("rad"))), AipsError);
        	AlwaysAssert(nearAbs(y, sin(cumulative.getValue("rad"))), AipsError);
        	cumulative += theta;
        }
        cout << "ok" << endl;
    }
    catch (const AipsError& x) {
        cerr << x.getMesg() << endl;
        return 1;
    }
    return 0;
}
