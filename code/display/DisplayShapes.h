//# DisplayShapes.h: The DisplayShapes module - AIPS++ general drawing utils
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2002
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

#ifndef TRIALDISPLAY_DISPLAYSHAPES_H
#define TRIALDISPLAY_DISPLAYSHAPES_H

namespace casa { //# NAMESPACE CASA - BEGIN

// <module>
//
// <summary>
// A module providing classes to draw general shapes onto a pixel canvas.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="None yet" date="yyyy/mm/dd" demos="">
// </reviewed>
//
// <prerequisite>
// <li> <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// <li> <linkto class="Annotations">Annotations</linkto>
// </prerequisite>
//
// <etymology>
// The module provides various classes that are responsible for
// "drawing" "shapes"
// </etymology>
//
// <synopsis>
//
// <h3>Motivation</h3>
//
// The motivation behind creation of a set of "DisplayShapes" was to enable
// a convinient way to draw basic shapes which would be general enough
// to use anywhere in AIPS++. The most obvious application is for use as
// Annotations.
//
// The shape objects themselves draw onto a pixelcanvas whenever it is
// passed to them. The have no event handling facilites, this must be
// handled by an outside class. <linkto class="Annotations">Annotations
// </linkto> is an example of such a class. It adds mouse handling etc
// and manages a list of shapes.
//
// <h3>The DisplayShapes</h3>
//
// DisplayShapes use <linkto class="PixelCanvas">PixelCanvas</linkto>
// primitives to draw themselves. They act as wrappers for primitives,
// and allow every shape to have a common interface. All DisplayShapes,
// for example have a "move" function. Such a function, when combined
// with others such as "inObject", enable mouse interactions to be far
// simpler than they would using pixelcanvas primitives. An interface
// class is also provided, called
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// This class is another layer on top of the DisplayShapes which uses
// records to further simplify the interface between many different types of
// shapes. The overall aim of the DisplayShapes is to present all shapes
// as a single "DisplayShapeInterface" object.
//
// <h3>Adding a new shape </h3>
//
// The first place a new shape must be added is as a class in the display
// shapes directory. This class should inherit from the base class,
// <linkto class="DisplayShape">DisplayShape</linkto>, or some other class
// which in turn inherits from it. Examples of appropriate classes would
// be <linkto class="DSBasic">DSBasic</linkto> or
// <linkto class="DSClosed">DSClosed</linkto>.
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>
// must also be made aware of the new shape, so it can properly construct
// it if requested. If the shape is to be used for annotations, it must
// be added, along with any special event handling requirements to the
// <linkto class="Annotations">Annotations</linkto> class.
//
// <h3>Using the DisplayShapeInterface</h3>
//
// Regardless of how much functionality the shapes share, there will
// always be a need to apply different options for different shapes. The
// information below is intended to outline the options available for all
// displayshapes.
//
// The below listed options are passed via Records to various shapes. This
// occurs either via get/set options, or via the constructor of
// DisplayShapeInterface.
//
//
// General Options :
//
// <center>
// <table>
// <TR>
// <h2> <TD> Record Entry </TD> <TD> Allowed Data </TD> <TD> Desc. </TD> </h2>
// </TR>
// <TR>
//   <TD> hashandles </TD> <TD> Boolean </TD> <TD> Will the object ever have
// handles? No calculations will be done regarding handle position if this is
// false - This is different to drawhandles. hashandles should only be set
// under definate circumstances - i.e. when a shape is being used as a handle
// or for a marker, which will never have a handle.</TD>
// </TR>
// <TR>
//   <TD> drawhandles </TD> <TD> Boolean </TD> <TD> Should handles be drawn
// currently? </TD>
// </TR>
// <TR>
//   <TD> handlecolor </TD> <TD> String </TD> <TD> Specify the color of the
// handles </TD>
// </TR>
// <TR>
//   <TD> handlestyle </TD> <TD> DisplayShape::HandleShape </TD> <TD> The
// of shape to use for the handles </TD>
// </TR>
// <TR>
//   <TD> handlesize </TD> <TD> Int </TD> <TD> The size of the handles in
// pixels (radius / height) </TD>
// </TR>
// <TR>
//   <TD> color </TD> <TD> String </TD> <TD> The color of the displayshape
// object</TD>
// </TR>
// </table>
//
// </center>
// </synopsis>
//
//
// Record fields specific to individual shapes should be documentented in
// their respective header files.
//
// <todo asof='2002/05/13'>
// <li> The "rotateAbout" functionality is currently not implemented
// </todo>
// </module>


} //# NAMESPACE CASA - END

#endif


