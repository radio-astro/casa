//# DDDEllipse.cc: implementation of ellipse DDD
//# Copyright (C) 1999,2000,2001,2002,2003
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

#include <display/DisplayDatas/DDDEllipse.h>

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/BasicMath/Math.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageUtilities.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayDatas/DDDHandle.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DDDEllipse::DDDEllipse(const Record &description, DrawingDisplayData *owner) 
: DDDObject(description, owner),
 itsOutline(True),
 itsMode(DDDObject::None),
 itsLogger(LogOrigin("DDDEllipse", "DDDEllipse(...)")),
 itsRectangle(False),
 itsFracX(False),
 itsFracY(False),
 itsDoRef(False)
{
  if (!(description.isDefined("center") && 
	description.isDefined("positionangle") &&
	description.isDefined("major") &&
	description.isDefined("minor"))) {
    throw(AipsError("Invalid record for constructing DDDEllipse"));
  }

// Decode parameters

  itsWorldParameters.resize(5);
  try {
     decode(description, True);
  } catch (AipsError x) {
     itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
  }  

// Initialize variable for parameters in pixel units.  These
// are not filled in until convertToPixel is called. 

  itsPixelCenter.resize(2);
  itsPixelShape.resize(3);

// Initialize points and handles variables

  itsHandles.resize(4);
  itsCorners.resize(4,2);
  itsPixelOffsets.resize(4,2);
}


DDDEllipse::~DDDEllipse()
{}

void DDDEllipse::draw(const Display::RefreshReason &reason,
                      WorldCanvas* pWorldCanvas) {

  // dk note:  Most of the tests below should be more general and on higher
  // levels (PassiveCachingDisplayData, DrawingDisplayData, or DDDObject).
  // The idea is to assure that the WC CS and other WC state has indeed
  // been set, and is appropriate to the world units of the ellipse.
  // Apparently Annotations/DisplayShape is intended to supercede all
  // this DDD code, although it's still used to draw regions, components
  // and beams.  Hopefully DDD code will not see more use (it just
  // doesn't have the generality to do the job right).  These patches
  // are intended simply to make beam drawing work better for now,
  // in most cases.  Hopefully, even that will be redone in future using
  // the DisplayShape code.

  if(pWorldCanvas==0) return;
  itsWorldCanvasPtr = pWorldCanvas;
  if(&(itsWorldCanvasPtr->coordinateSystem())==0) return;
	// (failure to check this was causing crashes....  dk)
  if (itsWorldCanvasPtr->coordinateSystem().nPixelAxes()<2u) return;

  itsCoordinateSystem = itsWorldCanvasPtr->coordinateSystem();

  Unit umaj = itsWorldParameters(2).getFullUnit(),
       umin = itsWorldParameters(3).getFullUnit();

  if(umaj==Unit("deg") && umin==Unit("deg")) {
	// Isolating the beam ellipse case as much as possible, to
	// minimize impact on use in regions--no reason in principle
	// to do this.

    // If axis codes weren't set, no 'active' DD took care of
    // 'size control' on the WC (so don't try to draw.).

    String xAxis = "xaxiscode (required match)",
           yAxis = "yaxiscode (required match)";
    if (!itsWorldCanvasPtr->existsAttribute(xAxis) ||
        !itsWorldCanvasPtr->existsAttribute(yAxis)) return;

    // Check that Direction coordinates are on canvas.  (Still draws
    // incorrectly in the oddball case of Dec on X, RA of Y...).

    for(uInt ax=0; ax<2; ax++) {
      Int crdno, axincrd;
      itsCoordinateSystem.findWorldAxis(crdno, axincrd, ax);
      if(crdno<0 || axincrd<0 ||
         itsCoordinateSystem.type(crdno)!=Coordinate::DIRECTION) return;
    }
  }



  try {

// Convert so maps screen pixels to world

     convertCoordinateSystem (itsCoordinateSystem, itsWorldCanvasPtr);

// Convert shape to screen pixels for drawing

    convertToPixel();
  } catch (AipsError x) {
    itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
    cerr << x.getMesg() << endl;
    return;
  }


  PixelCanvas *pCanvas = itsWorldCanvasPtr->pixelCanvas();
  pCanvas->setColor(color());
  pCanvas->setLineWidth(lineWidth());


// Draw shape

  if (itsRectangle) {
     if (itsOutline) {
        pCanvas->drawPolygon(itsCorners);
     } else {
        static Vector<Float> x1(5), y1(5);
        for (uInt i=0; i<4; i++) {
           x1[i] = itsCorners(i,0);
           y1[i] = itsCorners(i,1);
        }
//
        x1[4] = x1[0];
        y1[4] = y1[0];
//
        pCanvas->drawFilledPolygon(x1, y1);
     }
  } else {

// Drawing routine wants half-width

     pCanvas->drawEllipse(itsPixelCenter(0), itsPixelCenter(1), itsPixelShape(0)/2.0, 
                          itsPixelShape(1)/2.0, itsPixelShape(2)/C::degree, itsOutline);
  }

// Show handles as needed

  if (showingHandles()) {

    DDDHandle::Size handlesize = DDDHandle::NONE;
//
    if (itsPixelShape(0)>=25.0 && itsPixelShape(1)>=25.0) { 
      handlesize = DDDHandle::LARGE;
    } else if (itsPixelShape(0)>=12.0 && itsPixelShape(1)>=12.0) { 
      handlesize = DDDHandle::MEDIUM;
    } else if (itsPixelShape(0)>= 2.0 && itsPixelShape(1)>= 2.0) { 
      handlesize = DDDHandle::SMALL;
    }
    if (handlesize != DDDHandle::NONE) {
       for (uInt i=0; i< itsHandles.nelements(); i++) {
          itsHandles[i].resize(handlesize);
          pCanvas->drawFilledRectangle((Int)(itsHandles[i].blcX()+0.5),
                                       (Int)(itsHandles[i].blcY()+0.5),
                                       (Int)(itsHandles[i].trcX()+0.5),
                                       (Int)(itsHandles[i].trcY()+0.5));    
      }
    } 
  }
}



void DDDEllipse::operator()(const WCRefreshEvent& ev)
{
/*
// This stuff is handled in the draw() routine.
// This routine could now be eliminated. (dk)

   // These 'reasons' don't work worth a damn, for several reasons... (dk)
   if (ev.reason()==Display::LinearCoordinateChange ||
       ev.reason()==Display::PixelCoordinateChange) {

// This means a zoom or a resize.  Get CoordinateSystem

      // (dk note: assuming WC CS existed was causing crashes...)
      if(&(ev.worldCanvas()->coordinateSystem())==0) return;

      itsCoordinateSystem = ev.worldCanvas()->coordinateSystem();

// Convert to map from screen pixels to world and recompute
// shape in screen pixels

      convertCoordinateSystem(itsCoordinateSystem, itsWorldCanvasPtr);
      convertToPixel();
   }
*/
}



void DDDEllipse::operator()(const WCPositionEvent &ev) 
{
  if ((ev.key() != owner()->key()) || 

// new phrase in expression on next line...
//!(ev.modifiers() & owner()->keyModifier())) {

      False) {
    return;
  }

// These are in screen pixel coordinates

  Double x = (Double)ev.pixX();
  Double y = (Double)ev.pixY();
//
  if (showingHandles()) {    
    if (isFixed()) showHandles(False);
//
    if (ev.keystate()) {
       Bool onH = onHandle(itsHandles, x,y);
       if (!inPolygon(itsCorners,x,y) && !onH) {

// press button outside object, switch to handles invisible

          clearClickBuffer();
          showHandles(False);
       } else {

// press button inside object

          if (!isFixed()) {
             if (!storeClick(ev)) {	
                if (onH) {
                   if (isEditable()) itsMode = DDDObject::Handle;
                } else if (ev.modifiers() == Display::KM_Shift) {
    
// shift+MB1 held down -> rotate

                   if (isEditable()) itsMode = DDDObject::Rotate;
                } else {	  

// only MB1 -> move

                   if (isMovable()) {
                     itsMode = DDDObject::Move;
                     itsBaseMoveX = x;
                     itsBaseMoveY = y;
                   }
                }
             }
          }
       }
       return;
    }
  } else {
    if (inPolygon(itsCorners,x,y)) {
      if (!isFixed()) {
         showHandles(True);
         clearClickBuffer();
         itsMode = DDDObject::None;
         return;
      } 
    }
  }

// Update the world values of the object from the current
// screen pixel values when the MB is released from a Rotation.
// This is because we have deferred the world update until
// now for speed purposes.

  if (itsMode==DDDObject::Rotate && !ev.keystate()) {        // Release
     try {

// Update world values  and recompute in screen pixel

        updateWorldValues();
        convertToPixel();
     } catch (AipsError x) {
        cerr << x.getMesg();
        itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
     }
  }
}

void DDDEllipse::operator()(const WCMotionEvent &ev)
{
  if (isFixed()) return;
//
  if (!(ev.modifiers() & owner()->keyModifier()))   return;

// Pixel coordinates

  Double x = ev.pixX();
  Double y = ev.pixY();
//
  switch (itsMode) {
  case DDDObject::Move:                            // Translate
    {

// ev.worldCanvas()->pixelCanvas()->setCursor(52);

      Double dx = Double(x - itsBaseMoveX);
      Double dy = Double(y - itsBaseMoveY);
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      translateMatrix(itsCorners,dx,dy);
      itsPixelCenter(0) += dx;
      itsPixelCenter(1) += dy;
//
      for (uInt i=0; i< itsHandles.nelements();i++) {
	itsHandles[i].move(dx,dy);
      }

// Update the world center only. When next we recompute
// the pixel parameters they will reflect the new location
// Thus, as the shape roams around the image its position
// angle (in world coordinates) will change to reflect its
// new position (e.g. for DirectionCoordinate).

      updateWorldValues();
      convertToPixel();
//
      owner()->refresh();
      break;
    }
  case DDDObject::Handle:                           // Reshape
    {

// ev.worldCanvas()->pixelCanvas()->setCursor(120);

      x -= itsPixelCenter(0);
      y -= itsPixelCenter(1);
      Double dx = fabs(x*cos(-itsPixelShape(2))-y*sin(-itsPixelShape(2)));
      Double dy = fabs(x*sin(-itsPixelShape(2))+y*cos(-itsPixelShape(2)));
//      
      itsPixelShape(0) = 2*dx;
      if ( (ev.modifiers() & Display::KM_Ctrl) == 0 ) {
	itsPixelShape(1) = 2*dy;
      } else {
	itsPixelShape(1) = itsPixelShape(0);
      }
//
      createCorners();
      for (uInt i=0; i< itsHandles.nelements();i++) {
	itsHandles[i].createHandle(itsCorners.row(i));
      }
//
      owner()->refresh();
      break;
    }
  case DDDObject::Rotate:                            // Rotate
    {

// ev.worldCanvas()->pixelCanvas()->setCursor(50);

      Double dphi = atan2(y-itsPixelCenter(1), x-itsPixelCenter(0))-itsPixelShape(2);	
      itsPixelShape(2) += dphi;
//
      translateMatrix(itsCorners,-itsPixelCenter(0),-itsPixelCenter(1));
      itsCorners = rotateMatrix(itsCorners, dphi);
      translateMatrix(itsCorners, itsPixelCenter(0), itsPixelCenter(1));      
      for (uInt i=0; i< itsHandles.nelements();i++) {
	itsHandles[i].createHandle(itsCorners.row(i));
      }
//
      owner()->refresh();
      break;
    }

  default:
    {
      break;
    }
  }
}

Record DDDEllipse::description()  
{
  Record rec = DDDObject::description();
  encode(rec);
  return rec;
} 

void DDDEllipse::setDescription(const Record& description)  
{

  DDDObject::setDescription(description);
  decode(description, False);
  if (itsCoordinateSystem.nCoordinates()>0) convertToPixel();
  if (isFixed()) showHandles(False);
  owner()->refresh();
}


// Protected/private


void DDDEllipse::convertToPixel ()
{
// Center 

  const Vector<String>& units = itsCoordinateSystem.worldAxisUnits();

  String ux = itsWorldParameters(0).getUnit();
  String uy = itsWorldParameters(1).getUnit();
//
  Vector<Double> worldCen(itsCoordinateSystem.referenceValue().copy());
  worldCen.resize(min(3, Int(itsCoordinateSystem.nWorldAxes())), True);
//
  if (!itsFracX) {
     worldCen(0) = itsWorldParameters(0).getValue(units(0));
  }
  if (!itsFracY) {
     worldCen(1) = itsWorldParameters(1).getValue(units(1));
  }
//
  itsWorldCanvasPtr->worldToPix(itsPixelCenter, worldCen);
//
  if (itsFracX) {
     Float d = Float(itsWorldCanvasPtr->canvasDrawXSize());
     Float off = Float(itsWorldCanvasPtr->canvasDrawXOffset()) + 
                 Float(itsWorldCanvasPtr->canvasXOffset());
     itsPixelCenter(0) = off + d*itsWorldParameters(0).getValue();
  }
//
  if (itsFracY) {
     Float d = Float(itsWorldCanvasPtr->canvasDrawYSize());
     Float off = Float(itsWorldCanvasPtr->canvasDrawYOffset()) + 
                 Float(itsWorldCanvasPtr->canvasYOffset());
     itsPixelCenter(1) = off + d*itsWorldParameters(1).getValue();
  }

// Convert shape parameters to pixel (pa in rad)

  static IPosition pixelAxes(2, 0, 1);
  if (itsFracX || itsFracY) {

// Ho hum.  The shape converters need the world position.
// So figure it out from the current pixel center 

     static Vector<Quantum<Double> > tWorldParameters(5);
     static Vector<Double> tWorld;
//
     itsWorldCanvasPtr->pixToWorld(tWorld, itsPixelCenter);
//
     tWorldParameters = itsWorldParameters;
     tWorldParameters(0).setValue(tWorld(0));
     tWorldParameters(0).setUnit(units(0));
//
     tWorldParameters(1).setValue(tWorld(1));
     tWorldParameters(1).setUnit(units(1));
//
     ImageUtilities::worldWidthsToPixel (itsLogger, itsPixelShape, tWorldParameters,
                                         itsCoordinateSystem, pixelAxes, itsDoRef);
  } else {
     ImageUtilities::worldWidthsToPixel (itsLogger, itsPixelShape, itsWorldParameters, 
                                         itsCoordinateSystem, pixelAxes, itsDoRef);
  }

// Create corners of bounding box

   createCorners(); 

// Create handles 

  for (uInt i=0; i< itsHandles.nelements();i++) {
     itsHandles[i].createHandle(itsCorners.row(i));
  }
}


void DDDEllipse::updateWorldValues ()
{

// Center

   static Vector<Double> world;
   static Vector<Double> pixel(2);
//
   pixel(0) = itsPixelCenter(0);
   pixel(1) = itsPixelCenter(1);
   itsWorldCanvasPtr->pixToWorld(world, pixel);

// For fractional center units, when we let go of a move
// we must update the world center as a fraction.  Otherwise, e.g. letting
// go of a zoom, we don't update the center.

   const Vector<String>& units = itsCoordinateSystem.worldAxisUnits();
   if (itsFracX) {
      if (itsMode==DDDObject::Move) {
         Float d = Float(itsWorldCanvasPtr->canvasDrawXSize());
         Float off = Float(itsWorldCanvasPtr->canvasDrawXOffset()) + 
                     Float(itsWorldCanvasPtr->canvasXOffset());
         itsWorldParameters(0).setValue((itsPixelCenter(0)-off)/d);
      }
   } else {
      itsWorldParameters(0).setValue(world(0));
      itsWorldParameters(0).setUnit(Unit(units(0)));
   }
//
   if (itsFracY) {
      if (itsMode==DDDObject::Move) {
         Float d = Float(itsWorldCanvasPtr->canvasDrawYSize());
         Float off = Float(itsWorldCanvasPtr->canvasDrawYOffset()) + 
                     Float(itsWorldCanvasPtr->canvasYOffset());
         itsWorldParameters(1).setValue((itsPixelCenter(1)-off)/d);
      }
   } else {
      itsWorldParameters(1).setValue(world(1));
      itsWorldParameters(1).setUnit(Unit(units(1)));
   }

// Convert shape parameters to world 

   static Vector<Quantum<Double> > wParameters(3);
   static IPosition pixelAxes(2, 0, 1);
   static Vector<Double> pParameters(5);
//
   pParameters[0] = itsPixelCenter[0];
   pParameters[1] = itsPixelCenter[1];
   for (uInt i=0; i<3; i++) pParameters[i+2] = itsPixelShape[i];
   ImageUtilities::pixelWidthsToWorld (itsLogger, wParameters, pParameters, 
                                       itsCoordinateSystem, pixelAxes, itsDoRef);
   itsWorldParameters(2) = wParameters(0);
   itsWorldParameters(3) = wParameters(1);
   itsWorldParameters(4) = wParameters(2);
}

void DDDEllipse::decode(const RecordInterface& description, Bool required)
{
  String error;
  String pix("pix");
  String frac("frac");
  String unit;

// Get center

  if (required || description.isDefined("center")) {
     const RecordInterface& subRecord = description.asRecord("center");
     if (subRecord.nfields()!=2) {
        throw (AipsError ("DDDEllipse - must have two values for center"));
     }
//
     QuantumHolder h;
     for (uInt i=0; i<2; i++) {
        if (!h.fromRecord(error, subRecord.asRecord(i))) {
           throw (AipsError ("DDDEllipse - could not extract ellipse center because "+error));
        }
        itsWorldParameters(i) = h.asQuantumDouble();
     }
//
     String xUnit = itsWorldParameters(0).getFullUnit().getName();
     String yUnit = itsWorldParameters(1).getFullUnit().getName();
     if (xUnit==pix || yUnit==pix) {
        throw (AipsError ("DDDEllipse - 'pix' units not allowed for center"));
     }
//
     itsFracX = (xUnit==frac);
     itsFracY = (yUnit==frac);
  }

// Get major

  if (required || description.isDefined("major")) {
     QuantumHolder h;
     const RecordInterface& subRecord = description.asRecord("major");
     if (!h.fromRecord(error, subRecord)) {
        throw (AipsError ("DDDEllipse - could not extract ellipse major axis because "+error));
     }
     itsWorldParameters(2) = h.asQuantumDouble();
//
     unit= itsWorldParameters(2).getFullUnit().getName();
     if (unit==frac) {
        throw (AipsError ("DDDEllipse - 'frac' units not allowed for major"));
     }
  }

// Get minor

  if (required || description.isDefined("minor")) { 
     QuantumHolder h;
     const RecordInterface& subRecord = description.asRecord("minor");
     if (!h.fromRecord(error, subRecord)) {
        throw (AipsError ("DDDEllipse - could not extract ellipse minor axis because "+error));
     }
     itsWorldParameters(3) = h.asQuantumDouble();
//
     unit= itsWorldParameters(3).getFullUnit().getName();
     if (unit==frac) {
        throw (AipsError ("DDDEllipse - 'frac' units not allowed for minor"));
     }
  }

// Get position angle

  if (required || description.isDefined("positionangle")) { 
     QuantumHolder h;
     const RecordInterface& subRecord = description.asRecord("positionangle");
     if (!h.fromRecord(error, subRecord)) {
        throw (AipsError ("DDDEllipse - could not extract position angle because "+error));
     }
     itsWorldParameters(4) = h.asQuantumDouble();
//
     if (!itsWorldParameters(4).isConform(Unit("rad"))) {
        throw (AipsError ("DDDEllipse - units for positionangle must be angular"));
     }
  }
//
  if (description.isDefined("outline"))  {
     description.get("outline", itsOutline);
  }
//
  if (description.isDefined("rectangle"))  {
     description.get("rectangle", itsRectangle);
  }
//
  if (description.isDefined("doreference"))  {
     description.get("doreference", itsDoRef);
  }
}


void DDDEllipse::encode (RecordInterface& description) const
{
   String error;

   description.define ("type", String("ellipse"));


// Center

   {
      Record rec0, rec1;
      QuantumHolder qh0(itsWorldParameters(0));
      QuantumHolder qh1(itsWorldParameters(1));
      if (!qh0.toRecord(error, rec0)) {
         throw(AipsError(error));
      }
      if (!qh1.toRecord(error, rec1)) {
         throw(AipsError(error));
      }
      Record rec;
      rec.defineRecord(0, rec0);
      rec.defineRecord(1, rec1); 
      description.defineRecord("center", rec);
   }

// Major

   {
      Record rec;
      QuantumHolder qh(itsWorldParameters(2));
      if (!qh.toRecord(error, rec)) {
         throw(AipsError(error));
      }
      description.defineRecord("major", rec);
   }

// Minor

   {
      Record rec;
      QuantumHolder qh(itsWorldParameters(3));
      if (!qh.toRecord(error, rec)) {
         throw(AipsError(error));
      }
      description.defineRecord("minor", rec);
   }

// Position angle

   {
      Record rec;
      QuantumHolder qh(itsWorldParameters(4));
      if (!qh.toRecord(error, rec)) {
         throw(AipsError(error));
      }
      description.defineRecord("positionangle", rec);
   }
//
   description.define("outline", itsOutline);
   description.define("rectangle", itsRectangle);
   description.define("doreference", itsDoRef);
}



Matrix<Int> DDDEllipse::toIntPixel(const Matrix<Double>& points) 
{
  Matrix<Int> retmat(points.nrow(),points.ncolumn());
  for (uInt i = 0; i < points.nrow(); i++) {
    for (uInt j = 0; j < points.ncolumn(); j++) {
      retmat(i,j) = (Int)(points(i,j)+0.5);
    }
  }
  return retmat;
}



void DDDEllipse::createCorners() 
{  
  itsPixelOffsets(0,0) = -itsPixelShape(0)/2.0;    // major
  itsPixelOffsets(0,1) = -itsPixelShape(1)/2.0;    // minor
//
  itsPixelOffsets(1,0) = itsPixelShape(0)/2.0;
  itsPixelOffsets(1,1) = -itsPixelShape(1)/2.0;
//
  itsPixelOffsets(2,0) = itsPixelShape(0)/2.0;
  itsPixelOffsets(2,1) = itsPixelShape(1)/2.0;
//
  itsPixelOffsets(3,0) = -itsPixelShape(0)/2.0;
  itsPixelOffsets(3,1) = itsPixelShape(1)/2.0;
//
  itsCorners = rotateMatrix(itsPixelOffsets, itsPixelShape(2));
  translateMatrix(itsCorners, itsPixelCenter(0), itsPixelCenter(1));
}


DDDEllipse::DDDEllipse() 
{}

DDDEllipse::DDDEllipse(const DDDEllipse &) 
{}

void DDDEllipse::operator=(const DDDEllipse &) 
{}


void DDDEllipse::listWorld()
{
   cerr << "World" << endl;
   cerr << "  Center = " << itsWorldParameters(0) << ", " << itsWorldParameters(1) << endl;
   cerr << "  Major, minor, pa = " << itsWorldParameters(2).getValue(Unit("arcsec")) 
        << ", " << itsWorldParameters(3).getValue(Unit("arcsec")) 
        << ", " << itsWorldParameters(4).getValue(Unit("deg")) << endl;
}

void DDDEllipse::listPixel()
{
   cerr << "Pixel" << endl;
   cerr << "  Center = " << itsPixelCenter  << endl;
   cerr << "  Major, minor, pa = " << itsPixelShape(0) << ", " << itsPixelShape(1) << ", "
        << itsPixelShape(2)/C::degree  << endl;
}

} //# NAMESPACE CASA - END

