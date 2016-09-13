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
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <imageanalysis/Annotations/AnnSymbol.h>
#include <imageanalysis/Annotations/AnnCircle.h>
#include <imageanalysis/Annotations/AnnEllipse.h>

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
		String goodFile3 = parts[0]
		    + "/data/regression/unittest/imageanalysis/IO/goodAsciiAnnotationsFile3.txt";
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

		csys = CoordinateUtil::defaultCoords4D();
		SpectralCoordinate sp = csys.spectralCoordinate();
		cout << sp.referenceValue() << " "
			<< sp.worldAxisUnits() << endl;

		Double world, vel;
		sp.toWorld(world, 1);
		cout << "*** restfreq " << sp.restFrequencies() << endl;
		sp.frequencyToVelocity(vel, world);
		cout << "*** min " << world << endl;
		cout << "*** min " << vel << endl;

		sp.toWorld(world, 1998);
		sp.frequencyToVelocity(vel, world);

		cout << "*** max " << world << endl;
		cout << "*** max " << vel << endl;

		RegionTextParser parser3(
			goodFile3, csys, IPosition(4, 200, 200, 4, 2000),
			RegionTextParser::CURRENT_VERSION
		);
		for (uInt i=0; i<parser3.getLines().size(); i++) {
			cout << parser3.getLines()[i] << endl;
		}
		{
			cout << "Test label offset parsing (CAS-4358)" << endl;
			Quantity x(0, "deg");
			Quantity y(0, "deg");

			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			AnnSymbol symbol(
				x, y, csys,
			    AnnSymbol::POINT, Vector<Stokes::StokesTypes>(1, Stokes::I)
			);
			String label = "mylabel";
			symbol.setLabel(label);
			AlwaysAssert(symbol.getLabel() == label, AipsError);
			symbol.setLabelPosition("bottom");
			vector<Int> offset(2);
			offset[0] = -1;
			offset[1] = 4;
			symbol.setLabelOffset(offset);
			AlwaysAssert(symbol.getLabelOffset() == offset, AipsError);
			String labelcolor = "orange";
			symbol.setLabelColor(labelcolor);
			cout << symbol << endl;
			ostringstream oss;
			oss << symbol;
			RegionTextParser parser4(csys, IPosition(4, 200, 200, 4, 1), oss.str());
			Vector<AsciiAnnotationFileLine> lines = parser4.getLines();
			AlwaysAssert(lines.size() == 1, AipsError);
			AlwaysAssert(lines[0].getAnnotationBase()->getLabelOffset() == offset, AipsError);
			AlwaysAssert(lines[0].getAnnotationBase()->getLabelColorString() == labelcolor, AipsError);
		}
		{
			cout << "Test correlation writing and reading (CAS-4373)" << endl;
			Quantity x(-60, "arcmin");
			Quantity y(60, "arcmin");
			Quantity r(5, "arcmin");
			Vector<Stokes::StokesTypes> stokes(3);
			stokes[0] = Stokes::I;
			stokes[1] = Stokes::Q;
			stokes[2] = Stokes::U;
			IPosition shape(4,100,100, 3, 2);
			AnnCircle circle(x, y, r, csys, shape, stokes);
			ostringstream oss;
			oss << circle;
			cout << circle << endl;
			RegionTextParser parser4(csys, shape, oss.str());
			Vector<AsciiAnnotationFileLine> lines = parser4.getLines();
			AlwaysAssert(lines.size() == 1, AipsError);
			Vector<Stokes::StokesTypes> got = dynamic_cast<const AnnCircle *>(
					lines[0].getAnnotationBase()
				)->getStokes();
			AlwaysAssert(allEQ(got, stokes), AipsError);

		}
        {
            cout << "*** test writing and reading ellipse gives expected result, CAS-5190" << endl;
            IPosition shape(4,100,100, 3, 2);
            Vector<Stokes::StokesTypes> stokes(1, Stokes::I);
            AnnEllipse ellipse(
                Quantity(0, "pix"), Quantity(0, "pix"), Quantity(4, "arcsec"), Quantity(2, "arcsec"),
                Quantity(40, "deg"), csys, shape, stokes
            );
            ostringstream oss;
            oss << ellipse;
            RegionTextParser parser5(csys, shape, oss.str());
            Vector<AsciiAnnotationFileLine> lines = parser5.getLines();
            cout << "nlines " << lines.size() << endl;
            AlwaysAssert(lines.size() == 1, AipsError);
            cout << ellipse << endl;
            const AnnEllipse* got = dynamic_cast<const AnnEllipse *>(
                lines[0].getAnnotationBase()
            );
            AlwaysAssert(*got == ellipse, AipsError);
        }

    }
    catch (const AipsError& x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    }
    cout << "ok" << endl;
    return 0;
}

