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

#include <images/Images/ImageInputParameters.h>
#include <images/Images/FITSImage.h>
#include <casa/namespace.h>



String testim = "imageinputtest.fits";

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}

Bool areVectorsEqual(Vector<uInt> vec1, Vector<uInt> vec2) {
	if (vec1.size() != vec2.size()) {
		return False;
	}
	for (uInt i=0; i<vec1.size(); i++) {
		if(vec1[i] != vec2[i]) {
			return False;
		}
	}
	return True;
}


void testGood(const Vector<uInt>& expected, const String& range) {
	FITSImage testim("imageinputtest.fits");
	ImageInputParameters params(testim);
	String errmsg;
	AlwaysAssert(params.setSpectralRanges(errmsg, range), AipsError);
	Vector<uInt> specRanges = params.getSpectralRanges();
	AlwaysAssert(areVectorsEqual(specRanges, expected), AipsError);
}

void noRangesTest() {
	writeTestString("test no ranges set");
	FITSImage testim("imageinputtest.fits");
	ImageInputParameters params(testim);
	Vector<uInt> specRanges = params.getSpectralRanges();
	AlwaysAssert(specRanges.size() == 0, AipsError);
}

void singleChannelTest() {
	writeTestString("test single channel set");
	Vector<uInt> expected(2);
	expected[0] = 4;
	expected[1] = 4;
	testGood(expected, "4");
}

void channelRangeTest() {
	writeTestString("test channel range");
	Vector<uInt> expected(2);
	expected[0] = 2;
	expected[1] = 5;
	testGood(expected, "2-5");
}

void lessThanChannelTest() {
	writeTestString("test less than channel");
	Vector<uInt> expected(2);
	expected[0] = 0;
	expected[1] = 10;
	testGood(expected, "<11");
}

void lessThanOrEqualChannelTest() {
	writeTestString("test less than or equal channel");
	Vector<uInt> expected(2);
	expected[0] = 0;
	expected[1] = 11;
	testGood(expected, "<=11");
}

void greaterThanChannelTest() {
	writeTestString("test greater than channel");
	Vector<uInt> expected(2);
	expected[0] = 12;
	expected[1] = 30;
	testGood(expected, ">11");
}

void greaterThanOrEqualChannelTest() {
	writeTestString("test greater than or equal channel");
	Vector<uInt> expected(2);
	expected[0] = 11;
	expected[1] = 30;
	testGood(expected, ">=11");
}

void complexChannelTest1() {
	String spec = "1 - 5, 1 - 4";
	writeTestString("test complex specification" + spec);
	Vector<uInt> expected(2);
	expected[0] = 1;
	expected[1] = 5;
	testGood(expected, spec);
}

void complexChannelTest2() {
	String spec = "9,12,1 - 5, 1 - 4, >14, 2-4";
	writeTestString("test complex specification" + spec);
	Vector<uInt> expected(8);
	expected[0] = 1;
	expected[1] = 5;
	expected[2] = 9;
	expected[3] = 9;
	expected[4] = 12;
	expected[5] = 12;
	expected[6] = 15;
	expected[7] = 30;
	testGood(expected, spec);
}

int main() {

    try {
    	noRangesTest();
    	singleChannelTest();
    	channelRangeTest();
    	lessThanChannelTest();
    	lessThanOrEqualChannelTest();
    	greaterThanChannelTest();
    	greaterThanOrEqualChannelTest();
    	complexChannelTest1();
    	complexChannelTest2();




    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    cout << "ok" << endl;
    return 0;
}

