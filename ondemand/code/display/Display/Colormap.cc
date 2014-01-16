//# Colormap.cc: generating and selecting colors from a look-up map
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2002
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

#include <casa/Exceptions/Error.h>
#include <scimath/Functionals/Polynomial.h>
#include <casa/Logging/LogIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/ColormapDefinition.h>
#include <display/Display/PixelCanvasColorTable.h>
#include <display/Display/Colormap.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
	Colormap::Colormap() :
		itsName("mono"),
		itsIsRigid(False),
		itsRigidSize(0),
		itsAlpha(1),
		itsBrightness(0.5),
		itsBrightnessScale(0.0),
		itsContrast(0.5),
		itsContrastScale(1.0),
		itsInvertRed(False),
		itsInvertGreen(False),
		itsInvertBlue(False),
		itsPCColorTables(0, 4) {
		itsColormapDefinition = new ColormapDefinition(itsName);
		itsOwnShapingFunction = False;
		setShapingFunction(0);
	}

// User Constructor
	Colormap::Colormap(const String& name) :
		itsName(name),
		itsIsRigid(False),
		itsRigidSize(0),
		itsAlpha(1),
		itsBrightness(0.5),
		itsBrightnessScale(0.0),
		itsContrast(0.5),
		itsContrastScale(1.0),
		itsInvertRed(False),
		itsInvertGreen(False),
		itsInvertBlue(False),
		itsPCColorTables(0, 4) {
		itsColormapDefinition = new ColormapDefinition(itsName);
		itsOwnShapingFunction = False;
		setShapingFunction(0);
	}

// compute the maps
	Bool Colormap::calcRGBMaps(uInt reqSize,
	                           Vector<Float> & redMap,
	                           Vector<Float> & greenMap,
	                           Vector<Float> & blueMap,
	                           Vector<Float> & alphaMap) const {
		if (rigid() && itsRigidSize != reqSize)
			return False;

		redMap.resize(reqSize);
		greenMap.resize(reqSize);
		blueMap.resize(reqSize);
		alphaMap.resize( reqSize );

		Double lenpme = reqSize-1 + 0.00001;

		for (uInt i = 0; i < reqSize; i++) {
			// obtain function values
			itsColormapDefinition->getValue((*itsShapingFunction)
			                                (Float(Double(i) / lenpme)),
			                                redMap(i), greenMap(i), blueMap(i));

			// apply contrast correction
			redMap(i) = max(0.0f, min(1.0f, (redMap(i) - 0.5f) *
			                          itsContrastScale + 0.5f));
			greenMap(i) = max(0.0f, min(1.0f, (greenMap(i) - 0.5f) *
			                            itsContrastScale + 0.5f));
			blueMap(i) = max(0.0f, min(1.0f, (blueMap(i) - 0.5f) *
			                           itsContrastScale + 0.5f));
			alphaMap(i) = itsAlpha;

			// apply inversions
			if (itsInvertRed) {
				redMap(i) = 1.0 - redMap(i);
			}
			if (itsInvertGreen) {
				greenMap(i) = 1.0 - greenMap(i);
			}
			if (itsInvertBlue) {
				blueMap(i) = 1.0 - blueMap(i);
			}

			// apply brightness correction - AFTER inversions!
			redMap(i) = max(0.0f, min(1.0f, redMap(i) + itsBrightnessScale));
			greenMap(i) = max(0.0f, min(1.0f, greenMap(i) + itsBrightnessScale));
			blueMap(i) = max(0.0f, min(1.0f, blueMap(i) + itsBrightnessScale));

		}

		return True;
	}

// register a PixelCanvasColorTable
	void Colormap::registerPCColorTable(PixelCanvasColorTable *pcctbl) {
		// do we have this one already?
		if (itsPCColorTables.isDefined(pcctbl)) {
			// yes, increment ref count
			itsPCColorTables.define(pcctbl, itsPCColorTables(pcctbl) + 1);
		} else {
			// new definition, referenced once
			itsPCColorTables.define(pcctbl, 1);
		}
	}

// unregister a PixelCanvasColorTable
	void Colormap::unregisterPCColorTable(PixelCanvasColorTable *pcctbl) {
		// do we have this one?
		if (itsPCColorTables.isDefined(pcctbl)) {
			// yes, find refcount
			uInt val = itsPCColorTables(pcctbl);
			if (val > 1) {
				// reduce refcount
				itsPCColorTables.define(pcctbl, val - 1);
			} else {
				// remove it
				itsPCColorTables.remove(pcctbl);
			}
		} else {
			// error
		}
	}

// reinstall the colormap on the PixelCanvasColorTables that use it
	void Colormap::reinstall() {
		for (uInt i = 0; i < itsPCColorTables.ndefined(); i++) {
			(itsPCColorTables.getKey(i))->colormapManager().reinstallColormaps();
		}
	}

// do resizeCallbacks on the PixelCanvasColorTables that use this
	void Colormap::doResizeCallbacks() {
		for (uInt i = 0; i < itsPCColorTables.ndefined(); i++) {
			(itsPCColorTables.getKey(i))->doResizeCallbacks();
		}
	}

// Destructor
	Colormap::~Colormap() {
		if (itsOwnShapingFunction) {
			delete itsShapingFunction;
		}
		delete itsColormapDefinition;
	}

	void Colormap::setBrightness(const Float &brightness,
	                             const Bool &doReinstall) {
		itsBrightness = max(0.0f, min(1.0f, brightness));
		itsBrightnessScale = (itsBrightness - 0.5f) * 2.0f;
		if (doReinstall) {
			reinstall();
		}
	}

	void Colormap::setAlpha(const Float &alpha, const Bool &doReinstall) {
		itsAlpha = max(0.0f, min(1.0f, alpha));
		if (doReinstall) {
			reinstall();
		}
	}

	void Colormap::setContrast(const Float &contrast, const Bool &doReinstall) {
		itsContrast = max(0.0f, min(1.0f, contrast));
		itsContrastScale = pow(10.0, itsContrast / 0.5 - 1.0);
		if (doReinstall) {
			reinstall();
		}
	}

	void Colormap::setInvertFlags(const Bool &red, const Bool &green,
	                              const Bool &blue, const Bool &doReinstall) {
		itsInvertRed = red;
		itsInvertGreen = green;
		itsInvertBlue = blue;
		if (doReinstall) {
			reinstall();
		}
	}

	void Colormap::getInvertFlags(Bool &red, Bool &green, Bool &blue) const {
		red = itsInvertRed;
		green = itsInvertGreen;
		blue = itsInvertBlue;
	}

	void Colormap::setShapingFunction(Function1D<Float> *shapingfunc) {
		if (!shapingfunc) {
			if (!itsOwnShapingFunction) {
				itsShapingFunction = new Polynomial<Float>(1);
				Vector<Float> params(2);
				params(0) = 0.0;
				params(1) = 1.0;
				setShapingCoefficients(params);
				itsOwnShapingFunction = True;
			}
		} else {
			if (itsOwnShapingFunction) {
				delete itsShapingFunction;
				itsOwnShapingFunction = False;
			}
			itsShapingFunction = shapingfunc;
		}
	}

	void Colormap::setShapingCoefficients(const Vector<Float> &params,
	                                      const Bool &doReinstall) {
		itsShapingFunction->parameters().setParameters(params);
		if (doReinstall) {
			reinstall();
		}
	}

	void Colormap::setColormapDefinition( ColormapDefinition* definition ) {
		itsColormapDefinition = definition;
	}

	const Vector<Float> Colormap::getShapingCoefficients() const {
		return itsShapingFunction->parameters().getParameters();
	}

//========================= Standardized Functions ============================

// write to ostream support
	ostream & operator << (ostream & os, const Colormap & c) {
		// remove this warning when edited
		os << "[name=" << c.itsName << ",";
		if (c.itsIsRigid) os << "rigid(" << c.itsRigidSize << "),";

		Vector<Float> redMap;
		Vector<Float> greenMap;
		Vector<Float> blueMap;
		Vector<Float> alphaMap;

		uInt sz = (c.itsIsRigid) ? c.itsRigidSize : 20;

		os << "values for size of " << sz << ":";

		c.calcRGBMaps(sz, redMap, greenMap, blueMap, alphaMap );
		for (uInt i = 0; i < sz; i++) {
			uInt r = (uInt) (redMap(i) * 255.0);
			uInt g = (uInt) (greenMap(i) * 255.0);
			uInt b = (uInt) (blueMap(i) * 255.0);
			uInt alpha = (uInt)(alphaMap(i) * 255.0);
			os << "<" << r << "," << g << "," << b << "," << alpha<<">";
		}

		os << "]";

		return os;
	}

// write to AipsiO support
	AipsIO & operator << (AipsIO & aio, const Colormap &) {
		aio.putstart("Colormap", Colormap::ColormapVersion);

		// write values here.  check out aio.put

		// remove this warning when edited
		cerr << "Warning: class Colormap, AipsIO op << not completed";

		aio.putend();
		return aio;
	}

// write to LogIO support
	LogIO & operator << (LogIO & lio, const Colormap & c) {
		lio.output() << c;
		return lio;
	}

// read from AipsIO support
	AipsIO & operator >> (AipsIO & aio, Colormap &) {
		if (aio.getstart("Colormap") != Colormap::ColormapVersion) {
			throw(AipsError("AipsIO &operator>>(AipsIO &aio, Colormap &c) - "
			                "version on disk and in class do not match"));
		}

		// read values, initialize structures.  check out aio.getnew

		// remove this warning when edited
		cerr << "Warning: class Colormap, AipsIO op >> not completed";

		aio.getend();

		return aio;
	}


} //# NAMESPACE CASA - END

