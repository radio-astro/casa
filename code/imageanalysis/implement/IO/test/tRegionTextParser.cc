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

#include <imageanalysis/IO/RegionTextParser.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/EnvVar.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

#include <casa/namespace.h>

int main() {
	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		try {
			RegionTextParser parser("x", csys);
			AlwaysAssert(False, AipsError);
		}
		catch(AipsError x) {
			cout << "Caught expected exception: " << x.getMesg() << endl;
		}
		String *parts = new String[2];
		split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
		String goodFile = parts[0]
		    + "/data/regression/unittest/imageanalysis/IO/goodAsciiAnnotationsFile.txt";
		delete [] parts;

		RegionTextParser parser(goodFile, csys);
		cout << parser.getLines() << endl;

    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    }
    cout << "ok" << endl;
    return 0;
}

