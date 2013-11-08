//# dLatticeAsVector.cc: demo use of LatticeAsVector class on an XPixelCanvas
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
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <images/Images/TempImage.h>
#include <images/Images/PagedImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Inputs/Input.h>
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/LatticeAsVector.h>
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
void putData (Array<Complex>& data, const IPosition& pos,
              ImageInterface<Complex>& im);


int main(int argc, char **argv) {
	try {

		Input inputs(1);
		inputs.create("in", "", "Input image name?");
		inputs.create("scale", "1.0", "Amplitude Scale factor");
		inputs.create("color", "white", "Color");
		inputs.create("width", "1", "Width");
		inputs.create("arrow", "False", "Arrow");
		inputs.create("xinc", "1", "xinc");
		inputs.create("yinc", "1", "yinc");
		inputs.create("nx", "15", "nx");
		inputs.create("ny", "15", "ny");
		inputs.create("nz", "1", "nz");
		inputs.create("rotation", "0.0", "Extra rotatation");
		inputs.create("raster", "False", "Show Raster ?");
		inputs.create("ratio", "1.0", "Increment ratio");
		inputs.create("polarity", "1", "Polarity");
		inputs.create("copy", "False", "Copy TempImage to persistent image");
//
		inputs.readArguments(argc, argv);
		const String in = inputs.getString("in");
		const String color = inputs.getString("color");
		const Double scale = inputs.getDouble("scale");
		const Int width = inputs.getInt("width");
		const casa::Bool arrow = inputs.getBool("arrow");
		const Int nx = inputs.getInt("nx");
		const Int ny = inputs.getInt("ny");
		const Int nz = inputs.getInt("nz");
		const Int xinc = inputs.getInt("xinc");
		const Int yinc = inputs.getInt("yinc");
		const Double rotation = inputs.getDouble("rotation");
		const casa::Bool showRaster = inputs.getBool("raster");
		const Double ratio = inputs.getDouble("ratio");
		const Int polarity = inputs.getInt("polarity");
		const casa::Bool copy = inputs.getBool("copy");

// make a simple XWindow with embedded WorldCanvas:

		SimpleWorldCanvasApp app;
		WorldCanvas* wCanvas = app.worldCanvas();

// manage it with a WorldCanvasHolder:

		WorldCanvasHolder wcHolder(wCanvas);

// make image

		ImageInterface<Complex>* pImage = 0;
		ImageInterface<Float>* pImage2 = 0;
		if (in.empty()) {
			IPosition shape;
			DisplayCoordinateSystem cSys;
			Int nDim = 0;
			if (nz==1) {
				shape = IPosition(2, nx, ny);
				cSys = CoordinateUtil::defaultCoords2D();
				nDim = 2;
			} else {
				shape = IPosition(3, nx, ny, nz);
				cSys = CoordinateUtil::defaultCoords3D();
				nDim = 3;
			}
//
			Vector<Double> inc = cSys.increment();
			inc(1) *= ratio;
			cSys.setIncrement(inc);
			pImage = new TempImage<Complex>(shape, cSys);
//
			Int dny = ny / 3;
			IPosition shape2(shape);
			shape2(1) = dny;
			if (nDim==3) shape2(2) = 1;
			IPosition pos(nDim,0);
//
			Array<Complex> data(shape2);
			data.set(Complex(1.0,0.0));
			putData (data, pos, *pImage);
//
			pos(1) += dny;
			data.set(Complex(1.5*0.7071068, 1.5*0.7071068));
			putData (data, pos, *pImage);
//
			pos(1) += dny;
			data.set(Complex(0.0, 2.0));
			putData (data, pos, *pImage);
//
			if (showRaster) {
				IPosition shape3 = 2*shape;
				pImage2 = new TempImage<Float>(shape3, cSys);
				pImage2->set(1.0);
			}
//
			if (copy) {
				PagedImage<Complex> out(pImage->shape(), cSys, String("temp.persistent"));
				out.copyData(*pImage);
			}
		} else {
			String fileName(argv[1]);
			pImage = new PagedImage<Complex>(fileName);
			cout << "Loaded Image \"" << fileName << "\"" << endl;
		}
//
		const DisplayCoordinateSystem cSys = pImage->coordinates();
		const uInt nDim = pImage->ndim();
		if (nDim < 2) {
			throw(AipsError("image has less than two dimensions"));
		}

// a DisplayData to draw a vector image:

		LatticeAsVector<Complex>* lav;
		if (nDim == 2) {
			lav = new LatticeAsVector<Complex>(pImage, 0, 1);
		} else {
			IPosition fixedPos(nDim);
			fixedPos = 0;
			lav = new LatticeAsVector<Complex>(pImage, 0, 1, 2, fixedPos);
		}
		if (!lav) {
			throw(AipsError("couldn't build the display data"));
		}

// Set options

		{
			Record rec, recOut;
			rec.define("scale", Float(scale));
			rec.define("incx", xinc);
			rec.define("incy", yinc);
			rec.define("color", color);
			rec.define("line", width);
			rec.define("arrow", arrow);
			rec.define("rotation", rotation);
			rec.define("polarity", polarity);
			lav->setOptions(rec, recOut);
		}


// a DisplayData to draw a raster image:

		LatticeAsRaster<Float>* lar = 0;
		if (showRaster) {
			if (nDim == 2) {
				lar = new LatticeAsRaster<Float>(pImage2, 0, 1);
			} else {
				IPosition fixedPos(nDim);
				fixedPos = 0;
				lar = new LatticeAsRaster<Float>(pImage2, 0, 1, 2, fixedPos);
			}
			if (!lar) {
				throw(AipsError("couldn't build the display data"));
			}
		}

// a DisplayData to draw axis labels:

		AxesDisplayData* add;
		add = new AxesDisplayData();
		{
			Record rec, recOut;
			rec.define("xaxiscolor", "green");
			rec.define("labelcharfont", "italic");
			rec.define("labelcharsize", 1.3);
			add->setOptions(rec, recOut);
		}

// add the DisplayDatas to the display:

		if (showRaster) {
			wcHolder.addDisplayData((DisplayData *)lar);
		}
		wcHolder.addDisplayData((DisplayData *)lav);
		wcHolder.addDisplayData((DisplayData *)add);

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
//
		delete lav;
		delete lar;
		delete add;
		delete pImage;
		delete pImage2;
	} catch (const AipsError &x) {
		cerr << "Exception caught" << endl;
		cerr << "Message: " << x.getMesg() << endl;
		return 1;
	}
	return 0;
}



void putData (Array<Complex>& data, const IPosition& pos,
              ImageInterface<Complex>& im) {
	IPosition shp = im.shape();
	IPosition pos2(pos);
	if (shp.nelements()==3) {
		for (Int i=0; i<shp(2); i++) {
			Float dpa = i * 10.0 * C::pi / 180.0;
			Complex dpac(cos(dpa), sin(dpa));
			data *= dpac;
//
			pos2(2) = i;
			im.putSlice(data, pos2);
		}
	} else {
		im.putSlice(data, pos);
	}
}
