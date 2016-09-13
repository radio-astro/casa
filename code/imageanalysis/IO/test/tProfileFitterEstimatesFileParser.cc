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

#include <imageanalysis/IO/ProfileFitterEstimatesFileParser.h>

#include <components/SpectralComponents/GaussianSpectralElement.h>

#include <casa/Utilities/Assert.h>
#include <casa/OS/File.h>
#include <casa/OS/EnvVar.h>

#include <casa/namespace.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}


int main() {
	String *parts = new String[2];
	split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
	String datadir = parts[0] + "/data/regression/unittest/specfit/";
	delete [] parts;

    try {
        {
            writeTestString(
                "test constructor throws error in case where file does not exist"
            );
            Bool thrown = False;
            try {
            	ProfileFitterEstimatesFileParser parser(datadir + "bogusfile.txt");
            }
            catch (AipsError x) {
            	thrown = True;
            }
            if (! thrown) {
            	// if we get here an exception wasn't thrown so the test failed
            	throw AipsError("constructor should have thrown an expeption but did not");
            }
        }
        {
        	writeTestString(
        		"test constructor throws error in case where file is formatted incorrectly"
        	);
        	Bool thrown = False;
            try {
            	ProfileFitterEstimatesFileParser parser(datadir + "badProfileEstimatesFormat.txt");
            }
            catch (AipsError x) {
            	thrown = True;
            }
            if (! thrown) {
            	// if we get here an exception wasn't thrown so the test failed
            	throw AipsError("constructor should have thrown an exception but did not");
            }
        }
        {
        	writeTestString(
        		"test constructor throws error in case where fixed parameters contain unrecognized flag"
        	);
        	Bool thrown = False;
            try {
            	ProfileFitterEstimatesFileParser parser(
            		datadir + "badProfileFixedFormat.txt"
            	);
            }
            catch (AipsError x) {
            	thrown = True;
            }
            if (! thrown) {
            	// if we get here an exception wasn't thrown so the test failed
            	throw AipsError("constructor should have thrown an exception but did not");
            }
        }
        {
        	writeTestString(
        		"test constructor parses correctly formatted file"
        	);
            ProfileFitterEstimatesFileParser parser(datadir + "goodProfileEstimatesFormat.txt");
            SpectralList compList = parser.getEstimates();
            AlwaysAssert(compList.nelements() == 2, AipsError);
            Vector<Double> expectedPeak(2);
            expectedPeak[0] = 74;
            expectedPeak[1] = 135;
            Vector<Quantity> flux;
            Vector<Double> dirTuple;
            Vector<Double> expectedCenter(2);
            expectedCenter[0] = 4;
            expectedCenter[1] = 6.5;
            Vector<Double> expectedFWHM(2);
            expectedFWHM[0] = 3.0;
            expectedFWHM[1] = 4.1;
            Vector<String> expectedFixed(2);
            expectedFixed[0] = "";
            expectedFixed[1] = "p";
            for (uInt i=0; i<compList.nelements(); i++) {
            	GaussianSpectralElement* g = dynamic_cast<GaussianSpectralElement *>(compList[i]);
                AlwaysAssert(near(g->getAmpl(), expectedPeak[i]), AipsError);
                AlwaysAssert(near(g->getCenter(), expectedCenter[i]), AipsError);
                AlwaysAssert(near(g->getFWHM(), expectedFWHM[i]), AipsError);
                AlwaysAssert(! g->fixedCenter(), AipsError);
                AlwaysAssert(! g->fixedFWHM(), AipsError);
                switch (i) {
                case 0:
                	AlwaysAssert(! g->fixedAmpl(), AipsError);
                	break;
                case 1:
                    AlwaysAssert(g->fixedAmpl(), AipsError);
                    break;
                default:
                	throw AipsError("Shouldn't have gotten here");
                }
            }
        }
    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    }
    cout << "ok" << endl;
    return 0;
}

