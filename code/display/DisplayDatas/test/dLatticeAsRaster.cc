//# dLatticeAsRaster.cc: demo use of LatticeAsRaster class on an XPixelCanvas
//# Copyright (C) 1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/aips.h>
#include <casa/System/Aipsrc.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Inputs/Input.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/FITSImage.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/AxesDisplayData.h>
#include <display/Display/Colormap.h>
#include <display/DisplayEvents/AniPosEH.h>
#include <display/Display/Attribute.h>
#include <display/DisplayEvents/WCRTZoomer.h>
#include <display/DisplayEvents/WCCrosshairTool.h>
#include <display/DisplayEvents/PCITFiddler.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
int main(int argc, char **argv) {
	try {

		Input inputs(1);
		inputs.create("in", "", "Input file name");
		inputs.create("fits", "F", "Is FITS file ?");
		inputs.create("abs", "T", "Absolute labels ?");
		inputs.readArguments(argc, argv);
		String in = inputs.getString("in");
		const casa::Bool isFITS = inputs.getBool("fits");
		const casa::Bool isAbs = inputs.getBool("abs");

// make a simple XWindow with embedded WorldCanvas:

		SimpleWorldCanvasApp app;
		WorldCanvas *wCanvas = app.worldCanvas();

// manage it with a WorldCanvasHolder:

		WorldCanvasHolder wcHolder(wCanvas);

		if(in=="") in = Aipsrc::aipsRoot()+"/data/demo/Images/test_image";

		ImageInterface<Float>* pImage = 0;
		if (isFITS) {
			cout << "Trying to load FITS Image \"" << in << "\"" << endl;
			pImage = new FITSImage(in);
		} else {
			cout << "Trying to load AIPS++ Image \"" << in << "\"" << endl;
			pImage = new PagedImage<Float>(in);
		}

// check image dimensions:

		uInt nDim = pImage->ndim();
		if (nDim < 2) {
			throw(AipsError("image has less than two dimensions"));
		}

// a DisplayData to draw a raster image:

		LatticeAsRaster<Float> *lar;
		if (nDim == 2) {
			lar = new LatticeAsRaster<Float>(pImage, 0, 1);
		} else {
			IPosition fixedPos(nDim);
			fixedPos = 0;
			lar = new LatticeAsRaster<Float>(pImage, 0, 1, 2, fixedPos);
		}
		if (!lar) {
			throw(AipsError("couldn't build the display data"));
		}

		// use a more interesting colormap:
		Colormap *cmap = new Colormap(String("Hot Metal 2"));
		lar->setColormap(cmap, 1.0);

//  Add axis labels

		Record rec0, recOut;
		rec0.define("axislabelswitch", True);
		if (isAbs) {
			rec0.define("axislabelabsrel", "absolute");
		} else {
			rec0.define("axislabelabsrel", "relative");
		}
		lar->setOptions(rec0, recOut);


// add the DisplayDatas to the display:

		wcHolder.addDisplayData((DisplayData *)lar);

// add some tools:

		WCRTZoomer zoomer(wCanvas);
		AniPosEH aniPosEH;
		aniPosEH.addWorldCanvasHolder(&wcHolder);
		PCITFiddler stdFiddler(wCanvas->pixelCanvas(),
		                       PCITFiddler::StretchAndShift,
		                       Display::K_Pointer_Button2);
		WCCrosshairTool crosshair(wCanvas, Display::K_Pointer_Button3);

// run the application

		app.run();

	} catch (const AipsError &x) {
		cerr << "Exception caught" << endl;
		cerr << "Message: " << x.getMesg() << endl;
		return 1;
	}
	return 0;
}
