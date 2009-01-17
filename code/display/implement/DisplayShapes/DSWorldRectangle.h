//# DSWorldRectangle.h : Implementation of a world coords DSRectangle
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id: 
#ifndef TRIALDISPLAY_DSWORLDRECTANGLE_H
#define TRIALDISPLAY_DSWORLDRECTANGLE_H
#include <casa/aips.h>
#include <casa/Arrays/Vector.h> 
#include <casa/Arrays/Matrix.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <display/Display/PanelDisplay.h>

#include <display/DisplayShapes/DSRectangle.h>
#include <display/DisplayShapes/DisplayShapeWithCoords.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a world coords DSRectangle
// </summary>

class PanelDisplay;
class DSPixelRectangle;
class DSScreenRectangle;

class DSWorldRectangle : public DSRectangle , public DisplayShapeWithCoords{
  
public:
  
  DSWorldRectangle();
  DSWorldRectangle(const Record& settings, PanelDisplay* pd);
  DSWorldRectangle(DSScreenRectangle& other, PanelDisplay* pd);
  DSWorldRectangle(DSPixelRectangle& other, PanelDisplay* pd);

  virtual ~DSWorldRectangle();

  virtual void recalculateScreenPosition();

  // So we can update our WCs
  // <group>
  virtual void move(const Float& dX, const Float& dY);
  virtual void setCenter(const Float& xPos, const Float& yPos);
  virtual void rotate(const Float& angle);
  virtual void changePoint(const Vector<Float>&pos, const Int n);
  virtual void changePoint(const Vector<Float>& pos);
  virtual void draw(PixelCanvas* pc);
  // </group>

  // PLUS OTHERS!!

  virtual Bool setOptions(const Record& settings);
  virtual Record getOptions();

  

  virtual Record getRawOptions() {
    return DSRectangle::getOptions();
  }
  
  virtual PanelDisplay* panelDisplay()  {
    return itsPD;
  }
private:
  
  
  // Based on a pixel center
  //  virtual Bool chooseWC(const Float& centerX, const Float& centerY,
  //		PanelDisplay* pdx, 
  //WorldCanvas* wc);
  
  // The paneldisplay from which I may choose an appropriate WC
  PanelDisplay* itsPD;

  // The WC of my choosing
  WorldCanvas* itsWC;

  // The center of the marker in world co-ords.
  // x, y, major, minor, angle
  Vector<Quantum<Double> > itsWorldParameters;
  
  void updateWCoords();
};

} //# NAMESPACE CASA - END

#endif
