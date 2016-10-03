//# WorldCanvas.h: class for drawing in world coordinates on the PixelCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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

#ifndef TRIALDISPLAY_WORLDCANVAS_H
#define TRIALDISPLAY_WORLDCANVAS_H

#include <list>
#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <display/DisplayEvents/PCRefreshEH.h>
#include <display/DisplayEvents/PCMotionEH.h>
#include <display/DisplayEvents/PCPositionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/DisplayEH.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/AttValBase.h>
#include <display/Utilities/DisplayOptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;
	class Colormap;
	class WCCoordinateHandler;
	class WCSizeControlHandler;
	class WCResampleHandler;
	class WCDataScaleHandler;
	class WCPGFilter;
	class DisplayCoordinateSystem;
	class DisplayData;

// <summary>
// Implementation of drawing in world coordinates on top of a PixelCanvas.
//  </summary>
//
// <prerequisite>
// <li> <linkto class="PixelCanvas">PixelCanvas</linkto>
// <li> <linkto class="Attribute">Attribute</linkto>
// <li> <linkto class="AttributeBuffer">AttributeBuffer</linkto>
// </prerequisite>
//
// <etymology>
// WorldCanvas is a canvas on which primitives specified in world
// coordinates are drawn.
// </etymology>
//
// <synopsis>

// The world canvas manages a portion of a <linkto
// class="PixelCanvas">PixelCanvas</linkto> and provides mechanisms to
// allow the user to draw images and vectors in world coordinates.
// The position of the WorldCanvas may be dynamically changed.
//
// The WorldCanvas vector graphics commands assume world coordinates
// are specified and use a registered coordinate handler to map those
// coordinates via a linear system to pixel coordinates.  The pixel
// coordinates are then sent on to the <linkto
// class="PixelCanvas">PixelCanvas</linkto> where they are plotted.
//
// Images are drawn by sending a casacore::Matrix and specifying the
// world-coordinate location of the center of the lower-left pixel.
// If the dataMin and dataMax values have been set, they will be used
// to scale the data, otherwise the min and max will be scanned.
//
// Customizable tools associated with the WorldCanvas
// available to assist with various aspects of display.
// <ul>
// <li> fitting specified regions of an image to a screen pixel array
// (<linkto class="WCResampleHandler">WCResampleHandler</linkto>
// <li> scaling real-world values to integers in preparation for conversion to
//      color indices.
// (<linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto>
// <li> controlling the size of the <linkto class="WorldCanvas">WorldCanvas</linkto> prior
//      to drawing.
// (<linkto class="WCSizeControlHandler">WCSizeControlHandler</linkto>)
// <li> Performing the transformation from WorldCoordinates to LinearCoordinates
// (<linkto class="WCCoordinateHandler">WCCoordinateHandler</linkto>)
// </ul>
//
// The interface for these tools are implemented as abstract base
// classes, and the tools are implemented by deriving from these
// classes and overriding the functions contained therein.
//
// The WorldCanvas provides an interface to the <linkto
// class="PixelCanvas">PixelCanvas</linkto> caching mechanism.  Clever
// use of caching and parsing of the reason field of a <linkto
// class="WCRefreshEvent">WCRefreshEvent</linkto> can reduce
// unnecessary recomputation needed for refreshing the screen.
//
// The WorldCanvas maintains a set of <linkto
// class="Attribute">Attribute</linkto>s which the user can set on the
// WorldCanvas. An Attribute is a combination of a name and a value,
// the value can be of type casacore::uInt, casacore::Int, casacore::Float, casacore::Double, casacore::Bool and casacore::String,
// and Vectors of these. These Attributes serve two purposes: 1) to
// allow to place (using the member functions <src>setAttribute</src>
// and <src> setAttributes</src>) more or less arbitrary information
// on the WorldCanvas that other classes can read from the
// WorldCanvas. Since the name of an Attribute can be defined at run
// time, any name can be used (i.e. there is no pre-defined set of
// names that can only be used), and 2) to have a generic interface to
// some of the internal variables of the WorldCanvas. Some internal
// variabels can be set/read with their own interface routine
// (e.g. linXMin and friends using setZoomRectangleLCS), but they can
// also be modified/read using the Attribute interface (using
// getAttributeValue). The use of e.g. SetZoomRectangleLCS() and
// setAttribute() is equivalent. See <linkto
// class="AttributeBuffer">AttributeBuffer</linkto> for more details.
// The following Attributes (and their types) are pre-defined on the
// WorldCanvas and can be accessed using the Attribute interface
// (note: these Attributes cannot be deleted using removeAttribute()):
// <ul> <li> canvasXOffset (casacore::uInt): The offset of the WorldCanvas on
// the PixelCanvas (X) in 'screen pixels' <li> canvasYOffset (casacore::uInt):
// The offset of the WorldCanvas on the PixelCanvas (Y) in 'screen
// pixels' <li> canvasYSize (casacore::uInt): The size of the WorldCanvas on the
// PixelCanvas (X) in 'screen pixels' <li> canvasYSize (casacore::uInt): The
// size of the WorldCanvas on the PixelCanvas (Y) in 'screen pixels'
// <li> fracXOffset (casacore::Double): The fractional offset of the WorldCanvas
// on the Pixelcanvas (X) <li> fracYOffset (casacore::Double): The fractional
// offset of the WorldCanvas on the Pixelcanvas (Y) <li> fracXSize
// (casacore::Double): The fractional size of the WorldCanvas on the Pixelcanvas
// (X) <li> fracYSize (casacore::Double): The fractional size of the WorldCanvas
// on the Pixelcanvas (Y) <li> linXMin (casacore::Double): The current minimum
// linear coordinate (X) <li> linXMax (casacore::Double): The current maximum
// linear coordinate (X) <li> linYMin (casacore::Double): The current minimum
// linear coordinate (Y) <li> linYMax (casacore::Double): The current maximum
// linear coordinate (Y) <li> linXMinLimit (casacore::Double): The current limit
// on the minimum linear coordinate (X) <li> linXMaxLimit (casacore::Double):
// The current limit on the maximum linear coordinate (X) <li>
// linYMinLimit (casacore::Double): The current limit on the minimum linear
// coordinate (Y) <li> linYMaxLimit (casacore::Double): The current limit on the
// maximum linear coordinate (Y) <li> dataMin (casacore::Double): The data
// minimum of the WorldCanvas <li> dataMax (casacore::Double): The data maximum
// of the WorldCanvas </ul>
//
// The WorldCanvas knows three coordinate systems.
// <ul>
// <li> the Pixel coordinate system. This corresponds to the pixels in the
// PixelCanvas (and usually will correspond to pixels on the screen)
// <li> a Linear Coordinates system. This has to be set by the user of a
// WorldCanvas (like the WorldCanvasHolder). Normally this will correspond to
// the pixel coordinates of the data array that is displayed, but it is up to
// you to define this. The linear coordinate system is the input for the
// coordinate transformation to world coordinates using a
// WCCoordinateHandler. For example, a position event on the PixelCanvas has
// first its pixel coordinates converted to the linear coodinates and then
// these linear coordinates are converted to world coordinates. Zooming is
// defined by setting the linear coordinates to the area to which you want to
// zoom in to (although the zoom area can also be set using world coordinates,
// but these world coordinates are converted to linear coordinates
// internally).
// <li> a WorldCoordinate system. This is defined by the WCCoordinateHandler
// installed on the WorldCanvas. Normally this will map to the Aips++
// coordinate system that belongs to the data displayed, but this is not a
// requirement.  You can define whatever coordiante system you want, as long
// as you satisfy the interface (mainly that you use the linear coordinates of
// the WorldCanvas as input to get to you World coordinates).
// </ul>
// </synopsis>
//
// <motivation>
// Wanted a world-coordinate plotting canvas
// </motivation>
//
// <example>
// See the test programs in Display/test
// </example>
//
// <todo>
// <li> Implement contour drawing in world coordinates.
// <li> Fix clear to work correctly in multiple-WC contexts
// <li> complete value-tagged primitive rendering
// <li> figure out what to do with zooming
// <li> stream functions
// <li> drawImage function(s)
// </todo>

	class WorldCanvas : public PCRefreshEH,
		public PCMotionEH,
		public PCPositionEH,
		public DisplayEH,
		public DisplayOptions {

	public:

		// Construct a WorldCanvas on the given PixelCanvas, at the given
		// origin (in fractions of the PixelCanvas extent), and having the
		// given size (in the same units).
		WorldCanvas(PixelCanvas *pc, casacore::Double xOrigin = 0.0, casacore::Double yOrigin = 0.0,
		            casacore::Double xSize = 1.0, casacore::Double ySize = 1.0);

		// Destructor.
		virtual ~WorldCanvas();

		// Return a pointer to the PixelCanvas on which this WorldCanvas is
		// installed.
		PixelCanvas *pixelCanvas() const {
			return itsPixelCanvas;
		}

		// Add the given refresh, motion and position event handlers to the
		// WorldCanvas.
		// <group>
		void addRefreshEventHandler(DisplayEH &eh);
		void addMotionEventHandler(WCMotionEH &eh);
		void addPositionEventHandler(WCPositionEH &eh);
		// </group>

		// Remove the given refresh, motion and position event handlers from
		// the WorldCanvas.
		// <group>
		void removeRefreshEventHandler(const DisplayEH &eh);
		void removeMotionEventHandler(const WCMotionEH &eh);
		void removePositionEventHandler(const WCPositionEH &eh);
		// </group>

		// Call all of the motion and position event handlers that
		// are installed on the WorldCanvas.
		//# (Similar method for refresh handlers has been made private;
		//# the public method for that is refresh().    dk 4/05)
		// <group>
		void callMotionEventHandlers(const WCMotionEvent &ev);
		void callPositionEventHandlers(const WCPositionEvent &ev);
		// </group>

		// Handle other, generic types of events.  As with call*Handlers above,
		// WC handles these new events by simply distributing them to
		// event handlers registered on it.  However, rather than create
		// any more handler lists in WorldCanvas, generic event handlers
		// (DisplayEHs) piggyback on the RefreshEHList.  WorldCanvas is
		// also a DisplayEH, and all DisplayEHs implement handling of these
		// new events by overriding the (null) base class version
		// of this method.
		virtual void handleEvent(DisplayEvent& ev);

		// Handle implicit refresh, motion and position events occuring on
		// the PixelCanvas on which this WorldCanvas is installed.  These
		// functions translate PixelCanvas events into WorldCanvas events.
		// <group>
		void operator()(const PCRefreshEvent &pev);
		void operator()(const PCMotionEvent &pev);
		void operator()(const PCPositionEvent &pev);
		// </group>

		// Refresh the WorldCanvas for the given reason.  The refresh is
		// prevented from occuring if the PixelCanvas is not yet mapped to
		// the screen, or if the refreshes have been held with earlier
		// call/s to hold() which has/have not been matched with the same
		// number of calls to release().
		void refresh(const Display::RefreshReason &reason = Display::UserCommand,
		             const casacore::Bool &explicitrequest = true);

		// Is a refresh currently allowed?
		casacore::Bool refreshAllowed();

		// Hold and release response to refreshes requested with the
		// <src>refresh()</src> member function.  Multiple calls to
		// <src>hold()</src> can be made, and refreshes will not resume
		// until the same number of calls have been made to
		// <src>release()</src>.  Note that these functions do not affect
		// whether internally (implicitly) generated refreshes continue to
		// occur.  That is, refresh events due to PixelCanvas resize events,
		// or Colormap changes, for example, will continue to be acted upon.
		// <group>
		void hold();
		void release();
		// </group>

		// Set casacore::Coordinate, SizeControl, Resample and DataScale handlers for
		// the WorldCanvas.  If the given handler is 0, then resort to using
		// the default handler.
		// <group>
		void setCoordinateHandler(WCCoordinateHandler *ch);
		void setSizeControlHandler(WCSizeControlHandler *sh);
		void setResampleHandler(WCResampleHandler *rh);
		void setDataScaleHandler(WCDataScaleHandler *sh);
		// </group>

		// Set the location of the WorldCanvas on its PixelCanvas.
		void setWorldCanvasPosition(casacore::Double fracXOffset, casacore::Double fracYOffset,
		                            casacore::Double fracXSize, casacore::Double fracYSize);

		// Pixel, linear and world coordinate transformation functions.  For
		// the casacore::Vector versions, the coordinate mapping returns false if the
		// transformation failed.  For the casacore::Matrix versions, failures(i) on
		// input should be set to true if the i'th transformation should not
		// be attempted.  On output, failures(i) is true if the
		// transformation was not attempted, or failed.  If on input the
		// failures vector has zero length, it will be assumed that no prior
		// failures have occurred.
		// <group>
		casacore::Bool pixToLin(casacore::Vector<casacore::Double> &lin, const casacore::Vector<casacore::Double> &pix);
		casacore::Bool pixToLin(casacore::Matrix<casacore::Double> &lin, casacore::Vector<casacore::Bool> &failures,
		              const casacore::Matrix<casacore::Double> &pix);
		casacore::Bool linToPix(casacore::Vector<casacore::Double> &pix, const casacore::Vector<casacore::Double> &lin);
		casacore::Bool linToPix(casacore::Matrix<casacore::Double> &pix, casacore::Vector<casacore::Bool> &failures,
		              const casacore::Matrix<casacore::Double> &lin);
		casacore::Bool linToWorld(casacore::Vector<casacore::Double> &world, const casacore::Vector<casacore::Double> &lin);
		casacore::Bool linToWorld(casacore::Matrix<casacore::Double> &world, casacore::Vector<casacore::Bool> &failures,
		                const casacore::Matrix<casacore::Double> &lin);
		casacore::Bool worldToLin(casacore::Vector<casacore::Double> &lin, const casacore::Vector<casacore::Double> &world);
		casacore::Bool worldToLin(casacore::Matrix<casacore::Double> &lin, casacore::Vector<casacore::Bool> &failures,
		                const casacore::Matrix<casacore::Double> &world);
		casacore::Bool pixToWorld(casacore::Vector<casacore::Double> &world, const casacore::Vector<casacore::Double> &pix);
		casacore::Bool pixToWorld(casacore::Matrix<casacore::Double> &world, casacore::Vector<casacore::Bool> &failures,
		                const casacore::Matrix<casacore::Double> &pix);
		casacore::Bool worldToPix(casacore::Vector<casacore::Double> &pix, const casacore::Vector<casacore::Double> &world);
		casacore::Bool worldToPix(casacore::Matrix<casacore::Double> &pix, casacore::Vector<casacore::Bool> &failures,
		                const casacore::Matrix<casacore::Double> &world);
		// </group>

		// Register/unregister a Colormap on the PixelCanvas.  Registration
		// counts are remembered, so that a particular Colormap is
		// guaranteed to be available as long as that Colormap has been
		// registered more times than it has been unregistered.  Requests
		// are forwarded to the PixelCanvas.
		// <group>
		void registerColormap(Colormap *cmap, casacore::Float weight = 1.0);
		void registerColormap(Colormap *cmap, Colormap *cmapToReplace) ;
		void unregisterColormap(Colormap *cmap);
		// </group>

		// Set and retrieve the current Colormap on the PixelCanvas.  This
		// function must be called prior to using a WorldCanvas (or
		// PixelCanvas) drawing routines which is expected to use a
		// Colormap.  Passing an unregistered Colormap to setColormap will
		// result in an exception being thrown.  Requests are forwarded to
		// the PixelCanvas.
		// <group>
		void setColormap(Colormap *cmap);
		Colormap *colormap() const;
		// </group>

		// Display list support functions.  A display list is started with a
		// call to <src>newList()</src>, finished with a call to
		// <src>endList()</src>, and drawn with a call to
		// <src>drawList(x)</src>, with the argument <src>x</src> being the
		// list number returned by the original call to
		// <src>newList()</src>.  Lists can be deleted individually with
		// <src>deleteList(x)</src> or in total with
		// <src>deleteLists()</src>.  Requests are forwarded to the
		// PixelCanvas.
		// <group>
		casacore::uInt newList();
		void endList();
		void drawList(casacore::uInt list);
		void deleteList(casacore::uInt list);
		void deleteLists();
		casacore::Bool validList(casacore::uInt list);
		// </group>

		// Set various graphics attributes.  All of these requests are
		// passed directly to the PixelCanvas, except for
		// <src>setColor</src>, which also installs the requested color for
		// subsequent calls to PgPlot functions.
		// <group>
		void setColor(const casacore::String &color);
		void setClearColor(const casacore::String &color);
		casacore::Bool setFont(const casacore::String &fontName);
		void setForeground(casacore::uLong color);
		void setBackground(casacore::uLong color);
		void setLineWidth(casacore::Float width);
		void setLineStyle(Display::LineStyle style);
		void setCapStyle(Display::CapStyle style);
		void setJoinStyle(Display::JoinStyle style);
		void setFillStyle(Display::FillStyle style);
		void setFillRule(Display::FillRule rule);
		void setArcMode(Display::ArcMode mode);
		// </group>

		// Set/retrieve the background and foreground colors of the
		// WorldCanvas.  These can be different to those for the
		// PixelCanvas.  Indeed, they will be used as default colors on the
		// WorldCanvas when necessary.
		// <group>
		casacore::Bool setWorldBackgroundColor(const casacore::String color);
		casacore::Bool setWorldForegroundColor(const casacore::String color);
		casacore::String getWorldBackgroundColor() {
			return itsWorldBackgroundColor;
		}
		casacore::String getWorldForegroundColor() {
			return itsWorldForegroundColor;
		}
		// </group>

		// Set/retrieve the drawing buffer, the target destination for
		// graphics.  Requests are passed directly to the PixelCanvas.
		// <group>
		virtual void setDrawBuffer(Display::DrawBuffer buf);
		Display::DrawBuffer drawBuffer() const;
		// </group>

		// Set/retrieve the caching strategy on the PixelCanvas.
		// Appropriate values are Display::ClientAlways (use client memory
		// to cache images [safer]), Display::ServerAlways (use server
		// memory to cache images [faster]), and
		// Display::ServerMemoryThreshold (use server memory until a
		// threshold is reached [not yet implemented]).  Requests are passed
		// to the PixelCanvas.
		// <group>
		void setImageCacheStrategy(Display::ImageCacheStrategy strategy);
		Display::ImageCacheStrategy imageCacheStrategy() const;
		// </group>

		// Clear the WorldCanvas, or just the area on the WorldCanvas
		// but outside the drawing area, ie. the margins that are normally
		// reserved for axis labels and the like.
		// <group>
		void clear();
		void clearNonDrawArea();
		// </group>

		// Install the default options for this DisplayData
		virtual void setDefaultOptions();

		// Apply options stored in rec to the DisplayData; return value true
		// means a refresh is needed.  Any fields added to the
		// updatedOptions argument are options which have changed in some
		// way due to the setting of other options - ie. they are context
		// sensitive.
		virtual casacore::Bool setOptions(const casacore::Record &rec, casacore::Record &updatedOptions);

		// Retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions() const;

		// Set an Attribute or Attributes on the WorldCanvas.
		// <group>
		void setAttribute(Attribute& at);
		void setAttributes(AttributeBuffer& at);
		// </group>

		// Remove an Attribute.  Pre-defined Attributes of the WorldCanvas
		// cannot be removed (although nothing serious will happen if you
		// try).
		void removeAttribute(casacore::String& name);

		// User interface to get individual values from the attribute buffer.
		// <group>
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::uInt& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Int& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Float& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Double& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Bool& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::String& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::uInt>& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::Int>& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::Float>& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::Double>& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::Bool>& newValue) const;
		casacore::Bool getAttributeValue(const casacore::String& name, casacore::Vector<casacore::String>& newValue) const;
		// </group>

		// Check if a certain Attribute exists.
		casacore::Bool existsAttribute(casacore::String& name) const;

		// Get the type of an Attribute.
		AttValue::ValueType attributeType(casacore::String& name) const;

		// Position the PGPLOT filter on the WorldCanvas.  If linear is
		// specified false, then the alignment is done by world coordinates,
		// assuming that a linear approximation is valid.
		// void verifyPGFilterAlignment(const casacore::Bool &linear = true);

		// Acquire and release a PGPLOT device for this WorldCanvas.  This
		// is necessary since PGPLOT generally only supports 8 currently
		// active devices.  So refresh cycles on the WorldCanvas acquire a
		// PGPLOT device at the start, and release it at the end.  Cycles
		// are counted so that external user-classes can call these functions
		// if necessary in a nested state.  If <src>linear</src> is specified
		// as <src>false</src>, then the PGPLOT device is aligned by world
		// coordinates, under the assumption that a linear approximation is
		// valid, that is, the curvature is small.
		// <group>
		virtual void acquirePGPLOTdevice(const casacore::Bool &linear = true);
		virtual void releasePGPLOTdevice();
		// </group>

		// Return the PGPLOT device id for external use.
		virtual casacore::Int pgid() const;

		// Draw unrotated text at the given position.  If the conversion
		// from world to pixel coordinates fails, the text is not drawn, and
		// false is returned.  If linear is true, then the provided position
		// is actually in linear world canvas coordinates, rather than true
		// world coordinates.
		casacore::Bool drawText(const casacore::Vector<casacore::Double> &point, const casacore::String &text,
		              Display::TextAlign alignment = Display::AlignCenter,
		              const casacore::Bool &linear = false);

		// Draw a single point using the current color.  If the conversion
		// from world to pixel coordinates fails, the point is not drawn,
		// and false is the return value.  If linear is true, then the point
		// position is given in linear world canvas coordinates, not true
		// world coordinates.
		casacore::Bool drawPoint(const casacore::Vector<casacore::Double> &point, const casacore::Bool &linear = false);

		// Draw a single line using the current color.  If either of the
		// conversions from world to pixel coordinates fail, then the line
		// is not drawn, and false is returned.  If linear is true, then the
		// line endpoints are given in world canvas linear coordinates
		// rather than real world coordinates.
		casacore::Bool drawLine(const casacore::Vector<casacore::Double> &a, const casacore::Vector<casacore::Double> &b,
		              const casacore::Bool &linear = false);

		// Draw a bunch of points using the current color.  If any points
		// fail to convert then none of them are drawn, and false is
		// returned.  If linear is true, then the vertices are given in
		// linear world canvas coordinates rather than real world
		// coordinates.
		casacore::Bool drawPoints(const casacore::Matrix<casacore::Double> &points, const casacore::Bool &linear = false);

		// Draw a set of points using the current color.  Those points which
		// fail to convert, or lie outside the WorldCanvas drawing area, are
		// not drawn.
		casacore::Bool drawPoints(const casacore::Vector<casacore::Float> &px, const casacore::Vector<casacore::Float> &py,
		                casacore::Bool linear = false);

		// Draw a set of text strings using the current color.  If any
		// points fail to convert, then those particular strings are not
		// drawn.  <src>rotation</src> gives the rotation of the text in
		// degrees counter-clockwise from horizontal.  <src>xoffset</src>
		// and <src>yoffset</src> can be given to globally shift the labels
		// by the specified amounts (in units of the character height).  If
		// linear is true, then the vertices are given in linear world
		// canvas coordinates rather than true world coordinates.
		casacore::Bool drawTextStrings(const casacore::Vector<casacore::Float> &px, const casacore::Vector<casacore::Float> &py,
		                     const casacore::Vector<casacore::String> &strings,
		                     const casacore::Float rotation = 0.0,
		                     const casacore::Float xoffset = 0.0,
		                     const casacore::Float yoffset = 0.0,
		                     const casacore::Bool linear = false);

		// Draw a set of markers using the current color and a given pixel
		// <src>size</src>. If any points fail to convert, then those
		// particular points are not marked.  <src>markertype</src> is an
		// <src>Display::Marker</src>. If linear is true, then the points
		// are given in linear world canvas coordinates rather than true
		// world coordinates.
		casacore::Bool drawMarkers(const casacore::Vector<casacore::Float> &px, const casacore::Vector<casacore::Float> &py,
		                 const Display::Marker = Display::Cross, const casacore::Int size = 5,
		                 const casacore::Bool& linear = false);

		casacore::Bool drawMappedMarkers(const casacore::Vector<casacore::Float> &px, const casacore::Vector<casacore::Float> &py,
		                       const casacore::Vector<casacore::Float>& values,
		                       const casacore::Int sizemin = 1, const casacore::Int sizemax = 20,
		                       const Display::Marker = Display::Cross,
		                       const casacore::Bool& linear = false);

		// Draw pairs of lines using the current color.  If any points fail
		// to convert then the lines are not drawn and false is returned.
		// If linear is true, then the vertices are given as linear world
		// coordinates rather than true world coordinates.
		casacore::Bool drawLines(const casacore::Matrix<casacore::Double> &vertices, const casacore::Bool &linear = false);


		// Draw a polyline (connected line) between the vertices using the
		// current color.  If any coordinates fail to convert from world to
		// pixel, then the entire polyline is not drawn and false is
		// returned.  The end point is not implicitly connected to the
		// starting point.  If linear is true, then the provided vertices
		// are actually linear world canvas coordinates.
		casacore::Bool drawPolyline(const casacore::Matrix<casacore::Double> &vertices,
		                  const casacore::Bool &linear = false);

		// Draw a polygon (closed line, or line loop using the points)
		// using the current color.  If any coordinates fail to convert
		// then the polygon is not drawn.  The end point is implicitly
		// connected to the start point.  If linear is true, then the
		// provided vertices are actually linear world coordinates.
		casacore::Bool drawPolygon(const casacore::Matrix<casacore::Double> &vertices,
		                 const casacore::Bool &linear = false);

		// Draw a set of points in colors which are taken from the current
		// Colormap.
		casacore::Bool drawColormappedPoints(const casacore::Matrix<casacore::Double> &points,
		                           const casacore::Vector<casacore::Float> &values,
		                           const casacore::Bool &linear = false);

		// Draw a set of colored ellipses, possibly with outlines.  The x
		// and y locations must given, along with semi-major and semi-minor
		// axes, and position angle measured in degrees positive from the x
		// axis in a counter-clockwise direction.  The size of the ellipses
		// is globally scaled by the scale factor, and if <src>outline</src>
		// is <src>true</src>, then each ellipse will have an outline in the
		// current pen color.
		casacore::Bool drawColormappedEllipses(const casacore::Matrix<casacore::Double> &centres,
		                             const casacore::Vector<casacore::Float> &smajor,
		                             const casacore::Vector<casacore::Float> &sminor,
		                             const casacore::Vector<casacore::Float> &pangle,
		                             const casacore::Vector<casacore::Float> &colors,
		                             const casacore::Float &scale = 1.0,
		                             const casacore::Bool &outline = true,
		                             const casacore::Bool &linear = false);


		// This routine is specialized for drawing image restoring-beam ellipses.
		// Its parameters are defined so as to require as little conversion as
		// possible of restoring beam information as stored in an image header.
		//
		// It does nothing unless the axes on display map to the two axes of a
		// casacore::DirectionCoordinate within the WC CS (WorldCanvas::itsDisplayCoordinateSystem).
		// Center location cx,cy is specified as a fraction of WC draw area:
		// (0,0) is blc, (1,1) is trc; they default to (.1, .1).
		//
		// The unit strings for major,minor are given in in majunit, minunit, and
		// should be valid angular units (they default to "arcsec").  major/minor
		// are the _full_ major and minor axis sizes in terms of relative direction
		// world coordinates.
		//
		// pa specifies "position angle", in the angular units specified by paunit.
		// pa uses the image header convention "North-to-East"; more precisely,
		// 0 degrees aligns the major axis along increasing DirCoord(1) (commonly
		// Dec), 90 degrees aligns it along increasing DirCoord(0) (commonly RA).
		// (NB: increasing RA commonly means decreasing pixel/casacore::Lattice coordinates).
		// In the common case, this means that pa increases counterclockwise from
		// vertical.  Note that this is _not_ the pa convention in some other
		// PixelCanvas/WorldCanvas drawEllipse() routines (where pa is always
		// counterclockwise from horizontal).
		//
		// Also note: this routine attempts to do the right thing in oddball cases
		// such as displaying Dec on the horizontal axis (pa 0 would also be
		// horizontal in that case), distorted ('flexible') aspect ratio (ellipse
		// also distorted appropriately) or all-sky images in which the beam may be
		// displayed in a canvas area where absolute world coordinates do not exist.
		// It should even take care of uneven coordinate 'increments' (non-square
		// image data pixels).
		// (So far, it does _not_ correctly handle non-identity transformation
		// matrices in the casacore::DirectionCoordinate (e.g. rotated "North")-- which I
		// think is true in many other places as well... -- for someone's to-do list
		// (not mine, I hope)).
		casacore::Bool drawBeamEllipse(casacore::Float major, casacore::Float minor,  casacore::Float pa,
		                     casacore::String majunit="arcsec", casacore::String minunit="arcsec",
		                     casacore::String paunit="deg",
		                     casacore::Float cx=.1f, casacore::Float cy=.1f, casacore::Bool outline=true);


		// Draw a contour map at the specified levels, and place the lower
		// left pixel at blPos, and the upper right pixel at trPos.  If
		// <src>usePixelEdges</src> is true, then the given world positions
		// are the position of the blc and trc of the blc and trc pixels,
		// otherwise they are the positions of the centres of the pixels.
		// Note that the contours are not intrinsically drawn in world
		// coordinates.  For complex data, the conversion to real values is
		// done according to the last call to setComplexToRealMethod.
		// Returns true if OK, false if error...
		// <group>
		bool drawContourMap(const casacore::Vector<casacore::Double> &blPos,
		                    const casacore::Vector<casacore::Double> &trPos,
		                    const casacore::Matrix<casacore::Float> &data,
		                    const casacore::Vector<casacore::Float> &levels,
		                    const casacore::Bool usePixelEdges = false);


		bool drawContourMap(const casacore::Vector<casacore::Double> &blPos,
		                    const casacore::Vector<casacore::Double> &trPos,
		                    const casacore::Matrix<casacore::Complex> &data,
		                    const casacore::Vector<casacore::Float> &levels,
		                    const casacore::Bool usePixelEdges = false);
		// </group>

		// Draw a contour map at the specified levels, and place the lower
		// left pixel at blPos, and the upper right pixel at trPos.  If
		// <src>usePixelEdges</src> is true, then the given world positions
		// are the position of the blc and trc of the blc and trc pixels,
		// otherwise they are the positions of the centres of the pixels.
		// Note that the contours are not intrinsically drawn in world
		// coordinates.  For complex data, the conversion to real values is
		// done according to the last call to setComplexToRealMethod.  These
		// functions also have a <src>mask</src> argument, which is a
		// Boolean pixel mask whose shape must match that of
		// <src>data</src>, and only pixels in <src>data</src> where
		// corresponding pixels in <src>mask</src> are <src>true</src> will
		// be contoured.
		// Returns true if OK, false if error...
		// <group>
		bool drawContourMap(const casacore::Vector<casacore::Double> &blPos,
		                    const casacore::Vector<casacore::Double> &trPos,
		                    const casacore::Matrix<casacore::Float> &data,
		                    const casacore::Matrix<casacore::Bool> &mask,
		                    const casacore::Vector<casacore::Float> &levels,
		                    const casacore::Bool usePixelEdges = false);
		bool drawContourMap(const casacore::Vector<casacore::Double> &blPos,
		                    const casacore::Vector<casacore::Double> &trPos,
		                    const casacore::Matrix<casacore::Complex> &data,
		                    const casacore::Matrix<casacore::Bool> &mask,
		                    const casacore::Vector<casacore::Float> &levels,
		                    const casacore::Bool usePixelEdges = false);
		// </group>

		// Optimization to speed up colormap fiddling in 24bit mode (software
		// Colormap); see images_, below for usage.  Set opaqueMask to true to
		// draw masked pixels in the background color; otherwise they will be
		// transparent (letting whatever was drawn previously at that point show
		// through).
		casacore::Bool redrawIndexedImage(void* drawObj, Display::RefreshReason reason,
		                        casacore::Bool opaqueMask=false);

		// Remove image from the colormap change cache, if any (see images_, below).
		// Return value indicates whether there was anything to remove.
		casacore::Bool removeIndexedImage(void* drawObj);

		// Clear the whole colormap change cache (see images_, below).
		void clearColormapChangeCache();

		// Draw an image, mapping data values to Colormap entries, and place
		// the lower left pixel at blPos, and the upper right pixel at
		// trPos.  If <src>usePixelEdges</src> is true, then the given world
		// positions are the position of the blc and trc of the blc and trc
		// pixels, otherwise they are the positions of the centres of the
		// pixels.
		// See images_, below, for non-default usage of the drawObj parameter.
		// <group>
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Float> &data, const casacore::Bool usePixelEdges = false,
		               void* drawObj=0);
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Complex> &data,
		               const casacore::Bool usePixelEdges = false, void* drawObj=0);
		void drawImage(const casacore::Vector<casacore::Double> &blc,const casacore::Vector<casacore::Double> &trc, const casacore::Matrix<casacore::Float> &data,
			           const casacore::Matrix<casacore::Float> &dataRed, const casacore::Matrix<casacore::Float> &dataGreen,
			           const casacore::Matrix<casacore::Float> &dataBlue,const casacore::Bool usePixelEdges,void* drawObj = 0);
		void drawImage(const casacore::Vector<casacore::Double> &blc,const casacore::Vector<casacore::Double> &trc, const casacore::Matrix<casacore::Complex> &data,
					   const casacore::Matrix<casacore::Complex> &dataRed, const casacore::Matrix<casacore::Complex> &dataGreen,
					   const casacore::Matrix<casacore::Complex> &dataBlue,const casacore::Bool usePixelEdges,void* drawObj = 0);
		// </group>

		// Draw an image, mapping data values to Colormap entries, and place
		// the lower left pixel at blPos, and the upper right pixel at
		// trPos.  If <src>usePixelEdges</src> is true, then the given world
		// positions are the position of the blc and trc of the blc and trc
		// pixels, otherwise they are the positions of the centres of the
		// pixels.  These functions also have a <src>mask</src> argument,
		// which is a Boolean pixel mask whose shape must match that of
		// <src>data</src>, and only pixels in <src>data</src> where
		// corresponding pixels in <src>mask</src> are <src>true</src>
		// will be drawn.  Set opaqueMask to true to draw masked pixels in
		// the background color; otherwise they will be transparent (letting
		// whatever was drawn previously at that point show through).
		// See images_, below, for non-default usage of the drawObj parameter.
		// <group>
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Float> &data, const casacore::Matrix<casacore::Bool> &mask,
		               const casacore::Bool usePixelEdges = false, void* drawObj=0,
		               casacore::Bool opaqueMask=false);
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Complex> &data, const casacore::Matrix<casacore::Bool> &mask,
		               const casacore::Bool usePixelEdges = false, void* drawObj=0,
		               casacore::Bool opaqueMask=false);

		// </group>
		// Draw a component of a multi-channel image, mapping data values to
		// component levels, and place the lower left pixel at blPos, and
		// the upper right pixel at trPos.  If <src>usePixelEdges</src> is
		// true, then the given world positions are the position of the blc
		// and trc of the blc and trc pixels, otherwise they are the
		// positions of the centres of the pixels.  The components are not
		// drawn until flushComponentImages() is called.
		// <group>
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Float> &data,
		               const Display::ColorComponent &colorcomponent,
		               const casacore::Bool usePixelEdges = false);
		void drawImage(const casacore::Vector<casacore::Double> &blPos, const casacore::Vector<casacore::Double> &trPos,
		               const casacore::Matrix<casacore::Complex> &data,
		               const Display::ColorComponent &colorcomponent,
		               const casacore::Bool usePixelEdges = false);
		// </group>

		// Draw a vector map.
		// <group>
		bool drawVectorMap(const casacore::Vector<casacore::Double>& blc,
		                   const casacore::Vector<casacore::Double>& trc,
		                   const casacore::Matrix<casacore::Complex>& data,
		                   const casacore::Matrix<casacore::Bool>& mask,
		                   casacore::Float angleConversionFactor,
		                   casacore::Float phasePolarity,
		                   casacore::Bool debias, casacore::Float variance,
		                   casacore::Int xPixelInc, casacore::Int yPixelInc,
		                   casacore::Float scale, casacore::Bool arrow, casacore::Float barb,
		                   casacore::Float rotation,
		                   casacore::Double xWorldInc, casacore::Double yWorldInc,
		                   const casacore::Bool usePixelEdges);

		bool drawVectorMap(const casacore::Vector<casacore::Double>& blc,
		                   const casacore::Vector<casacore::Double>& trc,
		                   const casacore::Matrix<casacore::Float>& data,
		                   const casacore::Matrix<casacore::Bool>& mask,
		                   casacore::Float angleConversionFactor,
		                   casacore::Float phasePolarity,
		                   casacore::Bool debias, casacore::Float variance,
		                   casacore::Int xPixelInc, casacore::Int yPixelInc,
		                   casacore::Float scale, casacore::Bool arrow, casacore::Float barb,
		                   casacore::Float rotation,
		                   casacore::Double xWorldInc, casacore::Double yWorldInc,
		                   const casacore::Bool usePixelEdges);
		// </group>

// Draw marker maps.  Only makerType "square" available presently.
// The marker holds its shape in screen pixel coordinates.  This
// means a square is always a square regardless of aspect ratio
// Returns true if OK, false if error...
// <group>
		bool drawMarkerMap(const casacore::Vector<casacore::Double>& blc,
		                   const casacore::Vector<casacore::Double>& trc,
		                   const casacore::Matrix<casacore::Float>& data,
		                   const casacore::Matrix<casacore::Bool>& mask,
		                   casacore::Int xPixelInc, casacore::Int yPixelInc,
		                   casacore::Float scale, casacore::Double xWorldInc, casacore::Double yWorldInc,
		                   const casacore::String& markeType,
		                   casacore::Bool usePixelEdges);
		bool drawMarkerMap(const casacore::Vector<casacore::Double>& blc,
		                   const casacore::Vector<casacore::Double>& trc,
		                   const casacore::Matrix<casacore::Complex>& data,
		                   const casacore::Matrix<casacore::Bool>& mask,
		                   casacore::Int xPixelInc, casacore::Int yPixelInc,
		                   casacore::Float scale, casacore::Double xWorldInc, casacore::Double yWorldInc,
		                   const casacore::String& markerType,
		                   casacore::Bool usePixelEdges);
// </group>


		// Flush the component images, ie. compose a single image from the
		// buffered channel images and place it on the WorldCanvas.  This
		// effectively is passed on to the PixelCanvas where the component
		// images are cached.
		void flushComponentImages();

		// Buffer memory exchanges which operate only on the area of the
		// WorldCanvas.  (Not cacheable yet.)
		// <group>
		void copyBackBufferToFrontBuffer();
		void copyFrontBufferToBackBuffer();
		void swapBuffers();
		// </group>

		// Provide information on the extents of the linear coordinate
		// system.
		//  <group>
		casacore::Double linXMin() const {
			return itsLinXMin;
		}
		casacore::Double linYMin() const {
			return itsLinYMin;
		}
		casacore::Double linXMax() const {
			return itsLinXMax;
		}
		casacore::Double linYMax() const {
			return itsLinYMax;
		}
		// </group>

		// Provide information on the limits of the linear coordinate
		// system.
		// <group>
		casacore::Double linXMinLimit() const {
			return itsLinXMinLimit;
		}
		casacore::Double linYMinLimit() const {
			return itsLinYMinLimit;
		}
		casacore::Double linXMaxLimit() const {
			return itsLinXMaxLimit;
		}
		casacore::Double linYMaxLimit() const {
			return itsLinYMaxLimit;
		}
		// </group>

		// Provide information on the WorldCanvas offset and size.
		// <group>
		casacore::uInt canvasXOffset() const {
			return itsCanvasXOffset;
		}
		casacore::uInt canvasYOffset() const {
			return itsCanvasYOffset;
		}
		casacore::uInt canvasXSize() const {
			return itsCanvasXSize;
		}
		casacore::uInt canvasYSize() const {
			return itsCanvasYSize;
		}
		// </group>

		// Provide information on the WorldCanvas drawable offset and size.
		// <group>
		casacore::uInt canvasDrawXOffset() const {
			return itsCanvasDrawXOffset;
		}
		casacore::uInt canvasDrawYOffset() const {
			return itsCanvasDrawYOffset;
		}
		casacore::uInt canvasDrawXSize() const {
			return itsCanvasDrawXSize;
		}
		casacore::uInt canvasDrawYSize() const {
			return itsCanvasDrawYSize;
		}
		// </group>

		// Set the zoom rectangle to the specfied linear coordinate range.
		void setZoomRectangleLCS(const casacore::Vector<casacore::Double> &min,
		                         const casacore::Vector<casacore::Double> &max);

		// Move the zoom rectangle across the screen, ie. pan.
		void moveZoomRectangleLCS(double dx, double dy);

		// Reset the zoom to show the entire allowable range of the linear
		// coordinate system.
		void resetZoomRectangle();

		// Set the allowable range of the linear coordinate system.
		void setLinearCoordinateSystem(const casacore::Vector<casacore::Double> &blc,
		                               const casacore::Vector<casacore::Double> &trc,
		                               casacore::Bool resetZoom = true);

		// Functions to set and retrieve the minimum and maximum data values
		// for scaling data that is drawn on the WorldCanvas.  These values
		// are forwarded to the scale handler, when, for example, images are
		// drawn.
		// <group>
		casacore::Double dataMin() const {
			return itsDataMin;
		}
		void setDataMin(casacore::Double min) {
			itsDataMin = min;
		}
		casacore::Double dataMax() const {
			return itsDataMax;
		}
		void setDataMax(casacore::Double max) {
			itsDataMax = max;
		}
		void setDataMinMax(casacore::Double min, casacore::Double max) {
			itsDataMin = min;
			itsDataMax = max;
		}
		// </group>

		// ComplexToRealMethod defines which real component of a Complex
		// image to extract when it is necessary to convert casacore::Complex data
		// into real data.
		// <group>
		Display::ComplexToRealMethod complexToRealMethod() const {
			return itsComplexToRealMethod;
		}
		void setComplexToRealMethod(const Display::ComplexToRealMethod method) {
			itsComplexToRealMethod = method;
		}
		// </group>

		// Set and retrieve the DisplayCoordinateSystem of this WorldCanvas.  Set
		// with 0 and the WorldCanvas loses its DisplayCoordinateSystem!
		//#
		//#dk note 9/07 -- Yes, isn't that exciting, esp. since subsequent calls
		//# to coordinateSystem() will SEGV, and no method was provided to
		//# even test for it....  hasCS() added to somewhat bandage over this
		//# hazard...
		// <group>
		void setCoordinateSystem(const DisplayCoordinateSystem &csys);
		const DisplayCoordinateSystem &coordinateSystem() const;
		casacore::Bool hasCS() const {
			return itsCoordinateSystem!=0;
		}
		// </group>

		// Convenience functions returning whether a pixel coordinate is
		// within bounds of the WC's inner draw area, the WC, or the underlying PC.
		// <group>

		casacore::Bool inDrawArea(casacore::Int x, casacore::Int y) const {
			casacore::Int x0 = itsCanvasXOffset + itsCanvasDrawXOffset;
			casacore::Int x1 = x0 + itsCanvasDrawXSize;
			casacore::Int y0 = itsCanvasYOffset + itsCanvasDrawYOffset;
			casacore::Int y1 = y0 + itsCanvasDrawYSize;
			return  x>=x0 && x<x1  &&  y>=y0  && y<y1 ;
		}

		casacore::Bool inWC(casacore::Int x, casacore::Int y) const {
			casacore::Int x0 = itsCanvasXOffset;
			casacore::Int x1 = x0 + itsCanvasXSize;
			casacore::Int y0 = itsCanvasYOffset;
			casacore::Int y1 = y0 + itsCanvasYSize;
			return  x>=x0 && x<x1  &&  y>=y0  && y<y1 ;
		}

		casacore::Bool inPC(casacore::Int x, casacore::Int y);

		// </group>


		// Install a single restriction, or a buffer of restrictions, on the
		// WorldCanvas which DisplayData must match in order that they
		// be allowed to draw themselves.
		// <group>
		void setRestriction(const Attribute& restriction);
		void setRestrictions(const AttributeBuffer& resBuff);
		// </group>

		// Check if a named restriction exists.
		casacore::Bool existRestriction(const casacore::String& name) const;

		// Remove the named restriction, or all restrictions, from the
		// WorldCanvas.
		// <group>
		void removeRestriction(const casacore::String& restrictionName);
		void removeRestrictions();
		// </group>

		// Determine whether the restrictions installed on the
		// WorldCanvas match the given restriction or buffer of
		// restrictions.
		// <group>
		casacore::Bool matchesRestriction(const Attribute& restriction) const;
		casacore::Bool matchesRestrictions(const AttributeBuffer& buffer) const;
		// </group>

		// Return the buffer of restrictions installed on this
		// WorldCanvas.
		const AttributeBuffer *restrictionBuffer() const;
		// convienience function based on "restriction buffer"...
		int zIndex( ) const;

		// The DD in charge of setting WC coordinate state (0 if none).
		const DisplayData *csMaster() const {
			return itsCSmaster;
		}
		DisplayData *&csMaster() {
			return itsCSmaster;
		}

		// Is the specified DisplayData the one in charge of WC state?
		// (During DD::sizeControl() execution, it means instead that the
		// DD has permission to become CSmaster, if it can).
		casacore::Bool isCSmaster(const DisplayData *dd) const {
			return dd==csMaster() && dd!=0;
		}

		bool removeDD( const DisplayData *dd ) {
			bool result = (dd == csMaster( ));
			if ( result ) itsCSmaster = 0;
			return result;
		}

		// Return the names and units of the world coordinate axes.
		// <group>
		virtual casacore::Vector<casacore::String> worldAxisNames() const;
		virtual casacore::Vector<casacore::String> worldAxisUnits() const;
		// </group>

		const std::list<DisplayData*> &displaylist( ) const;

		static const casacore::String LEFT_MARGIN_SPACE_PG;
		static const casacore::String RIGHT_MARGIN_SPACE_PG;
		static const casacore::String BOTTOM_MARGIN_SPACE_PG;
		static const casacore::String TOP_MARGIN_SPACE_PG;

		std::string errorMessage( ) const { return error_string; }

	private:


		// Support for construction.
		void ctorInit();

		// Call all registered refresh handlers (public method is refresh()).
		void callRefreshEventHandlers(const WCRefreshEvent &ev);


		// Update canvas sizes/offsets from the fracOffsets, and vice versa.
		// <group>
		void updateCanvasSizesOffsets();
		void updateFracSizesOffsets();
		// </group>

		// Convert the given coordinate/s to pixel coordinates.  If linear
		// is true, then the given coordinates are in linear world coordinates,
		// otherwise they are real world coordinates.  A return value of false
		// indicates the conversion failed.
		// <group>
		casacore::Bool castingConversion(casacore::Vector<casacore::Int> &pixelpt, const casacore::Vector<casacore::Double> &worldpt,
		                       const casacore::Bool &linear);
		casacore::Bool castingConversion(casacore::Matrix<casacore::Int> &pixelpts, const casacore::Matrix<casacore::Double> &worldpts,
		                       const casacore::Bool &linear);
		casacore::Bool castingConversion(casacore::Matrix<casacore::Float> &pixelpts,
		                       const casacore::Matrix<casacore::Double> &worldpts,
		                       const casacore::Bool &linear);
		// </group>

		// Convert the given coordinate/s to pixel coordinates.  If
		// <src>linear</src> is <src>true</src>, then the input coordinates
		// are linear world coordinates, otherwise they are true world
		// coordinates.  This version applies clipping, so that any points
		// in the series which lie outside the linear coordinate range of
		// the WorldCanvas are discarded.  Thus the output Vectors can be
		// shorter than the input Vectors.  A mask indicating which of the
		// input points were valid is returned for user reference.
		casacore::Bool castingClippingConversion(casacore::Vector<casacore::Int> &pixelx, casacore::Vector<casacore::Int> &pixely,
		                               casacore::Vector<casacore::Bool> &validConversions,
		                               const casacore::Vector<casacore::Float> &worldx,
		                               const casacore::Vector<casacore::Float> &worldy,
		                               const casacore::Bool linear);

		// Actually draw the vector field.   All pa * angleConversionFactor
		// must be radians.  rotation  must be radians.  if amp is of
		// dimension 0, amplitude unity is assumed.  if mask if dimension
		// 0 all data are assumed good.
		bool drawVectorMap(const casacore::Vector<casacore::Double>& blc,
		                   const casacore::Vector<casacore::Double>& trc,
		                   const casacore::Matrix<casacore::Float>& amp,
		                   const casacore::Matrix<casacore::Float>& pa,
		                   const casacore::Matrix<casacore::Bool>& mask,
		                   casacore::Float angleConversionFactor,
		                   casacore::Float phasePolarity,
		                   casacore::Bool debias, casacore::Float variance,
		                   casacore::Int xPixelInc, casacore::Int yPixelInc,
		                   casacore::Float scale, casacore::Bool arrow, casacore::Float barb, casacore::Float rotation,
		                   casacore::Double xWorldInc, casacore::Double yWorldInc,
		                   const casacore::Bool usePixelEdges);


		// Trim and resample an image, returning the actual world BLC and
		// TRC for drawing, and the resampled image.
		void trimAndResampleImage(casacore::Vector<casacore::Double> &drawBlc,
		                          casacore::Vector<casacore::Double> &drawTrc,
		                          casacore::Matrix<casacore::Float> &sampledImage,
		                          const casacore::Vector<casacore::Double> &blc,
		                          const casacore::Vector<casacore::Double> &trc,
		                          const casacore::Matrix<casacore::Float> &data,
		                          const casacore::Bool &usePixelEdges = false);

		// Trim and resample an image, returning the actual world BLC and
		// TRC for drawing, and the resampled image.  This function takes
		// a mask indicating which pixels should be drawn.  This function
		// therefore also trims and resamples the mask.
		void trimAndResampleImage(casacore::Vector<casacore::Double> &drawBlc,
		                          casacore::Vector<casacore::Double> &drawTrc,
		                          casacore::Matrix<casacore::Float> &sampledImage,
		                          casacore::Matrix<casacore::Bool> &resampledMask,
		                          const casacore::Vector<casacore::Double> &blc,
		                          const casacore::Vector<casacore::Double> &trc,
		                          const casacore::Matrix<casacore::Float> &data,
		                          const casacore::Matrix<casacore::Bool> &mask,
		                          const casacore::Bool &usePixelEdges = false);

		// Draw an image where <src>scaledImage</src> gives the Colormap
		// index of each screen (PixelCanvas) pixel.  'blc' contains X and Y
		// PixelCanvas coordinates.  Masked version as well.
		// For masked version, set opaqueMask to true to draw masked pixels in
		// the background color; otherwise they will be transparent (letting
		// whatever was drawn previously at that point show through).
		// <group>
		void mapToColorAndDrawImage(const casacore::Vector<casacore::Int> &blc,
		                            const casacore::Matrix<casacore::uInt> &scaledImage);
		void mapToColorAndDrawImage(const casacore::Vector<casacore::Int> &blc,
		                            const casacore::Matrix<casacore::uInt> &scaledImage,
		                            const casacore::Matrix<casacore::Bool> &mask,
		                            casacore::Bool opaqueMask=false);
		// </group>
		// Draw a set of points where <src>scaledValues</src> gives the
		// Colormap index of each point.  Point coordinates ('points')
		// are either linear or world coordinates, as specified by 'linear'.
		casacore::Bool mapToColorAndDrawPoints(const casacore::Matrix<casacore::Double> &points,
		                             const casacore::Vector<casacore::uInt> &scaledValues,
		                             const casacore::Bool &linear = false);

		// Draw a set of ellipses where <src>scaledValues</src> gives the
		// Colormap index of each point.  Point coordinates ('points')
		// are either linear or world coordinates, as specified by 'linear'.
		casacore::Bool mapToColorAndDrawEllipses(const casacore::Matrix<casacore::Double> &centres,
		                               const casacore::Vector<casacore::Float> &smajor,
		                               const casacore::Vector<casacore::Float> &sminor,
		                               const casacore::Vector<casacore::Float> &pangle,
		                               const casacore::Vector<casacore::uInt> scaledValues,
		                               const casacore::Float &scale,
		                               const casacore::Bool &outline,
		                               const casacore::Bool &linear);




		// <b>***Cached***</b> blc pixel where this world canvas begins =
		// itsPixelCanvas->width()*frac(X|Y)Offset_.
		// <group>
		casacore::uInt itsCanvasXOffset;
		casacore::uInt itsCanvasYOffset;
		// </group>

		// <b>***Cached***</b> number of pixels in each dimension given to
		// the world canvas = itsPixelCanvas->width()*(frac(X|Y)Size_.
		// <group>
		casacore::uInt itsCanvasXSize;
		casacore::uInt itsCanvasYSize;
		// </group>

		// Fractional position of world canvas on pixel canvas.  The offset
		// values are always in the range of [0.0,1.0], and sizes must be
		// smaller or equal to (1.0 - offset) for each dimension.
		// <group>
		casacore::Double itsFracXOffset;
		casacore::Double itsFracYOffset;
		casacore::Double itsFracXSize;
		casacore::Double itsFracYSize;
		// </group>

		// <b>***Cached***</b> blc pixel where the world canvas 'draw area'
		// (inside margins, labels) begins, relative to WC blc.
		// <group>
		casacore::uInt itsCanvasDrawXOffset;
		casacore::uInt itsCanvasDrawYOffset;
		// </group>

		// <b>***Cached***</b> number of pixels in each dimension given to
		// the drawable part of the world canvas
		// <group>
		casacore::uInt itsCanvasDrawXSize;
		casacore::uInt itsCanvasDrawYSize;
		// </group>

		// Linear casacore::Coordinate System ranges.
		// <group>
		casacore::Double itsLinXMin;
		casacore::Double itsLinYMin;
		casacore::Double itsLinXMax;
		casacore::Double itsLinYMax;
		// </group>

		// Linear casacore::Coordinate System Limits.
		// <group>
		casacore::Double itsLinXMinLimit;
		casacore::Double itsLinYMinLimit;
		casacore::Double itsLinXMaxLimit;
		casacore::Double itsLinYMaxLimit;
		// </group>

		// Dynamic data minimum and maximum for this WorldCanvas.
		casacore::Double itsDataMin, itsDataMax;

		// Method to use to convert complex data into real values.
		Display::ComplexToRealMethod itsComplexToRealMethod;

		// Event handler lists and convenient iterators.
		// <group>
		casacore::List<DisplayEH *> itsRefreshEHList;
		casacore::List<WCPositionEH *> itsPositionEHList;
		casacore::List<WCMotionEH *> itsMotionEHList;
		mutable casacore::ListIter<DisplayEH *> *itsREHListIter;
		mutable casacore::ListIter<WCPositionEH *> *itsPEHListIter;
		mutable casacore::ListIter<WCMotionEH *> *itsMEHListIter;
		// </group>

		// Other handler lists.
		// <group>
		WCSizeControlHandler *itsSizeControlHandler;
		WCCoordinateHandler *itsCoordinateHandler;
		WCResampleHandler *itsResampleHandler;
		WCDataScaleHandler *itsDataScaleHandler;
		// </group>

		// Store whether we "own" the various handlers.
		// <group>
		casacore::Bool itsOwnSizeControlHandler;
		casacore::Bool itsOwnCoordinateHandler;
		casacore::Bool itsOwnResampleHandler;
		casacore::Bool itsOwnDataScaleHandler;
		// </group>

		// Buffer for Attributes.
		AttributeBuffer attributes;

		// Background/foreground colors.
		casacore::String itsWorldBackgroundColor;
		casacore::String itsWorldForegroundColor;

		// PixelCanvas pointer.
		PixelCanvas *itsPixelCanvas;

		// PGPLOT filter.
		WCPGFilter *itsPGFilter;

		// Status of hold/release.
		casacore::Int itsHoldCount;
		casacore::Bool itsRefreshHeld;
		Display::RefreshReason itsHeldReason;

		// The DisplayCoordinateSystem for this WorldCanvas.  New addition, only
		// supported and used by "new" classes.
		DisplayCoordinateSystem *itsCoordinateSystem;

		// This state is set true when the pointer is in this WC and a pointer
		// button is pressed (with no buttons pressed previously).  When true,
		// all PC motion and pointer button events are propagated to this WC's
		// handlers (only), regardless of whether the pointer has moved off the WC.
		// It is reset to false when all buttons are released.  This simulates
		// the 'automatic grab' (implemented in X for whole windows), on the WC
		// level.
		casacore::Bool itsGrabbing;


		// ColorIndexedImage_ stores the state of a WorldCanvas::drawImage()
		// rendering after the chosen data plane has been resampled to screen
		// (Pixelcanvas) pixels and scaled to indices within a fixed-size
		// Colormap, but prior to mapping to actual colors for display.
		// 'Caching' of this state is useful in 24-bit (trueColor) applications,
		// greatly speeding up redrawing when only the mapping to colors is changed.
		//# Formerly only one instance of this state was stored in a WorldCanvas
		//# (itsCachedImage and friends), but this caused confusion and errors
		//# when more that one DD was drawing on the WC.
		// Packaging this state allows different instances to be cached by
		// different callers of drawImage() if desired, to be reused by them
		// when appropriate via WC::redrawIndexedImage().  Reuse is 'appropriate'
		// only when nothing has changed since the original drawImage() call
		// except the actual colors assigned to the map indices; among other things,
		// the size of the Colormap used must be the same as that of the original.
		//
		// Note: this level of 'caching' is distinct from the more elaborate
		// 'drawlists' which are supported by WC/PC and cached in DisplayMethods.

		struct ColorIndexedImage_ {
			casacore::Matrix<casacore::uInt> data;		// colormap indices
			casacore::Matrix<casacore::Bool> mask;
			casacore::Vector<casacore::Int> blc;
			casacore::uInt colormapSize;

			ColorIndexedImage_() : data(), mask(), blc(2,0) {  }
			void clear() {
				data.resize();
				mask.resize();
				blc=0;
			}
			casacore::Bool maskValid() {
				return mask.nelements()!=0u &&
				       mask.shape().isEqual(data.shape());
			}
		};

		ColorIndexedImage_* makeColorIndexedImage(const casacore::Vector<casacore::Double> &blc,
				            const casacore::Vector<casacore::Double> &trc,
				            const casacore::Matrix<casacore::Float> &data,
				            const casacore::Bool usePixelEdges, void* drawObj);
		casacore::Matrix<casacore::uInt> mapToColor( const casacore::Matrix<casacore::uInt> & scaledImage);
		// Cache of pre-drawn ColorIndexedImage_'s.   When a caller of drawImage()
		// wants to save one, it passes a drawing-object pointer in the 'drawObj'
		// parameter for use as a retrieval key.  It should provide the same key
		// to redrawIndexedImage() in order to reuse the image.
		//# This mechanism attempts to avoid some erroneous reuses of
		//# 'itsCachedImage' among different DDs (bugs 4937, 5032).   (dk 3/05)
		casacore::SimpleOrderedMap<void*, ColorIndexedImage_*> images_;

		// Retrieve an indexed image to write onto.  Used (exclusively) by
		// WC::drawImage().  If one exists in the cache under this objId key,
		// clear it for reuse; otherwise return a new one. If a (non-zero) objId
		// was provided, it will be cached under that key; removeIndexedImage()
		// can be used to remove such cached images explicitly, but the cache is
		// also frequently cleared automatically.  If no caching is requested,
		// however, (objId=0), the caller must delete the image himself when
		// finished -- that type of call is equivalent to 'new ColorIndexedImage_'.
		ColorIndexedImage_* getClearedColorIndexedImage(void* drawObj=0);

		// A buffer to contain the restrictions that DisplayDatas must match
		// if they are to be allowed to draw themselves.
		AttributeBuffer itsRestrictions;


		// [First] responder to 'sizeControl', responsible for setting
		// WC CS, zoom window and draw area.  It will be 0 initially, and
		// whenever the old master is unregistered (until a new master responds).
		// This is a further attempt toward a coherent sense of 'who's in charge'
		// of WC[H] state (there is more to do).
		//
		// Some day, the WC CS should be directly responsible for all the Canvas's
		// coordinate conversions.  For now at least we'll know that they're done
		// by the DD below (which should be equivalent).
		DisplayData* itsCSmaster;

		// itsId & itsRef used to ensure thread-safe execution of pgplot

		casacore::uInt itsId; // id of wc instance


		// WorldCanvas::refresh is a recursive function. itsRef is used to
		// determine when the recursion is over. i.e, when the initial
		// refresh call is exiting
		casacore::uInt itsRef;

		std::string error_string;
	};


} //# NAMESPACE CASA - END

#endif
