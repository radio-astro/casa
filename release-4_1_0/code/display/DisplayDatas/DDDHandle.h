//# DDDHandle.h: class to draw handles on DDDObjects
//# Copyright (C) 1999,2000,2001
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

#ifndef TRIALDISPLAY_DDDHANDLE_H
#define TRIALDISPLAY_DDDHANDLE_H

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A class to draw and detect handles on a DDDObject
// </summary>

// <synopsis> This class defines a square handle and it's
// behaviour. This can be used to interact with DDDObjects
// </synopsis>

class DDDHandle {
  
public:
  // enum describing the size of the handle
  enum Size {NONE=0,SMALL,MEDIUM,LARGE};

  // Default constructor
  DDDHandle();
  // Constructor taking the position where to place the handle and it's size
  DDDHandle(Double posx, Double posy, DDDHandle::Size size=DDDHandle::MEDIUM);

  // Destructor.
  virtual ~DDDHandle();
  
  // This function returns true when the input poistion is within the handle
  virtual Bool underCursor(Double posx, Double posy) const; 

  // give the handle a new size
  virtual void resize(const DDDHandle::Size& size);
  // move the handle to a new position
  virtual void move(Double dx, Double dy);
  
  // <group>
  // utility funtions to retrieve blc/trc of the handle or the handle corners
  virtual Double blcX() const;
  virtual Double blcY() const;
  virtual Double trcX() const;
  virtual Double trcY() const;
  Matrix<Double> getHandle() const {return itsCorners;};
  // </group>

  // set up the handle
  // <group>
  void createHandle(Double x, Double y);
  void createHandle(const Vector<Double>& point);
  // </group>

  DDDHandle(const DDDHandle &);
  void operator=(const DDDHandle &);

private:
  // the corners
  Matrix<Double> itsCorners;
  // the size
  DDDHandle::Size itsSize;

};


} //# NAMESPACE CASA - END

#endif
