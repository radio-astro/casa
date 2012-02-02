//# DDDPolygon.cc: implementation of DDDPolygon
//# Copyright (C) 1999,2000,2001,2002
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

#include <display/DisplayDatas/DDDPolygon.h>

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

DDDPolygon::DDDPolygon(const Record &description, DrawingDisplayData *owner) 
: DDDObject(description, owner),
 itsOutline(True),
 itsNPoints(0),
 itsMode(DDDObject::None),
 itsRecompute(True),
 itsLogger(LogOrigin("DDDPolygon", "DDDPolygon(...)")),
 itsFracX(False),
 itsFracY(False)
{
  if (!(description.isDefined("x") && 
	description.isDefined("y"))) {
    throw(AipsError("Invalid record for constructing DDDPolygon"));
  }

// Decode parameters

  try {
     decode(description, True);
  } catch (AipsError x) {
     itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
  }  

// Bounding box of polygon

  itsCorners.resize(4,2);
}

DDDPolygon::~DDDPolygon() {
}

void DDDPolygon::draw(const Display::RefreshReason &reason,
                      WorldCanvas* pWorldCanvas) 
{
  itsWorldCanvasPtr = pWorldCanvas;
//
  PixelCanvas *pCanvas = itsWorldCanvasPtr->pixelCanvas();
  pCanvas->setColor(color());
  pCanvas->setLineWidth(lineWidth());

// Get CoordinateSystem.  Is ordered with x and y first
// Catch coordinate conversion errors and just don't draw

  if (itsRecompute) {
     try {
        itsCoordinateSystem = itsWorldCanvasPtr->coordinateSystem();

// Convert CoordinateSystem to screen pixels

        convertCoordinateSystem(itsCoordinateSystem, itsWorldCanvasPtr);

// Convert object from world to pixel units

        convertToPixel();

// Create corners of bounding box of polygon (pixels)

        createCorners(); 

// Create handles, one per vertex of polygon (pixels)

        createHandles(); 
     } catch (AipsError x) {
//        itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
        cerr << x.getMesg() << endl;
        return;
     }
  }

// Draw polygon

  if (itsOutline) {
     pCanvas->drawPolygon(itsPixelX, itsPixelY);
  } else {
     pCanvas->drawFilledPolygon(itsPixelX, itsPixelY);
  }

// Show handles as needed

  if (showingHandles()) {

    DDDHandle::Size handlesize = DDDHandle::NONE;
    Double px = abs(itsCorners(3,0) - itsCorners(0,0));
    Double py = abs(itsCorners(1,1) - itsCorners(0,1));
//
    if (px>=25.0 && py>=25.0) { 
      handlesize = DDDHandle::LARGE;
    } else if (px>=12.0 && py>=12.0) { 
      handlesize = DDDHandle::MEDIUM;
    } else if (px>= 2.0 && py>= 2.0) { 
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




void DDDPolygon::operator()(const WCPositionEvent &ev) 
{
  if ((ev.key() != owner()->key()) || 

// new phrase in expression on next line...
//!(ev.modifiers() & owner()->keyModifier())) {

      False) {
    return;
  }

// These are screen in pixel coordinates

  Double x = (Double)ev.pixX();
  Double y = (Double)ev.pixY();
//
  if (showingHandles()) {    
    if (isFixed()) showHandles(False);
//
    if (!inPolygon(itsPixelX, itsPixelY, x,y) && !onHandle(itsHandles, x, y) && ev.keystate()) {

// press button outside rectangle, switch to handles invisible

      clearClickBuffer();
      showHandles(False);
    } else if (ev.keystate()) {      // Press

// inside object

      if (!isFixed()) {
         if (!storeClick(ev)) {	
            if (onHandle(itsHandles, x, y)) {
               if (isEditable()) itsMode = DDDObject::Handle;
            } else if (ev.modifiers() == Display::KM_Shift) {

// shift+MB1 held down -> rotate. Not implemented.

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
  } else {
    if (inPolygon(itsPixelX, itsPixelY, x, y)) {
      if (!isFixed()) {
         showHandles(True);
         clearClickBuffer();
         itsMode = DDDObject::None;
      } 
    }
  }

// Update the world values of the object from the current
// screen pixel values when the MB is released. This
// release might come from a zoom or rotate or move.
// Even if the object is fixed, we still need to recompute
// following a zoom event.

  itsRecompute = False;
  if (!ev.keystate()) {        // Release
     try {
        updateWorldValues();
        itsRecompute = True;
     } catch (AipsError x) {
        itsLogger << LogIO::SEVERE << x.getMesg() << LogIO::POST;
     }
  }
}

void DDDPolygon::operator()(const WCMotionEvent &ev)
{
  if (isFixed()) return;

// ev.worldCanvas()->pixelCanvas()->unsetCursor();

  Double x = ev.pixX();
  Double y = ev.pixY();

/*
  if (onHandle(itsHandles, x, y)) {
    ev.worldCanvas()->pixelCanvas()->setCursor(120);
  } else {
    ev.worldCanvas()->pixelCanvas()->unsetCursor();
  }
*/

  if (!(ev.modifiers() & owner()->keyModifier())) {
    return;
  }
//
  switch (itsMode) {
  case DDDObject::Move:
    {

// ev.worldCanvas()->pixelCanvas()->setCursor(52);

      Double dx = Double(x - itsBaseMoveX);
      Double dy = Double(y - itsBaseMoveY);
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      translateMatrix(itsCorners,dx,dy);
//
      itsPixelX += dx;
      itsPixelY += dy;
//
      for (uInt i=0; i< itsHandles.nelements();i++) {
	itsHandles[i].move(dx,dy);
      }
//
      owner()->refresh();
      break;
    }
  case DDDObject::Handle:
    {

// ev.worldCanvas()->pixelCanvas()->setCursor(120);
// Modification by stretching vertices not yet implemented.
// Not yet handled.

       break;
    }
    
  case DDDObject::Rotate:            

// Not handled

    {
      break;
    }

  default:
    {
      break;
    }
  }
}

Record DDDPolygon::description()  
{
  Record rec = DDDObject::description();
  encode(rec);
  return rec;
} 

void DDDPolygon::setDescription(const Record& description)  
{

  DDDObject::setDescription(description);

// Decode parameters

  decode(description, False);
//
  itsRecompute = True;
  if (isFixed()) showHandles(False);
  owner()->refresh();
}


// Protected/private


void DDDPolygon::convertToPixel ()
{
  const Vector<String>& units = itsCoordinateSystem.worldAxisUnits();
//
  Vector<Double> world(itsCoordinateSystem.referenceValue().copy());
  Vector<Double> pixel(2);
  world.resize(min(3, Int(itsCoordinateSystem.nWorldAxes())), True);
//
  const Vector<Double>& x = itsWorldX.getValue();  
  const Vector<Double>& y = itsWorldY.getValue();
  Quantum<Double> tx(1.0, itsWorldX.getFullUnit());
  Quantum<Double> ty(1.0, itsWorldY.getFullUnit());
//
  itsPixelX.resize(itsNPoints, False);
  itsPixelY.resize(itsNPoints, False);
//
  for (uInt i=0; i<itsNPoints; i++) {
     tx.setValue(x(i));
     ty.setValue(y(i));
//
     if (!itsFracX) {
        world(0) = tx.getValue(units(0));            // Convert to CS units
     }
     if (!itsFracY) {
        world(1) = ty.getValue(units(1));
     }
//
     itsWorldCanvasPtr->worldToPix(pixel, world);
//
     if (itsFracX) {
        Float d = Float(itsWorldCanvasPtr->canvasDrawXSize());
        Float off = Float(itsWorldCanvasPtr->canvasDrawXOffset()) + 
                    Float(itsWorldCanvasPtr->canvasXOffset());
        
        itsPixelX(i) = off + d*tx.getValue();
     } else {
        itsPixelX(i) = pixel(0);
     }
//
     if (itsFracY) {
        Float d = Float(itsWorldCanvasPtr->canvasDrawYSize());
        Float off = Float(itsWorldCanvasPtr->canvasDrawYOffset()) + 
                    Float(itsWorldCanvasPtr->canvasYOffset());
        itsPixelY(i) = off + d*ty.getValue();
     } else {
        itsPixelY(i) = pixel(1);
     }
   }
}


void DDDPolygon::updateWorldValues ()
{
   const Vector<String>& units = itsCoordinateSystem.worldAxisUnits();
   Vector<Double> x = itsWorldX.getValue(units(0));
   Vector<Double> y = itsWorldY.getValue(units(1));
//
   Vector<Double> world;
   Vector<Double> pixel(2);
   for (uInt i=0; i<itsNPoints; i++) {
      pixel(0) = itsPixelX(i);
      pixel(1) = itsPixelY(i);
      itsWorldCanvasPtr->pixToWorld(world, pixel);

// For fractional center units, when we let go of a move
// we must update the world center as a fraction.  Otherwise, e.g. letting
// go of a zoom, we don't update the center.

      if (itsFracX) {
         if (itsMode==DDDObject::Move) {
            Float d = Float(itsWorldCanvasPtr->canvasDrawXSize());
            Float off = Float(itsWorldCanvasPtr->canvasDrawXOffset()) + 
                        Float(itsWorldCanvasPtr->canvasXOffset());
            x(i) = (pixel(0)-off)/d;
         }
      } else {
         x(i) = world(0);
      }
//
      if (itsFracY) {
         if (itsMode==DDDObject::Move) {
            Float d = Float(itsWorldCanvasPtr->canvasDrawYSize());
            Float off = Float(itsWorldCanvasPtr->canvasDrawYOffset()) + 
                        Float(itsWorldCanvasPtr->canvasYOffset());
            y(i) = (pixel(1)-off)/d;
         }
      } else {
        y(i) = world(1);
      }
   }
//
   itsWorldX.setValue(x);
   itsWorldX.setUnit(units(0));
   itsWorldY.setValue(y);
   itsWorldY.setUnit(units(1));
}

void DDDPolygon::decode(const RecordInterface& description, Bool required)
{
  String error;

// Get X

  Vector<Double> eV;
  Quantum<Vector<Double> > empty;
  empty.setValue(eV);
  empty.setUnit(Unit(String("km")));  
  if (required || description.isDefined("x")) {
     const RecordInterface& subRecord = description.asRecord("x");
     QuantumHolder h;
     if (!h.fromRecord(error, subRecord)) {
           throw (AipsError ("DDDPolygon - could not extract X vector because "+error));
     }
     itsWorldX.getValue().resize(0);
     itsWorldX = h.asQuantumVectorDouble();
     itsFracX = itsWorldX.getFullUnit().getName()==String("frac");
  }

// Get Y

  if (required || description.isDefined("y")) {
     const RecordInterface& subRecord = description.asRecord("y");
     QuantumHolder h;
     if (!h.fromRecord(error, subRecord)) {
           throw (AipsError ("DDDPolygon - could not extract Y vector because "+error));
     }
     itsWorldY.getValue().resize(0);
     itsWorldY = h.asQuantumVectorDouble();
     itsFracY = itsWorldY.getFullUnit().getName()==String("frac");
  }
//
  itsNPoints = itsWorldX.getValue().nelements();
  if (itsNPoints != itsWorldY.getValue().nelements()) {
     throw (AipsError("X and Y vectors must be the same length"));
  }
//
  if (description.isDefined("outline"))  {
     description.get("outline", itsOutline);
  }
}


void DDDPolygon::encode (RecordInterface& description) const
{
   String error;

   description.define ("type", String("polygon"));
   {
      Record rec;
      QuantumHolder qh(itsWorldX);
      if (!qh.toRecord(error, rec)) {
         throw(AipsError(error));
      }
      description.defineRecord("x", rec);
   }
//
   {
      Record rec;
      QuantumHolder qh(itsWorldY);
      if (!qh.toRecord(error, rec)) {
         throw(AipsError(error));
      }
      description.defineRecord("y", rec);
   }
//
   description.define("outline", itsOutline);
}



void DDDPolygon::createCorners() 
{  
  Double xmin, xmax;
  minMax(xmin, xmax, itsPixelX);
  Double ymin, ymax;
  minMax(ymin, ymax, itsPixelY);
//
  itsCorners(0,0) = xmin;
  itsCorners(0,1) = ymin;
//   
  itsCorners(1,0) = xmin;
  itsCorners(1,1) = ymax;
//
  itsCorners(2,0) = xmax;
  itsCorners(2,1) = ymax;
//
  itsCorners(3,0) = xmax;
  itsCorners(3,1) = ymin;
}

void DDDPolygon::createHandles()
{
  itsHandles.resize(itsNPoints);
  for (uInt i=0; i< itsHandles.nelements();i++) {
     itsHandles[i].createHandle(itsPixelX[i], itsPixelY[i]);
   }
}


DDDPolygon::DDDPolygon() 
{}

DDDPolygon::DDDPolygon(const DDDPolygon &) 
{}

void DDDPolygon::operator=(const DDDPolygon &) 
{}


void DDDPolygon::listWorld()
{
   cerr << "World" << endl;
   cerr << " x = " << itsWorldX.getValue() << endl;
   cerr << " y = " << itsWorldY.getValue() << endl;
}

void DDDPolygon::listPixel()
{
   cerr << "Pixel" << endl;
   cerr << " x = " << itsPixelX << endl;
   cerr << " y = " << itsPixelY << endl;
}

} //# NAMESPACE CASA - END

