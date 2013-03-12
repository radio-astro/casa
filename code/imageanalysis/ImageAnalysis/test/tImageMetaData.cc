//# tImageMetaData.cc:  test the ImageMetaData class
//# Copyright (C) 2009
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
//# $Id: tImageMetaData.cc 20749 2009-09-30 14:24:05Z gervandiepen $

#include <imageanalysis/ImageAnalysis/ImageMetaData.h>
#include <images/Images/FITSImage.h>
#include <casa/OS/Path.h>
#include <casa/OS/EnvVar.h>
#include <casa/namespace.h>
#include <casa/OS/File.h>

int main() {
    try {
    	String casapath = EnvironmentVariable::get("CASAPATH");
    	if (casapath.empty()) {
    		cerr << "CASAPATH env variable not defined. Can't find fixtures. Did you source the casainit.(c)sh file?" << endl;
    		return 1;
    	}

    	String *parts = new String[2];
    	split(casapath, parts, 2, String(" "));
    	String datadir = parts[0] + "/data/regression/unittest/imageanalysis/ImageAnalysis/";
    	delete [] parts;

        FITSImage fourAxesImage(datadir + "ngc5921.clean.fits");
        FITSImage twoAxesImage(datadir + "ngc5921.clean.no_freq.no_stokes.fits");
   
        ImageMetaData<Float> fourAxesImageMetaData(&fourAxesImage);
        ImageMetaData<Float> twoAxesImageMetaData(&twoAxesImage);
        {
            AlwaysAssert(fourAxesImageMetaData.nChannels() == 8, AipsError);
            AlwaysAssert(twoAxesImageMetaData.nChannels() == 0, AipsError);
        }

        {
            AlwaysAssert(fourAxesImageMetaData.isChannelNumberValid(1), AipsError);
            AlwaysAssert(! fourAxesImageMetaData.isChannelNumberValid(10), AipsError);
            AlwaysAssert(! twoAxesImageMetaData.isChannelNumberValid(0), AipsError);
        }

        {
            AlwaysAssert(fourAxesImageMetaData.stokesPixelNumber("I") == 0, AipsError);
            AlwaysAssert(fourAxesImageMetaData.stokesPixelNumber("Q") == -1, AipsError);
            AlwaysAssert(twoAxesImageMetaData.stokesPixelNumber("I") == -1, AipsError);
        }

        {
            AlwaysAssert(fourAxesImageMetaData.nStokes() == 1, AipsError);
            AlwaysAssert(twoAxesImageMetaData.nStokes() == 0, AipsError);
        }
    
        {
            AlwaysAssert(fourAxesImageMetaData.isStokesValid("I"), AipsError);
            AlwaysAssert(! fourAxesImageMetaData.isStokesValid("Q"), AipsError);
            AlwaysAssert(! twoAxesImageMetaData.isStokesValid("I"), AipsError);
        }

        {
        	// stokesAtPixel
        	AlwaysAssert(twoAxesImageMetaData.stokesAtPixel(0).empty(), AipsError);
        	AlwaysAssert(fourAxesImageMetaData.stokesAtPixel(0) == "I", AipsError);
        	AlwaysAssert(fourAxesImageMetaData.stokesAtPixel(1).empty(), AipsError);

        }

        {
            // TODO test image without a direction coordinate
            Vector<Int> directionShape = fourAxesImageMetaData.directionShape();
            AlwaysAssert(directionShape[0] == 6, AipsError);
            AlwaysAssert(directionShape[1] == 11, AipsError);

            directionShape = twoAxesImageMetaData.directionShape();
            AlwaysAssert(directionShape[0] == 6, AipsError);
            AlwaysAssert(directionShape[1] == 11, AipsError);
        } 

        {
            String message;
            AlwaysAssert(fourAxesImageMetaData.areChannelAndStokesValid(message, 1, "I"), AipsError);
            AlwaysAssert(! fourAxesImageMetaData.areChannelAndStokesValid(message, 15, "I"), AipsError);
            AlwaysAssert(! fourAxesImageMetaData.areChannelAndStokesValid(message, 1, "Q"), AipsError);
            AlwaysAssert(! twoAxesImageMetaData.areChannelAndStokesValid(message, 0, "I"), AipsError);
        }
        {
            /*
        	// getBeamArea
        	Double beamArea;
        	AlwaysAssert( fourAxesImageMetaData.getBeamArea(beamArea, "arcsec.arcsec"), AipsError);
        	//beamArea.convert(Unit("arcsec.arcsec"));
        	Double expectedArea = 2769.2432412865101;
        	AlwaysAssert(near(beamArea, expectedArea, 1e-8), AipsError);
        	FITSImage noBeam("jyperpixelimage.fits");
        	ImageMetaData noBeamMD = ImageMetaData(noBeam);
        	AlwaysAssert(! noBeamMD.getBeamArea(beamArea, "arcsec.arcsec"), AipsError);
            */
        }
        {
        	cout << "*** Test constructor" << endl;
        	TempImage<Float> x(
        		TiledShape(IPosition(4, 30, 30, 4, 30)),
        		CoordinateUtil::defaultCoords4D()
        	);
        	ImageInfo info = x.imageInfo();
        	GaussianBeam beam(
        		Quantity(4, "arcsec"), Quantity(2, "arcsec"),
        		Quantity(10, "deg")
        	);
        	info.setRestoringBeam(beam);
        	x.setImageInfo(info);
        	x.putAt(-20.5, IPosition(4, 1, 3, 2, 20));
        	x.putAt(92.6, IPosition(4, 1, 4, 2, 6));
        	ImageMetaData<Float> header(&x);
        	Record headRec = header.toRecord(True);
        	cout << "header looks like:" << endl;
        	headRec.print(cout);
        }

        cout<< "ok"<< endl;
    }
    catch (const AipsError& x) {
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    return 0;
}

