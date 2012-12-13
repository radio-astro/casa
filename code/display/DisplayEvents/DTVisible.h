//# DTVisible.h: class providing draw style settings for visible DisplayTools
//# Copyright (C) 1999,2000
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

#ifndef TRIALDISPLAY_DTVISIBLE_H
#define TRIALDISPLAY_DTVISIBLE_H

#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class providing draw style settings for visible DisplayTools.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "DTVisible" is a contraction and concatenation of "DisplayTool" and 
// "Visible", and provides additional interface features describing 
// drawing options for visible DisplayTools.
// </etymology>

// <synopsis>
// The purpose of this class is to store and provide information
// describing drawing settings for <linkto
// class=DisplayTool>DisplayTools</linkto> which will need to draw on
// a PixelCanvas or WorldCanvas.  Presently it only stores the drawing
// color and line width, and double click interval, but it will 
// probably grow into something
// more sophisticated.  The settings are taken from the users
// <src>.aipsrc</src> file as follows:
//
// <dd> <src>display.controls.color</src>
// <dt> Either a named color (eg. <src>red</src>) can be given, or
// the individual red, green and blue components specified in
// hexadecimal (eg. <src>#f0f033</src>).  The default color is 
// orange.
//
// <dd> <src>display.controls.linewidth</src>
// <dt> An integer should be given specifying the line width 
// to use when drawing DisplayTools.  The default value is one.
//
// <dd> <src>display.controls.doubleclickinterval</src>
// <dt> A real number specifying the time interval in seconds, 
// in which a double click will be deemed to have occurred.  The
// default value is 0.5 seconds.
// </synopsis>

// <example>
// </example>

// <motivation>
// Since many PixelCanvas- and WorldCanvas-based tools (derived from
// <linkto class=PCTool>PCTool</linkto> and <linkto
// class=WCTool>WCTool</linkto> respectively) will
// need to draw on the canvas, it makes sense to unify their selection
// of drawing color and style in one place: this class.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/18">
// None.
// </todo>

class DTVisible {

 public:
  
  // Default constructor.
  DTVisible();

  // Destructor.
  virtual ~DTVisible();

  // Return the color to use.
  virtual String drawColor() const
    { return itsDrawColor; }

  // Return the line width to use.
  virtual Int lineWidth() const
    { return itsLineWidth; }

  // Return the double click interval (in seconds).
  virtual Double doubleClickInterval() const
    { return itsDoubleClickInterval; }

 protected:

  // (Required) copy constructor.
  DTVisible(const DTVisible &other);

  // (Required) copy assignment.
  DTVisible &operator=(const DTVisible &other);

 private:

  // Store the color to use here.
  String itsDrawColor;

  // Store the line width here.
  Int itsLineWidth;

  // Store the double click interval (in seconds) here.
  Double itsDoubleClickInterval;

};


} //# NAMESPACE CASA - END

#endif
