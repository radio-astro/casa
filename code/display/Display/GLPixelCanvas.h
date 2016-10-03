//# GLPixelCanvas.h: Class defining OpenGL version PixelCanvas.
//# Copyright (C) 2001
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
//# $Id $

#ifndef TRIALDISPLAY_GLPIXELCANVAS_H
#define TRIALDISPLAY_GLPIXELCANVAS_H

#include <casa/aips.h>
#include <graphics/X11/X_enter.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <graphics/X11/X_exit.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/GLPixelCanvasColorTable.h>
#include <display/Display/GLSupport.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class GLPCDisplayList;
	class GLPCDisplayListEntry;
	class GLPCDisplayListElement;
	class GLPCTextureParams;

// <summary>
//	OpenGL implementation of PixelCanvas.
// </summary>

// <use visibility=export>

// <prerequisite>
// <li> <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// <li> <linkto class="GLPixelCanvasColorTable">GLPixelCanvasColorTable</linkto>
// <li> <linkto class="PixelCanvas">PixelCanvas</linkto>
// <li> <linkto class="X11PixelCanvas"> Knowledge of X11PixelCanvas could help</linkto>
// </prerequisite>
//
// <synopsis>
// Implementation of PixelCanvas for OpenGL. Most, but not all of the
// PixelCanvas functions are available. There is very little support for
// 3D.
// </synopsis>
// <thrown>
//    None
// </thrown>
// <todo asof="2001/09/25">
//#   <li>
// </todo>

	class GLPixelCanvas : public PixelCanvas {
	public:
// <note role=caution>
// Drawing can not take place until the window exists!
// </note>
		GLPixelCanvas(Widget parent, GLPixelCanvasColorTable * glpcctbl,
		              casacore::uInt width, casacore::uInt height);
		virtual ~GLPixelCanvas();

		// enabling/disabling of event tracking (unimplemented)
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
		virtual casacore::Bool supportsLists();

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

		// (Cacheable) Set the font to the recognizable font name. (unimplemented)
		virtual casacore::Bool setFont(const casacore::String &fontName);
		// (Cacheable) Draw text using that font aligned in some way to the
		// position. (unimplemented)
		virtual void drawText(casacore::Int x, casacore::Int y, const casacore::String &text,
		                      Display::TextAlign alignment = Display::AlignCenter);

		// (Cacheable) Draw an array of 2D color data as a raster image for zoom = <1,1>
		// <group>
		virtual void drawImage(const casacore::Matrix<casacore::uInt> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Int> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::uLong> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Float> &data, casacore::Int x, casacore::Int y);
		virtual void drawImage(const casacore::Matrix<casacore::Double> &data, casacore::Int x, casacore::Int y);
		// </group>
		// (Cacheable) Draw an image using color indexes. If in RGB mode,
		// the virtual colortable is used.
		void GLPixelCanvas::drawIndexedImage( const casacore::Matrix<casacore::uInt> &data,
		                                      casacore::Float x=0.0, casacore::Float y=0.0, casacore::Float z=0.0,
		                                      casacore::Float xscl=1.0, casacore::Float yscl=1.0);
		// (Cacheable) Draw an array of 2D color data as a raster image,
		// taking note of the <src>casacore::Bool</src> mask.
		// (opaqueMask ignored/unimplemented)
		// <group>
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
		virtual void bufferComponent(const casacore::Matrix<casacore::uInt> &data,
		                             const casacore::Int &x, const casacore::Int &y,
		                             const Display::ColorComponent
		                             &colorcomponent);

		// (NOT CACHEABLE!) Flush the component buffers.
		virtual void flushComponentBuffers();

		// (Cacheable) Draw a single point using current color
		// <group>
		virtual void drawPoint(casacore::Int x1, casacore::Int y1);
		virtual void drawPoint(casacore::Float x1, casacore::Float y1);
		virtual void drawPoint3D(casacore::Float x1, casacore::Float y1, casacore::Float z1);
		virtual void drawPoint(casacore::Double x1, casacore::Double y1);
		// </group>

		// (Cacheable) Draw N points specified as a Nx2 matrix
		// <group>
		virtual void drawPoints(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPoints(const casacore::Matrix<casacore::Float> &verts);
		void drawPoints3D(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPoints(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of points using current color
		// <group>
		virtual void drawPoints(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1);
		virtual void drawPoints(const casacore::Vector<casacore::Float> &x1,
		                        const casacore::Vector<casacore::Float> &y1);
		virtual void drawPoints3D(const casacore::Vector<casacore::Float> &x1,
		                          const casacore::Vector<casacore::Float> &y1, const casacore::Vector<casacore::Float> &z1);
		virtual void drawPoints(const casacore::Vector<casacore::Double> &x1,
		                        const casacore::Vector<casacore::Double> &y1);
		// </group>
// (cacheable) Change the size of the points (in pixels) drawn by drawPoints.
		void setPointSize(const casacore::uInt pointsize);

		// (Cacheable) Draw a single line using current color
		// <group>
		virtual void drawLine(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void drawLine(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2);
		virtual void drawLine3D(casacore::Float x1, casacore::Float y1, casacore::Float z1,
		                        casacore::Float x2, casacore::Float y2, casacore::Float z2);
		virtual void drawLine(casacore::Double x1, casacore::Double y1, casacore::Double x2, casacore::Double y2);
		// </group>

		// (Cacheable) Draw N/2 lines from an Nx2 matrix
		// <group>
		virtual void drawLines(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawLines(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawLines3D(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawLines(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of unrelated lines using current color
		// <group>
		virtual void drawLines(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1,
		                       const casacore::Vector<casacore::Int> &x2, const casacore::Vector<casacore::Int> &y2);
		virtual void drawLines(const casacore::Vector<casacore::Float> &x1, const casacore::Vector<casacore::Float> &y1,
		                       const casacore::Vector<casacore::Float> &x2,
		                       const casacore::Vector<casacore::Float> &y2);
		virtual void drawLines(const casacore::Vector<casacore::Double> &x1, const casacore::Vector<casacore::Double> &y1,
		                       const casacore::Vector<casacore::Double> &x2,
		                       const casacore::Vector<casacore::Double> &y2);
		// </group>

		// (Cacheable) Draw a single connected line between the given points.
		// <group>
		virtual void drawPolyline(const casacore::Vector<casacore::Int> &x1,
		                          const casacore::Vector<casacore::Int> &y1);
		virtual void drawPolyline(const casacore::Vector<casacore::Float> &x1,
		                          const casacore::Vector<casacore::Float> &y1);
		virtual void drawPolyline3D(const casacore::Vector<casacore::Float> &x1,
		                            const casacore::Vector<casacore::Float> &y1,
		                            const casacore::Vector<casacore::Float> &z1);
		virtual void drawPolyline(const casacore::Vector<casacore::Double> &x1,
		                          const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw N-1 connected lines from Nx2 matrix of vertices
		// <group>
		virtual void drawPolyline(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPolyline(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPolyline(const casacore::Matrix<casacore::Double> &verts);
		// </group>

		// (Cacheable) Draw a closed polygon
		// <group>
		virtual void drawPolygon(const casacore::Vector<casacore::Int> &x1, const casacore::Vector<casacore::Int> &y1);
		virtual void drawPolygon(const casacore::Vector<casacore::Float> &x1,
		                         const casacore::Vector<casacore::Float> &y1);
		virtual void drawPolygon3D(const casacore::Vector<casacore::Float> &x1,
		                           const casacore::Vector<casacore::Float> &y1,
		                           const casacore::Vector<casacore::Float> &z1);
		virtual void drawPolygon(const casacore::Vector<casacore::Double> &x1,
		                         const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw and fill a closed polygon
		// <group>
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Int> &x1,
		                               const casacore::Vector<casacore::Int> &y1);
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Float> &x1,
		                               const casacore::Vector<casacore::Float> &y1);
		virtual void drawFilledPolygon3D(const casacore::Vector<casacore::Float> &x1,
		                                 const casacore::Vector<casacore::Float> &y1,
		                                 const casacore::Vector<casacore::Float> &z1);
		virtual void drawFilledPolygon(const casacore::Vector<casacore::Double> &x1,
		                               const casacore::Vector<casacore::Double> &y1);
		// </group>

		// (Cacheable) Draw a closed N-sided polygon from Nx2 matrix of vertices
		// <group>
		virtual void drawPolygon(const casacore::Matrix<casacore::Int> &verts);
		virtual void drawPolygon(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPolygon3D(const casacore::Matrix<casacore::Float> &verts);
		virtual void drawPolygon(const casacore::Matrix<casacore::Double> &verts);
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
		virtual void setForeground(casacore::uLong color);
		virtual void setLineWidth(casacore::Float width);
		virtual void setLineStyle(Display::LineStyle style);
		virtual void setFillStyle(Display::FillStyle style);
		virtual void setDrawFunction(Display::DrawFunction function);
		// </group>

		// Unimplemented set graphics functions.
		// <group>
		virtual void setBackground(casacore::uLong color);
		virtual void setCapStyle(Display::CapStyle style);
		virtual void setJoinStyle(Display::JoinStyle style);
		virtual void setFillRule(Display::FillRule rule);
		virtual void setArcMode(Display::ArcMode mode);
		// </group>

		// Implemented get Graphics Attributes functions.
		// <group>
		virtual casacore::uLong                 getForeground()   const;
		virtual casacore::Float                 getLineWidth()    const;
		virtual Display::LineStyle    getLineStyle()    const;
		virtual Display::FillStyle    getFillStyle()    const;
		virtual Display::DrawFunction getDrawFunction() const;
		// </group>
		// Unimplemented get Graphics Attributes.
		// <group>
		virtual casacore::uLong                 getBackground()   const;
		virtual Display::CapStyle     getCapStyle()     const;
		virtual Display::JoinStyle    getJoinStyle()    const;
		virtual Display::FillRule     getFillRule()     const;
		virtual Display::ArcMode      getArcMode()      const;
		// </group>

		// (Cacheable) Option Control
		// Options listed in <linkto class=Display>DisplayEnums</linkto>
		// Only Display::ClipWindow is implemented.
		// <group>
		virtual casacore::Bool enable(Display::Option option);
		virtual casacore::Bool disable(Display::Option option);
		// </group>

		// Control the image-caching strategy. (Ignored)
		virtual void setImageCacheStrategy(Display::ImageCacheStrategy strategy);
		virtual Display::ImageCacheStrategy imageCacheStrategy() const;

		// (Cacheable) Setup the clip window.  The clip window, when enabled, allows
		// a user to clip all graphics output to a rectangular region on
		// the screen.
		// <group>
		virtual void setClipWindow(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void getClipWindow(casacore::Int &x1, casacore::Int &y1, casacore::Int &x2, casacore::Int &y2);
		// </group>

		// (Not Cacheable) Redraw the window
		// <group>
		void redraw() {
			repaint(true);
		}
		virtual void refresh(const Display::RefreshReason &reason =
		                         Display::UserCommand,
		                     const casacore::Bool &explicitrequest = true);
		// Just redraws window. (If redraw is T, the window will be redrawn. If
		// F, it will only be redrawn if autoRefresh is T.
		void repaint(const casacore::Bool redraw=false);
		// </group>

		// Cause display to flush any graphics commands not yet drawn
		virtual void flush();

		// (Cacheable) Clear the window using the background color
		// <note role=tip> If you want to clear the window use clear(),
		// not clear(IIII).
		// </note>
		// <group>
		virtual void clear();
		virtual void clear(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		// </group>

		// (Cacheable) Set the color to use for clearing the display
		// <group>
		virtual void setClearColor(casacore::uInt colorIndex);
		// <note role=caution> Setting named colors for Index mode isn't implemented.
		// </note>
		virtual void setClearColor(const casacore::String &colorname);
		virtual void setClearColor(float r, float g, float b);
		// </group>

		// (Not Cacheable) Get the current color to use for clearing the display.
		virtual casacore::uInt clearColor() const;
		virtual void getClearColor(float &r, float &g, float &b) const;

		// Get/set the current foreground/background colors.  These colors
		// should be used when the special Strings "foreground" and "background"
		// are given for a color.
		// <group>
		virtual void setDeviceForegroundColor(const casacore::String colorname);
		virtual casacore::String deviceForegroundColor() const;
		virtual void setDeviceBackgroundColor(const casacore::String colorname);
		virtual casacore::String deviceBackgroundColor() const;
		// </group>

		// Return the width of the GLPixelCanvas in pixels
		virtual casacore::uInt width() const;
		// Return the height of the GLPixelCanvas in pixels
		virtual casacore::uInt height() const;
		// Return the depth of the GLPixelCanvas in bits
		virtual casacore::uInt depth() const;
		// Get the pixel density (in dots per inch [dpi]) of the GLPixelCanvas
		// <note role=tip> The X server is queried. It may or may not have the
		// correct answer. </note>
		virtual void pixelDensity(casacore::Float &xdpi, casacore::Float &ydpi) const;

		// (Cacheable) Set current color (works in RGB or colormap mode)
		// <group>
		// <note role=tip> setColor(colorIndex) when called in RGB mode gets the
		// RGB values from the virtual color table. setForeground assumes the
		// value is a packed pixel. </note>
		virtual void setColor(casacore::uInt colorIndex);
		virtual void setColor(const casacore::String &colorname);
		// <note role=caution> Calling setRGBColor when in Index mode ignores
		// r, g & b and uses the current index value. </note>
		virtual void setRGBColor(float r, float g, float b);
		// </group>

		// Get color components in range 0 to 1 without actually
		// allocating the color.  This is needed to set up other
		// devices, for example PgPlot.
		virtual casacore::Bool getColorComponents(const casacore::String &colorname, casacore::Float &r,
		                                casacore::Float &g, casacore::Float &b);

		// (Not Cacheable) Returns the current color as a color index
		// <note> In RGB mode, this is only valid if setColor(index) has
		// been called. </note>
		virtual casacore::uInt color() const;

		// (Not Cacheable) Retuns the current color as an RGB triple
		virtual void getColor(float &r, float &g, float &b) const;

		// (Not Cacheable) Get color index value (works in RGB or colormap mode)
		// <note role=tip> Don't bother. No existing canvas implements these.
		// </note>
		// <group>
		virtual casacore::Bool getColor(casacore::Int x, casacore::Int y, casacore::uInt &color);
		// This should be removed since no one implements it.
		virtual casacore::Bool getRGBColor(casacore::Int x, casacore::Int y, float &r, float &g, float &b);
		// </group>

		// (Not Cacheable) resize request.  returns true if window was resized.
		// Will refresh if doCallbacks is true.
		virtual casacore::Bool resize(casacore::uInt reqXSize, casacore::uInt reqYSize, casacore::Bool doCallbacks = true);

		// (Not Cacheable) resize the colortable by requesting a new number of cells
		virtual casacore::Bool resizeColorTable(casacore::uInt newSize);

		// (Not Cacheable) resize the colortable by requesting a new RGB/HSV cube
		virtual casacore::Bool resizeColorTable(casacore::uInt nReds, casacore::uInt nGreens, casacore::uInt nBlues);

		// Need a mechanism to return the GLPixelCanvasColorTable so
		// drawing functions within classes can operate.
		virtual GLPixelCanvasColorTable * glpcctbl() const {
			return glpcctbl_;
		}
		// Return the pixel canvas color table
		PixelCanvasColorTable * pcctbl() const {
			return glpcctbl_;
		}

		void setPcctbl(PixelCanvasColorTable * pcctbl) {
			glpcctbl_ = (GLPixelCanvasColorTable *) pcctbl;
		}

		// save/restore the current translation.  This is called pushMatrix because
		// eventually we may want scaling or rotation to play a modest
		// role here.
		// <note role=tip> GLPixelCanvas uses OpenGL's push/pop matrix. </note>
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
		// <note role=tip> get/x/yTranslation are not currently implemented. </note>
		virtual void getTranslation(casacore::Int &xt, casacore::Int &yt) const;
		virtual casacore::Int xTranslation() const;
		virtual casacore::Int yTranslation() const;
		// </group>

		// (Not cacheable) set the draw buffer
		// <note role=caution> The draw buffer is always set to BACK if double
		// buffered and FRONT if single buffered. </note>
		virtual void setDrawBuffer(Display::DrawBuffer buf);
		// buffer memory exchanges
		// (Not cacheable)
		// <group>
		// <note role=caution> Of this group, only swapbuffers() works. </note>
		static void copyBuffer(GLenum from, GLenum to,
		                       GLint x, GLint y,
		                       GLsizei width, GLsizei height);
		void copyBuffer(GLenum from, GLenum to);
		virtual void copyBackBufferToFrontBuffer();
		virtual void copyFrontBufferToBackBuffer();
		virtual void swapBuffers();
		// </group>

		// partial buffer memory exchanges.  (x1,y1 are blc, x2,y2 are trc)
		// None of these are implemented.
		// <group>
		virtual void copyBackBufferToFrontBuffer(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void copyFrontBufferToBackBuffer(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		virtual void swapBuffers(casacore::Int x1, casacore::Int y1, casacore::Int x2, casacore::Int y2);
		// </group>

		// return true if refresh is allowed right now...
		virtual casacore::Bool refreshAllowed() const;

		// Make this canvas's context current.
		// <note role=caution> This must be called when switching between
		// GLPixelCanvases or else drawing will go to the wrong canvas. It is
		// not needed unless more than one GLPixelCanvas is active at a time.
		// </note>
		void makeCurrent();
		// Wait for X or GL commands to finish. Usually not needed.
		// <group>
		void waitX();
		void waitGL();
		// </group>
		// Print any GL error messages.
		// if msg is non NULL, it is printed first if there is an error.
		// nspaces is the number of spaces to indent the text.
		// Returns the number of errors. Usually only used internally.
		// <group>
		int glcheck(const char *msg=NULL, casacore::uInt nspaces=0);
		static int glCheck(const char *msg=NULL, casacore::uInt nspaces=0);
		// </group>

		// Various ways to draw images. Used internally.
		// <group>
		void drawTexturedImage(GLfloat x, GLfloat y, GLfloat z,
		                       GLsizei width, GLsizei height,
		                       GLfloat xscale, GLfloat yscale,
		                       GLenum format, const GLushort *pixels,
		                       GLPCTextureParams *params = NULL);

		void drawImagePixels( GLfloat x, GLfloat y, GLfloat z,
		                      GLsizei width, GLsizei height,
		                      GLfloat xscale, GLfloat yscale,
		                      GLenum format, const GLushort *pixels);

		static void drawPixels(GLfloat x, GLfloat y, GLfloat z,
		                       GLsizei width, GLsizei height,
		                       GLfloat xscale, GLfloat yscale,
		                       GLenum format, GLenum type,
		                       const GLvoid *pixels);

		// Calls drawImagePixels if in indexed mode or drawTexturedImage if RGB.
		void drawGLImage(GLfloat x, GLfloat y, GLfloat z,
		                 GLsizei width, GLsizei height,
		                 GLfloat xscale, GLfloat yscale,
		                 GLenum format, const GLushort *pixels);
// </group>

		void bufferZValue(const casacore::Float z=0.0) {
			itsComponentZ_ = z;
		}

		// Tracing support. May be useful when debugging graphics code.
		// <group>
		// Turn tracing on/off.
		void trace(Boolean trace) {
			trace_ = trace;
		}
		Boolean tracing()const {
			return trace_;
		}
		// Set trace level. (actually a mask). default is GLTraceNormal.
		// NOT fully implemented.
		void traceLevel(const casacore::uLong traceLevel) {
			traceLevel_ = traceLevel;
		}
		casacore::uLong traceLevel()const {
			return traceLevel_;
		}
		void postToStream(casacore::Bool p=false) {
			log_.postToStream(p);
		}
		void postToString(casacore::String *s=NULL) {
			log_.postToString(s);
		}
		void postString() {
			log_.postString();
		}
		// Add a note entry to traced output.
		void note(const char *note);
		// </group>

		// Normally GLPixelCanvas repaints the window when it is damaged. If
		// the user wishes to handle this, turn off auto refreshing.
		// <group>
		Boolean autoRefresh()const {
			return autoRefresh_;
		}
		void autoRefresh(Boolean refresh) {
			autoRefresh_ = refresh;
		}
		// </group>

		// OpenGL's push/popAttrib functions
		void pushAttrib(const GLbitfield attrib);
		void popAttrib();
		Boolean doubleBuffered()const {
			return doubleBuffered_;
		}
		Boolean isRGB()const {
			return isRGB_;
		}
	protected:

	private:
		// Handle the X expose event.  This is caught by the pixel canvas
		// and forwarded as a refresh event only if the pixel canvas changed
		// dimensions.  If there was no size change, the pixel canvas simply
		// copies its pixmap to the display without generating a refresh.
		void exposeHandler();

		// Called when window is resized.
		void resizeHandler();
		// Event Handling
		// <group>
		static void handleEventsCB(Widget w, GLPixelCanvas * xpc,
		                           XEvent * ev, Boolean *);
		void handleEvents(Widget w, XEvent * ev);
		Boolean handleResizeEvent(casacore::uInt w, casacore::uInt h);
		// handle colorTable resize
		static void colorTableResizeCB(PixelCanvasColorTable * pcctbl, casacore::uInt,
		                               GLPixelCanvas * xpc,
		                               Display::RefreshReason reason);

		// </group>

	private:
		void initComponents();

		// Internal handling of display lists.
		// There is a list of GLPCDisplayLists. Item 0 always exists and is used
		// to redraw the window after expose events. Any other entries are
		// created in response to newList() calls.
		// <group>
		// Return a pointer to a new GLPC display list.
		GLPCDisplayList *createList(const char *name=NULL);
		// Create a new display list and add to the list of lists.
		casacore::uInt makeList(const char *name=NULL);
		// Grow the list.
		void resizeList();
		// Go through the list of display lists removing any unused lists.
		void cleanLists();
		// Called when the window is resized or cleared. Deletes the repaint
		// list.
		void purgeLists();
		// Remove a display list from list of lists. Returns true if list
		// was removed. false if listid is invalid or list has already been removed.
		Boolean removeList(casacore::uInt listid);
		// Called to begin caching.
		void beginListEntry(const char *name=NULL);
		void beginListEntry(GLPCDisplayListElement *e);
		// Called to end caching.
		void endListEntry();
		// </group>

		// Print <name> if tracing is turned on.
		// Most user callable drawing routines call this.
		// <group>
		void traceCheck(const char *name, casacore::uLong traceLevel, casacore::uInt nspaces);
		void traceCheck(const char *name, casacore::uLong traceLevel) {
			traceCheck(name, traceLevel, nspaces_);
		}
		// </group>
		// Holds both a color index and RGBA values.
		class GLPCColorValue {
		public:
			GLPCColorValue() {
				index=0;
				red=green=blue=0.0;
				alpha=1.0;
			}
			casacore::uLong index;	// Index for PseudoColor, packed for RGB.
			casacore::Float red, green, blue, alpha;
		};
		// Store the new value in the struct.
		//  If RGB mode, index is interpreted as a packed RGB value.
		void storeColorValue(GLPCColorValue &v, casacore::uLong value);
		//  If RGB mode, index is interpreted as an index into the virtual CT.
		void storeColorIndex(GLPCColorValue &v, casacore::uInt index);
		void storeColorValue(GLPCColorValue &v, casacore::Float r, casacore::Float g, casacore::Float b);
		void storeColorValue(GLPCColorValue &v,
		                     casacore::Float r, casacore::Float g, casacore::Float b, casacore::Float a);
		// index is always treated as an index. (In RGB mode, the virtual
		// color table is used.
		void setCurrentColorIndex(casacore::uInt index, const char *name=NULL);
		void setCurrentColorValue(casacore::uLong value, const char *name=NULL);
		void setCurrentColorValue(casacore::Float r, casacore::Float g, casacore::Float b,
		                          const char *name=NULL);
		void setCurrentColorValue(casacore::Float r, casacore::Float g, casacore::Float b, casacore::Float a,
		                          const char *name=NULL);
		void setCurrentColorValue(const GLPCColorValue &v, const char *name=NULL);

		void setClearColorIndex(casacore::uInt index, const char *name=NULL);
		void setClearColorValue(casacore::uLong index, const char *name=NULL);
		void setClearColorValue(casacore::Float r, casacore::Float g, casacore::Float b,
		                        const char *name=NULL);
		void setClearColorValue(casacore::Float r, casacore::Float g, casacore::Float b, casacore::Float a,
		                        const char *name=NULL);
		// Tell OpenGL to use the given color index or RGB value depending
		// on whether we're in Index or RGB mode. If name is not NULL,
		// wrap the call in the usual begin/endListEntry, tracecheck.
		void setClearColorValue(const GLPCColorValue &v, const char *name=NULL);

		// Internal versions of public functions
		void drawColoredPoints_(const char *Name, const casacore::Vector<casacore::Int> &x1,
		                        const casacore::Vector<casacore::Int> &y1,
		                        const casacore::Vector<casacore::uInt> &colors);
		void drawColoredPoints_(const char *Name, const casacore::Vector<casacore::Float> &x1,
		                        const casacore::Vector<casacore::Float> &y1,
		                        const casacore::Vector<casacore::uInt> &colors);
		void drawColoredPoints_(const char *Name, const casacore::Vector<casacore::Double> &x1,
		                        const casacore::Vector<casacore::Double> &y1,
		                        const casacore::Vector<casacore::uInt> &colors);
		void pRWbuf(const char *str=NULL, int nspaces=0);
		// Routines to print warnings.
		void warn(const char *routine, const char *msg,
		          const SourceLocation *where)const;
		void unimplemented(const char *routine, const SourceLocation *where)const;

	private:
		::XDisplay	*display_;
		Widget	parent_;
		Widget	form_;
		Widget	drawArea_;
		XVisualInfo	*visualInfo_;
		Visual	*visual_;
		casacore::uInt		BlackPixel_;
		//  GL information.
		casacore::Bool		doubleBuffered_;
		casacore::Bool		isRGB_;			// If not, it's indexed.
		//
		GLPixelCanvasColorTable *glpcctbl_;
		GLXContext	context_;
		GLbitfield	bufferMask_;
		casacore::uInt		width_;
		casacore::uInt		height_;
		casacore::uInt		depth_;
		Boolean	doClipping_;
		casacore::Int		clipX1_, clipY1_;
		casacore::Int		clipX2_, clipY2_;
		casacore::String	itsDeviceForegroundColor_, itsDeviceBackgroundColor_;
		/// Current color.
		GLPCColorValue currentColor_;
		// Clear color.
		GLPCColorValue clearColor_;

		Display::LineStyle	lineStyle_;  // Store style types
		Display::FillStyle	fillStyle_;  // for 'get's.
		/////
		Boolean	exposeHandlerFirstTime_;
		// Component / multi-channel buffers etc.
		casacore::uShort	*itsComponents_;		// Length = 3*width*height.
		casacore::Float		itsComponentX_, itsComponentY_, itsComponentZ_;
		casacore::Float		itsComponentScaleX_, itsComponentScaleY_;
		casacore::uLong		itsComponentWidth_, itsComponentHeight_;
		// Display lists.
		casacore::uInt		listSize_;
		casacore::uInt		numinuse_;
		casacore::uInt		nextfree_;
		casacore::uInt		sizeincr_;
		casacore::uInt		currentListID_;			// Current display list id.
		GLPCDisplayList **dlists_;			// casacore::List of display lists.
		GLPCDisplayListElement *currentElement_;	// Current caching element.

		Boolean	trace_;		// Print tracing info?
		casacore::uLong		traceLevel_;
		casacore::uInt		nspaces_;	// # of spaces to indent trace printout.
		mutable GLLogIO log_;
		Boolean	autoRefresh_;
	};


} //# NAMESPACE CASA - END

#endif
