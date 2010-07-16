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

#include <images/Images/ImageCollapser.h>

#include <images/Images/ImageInputProcessor.h>
#include <images/Images/FITSImage.h>
#include <images/Images/PagedImage.h>

#include <casa/OS/Directory.h>
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

String outname() {
	return dirName + "/" + "test_" + String::toString(testNumber) + ".im";

}

void checkImage(
	const ImageInterface<Float> *gotImage, const String& expectedName
) {
	FITSImage expectedImage(expectedName);
	AlwaysAssert(gotImage->shape() == expectedImage.shape(), AipsError);
	Array<Float> diffData = gotImage->get() - expectedImage.get();
	AlwaysAssert(max(abs(diffData)) == 0, AipsError);
	CoordinateSystem gotCsys = gotImage->coordinates();
	CoordinateSystem expectedCsys = expectedImage.coordinates();
	Array<Double> diffPixels = gotCsys.referencePixel() - expectedCsys.referencePixel();
	AlwaysAssert(max(abs(diffPixels)) == 0, AipsError);
	Array<Double> fracDiffRef = (
			gotCsys.referenceValue() - expectedCsys.referenceValue()
		)/expectedCsys.referenceValue();
	AlwaysAssert(max(abs(fracDiffRef)) <= 1e-13, AipsError);
}

void checkImage(
	const String& gotName, const String& expectedName
) {
	PagedImage<Float> gotImage(gotName);
	checkImage(&gotImage, expectedName);
}


void testException(
	const String& test, const String& aggString,
    const String& imagename, const String& region,
    const String& box, const String& chans,
    const String& stokes, const String& mask,
    const uInt compressionAxis
) {
	writeTestString(test);
	Bool exceptionThrown = true;
	try {
		ImageCollapser collapser(
			aggString, imagename, region, box,
			chans, stokes, mask, compressionAxis,
			outname(), False
		);
		// should not get here, fail if we do.
		exceptionThrown = false;
		AlwaysAssert(false, AipsError);
	}
	catch (AipsError x) {
		AlwaysAssert(exceptionThrown, AipsError);
	}
}

int main() {
    pid_t pid = getpid();
    ostringstream os;
    os << "tImageCollapser_tmp_" << pid;
    dirName = os.str();
	Directory workdir(dirName);
    String goodImage("collapse_in.fits");
    const String ALL = ImageInputProcessor::ALL;
    try {
    	workdir.create();
    	testException(
    		"Exception if no image name given", "mean",
    		"", "", "", "", "", "", 0
    	);
    	testException(
    		"Exception if bogus image name given", "mean",
    		"mybogus.im", "", "", "", "", "", 0
    	);
    	testException(
    		"Exception if no aggregate string given", "",
    		goodImage, "", "", "", "", "", 0
    	);
    	testException(
    		"Exception if bogus aggregate string given", "bogus function",
    		goodImage, "", "", "", "", "", 0
    	);
    	testException(
    		"Exception if bogus region string given", "mean",
    		goodImage, "bogus_region", "", "", "", "", 0
    	);
    	testException(
    		"Exception if bogus box string given #1", "mean",
    		goodImage, "", "abc", "", "", "", 0
    	);
    	testException(
    		"Exception if bogus box string given #2", "mean",
    		goodImage, "", "0,0,1000,1000", "", "", "", 0
    	);
    	{
    		writeTestString("average full image collapse along axis 0");
    		ImageCollapser collapser(
    			"mean", goodImage, "", "", ALL,
    			ALL, "", 0, outname(), False
    		);
    		collapser.collapse(False);
    		checkImage(outname(), "collapse_avg_0.fits");
    	}
    	{
    		writeTestString("average full image collapse along axis 2");
    		ImageCollapser collapser(
    			"mean", goodImage, "", "", ALL,
    			ALL, "", 2, outname(), False
    		);
    		collapser.collapse(False);
    		checkImage(outname(), "collapse_avg_2.fits");
    	}
    	{
    		writeTestString("sum subimage collapse along axis 1");
    		ImageCollapser collapser(
    			"sum", goodImage, "", "1,1,2,2", "1-2",
    			"qu", "", 2, outname(), False
    		);
    		collapser.collapse(False);
    		// and check that we can overwrite the previous output
    		ImageCollapser collapser2(
        		"sum", goodImage, "", "1,1,2,2", "1-2",
        		"qu", "", 1, outname(), True
        	);
    		collapser2.collapse(False);
    		checkImage(outname(), "collapse_sum_1.fits");
    	}
    	{
    		writeTestString("Check not specifying out file is ok");
    		ImageCollapser collapser(
    			"mean", goodImage, "", "", ALL,
    			ALL, "", 2, "", False
    		);
    		ImageInterface<Float> *collapsed = collapser.collapse(True);
    		checkImage(collapsed, "collapse_avg_2.fits");
    		delete collapsed;
    	}
    	{
    		writeTestString("Check not wanting return pointer results in a NULL pointer being returned");
    		ImageCollapser collapser(
    			"mean", goodImage, "", "", ALL,
    			ALL, "", 2, "", False
    		);
    		ImageInterface<Float> *collapsed = collapser.collapse(False);
    		AlwaysAssert(collapsed == NULL, AipsError);
    	}
		workdir.removeRecursive();
        cout << "ok" << endl;
    }
    catch (AipsError x) {
    	if(workdir.exists()) {
    		workdir.removeRecursive();
    	}
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    return 0;
}

