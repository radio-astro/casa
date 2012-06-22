//# LatticeAsMarker.cc: Class to display lattice objects as markers
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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
#include <display/DisplayDatas/LatticeAsMarker.h>

#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <display/DisplayDatas/LatticePADMMarker.h>
#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/DataType.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template <class T>
LatticeAsMarker<T>::LatticeAsMarker(Array<T>* array, const uInt xAxis,
                                    const uInt yAxis, const uInt mAxis,
                                    const IPosition fixedPos)
 : LatticePADisplayData<T>(array, xAxis, yAxis, mAxis, fixedPos)
{
  setupElements();
  setDefaultOptions();
}

template <class T>
LatticeAsMarker<T>::LatticeAsMarker(Array<T>* array, const uInt xAxis,
                                    const uInt yAxis)
 : LatticePADisplayData<T>(array, xAxis, yAxis)
{
  setupElements();
  setDefaultOptions();
}

template <class T>
LatticeAsMarker<T>::LatticeAsMarker(ImageInterface<T>* image,
                                    const uInt xAxis, const uInt yAxis,
                                    const uInt mAxis,
                                    const IPosition fixedPos) 
 : LatticePADisplayData<T>(image, xAxis, yAxis, mAxis, fixedPos)
{
  setupElements();
  setDefaultOptions();
}

template <class T>
LatticeAsMarker<T>::LatticeAsMarker(ImageInterface<T>* image,
                                    const uInt xAxis, const uInt yAxis) 
 : LatticePADisplayData<T>(image, xAxis, yAxis)
{
  setupElements();
  setDefaultOptions();
}

template <class T>
LatticeAsMarker<T>::~LatticeAsMarker() 
{
  for (uInt i = 0; i < nelements(); i++) {
    if (DDelement[i]) {
       delete (static_cast<LatticePADMMarker<T>*>(DDelement[i]));
    }
  }  
}


template <class T>
void LatticeAsMarker<T>::setupElements()
{

  for (uInt i=0; i<nelements(); i++) if(DDelement[i]!=0) {
    delete static_cast<LatticePADMMarker<T>*>(DDelement[i]);
    DDelement[i]=0;  }
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
          (new LatticePADMMarker<T>(dispAxes(0), dispAxes(1), dispAxes(2),
                                    fixedPos, this));
    }
  } else {
    setNumImages(1);
    DDelement.resize(nelements());
    DDelement[0] = dynamic_cast<LatticePADisplayMethod<T>*>
        (new LatticePADMMarker<T>(dispAxes(0), dispAxes(1), this));
  }
  PrincipalAxesDD::setupElements();
}


template <class T>
void LatticeAsMarker<T>::setDefaultOptions() 
{
  LatticePADisplayData<T>::setDefaultOptions();
//
  itsScale = 1.0;
  itsLineWidth = 1.0;

// We try to make the initial increments so that not too many
// markers are drawn 
  
  IPosition shape = dataShape();
  itsIncX = max(3,shape(0) / 100);
  itsIncY = max(3,shape(1) / 100);
  itsColor = "foreground";
  itsMarker = "square";
}


template <class T>
Bool LatticeAsMarker<T>::setOptions(Record &rec, Record &recOut) 
{
  Bool ret = LatticePADisplayData<T>::setOptions(rec, recOut);
//
  Bool localchange = False;
  Bool error;
//
  localchange = readOptionRecord(itsScale, error, rec, "scale") ||
                localchange;
  localchange = readOptionRecord(itsIncX, error, rec, "incx") ||
                localchange;
  localchange = readOptionRecord(itsIncY, error, rec, "incy") ||
                localchange;
  localchange = readOptionRecord(itsLineWidth, error, rec, "line") ||
                localchange;
  localchange = readOptionRecord(itsColor, error, rec, "color") ||
 	        localchange;
  localchange = readOptionRecord(itsMarker, error, rec, "marker") ||
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
Record LatticeAsMarker<T>::getOptions() 
{
  Record rec = LatticePADisplayData<T>::getOptions();
  Record unset;
  unset.define("i_am_unset", "i_am_unset");
//
// phasetype and debiasing are only meaningful for COmplex data
//
  T* dummy;
  DataType type = whatType(dummy);
  AlwaysAssert(type==TpFloat || type==TpComplex, AipsError);
//
  Record marker;
  marker.define("dlformat", "marker");
  marker.define("listname", "Marker shape");
  marker.define("ptype", "choice");
  Vector<String> markerShape(1);
  markerShape(0) = "square";
  marker.define("popt", markerShape);
  marker.define("default", "square");
  marker.define("value", itsMarker);
  marker.define("allowunset", False);
  rec.defineRecord("marker", marker);
//
  Record scale;
  scale.define("dlformat", "scale");
  scale.define("listname", "Scale factor");
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
  color.define("listname", "Marker color");
  color.define("ptype", "userchoice");
  Vector<String> vcolor(11);
  vcolor(0) = "foreground"; vcolor(1) = "background";
  vcolor(2) = "black"; vcolor(3) = "white";
  vcolor(4) = "red"; vcolor(5) = "green";
  vcolor(6) = "blue"; vcolor(7) = "cyan";
  vcolor(8) = "magenta"; vcolor(9) = "yellow";
  vcolor(10) = "gray";
  color.define("popt", vcolor);
  color.define("default", "foreground");
  color.define("value", itsColor);
  color.define("allowunset", False);
  rec.defineRecord("color", color);
//
  return rec;
}

} //# NAMESPACE CASA - END

