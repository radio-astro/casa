//# DSBasic.cc: Basically, a base class for any shape which can be drawn with 
//# primitive lines.
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

#include <casa/aips.h>

#include <display/Display/DParameterRange.h>
#include <display/DisplayShapes/DSBasic.h> 
#include <display/Display/PixelCanvas.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSBasic::DSBasic() :
  DisplayShape() { 

  itsLineWidth = new DParameterRange<Int> ("linewidth", "Line Width", 
					   "Select the desired line width",
					   1,20,1,1,1);
}

DSBasic::DSBasic(const DSBasic& other) :
  DisplayShape(other),
  itsLineWidth(other.itsLineWidth) {
}



DSBasic::~DSBasic() {
  delete itsLineWidth; itsLineWidth = 0;
}

Record DSBasic::getOptions() {
  Record rec = DisplayShape::getOptions();
  rec.define("type", "shape");
  
  itsLineWidth->toRecord(rec);

  return rec;
}

Bool DSBasic::setOptions(const Record& settings) {
  Bool localChange = False;

  if (itsLineWidth->fromRecord(settings)) localChange = True;
  if (DisplayShape::setOptions(settings)) localChange = True;

  return localChange;
}

void DSBasic::setLineWidth(const Int pixels) {
  itsLineWidth->setValue(pixels);
}

Int DSBasic::getLineWidth() {
  return itsLineWidth->value();
}

void DSBasic::draw(PixelCanvas* pc) {
  DisplayShape::draw(pc);
}

void DSBasic::move(const Float& dX, const Float& dY) {
  DisplayShape::move(dX, dY);
}

void DSBasic::rotateAbout(const Float& angle, const Float& aboutX, 
			    const Float& aboutY) {
  DisplayShape::rotateAbout(angle, aboutX, aboutY);
}








} //# NAMESPACE CASA - END

