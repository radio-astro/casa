//# WCCSAxisLabeller.cc: labelling axes using a CoordinateSystem on a WC
//# Copyright (C) 1999,2000,2001,2002,2004
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
#include <casa/Exceptions.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasTable.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/iostream.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Regex.h>
#include <display/Display/WorldCanvas.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

 
//# display library includes:

//# this include:
#include <display/DisplayCanvas/WCCSAxisLabeller.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WCCSAxisLabeller::WCCSAxisLabeller() :
  useWCCS(False), 
  itsAbsolute(True),
  itsWorldAxisLabels(True),
  itsDoVelocity(True),
  itsZIndex(-1),
  itsHasCoordinateSystem(False),
  itsSpectralUnit("km/s"),
  itsDoppler("radio"),
  itsDirectionUnit("arcsec"),
  itsDirectionSystem("J2000"),
  itsFrequencySystem("LSRK"),
  itsZLabelType("none"),
  uiBase_(1) {
  String worldorpix;
  Aipsrc::find(worldorpix,"display.axislabels.world","on");
  itsWorldAxisLabels = 
    !worldorpix.matches(Regex("[ \t]*(([nN]o)|([oO]ff)|([fF](alse)*))[ \t\n]*"));
    
}

WCCSAxisLabeller::~WCCSAxisLabeller() {
}

void WCCSAxisLabeller::setCoordinateSystem(const CoordinateSystem& cSys) {
  itsCoordinateSystem = cSys;
  itsHasCoordinateSystem = True;
}


void WCCSAxisLabeller::setDefaultOptions() {
  WCAxisLabeller::setDefaultOptions();
//
  itsAbsolute = True;
  //itsWorldAxisLabels = True;
//
  Int after = -1;
  Int iS = itsCoordinateSystem.findCoordinate(Coordinate::SPECTRAL, after);
  if (iS>=0) {
     const SpectralCoordinate coord = itsCoordinateSystem.spectralCoordinate(iS);
     Double restFreq = coord.restFrequency(); 
     Vector<String> vunits;
     if (restFreq > 0) {
        itsSpectralUnit = String("km/s");
     } else {
        itsSpectralUnit = String("GHz");
     }
//
     MFrequency::Types ctype;
     MEpoch epoch;
     MPosition position;
     MDirection direction;
     coord.getReferenceConversion(ctype, epoch, position, direction);
     itsFrequencySystem = MFrequency::showType(ctype);
     itsDoppler = String("radio");
     setSpectralState();
  }
//
  after = -1;
  iS = itsCoordinateSystem.findCoordinate(Coordinate::DIRECTION, after);
  if (iS>=0) {
     const DirectionCoordinate coord = itsCoordinateSystem.directionCoordinate(iS);
     MDirection::Types ctype;
     coord.getReferenceConversion(ctype);
     itsDirectionSystem = MDirection::showType(ctype);
     itsDirectionUnit = String("arcsec");
     setDirectionState();
  }
//
  setAbsRelState();
  itsZLabelType = String("none");
}



Bool WCCSAxisLabeller::setOptions(const Record &rec, Record &updatedOptions) 
{
  Bool ret = WCAxisLabeller::setOptions(rec, updatedOptions);
  Bool localchange = False;
  Bool error;
  
  Bool sChg = readOptionRecord(itsDoppler, error, rec,
                               "axislabelvelocitytype");
       sChg = readOptionRecord(itsSpectralUnit, error, rec,
                               "axislabelspectralunit")      || sChg;
       sChg = readOptionRecord(itsFrequencySystem, error, rec,
                               "axislabelfrequencysystem")   || sChg;
  if(sChg) {
    setSpectralState();
    localchange = True;  } 
  
  
  Bool dChg = readOptionRecord(itsDirectionUnit, error, rec,
                               "axislabeldirectionunit");
  
  String dsSave = itsDirectionSystem;
  MDirection::Types ds;
  if(         readOptionRecord(itsDirectionSystem, error, rec,
                               "axislabeldirectionsystem") ) {
    if(MDirection::getType(ds, itsDirectionSystem)) dChg = True;    // (valid)
    else itsDirectionSystem = dsSave;  }		// (invalid -- revert)
  
  if(dChg) {
    setDirectionState();
    localchange = True;  }
    

  String value;
  if (readOptionRecord(value, error, rec,  "axislabelabsrel")) {
     Bool abs = (value=="absolute");
     if (abs != itsAbsolute) {
        itsAbsolute = abs;
        setAbsRelState();
        localchange = True;
     }
  }
//
  if (readOptionRecord(value, error, rec,  "axislabelpixelworld")) {
     Bool world = (value=="world");
     if (world != itsWorldAxisLabels) {
        itsWorldAxisLabels = world;
        localchange = True;
     }
  }
//
  if (readOptionRecord(itsZLabelType, error, rec, "axislabelzlabeltype")) {
     localchange = True;
  } 
//  
  if (localchange) {
    // invalidate existing draw lists, etc.
    invalidate();
  }
//
  ret = (ret || localchange);
  return ret;
}

Record WCCSAxisLabeller::getOptions() const 
{
  Record rec = WCAxisLabeller::getOptions();
//

  Record pixworld;
  pixworld.define("context", "Axis_label_properties");
  pixworld.define("dlformat", "axislabelpixelworld");
  pixworld.define("listname", "World or pixel coordinates");
  pixworld.define("ptype", "choice");
  Vector<String> v(2);
  v(0) = "world";
  v(1) = "pixel";
  pixworld.define("popt", v);
  pixworld.define("default", "world");
  if (itsWorldAxisLabels) {
     pixworld.define("value", "world");
  } else {
     pixworld.define("value", "pixel");
  }
  pixworld.define("allowunset", False);
  rec.defineRecord("axislabelpixelworld", pixworld);
//
  Record absrel;
  absrel.define("context", "Axis_label_properties");
  absrel.define("dlformat", "axislabelabsrel");
  absrel.define("listname", "Absolute or relative");
  absrel.define("ptype", "choice");
  v.resize(2);
  v(0) = "absolute";
  v(1) = "relative";
  absrel.define("popt", v);
  absrel.define("default", "absolute");
  if (itsAbsolute) {
     absrel.define("value", "absolute");
  } else {
     absrel.define("value", "relative");
  }  
  absrel.define("allowunset", False);
  rec.defineRecord("axislabelabsrel", absrel);
//
  Int after = -1;
  Int iD = itsCoordinateSystem.findCoordinate(Coordinate::DIRECTION, after);
  if (iD>=0) {
     {
        Record directionSystem;
        directionSystem.define("context", "Axis_label_properties");
        directionSystem.define("dlformat", "axislabeldirectionsystem");
        directionSystem.define("listname", "Direction Reference");
        directionSystem.define("ptype", "choice");
        Vector<String> vunits(5);
        vunits(0) = "J2000";
        vunits(1) = "B1950";
        vunits(2) = "GALACTIC";
        vunits(3) = "ECLIPTIC";
        vunits(4) = "SUPERGAL";
	
	// There are other possibilities for Direction Reference than those
	// above (unsure why they were not all included).  Assure at least
	// that the CS's native reference is included....
	Bool found=False;
	for(Int i=0; i<5; i++) if(itsDirectionSystem==vunits(i)) {
	  found=True; break;  }
	if(!found) {
	  vunits.resize(6, True);
	  vunits(5) = itsDirectionSystem;  }
	
        directionSystem.define("popt", vunits);
        directionSystem.define("default", itsDirectionSystem);
        directionSystem.define("value", itsDirectionSystem);
        directionSystem.define("allowunset", False);
        rec.defineRecord("axislabeldirectionsystem", directionSystem);
     }
//
     {
        Record directionunit;
        directionunit.define("context", "Axis_label_properties");
        directionunit.define("dlformat", "axislabeldirectionunit");
        directionunit.define("listname", "Direction unit");
        directionunit.define("ptype", "userchoice");
        Vector<String> vunits(4);
        vunits(0) = "deg";
        vunits(1) = "arcmin";
        vunits(2) = "arcsec";
        vunits(3) = "rad";
        directionunit.define("popt", vunits);
        directionunit.define("default", "arcsec");
        directionunit.define("value", itsDirectionUnit);
        directionunit.define("allowunset", False);
        rec.defineRecord("axislabeldirectionunit", directionunit);
     }
  }
//
  after = -1;
  Int iS = itsCoordinateSystem.findCoordinate(Coordinate::SPECTRAL, after);
  if (iS>=0) {
     {
        Record frequencySystem;
        frequencySystem.define("context", "Axis_label_properties");
        frequencySystem.define("dlformat", "axislabelfrequencysystem");
        frequencySystem.define("listname", "Spectral Reference");
        frequencySystem.define("ptype", "choice");
        Vector<String> vunits(5);
        vunits(0) = "LSRK";
        vunits(1) = "LSRD";
        vunits(2) = "BARY";
        vunits(3) = "GEO";
        vunits(4) = "TOPO";
        frequencySystem.define("popt", vunits);
        frequencySystem.define("default", itsFrequencySystem);
        frequencySystem.define("value", itsFrequencySystem);
        frequencySystem.define("allowunset", False);
        rec.defineRecord("axislabelfrequencysystem", frequencySystem);
     }
     {    
        Record spectralunit;
        spectralunit.define("context", "Axis_label_properties");
        spectralunit.define("dlformat", "axislabelspectralunit");
        spectralunit.define("listname", "Spectral unit");
        spectralunit.define("ptype", "userchoice");
//
        const SpectralCoordinate sc = itsCoordinateSystem.spectralCoordinate(iS);
        Double restFreq = sc.restFrequency(); 
        Vector<String> vunits;
        if (restFreq > 0) {
           vunits.resize(5);        
           vunits(0) = "km/s";
           vunits(1) = "m/s";
           vunits(2) = "GHz";
           vunits(3) = "MHz";
           vunits(4) = "Hz";
        } else {
           vunits.resize(3);        
           vunits(0) = "GHz";
           vunits(1) = "MHz";
           vunits(2) = "Hz";
        }
//
        spectralunit.define("popt", vunits);
        spectralunit.define("default", vunits(0));
        spectralunit.define("value", itsSpectralUnit);
        spectralunit.define("allowunset", False);
        rec.defineRecord("axislabelspectralunit", spectralunit);
     }
     {
        Record veltype;
        veltype.define("context", "Axis_label_properties");
        veltype.define("dlformat", "axislabelvelocitytype");
        veltype.define("listname", "Velocity type");
        veltype.define("ptype", "choice");
        Vector<String> vunits(3);
        vunits(0) = "optical";
        vunits(1) = "radio";
        vunits(2) = "true";
        veltype.define("popt", vunits);
        veltype.define("default", "radio");
        veltype.define("value", itsDoppler);
        veltype.define("allowunset", False);
        rec.defineRecord("axislabelvelocitytype", veltype);
     }
  }
  if (itsCoordinateSystem.nWorldAxes() > 2) {
    
    Record zlabeltype;
    zlabeltype.define("context", "Axis_label_properties");
    zlabeltype.define("dlformat", "axislabelzlabeltype");
    zlabeltype.define("listname", "Movie Axis label type");
    zlabeltype.define("ptype", "choice");
    Vector<String> vztype(3);
    vztype(0) = "none";
    vztype(1) = "world";
    vztype(2) = "pixel";
    zlabeltype.define("popt", vztype);
    zlabeltype.define("default", vztype(0));
    zlabeltype.define("value", itsZLabelType);
    zlabeltype.define("allowunset", False);
    rec.defineRecord("axislabelzlabeltype", zlabeltype);

  }
  
  return rec;
}

String WCCSAxisLabeller::xAxisText(WorldCanvas* wc) const {
//
// The CS is ordered so that the first 2 axes
// are the display axes
//

  // if (!isXAxisTextUnset()) {
  if (WCAxisLabeller::xAxisText() != "") {
	// Semi-kludge to avoid having to support "unset" in casaviewer.
	// dk 2/07.  Default axisText (e.g. "J2000 Right Ascension") will be
	// returned in place of "" (but not " "), as well in "unset" case.
    
    return WCAxisLabeller::xAxisText();
  }

  return axisText(0, wc);
}


String WCCSAxisLabeller::yAxisText(WorldCanvas* wc) const 
//
// The CS is ordered so that the first 2 axes
// are the display axes
//
{
  // if (!isYAxisTextUnset()) {
  if (WCAxisLabeller::yAxisText() != "") {
    return WCAxisLabeller::yAxisText();
  }
//
  return axisText(1, wc);
}


String WCCSAxisLabeller::axisText(Int worldAxis, WorldCanvas* wc) const 
//
// The CS is ordered so that the first 2 axes
// are the display axes
//
{

  CoordinateSystem cs;
   
  if(useWCCS && wc!=0 && wc->hasCS()) {
    cs = wc->coordinateSystem();
    setSpectralState(cs);
    setDirectionState(cs);  } 
  else cs = itsCoordinateSystem;
  
  Vector<String> nms = cs.worldAxisNames(), uns = cs.worldAxisUnits();
  if( worldAxis<0 || worldAxis>=Int(nms.nelements()) ||
                     worldAxis>=Int(uns.nelements()) ) return "";
  String base0    = nms(worldAxis);
  String prefUnit = uns(worldAxis);
  
  if (!itsWorldAxisLabels) prefUnit = String("pixels");
  
  String unitString = " (" + prefUnit + ")";
  if (prefUnit=="" || prefUnit==" " || prefUnit=="_") unitString = " ";
//
  Int coordinate, axisInCoordinate;
  cs.findWorldAxis(coordinate, axisInCoordinate, worldAxis);
  Coordinate::Type ctype = cs.type(coordinate);
  String base;
  const Vector<Int>& worldAxes = cs.worldAxes(coordinate);
//
  if (ctype == Coordinate::DIRECTION) {
     const DirectionCoordinate& dcoord = 
       cs.directionCoordinate(coordinate);
//
     MDirection::Types dtype = dcoord.directionType();
     MDirection::Types ctype;
     dcoord.getReferenceConversion(ctype);
//
     Bool isLong = True;
     if (worldAxes(1)==worldAxis) isLong = False;

// Depending on the requested labelling type, we convert
// the axis unit name to something sensible. This is
// because it's confusing to see Galactic coordinates
// called 'Right Ascension' say.

     uInt ctypeI = static_cast<uInt>(ctype);
     MDirection::GlobalTypes gType = MDirection::globalType(ctypeI);
     if (dtype != ctype) {
        if (gType==MDirection::GRADEC) {
           if (isLong) {
              base0 = "Right Ascension";
           } else {
              base0 = "Declination";
           }
        } else if (gType==MDirection::GHADEC) {
           if (isLong) {
              base0 = "Hour Angle";
           } else {
              base0 = "Declination";
           }
        } else if (gType==MDirection::GAZEL) {
           if (isLong) {
              base0 = "Azimuth";
           } else {
              base0 = "Elevation";
           }
        } else if (gType==MDirection::GLONGLAT) {
           if (isLong) {
              base0 = "Longitude";
           } else {
              base0 = "Latitude";
           }
        }
     }
//
     String stype = MDirection::showType(ctype);
     if (itsAbsolute) {
        if (itsWorldAxisLabels) {
           base = stype + String(" ") + base0;
        } else {
           base = stype + String(" ") + base0 + unitString;
        }
     } else {
        base = String("Relative ") + stype + String(" ") + base0 + unitString;
     }
  } else if (ctype == Coordinate::SPECTRAL) {
    const SpectralCoordinate& scoord = cs.spectralCoordinate(coordinate);
//
    MFrequency::Types ctype;
    MEpoch epoch;
    MPosition position;
    MDirection direction;
    scoord.getReferenceConversion(ctype, epoch, position, direction);
    String freqType = MFrequency::showType(ctype);
//
    if (itsWorldAxisLabels) {

// We must avoid making a unit from the String 'pixels'

       Unit pVU(itsSpectralUnit);
       Unit HZ("Hz");
       Unit KMS("km/s");
       if (pVU==KMS) {
          prefUnit = itsSpectralUnit;
          unitString = " (" + prefUnit + ")";
          if (prefUnit=="" || prefUnit==" " || prefUnit=="_") unitString = " ";
          base = freqType + " " + itsDoppler + " velocity" + unitString;
       } else if (pVU==HZ) {
          base = freqType + String(" ") + base0 + unitString;
       } else {
          base = freqType + String(" ") + base0 + unitString; 
       }
    } else {
       base = freqType + String(" ") + base0 + unitString; 
    }
//
    if (!itsAbsolute) {
      base = String("Relative ") + base;
    }
  } else if (ctype==Coordinate::STOKES) {
     base = base0;
     if (itsWorldAxisLabels) {
        if (!itsAbsolute) base = String("Relative ") + base;
     } else {
        if (!itsAbsolute) base = String("Relative ") + base + unitString;
     }
  } else {
    base = base0 + unitString;
    if (!itsAbsolute) base = String("Relative ") + base;
  }
  return base;
}





void WCCSAxisLabeller::setSpectralState (CoordinateSystem& cs) const
{
   static LogIO os(LogOrigin("WCCSAxisLabeller", "setSpectralState", WHERE));

// Set velocity and/or world unit state in SpectralCoordinate

   String errorMsg;
   if (!CoordinateUtil::setSpectralState (errorMsg, cs,
                                          itsSpectralUnit, itsDoppler)) {
      os << errorMsg << LogIO::EXCEPTION;
   }

// Set Spectral Conversion Layer

   if (!CoordinateUtil::setSpectralConversion (errorMsg, cs,
                                               itsFrequencySystem)) {
      os << errorMsg << LogIO::EXCEPTION;
   }

// Set Spectral formatting (for movie axis labelling done via formatter)

   if (!CoordinateUtil::setSpectralFormatting (errorMsg, cs,
                                               itsSpectralUnit, itsDoppler)) {
      os << errorMsg << LogIO::EXCEPTION;
   }
   
// Indicate whether we are asking for kms or Hz conformant spectral units

   static Unit KMS(String("km/s"));
   Unit t(itsSpectralUnit);
   itsDoVelocity = (t==KMS);
}


void WCCSAxisLabeller::setDirectionState (CoordinateSystem& cs) const
{
   static  LogIO os(LogOrigin("WCCSAxisLabeller", "setDirectionState", WHERE));

// The user given units are only used for relative labels

   Int after = -1;
   Int iD = cs.findCoordinate(Coordinate::DIRECTION, after);
   if (iD>=0) {
      DirectionCoordinate coord(cs.directionCoordinate(iD));
      Vector<String> worldAxisUnits(coord.worldAxisUnits().copy());
//
      if (itsAbsolute) {
         worldAxisUnits = String("deg");
      } else {
         worldAxisUnits = itsDirectionUnit;
      }
//
      if (!coord.setWorldAxisUnits(worldAxisUnits)) {
         os << LogIO::SEVERE << coord.errorMessage() << LogIO::POST;
         return;
      }

// Set conversion type.  This lets the DC convert to other direction systems

      MDirection::Types cSystem;
      MDirection::getType(cSystem, itsDirectionSystem);
      coord.setReferenceConversion(cSystem);

// Replace in CS

      cs.replaceCoordinate(coord, iD);
   }
}


void WCCSAxisLabeller::setAbsRelState ()
//
// For absolute coordinates
//   Direction  - degrees
//   Spectral   - GHz
//   Linear     - native
// For relative coordinates
//   Direction  
//     RA/DEC  - arcsec
//     LON/LAT - degrees
//   Spectral   - GHz
//   Linear     - native
// 
// The Spectral unit is under user control (e.g. km/s) so we don't
// overwrite what they have provided. At some point the
// other coordinates will be under user control as well,
// but for now above is what they get
//
{
   const uInt n = itsCoordinateSystem.nCoordinates();
   
   for (uInt i=0; i<n; i++) {
      if (itsCoordinateSystem.type(i)==Coordinate::DIRECTION) {
         setDirectionState();
      } else if (itsCoordinateSystem.type(i)==Coordinate::SPECTRAL) {
         setSpectralState();
      } else {
// Leave as native world axis units
      }
   }
}

} //# NAMESPACE CASA - END

