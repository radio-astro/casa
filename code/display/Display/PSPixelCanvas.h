//# PSPixelCanvas.h: implementation of PixelCanvas for PostScript devices
//# Copyright (C) 1999,2000,2001,2002,2003
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

#ifndef TRIALDISPLAY_PSPIXELCANVAS_H
#define TRIALDISPLAY_PSPIXELCANVAS_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/PCMotionEH.h>
#include <display/DisplayEvents/PCPositionEH.h>
#include <display/DisplayEvents/PCRefreshEH.h>
#include <display/Display/PCVGBuffer.h>
#include <display/Display/PSPixelCanvasColorTable.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/DLFont.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of the PixelCanvas for PostScript devices.
// </summary>
//
// <prerequisite>
// <ol>
// <li> <linkto class="PixelCanvas">PixelCanvas</linkto>
// <li> <linkto class="PSPixelCanvasColorTable">PSPixelCanvasColorTable</linkto>
// <li> <linkto class="PSDriver">PSDriver</linkto>
// </ol>
// </prerequisite>
// <use visibility=export>
//
// <etymology>
// PSPixelCanvas is a mechanism for drawing on paper.
// </etymology>
//

//
// <motivation>
// </motivation>
//
// <synopsis>
// PSPixelCanvas implements <linkto class="PixelCanvas">PixelCanvas</linkto>
// for PostScript output. See
// <linkto class="PSPixelCanvas#Example">Example</linkto> for example
// usage.
// </synopsis>

//<a name="Example"></a>
// <example>
// <ol>
// <li> Create an output stream.
// <li> Create a PSDriver choosing a page type (letter, A4, etc.).
//	(PSDriver can take a filename as an argument instead of an ostream).
// See: <linkto class="PSDriver#Synopsis">PSDriver</linkto> for more on
//	creating PSDrivers..
// <li> Create PSPixelCanvasColorTable
// <li> Create PSPixelCanvas
// <li> Draw something pretty.
// <li> Delete everything and close the stream.
// <note role=tip>Neither PSPixelCanvasColorTable nor PSPixelCanvas delete
//	their PSDriver.</note>
// </ol>
// <srcblock>
//
// PSDriver *psd = new PSDriver("test.ps", PSDriver::LETTER);
// PSPixelCanvasColorTable *pscct = new PSPixelCanvasColorTable(psd);
// PSPixelCanvas psc = new PSPixelCanvas( psd, pscct);
//  ... (a miracle happens).
// delete psc;
// delete pscct;
// delete psd;
// </srcblock>
//
// <note role=tip>Since PostScript does not deal in in pixels, PSPixelCanvas
// assumes an adjustable number of pixels per inch. (Currently 1200dpi).
// Use the setResolution function to change this if the calling progam
// can't handle a 128M pixel display.</note>
// </example>
//
// <todo>
// <ol>
// <li>	Lots of 'get' functions. Some don't make sense for PostScript.
//	Others haven't been done yet.
// <li>	Events.
// <li> Partial support for X11 color names is hacked in. Doing it
//	'correctly' would require changing the constructors.
// <li> No display list support.
// <li> Foreground/background/clear color support is in. However, these
//	and many other routines don't really support RGB mode yet.
// <li> See: <linkto class="PSDriver#ToDo">PSDriver</linkto> for another list.
// </ol>
// </todo>
	class PSPixelCanvas : public PixelCanvas {
	public:
		PSPixelCanvas(PSDriver *ps, PSPixelCanvasColorTable * );
		virtual ~PSPixelCanvas();

		// enabling/disabling of event tracking
		// <group>
		virtual void enableMotionEvents();
		virtual void disableMotionEvents();
		virtual void enablePositionEvents();
		virtual void disablePositionEvents();
		// </group>

		// Does this canvas support cached display lists?  The user of the
		// canvas should always check this, because undefined behaviour can
		// result when an attempt is made to use a list on a PixelCanvas
		// which does not support lists.
		virtual casacore::Bool supportsLists() {
			return false;
		}

		// begin caching display commands - return list ID
		virtual casacore::uInt newList();
		// end caching display commands
		virtual void endList();
		// (Cacheable) recall cached display commands
		virtual void drawList(casacore::uInt list);
		// translate all lists
		virtual void translateAllLists(casacore::Int xt, casacore::Int yt);
		// translate the list
		virtual void translateList(casacore::uInt list, casacore::Int xt, casacore::Int yt);
		// remove list from cache
		virtual void deleteList(casacore::uInt list);
		// flush all lists from the cache
		virtual void deleteLists();
		// return true if the list exists
		virtual casacore::Bool validList(casacore::uInt list);

		// (Cacheable) Set the font to the PostScript font name and size.
		virtual casacore::Bool setFont(const casacore::String &fontName, const casacore::Int size);
		virtual casacore::Bool setFont(DLFont* font) {
			return setFont(font->getPSValue(), font->getSize());
		}

		// (Cacheable) Set the font to the PostScript font name.
		virtual casacore::Bool setFont(const casacore::String &fontName);
		// (Cacheable) Draw text using that font aligned in some way to the
		// position
		virtual void drawText(casacore::Int x, casacore::Int y, const casacore::String &text,
		                      Display::TextAlign alignment = Display::AlignCenter);
		virtual void drawText(casacore::Int x, casacore::Int y, const casacore::String &text, const casacore::Float& angle,
		                      Display::TextAlign alignment = Display::AlignCenter);
		// (Cacheable) Draw an array of 2D color data as a raster image for zoom = <1,1>
		// <group>
		virtual void drawImage(const casacore::Matrix<casacore::uInt> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Int> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::uLong> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Float> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Double> &data, casacore::Int x, casacore::Int y);
		// </group>
		// <group>
		// Draw a 2D color image with masking.
		// (opaqueMask ignored; (unfortunately,) PSPixelcanvas always draws
		// _opaque_ masks).
		virtual void drawImage(const casacore::Int &x, const casacore::Int &y,
		                       const casacore::Matrix<casacore::uInt> &data,
		                       const casacore::Matrix<casacore::Bool> &mask,
		                       casacore::Bool opaqueMask=false);
		// </group>

		// (Cacheable) Draw an array of 2D color data as a raster image for any positive integer zoom
		// <group>
		virtual void drawImage(const casacore::Matrix<casacore::uInt> &data, casacore::Int x, casacore::Int y,
		                       casacore::uInt xzoom, casacore::uInt yzoom);
		virtual void drawImage(const casacore::Matrix<casacore::Int> &data, casacore::Int x, casacore::Int y,
		                       casacore::uInt xzoom, casacore::uInt yzoom);
		virtual void drawImage(const casacore::Matrix<casacore::uLong> &data, casacore::Int x, casacore::Int y,
		                       casacore::uInt xzoom, casacore::uInt yzoom);
		virtual void drawImage(const casacore::Matrix<casacore::Float> &data, casacore::Int x, casacore::Int y,
		                       casacore::uInt xzoom, casacore::uInt yzoom);
		virtual void drawImage(const casacore::Matrix<casacore::Double> &data, casacore::Int x, casacore::Int y,
		                       casacore::uInt xzoom, casacore::uInt yzoom);
		// </group>

		// (Cacheable) Draw a component of a multi-channel image, storing it
		// in buffers until flushComponentImages() is called.
		virtual void drawImage(const casacore::Matrix<casacore::uInt> &data, const casacore::Int &x, const casacore::Int &y,
		                       const Display::ColorComponent &colorcomponent);

		// Fill one of the channel buffers.
		void bufferComponent(const casacore::Matrix<casacore::uInt> &data,
		                     const casacore::Int &x, const casacore::Int &y,
		                     const Display::ColorComponent &colorcomponent);

		// (NOT CACHEABLE!) Flush the component buffers.
		void flushComponentBuffers();

		// (Cacheable) Draw a single point using current color
		// <group>
		virtual void drawPoint(casacore::Int x1, casacore::Int y1);
		virtual void drawPoint(casacore::Float x1, casacore::Float y1);
		virtual void drawPoint(casacore::Double x1, casacore::Double y1);
		// </group>

		// (Cacheable) Draw N points specified as a Nx2 matrix
		// <group>
		virtual void drawPoints(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPoints(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPoints(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of points using current color
		// <group>
		virtual void drawPoints(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1);
		virtual void drawPoints(const casacore::Vector<casacore::Float> &x1, const casacore::Vector<casacore::Float> &y1);
		virtual void drawPoints(const casacore::Vector<casacore::Double> &x1,
		                        const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw a single line using current color
		// <group>
		virtual void drawLine(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void drawLine(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2);
		virtual void drawLine(casacore::Double x1, casacore::Double y1, casacore::Double x2, casacore::Double y2);
		// </group>

		// (Cacheable) Draw N/2 lines from an Nx2 matrix
		// <group>
		virtual void drawLines(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawLines(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawLines(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of unrelated lines using current color
		// <group>
		virtual void drawLines(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1,
		                       const casacore::Vector<casacore::Int> &x2, const casacore::Vector<casacore::Int> &y2);
		virtual void drawLines(const casacore::Vector<casacore::Float> &x1, const casacore::Vector<casacore::Float> &y1,
		                       const casacore::Vector<casacore::Float> &x2, const casacore::Vector<casacore::Float> &y2);
		virtual void drawLines(const casacore::Vector<casacore::Double> &x1, const casacore::Vector<casacore::Double> &y1,
		                       const casacore::Vector<casacore::Double> &x2, const casacore::Vector<casacore::Double> &y2);
		// </group>

		// (Cacheable) Draw a single connected line between the points given
		// <group>
		virtual void drawPolyline(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1);
		virtual void drawPolyline(const casacore::Vector<casacore::Float> &x1,
		                          const casacore::Vector<casacore::Float> &y1);
		virtual void drawPolyline(const casacore::Vector<casacore::Double> &x1,
		                          const casacore::Vector<casacore::Double> &y1);
		// </group>
		// <group>
		virtual void drawPolylines(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1,
		                           const int close=0, const int fill=0);
		virtual void drawPolylines(const casacore::Vector<casacore::Float> &x1,
		                           const casacore::Vector<casacore::Float> &y1,
		                           const int close=0, const int fill=0);
		virtual void drawPolylines(const casacore::Vector<casacore::Double> &x1,
		                           const casacore::Vector<casacore::Double> &y1,
		                           const int close=0, const int fill=0);
		// </group>

		// (Cacheable) Draw N-1 connected lines from Nx2 matrix of vertices
		// <group>
		virtual void drawPolyline(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPolyline(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPolyline(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw N-1 connected lines from Nx2 matrix of vertices
		// <group>
		virtual void drawPolylines(const casacore::Matrix<casacore::Int> &verts,
		                           const int close=0, const int fill=0);
		virtual void drawPolylines(const casacore::Matrix<casacore::Float> &verts,
		                           const int close=0, const int fill=0);
		virtual void drawPolylines(const casacore::Matrix<casacore::Double> &verts,
		                           const int close=0, const int fill=0);
		// </group>

		// (Cacheable) Draw a closed polygon
		// <group>
		virtual void drawPolygon(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1);
		virtual void drawPolygon(const casacore::Vector<casacore::Float> &x1, const casacore::Vector<casacore::Float> &y1);
		virtual void drawPolygon(const casacore::Vector<casacore::Double> &x1,
		                         const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw and fill a closed polygon
		// <group>
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Int> &x1,
		                               const casacore::Vector<casacore::Int> &y1);
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Float> &x1,
		                               const casacore::Vector<casacore::Float> &y1);
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Double> &x1,
		                               const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw a closed N-sided polygon from Nx2 matrix of vertices
		// <group>
		virtual void drawPolygon(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPolygon(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPolygon(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a rectangle
		// <group>
		virtual void drawRectangle(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2, const int fill);
		virtual void drawRectangle(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2,
		                           const int fill);
		virtual void drawRectangle(casacore::Double x1, casacore::Double y1, casacore::Double x2, casacore::Double y2,
		                           const int fill);
		// </group>

		// (Cacheable) Draw a rectangle
		// <group>
		virtual void drawRectangle(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void drawRectangle(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2);
		virtual void drawRectangle(casacore::Double x1, casacore::Double y1, casacore::Double x2, casacore::Double y2);
		// </group>

		// (Cacheable) Draw a filled rectangle
		// <group>
		virtual void drawFilledRectangle(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void drawFilledRectangle(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2);
		virtual void drawFilledRectangle(casacore::Double x1, casacore::Double y1, casacore::Double x2,
		                                 casacore::Double y2);
		// </group>

		// (Cacheable) Draw a set of points, specifying a color per point to be drawn.
		// <group>
		virtual void drawColoredPoints(const casacore::Vector<casacore::Int> &x1,
		                               const casacore::Vector<casacore::Int> &y1,
		                               const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredPoints(const casacore::Vector<casacore::Float> &x1,
		                               const casacore::Vector<casacore::Float> &y1,
		                               const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredPoints(const casacore::Vector<casacore::Double> &x1,
		                               const casacore::Vector<casacore::Double> &y1,
		                               const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredPoints(const casacore::Matrix<casacore::Int> &xy,
		                               const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredPoints(const casacore::Matrix<casacore::Float> &xy,
		                               const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredPoints(const casacore::Matrix<casacore::Double> &xy,
		                               const casacore::Vector<casacore::uInt> &colors);
		// </group>

		// (Cacheable) Draw a set of lines, specifying a color per line to be drawn.
		// <group>
		virtual void drawColoredLines(const casacore::Vector<casacore::Int> &x1,
		                              const casacore::Vector<casacore::Int> &y1,
		                              const casacore::Vector<casacore::Int> &x2,
		                              const casacore::Vector<casacore::Int> &y2,
		                              const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredLines(const casacore::Vector<casacore::Float> &x1,
		                              const casacore::Vector<casacore::Float> &y1,
		                              const casacore::Vector<casacore::Float> &x2,
		                              const casacore::Vector<casacore::Float> &y2,
		                              const casacore::Vector<casacore::uInt> &colors);
		virtual void drawColoredLines(const casacore::Vector<casacore::Double> &x1,
		                              const casacore::Vector<casacore::Double> &y1,
		                              const casacore::Vector<casacore::Double> &x2,
		                              const casacore::Vector<casacore::Double> &y2,
		                              const casacore::Vector<casacore::uInt> &colors);
		// </group>

		// Set Graphics Attributes
		// Options for functions with enum argument
		// listed in <linkto class=Display>DisplayEnums</linkto>
		// <group>
		virtual void setDrawFunction(Display::DrawFunction function);
		virtual void setForeground(casacore::uLong color);
		virtual void setBackground(casacore::uLong color);
		//virtual void setLineWidth(casacore::uInt width);
		virtual void setLineWidth(casacore::Float width);
		void lineWidth(casacore::Float width);
		float lineWidth()const {
			return lineWidth_;
		}
		virtual void setLineStyle(Display::LineStyle style);
		virtual void setCapStyle(Display::CapStyle style);
		virtual void setJoinStyle(Display::JoinStyle style);
		virtual void setFillStyle(Display::FillStyle style);
		virtual void setFillRule(Display::FillRule rule);
		virtual void setArcMode(Display::ArcMode mode);
		// </group>

		// Get Graphics Attributes
		// <group>
		virtual Display::DrawFunction getDrawFunction() const;
		virtual casacore::uLong                 getForeground()   const;
		virtual casacore::uLong                 getBackground()   const;
		//#virtual casacore::uInt                getLineWidth()    const;
		virtual casacore::Float                 getLineWidth()    const;
		virtual Display::LineStyle    getLineStyle()    const;
		virtual Display::CapStyle     getCapStyle()     const;
		virtual Display::JoinStyle    getJoinStyle()    const;
		virtual Display::FillStyle    getFillStyle()    const;
		virtual Display::FillRule     getFillRule()     const;
		virtual Display::ArcMode      getArcMode()      const;
		// </group>

		// (Cacheable) Option Control
		// Options listed in <linkto class=Display>DisplayEnums</linkto>
		// <group>
		virtual casacore::Bool enable(Display::Option option);
		virtual casacore::Bool disable(Display::Option option);
		// </group>

		// Control the image-caching strategy
		virtual void setImageCacheStrategy(Display::ImageCacheStrategy strategy);
		virtual Display::ImageCacheStrategy imageCacheStrategy() const;

		// (Cacheable) Setup the clip window.  The clip window, when enabled, allows
		// a user to clip all graphics output to a rectangular region on
		// the screen.
		// Once set, it is only possible to REDUCE the size of the current clip
		// window, not increase it.
		// <group>
		virtual void setClipWindow(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void getClipWindow(casacore::Int &x1, casacore::Int &y1, casacore::Int &x2, casacore::Int &y2);
		// </group>

		// (Not Cacheable) Redraw the window
		// <group>
		virtual void refresh(const Display::RefreshReason &reason =
		                         Display::UserCommand,
		                     const casacore::Bool &explicitrequest = true);
		// </group>

		// Cause display to flush any graphics commands not yet drawn
		virtual void flush();

		// (Cacheable) Clear the window using the background color
		// <group>
		virtual void clear();
		virtual void clear(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		// </group>

		// (Cacheable) Set the color to use for clearing the display
		// <group>
		virtual void setClearColor(casacore::uInt colorIndex);
		virtual void setClearColor(const casacore::String &colorname);
		virtual void setClearColor(float r, float g, float b);
		// </group>

		// (Not Cacheable) Get the current color to use for clearing the display.
		virtual casacore::uInt clearColor() const;
		virtual void getClearColor(float &r, float &g, float &b) const;

		// Return the width of the PSPixelCanvas in pixels
		virtual casacore::uInt width() const;
		// Return the height of the PSPixelCanvas in pixels
		virtual casacore::uInt height() const;
		// Return the depth of the PSPixelCanvas in bits
		virtual casacore::uInt depth() const;

		// Get the pixel density (in dots per inch [dpi]) of the PixelCanvas
		virtual void pixelDensity(casacore::Float &xdpi, casacore::Float &ydpi) const;

		// (Cacheable) Set current color (works in RGB or colormap mode)
		// <group>
		virtual void setColor(casacore::uInt colorIndex);
		virtual void setColor(const casacore::String &colorname);
		virtual void setRGBColor(float r, float g, float b);
		virtual void setHSVColor(float h, float s, float v);
		// </group>

		// (Not Cacheable) Returns the current color as a color index
		virtual casacore::uInt color() const;

		// (Not Cacheable) Retuns the current color as an RGB triple
		virtual void getColor(float &r, float &g, float &b) const;
		virtual casacore::Bool getColorComponents(const casacore::String &colorname,
		                                casacore::Float &r, casacore::Float &g, casacore::Float &b);
		// (Not Cacheable) Get color index value (works in RGB or colormap mode)
		// <group>
		virtual casacore::Bool getColor(casacore::Int x, casacore::Int y, casacore::uInt &color);
		virtual casacore::Bool getRGBColor(casacore::Int x, casacore::Int y, float &r, float &g, float &b);
		virtual casacore::Bool getHSVColor(casacore::Int x, casacore::Int y, float &h, float &s, float &v);
		// </group>
		// Get/set the current foreground/background colors.  These colors
		// should be used when the special Strings "foreground" and "background"
		// are given for a color.
		// <group>
		virtual void setDeviceForegroundColor(const casacore::String colorname);
		virtual casacore::String deviceForegroundColor() const;
		virtual void setDeviceBackgroundColor(const casacore::String colorname);
		virtual casacore::String deviceBackgroundColor() const;
		// </group>

		// <group>
		// (Not Cacheable) resize request.  returns true if window was resized.
		// Will refresh if doCallbacks is true.
		virtual casacore::Bool resize(casacore::uInt reqXSize, casacore::uInt reqYSize, casacore::Bool doCallbacks = true);

		// (Not Cacheable) resize the colortable by requesting a new number of cells
		virtual casacore::Bool resizeColorTable(casacore::uInt newSize);

		// (Not Cacheable) resize the colortable by requesting a new RGB/HSV cube
		virtual casacore::Bool resizeColorTable(casacore::uInt nReds, casacore::uInt nGreens, casacore::uInt nBlues);

		// Need a mechanism to return the PSPixelCanvasColorTable so
		// drawing functions within classes can operate.
		virtual PixelCanvasColorTable * pcctbl() const;

		virtual void setPcctbl(PixelCanvasColorTable * pcctbl);
		PSPixelCanvasColorTable *PSpcctbl()const {
			return pcct_;
		}
		// </group>

		// save/restore the current translation.  This is called pushMatrix because
		// eventually we may want scaling or rotation to play a modest
		// role here.
		// <group>
		virtual void pushMatrix();
		virtual void popMatrix();
		// </group>
		// zero the current translation
		virtual void loadIdentity();

		// translation functions
		// translate applies a relative translation to the current matrix and
		// can be used to position graphics.  Together with pushMatrix and
		// popMatrix it can be used to build heirarchical scenes.
		// <group>
		virtual void translate(casacore::Int xt, casacore::Int yt);
		virtual void getTranslation(casacore::Int &xt, casacore::Int &yt) const;
		virtual casacore::Int xTranslation() const;
		virtual casacore::Int yTranslation() const;
		// </group>

		// (Not cacheable) set the draw buffer
		// None of the buffer routines are supported.
		virtual void setDrawBuffer(Display::DrawBuffer buf);
		// buffer memory exchanges
		// (Not cacheable)
		// <group>
		virtual void copyBackBufferToFrontBuffer();
		virtual void copyFrontBufferToBackBuffer();
		virtual void swapBuffers();
		// </group>

		// partial buffer memory exchanges.  (x1,y1 are blc, x2,y2 are trc)
		// <group>
		virtual void copyBackBufferToFrontBuffer(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void copyFrontBufferToBackBuffer(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void swapBuffers(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		// </group>

//#  ////////////////////////////////////////////////////////////////
		//		PSPixelCanvas specific misc. functions.

//#  // Converts from "pixels" to points.
//# //<group>
//#   inline float xToPoints(const double x)const{return (float)(x*xppp_);}
//#   inline float yToPoints(const double y)const{return (float)(y*yppp_);}
//# //</group>
//#   // Convert from points to pixels.
//# //<group>
//#   inline double pointsToX(const float x)const{return (double)(x/xppp_);}
//#   inline double pointsToY(const float y)const{return (double)(y/yppp_);}
//# //</group>

		// Draw a scaled image on PostScript canvas.
		void drawPSImage(const int x, const int y,
		                 const int iwidth, const int iheight,
		                 const float xzoom, const float yzoom,
		                 const casacore::uShort *data, const int bpc,
		                 const Display::ColorModel=Display::Index);
		// Set/get declared resolution in pixels per supplied dimension.
//<group>
		void setResolution(const float xres, const float yres,
		                   const PSDriver::Dimension = PSDriver::INCHES);
		void getResolution(float &xres, float &yres,
		                   const PSDriver::Dimension = PSDriver::INCHES)const;
//</group>
		// Whether to enable pixel interpolation for drawImage().
//<group>
		void smooth(const casacore::Bool smth) {
			smooth_ = smth;
		}
		casacore::Bool smooth()const {
			return smooth_;
		}
//</group>
		// Whether to put tracing comments in the output.
//<group>
		casacore::Bool annotate()const {
			return annotate_;
		}
		void annotate(const casacore::Bool a) {
			annotate_ = a;
		}
//</group>

		virtual casacore::Float pixelScaling() const;

//#  ////////////////////////////////////////////////////////////////

	protected:


	private:
		void pspcinit(PSDriver *ps, PSPixelCanvasColorTable * );
		void clearRectangle(const float x0, const float y0,
		                    const float x1, const float y1);
// Used in annotating output file.
//<group>
		void note(const char *);
		void note(const char *, const casacore::uLong);
//</group>
	private:
		PSPixelCanvasColorTable *pcct_;
		PSDriver		*ps;
		float			xppp_, yppp_; // # Of points per pixel.
		float			lineWidth_;
		Display::LineStyle	lineStyle_;
		Display::CapStyle	capStyle_;
		Display::JoinStyle	joinStyle_;
		Display::FillStyle	fillStyle_;
		Display::FillRule	fillRule_;
		Display::ArcMode	arcMode_;
		casacore::String			defaultForegroundColorName_;
		casacore::String			defaultBackgroundColorName_;
		// Color indexes the three colors we know about.
		casacore::uLong			foreground_, background_, clearColor_;
		float			xt_, yt_;
		float			clipx0_, clipy0_, clipx1_, clipy1_;
		casacore::Bool			haveClipInfo_;
		casacore::Bool			clipEnabled_;
		casacore::Bool			smooth_;
		casacore::Bool			annotate_;
		// Component support
		casacore::Float itsComponentX, itsComponentY;
		casacore::uLong itsComponentWidth, itsComponentHeight;
		casacore::Int itsComponentBPC;
		casacore::uShort *itsComponents; // Length = 3*width*height.
	};


} //# NAMESPACE CASA - END

#endif
