//# PixelCanvas.cc: abstract interface to pixel-oriented drawing routines
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/Constants.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/Colormap.h>
#include <display/Display/ColorConversion.h>
#include <display/DisplayEvents/PCMotionEH.h>
#include <display/DisplayEvents/PCPositionEH.h>
#include <display/DisplayEvents/PCRefreshEH.h>

namespace casa { //# NAMESPACE CASA - BEGIN

PixelCanvas::PixelCanvas() :
  defaultColormapActive_(False),
  colormap_(0),
  drawMode_(Display::Draw), 
  drawBuffer_(Display::DefaultBuffer), 
  colorModel_(Display::RGB),
  refreshActive_(False),
  nRegisteredColormaps_(0),
  vgbuf_(this, 1024) {
}

PixelCanvas::PixelCanvas(PixelCanvasColorTable * pcctbl) :
  defaultColormapActive_(False),
  colormap_(0),
  drawMode_(Display::Draw), 
  drawBuffer_(Display::DefaultBuffer), 
  colorModel_(Display::RGB),
  refreshActive_(False),
  nRegisteredColormaps_(0),
  vgbuf_(this, 1024) {
  // register the default colormap if color table is in index mode
  if (pcctbl->colorModel() == Display::Index)
    {
      pcctbl->registerColormap(pcctbl->defaultColormap());
      setColormap(pcctbl->defaultColormap());
      defaultColormapActive_ = True;
    }

  // register with the PixelCanvasColorTable
  //pcctbl->registerPixelCanvas(this);
}

PixelCanvas::~PixelCanvas()
{
}

void PixelCanvas::setColormap(Colormap * map) {
  colormap_ = map;
}

void PixelCanvas::setColorModel(Display::ColorModel model)
{
  switch(model)
    {
    case Display::Index:
      throw(AipsError("Invalid multichannel color model"));
      return;
    case Display::RGB:
    case Display::HSV:
      colorModel_ = model;
      break;
    }
}

void PixelCanvas::mapToColor3(Array<uLong> & out,
			      const Array<Float> & chan1in,
			      const Array<Float> & chan2in,
			      const Array<Float> & chan3in)
{
  if (colorModel_ == pcctbl()->colorModel())
    pcctbl()->mapToColor3(out, chan1in, chan2in, chan3in);
  else
    {
      IPosition shape = chan1in.shape();
      Array<Float> chan1t(shape);
      Array<Float> chan2t(shape);
      Array<Float> chan3t(shape);
      pcctbl()->colorSpaceMap(colorModel_,
		    chan1in, chan2in, chan3in,
		    chan1t, chan2t, chan3t);
      pcctbl()->mapToColor3(out, chan1t, chan2t, chan3t);
    }
}

void PixelCanvas::mapToColor3(Array<uLong> & out,
			      const Array<Double> & chan1in,
			      const Array<Double> & chan2in,
			      const Array<Double> & chan3in)
{
  if (colorModel_ == pcctbl()->colorModel())
    pcctbl()->mapToColor3(out, chan1in, chan2in, chan3in);
  else
    {
      IPosition shape = chan1in.shape();
      Array<Float> chan1i(shape);
      Array<Float> chan2i(shape);
      Array<Float> chan3i(shape);
      Array<Float> chan1o(shape);
      Array<Float> chan2o(shape);
      Array<Float> chan3o(shape);
      convertArray(chan1i, chan1in);
      convertArray(chan2i, chan2in);
      convertArray(chan3i, chan3in);
      pcctbl()->colorSpaceMap(colorModel_,
		    chan1i, chan2i, chan3i,
		    chan1o, chan2o, chan3o);
      pcctbl()->mapToColor3(out, chan1o, chan2o, chan3o);
    }
}

void PixelCanvas::mapToColor3(Array<uLong> & out,
			      const Array<uInt> & chan1in,
			      const Array<uInt> & chan2in,
			      const Array<uInt> & chan3in)
{
  if (colorModel_ == pcctbl()->colorModel())
    pcctbl()->mapToColor3(out, chan1in, chan2in, chan3in);
  else
    {
      /*
      IPosition shape = chan1in.shape();
      Array<Float> chan1t(shape);
      Array<Float> chan2t(shape);
      Array<Float> chan3t(shape);
      pcctbl()->colorSpaceMap(colorModel_,
		    chan1in, chan2in, chan3in,
		    chan1t, chan2t, chan3t);
      pcctbl()->mapToColor3(out, chan1t, chan2t, chan3t);
      */
      throw(AipsError("Color conversion not supported yet in "
		      "PixelCanvas::mapToColor3"));
    }
}

//
// REFRESH EVENT
//

void PixelCanvas::callRefreshEventHandlers(Display::RefreshReason reason)
{
  if (!refreshAllowed()) {
    return;
  }
  refreshActive_ = True;
  PCRefreshEvent ev(this,reason);
  ListIter<void *> it1(&refreshEHList_);
  while(!it1.atEnd())
    {
      (*((PCRefreshEH *) it1.getRight()))(ev);
      it1++;
    }
  refreshActive_ = False;
}

void PixelCanvas::addRefreshEventHandler(const PCRefreshEH & eh)
{
  ListIter<void *> it1(&refreshEHList_);
  it1.toEnd();
  it1.addRight((void *) &eh);
}

void PixelCanvas::removeRefreshEventHandler(const PCRefreshEH & eh)
{
  ListIter<void *> it1(&refreshEHList_);
  while(!it1.atEnd())
    {
      if (it1.getRight() == (void *) &eh)
	{
	  it1.removeRight();
	  break;
	}
      it1++;
    }
}

//
//  MOTION EVENT
//

void PixelCanvas::callMotionEventHandlers(Int x, Int y, uInt state)
{
  PCMotionEvent ev(this,x,y,state);
  ConstListIter<void *> it1(&motionEHList_);
  while(!it1.atEnd())
    {
      (*((PCMotionEH *) it1.getRight()))(ev);
      it1++;
    }
}

void PixelCanvas::addMotionEventHandler(const PCMotionEH &eh)
{
  if (motionEHList_.len() == 0) 
    enableMotionEvents();
  ListIter<void *> it1(&motionEHList_);
  it1.toEnd();
  it1.addRight((void *) &eh);
}

void PixelCanvas::removeMotionEventHandler(const PCMotionEH & eh)
{
  ListIter<void *> it1(&motionEHList_);
  while(!it1.atEnd())
    {
      if (it1.getRight() == (void *) &eh)
	{
	  it1.removeRight();
	  break;
	}
      it1++;
    }
  if (motionEHList_.len() == 0)
    disableMotionEvents();
}

//
//  POSITION EVENT
//

void PixelCanvas::callPositionEventHandlers(Display::KeySym keysym, Bool keystate,
					    Int x, Int y, uInt state)
{
  PCPositionEvent ev(this, keysym, keystate, x, y, state);
  ConstListIter<void *> it1(&positionEHList_);
  while(!it1.atEnd())
    {
      (*((PCPositionEH *) it1.getRight()))(ev);
      it1++;
    }
}

void PixelCanvas::addPositionEventHandler(const PCPositionEH & eh)
{
  ListIter<void *> it1(&positionEHList_);
  it1.toEnd();
  it1.addRight((void *) &eh);
}

void PixelCanvas::removePositionEventHandler(const PCPositionEH & eh)
{
  ListIter<void *> it1(&positionEHList_);
  while(!it1.atEnd())
    {
      if (it1.getRight() == (void *) &eh)
	{
	  it1.removeRight();
	  break;
	}
      it1++;
    }
}

//
// Color functions
//

void PixelCanvas::setHSVColor(float h, float s, float v)
{
  float r,g,b;
  hsvToRgb(h,s,v,r,g,b);
  setRGBColor(r,g,b);
}

Bool PixelCanvas::getHSVColor(Int x, Int y, float &h, float &s, float &v)
{
  float r,g,b;
  Bool retval = getRGBColor(x,y,r,g,b);
  if (retval) rgbToHsv(r,g,b,h,s,v);
  return retval;
}

void PixelCanvas::registerColormap(Colormap * cmap, Float weight)
{
  if (pcctbl()->colorModel() != Display::Index) {
    // SHOULD CHANGE THIS INTO A LOG MESSAGE...
    //throw(AipsError("Colormaps cannot be registered on HSV or RGB "
    //		    "PixelCanvases"));
    return;
  }

  // unregister default map if necessary
  //if (defaultColormapActive_) 
  ColormapManager& cmm = pcctbl()->colormapManager();
  if ((cmm.nMaps() == 1) && (cmm.getMap(0) == 
			     pcctbl()->defaultColormap())) {
    pcctbl()->unregisterColormap(pcctbl()->defaultColormap());
    pcctbl()->registerColormap(cmap, weight);
    setColormap(cmap);
    defaultColormapActive_ = False;
  } else {
    pcctbl()->registerColormap(cmap, weight);
  }

  nRegisteredColormaps_++;
}

void PixelCanvas::registerColormap(Colormap *cmap, Colormap *cmapToReplace) {
  if (pcctbl()->colorModel() != Display::Index) {
    return;
  }
  pcctbl()->registerColormap(cmap, cmapToReplace);
  setColormap(cmap);
  ColormapManager& cmm = pcctbl()->colormapManager();
  defaultColormapActive_ = ((cmm.nMaps() == 1) && 
				  (cmm.getMap(0) == 
				   pcctbl()->defaultColormap()));
}

void PixelCanvas::unregisterColormap(Colormap * cmap)
{
  if (pcctbl()->colorModel() != Display::Index) {
    // SHOULD LOG A MESSAGE HERE...
    //throw(AipsError("Colormaps cannot be unregistered on HSV or RGB "
    //		    "PixelCanvases"));
    return;
  }

  pcctbl()->unregisterColormap(cmap);
  nRegisteredColormaps_--;

  // register default map if necessary
  //if (nRegisteredColormaps_ == 0)
  ColormapManager& cmm = pcctbl()->colormapManager();
  if (cmm.nMaps() == 0) {
    pcctbl()->registerColormap(pcctbl()->defaultColormap());
    setColormap(pcctbl()->defaultColormap());
    defaultColormapActive_ = True;  }
  else if(!colormapRegistered()) {
    setColormap((Colormap *)(cmm.getMap(0)));  }
}


void PixelCanvas::drawMarker(const Int& x1, const Int& y1,
			     const Display::Marker& marker, 
			     const Int& pixelHeight) {
  drawMarker(Float(x1), Float(y1), marker, pixelHeight);
}

void PixelCanvas::drawMarker(const Float& x1, const Float& y1,
			     const Display::Marker& marker, 
			     const Int& pixelHeight) {
  
  Matrix<Float> scaled(0,0);

  // On odd sized shapes, the markers don't scale nicely, so:

  if (pixelHeight <= 0) return;

  Int adjustedHeight;
  adjustedHeight  = Int(Float(pixelHeight)*pixelScaling()+0.5);

  
  if (adjustedHeight % 2 != 0) {
    adjustedHeight += 1;
  }

  scaled = getMarker(marker, Float(adjustedHeight));

  Vector<Float> x,y;
  for (uInt i=0 ;i<scaled.nrow() ; i++) {
    scaled(i,0) += x1;
    scaled(i,1) += y1;
  }

  switch(marker) {
  case Display::Triangle:
    drawPolygon(scaled);
    break;
  case Display::InvertedTriangle:
    drawPolygon(scaled);
    break;
  case Display::Square:
    drawPolygon(scaled);
    break;
  case Display::Diamond:
    drawPolygon(scaled);
    break;
  case Display::Cross:
    drawPolyline(scaled);
    break;
  case Display::X:
    drawPolyline(scaled);
    break;
  case Display::Circle:
    drawEllipse(x1,y1, adjustedHeight / 2, adjustedHeight / 2 , 0, True);
    break;
    // NYI in DisplayMarkerDefinitions.cc
    /*
      case Display::Pentagon:
      drawPolygon(scaled);
      break;
      case Display::Hexagon:
      drawPolygon(scaled);
      break;
    */
  case Display::FilledCircle:
    drawEllipse(x1,y1,adjustedHeight / 2, adjustedHeight / 2, 0, False);
    break;
  case Display::FilledSquare:
    x = scaled.column(0);
    y = scaled.column(1);
    drawFilledPolygon(x,y);
    break;
  case Display::FilledTriangle:
    x = scaled.column(0);
    y = scaled.column(1);
    drawFilledPolygon(x,y);
    break;
  case Display::FilledDiamond:
    x = scaled.column(0);
    y = scaled.column(1);
    drawFilledPolygon(x,y);
    break;
  case Display::FilledInvertedTriangle:
    x = scaled.column(0);
    y = scaled.column(1);
    drawFilledPolygon(x,y);
    break;
    // NYI in DisplayMarkerDefinitions.cc
    /*
      case Display::FilledPentagon:
      x = scaled.column(0);
      y = scaled.column(1);
      drawFilledPolygon(x,y);
      break;
      case Display::FilledHexagon:
      x = scaled.column(0);
      y = scaled.column(1);
      drawFilledPolygon(x,y);
      break;
    */
  case Display::CircleAndCross:
    drawPolyline(scaled);
    drawEllipse(x1,y1,adjustedHeight / 2, adjustedHeight / 2, True);
    break;
  case Display::CircleAndX:
    drawPolyline(scaled);
    drawEllipse(x1,y1,adjustedHeight / 2, adjustedHeight  / 2, True);
    break;
  case Display::CircleAndDot:
    drawEllipse(x1,y1,adjustedHeight / 2, adjustedHeight  / 2, True);
    drawEllipse(x1,y1,adjustedHeight / 15, adjustedHeight  / 15, False);
    break;
  default:
    drawPolyline(scaled);
    break;
  }
}

void PixelCanvas::drawMarker(const Double& x1, const Double& y1,
			     const Display::Marker& marker, const Int& pixelHeight) {
  drawMarker(Float(x1), Float(y1), marker, pixelHeight);

}



void PixelCanvas::drawEllipse(const Float &cx, const Float &cy,
			      const Float &smajor, const Float &sminor,
			      const Float &pangle, Bool outline,
			      Float xstretch, Float ystretch)
{
  Float theta = pangle * (Float)C::degree;
  Float ang, temp;
  uInt nsegments = uInt( min(5000., 
			     2 * C::pi * 
                             max(abs(smajor), abs(sminor)) * 
                             max(abs(xstretch), abs(ystretch)) + 0.5) );
  if (nsegments < 4) nsegments = 4;

  Vector<Float> px(nsegments), py(nsegments);
  for (uInt j = 0; j < nsegments; j++) {
    ang = (Float)j / (Float)(nsegments - 1) * C::circle;
    px(j) = smajor * cos(ang);
    py(j) = sminor * sin(ang);
    ang = theta;
    temp  = cx + xstretch * ( px(j) * cos(ang) - py(j) * sin(ang) );
    py(j) = cy + ystretch * ( px(j) * sin(ang) + py(j) * cos(ang) );
    px(j) = temp;
  }
//
  if (outline) {
     drawPolygon(px, py);
  } else {
     drawFilledPolygon(px, py);
  }
}

void PixelCanvas::drawColoredEllipses(const Matrix<Float> &centres,
				      const Vector<Float> &smajor,
				      const Vector<Float> &sminor,
				      const Vector<Float> &pangle,
				      const Vector<uInt> &colors,
				      const Float &scale,
				      const Bool &outline) {
  // Use pure virtual methods to draw a set of colored ellipses.

  uInt nellipses = centres.nrow();
  if ((smajor.nelements() != nellipses) ||
      (sminor.nelements() != nellipses) ||
      (pangle.nelements() != nellipses) ||
      (colors.nelements() != nellipses)) {
    throw(AipsError("Invalid vectors given to PixelCanvas::"
		    "drawColoredEllipses"));
  }

  // scale
  Vector<Float> sa = smajor * scale;
  Vector<Float> sb = sminor * scale;
  // convert degrees to radians

  Vector<Float> theta = pangle * (Float)C::degree;

  uInt nsegments = 0;
  Vector<Float> px(0), py(0);
  Float ang = 0.0;
  Float temp = 0.0;

  for (uInt i = 0; i < nellipses; i++) {
    nsegments = uInt(2 * C::pi * sa(i) + 0.5);
    px.resize(nsegments);
    py.resize(nsegments);
    for (uInt j = 0; j < nsegments; j++) {
      //ang = theta(i) + (Float)j / (Float)(nsegments - 1) * C::circle;
      //px(j) = centres(i, 0) + sa(i) * cos(ang) - sb(i) * sin(ang);
      //py(j) = centres(i, 1) + sa(i) * sin(ang) + sb(i) * cos(ang);
      ang = (Float)j / (Float)(nsegments - 1) * C::circle;
      px(j) = sa(i) * cos(ang);
      py(j) = sb(i) * sin(ang);
      ang = theta(i);
      temp = centres(i, 0) + px(j) * cos(ang) - py(j) * sin(ang);
      py(j) = centres(i, 1) + px(j) * sin(ang) + py(j) * cos(ang);
      px(j) = temp;
    }
    setColor(colors(i));
    drawFilledPolygon(px, py);
    if (outline) {
      setColor("foreground");
      drawPolygon(px, py);
    }
  }
  
}

} //# NAMESPACE CASA - END

