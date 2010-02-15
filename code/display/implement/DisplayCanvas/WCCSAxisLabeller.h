//# WCCSAxisLabeller.h: labelling axes using a CoordinateSystem on a WC
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

#ifndef TRIALDISPLAY_WCCSAXISLABELLER_H
#define TRIALDISPLAY_WCCSAXISLABELLER_H

#include <casa/aips.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <display/DisplayCanvas/WCAxisLabeller.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas axis labelling using a CoordinateSystem.
// </summary>
//
// <synopsis>
// This (base) class adds to the interface of WCAxisLabeller functions
// which support the use/provision of a CoordinateSystem to assist with
// axis labelling.
// </synopsis>

class WCCSAxisLabeller : public WCAxisLabeller {

 public:

  // Constructor
  WCCSAxisLabeller();

  // Destructor
  virtual ~WCCSAxisLabeller();

  // Install a CoordinateSystem.
  //#dk note: See Bool useWCCS, below; in some cases the WorldCanvas's
  //#   own CS is now used to draw labels, although the CS set here is 
  //#   still needed for certain things (default user interface, e.g.)
  //#   when the WC is unknown.  (10/07)
  virtual void setCoordinateSystem(const CoordinateSystem& coordsys);

  // Get the CoordinateSystem.
  virtual CoordinateSystem coordinateSystem() const {
    return itsCoordinateSystem;  }

  // Has a CS been set?
  Bool hasCoordinateSystem() const { return itsHasCoordinateSystem; };

  // Setting this True allows the labeller to use the WorldCanvas's
  // own CS to draw labels (although itsCoordinateSystem is still
  // needed for certain things at present).  Default: False.
  //#dk (See WCCSNLAxisLabeller::draw() and usage in PADD::setupElements()).
  //#
  //# (Kludge upon kludge, I know... all stemming from the original design
  //# flaw: not realizing that WC needs its _own CS_ (related to but _not
  //# the same_ as the Images'), and then trying to tack one on later...).
  Bool useWCCS;
  
  // install the default options for this labeller.
  virtual void setDefaultOptions();

  // apply options stored in rec to the labeller; return value
  // True means a refresh is needed.  Any fields added to the
  // updatedOptions argument are options which have changed in
  // some way due to the setting of other options - ie. they 
  // are context sensitive.
  virtual Bool setOptions(const Record &rec, Record &updatedOptions);

  // retrieve the current and default options and parameter types.
  virtual Record getOptions() const;

  // return the X and Y label text - over-ridden from base class
  // <group>
  //# virtual String xAxisText(WorldCanvas* wc=0) const;
  //# virtual String yAxisText(WorldCanvas* wc=0) const;
  //# (Compiler whines unless you do it this way instead... grr...).
  virtual String xAxisText(WorldCanvas* wc) const;
  virtual String yAxisText(WorldCanvas* wc) const;
  virtual String xAxisText() const { return xAxisText(0);  }
  virtual String yAxisText() const { return yAxisText(0);  }
  // </group>
  
  virtual String zLabelType() const { return itsZLabelType; };

  virtual void setZIndex(Int zindex) { itsZIndex = zindex; };

  // DD 'Absolute Pixel Coordinates', e.g. channel numbers, are internally
  // 0-based (they begin numbering at 0), but 'Absolute Pixel coordinates'
  // have traditionally been displayed as 1-based in the glish viewer.
  // uiBase_, and related methods uiBase() and setUIBase(), allow newer
  // (python/Qt-based) code to cause such labelling to be produced with
  // 0-based values instead.  Unless setUIBase(0) is called, the
  // traditional 1-based labelling behavior is retained by default.
  //
  // If you are using 0-basing for 'Absolute Pixel Coordinate' labelling,
  // you should call setUIBase(0), before using draw().
  // <group>
  virtual Int uiBase() { return uiBase_;  }
  
  virtual void setUIBase(Int uibase) {
    if(uibase==0 || uibase==1) uiBase_ = uibase;  }
  // </group>
 
 
 protected:
  Bool itsAbsolute;
  Bool itsWorldAxisLabels;
  mutable Bool itsDoVelocity;
  Int itsZIndex;
  Vector<Double> itsRefPixel;

  // Set spectral state onto given CS
  void setSpectralState(CoordinateSystem& cs) const;

  // Set direction state onto given CS
  void setDirectionState(CoordinateSystem& cs) const;

 private:

  CoordinateSystem itsCoordinateSystem;
  Bool itsHasCoordinateSystem;
  String itsSpectralUnit;
  String itsDoppler;
  String itsDirectionUnit;
  String itsDirectionSystem;
  String itsFrequencySystem;
  String itsZLabelType;

  Int uiBase_;		// (initialized to 1; see uiBase(), above).
  
  // Generate axis text for specified axis
  //# String axisText(Int worldAxis, WorldCanvas* wc=0) const;
  //# (Compiler whines unless you do it this way instead... grr...).
  String axisText(Int worldAxis, WorldCanvas* wc) const;
  String axisText(Int worldAxis) const { return axisText(worldAxis, 0);  }
 
  // Set new spectral state in itsCoordinateSystem
  void setSpectralState() {  setSpectralState(itsCoordinateSystem);  }

  // Set new direction state in itsCoordinateSystem
  void setDirectionState() { setDirectionState(itsCoordinateSystem);  }

  // Set absolute/relative state in itsCoordinateSystem
  void setAbsRelState();

};


} //# NAMESPACE CASA - END

#endif
