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

#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>

#include <images/Images/FITSImage.h>
#include <images/Images/ImageUtilities.h>

#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>

#include <casa/namespace.h>

#include <sys/types.h>

#include <unistd.h>
#include <iomanip>



uInt testNumber = 0;

void writeTestString(const String& test) {
    cout << "\n" << "*** " << testNumber << ": "
    	<< test << " ***" << endl;
    testNumber++;
}

String dirName;


void checkImage(
	const ImageInterface<Float> *gotImage, const String& expectedName
) {
	Float ftol = 2e-11; // 3.0e-13 is too small.

	PagedImage<Float> expectedImage(expectedName);
	AlwaysAssert(gotImage->shape() == expectedImage.shape(), AipsError);
	Array<Float> gotchunk = gotImage->get();
	Array<Float> expchunk = expectedImage.get();

	cout << "got " << gotImage->name() << endl;
	cout << "exp " << expectedName << endl;
	Float nanvalue = 1453.4953342425;
	Array<Float>::iterator iter;
	Array<Float>::iterator jiter = expchunk.begin();
	for (
		iter=gotchunk.begin();
		iter!=gotchunk.end(); iter++, jiter++
	) {
		if (isNaN(*iter)) {
			*iter = nanvalue;
		}
		if (isNaN(*jiter)) {
			*jiter = nanvalue;
		}
	}
	Array<Float> diffData = gotchunk - expchunk;
    Float maxdiff = max(abs(diffData));
    cout << "maxdiff " << maxdiff << endl;
    if(maxdiff > ftol){
    	cerr << "For expectedImage " << expectedName << ":" << endl;
    	cerr << "\tmaxdiff = " << maxdiff << endl;
    	cerr << "\t   ftol = " << ftol << endl;
    }
    cout << "*** max diff " << abs(diffData) << endl;
    cout << "*** ftol " << ftol << endl;
	AlwaysAssert(max(abs(diffData)) <= ftol, AipsError);
	CoordinateSystem gotCsys = gotImage->coordinates();
	CoordinateSystem expectedCsys = expectedImage.coordinates();
	Array<Double> diffPixels = gotCsys.referencePixel() - expectedCsys.referencePixel();
	AlwaysAssert(max(abs(diffPixels)) == 0, AipsError);
	Array<Double> fracDiffRef = (
			gotCsys.referenceValue() - expectedCsys.referenceValue()
		)/expectedCsys.referenceValue();
	AlwaysAssert(max(abs(fracDiffRef)) <= ftol, AipsError);
}

void checkImage(
	const String& gotName, const String& expectedName
) {
	PagedImage<Float> gotImage(gotName);
	checkImage(&gotImage, expectedName);
}

void testException(
	const String& test,
    const ImageInterface<Float>& image, const String& region,
    const String& box, const String& chans,
    const String& stokes, const String& mask,
    const uInt axis, const Bool multiFit,
    const String& residual, const String& model,
    const uInt ngauss, const Int polyOrder,
    const String& estimatesFilename=""
) {
	writeTestString(test);
	Bool exceptionThrown = true;
	try {
		ImageProfileFitter fitter(
			&image, region, 0, box,
		    chans, stokes,
		    mask, axis, multiFit,
		    residual, model, ngauss,
		    polyOrder, estimatesFilename
		);

		// should not get here, fail if we do.
		exceptionThrown = false;
	}
	catch (AipsError x) {}
	AlwaysAssert(exceptionThrown, AipsError);
}

int main() {
    pid_t pid = getpid();
    ostringstream os;
    os << "tImageProfileFitter_tmp_" << pid;
    dirName = os.str();
	Directory workdir(dirName);
	String *parts = new String[2];
	split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
	String datadir = parts[0] + "/data/regression/unittest/specfit/";
    FITSImage goodImage(datadir + "specfit_multipix_2gauss.fits");
    FITSImage goodPolyImage(datadir + "specfit_multipix_poly_2gauss.fits");
	workdir.create();
	uInt retVal = 0;
    try {
    	testException(
    	    "Exception if given axis is out of range", goodImage, "",
    	    "", "", "", "", 5, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if given axis is out of range", goodImage, "bogus",
    		"", "", "", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus box string given #1", goodImage, "",
    		"abc", "", "", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus box string given #2", goodImage, "",
    		"0,0,1000,1000", "", "", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus chans string given #1", goodImage, "",
    		"", "abc", "", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus chans string given #2", goodImage, "",
    		"", "0-200", "", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus stokes string given #1", goodImage, "",
    		"", "", "abc", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if bogus stokes string given #2", goodImage, "",
    		"", "", "v", "", 2, False, "", "", 1, -1
    	);
    	testException(
    		"Exception if no gaussians and no polynomial specified", goodImage, "",
    		"", "", "", "", 2, False, "", "", 0, -1
    	);
    	testException(
    		"Exception if bogus estimates file given", goodImage, "",
    		"", "", "", "", 2, False, "", "", 1, -1, "bogusfile"
    	);
    	testException(
    		"Exception if badly formatted estimates file given", goodImage, "",
    		"", "", "", "", 2, False, "", "", 1, -1,
    		datadir + "badProfileEstimatesFormat.txt"
    	);
    	{
    		writeTestString("test results of non-multi-pixel two gaussian fit");
            ImageProfileFitter *fitter;
            LogIO log;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2, False,
            	"", "", 2, -1, ""
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool("converged");

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		AlwaysAssert(((Vector<String>)results.asArrayString("type0"))[0] == "GAUSSIAN", AipsError);

    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("amp0"))[0] - (49.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("ampErr0"))[0] - (4.0) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("center0"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("centerErr0"))[0] - (1.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhm0"))[0] - (42.4) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhmErr0"))[0] - (4.0) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	{
    		writeTestString("test results of multi-pixel two gaussian fit");
    		ImageProfileFitter fitter(
    				&goodImage, "", 0, "", "", "", "", 2, True,
    				"", "", 2, -1, ""
    		);
    		Record results = fitter.fit();

    		writeTestString("-- test correct number of fits performed");
    		Array<Bool> converged = results.asArrayBool("converged");
    		AlwaysAssert(converged.size() == 81, AipsError);

    		writeTestString("  -- test all fits converged");
    		AlwaysAssert(allTrue(converged), AipsError);
    		writeTestString(
    			"  -- test all fits were for 2 gaussians except for the first which was for 1"
    		);

    		Array<Int> nComps = results.asArrayInt("ncomps");
    		for (Array<Int>::const_iterator iter=nComps.begin(); iter!=nComps.end(); iter++) {
    			Int expected = iter != nComps.begin() ? 2 : 1;
    			AlwaysAssert(*iter == expected, AipsError);
    		}

    		writeTestString("  -- Test fit component types");
    		for(uInt i=0; i<2; i++) {
    			String num = String::toString(i);
    			Array<String> types = results.asArrayString("type" + num);
    			for (Array<String>::const_iterator iter=types.begin(); iter!=types.end(); iter++) {
    				String expected = (i == 1 && iter == types.begin()) ? "UNDEF" : "GAUSSIAN";
    				AlwaysAssert(*iter == expected, AipsError);
    			}
    		}
    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	/*
    	 * FIXME reinstitute when isNaN() issue resolved
*/
    	{
    		writeTestString("test writing result images of multi-pixel two gaussian fit");
    		String center = dirName + "/center";
    		String centerErr = dirName + "/centerErr";
    		String fwhm = dirName + "/fwhm";
    		String fwhmErr = dirName + "/fwhmErr";
    		String amp = dirName + "/amp";
    		String ampErr = dirName + "/ampErr";

    		ImageProfileFitter fitter(
    			&goodImage, "", 0, "", "", "", "", 2, True,
    			"", "", 2, -1, "", amp, ampErr, center, centerErr,
    			fwhm, fwhmErr
    		);
    		Record results = fitter.fit();

    		Vector<String> names(12);
    		names[0] = "center_0";
    		names[1] = "centerErr_0";
    		names[2] = "center_1";
    		names[3] = "centerErr_1";
       		names[4] = "amp_0";
        	names[5] = "ampErr_0";
        	names[6] = "amp_1";
        	names[7] = "ampErr_1";
       		names[8] = "fwhm_0";
        	names[9] = "fwhmErr_0";
        	names[10] = "fwhm_1";
        	names[11] = "fwhmErr_1";

    		for (uInt i=0; i<names.size(); i++) {
    			checkImage(dirName + "/" + names[i], datadir + names[i]);
    		}
    	}

    	{
    		writeTestString("test results of multi-pixel two gaussian, order 3 polynomial fit");
    		ImageProfileFitter fitter(
    				&goodPolyImage, "", 0, "", "", "", "", 2, True,
    				"", "", 2, 3, ""
    		);
    		Record results = fitter.fit();

    		writeTestString(" -- test correct number of fits attempted");
    		Array<Bool> converged = results.asArrayBool("converged");
    		AlwaysAssert(converged.size() == 81, AipsError);
    		writeTestString("  -- test all but one fits converged");
    		AlwaysAssert(nfalse(converged) == 1, AipsError);
    		AlwaysAssert(! converged(IPosition(4, 0, 8, 0, 0)), AipsError);
    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	{
    		writeTestString("test results of non-multi-pixel one gaussian fit with estimates file");
            ImageProfileFitter *fitter;
            LogIO log;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2, False,
            	"", "", 1, -1, datadir + "goodProfileEstimatesFormat_2.txt"
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool("converged");

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		AlwaysAssert(((Vector<String>)results.asArrayString("type0"))[0] == "GAUSSIAN", AipsError);

    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("amp0"))[0] - (49.7) < 0.1), AipsError);
    		cout << "amperr " << (Vector<Double>)results.asArrayDouble("ampErr0")[0] << endl;
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("ampErr0"))[0] - (4.0) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("center0"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("centerErr0"))[0] - (1.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhm0"))[0] - (42.4) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhmErr0"))[0] - (4.0) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}

    	{
    		writeTestString("test results of non-multi-pixel one gaussian fit with estimates file holding peak constant");
            ImageProfileFitter *fitter;
            LogIO log;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2, False,
            	"", "", 1, -1, datadir + "goodProfileEstimatesFormat_3.txt"
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool("converged");

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		AlwaysAssert(((Vector<String>)results.asArrayString("type0"))[0] == "GAUSSIAN", AipsError);

    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("amp0"))[0] - (45) < 1e-6), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("ampErr0"))[0] - (0.0) < 1e-6), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("center0"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("centerErr0"))[0] - (1.9) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhm0"))[0] - (45.6) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asArrayDouble("fwhmErr0"))[0] - (3.8) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}

    	cout << endl << "All " << testNumber << " tests succeeded" << endl;
        cout << "ok" << endl;
    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        retVal = 1;
    } 
	workdir.removeRecursive();

    return retVal;
}

