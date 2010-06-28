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

#include <images/Images/ImageInputProcessor.h>
#include <images/Images/FITSImage.h>
#include <casa/namespace.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}

void _checkCorner(const Record& gotRecord, const Vector<Double>& expected) {
	for (uInt i=0; i<expected.size(); i++) {
		uInt fieldNumber = i+1;
		Double got = gotRecord.asRecord(RecordFieldId("*" + String::toString(fieldNumber)))
			.asDouble(RecordFieldId("value"));
	    AlwaysAssert(fabs((got-expected[i])/expected[i]) < 1e-9, AipsError);
	}
}

int main() {
    try {
    	ImageInputProcessor processor;
    	ImageInterface<Float> *image;
    	Record region;
    	String diagnostics;
    	String goodImage = "image_input_processor.im";
		Bool fail = True;
    	try {
    		writeTestString("Bad image name throws exception");
    		processor.process(
    			image, region, diagnostics, "bogus_image", 0, "", "", "", "",
    			ImageInputProcessor::USE_ALL_STOKES
    		);
    		// should not get here
    		fail = False;
    		AlwaysAssert(false, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad region name throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "bogus.rgn", "", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(false, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad region name in another image throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "bogus.im:bogus.rgn", "", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #1 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "-1,0,10,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #2 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "0,-1,10,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = false;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #3 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "0,0,100 ,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #4 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,100", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #5 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "5, 0, 0,10 ,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
    	try {
    		writeTestString("Bad box spec #6 throws exception");
    		processor.process(image, region, diagnostics, goodImage, 0, "", "a, 0,10 ,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
    		// should not get here
    		fail = False;
    		AlwaysAssert(False, AipsError);
    	}
    	catch (AipsError) {
    		// should get here with fail = true
    		AlwaysAssert(fail, AipsError);
    	}
       	try {
        	writeTestString("Bad box spec #7 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "1a, 0,10 ,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #1 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "1", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #2 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "a", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #3 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "a-b", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #4 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "0-b", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #5 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", ">0", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #6 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "-1", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid channel spec #7 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "<5", "", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid stokes spec #1 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "", "b", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
       	try {
        	writeTestString("Valid box spec with invalid stokes spec #2 throws exception");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,10 ,10", "", "yy", ImageInputProcessor::USE_ALL_STOKES);
        	// should not get here
        	fail = False;
        	AlwaysAssert(False, AipsError);
        }
        catch (AipsError) {
        	// should get here with fail = true
        	AlwaysAssert(fail, AipsError);
        }
    	Vector<Double> expectedBlc(4);
    	expectedBlc[0] = 1.24795230e+00;
    	expectedBlc[1] = 7.82549990e-01;
    	expectedBlc[2] = 4.73510000e+09;
    	expectedBlc[3] = 1;
    	Vector<Double> expectedTrc(4);
    	expectedTrc[0] = 1.24793182e+00;
    	expectedTrc[1] = 7.82564556e-01;
    	expectedTrc[2] = 4.73510000e+09;
    	expectedTrc[3] = 4;
        {
        	writeTestString("Valid box specification succeeds");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10", "", "", ImageInputProcessor::USE_ALL_STOKES);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification with valid channel specification #1 succeeds");
        	processor.process(image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10", "0-0", "", ImageInputProcessor::USE_ALL_STOKES);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification with valid channel specification #2 succeeds");
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10",
        		"0", "", ImageInputProcessor::USE_ALL_STOKES
        	);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification with valid stokes specification #1 succeeds");
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10",
        		"", "QVIU", ImageInputProcessor::USE_ALL_STOKES
        	);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification with valid stokes specification #2 succeeds");
        	expectedTrc[3] = 3;
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10", "",
        		"QIU", ImageInputProcessor::USE_ALL_STOKES
        	);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification with valid stokes specification #3 succeeds");
        	expectedBlc[3] = expectedTrc[3] = 2;
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10",
        		"", "Q", ImageInputProcessor::USE_ALL_STOKES
        	);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification using all polarizations for blank stokes");
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10",
        		"", "", ImageInputProcessor::USE_ALL_STOKES
        	);
        	expectedBlc[3] = 1;
        	expectedTrc[3] = 4;
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
        {
        	writeTestString("Valid box specification using first polarizations for blank stokes");
        	expectedTrc[3] = 1;
        	expectedBlc[3] = 1;
        	processor.process(
        		image, region, diagnostics, goodImage, 0, "", "0, 0,  10,10",
        		"", "  ", ImageInputProcessor::USE_FIRST_STOKES
        	);
        	_checkCorner(region.asRecord(RecordFieldId("blc")), expectedBlc);
        	_checkCorner(region.asRecord(RecordFieldId("trc")), expectedTrc);
        }
    }
    catch (AipsError x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    cout << "ok" << endl;
    return 0;
}

