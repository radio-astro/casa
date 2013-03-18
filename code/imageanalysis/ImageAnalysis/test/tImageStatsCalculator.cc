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


#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <images/Images/FITSImage.h>
#include <casa/namespace.h>
#include <casa/OS/EnvVar.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}


int main() {
	Bool ok = True;
	try {
         String casapath = EnvironmentVariable::get("CASAPATH");
         if (casapath.empty()) {
            cerr << "CASAPATH env variable not defined. Can't find fixtures. Did you source the casainit.(c)sh file?" << endl;
            return 1;
        }

        String *parts = new String[2];
        split(casapath, parts, 2, String(" "));
        String datadir = parts[0] + "/data/regression/unittest/imageanalysis/ImageAnalysis/";
        delete [] parts;
		writeTestString("Verify fix for CAS-2195: error if image has no direction coordinate but does have linear coordiante");
		FITSImage fits(datadir + "/linearCoords.fits");
		Vector<Int> axes(2);
		axes[0] = 0;
		axes[1] = 1;
		ImageStatsCalculator calculator(&fits, 0, "", False);
		calculator.setAxes(axes);
		Record statsOut = calculator.statistics();
		Vector<Int> got;
		Vector<Int> expected(2);
		expected[0] = 4;
		expected[1] = 0;
		statsOut.get(RecordFieldId("minpos"), got);
		AlwaysAssert(got.size() == expected.size(), AipsError);
		for (uInt i = 0; i<expected.size(); i++) {
			AlwaysAssert(got[i] == expected[i], AipsError);
		}
		expected[0] = 3;
		expected[1] = 10;
		statsOut.get(RecordFieldId("maxpos"), got);
		AlwaysAssert(got.size() == expected.size(), AipsError);
		for (uInt i = 0; i<expected.size(); i++) {
			AlwaysAssert(got[i] == expected[i], AipsError);
		}

        cout << "ok" << endl;
	}
    catch (AipsError x) {
    	ok = False;
        cerr << "Exception caught: " << x.getMesg() << endl;
    }
	return ok ? 0 : 1;
}

