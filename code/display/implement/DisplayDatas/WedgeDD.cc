//# WedgeDD.cc: A color wedge DisplayData
//# Copyright (C) 2001,2002
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

#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Regex.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <display/DisplayDatas/WedgeDM.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>
#include <display/Display/Attribute.h>


namespace casa { //# NAMESPACE CASA - BEGIN

WedgeDD::WedgeDD() :
  itsMin(0.0),
  itsMax(1.0),
  itsLength(512),
  itsDataUnit(""),
  itsPowerCycles(0.0),
  itsPowerScaleHandler(0),
  itsOptionsMode("vertical") {
  itsColorbar.resize(2,itsLength);
  itsPowerScaleHandler = new WCPowerScaleHandler;
  setup();
}

void WedgeDD::setup() {

  // The default for restorePCColormap_ is False.  Derived DDs such as
  // this one can set it True so that the colormap on the PixelCanvas
  // before the DD draws is restored to it afterward.  The 'colormap
  // fiddling' mouse tools can (unfortunately) only operate on the PC's 
  // current colormap; this kludge is an attempt to assure that the 'right'
  // one is left there (the one used by the last "main" DD to draw).
  
  CachingDisplayData::restorePCColormap_ = True;
 

  for (uInt l=0; l<itsLength; l++) {
    itsColorbar(0,l) = (Float)l/(Float)(itsLength-1);
    itsColorbar(1,l) = (Float)l/(Float)(itsLength-1);
  }

  uInt i = 0;
  uInt k = 1;
  if (itsOptionsMode == "horizontal") {
    i = 1;
    k = 0;
  }
  Vector<String> names(2);
  names(i) = "";
  names(k) = "";
  Vector<String> units(2);
  units(i) = "";
  units(k) = itsDataUnit;

  Matrix<Double> pc(2,2); pc= 0.0; pc.diagonal() = 1.0;
  Vector<Double> refVal(2,0.0);
  refVal(k) = itsMin;
  Vector<Double> inc(2,1.0);
  inc(k) = (itsMax-itsMin)/static_cast<Double>(itsLength);
  Vector<Double> refPix(2,0.0);
  
  LinearCoordinate lc(names, units, refVal, inc, pc, refPix);
  itsCoordinateSystem.addCoordinate(lc);
  Vector<Double> blc(2,0.0);
  Vector<Double> trc(2,1.0);
  trc(k) = static_cast<Double>(itsLength)-1.0;
  setCoordinateSystem(itsCoordinateSystem, blc, trc);
  itsAxisLabeller.setCoordinateSystem(itsCoordinateSystem);
  Record rec, recOut;
  if (itsOptionsMode == "horizontal") {
    rec.define("xgridtype","Tick Marks");
    rec.define("ygridtype","None");
    rec.define("labelposition","top-right");
  } else {
    rec.define("ygridtype","Tick Marks");
    rec.define("xgridtype","None");
    rec.define("labelposition","bottom-right");
  }
  
  rec.define("ticklength",40.0);
  itsAxisLabeller.setOptions(rec, recOut);  
  itsAxisLabeller.setAxisLabelSwitch(True);
}

void WedgeDD::updateCsys() {
  if (itsMin == itsMax) return;
  uInt i = 0;
  uInt k = 1;
  if (itsOptionsMode == "horizontal") {
    i = 1;
    k = 0;
  }
  Vector<String> units(2);
  units(i) = "";
  units(k) = itsDataUnit;
  
  const LinearCoordinate& lc = itsCoordinateSystem.linearCoordinate(0);
  LinearCoordinate lc2(lc);
  lc2.overwriteWorldAxisUnits(units);
  itsCoordinateSystem.replaceCoordinate(lc2, 0);
  Vector<Double> refVal(2,0.0);
  refVal(k) = itsMin;
  itsCoordinateSystem.setReferenceValue(refVal);
  Vector<Double> inc(2,1.0);
  inc(k) = (itsMax-itsMin)/static_cast<Double>(itsLength);
  itsCoordinateSystem.setIncrement(inc);
  Vector<Double> blc(2,0.0);
  Vector<Double> trc(2,1.0);
  trc(k) = static_cast<Double>(itsLength)-1.0;
  setCoordinateSystem(itsCoordinateSystem, blc, trc);
  Record rec, recOut;
  if (itsOptionsMode == "horizontal") {
    rec.define("xgridtype","Tick marks");   
    rec.define("ygridtype","None");
    rec.define("labelposition","top-right");
    rec.define("ticklength",20.0);
  } else {
    rec.define("xgridtype","None");
    rec.define("ygridtype","Tick marks");   
    rec.define("labelposition","bottom-right");
    rec.define("ticklength",40.0);
  }
  
  itsAxisLabeller.setOptions(rec, recOut); 
  
  itsAxisLabeller.setCoordinateSystem(itsCoordinateSystem);
  itsAxisLabeller.invalidate();
}

WedgeDD::~WedgeDD() {
  delete itsPowerScaleHandler;
  itsPowerScaleHandler = 0;
}

Bool WedgeDD::labelAxes(const WCRefreshEvent &ev) {
  if (itsMin == itsMax) {
    return False;
  }
  if (!itsAxisLabeller.axisLabelSwitch()) {
    return False;
  }
  try {
    itsAxisLabeller.draw(ev);
  } catch (const AipsError &x) {
    if (&x) { // use x to avoid compiler warning
      return False;
    }
  } 
  return True;
}

const Unit WedgeDD::dataUnit() {
  Unit unit(itsDataUnit);
  return unit;
}

String WedgeDD::showValue(const Vector<Double> &world) {
  String retval;
  // IMPLEMENT THIS
  return retval;
}

void WedgeDD::setDefaultOptions() {
  ActiveCaching2dDD::setDefaultOptions();
}

Bool WedgeDD::setOptions(Record &rec, Record &recOut) {
  Bool ret = ActiveCaching2dDD::setOptions(rec, recOut);
  Bool error = False;
  Bool localchange = False;
  Bool localchange1 = False;

  localchange = readOptionRecord(itsPowerCycles, error, rec,
				 "powercycles");
  localchange = (itsPowerScaleHandler->setOptions(rec, recOut) ||
		 localchange);

  localchange1 = (readOptionRecord(itsDataUnit, error, rec,
				   "dataunit") || localchange1);  

  localchange1 = (readOptionRecord(itsMin, error, rec,
				  "datamin") || localchange1);
  localchange1 = (readOptionRecord(itsMax, error, rec,
				   "datamax") || localchange1);  
  localchange1 = (readOptionRecord(itsOptionsMode, error, rec,
				   "orientation") || localchange1);  
  // distribute options to the axis labeller
  // remove wedge prefix
  Vector<String> fnames(5);
  fnames(0) = "axistext";      fnames(1) = "axistextcolor";
  fnames(2) = "labelcharsize"; fnames(3) = "labellinewidth";
  fnames(4) = "labelcharfont";
  
  Record rec2(rec);
  for (uInt i=0; i < fnames.nelements(); i++) {
    String str = "wedge"+fnames(i);
    if (rec.isDefined(str)) {
      if (i == 0 || i==1) {
	rec2.renameField("x"+fnames(i),str);
	rec.renameField("y"+fnames(i),str);
	rec.mergeField(rec2,"x"+fnames(i));
      } else {
	rec.renameField(fnames(i),str);
      }
    }
  }
  Record updatedOptions;
  Bool axisUpdated = itsAxisLabeller.setOptions(rec, updatedOptions);
  localchange = (localchange|| axisUpdated);

  if (localchange1) {
    updateCsys();    
    String attString("resetCoordinates");
    Attribute resetCoordinatesAtt(attString, True);
    setAttributeOnPrimaryWCHs(resetCoordinatesAtt);
  }
  localchange = (localchange|| localchange1);
  ret = (ret || localchange);
  return ret;
}

Record WedgeDD::getOptions() {
  Record rec = ActiveCaching2dDD::getOptions();
  Record datamin;
  datamin.define("value", itsMin);
  rec.defineRecord("datamin", datamin);

  Record datamax;
  datamax.define("value", itsMax);
  rec.defineRecord("datamax", datamax);

  Record ori;
  ori.define("value", itsOptionsMode);
  rec.defineRecord("orientation", ori);

  Record axisrec = itsAxisLabeller.getOptions();

  // add "wedge" prefix to make these options unique
  Vector<String> fnames(5);
  fnames(0) = "axistext";      fnames(1) = "axistextcolor";
  fnames(2) = "labelcharsize"; fnames(3) = "labellinewidth";
  fnames(4) = "labelcharfont";
  
  // severe hacking of the record, to distribute the options to both x
  // and y axis
  String field("dlformat");
  String field2("context");
  String prefix("");
  for (uInt i=0; i < fnames.nelements(); i++) {
    if (i == 0 || i==1) {
      prefix = "y";
    } else {
      prefix = "";
    }
    rec.mergeField(axisrec, prefix+fnames(i));
    Record tmp = rec.subRecord(prefix+fnames(i));
    if (i == 0 || i==1) {
      String lname;      
      tmp.get("listname",lname);
      Regex reg("[xXyY]+ ");
      lname.del(reg);
      tmp.define(String("listname"),lname);
    }
    tmp.define(field,String("wedge"+fnames(i)));
    tmp.define(field2,String("Color_Wedge"));
    rec.defineRecord(prefix+fnames(i),tmp);
    rec.renameField("wedge"+fnames(i),prefix+fnames(i));   
  }
  return rec;
}

CachingDisplayMethod *WedgeDD::newDisplayMethod(
    WorldCanvas *worldCanvas,
    AttributeBuffer *wchAttributes,
    AttributeBuffer *ddAttributes,
    CachingDisplayData *dd) {
  return new WedgeDM(worldCanvas, wchAttributes, ddAttributes, dd);
}

AttributeBuffer WedgeDD::optionsAsAttributes() {
  AttributeBuffer buffer = ActiveCaching2dDD::optionsAsAttributes();
  buffer.set("dataunit",itsDataUnit);
  buffer.set("datamin",itsMin);
  buffer.set("datamax",itsMax);
  buffer.set("powercycles",itsPowerCycles);
  buffer.set("orientation",itsOptionsMode);
  return buffer;
}

void WedgeDD::notifyUnregister(WorldCanvasHolder &wcHolder,
				       Bool ignoreRefresh) {
  // disconnected from WC[H]--be sure drawlists it may be
  // holding for us are deleted.
  itsAxisLabeller.invalidate();
  ActiveCaching2dDD::notifyUnregister(wcHolder, ignoreRefresh);
}

WedgeDD::WedgeDD(const WedgeDD &) {
  // MUST IMPLEMENT
}

void WedgeDD::operator=(const WedgeDD &) {
  // MUST IMPLEMENT
}

} //# NAMESPACE CASA - END

