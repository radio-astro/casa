//# tImageFitter.cc:  test the PagedImage class
//# Copyright (C) 1994,1995,1998,1999,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
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
//# $Id: $


#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/TempImage.h>
#include <casa/namespace.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}


int main() {
	Bool ok = True;
	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords3D();
		TiledShape shape(IPosition(3, 20));
        SPCIIF image(new TempImage<Float>(shape, csys));
        PixelValueManipulator<Float> pvm(image, NULL, "");
        Record ret = pvm.getProfile(
        	2, "mean", "Hz", PixelValueManipulatorData::DEFAULT, NULL, ""
        );
        cout << ret << endl;
        ret = pvm.getProfile(
        	2, "mean", "GHz", PixelValueManipulatorData::DEFAULT, NULL, ""
        );
        cout << ret << endl;
        cout << "ok" << endl;
	}
    catch (const AipsError& x) {
    	ok = False;
        cerr << "Exception caught: " << x.getMesg() << endl;
    }
	return ok ? 0 : 1;
}

