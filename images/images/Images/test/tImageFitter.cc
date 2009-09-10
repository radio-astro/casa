//# tPagedImage.cc:  test the PagedImage class
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
//# $Id: tPagedImage.cc 20648 2009-06-29 07:22:00Z gervandiepen $

#include <casa/OS/Path.h>
#include <casa/namespace.h>
#include <casa/BasicMath/Math.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/ImageFitter.h>
#include <measures/Measures/MDirection.h>
#include <components/ComponentModels/ComponentShape.h>
#include <casa/BasicSL/Constants.h>

int main(Int argc, char *argv[]) {
    // just to eliminate compiler warning about argc not being used
    if (argc > 1) {
        cerr << "WARNING: This test takes no command line options, "
            << "so the options you specified will be ignored" << endl;
    }

    Path path(argv[0]);
    String fixturesDir = path.dirName() + "/fixtures/tImageFitter/";
    try {
        ImageFitter fitter = ImageFitter(fixturesDir + "gaussian_model.fits");
        ComponentList compList = fitter.fit();
        Vector<Quantity> flux;
        compList.getFlux(flux,0);
        // I stokes flux test
        AlwaysAssert(near(flux(0).getValue(), 60318.5801, 1e-4), AipsError);
        // Q stokes flux test
        AlwaysAssert(flux(1).getValue() == 0, AipsError);
        MDirection direction = compList.getRefDirection(0);
        AlwaysAssert(near(direction.getValue().getLong("rad").getValue(), 0.000213318, 1e-5), AipsError);
        AlwaysAssert(near(direction.getValue().getLat("rad").getValue(), 1.939254e-5, 1e-5), AipsError);

        Double arcsecsPerRadian = 180*3600/C::pi;
        Vector<Double> parameters = compList.getShape(0)->parameters();

        Double majorAxis = arcsecsPerRadian*parameters(0);
        AlwaysAssert(near(majorAxis, 23.548201, 1e-7), AipsError);

        Double minorAxis = arcsecsPerRadian*parameters(1);
        AlwaysAssert(near(minorAxis, 18.838560, 1e-7), AipsError);

        Double positionAngle = 180/C::pi*parameters(2);
        AlwaysAssert(near(positionAngle, 120.0, 1e-7), AipsError);

        cout<< "ok"<< endl;
    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    return 0;
}

