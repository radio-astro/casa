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


#include <casa/OS/Directory.h>
#include <images/Images/ImageReorderer.h>
#include <images/Images/FITSImage.h>
#include <images/Images/PagedImage.h>

#include <casa/namespace.h>

#include <sys/types.h>
#include <unistd.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}

void testException(const String& test, const String& imagename, const String& order, const String outname) {
	writeTestString(test);
	Bool exceptionThrown = true;
	try {
		ImageReorderer reorderer(imagename, order, outname);
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
    os << "tImageReorderer_tmp_" << pid;
    String dirName = os.str();
	Directory workdir(dirName);
	String goodInputImage = "reorder_in.fits";
	Bool ok = True;
	try {
		testException(
			"test no specified input image will throw an exception", "", "", ""
		);
		testException(
			"test no specified output image will throw an exception", "x", "", ""
		);
		testException(
			"test non-writable output image will throw an exception",
			goodInputImage, "012", "/x.im"
		);
		testException(
			"test file exists with specified output image will throw an exception",
			goodInputImage, "012", "reorder_exists.im"
		);
		testException(
			"test non-existent input image name will throw an exception",
			"blah", "012", "out.im"
		);
		testException(
			"test more specified axes than correct will throw an exception",
			goodInputImage, "0123", "out.im"
		);
		testException(
			"test fewer specified axes than correct will throw an exception",
			goodInputImage, "01", "out.im"
		);
		testException(
			"test specifying an axis number not in the image throws an exception",
			goodInputImage, "013", "out.im"
		);

		workdir.create();
		{
			writeTestString("test no reordering");
			String outname = dirName + "/reorder_012_out.im";
			ImageReorderer reorderer(goodInputImage, "012", outname);
			reorderer.reorder();
			FITSImage inImage(goodInputImage);
			PagedImage<Float> outImage(outname);
			AlwaysAssert(inImage.shape() == outImage.shape(), AipsError);
			Array<Float> inData = inImage.get();
			Array<Float> outData = outImage.get();
			vector<Float> inVec, outVec;
			inData.tovector(inVec);
			outData.tovector(outVec);
			for(uInt i=0; i<inVec.size(); i++) {
				AlwaysAssert(inVec[i] == outVec[i], AipsError);
			}
			Vector<Double> inRefPix = inImage.coordinates().referencePixel();
			Vector<Double> outRefPix = outImage.coordinates().referencePixel();
			Vector<Double> inRefVal = inImage.coordinates().referenceValue();
			Vector<Double> outRefVal = outImage.coordinates().referenceValue();

			for (uInt i=0; i<inRefPix.size(); i++) {
				AlwaysAssert(inRefPix[i] == outRefPix[i], AipsError);
				AlwaysAssert(inRefVal[i] == outRefVal[i], AipsError);
			}
		}

		{
			writeTestString("test 201 reordering");
			String outname = dirName +  "/reorder_201_out.im";
			ImageReorderer reorderer(goodInputImage, "201", outname);
			reorderer.reorder();
			FITSImage inImage(goodInputImage);
			PagedImage<Float> outImage(outname);
			IPosition inShape = inImage.shape();

			IPosition outShape = outImage.shape();
			IPosition outMap(3,2,0,1);

			Array<Float> inData = inImage.get();
			Array<Float> outData = outImage.get();
			Cube<Float> inCube;
			inCube.reference(inData);
			Cube<Float> outCube;
			outCube.reference(outData);

			vector<Float> inVec, outVec;
			inData.tovector(inVec);
			outData.tovector(outVec);
			for(uInt i=0; i<inShape[0]; i++) {
				for(uInt j=0; j<inShape[1]; j++) {
					for(uInt k=0; k<inShape[2]; k++) {
						AlwaysAssert(inCube(i,j,k) == outCube(k,i,j), AipsError);
					}
				}
			}
			Vector<Double> inRefPix = inImage.coordinates().referencePixel();
			Vector<Double> outRefPix = outImage.coordinates().referencePixel();
			Vector<Double> inRefVal = inImage.coordinates().referenceValue();
			Vector<Double> outRefVal = outImage.coordinates().referenceValue();

			for(uInt i=0; i<inShape.size(); i++) {
				AlwaysAssert(outShape[i] == inShape[outMap[i]], AipsError);
				AlwaysAssert(outRefPix[i] == inRefPix[outMap[i]], AipsError);
				AlwaysAssert(outRefVal[i] == inRefVal[outMap[i]], AipsError);
			}
		}

		cout << "ok" << endl;
	}
    catch (AipsError x) {
    	ok = False;
        cerr << "Exception caught: " << x.getMesg() << endl;
    }
	if(workdir.exists()) {
		workdir.removeRecursive();
	}
	return ok ? 0 : 1;
}

