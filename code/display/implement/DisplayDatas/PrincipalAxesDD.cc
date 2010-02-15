//# PrincipalAxesDD.cc: Base class for axis-bound DisplayData objects
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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

#include <cctype>
#include <string>
#include <algorithm>
#include <casa/stdio.h>
#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>    
#include <display/DisplayEvents/WCPositionEvent.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <display/DisplayEvents/WCRefreshEvent.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/AttributeBuffer.h>
#include <display/DisplayDatas/PrincipalAxesDM.h>
#include <display/DisplayCanvas/WCCSNLAxisLabeller.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>



namespace casa { //# NAMESPACE CASA - BEGIN

// constructor
PrincipalAxesDD::PrincipalAxesDD(uInt xAxis, uInt yAxis, 
				 Int mAxis, Bool axisLabels) 
: iAmRubbish(True),
  itsNotation(Coordinate::MIXED),
  itsNumImages(0),
  itsSpectralUnit("km/s"),
  itsDoppler("radio"),
  itsAbsolute(True),
  itsFractionalPixels(False),
  itsUsesAxisLabels(axisLabels)
{
  itsDisplayAxes.resize(3);
  itsDisplayAxes(0) = xAxis;
  itsDisplayAxes(1) = yAxis;
  itsDisplayAxes(2) = mAxis;
}

// default destructor
PrincipalAxesDD::~PrincipalAxesDD() 
{
  if ( itsUsesAxisLabels ) {
    for (uInt i = 0; i < itsNumImages; i++) {
      delete ((WCCSNLAxisLabeller *)itsAxisLabellers[i]);
    }
  }
};

// Convert a 2d WorldCanvas linear (data pixel) coordinate to a 
// world coordinate.  Emit a 2d world coordinate if there is 
// no dependency on a non-displayed axis, otherwise emit a 3d
// world coordinate.
Bool PrincipalAxesDD::linToWorld(Vector<Double> &world,
				 const Vector<Double> &lin) 
{  
// just use linToFullWorld and prune the result

  if (linToFullWorld(itsFullWorldTmp1, lin)) {
    String attString("hiddenDependentWorldAxis");
    if (existsAttribute(attString)) {
      Int hiddenDependentWorldAxis;
      getAttributeValue(attString, hiddenDependentWorldAxis);
      if (hiddenDependentWorldAxis > -1) {
	world.resize(3);
	world(2) = itsFullWorldTmp1(hiddenDependentWorldAxis);

// record this most recently calculated hidden coord world value

	attString = "hiddenDependentAxisWorldValue";
	Attribute att(attString, world(2));
	setAttribute(att);
      }
    } else {
       world.resize(2);
    }
//
    world(0) = itsFullWorldTmp1(0);
    world(1) = itsFullWorldTmp1(1);
    return True;
  } else {
    return False;
  }
}

// Convert a 2d WorldCanvas linear (data pixel) coordinate to a 
// complete world coordinate.
Bool PrincipalAxesDD::linToFullWorld(Vector<Double> &fullWorld,
				     const Vector<Double> &lin) 
{
// some sanity checks

  if (iAmRubbish) {
    return False;
  }

// the input must be a 2d WorldCanvas linear (data pixel) coordinate,
// and the output will be a full world coordinate on the WorldCanvas.

  if (lin.nelements() != 2) {
    return False;
  }

// The first 3 pixel axes of itsCoordSys have been transposed
// to be the first 3 display axes.  The others are in any old order.

  if (itsPixelInTmp2.nelements()!=nPixelAxes) itsPixelInTmp2.resize(nPixelAxes);
  itsPixelInTmp2(0) = lin(0);
  itsPixelInTmp2(1) = lin(1);
  if (nPixelAxes > 2) {
    itsPixelInTmp2(2) = activeZIndex_;
  }
  for (uInt i = 3; i<itsAddPixPos.nelements(); i++) {
    itsPixelInTmp2(i) = itsAddPixPos(i);
  }
//
  return itsCoordSys.toWorld(fullWorld, itsPixelInTmp2);
}

// Convert a 2d or 3d world coordinate to a 2d WorldCanvas linear (data pixel)
// coordinate.  3d is only allowed if there is a hidden coordinate, and 
// it is desirable to specify it.
Bool PrincipalAxesDD::worldToLin(Vector<Double> &lin,
				 const Vector<Double> &world) 
{
  
// sanity checks

  if (iAmRubbish) {
    return False;
  }

// the input must be a 2d or 3d world coordinate on the WorldCanvas, and the 
// output will be a 2d linear coordinate on the WorldCanvas.

  uInt nWorld = world.nelements();
  if (nWorld < 2 || nWorld> 3) return False;

//
  if (itsWorldInTmp3.nelements() != nWorldAxes) {
    itsWorldInTmp3.resize(nWorldAxes);
    itsWorldAxesTmp3.resize(nWorldAxes);
  }
  if (itsPixelInTmp3.nelements() != nPixelAxes) {
    itsPixelInTmp3.resize(nPixelAxes);
    itsPixelAxesTmp3.resize(nPixelAxes);
  }
//
  itsWorldAxesTmp3 = False;
  itsPixelAxesTmp3 = True;

// fill in what we know in world coords

  itsWorldInTmp3(0) = world(0);
  itsWorldInTmp3(1) = world(1);
  itsWorldAxesTmp3(0) = itsWorldAxesTmp3(1) = True;
  
// fill in what we know in pixel coords

  itsPixelAxesTmp3(0) = itsPixelAxesTmp3(1) = False;
  if (nPixelAxes > 2) {
    itsPixelInTmp3(2) = activeZIndex_;
  }
  for (uInt i = 3; i < itsAddPixPos.nelements(); i++) {
    itsPixelInTmp3(i) = itsAddPixPos(i);
  }
//
  String attString("hiddenDependentWorldAxis");
  if (existsAttribute(attString)) {
    Int hiddenDependentWorldAxis;
    getAttributeValue(attString, hiddenDependentWorldAxis);
    if (hiddenDependentWorldAxis > -1) {
      Int pAxis = itsCoordSys.worldAxisToPixelAxis(hiddenDependentWorldAxis);
      if (nWorld==3) {
	itsWorldInTmp3(hiddenDependentWorldAxis) = world(2);
	itsWorldAxesTmp3(hiddenDependentWorldAxis) = True;
        if (pAxis >= 0) {
  	   itsPixelAxesTmp3(pAxis) = False;
        }
      } else {
	attString = "hiddenDependentAxisWorldValue";
	if (existsAttribute(attString)) {
	  Double attValue;
	  getAttributeValue(attString, attValue);
	  itsWorldInTmp3(hiddenDependentWorldAxis) = attValue;
	  itsWorldAxesTmp3(hiddenDependentWorldAxis) = True;
          if (pAxis >= 0) {
   	     itsPixelAxesTmp3(pAxis) = False;
          }
	}
      }
    }
  }

// do conversion. return first two pixel coords - these are 
// assumed to have been reordered correctly in sync with the world axes.

  if (itsCoordSys.toMix(itsWorldOutTmp3, itsPixelOutTmp3, 
                                itsWorldInTmp3, itsPixelInTmp3,
				itsWorldAxesTmp3, itsPixelAxesTmp3, 
                                itsWorldMin, itsWorldMax)) {
    lin.resize(2);
    lin(0) = itsPixelOutTmp3(0);
    lin(1) = itsPixelOutTmp3(1);
    return True;
  } else {
    return False;
  }
}

String PrincipalAxesDD::showPosition(const Vector<Double> &world,
				     const Bool &displayAxesOnly) 
{
  return showPosition(world, itsAbsolute, displayAxesOnly);
}

// <drs> - called to get state information for cursor tracking
String PrincipalAxesDD::showPosition(const Vector<Double> &world,
				     const Bool &showAbs,
				     const Bool &displayAxesOnly) 
{
// The returned full coordinates are in 'normal' transpose
// order suitable for itsPosTrackCoordSys

  if (!conformed() || 
      !getFullCoord(itsFullWorldTmp4, itsFullPixelTmp4, world)) {
    return "\n";
	// Return two lines in these cases.  This will happen
	// if the DD fails the conformsTo() check, which should be
	// performed prior to calling this routine.  (showValue() and
	// showPosition should really be modified to pass in the WCH
	// and do the conformsTo() check themselves instead, since the
	// methods' results are a function of the wch's zIndex as well...).
  }

  String retval;
  String units("");
  Int j;
  uInt n = itsFullWorldTmp4.nelements();
  if (displayAxesOnly) n = 2;

  // Append pixel coordinates on first line
  retval += "Pixel: ";
  
  Int offset = uiBase();
  // 'Absolute pixel coordinates' will be numbered for
  // the user from uiBase(), which is either 0 or 1.
  // (Internally they are always numbered from 0).
  if (!showAbs) {
     itsPosTrackCoordSys.makePixelRelative(itsFullPixelTmp4);
     offset = 0;
  }
  for (uInt i=0; i<n; i++) {
     if (displayAxesOnly) {
        j = itsDisplayAxes(i);
     } else {
        j = i;
     }

     ostringstream oss;
     if (itsFractionalPixels) {
        oss << itsFullPixelTmp4(j)+offset;
     } else {
        oss << Int(itsFullPixelTmp4(j)+0.5+offset);
     }
     
     if(i < n - 1) oss << " ";

     retval += String(oss);
  }
  
  // Append world coordinates on second line
  retval += "\n";
  
  for (uInt i=0; i<n; i++) {
     if (displayAxesOnly) {
        j = itsDisplayAxes(i);
     } else {
        j = i;
     }

     // The world vector comes from itsCoordSys.  We should really
     // use the formatQuantity interface here if we want to be able to handle
     // any potential divergence of the world axis units in itsCoordSys
     // and itsPosTrackCoordSys.  This just makes it a little slower but
     // its acceptable. 

     units = String("");

     // if it is a spectral value, we look at what notation the
     // user wants (scientific or fixed)
     int coordNum, axisInCoord;
     itsPosTrackCoordSys.findWorldAxis(coordNum, axisInCoord, j);     
     
     Coordinate::formatType notation = itsNotation;
     if (showAbs &&
         itsPosTrackCoordSys.type(coordNum) == Coordinate::DIRECTION) {    
         notation = Coordinate::DEFAULT;
     }
     
     
     String fmtdCoord = itsPosTrackCoordSys.format(units, notation, 
                                                   itsFullWorldTmp4[j], j, 
                                                   True, showAbs);

     if ((units != String("")) && (units != String(" "))) {
         fmtdCoord += String(" ") + units;
     }
     
     retval += fmtdCoord;
     if(i < n - 1) retval += "  ";

     if (itsPosTrackCoordSys.type(coordNum) == Coordinate::SPECTRAL) {
	SpectralCoordinate spec_coord = itsPosTrackCoordSys.spectralCoordinate(coordNum);
	MFrequency::Types freq_type = spec_coord.frequencySystem(true);		// effective type (passing false would return the native type)
	String frequency = MFrequency::showType(freq_type);
	std::transform(frequency.begin(), frequency.end(), frequency.begin(), tolower);
	MDoppler::Types velocity_type = spec_coord.velocityDoppler( );
	String velocity = MDoppler::showType(velocity_type);
	std::transform(velocity.begin(), velocity.end(), velocity.begin(), tolower);
	retval += " (" + frequency + "/" + velocity + ")";
     }

  }
  
  // Old world or pixel code:
  /*
  if (showWorld) {
     Bool firstline=True;
     for (uInt i=0; i<n; i++) {
        if (displayAxesOnly) {
           j = itsDisplayAxes(i);
        } else {
           j = i;
        }

// The world vector comes from itsCoordSys.  We should really
// use the formatQuantity interface here if we want to be able to handle
// any potential divergence of the world axis units in itsCoordSys
// and itsPosTrackCoordSys.  This just makes it a little slower but
// its acceptable. 

        units = String("");

	// if it is a spectral value, we look at what notation the
	// user wants (scientific or fixed)
 	Coordinate::formatType format = Coordinate::FIXED;
	int coordNum, axisInCoord;
	itsPosTrackCoordSys.findWorldAxis(coordNum, axisInCoord, j);
	if (itsPosTrackCoordSys.type(coordNum) == Coordinate::SPECTRAL) {
	  if (itsSpectralNotationFixed) {
	    format = Coordinate::FIXED;
	  } else {
	    format = Coordinate::SCIENTIFIC;
	  }
	} else {
	  format = Coordinate::DEFAULT;
	}

	String fmtdCoord = itsPosTrackCoordSys.format(units,
				      format, 
                               	      itsFullWorldTmp4[j], j, 
                                      True, showAbs);
//
        if ((units != String("")) && (units != String(" "))) {
            fmtdCoord += String(" ") + units;
        }

	if (i > 0) {
	   if(firstline &&
	      (i==n/2 || retval.length() + fmtdCoord.length() >= 36)) {
	      retval += String("\n");	// use second line--avoid overruns
	      firstline = False;	// which cause irritating resizes.
	   } else {
	      retval += String(" ");
	   }
        }

	retval += fmtdCoord;
     }
  } else {
     Int offset = uiBase();
	// 'Absolute pixel coordinates' will be numbered for
	// the user from uiBase(), which is either 0 or 1.
	// (Internally they are always numbered from 0).
     if (!showAbs) {
        itsPosTrackCoordSys.makePixelRelative(itsFullPixelTmp4);
        offset = 0;
     }
     for (uInt i=0; i<n; i++) {
        if (displayAxesOnly) {
           j = itsDisplayAxes(i);
        } else {
           j = i;
        }
        ostringstream oss;
        if (itsFractionalPixels) {
           oss << itsFullPixelTmp4(j)+offset;
        } else {
           oss << Int(itsFullPixelTmp4(j)+0.5+offset);
        }
        
	if(i+1==n/2) oss << endl;	// Put the second half of the
	else         oss << " ";	// output on a second line.

	retval += String(oss);
     }
     retval += " pixels";
  }
  */
  
  return retval;
}

Bool PrincipalAxesDD::setActiveZIndex_(Int zindex) {
  if (nelements()==1) {			// (If there is only one frame on
    activeZIndex_ = 0;			// the animation axis, use that frame
    zIndexConformed_ =  True;  }	// regardless of zindex passed in).
  else {
    activeZIndex_ = zindex;
    zIndexConformed_  =  zindex>=0 && zindex<Int(nelements());  }
  
  // Keep itsFixedPosition(zAxis) in sync with latest zindex.
  
  if( itsDisplayAxes.nelements()   > 2u &&
      itsFixedPosition.nelements() > uInt(itsDisplayAxes[2]) ) {
    itsFixedPosition[itsDisplayAxes[2]] = zIndexConformed_? 
 					  activeZIndex_ : 0;  }
  
  return zIndexConformed_;  }

  
Vector<String> PrincipalAxesDD::worldAxisNames() 
{
  Vector<String> tmpVec(2);

// return only displayed axis names; user can
// obtain others from data itself.

  tmpVec(0) = itsCoordSys.worldAxisNames()(0);
  tmpVec(1) = itsCoordSys.worldAxisNames()(1);
  
  return tmpVec;
}

Vector<String> PrincipalAxesDD::worldAxisUnits() 
{
  Vector<String> tmpVec(2);
  tmpVec(0) = itsCoordSys.worldAxisUnits()(0);
  tmpVec(1) = itsCoordSys.worldAxisUnits()(1);

  return tmpVec;
}

Vector<Double> PrincipalAxesDD::worldAxisIncrements() 
{
  Vector<Double> tmpVec(2);
  tmpVec(0) = itsCoordSys.increment()(0);
  tmpVec(1) = itsCoordSys.increment()(1);

  return tmpVec;
}

void PrincipalAxesDD::worldAxisType(Coordinate::Type &type, Int &coordinate,
				    Int &axisincoord,
				    const uInt &worldaxisnum) 
{
  itsCoordSys.findWorldAxis(coordinate, axisincoord, worldaxisnum);
  if ((coordinate < 0) || (axisincoord < 0)) {
    throw(AipsError("Couldn't find requested axis"));
  }
  type = itsCoordSys.type(coordinate);
}

String PrincipalAxesDD::worldAxisCode(const uInt &worldaxisnum) 
{
  Coordinate::Type ctype;
  Int coordinate, axisincoord;
  worldAxisType(ctype, coordinate, axisincoord, worldaxisnum);
  String retval;
  switch (ctype) {
  case Coordinate::LINEAR:
    {
      LinearCoordinate lcoord = 
	itsCoordSys.linearCoordinate(coordinate);
      retval = lcoord.showType();
    }
    break;
  case Coordinate::DIRECTION:
    {
      DirectionCoordinate dcoord = 
	itsCoordSys.directionCoordinate(coordinate);
      retval = dcoord.showType() + 
	MDirection::showType(dcoord.directionType());
    }
    break;
  case Coordinate::SPECTRAL:
    {
      SpectralCoordinate scoord = 
	itsCoordSys.spectralCoordinate(coordinate);
      retval = scoord.showType() + 
	MFrequency::showType(scoord.frequencySystem());
    }
    break;
  case Coordinate::STOKES:
    {
      StokesCoordinate scoord = 
	itsCoordSys.stokesCoordinate(coordinate);
      retval = scoord.showType();
    }
    break;
  case Coordinate::TABULAR:
    {
      TabularCoordinate tcoord = 
	itsCoordSys.tabularCoordinate(coordinate);
      retval = tcoord.showType();
    }
    break;
  default:
    // nothing special
    break;
  }
  char chnum[20];
  sprintf(chnum, "%d", axisincoord);
  retval = retval + String(chnum);
  return retval;
}

const uInt PrincipalAxesDD::nelements(const WorldCanvasHolder 
				      &wcHolder) const 
{
  return itsNumImages;
}

const uInt PrincipalAxesDD::nelements() const 
{
  return itsNumImages;
}

void PrincipalAxesDD::setDataMin(Double datmin) 
{
  datamin = datmin;
}

void PrincipalAxesDD::setDataMax(Double datmax) 
{
  datamax = datmax;
}

Double PrincipalAxesDD::getDataMin() 
{
  return datamin;
}

Double PrincipalAxesDD::getDataMax() 
{
  return datamax;
}

Bool PrincipalAxesDD::sizeControl(WorldCanvasHolder &wch,
                                  AttributeBuffer &holderBuf) {
  // Set the World Canvas Coordinate state: the WC CS, draw area and
  // zoom window.  The 'CS master' handles it.
  // See notes at top of DisplayData.h.

  // During a sizeControl call, isCSmaster()==True means that the CS master
  // role is on offer.  In that case, acceptance of the role is indicated by
  // the sizeControl return value.  A DD could still refuse the role at this
  // point by returning False (although this derivation does not).  In 
  // that case, the role would be offered to other DDs, if any.
  
  // If isCSmaster()==False, another DD is CS master, and we are not
  // allowed to change WC CS or axis codes.  We _would_ be allowed to alter
  // maximum zoom extents or, e.g., tweak the zoom window to align on pixel
  // boundaries, but this derivation does not.
  
  if(!isCSmaster(&wch)) return False;
  
  
  // CS master is on offer; we will accept that role by returning True.
  // This means taking charge of setting the WC CS (and its 'axis codes'),
  // the canvas draw area, zoom window, and maixmum zoom extents.
  
  // The CSMaster will also be called upon to perform coordinate
  // conversions for the WC.  (Eventually, the WC CS should really
  // do that; that's what it's there for, after all...).

  WorldCanvas *wCanvas = wch.worldCanvas();

  // For the WC's CS, 'remove' pixel axes from the DD's 'transposed' CS
  // (that is, transposed from data order to the order of user-chosen
  // display and slice control axes).  Starting from the animation axis,
  // the controls' current slice position settings are substituted into
  // the WC CS as the 'fixed' pixel coordinates for these 'removed' axes.
  
  // (dk note 7/04: itsFixedPosition for the Z axis is (still) poorly
  // synchronized with the zIndex of the WC[H].  It is somewhat redundant
  // state with activeZIndex_, which is rather more reliably updated,
  // via setActiveZIndex_() (or the deprecated findActiveImage()).  As a
  // result, the wccs below may not be 'fixed' at the correct Z position,
  // although at present this has relatively minor impact.  The fix is
  // probably to set itsFixedPosition[itsDisplayaxes[2]] = activeZIndex_
  // just before returning from setActiveZIndex_()).
  //
  // (dk note 9/07: itsFixedPosition now synched to activeZIndex_ per above.
  // The next statement assures that the latest zIndex is retrived into
  // these from the wch, where the definitive value is kept). 
  
  conformsToZIndex(wch);
  

  CoordinateSystem wccs = itsCoordSys;
  if (wccs.nPixelAxes() > 2) {
    const uInt startAxis = 2;
    removePixelAxes (wccs, startAxis, itsFixedPosition);
  }
  wCanvas->setCoordinateSystem(wccs);


  // X and Y axis codes are WC Attributes which describe
  // the WC CS's world coordinate types, e.g.  RA J2000 on X, VEL(TOPO) on Y.
  // (They also encode the 'axis in coordinate', which is probably irrelevant).
  //
  // This is still not enough to distinguish, e.g., two Linear or
  // TabularCoordinates.  (It's a good idea for a Caching DD to add axis
  // names as well (at least), in optionsAsAttributes()...).
  //
  // Axis codes need improvement.  They should encode everything needed by
  // a DD in most cases to decide whether it is compatible with the WC's
  // current world coordinates (although if necessary a DD can look at
  // WC CS info directly as well).   If the DD is not compatible, it should
  // return False from conformsToCS(), and should not draw. 
  // Ideally, axis codes would not be limited to two world coordinates as
  // they are now.  (In the example above, world space is 3-dimensional, and
  // axis codes should include a Declination world coordinate as well).  
  // Neither should it be necessary in all cases to identify one world
  // coordinate with linear X and one with linear Y.

  String xAxis = "xaxiscode (required match)",
         yAxis = "yaxiscode (required match)";
  Attribute xAxisCode(xAxis, worldAxisCode(0));
  Attribute yAxisCode(yAxis, worldAxisCode(1));
  
  wCanvas->setAttribute(xAxisCode); wCanvas->setAttribute(yAxisCode);

  
  // add info on native coordinate units

  Attribute xAxisUnits("xaxisunits", worldAxisUnits()(0));
  Attribute yAxisUnits("yaxisunits", worldAxisUnits()(1));
  wCanvas->setAttribute(xAxisUnits); wCanvas->setAttribute(yAxisUnits);


  // The 'zoom window' consists of 'linear' (which usually means
  // data-pixel) coordinates for the corners of the desired viewing
  // area: lin{X,Y}{Min,Max} (in other words, the region of data
  // currently on view).  By convention, the _center_ of the data's 
  // blc pixel has coordinate 0, its bottom edge is at -.5.

  Double linXMin, linXMax, linYMin, linYMax;

  Double minX=0.,	// maximun permitted 'linear coordinate'
	 minY=0.,	// (data pixel) zoom extents.
	 maxX = Double(max( 1, dataShape()[itsDisplayAxes[0]] )),
	 maxY = Double(max( 1, dataShape()[itsDisplayAxes[1]] ));

	 
  // Zoom window boundaries are aligned on data pixel edges
  // ("edge") or pixel centers ("center").
  
  // (dk note: PixelTreatment and its relatives are overcomplicated,
  // muddled and illegible; they should have been exposed at birth.
  // Optional edge alignment (vs. none) could be offered for the zoom
  // window (though it really isn't necessary).  But "center" is unneeded,
  // and the WC::drawImage methods (and the DMs) should deal exclusively
  // with edge coordinates, and not have to know anything about the
  // alignment setting).
  
  Bool edge = (itsOptionsPixelTreatment == "edge" || maxX==1. || maxY==1.);
		// (Data must be at least 2 pixels wide on
		// both axes to allow "center" alignment).

  // For "edge" alignment (only), .5 is temporarily added to all data pixel
  // coordinates (maxX, linXMax, etc.), and subtracted at the end.  That
  // way, either type of alignment is accomplished by rounding these values
  // to the nearest integer.

  Double shift = (edge)?  .5 : 0.;
  if(!edge) { maxX--; maxY--;  }
	// These maximum zoom extent values now include the
	// the temporary .5 pixel as well, where needed.


  // The (pre-adjustment) zoom window is determined by one of four
  // sources (in increasing order of priority):

  // 1) The values already on the WC.

  // 2) An order to 'unzoom', or zoom-to-extent (the "resetCoordinates"
  //    Attribute.  This is all this attribute does now; the CSMaster DD's
  //    sizeControl always assumes that the WC CS and corresponding
  //    axis codes are to be [re]set as it chooses).

  // 3) Attributes set through the WC::setZoomRectangleLCS() method (usually
  //    by the mouse zoomer) to order a zoom ("manualZoom{Blc,Trc}").
  
  // 4) A zoom-to-extent will also occur, overriding any zoom order above,
  //    under the true 'reset' condition, which is shown below.

  // Any such 'command Attributes' from cases 2) and 3) are removed
  // from the WC after they have been processed.

  Bool reset = !wch.wasCSmaster(this);
	// 'reset' condition: this is the first sizeControl since this
	// DD was made CS master.  'reset' will force 'unzoom'.
	// (However: if the _same_ master is changing axis codes, it
	// should already have set zoom-order attributes appropriately
	// elsewhere.  See, e.g., unzoom order in ::setAxes()).
  

  static String unZoom = "resetCoordinates",
		 zoomB = "manualZoomBlc", zoomT = "manualZoomTrc";

  Bool zoom = False;
  
  Vector<Double> zoomBlc, zoomTrc;
  if (!reset &&  wCanvas->existsAttribute(zoomB) &&
		 wCanvas->existsAttribute(zoomT)) {
    wCanvas->getAttributeValue(zoomB, zoomBlc);
    wCanvas->getAttributeValue(zoomT, zoomTrc);
    
    zoom = (zoomBlc.nelements() >= 2) && (zoomTrc.nelements() >= 2);  }

  // An 'unzoom' order on the canvas would be overridden by any 'zoom' order
  // there (but 'reset' would override that).

  Bool unzoom = reset;
  
  if (!reset && !zoom && wCanvas->existsAttribute(unZoom)) {
    
    wCanvas->getAttributeValue(unZoom, unzoom);  }


  if (unzoom) {
    linXMin = minX; linYMin = minY;	// zoom to extents (temporary
    linXMax = maxX; linYMax = maxY;  }	// shift already included).
  
  else {
    if(zoom) {
      linXMin = zoomBlc[0]; linXMax = zoomTrc[0];	// zoom order.
      linYMin = zoomBlc[1]; linYMax = zoomTrc[1];  }
    
    else {
      linXMin = wCanvas->linXMin(); linXMax = wCanvas->linXMax();
      linYMin = wCanvas->linYMin(); linYMax = wCanvas->linYMax();  }
		// no [un]zoom order--keep the WC's current zoom window).
    
    linXMin += shift; linXMax += shift;    // add temporary shift.
    linYMin += shift; linYMax += shift;  }

    
  wCanvas->removeAttribute(zoomB);	// '[Un]zoom order' attributes
  wCanvas->removeAttribute(zoomT);	// have been acted upon as
  wCanvas->removeAttribute(unZoom);	// needed--remove them.

  
  // Rounding up or down at .5 is just to make "center" and "edge" options
  // inverses of each other if toggled (instead of drifting upwards).

  Double roundUp = .5,   roundDn = .5 - 1.e-5;
  Double     minHalf = roundUp,  maxHalf = roundDn;
  if(edge) { minHalf = roundDn;  maxHalf = roundUp;  }

  // Assure at least 1 pixel width, align, clip to max extents.

  linXMin = min(linXMin, (linXMin+linXMax)/2.-.5 );
	// (Expands to 1 pixel from center, if zoom window is tiny)
  linXMin = max(minX, min(maxX-1.,   floor(linXMin + minHalf)));
  linXMax = max(linXMin+1, min(maxX, floor(linXMax + maxHalf)));

  linYMin = min(linYMin, (linYMin+linYMax)/2.-.5 );
  linYMin = max(minY, min(maxY-1.,   floor(linYMin + minHalf)));
  linYMax = max(linYMin+1, min(maxY, floor(linYMax + maxHalf)));

  // Shift back to normal linear coordinates.

  linXMin -= shift; linXMax -= shift;
  linYMin -= shift; linYMax -= shift;
  minX    -= shift; minY    -= shift;
  maxX    -= shift; maxY    -= shift;

  // Ship out final zoom window and max extents.

  holderBuf.add("linXMin", linXMin);
  holderBuf.add("linXMax", linXMax);
  holderBuf.add("linYMin", linYMin);
  holderBuf.add("linYMax", linYMax);
  holderBuf.add("linXMinLimit", minX);
  holderBuf.add("linXMaxLimit", maxX);
  holderBuf.add("linYMinLimit", minY);
  holderBuf.add("linYMaxLimit", maxY);



  // Adjust draw area for desired data pixel aspect.

  uInt canvasDrawXSize = wCanvas->canvasDrawXSize();
  uInt canvasDrawYSize = wCanvas->canvasDrawYSize();

  Int imageXSize = Int(linXMax - linXMin);
  Int imageYSize = Int(linYMax - linYMin);

  Double drawXSize = Double(canvasDrawXSize)/Double(imageXSize);
  Double drawYSize = Double(canvasDrawYSize)/Double(imageYSize);

  if ((itsOptionsAspect == "fixed world") &&
      (worldAxisUnits()(0) == worldAxisUnits()(1))) {
    
    // equal screen pixels per world unit on X and Y
    
    Double drawSize = min(drawXSize / fabs(worldAxisIncrements()(0)),
			  drawYSize / fabs(worldAxisIncrements()(1)));
    drawXSize = max(1.0, drawSize*fabs(worldAxisIncrements()(0))*imageXSize);
    drawYSize = max(1.0, drawSize*fabs(worldAxisIncrements()(1))*imageYSize);
  
  } else if(itsOptionsAspect=="fixed lattice"){
    
    // equal screen pixels per data lattice pixel ('square data pixels')
    
    Double drawSize = min(drawXSize, drawYSize);
    drawXSize = drawSize*imageXSize;
    drawYSize = drawSize*imageYSize;
  
  } else {
    
    // 'flexible' -- data pixel aspect adjusted to use all available area.
    
    drawXSize = (Double)canvasDrawXSize;
    drawYSize = (Double)canvasDrawYSize;
  }

  // recenter draw area

  uInt canvasDrawXOffset = wCanvas->canvasDrawXOffset() +
    uInt((Double(canvasDrawXSize)-drawXSize)/2.0 + 0.5);
  uInt canvasDrawYOffset = wCanvas->canvasDrawYOffset() + 
    uInt((Double(canvasDrawYSize)-drawYSize)/2.0 + 0.5);

  // set size and position of drawable on WorldCanvas

  holderBuf.add("canvasDrawXSize", uInt(drawXSize + 0.5));
  holderBuf.add("canvasDrawYSize", uInt(drawYSize + 0.5));
  holderBuf.add("canvasDrawXOffset", canvasDrawXOffset);
  holderBuf.add("canvasDrawYOffset", canvasDrawYOffset);

  // we store the sizeControl state so we can check in the refreshEH if the
  // refreshEH can assume that the sizeControl has been done by this
  // ImageDisplayData.  
  // (dk note: this is very old stuff ('ImageDisplayData'?!  :-)
  // isCSmsater() should really be used to determine this now...).

  sizeControlBuf = holderBuf;


  return True;
}


// Position event handler called by WorldCanvasHolder.  Information is
// added to the event, and the event gets distributed over all the
// event handlers that are installed on this DisplayData, provided the
// event handler is of the correct type:
void PrincipalAxesDD::positionEH(const WCPositionEvent &ev) {
  if (iAmRubbish) {
    throw(AipsError("PrincipalAxesDD::positionEH - "
		    "object in incorrect state"));
  }
  DisplayData::positionEH(ev);
}

// Motion event handler used by the WorldCanvasHolder.  Information is
// added to the event, and the event gets distributed across all the 
// event handlers installed on this display data, provided the event
// handler is of the correct type:
void PrincipalAxesDD::motionEH(const WCMotionEvent &ev) {
  if (iAmRubbish) {
    throw(AipsError("PrincipalAxesDD::motionEH - "
		    "object in incorrect state"));
  }
  DisplayData::motionEH(ev);
}



// Refresh event handler used by the WorldCanvasHolder.  The event gets
// distributed over the event handlers that are installed on the 
// DisplayData.  Contrary to the position and motion events, the refresh
// event is identical to the one generated by the WorldCanvas, and no
// extra information is added.  No restriction on the type of the
// event handler exists.  After calling the handler/s on the DisplayData,
// the actual refresh of the WorldCanvas is done.
void PrincipalAxesDD::refreshEH(const WCRefreshEvent &ev)
{

  if (iAmRubbish) {
    throw(AipsError("PrincipalAxesDD::refreshEH - "
		    "object in incorrect state"));
  }

  WorldCanvas *wCanvas = ev.worldCanvas();
  WorldCanvasHolder *wcHolder = findHolder(wCanvas);

  if (!wcHolder) return;	// (shouldn't happen)


// if the colortable has changed, we must delete all the display
// lists (regardless of whether we draw this time), because they
// will no longer apply.

  if (ev.reason() == Display::ColorTableChange) {
    for (uInt i = 0; i < nelements(); i++) {
      ((PrincipalAxesDM *)DDelement[i])->cleanup();
    }
  }

  if ( !conformed() || activeZIndex_<0 ||
       uInt(activeZIndex_) >= nelements() ) return;
	// (for safety: redundant with conformsTo() check done on WCH).

// create iterator and temporary pointers

  ConstListIter<WCRefreshEH*> iter(refreshEventHandlerList());
  WCRefreshEH *tmp_EH = 0;

// loop

  while (!iter.atEnd()) {
    tmp_EH = iter.getRight();
    (*tmp_EH)(ev);
    iter++;
  }

  PrincipalAxesDM *theImage = 0;
  theImage = (PrincipalAxesDM *)DDelement[activeZIndex_];

// install a colormap if needed

  if (colormap() != 0) wCanvas->setColormap(colormap());
  else if(!wCanvas->pixelCanvas()->colormapRegistered())
	 wCanvas->setColormap( (Colormap *)
	(wCanvas->pixelCanvas()->pcctbl()->colormapManager().getMap(0)) );
			// (shouldn't happen)


  theImage->draw(ev.reason(), *wcHolder);

  
  // Derived classes implement if applicable (at present, LatticePADDs
  // will draw a beam ellipse if they have an image with beam data and
  // the WorldCanvas CoordinateSystem is set for sky coordinates).
  drawBeamEllipse_(wCanvas);
  
}




void PrincipalAxesDD::setAxes(const uInt xAxis,
			      const uInt yAxis,
			      const uInt mAxis,
			      const IPosition fixedPos,
			      Bool reset) 
{
// store any existing axis labelling options

  Record axisrec;
  if (itsUsesAxisLabels) {
    if (itsNumImages > 0) {
      axisrec = ((WCCSNLAxisLabeller *)itsAxisLabellers[0])->getOptions();
    }
  }
//
  iAmRubbish = False;
  itsDisplayAxes(0) = xAxis;
  itsDisplayAxes(1) = yAxis;
  itsDisplayAxes(2) = mAxis;
  
  setup(fixedPos);	// a major part (along with worldToLin and pals)
  setupElements();	// of the ugliness mentioned below....
  
  if(reset) {
  
    // dk note: 'reset' (default True) is passed as False instead by setOptions
    // if only a slider position was changed.  In that case, we don't want
    // to unzoom or reset animator position (this is a bugfix).  
    // Properly, the code which switches axes should be separated from the code
    // to set slice positions.  But all that code and state is so ugly I just
    // can't bear (or spare the time) to go there right now....

    setActiveZIndex_(0);
	// (itsFixedPosition[mAxis] is probably a better zIndex choice...
	// but this is not really the place to set this, though, and is
	// probably a no-op anyway...).
  
    // order unzoom
    String attString("resetCoordinates");
    Attribute resetCoordinatesAtt(attString, True);
    setAttributeOnPrimaryWCHs(resetCoordinatesAtt);

    // convince all WCes with this DD as first registered that they
    // need to have their animators reset themselves...

    // (unneeded now: this is a message to old-style (obsolete)
    // Animator.  See the "setanimator" field to the setOptions() output
    // record (below) for the real message).

    attString = "resetAnimator";
    Attribute resetAnimatorAtt(attString, True);
    setAttributeOnPrimaryWCHs(resetAnimatorAtt);
  }

// restore the axis labelling options

  if (itsUsesAxisLabels) {
    Record updatedOptions;
    for (uInt i = 0; i < itsNumImages; i++) {
      ((WCCSNLAxisLabeller *)itsAxisLabellers[i])->setOptions(axisrec,
							      updatedOptions);
    }
  }
}

void PrincipalAxesDD::setCoordinateSystem(const CoordinateSystem& coordsys) 
{
  itsCoordSys = coordsys;

// make sure any DirectionCoordinate has radians as native units
// as this is assumed in various places

  for (uInt i = 0; i < itsCoordSys.nCoordinates(); i++) {
    if (itsCoordSys.type(i) == Coordinate::DIRECTION) {
      CoordinateUtil::setDirectionUnit (itsCoordSys, String("rad"), Int(i));
    }
  }

// Set toMix ranges.  The min/max are stored in the CS
// but its a little expensive to fish it out every time
// (they are actually retrieved from each Coordinate)
// toMix is called.  So we hold a copy here as well
// but have to be careful to update it when appropriate

  itsCoordSys.setWorldMixRanges (dataShape());
  itsWorldMin.resize(itsCoordSys.nWorldAxes());
  itsWorldMax.resize(itsCoordSys.nWorldAxes());
  itsWorldMin = itsCoordSys.worldMixMin();
  itsWorldMax = itsCoordSys.worldMixMax();

// Make copies.

  itsOrigCoordSys = itsCoordSys;
  itsPosTrackCoordSys = itsOrigCoordSys;
  setSpectralFormatting(itsPosTrackCoordSys,  itsDoppler, itsSpectralUnit);
}


void PrincipalAxesDD::restoreCoordinateSystem() 
{
  itsCoordSys = itsOrigCoordSys;
  itsPosTrackCoordSys = itsOrigCoordSys;
  setSpectralFormatting(itsPosTrackCoordSys,  itsDoppler, itsSpectralUnit);
}

void PrincipalAxesDD::installFakeCoordinateSystem() 
{
  itsCoordSysBackup = itsCoordSys;
  itsCoordSys = 
    CoordinateUtil::makeCoordinateSystem(dataShape(), True);
}

void PrincipalAxesDD::removeFakeCoordinateSystem() 
{
  itsCoordSys = itsCoordSysBackup;
}



// ************************************************************
// PROTECTED

void PrincipalAxesDD::setup(IPosition fixedPos) 
{
  itsFixedPosition = fixedPos;

// Restore to original CS. All CSs are now the same and in the original order

  restoreCoordinateSystem();
  CoordinateSystem tCS = itsCoordSys;

// Fill in axis names if needed (I [nebk] doubt this code is needed)

  Vector<String> wAxisNames(tCS.worldAxisNames().copy());
  for (uInt i = 0; i < wAxisNames.nelements(); i++) {
    Bool rename = False;
    if (wAxisNames(i) == String("")) {
      rename = True;
    } else {
      for (uInt j = 0; j < i; j++) {
	if (wAxisNames(j) == wAxisNames(i)) {
	  rename = True;
	}
      }
    }
    if (rename) {
      char ch_name[20];
      sprintf(ch_name, "Axis %d", i + 1);
      wAxisNames(i) = String(ch_name);
    }
  }
  tCS.setWorldAxisNames(wAxisNames);

// Re-set CS (save as itsOriginalCS, copy to itsCS and itsPosTrackCS)
// After this we work with itsCS again

  setCoordinateSystem(tCS);
//
  nWorldAxes = itsCoordSys.nWorldAxes();
  nPixelAxes = itsCoordSys.nPixelAxes();
  nArrayAxes = dataDim();

// make sure we have at least 2 axes:

  if (nArrayAxes < 2) {
    throw(AipsError("PrincipalAxesDD::setup - "
		    "Data has less than two dimensions"));
  }

// make sure array has same number of axes as coord system:

  if (nPixelAxes != nArrayAxes) {
    throw(AipsError("PrincipalAxesDD::setup - "
		    "Coordinate system and data do not conform"));
  }

// make sure itsFixedPosition is useable:

  if (itsFixedPosition.nelements() != nPixelAxes) {
    throw(AipsError("PrincipalAxesDD::setup - "
		    "Fixed position does not conform with data"));
  }


// length is the number of displayaxes - it's either two or three.

  uInt length = nPixelAxes;
  if (length > 3) length = 3;

// if length != 2 or 3, I do not know what to do.
// this needs to be fixed in the future

  if (length< 2 || length>3) {
    throw(AipsError("PrincipalAxesDD::setup - "
		    "Cannot deal with < 2 or > 3 axes"));
  }

// Setup pixel axes transpose vector. The pixel axes of the coord 
// system are  guarenteed, when you open the image, to be in 
// the order of the  image pixel axes.  We tranpose the pixel
// axes so the first 2/3 are those we are displaying in x, y & z

  itsTransPixelAxes.resize(nPixelAxes);
  itsTransWorldAxes.resize(nWorldAxes);
  Int pixelAxis, worldAxis;
  Int coordinate, axisInCoordinate;
  Coordinate::Type type0=Coordinate::LINEAR, type1=Coordinate::LINEAR;
	// (initialized to make compiler happy--should be reset below)

  for (uInt axis=0; axis<length; axis++) {
    pixelAxis = itsDisplayAxes(axis);
    itsTransPixelAxes(axis) = pixelAxis;

//  Each pixel axis is guarenteed to have a world axis

    worldAxis = itsCoordSys.pixelAxisToWorldAxis(pixelAxis);
    itsTransWorldAxes(axis) = worldAxis;
//
    itsCoordSys.findPixelAxis(coordinate, axisInCoordinate, pixelAxis);
    if (axis==0) {
       type0 = itsCoordSys.type(coordinate);
    } else if (axis==1) {
       type1 = itsCoordSys.type(coordinate);
    }
  }

// fill up the remaining axes for the tranposition pixel axes

  Int length2 = length;
  for (uInt index=0; index<nPixelAxes && length2<Int(nPixelAxes); index++) {
    if (indexInserted(index, length2, itsTransPixelAxes)) length2++;
  }

// See if there is a hidden dependent world axis.

  Int pAxis = -1;
  if (type0==Coordinate::DIRECTION && type1!=Coordinate::DIRECTION) {
     pAxis = itsTransPixelAxes(0);
  } else if (type1==Coordinate::DIRECTION && type0!=Coordinate::DIRECTION) {     
     pAxis = itsTransPixelAxes(1);
  }

// We know one of the first two display axes is Direction and
// the other is not.  Find the world axis of the Direction Coordinate
// that is not being displayed. This is the hidden dependent world axis
// If there are removed pixel axes, the hdwa might not have
// a corresponding pixel axis.  This algorithm will find it.

  Int hiddenDependentWorldAxis = -1;
  if (pAxis!=-1) {
     itsCoordSys.findPixelAxis(coordinate, axisInCoordinate, pAxis);
     Int wAxis = itsCoordSys.pixelAxisToWorldAxis(pAxis);
     Vector<Int> worldAxes = itsCoordSys.worldAxes(coordinate);
//
     for (uInt i=0; i<worldAxes.nelements(); i++) {
        if (worldAxes(i) != wAxis) {
           hiddenDependentWorldAxis = worldAxes(i);
           break;
        }
     }
  }

// Now we must set up the rest of the world axis transposition vector.
// Any old order will do

  length2 = length;
  for (uInt index=0; index<nWorldAxes && length2<Int(nWorldAxes); index++) {
    if (indexInserted(index, length2, itsTransWorldAxes)) length2++;
  }

// Tranpose the hidden dependent axis as well.

  if (hiddenDependentWorldAxis > -1) {
     for (uInt i=0; i<nWorldAxes; i++) {
        if (itsTransWorldAxes(i) == hiddenDependentWorldAxis)
	    hiddenDependentWorldAxis = i;
     }
   }
//
  String attString("hiddenDependentWorldAxis");
  if (existsAttribute(attString)) {
    removeAttribute(attString);
  }
  attString = "hiddenDependentAxisWorldValue";
  if (existsAttribute(attString)) {
    removeAttribute(attString);
  }  
  if (hiddenDependentWorldAxis > -1) {
    String attString("hiddenDependentWorldAxis");
    Attribute att(attString, hiddenDependentWorldAxis);
    setAttribute(att);
  }

// fix up itsFixedPosition.  This is always in the original order
// of the data (i.e. no transposition)

// (dk note: fix what?  This likely has no effect, since setOptions resets
//  it to 0 whenever the axis is moved off-display -- the only time that
//  fixedPos matters.  Formerly, this did help to cause a bug...).

  itsFixedPosition(itsDisplayAxes(0)) = dataShape()(itsDisplayAxes(0)) / 2;
  itsFixedPosition(itsDisplayAxes(1)) = dataShape()(itsDisplayAxes(1)) / 2;
  
  
  itsAddPixPos.resize(nPixelAxes);
  for (uInt index=0; index<nPixelAxes; index++){
    itsAddPixPos(index) = itsFixedPosition(index);
  }

// transpose pixel position and CoordinateSystem

  normalToTransposed(itsAddPixPos, itsTransPixelAxes);
  itsCoordSys.transpose(itsTransWorldAxes, itsTransPixelAxes);

  IPosition oldShape = dataShape();
  IPosition newShape(oldShape.nelements());
  for (uInt i=0;i<newShape.nelements(); i++) {
     newShape(i) = oldShape(itsTransPixelAxes(i));
  }

// make sure toMix ranges are adjusted as well

  itsWorldMin = itsCoordSys.worldMixMin();
  itsWorldMax = itsCoordSys.worldMixMax();

}

  
void PrincipalAxesDD::setupElements()
{

  if (itsUsesAxisLabels) {
// delete excess leftover AxisLabellers, if any.

    uInt nnew = nelements(), nold = itsAxisLabellers.nelements();
    for(uInt i=nnew; i<nold; i++) {
       delete ((WCCSNLAxisLabeller *)itsAxisLabellers[i]);
    }

// forceSmaller=True necessary here; otherwise, after
// itsAxisLabellers.resize(nnew),
// itsAxisLabellers.nelements() may not be == nnew!!  )-;
    itsAxisLabellers.resize(nnew, True);

// Copy the CoordinateSystem for the Axis Labeller.  
// We copy the working version which has been reordered to
// the display axes order  and who knows what else done to it !

    CoordinateSystem axisLabelCS = itsCoordSys;

  // Remove all hidden pixel axes and assign replacement value
  // (the hidden axes values) to be used in coordinate conversions
  // in the axis labeller

    const uInt startAxis = 3;
    removePixelAxes (axisLabelCS, startAxis, itsFixedPosition);

// Set default nice world axis units and velocity preference

    CoordinateUtil::setNiceAxisLabelUnits(axisLabelCS);
    const uInt nPixelAxes = axisLabelCS.nPixelAxes();

  // Now prepare an AxisLabeller and CS for each plane of the movie axis

  // dk note 4/02: this has to be fixed.  I have seen this loop take up to
  // 78kb per plane (!).  This can be far too expensive if there are many
  // frames on the movie axis.  It needs (just one) WorldAxesDD instead,
  // and updating of the WC CS before each axis draw.

    for (uInt index = 0; index < nnew; index++) 
    {
      CoordinateSystem tCS = axisLabelCS;

// Remove the movie pixel axis and assign replacement
// value to the movie axis index

      if (nPixelAxes > 2) tCS.removePixelAxis(2, index);
 
// Recycle old axis labellers where possible--saves a little time.
      WCCSNLAxisLabeller* tLab = 0;
  
      if(index>=nold) {
         tLab = new WCCSNLAxisLabeller();
      } else {
         tLab = static_cast<WCCSNLAxisLabeller*>(itsAxisLabellers[index]);
	 tLab->invalidate();	//(discard old drawlist, if any).
      }
// Now give the labeller its own CS and set the default options
      tLab->setCoordinateSystem(tCS);
      tLab->useWCCS = True;
	// (new) signal to use WC CS anyway where appropriate (but tCS
	// still needed for some things).
      tLab->setDefaultOptions();  // (needed after setCS(), for CS opts...).
      if (nelements() > 1) tLab->setZIndex(index);
      itsAxisLabellers[index] = tLab;                // Takes over the pointer
    }
  } 
}

Bool PrincipalAxesDD::getFullCoord(Vector<Double>& fullWorld,
				   Vector<Double>& fullPixel,
				   const Vector<Double> &world) 
//
// Convert a 2d world coordinate on the world canvas to a full world
// coordinate and pixel coordinate.  
//
{
// sanity checks

  if (iAmRubbish) {
    return False;
  }

// the input must be a 2d world coordinate on the WorldCanvas, and the 
// output will be a 2d linear coordinate on the WorldCanvas.

  if (world.shape()(0) < 2) {
    return False;
  }

// temporary holding places

  if (itsWorldInTmp1.nelements()!=nWorldAxes) {
     itsWorldInTmp1.resize(nWorldAxes);
     itsWorldAxesTmp1.resize(nWorldAxes);
  }
//
  if (itsPixelInTmp1.nelements()!=nPixelAxes) {
     itsPixelInTmp1.resize(nPixelAxes);
     itsPixelAxesTmp1.resize(nPixelAxes);
  }

// Fill in what we know in pixel coords

  itsPixelAxesTmp1 = True;
  itsPixelAxesTmp1(0) = itsPixelAxesTmp1(1) = False;
  if (nPixelAxes > 2) {
    itsPixelInTmp1(2) = activeZIndex_;
  }
  for (uInt i = 3; i < itsAddPixPos.nelements(); i++) {
    itsPixelInTmp1(i) = itsAddPixPos(i);
  }

// Fill in what we know in world coords. 
// N.B. If there is a world axis for which the  pixel axis has been 
// removed (e.g. a moment image), then we set worldAxes = F
// for that axis.  This means the replacement pixel coordinate
// stored in the CoordinateSystem will be used for computations
// via the toMix call.

  itsWorldInTmp1(0) = world(0);
  itsWorldInTmp1(1) = world(1);
  itsWorldAxesTmp1 = False;
  itsWorldAxesTmp1(0) = itsWorldAxesTmp1(1) = True;

// do conversion

  if (itsCoordSys.toMix(fullWorld, fullPixel, itsWorldInTmp1, itsPixelInTmp1,
				itsWorldAxesTmp1, itsPixelAxesTmp1, itsWorldMin,
				itsWorldMax)) {
// transpose to normal ordering

    transposedToNormal(fullWorld, itsTransWorldAxes);
    transposedToNormal(fullPixel, itsTransPixelAxes);
    return True;
  } else {
    return False;
  }
}

Bool PrincipalAxesDD::indexInserted(Int index, Int length,
    			            Vector<Int> &testVec) 
{
  // sanity check
  if (length>=Int(testVec.nelements()) || length <= 0) {
    return False;
  }
  
  for (Int i=0; i<length; i++) {
    if (testVec(i) == index) {
      return False;
    }
  }

  testVec(length) = index;
  return True;
}

void PrincipalAxesDD::normalToTransposed(Vector<Double>& coord, 
                                         const Vector<Int>& transPos) 
{
  Int length = coord.shape()(0);
  if (length != transPos.shape()(0) ) return;
//  
  Vector<Double> tmpD(length);
  for (Int i = 0; i < length; i++) {
    tmpD(i) = coord(transPos(i));
  }
  coord = tmpD;
}

void PrincipalAxesDD::transposedToNormal(Vector<Double>& coord, 
                                         const Vector<Int>& transPos) 
{
  Int length = coord.shape()(0);
  
  if (length != transPos.shape()(0) ) {
    return;
  }
  
  Vector<Double> tmpD(length);
  for (Int i = 0; i < length; i++) {
    tmpD(transPos(i)) = coord(i);
  }
  coord = tmpD;  
}

Bool PrincipalAxesDD::findActiveImage(WorldCanvasHolder &wch) {
  // dk note: This method is deprecated; use conformsTo(wch) instead.
  //
  // This routine uses older, more general restriction matching for PADMs.
  // Its use was mixed, somewhat inconsistently, with setActiveImage()
  // (setActiveZIndex_() is now used everywhere instead, via conformsTo()).
  // setActiveImage (now setActiveZIndex_) assumes that DDelement[i] will
  // have zIndex restriction == i, and that that is the only relevant
  // restriction for the DMs on the WCH (it is the only restriction that
  // was ever really used for PADM and derivatives).
  // findActiveImage() and conformsTo() assure that internal state
  // (activeZIndex_) is 'focused' on the WC[H] of interest (including its
  // zIndex).
  //
  // CachingDD (from which new DDs should be derived) implements
  // general DM restriction matching as well, but its optionsAsAttributes()
  // applies generalized restrictions only to a DD's own DMs (cached images),
  // not to those of all DDs registered on the canvas.
  // Canvas-wide state intended to restrict restrict all the canvas's
  // DDs/DMs is more specifically defined now (although it can be expanded
  // as well, with care).  The WC's coordinate state consists of the WC CS,
  // corresponding axis codes, zoom window and extents, canvas draw area
  // definition, and zIndex.  All but the latter is controlled by
  // the CSMaster DD (see DD::sizeControl).  zIndex, as well as bIndex
  // (animation and blink frame numbers) are controlled by the animator,
  // via MultiWCHolder.  DD conformity to canvas-wide state is tested via
  // conformsTo(wch).

  zIndexConformed_ = False;
  activeZIndex_ = 0;
  
  conformsToRstrs(wch);
  conformsToCS(wch);
	// Matches restriction buffers and checks coordinate compatibility
	// between DD and WC[H].   Note: this _will_ test any new, general
	// WCH restrictions against those of the DDs (as long as a way is
	// provided to place them on these objects).  (Note the difference
	// between DD restrictions, tested above, and DM restrictions,
	// tested below).

  if(rstrsConformed_ && csConformed_) {
    const AttributeBuffer *wcRes = wch.restrictionBuffer();

    // (This way of doing the DM restriction check can be slow, esp. on
    // a long movie axis...).
    for (uInt i = 0; i < itsNumImages; i++ ) {
      if (static_cast<PrincipalAxesDM*>(DDelement[i])->matches(
          *const_cast<AttributeBuffer*>(wcRes) )) {
        activeZIndex_ = i;
        zIndexConformed_ = True;  }  }  }
  
  // Keep itsFixedPosition(zAxis) in sync with latest zindex.
  
  if( itsDisplayAxes.nelements()   > 2u &&
      itsFixedPosition.nelements() > uInt(itsDisplayAxes[2]) ) {
    itsFixedPosition[itsDisplayAxes[2]] = activeZIndex_;  }
  
  return conformed();
}

Bool PrincipalAxesDD::labelAxes(const WCRefreshEvent &ev) 
{
  if (iAmRubbish) {
    throw(AipsError("PrincipalAxesDD::refreshEH - "
		    "object in incorrect state"));
  }
  
  // not using internal labellers
  if (!itsUsesAxisLabels) return False;

  if( !csConformed_ || activeZIndex_<0 ||
      uInt(activeZIndex_) >= itsAxisLabellers.nelements() ) return False;
	// (...but CS master with invalid blink restriction can still label--
	// its labelling CS is more reliable than other DDs'....  This
	// still needs work.  dk 12/04)
  
  if (!((WCAxisLabeller *)(itsAxisLabellers[activeZIndex_]))->
      axisLabelSwitch()) {
    return False;
  }
  
  try {
    WCCSNLAxisLabeller *theLabeller;
    theLabeller = (WCCSNLAxisLabeller *)itsAxisLabellers[activeZIndex_];
    theLabeller->setUIBase(uiBase());	// propagate DD setting to labeller;
	// determines whether to number from 0 or 1 when labelling in
	// 'Absolute Pixel' mode -- see uiBase() for details.
    theLabeller->draw(ev);
  } catch (const AipsError &x) {
    if (&x) { // use x to avoid compiler warning
      return False;
    }
  } 
  return True;
}

void PrincipalAxesDD::cleanup()
{
  for (uInt i = 0; i < itsNumImages; i++) {
    ((PrincipalAxesDM *)DDelement[i])->cleanup();
  }
  sizeControlBuf.clear();
}

void PrincipalAxesDD::setDefaultOptions()
{
  DisplayData::setDefaultOptions();
//
  const Vector<String>& pixelAxisNames =
  			worldToPixelAxisNames(itsCoordSys);
  itsOptionsXAxis = pixelAxisNames(0);
  itsOptionsYAxis = pixelAxisNames(1);
  if (pixelAxisNames.nelements() > 2) {
    itsOptionsZAxis = pixelAxisNames(2);
  }
//
  itsOptionsPixelTreatment = "edge";
  Int nAxes = pixelAxisNames.nelements();
  itsFixedPosition.resize(nAxes);
  itsFixedPosition = 0;

// Set the aspect to flexible if user is displaying unlike axes

  Int c0, c1, a0, a1;
  itsCoordSys.findPixelAxis(c0, a0, 0);
  itsCoordSys.findPixelAxis(c1, a1, 1);
  
  itsOptionsAspect = "fixed world";
  if (itsCoordSys.type(c0) != itsCoordSys.type(c1)) {
     itsOptionsAspect = "flexible";
  }

// If there is a SpectralCoordinate, set default velocity state and
// spectral units in all of the coordinate systems that we are using.
// As the user interacts with the Adjust GUIs etc this equality
// of state will diverge

  setVelocityState (itsCoordSys, itsDoppler, itsSpectralUnit);
  setVelocityState (itsOrigCoordSys, itsDoppler, itsSpectralUnit);

// Set up the default formatting for the SpectralCoordinate (handles km/s or Hz)

  setSpectralFormatting (itsPosTrackCoordSys, 
                         itsDoppler, itsSpectralUnit);
}

Bool PrincipalAxesDD::setLabellerOptions(Record &rec, Record &recout)
{
  if (!itsUsesAxisLabels) return False;

  WCCSNLAxisLabeller* tLab =
  		      static_cast<WCCSNLAxisLabeller*>(itsAxisLabellers[0]);
  Bool axisUpdated = tLab->setOptions(rec, recout);
  for (uInt i = 1; i < itsNumImages; i++) {
    tLab = static_cast<WCCSNLAxisLabeller*>(itsAxisLabellers[i]);  
    tLab->setOptions(rec, recout);
  }
  return axisUpdated;
}



Bool PrincipalAxesDD::setOptions(Record &rec, Record &recOut) 
{
  Bool ret = DisplayData::setOptions(rec, recOut);

  Bool error;

// Deal with local things; Position tracking stuff. Set the formatting state
// in the CS used for position tracking

  String frequency_system("");
  Bool vtchg = readOptionRecord(itsDoppler, error, rec, "velocitytype");
  Bool spchg = readOptionRecord(itsSpectralUnit, error, rec, "spectralunit");
  Bool refframechg = readOptionRecord(frequency_system, error, rec, "axislabelfrequencysystem");
  if(vtchg || spchg || refframechg ) {
	setSpectralFormatting(itsPosTrackCoordSys, itsDoppler, itsSpectralUnit, frequency_system);
	setSpectralFormatting(itsCoordSys, itsDoppler, itsSpectralUnit, frequency_system);
  }

//
  String value;
  if (readOptionRecord(value, error, rec,  "absrel")) {
     itsAbsolute = (value=="absolute");
  }
  if (readOptionRecord(value, error, rec,  "fracpix")) {
     itsFractionalPixels = (value=="fractional");
  }
  if (readOptionRecord(value, error, rec,  "spectralnotation")) {
      if(value == "Fixed") itsNotation = Coordinate::FIXED;
      else if(value == "Scientific") itsNotation = Coordinate::SCIENTIFIC;
      else itsNotation = Coordinate::MIXED;
  }
//
  Bool localchange = False;

// distribute options to all the axis labellers...

  Record updatedOptions;
  localchange = (localchange || setLabellerOptions(rec, updatedOptions));

  
  // Deal with changes to display/animator axes or to slice positions
  // on 'hidden axis' sliders.
  
  Bool axischange = False,   sliderchange = False;
  
  axischange = readOptionRecord(itsOptionsXAxis, error, rec,
                                     "xaxis") || axischange;
  axischange = readOptionRecord(itsOptionsYAxis, error, rec,
                                     "yaxis") || axischange;
  Bool zaxischange = readOptionRecord(itsOptionsZAxis, error, rec,
                                     "zaxis");
  axischange = zaxischange || axischange;

  
  if (axischange) itsFixedPosition = 0;
    // Axes have changed: instead of retrieving axis position values
    // from 'hidden axis' sliders, reset positions to 0 on each axis.
    
    // (dk: I don't really like this reset, and it probably isn't necessary;
    //  I don't have time to check thoroughly.  If we just left it alone,
    //  I think it would better keep track of the slice we really want to
    //  be on).
  
  else {

    // Set hidden axis positions from sliders

    for (uInt loc = 3; loc < nPixelAxes; loc++) {
      Record haxis;
      char dlformatchr[12];
      sprintf(dlformatchr, "haxis%d", loc - 2);
      if (rec.isDefined(dlformatchr) &&	rec.dataType(dlformatchr) == TpInt) {
	Int temp;
	rec.get(dlformatchr, temp);
	temp-=uiBase(); // one->zero offset -- see uiBase().
	Int axis = itsTransPixelAxes(loc);
	if (itsFixedPosition(axis) != temp) {
	  itsFixedPosition(axis) = temp;
	  sliderchange = True;
	}
      }
    }
  }

  
  if (axischange || sliderchange) {
  
    // update [the poorly-documented, tangled mess of] internal data, in
    // response to change in axis transposition or 'hidden axis' slice
    // positions.
  
    uInt xAxis, yAxis, zAxis;
    xAxis = yAxis = zAxis = 0;
    const Vector<String>& pixelAxisNames =
    			  worldToPixelAxisNames(itsOrigCoordSys);

    for (uInt axis = 0; axis < nPixelAxes; axis++) {
      if (pixelAxisNames(axis) == itsOptionsXAxis) {
	xAxis = axis;
      } 
      if (pixelAxisNames(axis) == itsOptionsYAxis) {
	yAxis = axis;
      } 
      if (pixelAxisNames(axis) == itsOptionsZAxis) {
	zAxis = axis;
      }
    }
    setAxes(xAxis, yAxis, zAxis, itsFixedPosition, axischange);
	// (setAxes must be called even if axischange=F, if
	// slider positions change....  In that case, we _don't_
	// want to reset zoom state or animator position....
	// Code for changing slider position should really be
	// separated from code to swap axes, instead of this).
  }

  if(axischange && !recOut.isDefined("setanimator")) {
    
    // Axis change means there may be a change to number of animation
    // frames.  Sending out even an empty "setanimator" field will
    // cause the animator to poll DDs for correct number of frames.
    
    Record setanimrec;
    
    if(isCSmaster() && zaxischange)  setanimrec.define("zindex",0);
	// Only CS master is allowed to reset the current frame number.
	// (As with position sliders, above, this code could be improved
	// to better remember what frame we really want to be on....)
    
    recOut.defineRecord("setanimator",setanimrec);  }
	// Note: this is not a change to an autogui parameter,
	// but a request to update the number of animator frames.

  if (axischange) {
    
    // send out (new) values, names, ranges, etc.
    // to the axis position sliders.

    const Vector<String>& pixelAxisNames =
    			  worldToPixelAxisNames(itsOrigCoordSys);
    for (uInt loc = 3; loc < nPixelAxes; loc++) {
      Int axis = itsTransPixelAxes(loc);
      Record haxis;
      char dlformatchr[12];
      sprintf(dlformatchr, "haxis%d", loc - 2);
      haxis.define("dlformat", dlformatchr);
      haxis.define("listname", pixelAxisNames(axis));
      haxis.define("ptype", "intrange");
      haxis.define("pmin", uiBase());
      haxis.define("pmax", (Int)dataShape()(axis) - 1 + uiBase());
      haxis.define("default", uiBase());
      haxis.define("value", (Int)itsFixedPosition(axis) + uiBase());
	// see uiBase() for information about slider position
	// numbering from either 0 or 1.
      haxis.define("allowunset", False);
      haxis.define("context", "Hidden_axes");
      recOut.defineRecord(dlformatchr, haxis);
    }
  }

  localchange = (localchange || axischange || sliderchange);

  localchange = (readOptionRecord(itsOptionsAspect, error, rec, 
					"aspect") || localchange);
  localchange = (readOptionRecord(itsOptionsPixelTreatment, error, rec,
					"pixeltreatment") || localchange);

  ret =  (ret || localchange);
  return ret;
};


Record PrincipalAxesDD::getOptions()
{
  Record rec = DisplayData::getOptions();
//

  const uInt nPixelAxes = itsCoordSys.nPixelAxes();
  const Vector<String>& pixelAxisNames = worldToPixelAxisNames (itsCoordSys);
//
  Record xaxis;
  xaxis.define("dlformat", "xaxis");
  xaxis.define("listname", "X-axis");
  xaxis.define("ptype", "choice");
  xaxis.define("popt", pixelAxisNames);
  xaxis.define("default", pixelAxisNames(0));
  xaxis.define("value", itsOptionsXAxis);
  xaxis.define("allowunset", False);
  xaxis.define("context", "Display_axes");
  xaxis.define("dependency_group", "axes");
  xaxis.define("dependency_type", "exclusive");
  xaxis.define("dependency_list", "yaxis zaxis");
  rec.defineRecord("xaxis", xaxis);
//
  Record yaxis;
  yaxis.define("dlformat", "yaxis");
  yaxis.define("listname", "Y-axis");
  yaxis.define("ptype", "choice");
  yaxis.define("popt", pixelAxisNames);
  yaxis.define("default", pixelAxisNames(1));
  yaxis.define("value", itsOptionsYAxis);
  yaxis.define("allowunset", False);
  yaxis.define("context", "Display_axes");
  yaxis.define("dependency_group", "axes");
  yaxis.define("dependency_type", "exclusive");
  yaxis.define("dependency_list", "xaxis zaxis");
  rec.defineRecord("yaxis", yaxis);
//
  if (nPixelAxes > 2) {
//
    Record zaxis;
    zaxis.define("dlformat", "zaxis");
    zaxis.define("listname", "Z-axis");
    zaxis.define("ptype", "choice");
    zaxis.define("popt", pixelAxisNames);
    zaxis.define("default", pixelAxisNames(2));
    zaxis.define("value", itsOptionsZAxis);
    zaxis.define("allowunset", False);
    zaxis.define("context", "Display_axes");
    zaxis.define("dependency_group", "axes");
    zaxis.define("dependency_type", "exclusive");
    zaxis.define("dependency_list", "xaxis yaxis");
    rec.defineRecord("zaxis", zaxis);
  }
//
  for (uInt loc = 3; loc < nPixelAxes; loc++) {
    Int axis = itsTransPixelAxes(loc);
    Record haxis;
    char dlformatchr[12];
    sprintf(dlformatchr, "haxis%d", loc - 2);
    haxis.define("dlformat", dlformatchr);
    haxis.define("listname", pixelAxisNames(loc));
    haxis.define("ptype", "intrange");
    haxis.define("pmin", uiBase());
    haxis.define("pmax", (Int)dataShape()(axis) - 1 + uiBase());
    haxis.define("default", uiBase());
    haxis.define("value", (Int)itsFixedPosition(axis) + uiBase());
	// see uiBase() for information about slider position
	// numbering from either 0 or 1.
    haxis.define("allowunset", False);
    haxis.define("context", "Hidden_axes");
    rec.defineRecord(dlformatchr, haxis);
  }
//
  if (itsUsesAxisLabels) {
    Record labelrec = getLabellerOptions();
    rec.merge(labelrec);
  }
  return rec;
}

Record PrincipalAxesDD::getLabellerOptions(){

  Record rec;
//
  Record aspect;
  aspect.define("dlformat", "aspect");
  aspect.define("listname", "Aspect ratio");
  aspect.define("ptype", "choice");
  Vector<String> vaspect(3);
  vaspect(0) = "fixed lattice";
  vaspect(1) = "fixed world";
  vaspect(2) = "flexible";
  aspect.define("popt", vaspect);
  aspect.define("default", vaspect(1));
  aspect.define("value", itsOptionsAspect);
  aspect.define("allowunset", False);
  rec.defineRecord("aspect", aspect);
//
  Record pixeltreatment;
  pixeltreatment.define("dlformat", "pixeltreatment");
  pixeltreatment.define("listname", "Pixel treatment");
  pixeltreatment.define("ptype", "choice");
  Vector<String> vpixeltreatment(2);
  vpixeltreatment(0) = "center";
  vpixeltreatment(1) = "edge";
  pixeltreatment.define("popt", vpixeltreatment);
  pixeltreatment.define("default", "center");
  pixeltreatment.define("value", itsOptionsPixelTreatment);
  pixeltreatment.define("allowunset", False);
  rec.defineRecord("pixeltreatment", pixeltreatment);
//
  Record absrel;
  absrel.define("context", "Position_tracking");
  absrel.define("dlformat", "absrel");
  absrel.define("listname", "Absolute or relative");   
  absrel.define("ptype", "choice");
  Vector<String> v(2);
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
  rec.defineRecord("absrel", absrel);
//
  Record fracpix;
  fracpix.define("context", "Position_tracking");
  fracpix.define("dlformat", "fracpix");
  fracpix.define("listname", "Fractional or integral pixel coordinates");   
  fracpix.define("ptype", "choice");
  v.resize(2);
  v(0) = "fractional";
  v(1) = "integral";
  fracpix.define("popt", v);
  fracpix.define("default", "integral");
  if (itsFractionalPixels) {
     fracpix.define("value", "fractional");
  } else {
     fracpix.define("value", "integral");
  }
  fracpix.define("allowunset", False);
  rec.defineRecord("fracpix", fracpix);
//
  Int after = -1;
  Int iS = itsOrigCoordSys.findCoordinate(Coordinate::SPECTRAL, after);
  if (iS>=0) {
     Record spectralunit;
     spectralunit.define("context", "Position_tracking");
     spectralunit.define("dlformat", "spectralunit");
     spectralunit.define("listname", "Spectral unit");   
     spectralunit.define("ptype", "userchoice");
//
     Vector<String> vunits;
     if (canHaveVelocityUnit (itsOrigCoordSys)) {
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
     rec.defineRecord("spectralunit", spectralunit);
//
     Record veltype;
     veltype.define("context", "Position_tracking");
     veltype.define("dlformat", "velocitytype");
     veltype.define("listname", "Velocity type");   
     veltype.define("ptype", "choice");
     vunits.resize(3);
     vunits(0) = "optical";
     vunits(1) = "radio";
     vunits(2) = "true";
     veltype.define("popt", vunits);
     veltype.define("default", "radio");
     veltype.define("value", itsDoppler);
     veltype.define("allowunset", False);
     rec.defineRecord("velocitytype", veltype);
//
     Record spectralnotation;
     spectralnotation.define("context", "Position_tracking");
     spectralnotation.define("dlformat", "spectralnotation");
     spectralnotation.define("listname", "Numeric notation");   
     spectralnotation.define("ptype", "choice");
     Vector<String> choices(3);
     choices(0) = "Flexible";
     choices(1) = "Scientific";
     choices(2) = "Fixed";
     spectralnotation.define("popt", choices);
     spectralnotation.define("default", "Flexible");
     spectralnotation.define("value", "Flexible");     
     spectralnotation.define("allowunset", False);
     rec.defineRecord("spectralnotation", spectralnotation);

  }
  if (itsUsesAxisLabels) {
    Record axisrec = ((WCCSNLAxisLabeller *)itsAxisLabellers[0])->getOptions();
  
    rec.merge(axisrec);
  }

  return rec;
}

void PrincipalAxesDD::setSpectralFormatting  (CoordinateSystem& cSys,
                                              const String &doppler,
                                              const String &unit,
					      const String &freq_sys)
{
   static LogIO os(LogOrigin("PrincipleAxesDD", "setSpectralFormatting", WHERE));
   String errorMsg;
   if( doppler.length() > 0 && unit.length( ) > 0 &&
       ! CoordinateUtil::setSpectralFormatting(errorMsg, cSys, unit, doppler) ) {
      os << LogIO::WARN 
         << "Failed to update SpectralCoordinate formatting because"
         << errorMsg << LogIO::POST;
   }
   if( freq_sys.length( ) > 0 && ! CoordinateUtil::setSpectralConversion(errorMsg, cSys, freq_sys) ) {
      os << LogIO::WARN
         << "Failed to update SpectralCoordinate reference frame because"
         << errorMsg << LogIO::POST;
   }
}


void PrincipalAxesDD::setVelocityState (CoordinateSystem& cSys,
                                        const String& doppler,
                                        const String& unit)
{
   static LogIO os(LogOrigin("PrincipleAxesDD", "setSpectralState", WHERE));
   String errorMsg;
   if (!CoordinateUtil::setVelocityState (errorMsg, cSys, unit, doppler)) {
      os << LogIO::WARN << "Failed to update SpectralCoordinate velocity state because" << LogIO::POST;
      os << errorMsg << LogIO::POST;
   }
}

void PrincipalAxesDD::removePixelAxes (CoordinateSystem& cSys,
                                       uInt startAxis, 
                                       const IPosition& fixedPosition)
//
// The CS is transposed.  
// fixedPosition is not tranposed.
// startAxis refers to the transposed CS
//
{
  const Int nRemove = cSys.nPixelAxes() - startAxis;
  if (nRemove > 0) {
     Vector<Int> removeAxes(nRemove);
     Vector<Double> replacementValues(nRemove);
     for (Int i=0; i<nRemove; i++) {
        uInt j = startAxis + i;
        uInt k = itsTransPixelAxes(j);
//
        removeAxes(i) = j;
        replacementValues(i) = fixedPosition(k);  
     }
     CoordinateUtil::removePixelAxes(cSys, replacementValues, 
                                     removeAxes, True);
  }
}


Bool PrincipalAxesDD::canHaveVelocityUnit (const CoordinateSystem& cSys) const
{
  Int after = -1;
  Int iS = cSys.findCoordinate(Coordinate::SPECTRAL, after);
  if (iS>=0) {
     const SpectralCoordinate& sc = cSys.spectralCoordinate(iS);
     if (sc.restFrequency() > 0) return True;
  }
  return False;
}


Bool PrincipalAxesDD::conformsToCS(const WorldCanvasHolder& wch) {
  // Is the DD is capable (in its current state) of drawing
  // in the current CoordinateSystem of the WCH's WorldCanvas?
  // This implementation just checks for matching axis codes.
  //
  // This forms a part of the checks requested by DD::conformsTo(wch),
  // which also checks compatibility of the DD with any zIndex or other
  // WCH restrictions, and serves (via setActiveZIndex_()) to inform the
  // DD of the passed WCH's current zIndex value.

  WorldCanvas *wc = wch.worldCanvas();
  String xAxis = "xaxiscode (required match)",
         yAxis = "yaxiscode (required match)";
  String xcode, ycode;
  
  return csConformed_ =
	  (!wc->getAttributeValue(xAxis, xcode) || xcode==worldAxisCode(0))
       && (!wc->getAttributeValue(yAxis, ycode) || ycode==worldAxisCode(1));
}

  
void PrincipalAxesDD::notifyUnregister(WorldCanvasHolder &wcHolder,
                                       Bool ignoreRefresh) {
  // disconnected from wc--be sure drawlists it may be holding for us
  // are deleted.
  cleanup();
  if (itsUsesAxisLabels){
    for (uInt i = 0; i < itsNumImages; i++) {
      static_cast<WCCSNLAxisLabeller*>(itsAxisLabellers[i])->invalidate();  
    }
  } 
  DisplayData::notifyUnregister(wcHolder, ignoreRefresh);
}


PrincipalAxesDD::PrincipalAxesDD()
: DisplayData() 
{}

PrincipalAxesDD::PrincipalAxesDD(const PrincipalAxesDD &) 
{}

void PrincipalAxesDD::operator=(const PrincipalAxesDD &) 
{}


Vector<String> PrincipalAxesDD::worldToPixelAxisNames (const CoordinateSystem& cSys) const
{
//
// Every pixel axs must have a world axis, so don't check for removal
//
   const Vector<String>& names = cSys.worldAxisNames();
   Vector<String> tmp(cSys.nPixelAxes());
   for (uInt pixelAxis=0; pixelAxis<cSys.nPixelAxes(); pixelAxis++) {
      Int worldAxis = cSys.pixelAxisToWorldAxis(pixelAxis);
      tmp(pixelAxis) = names(worldAxis);
   }
   return tmp;
}

} //# NAMESPACE CASA - END

