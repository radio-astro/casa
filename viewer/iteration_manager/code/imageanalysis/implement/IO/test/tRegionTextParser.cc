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
		Vector<Double> refVal = csys.referenceValue();
		cout << refVal << endl;
		Vector<String> units = csys.worldAxisUnits();
		units[0] = units[1] = "rad";
		csys.setWorldAxisUnits(units);
		refVal[0] = 4.296556;
		refVal[1] = 0.240673;
		csys.setReferenceValue(refVal);
		try {
			RegionTextParser parser(
				"x", csys, IPosition(4, 2000, 2000, 4, 2000),
				RegionTextParser::CURRENT_VERSION
			);
			AlwaysAssert(False, AipsError);
		}
		catch(AipsError x) {
			cout << "Caught expected exception: " << x.getMesg() << endl;
		}
		String *parts = new String[2];
		split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
		String goodFile = parts[0]
		    + "/data/regression/unittest/imageanalysis/IO/goodAsciiAnnotationsFile.txt";
		String goodFile2 = parts[0]
				    + "/data/regression/unittest/imageanalysis/IO/goodAsciiAnnotationsFile2.txt";
		delete [] parts;

		RegionTextParser parser(
			goodFile, csys, IPosition(4, 2000, 2000, 4, 2000),
			RegionTextParser::CURRENT_VERSION
		);
		cout << parser.getLines() << endl;
		cout << parser.getLines().size() << endl;

		AlwaysAssert(parser.getLines().size() == 33, AipsError);

		RegionTextParser parser2(
			goodFile2, csys, IPosition(4, 2000, 2000, 4, 2000),
			RegionTextParser::CURRENT_VERSION
		);

		cout << parser2.getLines().size() << endl;
		AlwaysAssert(parser2.getLines().size() == 34, AipsError);


    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    }
    cout << "ok" << endl;
    return 0;
}
