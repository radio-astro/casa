//# DDDObject.cc: base object interface for DrawingDisplayData 
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

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayDatas/DDDObject.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DDDObject::DDDObject(const Record &description, DrawingDisplayData *owner) :

  itsOwner(owner),
  itsShowHandles(False),
  itsEditable(False),
  itsMovable(True),
  itsLineWidth(1) {
  if (description.isDefined("color")) {
    description.get("color", itsColor);
  } else {
    itsColor = "foreground";
  }
  if (description.isDefined("label")) {
    description.get("label", itsLabel);
  } else {
    itsLabel = "";
  }
  if (description.isDefined("linewidth")) {
    description.get("linewidth", itsLineWidth);
  } 
  if (description.isDefined("editable")) {
    description.get("editable", itsEditable);
  } 
  if (description.isDefined("movable")) {
    description.get("movable", itsMovable);
  } 
  if (description.isDefined("id")) {
    description.get("id", itsObjectID);
  } else {
    throw(AipsError("No id field in object description"));
  }
  clearClickBuffer();

// Add fractional units 

  UnitMap::putUser("frac",UnitVal(1.0), "fractional units");
}

DDDObject::~DDDObject() {
}

void DDDObject::showHandles(const Bool show, const Bool tellOwner) {
  Bool state = itsShowHandles;
  itsShowHandles = show;
  if (itsShowHandles && !state) {
    itsOwner->addMotionEventHandler(this);
  } else if (!itsShowHandles && state) {
    itsOwner->removeMotionEventHandler(*this);
  }
  if (itsShowHandles != state && tellOwner) {
    itsOwner->setHandleState(this, itsShowHandles);
  }
}

Record DDDObject::description() 
{
  Record rec;
//
  rec.define("color", itsColor);
  rec.define("label", itsLabel);
  rec.define("linewidth", itsLineWidth);
  rec.define("editable", itsEditable);
  rec.define("movable", itsMovable);
  rec.define("id", itsObjectID);
  return rec;
}

void DDDObject::setDescription(const Record &rec) 
{
  if (rec.isDefined("color")) {
     itsColor = rec.asString("color");
  }
  if (rec.isDefined("label")) {
     itsLabel = rec.asString("label");
  }
  if (rec.isDefined("linewidth")) {
    itsLineWidth = rec.asInt("linewidth");
  }
  if (rec.isDefined("editable")) {
    itsEditable = rec.asBool("editable");
  }
  if (rec.isDefined("movable")) {
    itsMovable = rec.asBool("movable");
  }
  itsObjectID = rec.asInt("id");
}

Bool DDDObject::storeClick(const DisplayEvent &ev) {
  its2ndLastClickTime = itsLastClickTime;
  itsLastClickTime = ev.timeOfEvent();
  if ((itsLastClickTime > 0.) && (its2ndLastClickTime > 0.) &&
      (abs(itsLastClickTime - its2ndLastClickTime) < 
       itsOwner->doubleClickInterval())) {
    itsOwner->doubleClick(itsObjectID);
    clearClickBuffer();
    return True;
  }
  return False;
}

void DDDObject::clearClickBuffer() {
  its2ndLastClickTime = -99999.9;
  itsLastClickTime = -99999.9;
}

DDDObject::DDDObject() {
}

DDDObject::DDDObject(const DDDObject &) {
}

void DDDObject::operator=(const DDDObject &) {
}


Matrix<Double> DDDObject::rotateMatrix(const Matrix<Double>& points,
                                       Double angle)
{  
  Matrix<Double> retmat(points.nrow(),points.ncolumn());

// rotates the matrix around itsCenter
   
  Double cosa = cos(angle);
  Double sina = sin(angle);
  Matrix<Double> rotMat(2,2);

//need  transposed rotmatrix here, because (x,y) a row vector

  rotMat(0,0) = cosa;
  rotMat(1,0) = -sina;
  rotMat(0,1) = sina;
  rotMat(1,1) =  cosa;
//
//  retmat = matrixProduct(points,rotMat);
  return product (points, rotMat);
}

void DDDObject::translateMatrix(Matrix<Double>& points,
                                Double dx, Double dy)
{
// shift by dx, dy

  for (uInt k=0;k<2;k++) {
    for (uInt l=0;l<4;l++) {
      if (k==0) points(l,k) += dx;
      else if (k==1) points(l,k) += dy;
    }
  }
}


Bool DDDObject::inPolygon(const Matrix<Double>& pol,
                          Double x, Double y)
{
  const Vector<Double>& xP = pol.column(0);
  const Vector<Double>& yP = pol.column(1);
  return inPolygon(xP, yP, x, y);
}

Bool DDDObject::inPolygon(const Vector<Double>& xP, const Vector<Double>& yP,
                          Double x, Double y)
{	
  AlwaysAssert(xP.nelements()==yP.nelements(), AipsError);
  Bool isInside = False;
  uInt n = xP.nelements();
  for (uInt i=0,j=n-1; i < n; j=i++) {
    if ((((yP(i)<=y) && (y<yP(j))) ||
         ((yP(j)<=y) && (y<yP(i)))) &&
        (x < (xP(j) - xP(i)) * (y - yP(i)) / (yP(j) - yP(i)) + xP(i))) {
      isInside = !isInside;
    }
  }
  return isInside;
}


Bool DDDObject::onHandle(const Block<DDDHandle>& handles,
                         Double x, Double y)
{
  for (uInt i=0; i< handles.nelements();i++) {
    if (handles[i].underCursor(x,y)) {
      return True;
    }
  }
  return False;
}


void DDDObject::convertCoordinateSystem (CoordinateSystem& cSys, WorldCanvas* wcPtr) const
{
  uInt n = cSys.nPixelAxes();
  static Vector<Float> screenShape(2);
  static Vector<Double> imageShape(2);
  static Vector<Float> fac;
  static Vector<Float> offset;
  static Vector<Int> newShp;
  static Vector<Double> pixel, world;
//
  screenShape(0) = Float(wcPtr->canvasDrawXSize());
  screenShape(1) = Float(wcPtr->canvasDrawYSize());
  
// Need plus 1 if pixel treatement is 'center'
// I cant find that out here, so use 'edge' which is the default.

//  imageShape(0) = wcPtr->linXMax() - wcPtr->linXMin() + 1.0;
//  imageShape(1) = wcPtr->linYMax() - wcPtr->linYMin() + 1.0;

  imageShape(0) = wcPtr->linXMax() - wcPtr->linXMin();
  imageShape(1) = wcPtr->linYMax() - wcPtr->linYMin();

// Find blc in world

  pixel.resize(n);
  pixel = cSys.referencePixel().copy();
  pixel(0) = 0.0;
  pixel(1) = 0.0;
  Bool ok = cSys.toWorld(world, pixel);

// Account for start of image display from start of pixel canvas
// I can't seem to get this quite right... But it's seems to be
// close enough for now (used in shape conversions)
//
  fac.resize(n);
  offset.resize(n);
  fac = 1.0;
  fac(0) = imageShape(0) / screenShape(0);
  fac(1) = imageShape(1) / screenShape(1);
  offset = 0.0;
  cSys.subImageInSitu(offset, fac, newShp);

// Convert blc world to subimage pixel

  ok = cSys.toPixel(pixel, world);
//  
  Float dx = Float(wcPtr->canvasDrawXOffset()) + Float(wcPtr->canvasXOffset());
  Float dy = Float(wcPtr->canvasDrawYOffset()) + Float(wcPtr->canvasYOffset());
//cerr << "x offsets = " << wcPtr->canvasDrawXOffset() << ", " << wcPtr->canvasXOffset() << endl;
//
  Vector<Double> rp = cSys.referencePixel().copy();
//  Vector<Double> tmp = rp.copy();

  rp(0) += (dx - pixel(0));
  rp(1) += (dy - pixel(1));
  cSys.setReferencePixel(rp);

//cerr << "old, new refpix = " << tmp << rp << endl;
} 


} //# NAMESPACE CASA - END

