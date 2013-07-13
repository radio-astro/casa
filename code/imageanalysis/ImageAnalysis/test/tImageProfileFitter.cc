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

#include <casa/IO/FiledesIO.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageUtilities.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <imageanalysis/IO/ImageProfileFitterResults.h>

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

	Array<Bool> gotmask = gotImage->getMask(False);
	Array<Bool> expmask = expectedImage.getMask(False);
	AlwaysAssert(allTrue(gotmask == expmask), AipsError);

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
	AlwaysAssert(
		gotImage->units() == expectedImage.units(), AipsError
	);
	CoordinateSystem gotCsys = gotImage->coordinates();
	CoordinateSystem expectedCsys = expectedImage.coordinates();
	Array<Double> diffPixels = gotCsys.referencePixel() - expectedCsys.referencePixel();
	AlwaysAssert(max(abs(diffPixels)) == 0, AipsError);
	// don't use fraction because sometimes denominator is zero
	Array<Double> diffRef = gotCsys.referenceValue() - expectedCsys.referenceValue();
	AlwaysAssert(max(abs(diffRef)) <= ftol, AipsError);
	AlwaysAssert(allTrue(gotCsys.worldAxisNames() == expectedCsys.worldAxisNames()), AipsError);
	AlwaysAssert(allTrue(gotCsys.worldAxisUnits() == expectedCsys.worldAxisUnits()), AipsError);
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
    const uInt axis,
    const uInt ngauss,
    const String& estimatesFilename=""
) {
	writeTestString(test);
	Bool exceptionThrown = true;
	try {
		ImageProfileFitter fitter(
			&image, region, 0, box,
		    chans, stokes,
		    mask, axis,
		    ngauss,
		    estimatesFilename,
		    SpectralList()
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
    FITSImage gaussTripletImage(datadir + "gauss_triplet.fits");
    FITSImage twoLorentziansImage(datadir + "two_lorentzians.fits");

	workdir.create();
	uInt retVal = 0;
    try {
    	testException(
    	    "Exception if given axis is out of range", goodImage, "",
    	    "", "", "", "", 5, 1
    	);
    	testException(
    		"Exception if given axis is out of range", goodImage, "bogus",
    		"", "", "", "", 2, 1
    	);
    	testException(
    		"Exception if bogus box string given #1", goodImage, "",
    		"abc", "", "", "", 2, 1
    	);
    	testException(
    		"Exception if bogus box string given #2", goodImage, "",
    		"0,0,1000,1000", "", "", "", 2, 1
    	);
    	testException(
    		"Exception if bogus chans string given #1", goodImage, "",
    		"", "abc", "", "", 2, 1
    	);
    	testException(
    		"Exception if bogus chans string given #2", goodImage, "",
    		"", "0-200", "", "", 2, 1
    	);
    	testException(
    		"Exception if bogus stokes string given #1", goodImage, "",
    		"", "", "abc", "", 2, 1
    	);
    	testException(
    		"Exception if bogus stokes string given #2", goodImage, "",
    		"", "", "v", "", 2, 1
    	);
    	{
			writeTestString("Exception if no gaussians and no polynomial specified");
			Bool exceptionThrown = true;
    		try {
    			ImageProfileFitter fitter(
    				&goodImage, "", 0, "", "", "", "", 2,
    				0, "", SpectralList()
    			);
    			fitter.fit();
    			exceptionThrown = False;
    		}
    		catch (AipsError x) {}
    		AlwaysAssert(exceptionThrown, AipsError);
    	}
    	testException(
    		"Exception if bogus estimates file given", goodImage, "",
    		"", "", "", "", 2, 1, "bogusfile"
    	);
    	testException(
    		"Exception if badly formatted estimates file given", goodImage, "",
    		"", "", "", "", 2, 1,
    		datadir + "badProfileEstimatesFormat.txt"
    	);
    	{
    		writeTestString("test results of non-multi-pixel two gaussian fit");
            ImageProfileFitter *fitter;
            LogIO log;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2,
            	2, "", SpectralList()
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool(ImageProfileFitterResults::_CONVERGED );

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		cout << results << endl;
    		AlwaysAssert(((Vector<String>)results.asArrayString("type"))[0] == "GAUSSIAN", AipsError);
    		IPosition expShape(goodImage.ndim() + 1, 1);
    		expShape[expShape.nelements() - 1] = 2;
    		cout << "*** exp " << expShape << endl;
    		cout << "*** got " << results.asRecord("gs").asArrayDouble("amp").shape() << endl;
    		AlwaysAssert(
    		    results.asRecord("gs").asArrayDouble("amp").shape().isEqual(
    		    	expShape
    		    ), AipsError
    		);
    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert(
    			(
    				((Vector<Double>)results.asRecord("gs").asArrayDouble("amp"))[0] - (49.7) < 0.1
    			), AipsError
    		);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("ampErr"))[0] - (4.0) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("center"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("centerErr"))[0] - (1.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhm"))[0] - (42.4) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhmErr"))[0] - (4.0) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	{
    		writeTestString("test results of multi-pixel two gaussian fit");
    		ImageProfileFitter fitter(
    				&goodImage, "", 0, "", "", "", "", 2,
    				2, "", SpectralList()
    		);
    		fitter.setDoMultiFit(True);
    		Record results = fitter.fit();

    		writeTestString("-- test correct number of fits performed");
    		Array<Bool> converged = results.asArrayBool(ImageProfileFitterResults::_CONVERGED);
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
    		Array<String> types = results.asArrayString("type");
    		AlwaysAssert(ntrue(types == Array<String>(types.shape(), "GAUSSIAN")) == 2*81-1, AipsError);
    		cout << "types " << types << endl;
    		AlwaysAssert(types(IPosition(5, 0, 0, 0, 0, 1)) == "UNDEF", AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}

    	{
    		writeTestString("test writing result images of multi-pixel two gaussian fit");
    		String center = dirName + "/center";
    		String centerErr = dirName + "/centerErr";
    		String fwhm = dirName + "/fwhm";
    		String fwhmErr = dirName + "/fwhmErr";
    		String amp = dirName + "/amp";
    		String ampErr = dirName + "/ampErr";
    		String integral = dirName + "/integral";
    		String integralErr = dirName + "/integralErr";

    		ImageProfileFitter fitter(
    			&goodImage, "", 0, "", "", "", "", 2,
    			2, "", SpectralList()
    		);
    		fitter.setDoMultiFit(True);
    		fitter.setAmpName(amp);
    		fitter.setAmpErrName(ampErr);
    		fitter.setCenterName(center);
    		fitter.setCenterErrName(centerErr);
    		fitter.setFWHMName(fwhm);
    		fitter.setFWHMErrName(fwhmErr);
    		fitter.setIntegralName(integral);
    		fitter.setIntegralErrName(integralErr);

    		Record results = fitter.fit();

    		Vector<String> names(8);
    		names[0] = "center";
    		names[1] = "centerErr";
       		names[2] = "amp";
        	names[3] = "ampErr";
       		names[4] = "fwhm";
        	names[5] = "fwhmErr";
        	names[6] = "integral";
        	names[7] = "integralErr";
    		for (uInt i=0; i<names.size(); i++) {
    			checkImage(dirName + "/" + names[i], datadir + names[i]);
    		}
    	}
    	Array<Double> sum0;
    	{
    		writeTestString("test results of multi-pixel two gaussian, order 3 polynomial fit");
    		ImageProfileFitter fitter(
    			&goodPolyImage, "", 0, "", "", "", "", 2,
    			2, "", SpectralList()
    		);
    		fitter.setPolyOrder(3);
    		fitter.setDoMultiFit(True);
    		Record results = fitter.fit();

    		writeTestString(" -- test correct number of fits attempted");
    		Array<Bool> converged = results.asArrayBool(ImageProfileFitterResults::_CONVERGED );
    		AlwaysAssert(converged.size() == 81, AipsError);
    		writeTestString("  -- test all but one fits converged");
    		cout << "number not converged " << nfalse(converged) << endl;
    		AlwaysAssert(nfalse(converged) == 1, AipsError);
    		AlwaysAssert(! converged(IPosition(4, 0, 8, 0, 0)), AipsError);
    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    		Array<Double> center = results.asRecord("gs").asArrayDouble("center");
    		IPosition begin(center.ndim(), 0);
    		IPosition end = center.shape() - 1;
    		IPosition sumShape(center.shape());
    		sumShape[sumShape.size()-1] = 1;
    		sum0.resize(sumShape);
    		sum0 = 0;
    		for (uInt i=0; i<center.shape()[center.ndim()-1]; i++) {
    			begin[begin.size()-1] = i;
    			end[end.size()-1] = i;
    			sum0 += center(begin, end);
    		}

    	}
    	Array<Bool> converged;
    	Array<Double> center;
    	{
    		writeTestString("test results of multi-pixel two gaussian, order 3 polynomial fit with estimates file");
    		ImageProfileFitter fitter(
    			&goodPolyImage, "", 0, "", "", "", "", 2,
    			2, datadir + "poly+2gauss_estimates.txt", SpectralList()
    		);
    		fitter.setPolyOrder(3);
    		fitter.setDoMultiFit(True);
    		Record results = fitter.fit();

    		writeTestString(" -- test correct number of fits attempted");
    		converged = results.asArrayBool( ImageProfileFitterResults::_CONVERGED );
    		AlwaysAssert(converged.size() == 81, AipsError);
    		writeTestString("  -- test all but one fits converged");
    		cout << "number not converged " << nfalse(converged) << endl;
    		AlwaysAssert(nfalse(converged) == 1, AipsError);
    		AlwaysAssert(! converged(IPosition(4, 0, 0, 0, 0)), AipsError);
    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    		Array<Double> sum1;
    		center = results.asRecord("gs").asArrayDouble("center");
    		IPosition begin(center.ndim(), 0);
    		IPosition end = center.shape() - 1;
    		IPosition sumShape(center.shape());
    		sumShape[sumShape.size()-1] = 1;
    		sum1.resize(sumShape);
    		sum1 = 0;
    		for (uInt i=0; i<center.shape()[center.ndim()-1]; i++) {
    			begin[begin.size()-1] = i;
    			end[end.size()-1] = i;
    			sum1 += center(begin, end);
    		}
    		Array<Double>::const_iterator iter1 = sum1.begin();
    		uInt count = 0;
    		writeTestString("  -- Test of value consistency");
    		for(
    			Array<Double>::const_iterator iter0=sum0.begin();
    			iter0!=sum0.end(); iter0++, iter1++) {
    			if (! isNaN(*iter0)) {
    				AlwaysAssert(near(*iter0, *iter1, 1e-7), AipsError);
    				count++;
    			}
    		}
    		AlwaysAssert(count >= 65, AipsError);
    	}
    	{
    		writeTestString("test results of multi-pixel two gaussian, order 3 polynomial fit with spectral list estimates");
    		SpectralList sl;
    		GaussianSpectralElement g0(50, 90, 10);
    		GaussianSpectralElement g1(10, 30, 7);
    		sl.add(g0);
    		sl.add(g1);
    		ImageProfileFitter fitter(
    			&goodPolyImage, "", 0, "", "", "", "", 2,
    			2, "", sl
    		);
    		fitter.setPolyOrder(3);
    		fitter.setDoMultiFit(True);
    		Record results = fitter.fit();

    		writeTestString(" -- test converged array");
    		AlwaysAssert(allTrue(results.asArrayBool( ImageProfileFitterResults::_CONVERGED ) == converged), AipsError);
    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    		IPosition start(5,0,1,0,0, 0);
    		IPosition end(5,8,8,0,0, 1);
    		writeTestString("  -- Test center values");
    		AlwaysAssert(
    			allNear(results.asRecord("gs").asArrayDouble("center")(start, end), center(start, end), 1e-8),
    			AipsError
    		);
    	}
    	{
    		writeTestString("test results of non-multi-pixel one gaussian fit with estimates file");
            ImageProfileFitter *fitter;
            LogIO log;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2,
            	1, datadir + "goodProfileEstimatesFormat_2.txt", SpectralList()
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool(ImageProfileFitterResults::_CONVERGED);

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		AlwaysAssert(((Vector<String>)results.asArrayString("type"))[0] == "GAUSSIAN", AipsError);

    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("amp"))[0] - (49.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("ampErr"))[0] - (4.0) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("center"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("centerErr"))[0] - (1.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhm"))[0] - (42.4) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhmErr"))[0] - (4.0) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	{
    		writeTestString("test results of non-multi-pixel one gaussian fit with estimates file holding peak constant");
            ImageProfileFitter *fitter;
            fitter =  new ImageProfileFitter(
            	&goodImage, "", 0, "", "", "", "", 2,
            	1, datadir + "goodProfileEstimatesFormat_3.txt", SpectralList()
            );

    		Record results = fitter->fit();
            delete fitter;
            Vector<Bool> converged = results.asArrayBool(ImageProfileFitterResults::_CONVERGED );

    		writeTestString("  -- Results arrays have one member");
    		AlwaysAssert(converged.size() == 1, AipsError);

    		writeTestString("  -- Fit converged");
    		AlwaysAssert(converged[0], AipsError);

    		writeTestString("  -- Only one gaussian fit");
    		AlwaysAssert(((Vector<Int>)results.asArrayInt("ncomps"))[0] == 1, AipsError);

    		writeTestString("  -- It is a gaussian and not something else");
    		AlwaysAssert(((Vector<String>)results.asArrayString("type"))[0] == "GAUSSIAN", AipsError);

    		writeTestString("  -- Various tests of the fit values");
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("amp"))[0] - (45) < 1e-6), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("ampErr"))[0] - (0.0) < 1e-6), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("center"))[0] - (-237.7) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("centerErr"))[0] - (1.9) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhm"))[0] - (45.6) < 0.1), AipsError);
    		AlwaysAssert((((Vector<Double>)results.asRecord("gs").asArrayDouble("fwhmErr"))[0] - (3.8) < 0.1), AipsError);

    		writeTestString("  -- Test of fit units");
    		AlwaysAssert(results.asString("xUnit") == "km/s", AipsError);
    		AlwaysAssert(results.asString("yUnit") == "Jy", AipsError);
    	}
    	{
    	    writeTestString("test results of non-multi-fit gaussian triplet");
    	    vector<GaussianSpectralElement> g(3);
    	    g[0] = GaussianSpectralElement(1.2, 20, 4);
    	    g[1] = GaussianSpectralElement(0.8, 72, 4);
    	    g[2] = GaussianSpectralElement(0.6, 100, 4);
    	    Matrix<Double> constraints(2,3, 0);
    	    constraints(0,0) = 0.7;
    	    constraints(0,1) = 52;
    	    constraints(1,0) = 0.55;
    	    GaussianMultipletSpectralElement gm(g, constraints);
    	    SpectralList triplet(gm);
    	    ImageProfileFitter fitter(
    	    	&gaussTripletImage, "", 0, "", "", "", "", 2,
    	        1, "", triplet
    	    );
    	    Record results = fitter.fit();
            writeTestString("Test component type");
            AlwaysAssert(
                ((Vector<String>)results.asArrayString("type"))[0]
                    == "GAUSSIAN MULTIPLET",
                AipsError
            );
            Record gm0 = results.asRecord("gm0");
            Vector<Double> exp(3);
            exp[0] = 4.15849;
            exp[1] = 2.91095;
            exp[2] = 2.28717;
            writeTestString("Test amplitudes");
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("amp"),
                    exp, 1e-5
                ),
                AipsError
            );  
            exp[0] = 1149.73;
            exp[1] = 1138.76;
            exp[2] = 1133.66;
            writeTestString("Test centers");
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("center"),
                    exp, 1e-5
                ),
                AipsError
            );  
            exp[0] = 5.75308;
            exp[1] = 4.09405;
            exp[2] = 3.93497;
            writeTestString("Test fwhms");
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("fwhm"),
                    exp, 1e-5
                ),
                AipsError
            );  
            exp[0] = 0.0301945;
            exp[1] = 0.0211362;
            exp[2] = 0.016607;
            writeTestString("Test amplitude errors");
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("ampErr"),
                    exp, 1e-5
                ),
                AipsError
            );  
            exp[0] = 0.0221435;
            exp[1] = 0.0221435;
            exp[2] = 0.0475916;
            writeTestString("Test center errors");
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("centerErr"),
                    exp, 1e-5
                ),
                AipsError
            );  
            exp[0] = 0.0556099;
            exp[1] = 0.085414;
            exp[2] = 0.0987483;
            writeTestString("Test fwhm errors");
            cout << (Vector<Double>)gm0.asArrayDouble("fwhmErr") << endl;
            AlwaysAssert(
                allNear(
                    (Vector<Double>)gm0.asArrayDouble("fwhmErr"),
                    exp, 1e-5
                ),
                AipsError
            );  
            writeTestString("Test constrained amplitude ratios");
            Vector<Double> amp = (Vector<Double>)gm0.asArrayDouble("amp");
            Vector<Double> ampErr = (Vector<Double>)gm0.asArrayDouble("ampErr");
            AlwaysAssert(
                near(amp[1]/amp[0], constraints(0,0), 1e-5), AipsError
            );             
            AlwaysAssert(
                near(amp[2]/amp[0], constraints(1,0), 1e-5), AipsError
            );             
            AlwaysAssert(
                near(ampErr[1]/ampErr[0], constraints(0,0), 1e-5), AipsError
            );             
            AlwaysAssert(
                near(ampErr[2]/ampErr[0], constraints(1,0), 1e-5), AipsError
            );             
            writeTestString("Test constrained center offset");
            Vector<Double> center = (Vector<Double>)gm0.asArrayDouble("center");
            Vector<Double> centerErr = (Vector<Double>)gm0.asArrayDouble("centerErr");
            CoordinateSystem csys = gaussTripletImage.coordinates();
            Double increment = -csys.increment()[2]/csys.spectralCoordinate().restFrequencies()[0]*C::c/1000;
            AlwaysAssert(
                near(center[1]-center[0], constraints(0,1)*increment, 1e-5), AipsError
            );             
            AlwaysAssert(
                near(centerErr[1], centerErr[0], 1e-5), AipsError
            );             
    	}
    	{
    	    writeTestString("test results of multi-fit gaussian triplet");
    	    vector<GaussianSpectralElement> g(3);
    	    g[0] = GaussianSpectralElement(1.2, 20, 4);
    	    g[1] = GaussianSpectralElement(0.8, 72, 4);
    	    g[2] = GaussianSpectralElement(0.6, 100, 4);
    	    Matrix<Double> constraints(2,3, 0);
    	    constraints(0,0) = 0.70;
    	    constraints(0,1) = 52;
    	    constraints(1,0) = 0.55;
    	    GaussianMultipletSpectralElement gm(g, constraints);
    	    SpectralList triplet(gm);
    	    ImageProfileFitter fitter(
    	    	&gaussTripletImage, "", 0, "", "", "", "", 2,
    	        1, "", triplet
    	    );
    	    fitter.setDoMultiFit(True);
            Vector<String> names(8);
            String s = dirName + "/";
            names[0] = "center";
    		names[1] = "centerErr";
    		names[2] = "fwhm";
    		names[3] = "fwhmErr";
    		names[4] = "amp";
    		names[5] = "ampErr";
    		names[6] = "integral";
    		names[7] = "integralErr";

    		fitter.setDoMultiFit(True);
    		fitter.setCenterName(s + names[0]);
    		fitter.setCenterErrName(s + names[1]);
    		fitter.setFWHMName(s + names[2]);
    		fitter.setFWHMErrName(s + names[3]);
    		fitter.setAmpName(s + names[4]);
    		fitter.setAmpErrName(s + names[5]);
    		fitter.setIntegralName(s + names[6]);
    		fitter.setIntegralErrName(s + names[7]);
    		String logfile = "mylog.txt";
    		fitter.setLogfile(logfile);

    	    Record results = fitter.fit();
    	    writeTestString("test solution image results");
    		for (uInt i=0; i<names.size(); i++) {
    			checkImage(
                    s + names[i] + "_gm",
                    datadir + names[i] + "_gm"
                );
    		}
    		Int fd = open(logfile.c_str(), O_RDWR | O_APPEND);
    		FiledesIO fio(fd);
    		// check that there's something in the log file
    		AlwaysAssert(fio.length() > 1e4, AipsError);
    	}
    	{
    		writeTestString("Test fitting of Lorentzians");
    		SpectralList myList;
    		myList.add(LorentzianSpectralElement(1, 30, 4));
    		myList.add(LorentzianSpectralElement(7, 111, 4));
    		ImageProfileFitter fitter(
    			&twoLorentziansImage, "", 0, "", "", "", "", 2,
    			1, "", myList
    		);
			fitter.setDoMultiFit(True);
			Vector<String> names(8);
			String s = dirName + "/";
			names[0] = "center";
			names[1] = "centerErr";
			names[2] = "fwhm";
			names[3] = "fwhmErr";
			names[4] = "amp";
			names[5] = "ampErr";
			names[6] = "integral";
			names[7] = "integralErr";

			fitter.setDoMultiFit(True);
			fitter.setCenterName(s + names[0]);
			fitter.setCenterErrName(s + names[1]);
			fitter.setFWHMName(s + names[2]);
			fitter.setFWHMErrName(s + names[3]);
			fitter.setAmpName(s + names[4]);
			fitter.setAmpErrName(s + names[5]);
			fitter.setIntegralName(s + names[6]);
			fitter.setIntegralErrName(s + names[7]);
			String logfile = "lorentzians_fit.log";
			fitter.setLogfile(logfile);

			Record results = fitter.fit();
			writeTestString("test solution image results");
			for (uInt i=0; i<names.size(); i++) {
				checkImage(
					s + names[i] + "_ls",
					datadir + names[i] + "_ls"
				);
			}
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

