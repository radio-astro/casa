//# LatticeAsContour.cc: Class to display lattice objects as contoured images
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
#include <casa/System/Aipsrc.h>
#include <casa/System/AipsrcValue.h>
#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <lattices/Lattices/Lattice.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/LatticePADMContour.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <casa/BasicMath/Math.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// >2d array-based ctor
template <class T>
LatticeAsContour<T>::LatticeAsContour(Array<T> *array, const uInt xAxis,
				      const uInt yAxis, const uInt mAxis,
				      const IPosition fixedPos) :
  LatticePADisplayData<T>(array, xAxis, yAxis, mAxis, fixedPos),
  itsMinContour(0), itsMaxContour(0) {
  constructParameters_();
  setupElements();
  setDefaultOptions();
}

// 2d array-based ctor
template <class T>
LatticeAsContour<T>::LatticeAsContour(Array<T> *array, const uInt xAxis,
				      const uInt yAxis) :
  LatticePADisplayData<T>(array, xAxis, yAxis),
  itsMinContour(0), itsMaxContour(0) {
  constructParameters_();
  setupElements();
  setDefaultOptions();
}

// >2d image-based ctor
template <class T>
LatticeAsContour<T>::LatticeAsContour(ImageInterface<T> *image,
				      const uInt xAxis, const uInt yAxis,
				      const uInt mAxis,
				      const IPosition fixedPos) :
  LatticePADisplayData<T>(image, xAxis, yAxis, mAxis, fixedPos),
  itsMinContour(0), itsMaxContour(0) {
  constructParameters_();
  setupElements();
  setDefaultOptions();
}

// 2d image-based ctor
template <class T>
LatticeAsContour<T>::LatticeAsContour(ImageInterface<T> *image,
				      const uInt xAxis, const uInt yAxis) :
  LatticePADisplayData<T>(image, xAxis, yAxis),
  itsMinContour(0), itsMaxContour(0) {
  constructParameters_();
  setupElements();
  setDefaultOptions();
}

template <class T>
LatticeAsContour<T>::~LatticeAsContour() {
  for (uInt i = 0; i < nelements(); i++) {
    delete ((LatticePADMContour<T> *)DDelement[i]);
  }
  if(itsMinContour!=0) { delete itsMinContour; itsMinContour=0;  }
  if(itsMaxContour!=0) { delete itsMaxContour; itsMaxContour=0;  }
}

// Oke, here we setup the elements using LatticePADMContour
template <class T>
void LatticeAsContour<T>::setupElements() {

  for (uInt i=0; i<nelements(); i++) if(DDelement[i]!=0) {
    delete static_cast<LatticePADMContour<T>*>(DDelement[i]);
    DDelement[i]=0;  }
				// Delete old DMs, if any.

  IPosition fixedPos = fixedPosition();
  Vector<Int> dispAxes = displayAxes();
  if (nPixelAxes > 2) {
    setNumImages(dataShape()(dispAxes(2)));
    DDelement.resize(nelements());
    for (uInt index = 0; index < nelements(); index++) {
      fixedPos(dispAxes(2)) = index;
      DDelement[index] = (LatticePADisplayMethod<T> *)new
	LatticePADMContour<T>(dispAxes(0), dispAxes(1), dispAxes(2),
			      fixedPos, this);
    }
  } else {
    setNumImages(1);
    DDelement.resize(nelements());
    DDelement[0] = (LatticePADisplayMethod<T> *)new
      LatticePADMContour<T>(dispAxes(0), dispAxes(1), this);
  }
  PrincipalAxesDD::setupElements();
}

template <class T>
void LatticeAsContour<T>::setDefaultOptions() {
  LatticePADisplayData<T>::setDefaultOptions();
  Record rec, recOut;
  rec.define("resample", "bilinear");
  LatticePADisplayData<T>::setOptions(rec, recOut);
  getMinAndMax();	// (actually, this _computes_ and sets min and max.)

  itsLevels.resize(5);  for(Int i=0; i<5; i++) itsLevels[i] = i;
   
  setStdContourLimits_();
  
  AipsrcValue<Float>::find(itsLine,"display.contour.linewidth",0.5f);  
  itsDashNeg = True;
  itsDashPos = False;
  Aipsrc::find(itsColor,"display.contour.color","foreground");  
}

template <class T>
void LatticeAsContour<T>::constructParameters_() {
  // Construct user option DisplayParameters (for min/max contour.)
  // (To be used by constructors only.)

  if(itsMinContour!=0) return;	// (already done).
  
  itsMinContour = new DParameterRange<Float>("mincontour",
                  "Base Contour Level", 
                  "Actual contour level corresping to\n"
		  "a '0' in the 'Levels' textbox.",
		  0., 1.,  .001,  0., 0.,	// (set for real below).
	          "", True, True);

  itsMaxContour = new DParameterRange<Float>("maxcontour",
                  "Maximum Contour Level",
		  "Actual contour level corresping to to a '1' in\n"
		  "the 'Levels' textbox (or to the maximum level\n"
		  "entered there, whichever is greater.)", 
		  0., 1.,  .001,  1., 1.,	// (set for real below).
	          "", True, True);  }
  

template <class T>
void LatticeAsContour<T>::setStdContourLimits_(Record* recOut) {
  // Set standard limits/values for contour sliders.  If recOut is provided,
  // they will be set onto it in a manner suitable for updating gui via
  // setOptions.
  
  // These heuristics for setting the default initial contour level bracket
  // for the sliders (mincontour, maxcontour) could be improved.  Ideally,
  // mincontour should be where the data begins to show above the noise;
  // maxcontour should be the highest level where the contour would still
  // be significantly visible (say about the 90th or 95th percentile.)
  // Both determinations could take time and/or sophisticated code.
  // The user can adjust this easily by eye afterward, but the biggest
  // problem with this simple code is that wild outliers can make the
  // range orders of magnitude too large and mess up slider resolution
  // as a result.  Clipping min-maxcontour to a reasonable percentile
  // bracket is advisable, but the computation (which involves sorting)
  // would need to just sample large datasets, for efficiency.
  
  Float dmin=getDataMin(), dmax=getDataMax();
  Float dmin2 = dmin;
  if(dmin < 0. && dmax > 2.*(-dmin)) dmin2=0.;
	// (data <0 assumed to be noise in this case)
	// (btw, in this (typical) case, a good guess
	//  for mincontour might be .5*(-dmin) ~= 1.5 or 2 * sigma...)
  Float delta = abs(dmax - dmin2);
  Float mincontour = dmin2 + .18 * delta;
  Float maxcontour = dmin2 + .9 * delta;
  
  Float res = delta*.01;
  if(res<=0.) res = .001;
  else res = pow(10., floor(log10(res)));
		// slider resolution (will be a power of ten.)
  if(res<=0.) res = .001;	// (Safety... doubtful need).
  
  if(maxcontour-mincontour > 2*res) {
    mincontour = floor(mincontour/res + .5)*res;	// rounded to
    maxcontour = floor(maxcontour/res + .5)*res;  }	// nearest res
  
  if(dmax-dmin > 2*res) {
    dmin = -floor((-dmin/res))*res;		// rounded up
    dmax =  floor(  dmax/res) *res;  }		// rounded down...
  						// (to nearest res)
  
  
  itsMinContour->setMinimum(dmin);
  itsMinContour->setMaximum(dmax);
  itsMinContour->setResolution(res);
  itsMinContour->setDefaultValue(mincontour);
  itsMinContour->setValue(mincontour);
  
  itsMaxContour->setMinimum(dmin);
  itsMaxContour->setMaximum(dmax);
  itsMaxContour->setResolution(res);
  itsMaxContour->setDefaultValue(maxcontour);
  itsMaxContour->setValue(maxcontour);
    
  if(recOut!=0) {
    Bool wholeRecord=True, overwrite=True;
    itsMinContour->toRecord(*recOut, wholeRecord, overwrite);
    itsMinContour->toRecord(*recOut, wholeRecord, overwrite);  }  }
  
  
         
template <class T>
Vector<Float> LatticeAsContour<T>::levels() {
  // Compute/return actual selected contour levels, from user's relative
  // levels and the actual min and max contour values (from the sliders.)
  
  Int nlevels = itsLevels.nelements();
  Vector<Float> abslevels(nlevels);
  Vector<Float> rellevels(itsLevels);	// Entered (relative) levels.
  
  if(nlevels>0) {
    Float min = itsMinContour->value(),		// 'slider' values.
          max = itsMaxContour->value();
    
    Float delta = max - min;
    
    Float reldelta = 1.;
    for(Int i=0; i<nlevels; i++) {
      if(reldelta<rellevels[i]) reldelta = rellevels[i];  }
		// max of textbox (relative) levels and 1.
    
    // The linear scaling: from [0, max(1., rellevels)] -> [min, max]
    
    for(Int i=0; i<nlevels; i++) {
      abslevels[i] = min + rellevels[i]*delta/reldelta;  }

    // Assure the levels are sorted, for good measure.
    
    for(Int i=0; i<nlevels; i++) {
      for(Int j=i+1; j<nlevels; j++) {
        if(abslevels[j]<abslevels[i]) { 
          Float t=abslevels[i];
	  abslevels[i]=abslevels[j]; 
	  abslevels[j]=t;  }  }  }  }
 
  return abslevels;  }


  
template <class T>
String LatticeAsContour<T>::levelString(Int prec) { 
  // Actual levels, in String format

  Vector<Float> lvls; lvls = levels();
  Int nlvls = lvls.size();
  
  if(prec<=0) {
    // Try to determine  a precision that is low enough not to
    // clutter up tracking.
    Float absmax = max(abs(itsMinContour->minimum()),
                       abs(itsMaxContour->maximum()));
    Float res = itsMaxContour->resolution();
    if(absmax==0.) absmax = 1.;	// (safety)
    if(res<=0.)    res = .001;	// (safety)
    prec = max (2, ifloor(log10(absmax/res))+1);  }
    
  ostringstream os;
  os<<setprecision(prec);
  if(nlvls>0) os<<lvls[0];
  for(Int i=1; i<nlvls; i++) os<<" "<<lvls[i];
  os<<flush;
  
  return String(os);  }
    
  
  
template <class T>
String LatticeAsContour<T>::showPosition(const Vector<Double> &wld,
                                         const Bool &abs, const Bool &dsp) {
  // Adds contour level information to the standard position tracking
  // string from PADD.
  String pos = PrincipalAxesDD::showPosition(wld, abs, dsp)+"\nContours: ";
  String lvls = levelString();
  pos += (lvls=="")? "[none]" : lvls;
    
  return pos;  }


  


template <class T>
Bool LatticeAsContour<T>::setOptions(Record &rec, Record &recOut) {
  Bool ret = LatticePADisplayData<T>::setOptions(rec, recOut);

  Bool localchange = False;
  Bool error;

  localchange = (readOptionRecord(itsLine, error, rec, "line") ||
		       localchange);
  localchange = (readOptionRecord(itsDashNeg, error, rec, "dashneg") ||
		       localchange);
  localchange = (readOptionRecord(itsDashPos, error, rec, "dashpos") ||
		       localchange);
  localchange = (readOptionRecord(itsColor, error, rec, "color") ||
		       localchange);

  Bool lvlChg=False;
  
  if (rec.isDefined("levels")) {
    DataType dtype = rec.dataType("levels");
    
    Record* valrec = &rec;
    String fldnm = "levels";
    Record subrec;
    
    if(dtype==TpRecord) {
      subrec = rec.subRecord("levels");
      if(subrec.isDefined("value")) {
        valrec = &subrec;
        dtype = subrec.dataType("value");
        fldnm = "value";  }  }	// (All this is a bit of a pain....)
        
    Vector<Float> newlevels;

    if ((dtype == TpArrayFloat) || (dtype == TpArrayDouble) ||
	(dtype == TpArrayInt) ||
	(dtype == TpFloat) || (dtype == TpDouble) || (dtype == TpInt)) {

      switch (dtype) {
      case TpFloat:
      case TpArrayFloat: {
	Vector<Float> temp;
	valrec->get(fldnm, temp);
	newlevels.resize(temp.nelements());
	for (uInt i = 0; i < newlevels.nelements(); i++) {
	  newlevels(i) = temp(i);
	}
	break; }
      case TpDouble:
      case TpArrayDouble: {
	Vector<Double> temp;
	valrec->get(fldnm, temp);
	newlevels.resize(temp.nelements());
	for (uInt i = 0; i < newlevels.nelements(); i++) {
	  newlevels(i) = temp(i);
	}
	break; }
      case TpInt:
      case TpArrayInt: {
	Vector<Int> temp;
	valrec->get(fldnm, temp);
	newlevels.resize(temp.nelements());
	for (uInt i = 0; i < newlevels.nelements(); i++) {
	  newlevels(i) = temp(i);
	}
	break; }
      default:
	// not possible!
	break;
      }
//
      Bool diff = (newlevels.nelements() != itsLevels.nelements());
      if (!diff) {
	for (uInt i = 0; i < newlevels.nelements(); i++) {
	  diff = (newlevels(i) != itsLevels(i));
	  if (diff) break;
	}
      }
      if (diff) {
	itsLevels.resize(newlevels.nelements());
	for (uInt i = 0; i < newlevels.nelements(); i++) {
	  itsLevels(i) = newlevels(i);
	}
	lvlChg = True;
      }
    }
  }

  lvlChg = itsMinContour->fromRecord(rec) || lvlChg;
  lvlChg = itsMaxContour->fromRecord(rec) || lvlChg;
  
  if(lvlChg) recOut.define("trackingchange", True);
	// 'Signals' desire to update tracking info for this DD (where
	// contour levels are also displayed).  (I wish we _could_ use true
	// (Qt) signals on this level, and not have to wait for setOpts
	// to signal stuff...)

  localchange = localchange || lvlChg;
  
  // must come last - this forces ret to be True or False:
  // (dk: no, this is probably obs. rubbish (and done wrong initially...))
  if (rec.isDefined("refresh") && (rec.dataType("refresh") == TpBool)) {
    Bool ref;
    rec.get("refresh", ref);
    localchange = localchange || ref;
  }

  ret = (ret || localchange);
  
  return ret;
}

template <class T>
Record LatticeAsContour<T>::getOptions() {
  Record rec = LatticePADisplayData<T>::getOptions();

  Record levels;
  levels.define("dlformat", "levels");
  levels.define("listname", "Contour Levels");
  levels.define("ptype", "array");
  Vector<Float> vlevels(5);
  vlevels(0) = 1.0; vlevels(1) = 2.0; vlevels(2) = 3.0;
  vlevels(3) = 4.0; vlevels(4) = 5.0;
  levels.define("default", vlevels);
  levels.define("value", itsLevels);
  levels.define("allowunset", False);
  levels.define("help", "These are relative contour levels.  They will be\n"
                        "scaled linearly, from [0, 1] (or zero to the\n"
			"maximum level entered here, if greater than one),\n"
			"onto the 'real' contour level bracket as set\n"
			"on the sliders.");
  rec.defineRecord("levels", levels);

  itsMinContour->toRecord(rec);
  itsMaxContour->toRecord(rec);

  Record line;
  line.define("dlformat", "line");
  line.define("listname", "Line width");
  line.define("ptype", "floatrange");
  line.define("pmin", Float(0.0));
  line.define("pmax", Float(5.0));
  line.define("presolution", Float(0.1));
  line.define("default", Float(0.5));
  line.define("value", itsLine);
  line.define("allowunset", False);
  rec.defineRecord("line", line);

  Record dashNeg;
  dashNeg.define("dlformat", "dashneg");
  dashNeg.define("listname", "Dash negative contours?");
  dashNeg.define("ptype", "boolean");
  dashNeg.define("default", True);
  dashNeg.define("value", itsDashNeg);
  dashNeg.define("allowunset", False);
  rec.defineRecord("dashneg", dashNeg);

  Record dashPos;
  dashPos.define("dlformat", "dashpos");
  dashPos.define("listname", "Dash positive contours?");
  dashPos.define("ptype", "boolean");
  dashPos.define("default", False);
  dashPos.define("value", itsDashPos);
  dashPos.define("allowunset", False);
  rec.defineRecord("dashpos", dashPos);

  Record color;
  color.define("dlformat", "color");
  color.define("listname", "Line color");
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

  return rec;
}

} //# NAMESPACE CASA - END

