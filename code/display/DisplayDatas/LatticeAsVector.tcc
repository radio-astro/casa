//# LatticeAsVector.cc: Class to display lattice objects as vector fields
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
#include <display/DisplayDatas/LatticeAsVector.h>

#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <display/DisplayDatas/LatticePADMVector.h>
#include <images/Images/ImageInterface.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/DataType.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	LatticeAsVector<T>::LatticeAsVector(Array<T>* array, const uInt xAxis,
	                                    const uInt yAxis, const uInt mAxis,
	                                    const IPosition fixedPos)
		: LatticePADisplayData<T>(array, xAxis, yAxis, mAxis, fixedPos),
		  itsUnits(Unit()) {
		setupElements();
		setDefaultOptions();
	}

	template <class T>
	LatticeAsVector<T>::LatticeAsVector(Array<T>* array, const uInt xAxis,
	                                    const uInt yAxis)
		: LatticePADisplayData<T>(array, xAxis, yAxis),
		  itsUnits(Unit()) {
		setupElements();
		setDefaultOptions();
	}

	template <class T>
	LatticeAsVector<T>::LatticeAsVector(SHARED_PTR<ImageInterface<T> > image,
	                                    const uInt xAxis, const uInt yAxis,
	                                    const uInt mAxis,
	                                    const IPosition fixedPos)
		: LatticePADisplayData<T>(image, xAxis, yAxis, mAxis, fixedPos),
		  itsUnits(Unit()) {
		setupElements();
		setDefaultOptions();
		itsUnits = image->units();
	}

	template <class T>
	LatticeAsVector<T>::LatticeAsVector(SHARED_PTR<ImageInterface<T> > image,
	                                    const uInt xAxis, const uInt yAxis)
		: LatticePADisplayData<T>(image, xAxis, yAxis),
		  itsUnits(Unit()) {
		setupElements();
		setDefaultOptions();
		itsUnits = image->units();
	}

	template <class T>
	LatticeAsVector<T>::~LatticeAsVector() {
		for (uInt i = 0; i < nelements(); i++) {
			if (DDelement[i]) {
				delete (static_cast<LatticePADMVector<T>*>(DDelement[i]));
			}
		}
	}


	template <class T>
	void LatticeAsVector<T>::setupElements() {
		for (uInt i=0; i<nelements(); i++) if(DDelement[i]!=0) {
				delete static_cast<LatticePADMVector<T>*>(DDelement[i]);
				DDelement[i]=0;
			}
		// Delete old DMs, if any.

		IPosition fixedPos = fixedPosition();
		Vector<Int> dispAxes = displayAxes();
		AlwaysAssert(dispAxes.nelements()>=2,AipsError);
//
		if (nPixelAxes > 2) {
			setNumImages(dataShape()(dispAxes(2)));
			DDelement.resize(nelements());
			for (uInt index = 0; index < nelements(); index++) {
				fixedPos(dispAxes(2)) = index;
//
				DDelement[index] = dynamic_cast<LatticePADisplayMethod<T>*>
				                   (new LatticePADMVector<T>(dispAxes(0), dispAxes(1), dispAxes(2),
				                           fixedPos, this));
			}
		} else {
			setNumImages(1);
			DDelement.resize(nelements());
			DDelement[0] = dynamic_cast<LatticePADisplayMethod<T>*>
			               (new LatticePADMVector<T>(dispAxes(0), dispAxes(1), this));
		}
		PrincipalAxesDD::setupElements();
	}


	template <class T>
	void LatticeAsVector<T>::setDefaultOptions() {
		LatticePADisplayData<T>::setDefaultOptions();
//
		itsScale = 1.0;
		itsLineWidth = 1.0;

// We try to make the initial increments so that not too many
// vectors are drawn because it's  quite time consuming
// drawing billions of little vectors

		IPosition shape = dataShape();
		itsIncX = max(3,shape(0) / 100);
		itsIncY = max(3,shape(1) / 100);
		LogIO os(LogOrigin("LatticeAsVector", "setDefaultOptions", WHERE));
		os << LogIO::NORMAL << "Initial X and Y increments set to " << itsIncX << ", " << itsIncY << endl;
		os << "Use Adjust GUI to change to suit" << LogIO::POST;
//
		itsArrow = False;
		itsBarb = 0.3;
		itsColor = "foreground";
		itsRotation = 0.0;
		itsConstAmp = False;
//
		T* dummy = NULL;
		DataType type = whatType(dummy);
		AlwaysAssert(type==TpFloat || type==TpComplex, AipsError);
		if (type == TpFloat) {
			itsPhaseType = "normal";
			itsVar = 0.0;
		} else {
			itsPhaseType = "polarimetric";
			itsVar = getVariance();
		}
		itsDebias = False;
	}


	template <class T>
	Bool LatticeAsVector<T>::setOptions(Record &rec, Record &recOut) {
		Bool ret = LatticePADisplayData<T>::setOptions(rec, recOut);
//
		Bool localchange = False;
		Bool error;
//
		localchange = readOptionRecord(itsDebias, error, rec, "debias") ||
		              localchange;
		localchange = readOptionRecord(itsVar, error, rec, "variance") ||
		              localchange;
		localchange = readOptionRecord(itsPhaseType, error, rec, "phasetype") ||
		              localchange;
		localchange = readOptionRecord(itsScale, error, rec, "scale") ||
		              localchange;
		localchange = readOptionRecord(itsIncX, error, rec, "incx") ||
		              localchange;
		localchange = readOptionRecord(itsIncY, error, rec, "incy") ||
		              localchange;
		localchange = readOptionRecord(itsArrow, error, rec, "arrow") ||
		              localchange;
		localchange = readOptionRecord(itsBarb, error, rec, "barb") ||
		              localchange;
		localchange = readOptionRecord(itsRotation, error, rec, "rotation") ||
		              localchange;
		localchange = readOptionRecord(itsLineWidth, error, rec, "line") ||
		              localchange;
		localchange = readOptionRecord(itsColor, error, rec, "color") ||
		              localchange;
		localchange = readOptionRecord(itsConstAmp, error, rec, "constamp") ||
		              localchange;
//
// must come last - this forces ret to be True or False:
//
		if (rec.isDefined("refresh") && (rec.dataType("refresh") == TpBool)) {
			rec.get("refresh", ret);
		}
//
		ret = ret || localchange;
		return ret;
	}


	template <class T>
	Record LatticeAsVector<T>::getOptions( bool scrub ) const {
		Record rec = LatticePADisplayData<T>::getOptions(scrub);
		Record unset;
		unset.define("i_am_unset", "i_am_unset");
//
// phasetype and debiasing are only meaningful for COmplex data
//
		T* dummy = NULL;
		DataType type = whatType(dummy);
		AlwaysAssert(type==TpFloat || type==TpComplex, AipsError);
		if (type == TpComplex) {
			Record phasetype;
			phasetype.define("dlformat", "phasetype");
			phasetype.define("listname", "Phase Type");
			phasetype.define("ptype", "choice");
			Vector<String> values(2);
			values(0) = "normal";
			values(1) = "polarimetric";
			phasetype.define("popt", values);
			phasetype.define("default", values(1));
			phasetype.define("value", itsPhaseType);
			phasetype.define("allowunset", False);
			rec.defineRecord("phasetype", phasetype);
//
			Record constamp;
			constamp.define("dlformat", "constamp");
			constamp.define("listname", "Constant amplitude ?");
			constamp.define("ptype", "boolean");
			constamp.define("default", False);
			constamp.define("value", itsConstAmp);
			constamp.define("allowunset", False);
			rec.defineRecord("constamp", constamp);
//
			Record debias;
			debias.define("dlformat", "debias");
			debias.define("listname", "Debias amplitude ?");
			debias.define("ptype", "boolean");
			debias.define("default", Bool(False));
			debias.define("value", itsDebias);
			debias.define("allowunset", False);
			rec.defineRecord("debias", debias);
//
			Record var;
			var.define("dlformat", "variance");
			var.define("listname", "Variance of noise for debiasing");
			var.define("ptype", "scalar");
			var.define("default", itsVar);
			var.define("value", itsVar);
			var.define("allowunset", False);
			rec.defineRecord("var", var);
		}
//
		Record scale;
		scale.define("dlformat", "scale");
		scale.define("listname", "Amplitude scale factor");
		scale.define("ptype", "scalar");
		scale.define("default", Float(1.0));
		scale.define("value", itsScale);
		scale.define("allowunset", False);
		rec.defineRecord("scale", scale);
//
		Record incX;
		incX.define("dlformat", "incx");
		incX.define("listname", "X-increment");
		incX.define("ptype", "intrange");
		incX.define("pmin", 1);
		incX.define("pmax", itsIncX * 2);
		incX.define("default", 3);
		incX.define("value", itsIncX);
		incX.define("provideentry", True);
		incX.define("allowunset", False);
		rec.defineRecord("incx", incX);
//
		Record incY;
		incY.define("dlformat", "incy");
		incY.define("listname", "Y-increment");
		incY.define("ptype", "intrange");
		incY.define("pmin", 1);
		incY.define("pmax", itsIncY*2);
		incY.define("default", 3);
		incY.define("value", itsIncY);
		incY.define("provideentry", True);
		incY.define("allowunset", False);
		rec.defineRecord("incy", incY);
//
		Record rot;
		rot.define("dlformat", "rotation");
		rot.define("listname", "Extra rotation");
		rot.define("ptype", "scalar");
		rot.define("default", Float(0.0));
		rot.define("value", itsRotation);
		rot.define("allowunset", False);
		rec.defineRecord("rotation", rot);
//
		Record arrow;
		arrow.define("dlformat", "arrow");
		arrow.define("listname", "Show arrow heads ?");
		arrow.define("ptype", "boolean");
		arrow.define("default", Bool(False));
		arrow.define("value", itsArrow);
		arrow.define("allowunset", False);
		rec.defineRecord("arrow", arrow);
//
		Record barb;
		barb.define("dlformat", "barb");
		barb.define("listname", "Arrow head shape");
		barb.define("ptype", "floatrange");
		barb.define("pmin", Float(0.0));
		barb.define("pmax", Float(1.0));
		barb.define("presolution", Float(0.1));
		barb.define("default", Float(0.3));
		barb.define("value", itsBarb);
		barb.define("allowunset", False);
		rec.defineRecord("barb", barb);
//
		Record line;
		line.define("dlformat", "line");
		line.define("listname", "Line width");
		line.define("ptype", "floatrange");
		line.define("pmin", Float(0.0));
		line.define("pmax", Float(5.0));
		line.define("presolution", Float(0.1));
		line.define("default", Float(0.5));
		line.define("value", itsLineWidth);
		line.define("allowunset", False);
		rec.defineRecord("line", line);
//
		Record color;
		color.define("dlformat", "color");
		color.define("listname", "Line color");
		color.define("ptype", "userchoice");
		Vector<String> vcolor(11);
		vcolor(0) = "foreground";
		vcolor(1) = "background";
		vcolor(2) = "black";
		vcolor(3) = "white";
		vcolor(4) = "red";
		vcolor(5) = "green";
		vcolor(6) = "blue";
		vcolor(7) = "cyan";
		vcolor(8) = "magenta";
		vcolor(9) = "yellow";
		vcolor(10) = "gray";
		color.define("popt", vcolor);
		color.define("default", "foreground");
		color.define("value", itsColor);
		color.define("allowunset", False);
		rec.defineRecord("color", color);
//
		return rec;
	}


#if ! defined(__APPLE__)
// This is specialized for each type of T. Defining it here
// and specializing results in multiply defined symbols on
// Mac OSX
	template <class T>
	const T LatticeAsVector<T>::dataValue(IPosition pos) {

		// Default template (needed by compiler(?); probably not used).

		return LatticePADisplayData<T>::dataValue(pos);
	}
#endif

} //# NAMESPACE CASA - END

