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
// Images are drawn by sending a Matrix and specifying the
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
// the value can be of type uInt, Int, Float, Double, Bool and String,
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
// <ul> <li> canvasXOffset (uInt): The offset of the WorldCanvas on
// the PixelCanvas (X) in 'screen pixels' <li> canvasYOffset (uInt):
// The offset of the WorldCanvas on the PixelCanvas (Y) in 'screen
// pixels' <li> canvasYSize (uInt): The size of the WorldCanvas on the
// PixelCanvas (X) in 'screen pixels' <li> canvasYSize (uInt): The
// size of the WorldCanvas on the PixelCanvas (Y) in 'screen pixels'
// <li> fracXOffset (Double): The fractional offset of the WorldCanvas
// on the Pixelcanvas (X) <li> fracYOffset (Double): The fractional
// offset of the WorldCanvas on the Pixelcanvas (Y) <li> fracXSize
// (Double): The fractional size of the WorldCanvas on the Pixelcanvas
// (X) <li> fracYSize (Double): The fractional size of the WorldCanvas
// on the Pixelcanvas (Y) <li> linXMin (Double): The current minimum
// linear coordinate (X) <li> linXMax (Double): The current maximum
// linear coordinate (X) <li> linYMin (Double): The current minimum
// linear coordinate (Y) <li> linYMax (Double): The current maximum
// linear coordinate (Y) <li> linXMinLimit (Double): The current limit
// on the minimum linear coordinate (X) <li> linXMaxLimit (Double):
// The current limit on the maximum linear coordinate (X) <li>
// linYMinLimit (Double): The current limit on the minimum linear
// coordinate (Y) <li> linYMaxLimit (Double): The current limit on the
// maximum linear coordinate (Y) <li> dataMin (Double): The data
// minimum of the WorldCanvas <li> dataMax (Double): The data maximum
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
		WorldCanvas(PixelCanvas *pc, Double xOrigin = 0.0, Double yOrigin = 0.0,
		            Double xSize = 1.0, Double ySize = 1.0);

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
		             const Bool &explicitrequest = True);

		// Is a refresh currently allowed?
		Bool refreshAllowed();

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

		// Set Coordinate, SizeControl, Resample and DataScale handlers for
		// the WorldCanvas.  If the given handler is 0, then resort to using
		// the default handler.
		// <group>
		void setCoordinateHandler(WCCoordinateHandler *ch);
		void setSizeControlHandler(WCSizeControlHandler *sh);
		void setResampleHandler(WCResampleHandler *rh);
		void setDataScaleHandler(WCDataScaleHandler *sh);
		// </group>

		// Set the location of the WorldCanvas on its PixelCanvas.
		void setWorldCanvasPosition(Double fracXOffset, Double fracYOffset,
		                            Double fracXSize, Double fracYSize);

		// Pixel, linear and world coordinate transformation functions.  For
		// the Vector versions, the coordinate mapping returns False if the
		// transformation failed.  For the Matrix versions, failures(i) on
		// input should be set to True if the i'th transformation should not
		// be attempted.  On output, failures(i) is True if the
		// transformation was not attempted, or failed.  If on input the
		// failures vector has zero length, it will be assumed that no prior
		// failures have occurred.
		// <group>
		Bool pixToLin(Vector<Double> &lin, const Vector<Double> &pix);
		Bool pixToLin(Matrix<Double> &lin, Vector<Bool> &failures,
		              const Matrix<Double> &pix);
		Bool linToPix(Vector<Double> &pix, const Vector<Double> &lin);
		Bool linToPix(Matrix<Double> &pix, Vector<Bool> &failures,
		              const Matrix<Double> &lin);
		Bool linToWorld(Vector<Double> &world, const Vector<Double> &lin);
		Bool linToWorld(Matrix<Double> &world, Vector<Bool> &failures,
		                const Matrix<Double> &lin);
		Bool worldToLin(Vector<Double> &lin, const Vector<Double> &world);
		Bool worldToLin(Matrix<Double> &lin, Vector<Bool> &failures,
		                const Matrix<Double> &world);
		Bool pixToWorld(Vector<Double> &world, const Vector<Double> &pix);
		Bool pixToWorld(Matrix<Double> &world, Vector<Bool> &failures,
		                const Matrix<Double> &pix);
		Bool worldToPix(Vector<Double> &pix, const Vector<Double> &world);
		Bool worldToPix(Matrix<Double> &pix, Vector<Bool> &failures,
		                const Matrix<Double> &world);
		// </group>

		// Register/unregister a Colormap on the PixelCanvas.  Registration
		// counts are remembered, so that a particular Colormap is
		// guaranteed to be available as long as that Colormap has been
		// registered more times than it has been unregistered.  Requests
		// are forwarded to the PixelCanvas.
		// <group>
		void registerColormap(Colormap *cmap, Float weight = 1.0);
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
		uInt newList();
		void endList();
		void drawList(uInt list);
		void deleteList(uInt list);
		void deleteLists();
		Bool validList(uInt list);
		// </group>

		// Set various graphics attributes.  All of these requests are
		// passed directly to the PixelCanvas, except for
		// <src>setColor</src>, which also installs the requested color for
		// subsequent calls to PgPlot functions.
		// <group>
		void setColor(const String &color);
		void setClearColor(const String &color);
		Bool setFont(const String &fontName);
		void setForeground(uLong color);
		void setBackground(uLong color);
		void setLineWidth(Float width);
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
		Bool setWorldBackgroundColor(const String color);
		Bool setWorldForegroundColor(const String color);
		String getWorldBackgroundColor() {
			return itsWorldBackgroundColor;
		}
		String getWorldForegroundColor() {
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

		// Apply options stored in rec to the DisplayData; return value True
		// means a refresh is needed.  Any fields added to the
		// updatedOptions argument are options which have changed in some
		// way due to the setting of other options - ie. they are context
		// sensitive.
		virtual Bool setOptions(const Record &rec, Record &updatedOptions);

		// Retrieve the current and default options and parameter types.
		virtual Record getOptions() const;

		// Set an Attribute or Attributes on the WorldCanvas.
		// <group>
		void setAttribute(Attribute& at);
		void setAttributes(AttributeBuffer& at);
		// </group>

		// Remove an Attribute.  Pre-defined Attributes of the WorldCanvas
		// cannot be removed (although nothing serious will happen if you
		// try).
		void removeAttribute(String& name);

		// User interface to get individual values from the attribute buffer.
		// <group>
		Bool getAttributeValue(const String& name, uInt& newValue) const;
		Bool getAttributeValue(const String& name, Int& newValue) const;
		Bool getAttributeValue(const String& name, Float& newValue) const;
		Bool getAttributeValue(const String& name, Double& newValue) const;
		Bool getAttributeValue(const String& name, Bool& newValue) const;
		Bool getAttributeValue(const String& name, String& newValue) const;
		Bool getAttributeValue(const String& name, Vector<uInt>& newValue) const;
		Bool getAttributeValue(const String& name, Vector<Int>& newValue) const;
		Bool getAttributeValue(const String& name, Vector<Float>& newValue) const;
		Bool getAttributeValue(const String& name, Vector<Double>& newValue) const;
		Bool getAttributeValue(const String& name, Vector<Bool>& newValue) const;
		Bool getAttributeValue(const String& name, Vector<String>& newValue) const;
		// </group>

		// Check if a certain Attribute exists.
		Bool existsAttribute(String& name) const;

		// Get the type of an Attribute.
		AttValue::ValueType attributeType(String& name) const;

		// Position the PGPLOT filter on the WorldCanvas.  If linear is
		// specified False, then the alignment is done by world coordinates,
		// assuming that a linear approximation is valid.
		// void verifyPGFilterAlignment(const Bool &linear = True);

		// Acquire and release a PGPLOT device for this WorldCanvas.  This
		// is necessary since PGPLOT generally only supports 8 currently
		// active devices.  So refresh cycles on the WorldCanvas acquire a
		// PGPLOT device at the start, and release it at the end.  Cycles
		// are counted so that external user-classes can call these functions
		// if necessary in a nested state.  If <src>linear</src> is specified
		// as <src>False</src>, then the PGPLOT device is aligned by world
		// coordinates, under the assumption that a linear approximation is
		// valid, that is, the curvature is small.
		// <group>
		virtual void acquirePGPLOTdevice(const Bool &linear = True);
		virtual void releasePGPLOTdevice();
		// </group>

		// Return the PGPLOT device id for external use.
		virtual Int pgid() const;

		// Draw unrotated text at the given position.  If the conversion
		// from world to pixel coordinates fails, the text is not drawn, and
		// False is returned.  If linear is True, then the provided position
		// is actually in linear world canvas coordinates, rather than true
		// world coordinates.
		Bool drawText(const Vector<Double> &point, const String &text,
		              Display::TextAlign alignment = Display::AlignCenter,
		              const Bool &linear = False);

		// Draw a single point using the current color.  If the conversion
		// from world to pixel coordinates fails, the point is not drawn,
		// and False is the return value.  If linear is True, then the point
		// position is given in linear world canvas coordinates, not true
		// world coordinates.
		Bool drawPoint(const Vector<Double> &point, const Bool &linear = False);

		// Draw a single line using the current color.  If either of the
		// conversions from world to pixel coordinates fail, then the line
		// is not drawn, and False is returned.  If linear is True, then the
		// line endpoints are given in world canvas linear coordinates
		// rather than real world coordinates.
		Bool drawLine(const Vector<Double> &a, const Vector<Double> &b,
		              const Bool &linear = False);

		// Draw a bunch of points using the current color.  If any points
		// fail to convert then none of them are drawn, and False is
		// returned.  If linear is True, then the vertices are given in
		// linear world canvas coordinates rather than real world
		// coordinates.
		Bool drawPoints(const Matrix<Double> &points, const Bool &linear = False);

		// Draw a set of points using the current color.  Those points which
		// fail to convert, or lie outside the WorldCanvas drawing area, are
		// not drawn.
		Bool drawPoints(const Vector<Float> &px, const Vector<Float> &py,
		                Bool linear = False);

		// Draw a set of text strings using the current color.  If any
		// points fail to convert, then those particular strings are not
		// drawn.  <src>rotation</src> gives the rotation of the text in
		// degrees counter-clockwise from horizontal.  <src>xoffset</src>
		// and <src>yoffset</src> can be given to globally shift the labels
		// by the specified amounts (in units of the character height).  If
		// linear is True, then the vertices are given in linear world
		// canvas coordinates rather than true world coordinates.
		Bool drawTextStrings(const Vector<Float> &px, const Vector<Float> &py,
		                     const Vector<String> &strings,
		                     const Float rotation = 0.0,
		                     const Float xoffset = 0.0,
		                     const Float yoffset = 0.0,
		                     const Bool linear = False);

		// Draw a set of markers using the current color and a given pixel
		// <src>size</src>. If any points fail to convert, then those
		// particular points are not marked.  <src>markertype</src> is an
		// <src>Display::Marker</src>. If linear is True, then the points
		// are given in linear world canvas coordinates rather than true
		// world coordinates.
		Bool drawMarkers(const Vector<Float> &px, const Vector<Float> &py,
		                 const Display::Marker = Display::Cross, const Int size = 5,
		                 const Bool& linear = False);

		Bool drawMappedMarkers(const Vector<Float> &px, const Vector<Float> &py,
		                       const Vector<Float>& values,
		                       const Int sizemin = 1, const Int sizemax = 20,
		                       const Display::Marker = Display::Cross,
		                       const Bool& linear = False);

		// Draw pairs of lines using the current color.  If any points fail
		// to convert then the lines are not drawn and False is returned.
		// If linear is True, then the vertices are given as linear world
		// coordinates rather than true world coordinates.
		Bool drawLines(const Matrix<Double> &vertices, const Bool &linear = False);


		// Draw a polyline (connected line) between the vertices using the
		// current color.  If any coordinates fail to convert from world to
		// pixel, then the entire polyline is not drawn and False is
		// returned.  The end point is not implicitly connected to the
		// starting point.  If linear is True, then the provided vertices
		// are actually linear world canvas coordinates.
		Bool drawPolyline(const Matrix<Double> &vertices,
		                  const Bool &linear = False);

		// Draw a polygon (closed line, or line loop using the points)
		// using the current color.  If any coordinates fail to convert
		// then the polygon is not drawn.  The end point is implicitly
		// connected to the start point.  If linear is True, then the
		// provided vertices are actually linear world coordinates.
		Bool drawPolygon(const Matrix<Double> &vertices,
		                 const Bool &linear = False);

		// Draw a set of points in colors which are taken from the current
		// Colormap.
		Bool drawColormappedPoints(const Matrix<Double> &points,
		                           const Vector<Float> &values,
		                           const Bool &linear = False);

		// Draw a set of colored ellipses, possibly with outlines.  The x
		// and y locations must given, along with semi-major and semi-minor
		// axes, and position angle measured in degrees positive from the x
		// axis in a counter-clockwise direction.  The size of the ellipses
		// is globally scaled by the scale factor, and if <src>outline</src>
		// is <src>True</src>, then each ellipse will have an outline in the
		// current pen color.
		Bool drawColormappedEllipses(const Matrix<Double> &centres,
		                             const Vector<Float> &smajor,
		                             const Vector<Float> &sminor,
		                             const Vector<Float> &pangle,
		                             const Vector<Float> &colors,
		                             const Float &scale = 1.0,
		                             const Bool &outline = True,
		                             const Bool &linear = False);


		// This routine is specialized for drawing image restoring-beam ellipses.
		// Its parameters are defined so as to require as little conversion as
		// possible of restoring beam information as stored in an image header.
		//
		// It does nothing unless the axes on display map to the two axes of a
		// DirectionCoordinate within the WC CS (WorldCanvas::itsDisplayCoordinateSystem).
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
		// (NB: increasing RA commonly means decreasing pixel/Lattice coordinates).
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
		// matrices in the DirectionCoordinate (e.g. rotated "North")-- which I
		// think is true in many other places as well... -- for someone's to-do list
		// (not mine, I hope)).
		Bool drawBeamEllipse(Float major, Float minor,  Float pa,
		                     String majunit="arcsec", String minunit="arcsec",
		                     String paunit="deg",
		                     Float cx=.1f, Float cy=.1f, Bool outline=True);


		// Draw a contour map at the specified levels, and place the lower
		// left pixel at blPos, and the upper right pixel at trPos.  If
		// <src>usePixelEdges</src> is True, then the given world positions
		// are the position of the blc and trc of the blc and trc pixels,
		// otherwise they are the positions of the centres of the pixels.
		// Note that the contours are not intrinsically drawn in world
		// coordinates.  For complex data, the conversion to real values is
		// done according to the last call to setComplexToRealMethod.
		// Returns true if OK, false if error...
		// <group>
		bool drawContourMap(const Vector<Double> &blPos,
		                    const Vector<Double> &trPos,
		                    const Matrix<Float> &data,
		                    const Vector<Float> &levels,
		                    const Bool usePixelEdges = False);


		bool drawContourMap(const Vector<Double> &blPos,
		                    const Vector<Double> &trPos,
		                    const Matrix<Complex> &data,
		                    const Vector<Float> &levels,
		                    const Bool usePixelEdges = False);
		// </group>

		// Draw a contour map at the specified levels, and place the lower
		// left pixel at blPos, and the upper right pixel at trPos.  If
		// <src>usePixelEdges</src> is True, then the given world positions
		// are the position of the blc and trc of the blc and trc pixels,
		// otherwise they are the positions of the centres of the pixels.
		// Note that the contours are not intrinsically drawn in world
		// coordinates.  For complex data, the conversion to real values is
		// done according to the last call to setComplexToRealMethod.  These
		// functions also have a <src>mask</src> argument, which is a
		// Boolean pixel mask whose shape must match that of
		// <src>data</src>, and only pixels in <src>data</src> where
		// corresponding pixels in <src>mask</src> are <src>True</src> will
		// be contoured.
		// Returns true if OK, false if error...
		// <group>
		bool drawContourMap(const Vector<Double> &blPos,
		                    const Vector<Double> &trPos,
		                    const Matrix<Float> &data,
		                    const Matrix<Bool> &mask,
		                    const Vector<Float> &levels,
		                    const Bool usePixelEdges = False);
		bool drawContourMap(const Vector<Double> &blPos,
		                    const Vector<Double> &trPos,
		                    const Matrix<Complex> &data,
		                    const Matrix<Bool> &mask,
		                    const Vector<Float> &levels,
		                    const Bool usePixelEdges = False);
		// </group>

		// Optimization to speed up colormap fiddling in 24bit mode (software
		// Colormap); see images_, below for usage.  Set opaqueMask to True to
		// draw masked pixels in the background color; otherwise they will be
		// transparent (letting whatever was drawn previously at that point show
		// through).
		Bool redrawIndexedImage(void* drawObj, Display::RefreshReason reason,
		                        Bool opaqueMask=False);

		// Remove image from the colormap change cache, if any (see images_, below).
		// Return value indicates whether there was anything to remove.
		Bool removeIndexedImage(void* drawObj);

		// Clear the whole colormap change cache (see images_, below).
		void clearColormapChangeCache();

		// Draw an image, mapping data values to Colormap entries, and place
		// the lower left pixel at blPos, and the upper right pixel at
		// trPos.  If <src>usePixelEdges</src> is True, then the given world
		// positions are the position of the blc and trc of the blc and trc
		// pixels, otherwise they are the positions of the centres of the
		// pixels.
		// See images_, below, for non-default usage of the drawObj parameter.
		// <group>
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Float> &data, const Bool usePixelEdges = False,
		               void* drawObj=0);
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Complex> &data,
		               const Bool usePixelEdges = False, void* drawObj=0);
		void drawImage(const Vector<Double> &blc,const Vector<Double> &trc, const Matrix<Float> &data,
			           const Matrix<Float> &dataRed, const Matrix<Float> &dataGreen,
			           const Matrix<Float> &dataBlue,const Bool usePixelEdges,void* drawObj = 0);
		void drawImage(const Vector<Double> &blc,const Vector<Double> &trc, const Matrix<Complex> &data,
					   const Matrix<Complex> &dataRed, const Matrix<Complex> &dataGreen,
					   const Matrix<Complex> &dataBlue,const Bool usePixelEdges,void* drawObj = 0);
		// </group>

		// Draw an image, mapping data values to Colormap entries, and place
		// the lower left pixel at blPos, and the upper right pixel at
		// trPos.  If <src>usePixelEdges</src> is True, then the given world
		// positions are the position of the blc and trc of the blc and trc
		// pixels, otherwise they are the positions of the centres of the
		// pixels.  These functions also have a <src>mask</src> argument,
		// which is a Boolean pixel mask whose shape must match that of
		// <src>data</src>, and only pixels in <src>data</src> where
		// corresponding pixels in <src>mask</src> are <src>True</src>
		// will be drawn.  Set opaqueMask to True to draw masked pixels in
		// the background color; otherwise they will be transparent (letting
		// whatever was drawn previously at that point show through).
		// See images_, below, for non-default usage of the drawObj parameter.
		// <group>
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Float> &data, const Matrix<Bool> &mask,
		               const Bool usePixelEdges = False, void* drawObj=0,
		               Bool opaqueMask=False);
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Complex> &data, const Matrix<Bool> &mask,
		               const Bool usePixelEdges = False, void* drawObj=0,
		               Bool opaqueMask=False);

		// </group>
		// Draw a component of a multi-channel image, mapping data values to
		// component levels, and place the lower left pixel at blPos, and
		// the upper right pixel at trPos.  If <src>usePixelEdges</src> is
		// True, then the given world positions are the position of the blc
		// and trc of the blc and trc pixels, otherwise they are the
		// positions of the centres of the pixels.  The components are not
		// drawn until flushComponentImages() is called.
		// <group>
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Float> &data,
		               const Display::ColorComponent &colorcomponent,
		               const Bool usePixelEdges = False);
		void drawImage(const Vector<Double> &blPos, const Vector<Double> &trPos,
		               const Matrix<Complex> &data,
		               const Display::ColorComponent &colorcomponent,
		               const Bool usePixelEdges = False);
		// </group>

		// Draw a vector map.
		// <group>
		bool drawVectorMap(const Vector<Double>& blc,
		                   const Vector<Double>& trc,
		                   const Matrix<Complex>& data,
		                   const Matrix<Bool>& mask,
		                   Float angleConversionFactor,
		                   Float phasePolarity,
		                   Bool debias, Float variance,
		                   Int xPixelInc, Int yPixelInc,
		                   Float scale, Bool arrow, Float barb,
		                   Float rotation,
		                   Double xWorldInc, Double yWorldInc,
		                   const Bool usePixelEdges);

		bool drawVectorMap(const Vector<Double>& blc,
		                   const Vector<Double>& trc,
		                   const Matrix<Float>& data,
		                   const Matrix<Bool>& mask,
		                   Float angleConversionFactor,
		                   Float phasePolarity,
		                   Bool debias, Float variance,
		                   Int xPixelInc, Int yPixelInc,
		                   Float scale, Bool arrow, Float barb,
		                   Float rotation,
		                   Double xWorldInc, Double yWorldInc,
		                   const Bool usePixelEdges);
		// </group>

// Draw marker maps.  Only makerType "square" available presently.
// The marker holds its shape in screen pixel coordinates.  This
// means a square is always a square regardless of aspect ratio
// Returns true if OK, false if error...
// <group>
		bool drawMarkerMap(const Vector<Double>& blc,
		                   const Vector<Double>& trc,
		                   const Matrix<Float>& data,
		                   const Matrix<Bool>& mask,
		                   Int xPixelInc, Int yPixelInc,
		                   Float scale, Double xWorldInc, Double yWorldInc,
		                   const String& markeType,
		                   Bool usePixelEdges);
		bool drawMarkerMap(const Vector<Double>& blc,
		                   const Vector<Double>& trc,
		                   const Matrix<Complex>& data,
		                   const Matrix<Bool>& mask,
		                   Int xPixelInc, Int yPixelInc,
		                   Float scale, Double xWorldInc, Double yWorldInc,
		                   const String& markerType,
		                   Bool usePixelEdges);
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
		Double linXMin() const {
			return itsLinXMin;
		}
		Double linYMin() const {
			return itsLinYMin;
		}
		Double linXMax() const {
			return itsLinXMax;
		}
		Double linYMax() const {
			return itsLinYMax;
		}
		// </group>

		// Provide information on the limits of the linear coordinate
		// system.
		// <group>
		Double linXMinLimit() const {
			return itsLinXMinLimit;
		}
		Double linYMinLimit() const {
			return itsLinYMinLimit;
		}
		Double linXMaxLimit() const {
			return itsLinXMaxLimit;
		}
		Double linYMaxLimit() const {
			return itsLinYMaxLimit;
		}
		// </group>

		// Provide information on the WorldCanvas offset and size.
		// <group>
		uInt canvasXOffset() const {
			return itsCanvasXOffset;
		}
		uInt canvasYOffset() const {
			return itsCanvasYOffset;
		}
		uInt canvasXSize() const {
			return itsCanvasXSize;
		}
		uInt canvasYSize() const {
			return itsCanvasYSize;
		}
		// </group>

		// Provide information on the WorldCanvas drawable offset and size.
		// <group>
		uInt canvasDrawXOffset() const {
			return itsCanvasDrawXOffset;
		}
		uInt canvasDrawYOffset() const {
			return itsCanvasDrawYOffset;
		}
		uInt canvasDrawXSize() const {
			return itsCanvasDrawXSize;
		}
		uInt canvasDrawYSize() const {
			return itsCanvasDrawYSize;
		}
		// </group>

		// Set the zoom rectangle to the specfied linear coordinate range.
		void setZoomRectangleLCS(const Vector<Double> &min,
		                         const Vector<Double> &max);

		// Move the zoom rectangle across the screen, ie. pan.
		void moveZoomRectangleLCS(double dx, double dy);

		// Reset the zoom to show the entire allowable range of the linear
		// coordinate system.
		void resetZoomRectangle();

		// Set the allowable range of the linear coordinate system.
		void setLinearCoordinateSystem(const Vector<Double> &blc,
		                               const Vector<Double> &trc,
		                               Bool resetZoom = True);

		// Functions to set and retrieve the minimum and maximum data values
		// for scaling data that is drawn on the WorldCanvas.  These values
		// are forwarded to the scale handler, when, for example, images are
		// drawn.
		// <group>
		Double dataMin() const {
			return itsDataMin;
		}
		void setDataMin(Double min) {
			itsDataMin = min;
		}
		Double dataMax() const {
			return itsDataMax;
		}
		void setDataMax(Double max) {
			itsDataMax = max;
		}
		void setDataMinMax(Double min, Double max) {
			itsDataMin = min;
			itsDataMax = max;
		}
		// </group>

		// ComplexToRealMethod defines which real component of a Complex
		// image to extract when it is necessary to convert Complex data
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
		Bool hasCS() const {
			return itsCoordinateSystem!=0;
		}
		// </group>

		// Convenience functions returning whether a pixel coordinate is
		// within bounds of the WC's inner draw area, the WC, or the underlying PC.
		// <group>

		Bool inDrawArea(Int x, Int y) const {
			Int x0 = itsCanvasXOffset + itsCanvasDrawXOffset;
			Int x1 = x0 + itsCanvasDrawXSize;
			Int y0 = itsCanvasYOffset + itsCanvasDrawYOffset;
			Int y1 = y0 + itsCanvasDrawYSize;
			return  x>=x0 && x<x1  &&  y>=y0  && y<y1 ;
		}

		Bool inWC(Int x, Int y) const {
			Int x0 = itsCanvasXOffset;
			Int x1 = x0 + itsCanvasXSize;
			Int y0 = itsCanvasYOffset;
			Int y1 = y0 + itsCanvasYSize;
			return  x>=x0 && x<x1  &&  y>=y0  && y<y1 ;
		}

		Bool inPC(Int x, Int y);

		// </group>


		// Install a single restriction, or a buffer of restrictions, on the
		// WorldCanvas which DisplayData must match in order that they
		// be allowed to draw themselves.
		// <group>
		void setRestriction(const Attribute& restriction);
		void setRestrictions(const AttributeBuffer& resBuff);
		// </group>

		// Check if a named restriction exists.
		const Bool existRestriction(const String& name) const;

		// Remove the named restriction, or all restrictions, from the
		// WorldCanvas.
		// <group>
		void removeRestriction(const String& restrictionName);
		void removeRestrictions();
		// </group>

		// Determine whether the restrictions installed on the
		// WorldCanvas match the given restriction or buffer of
		// restrictions.
		// <group>
		Bool matchesRestriction(const Attribute& restriction) const;
		Bool matchesRestrictions(const AttributeBuffer& buffer) const;
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
		Bool isCSmaster(const DisplayData *dd) const {
			return dd==csMaster() && dd!=0;
		}

		bool removeDD( const DisplayData *dd ) {
			bool result = (dd == csMaster( ));
			if ( result ) itsCSmaster = 0;
			return result;
		}

		// Return the names and units of the world coordinate axes.
		// <group>
		virtual Vector<String> worldAxisNames() const;
		virtual Vector<String> worldAxisUnits() const;
		// </group>

		const std::list<DisplayData*> &displaylist( ) const;

		static const String LEFT_MARGIN_SPACE_PG;
		static const String RIGHT_MARGIN_SPACE_PG;
		static const String BOTTOM_MARGIN_SPACE_PG;
		static const String TOP_MARGIN_SPACE_PG;

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
		// is True, then the given coordinates are in linear world coordinates,
		// otherwise they are real world coordinates.  A return value of False
		// indicates the conversion failed.
		// <group>
		Bool castingConversion(Vector<Int> &pixelpt, const Vector<Double> &worldpt,
		                       const Bool &linear);
		Bool castingConversion(Matrix<Int> &pixelpts, const Matrix<Double> &worldpts,
		                       const Bool &linear);
		Bool castingConversion(Matrix<Float> &pixelpts,
		                       const Matrix<Double> &worldpts,
		                       const Bool &linear);
		// </group>

		// Convert the given coordinate/s to pixel coordinates.  If
		// <src>linear</src> is <src>True</src>, then the input coordinates
		// are linear world coordinates, otherwise they are true world
		// coordinates.  This version applies clipping, so that any points
		// in the series which lie outside the linear coordinate range of
		// the WorldCanvas are discarded.  Thus the output Vectors can be
		// shorter than the input Vectors.  A mask indicating which of the
		// input points were valid is returned for user reference.
		Bool castingClippingConversion(Vector<Int> &pixelx, Vector<Int> &pixely,
		                               Vector<Bool> &validConversions,
		                               const Vector<Float> &worldx,
		                               const Vector<Float> &worldy,
		                               const Bool linear);

		// Actually draw the vector field.   All pa * angleConversionFactor
		// must be radians.  rotation  must be radians.  if amp is of
		// dimension 0, amplitude unity is assumed.  if mask if dimension
		// 0 all data are assumed good.
		bool drawVectorMap(const Vector<Double>& blc,
		                   const Vector<Double>& trc,
		                   const Matrix<Float>& amp,
		                   const Matrix<Float>& pa,
		                   const Matrix<Bool>& mask,
		                   Float angleConversionFactor,
		                   Float phasePolarity,
		                   Bool debias, Float variance,
		                   Int xPixelInc, Int yPixelInc,
		                   Float scale, Bool arrow, Float barb, Float rotation,
		                   Double xWorldInc, Double yWorldInc,
		                   const Bool usePixelEdges);


		// Trim and resample an image, returning the actual world BLC and
		// TRC for drawing, and the resampled image.
		void trimAndResampleImage(Vector<Double> &drawBlc,
		                          Vector<Double> &drawTrc,
		                          Matrix<Float> &sampledImage,
		                          const Vector<Double> &blc,
		                          const Vector<Double> &trc,
		                          const Matrix<Float> &data,
		                          const Bool &usePixelEdges = False);

		// Trim and resample an image, returning the actual world BLC and
		// TRC for drawing, and the resampled image.  This function takes
		// a mask indicating which pixels should be drawn.  This function
		// therefore also trims and resamples the mask.
		void trimAndResampleImage(Vector<Double> &drawBlc,
		                          Vector<Double> &drawTrc,
		                          Matrix<Float> &sampledImage,
		                          Matrix<Bool> &resampledMask,
		                          const Vector<Double> &blc,
		                          const Vector<Double> &trc,
		                          const Matrix<Float> &data,
		                          const Matrix<Bool> &mask,
		                          const Bool &usePixelEdges = False);

		// Draw an image where <src>scaledImage</src> gives the Colormap
		// index of each screen (PixelCanvas) pixel.  'blc' contains X and Y
		// PixelCanvas coordinates.  Masked version as well.
		// For masked version, set opaqueMask to True to draw masked pixels in
		// the background color; otherwise they will be transparent (letting
		// whatever was drawn previously at that point show through).
		// <group>
		void mapToColorAndDrawImage(const Vector<Int> &blc,
		                            const Matrix<uInt> &scaledImage);
		void mapToColorAndDrawImage(const Vector<Int> &blc,
		                            const Matrix<uInt> &scaledImage,
		                            const Matrix<Bool> &mask,
		                            Bool opaqueMask=False);
		// </group>
		// Draw a set of points where <src>scaledValues</src> gives the
		// Colormap index of each point.  Point coordinates ('points')
		// are either linear or world coordinates, as specified by 'linear'.
		Bool mapToColorAndDrawPoints(const Matrix<Double> &points,
		                             const Vector<uInt> &scaledValues,
		                             const Bool &linear = False);

		// Draw a set of ellipses where <src>scaledValues</src> gives the
		// Colormap index of each point.  Point coordinates ('points')
		// are either linear or world coordinates, as specified by 'linear'.
		Bool mapToColorAndDrawEllipses(const Matrix<Double> &centres,
		                               const Vector<Float> &smajor,
		                               const Vector<Float> &sminor,
		                               const Vector<Float> &pangle,
		                               const Vector<uInt> scaledValues,
		                               const Float &scale,
		                               const Bool &outline,
		                               const Bool &linear);




		// <b>***Cached***</b> blc pixel where this world canvas begins =
		// itsPixelCanvas->width()*frac(X|Y)Offset_.
		// <group>
		uInt itsCanvasXOffset;
		uInt itsCanvasYOffset;
		// </group>

		// <b>***Cached***</b> number of pixels in each dimension given to
		// the world canvas = itsPixelCanvas->width()*(frac(X|Y)Size_.
		// <group>
		uInt itsCanvasXSize;
		uInt itsCanvasYSize;
		// </group>

		// Fractional position of world canvas on pixel canvas.  The offset
		// values are always in the range of [0.0,1.0], and sizes must be
		// smaller or equal to (1.0 - offset) for each dimension.
		// <group>
		Double itsFracXOffset;
		Double itsFracYOffset;
		Double itsFracXSize;
		Double itsFracYSize;
		// </group>

		// <b>***Cached***</b> blc pixel where the world canvas 'draw area'
		// (inside margins, labels) begins, relative to WC blc.
		// <group>
		uInt itsCanvasDrawXOffset;
		uInt itsCanvasDrawYOffset;
		// </group>

		// <b>***Cached***</b> number of pixels in each dimension given to
		// the drawable part of the world canvas
		// <group>
		uInt itsCanvasDrawXSize;
		uInt itsCanvasDrawYSize;
		// </group>

		// Linear Coordinate System ranges.
		// <group>
		Double itsLinXMin;
		Double itsLinYMin;
		Double itsLinXMax;
		Double itsLinYMax;
		// </group>

		// Linear Coordinate System Limits.
		// <group>
		Double itsLinXMinLimit;
		Double itsLinYMinLimit;
		Double itsLinXMaxLimit;
		Double itsLinYMaxLimit;
		// </group>

		// Dynamic data minimum and maximum for this WorldCanvas.
		Double itsDataMin, itsDataMax;

		// Method to use to convert complex data into real values.
		Display::ComplexToRealMethod itsComplexToRealMethod;

		// Event handler lists and convenient iterators.
		// <group>
		List<DisplayEH *> itsRefreshEHList;
		List<WCPositionEH *> itsPositionEHList;
		List<WCMotionEH *> itsMotionEHList;
		mutable ListIter<DisplayEH *> *itsREHListIter;
		mutable ListIter<WCPositionEH *> *itsPEHListIter;
		mutable ListIter<WCMotionEH *> *itsMEHListIter;
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
		Bool itsOwnSizeControlHandler;
		Bool itsOwnCoordinateHandler;
		Bool itsOwnResampleHandler;
		Bool itsOwnDataScaleHandler;
		// </group>

		// Buffer for Attributes.
		AttributeBuffer attributes;

		// Background/foreground colors.
		String itsWorldBackgroundColor;
		String itsWorldForegroundColor;

		// PixelCanvas pointer.
		PixelCanvas *itsPixelCanvas;

		// PGPLOT filter.
		WCPGFilter *itsPGFilter;

		// Status of hold/release.
		Int itsHoldCount;
		Bool itsRefreshHeld;
		Display::RefreshReason itsHeldReason;

		// The DisplayCoordinateSystem for this WorldCanvas.  New addition, only
		// supported and used by "new" classes.
		DisplayCoordinateSystem *itsCoordinateSystem;

		// This state is set True when the pointer is in this WC and a pointer
		// button is pressed (with no buttons pressed previously).  When True,
		// all PC motion and pointer button events are propagated to this WC's
		// handlers (only), regardless of whether the pointer has moved off the WC.
		// It is reset to False when all buttons are released.  This simulates
		// the 'automatic grab' (implemented in X for whole windows), on the WC
		// level.
		Bool itsGrabbing;


		// ColorIndexedImage_ stores the state of a WorldCanvas::drawImage()
		// rendering after the chosen data plane has been resampled to screen
		// (Pixelcanvas) pixels and scaled to indices within a fixed-size
		// Colormap, but prior to mapping to actual colors for display.
		// 'Caching' of this state is useful in 24-bit (TrueColor) applications,
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
			Matrix<uInt> data;		// colormap indices
			Matrix<Bool> mask;
			Vector<Int> blc;
			uInt colormapSize;

			ColorIndexedImage_() : data(), mask(), blc(2,0) {  }
			void clear() {
				data.resize();
				mask.resize();
				blc=0;
			}
			Bool maskValid() {
				return mask.nelements()!=0u &&
				       mask.shape().isEqual(data.shape());
			}
		};

		ColorIndexedImage_* makeColorIndexedImage(const Vector<Double> &blc,
				            const Vector<Double> &trc,
				            const Matrix<Float> &data,
				            const Bool usePixelEdges, void* drawObj);
		Matrix<uInt> mapToColor( const Matrix<uInt> & scaledImage);
		// Cache of pre-drawn ColorIndexedImage_'s.   When a caller of drawImage()
		// wants to save one, it passes a drawing-object pointer in the 'drawObj'
		// parameter for use as a retrieval key.  It should provide the same key
		// to redrawIndexedImage() in order to reuse the image.
		//# This mechanism attempts to avoid some erroneous reuses of
		//# 'itsCachedImage' among different DDs (bugs 4937, 5032).   (dk 3/05)
		SimpleOrderedMap<void*, ColorIndexedImage_*> images_;

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

		uInt itsId; // id of wc instance


		// WorldCanvas::refresh is a recursive function. itsRef is used to
		// determine when the recursion is over. i.e, when the initial
		// refresh call is exiting
		uInt itsRef;

		std::string error_string;
	};


} //# NAMESPACE CASA - END

#endif
