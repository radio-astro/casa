//# QtPixelCanvas.qo.h: (barebones) Qt implementation of PixelCanvas
//# Copyright (C) 2005
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
#ifndef QTPIXELCANVAS_H
#define QTPIXELCANVAS_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/PixelCanvas.h>
//#include <display/DisplayEvents/PCMotionEH.h>
#include <casa/BasicMath/Math.h>
#include <display/QtViewer/QtPCColorTable.h>
//#include "PCEventDispatcher.h"
#include <list>

#include <graphics/X11/X_enter.h>
#  include <QWidget>
#  include <QPainter>
#  include <QPixmap>
#  include <QPicture>
#  include <QImage>
#  include <QPen>
#include <graphics/X11/X_exit.h>


namespace casa {

	class q_draw;

	class QtPixelCanvas : public QWidget, public PixelCanvas {

//# (Note: derivation from QWidget must be first above, for Qt's
//# 'meta-object compiler' to correctly process the following macro).

		Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

	public:

//# RELEVANT (OR ALREADY-IMPLEMENTED) METHODS


		QtPixelCanvas(QWidget *parent=0);
		~QtPixelCanvas();

		void beginLabelAndAxisCaching( ) {
			cache_label_and_axis = True;
		}
		void endLabelAndAxisCaching( QPainter &qp );

	protected:

		std::list<q_draw*> label_and_axis_cache;

		// QT EVENT HANDLERS -- these create PixelCanvas events.
		// <group>
		void paintEvent(QPaintEvent* e);
		void resizeEvent(QResizeEvent* e);
		void mouseMoveEvent(QMouseEvent* e);
		void mousePressEvent(QMouseEvent* e);
		void mouseReleaseEvent(QMouseEvent* e);
		void keyPressEvent(QKeyEvent* e );
		void enterEvent( QEvent* e );
		// </group>

		Bool cache_label_and_axis;

	public:

		//# OTHER QWIDGET DERIVED METHODS.

		// Returns a reasonable suggestion for canvas size; it will
		// be used initially unless overridden by a resize/setGeometry
		// call, on this widget or a parent.
		virtual QSize sizeHint() const {
			return QSize(400, 300);
		}




		//#     PIXELCANVAS METHODS


		//#     Refresh Cycle Methods -- not to be called by
		//#     refresh / drawing handlers (rather the reverse).
		//#     Not cacheable.


		virtual void refresh(const Display::RefreshReason &reason =
		                         Display::UserCommand,
		                     const Bool &explicitrequest = True);

		static void colorTableResizeCB(PixelCanvasColorTable *, uInt,
		                               QtPixelCanvas * qtpc,
		                               Display::RefreshReason reason);


		// set the draw buffer
		// Only 'FrontBuffer' and 'BackBuffer' are supported; This value
		// will be 'Display::BackBuffer', unless explicitly set to
		// 'Display::FrontBuffer'.
		virtual void setDrawBuffer(Display::DrawBuffer buf) {
			if(buf==Display::FrontBuffer) setDrawBuffer_(buf);
			else setDrawBuffer_(Display::BackBuffer);
			p_begin_();
		}	// Refocus painter on new buffer if necessary.


// buffer memory exchanges
		// <group>
		virtual void copyBackBufferToFrontBuffer();
		virtual void copyFrontBufferToBackBuffer();
		virtual void swapBuffers();
		// </group>

		// partial buffer memory exchanges.  (x1,y1 are blc, x2,y2 are trc)
		// <group>
		virtual void copyBackBufferToFrontBuffer(Int x1, Int y1, Int x2, Int y2);
		virtual void copyFrontBufferToBackBuffer(Int x1, Int y1, Int x2, Int y2);
		virtual void swapBuffers(Int x1, Int y1, Int x2, Int y2);
		// </group>




		//#    Drawing Methods.  (Buffer and draw mode should already be set.
		//#    QPainter p_ should (and will) be open on current paint device).
		//#    cacheable.


		// Clear the window using the background color
		// <group>
		virtual void clear();
		virtual void clear(Int x1, Int y1, Int x2, Int y2);
		// </group>

		// Draw an array of 2D color data as a raster image
		//#dk note:   Color data Arrays passed to draw*()
		//#           are produced by maptoColor[3] (see below).
		virtual void drawImage(const Matrix<uInt> &data, Int x, Int y);

		// The version used with masks.
		//# (the one with the gratuitously transposed arguments...)
		// Set opaqueMask to True to draw masked pixels in the background color;
		// otherwise they will be transparent, letting whatever was drawn
		// previously at that point show through.
		virtual void drawImage(const Int &x, const Int &y,
		                       const Matrix<uInt> &data,
		                       const Matrix<Bool> &mask,
		                       Bool opaqueMask=False) ;


		virtual void drawLines(const Matrix<Float> &verts);

		virtual void drawRectangle(Int x1, Int y1,  Int x2, Int y2);

		virtual void drawFilledRectangle(Int x1, Int y1,  Int x2, Int y2);

		virtual void drawPolygon(const Vector<Float> &x,
		                         const Vector<Float> &y);

		virtual void drawFilledPolygon(const Vector<Float> &x,
		                               const Vector<Float> &y);


//# (This routine was cut in favor of the PixelCanvas one, mainly
//# because Qt's method of implementing stretch (transformation QMatrix)
//# affects drawing line widths as well, which is not what was wanted
//# in the case of beam ellipses.  dk  6/07).
//#
//#   // Draw a single ellipse using the current pen (ie. color,
//#   // thickness, style).  The x and y location must be given, along
//#   // with the semi-major and -minor axis lengths, and the position
//#   // angle measured in degrees positive from the x axis in a
//#   // counter-clockwise direction. If outline is False, the
//#   // ellipse is solid filled, else it is just outlined.
//#   // xstretch, ystretch should be left defaulted to 1 in most cases;
//#   // see usage example in WorldCanvas::drawBeamEllipse(), where they are
//#   // used to stretch a beam ellipse when the display is also linearly
//#   // stretched away from the aspect ratio natural to the sky projection.
//#   // They multiply the relative x and y screen coordinates of ellipse
//#   // points, _before_ they are added to cx and cy.  xstretch, ystretch
//#   // can be negative (though smajor, sminor normally would not be).
//#   virtual void drawEllipse(const Float &cx, const Float &cy,
//# 			   const Float &smajor, const Float &sminor,
//# 			   const Float &pangle, Bool outline = True,
//# 			   Float xstretch = 1.0, Float ystretch = 1.0);






		//# (...That's it?... all that's really used??   (!!))


		//#dk Note: mapToColor[3] are implemented in the base class; they just
		//#   pass the chore to [Qt]PCCT.  The ones actually used are:
		//#
		//#  virtual void mapToColor(const Colormap * map,
		//#			  Array<uInt> & outArray,
		//#			  const Array<uInt> & inArray,
		//#			  Bool rangeCheck = True) const;
		//#
		//#
		//#   [and, when/if non-Index (3-channel)  ColorModels
		//#   are implemented for QtPixelCanvas]:
		//#
		//#  virtual void mapToColor3(Array<uLong> & out,
		//#			   const Array<uInt> & chan1in,
		//#			   const Array<uInt> & chan2in,
		//#			   const Array<uInt> & chan3in) {  }
		//#
		//#   The color data output matrix is passed to drawImage, above






		//#    Graphics Context Methods


		// Get/set the current foreground/background colors.  These colors
		// should be used when the special Strings "foreground" and "background"
		// are given for a color.
		// <group>
		virtual void setDeviceForegroundColor(const String colorname);
		virtual String deviceForegroundColor() const;
		virtual void setDeviceBackgroundColor(const String colorname);
		virtual String deviceBackgroundColor() const;
		// </group>

		virtual void setColor(const String &colorname);
		virtual void setRGBColor(float r, float g, float b);

		// Set/enable/disable clip window.  'ClipWindow' is the only
		// Display::Option supported at present.  Clip window must be
		// enabled as well as set, to operate.
		// <group>
		virtual void setClipWindow(Int x1, Int y1, Int x2, Int y2);
		virtual Bool enable(Display::Option option);
		virtual Bool disable(Display::Option option);
		// </group>



		//#    Informational Methods


		// Return the width of the PixelCanvas in pixels
		virtual uInt width() const {
			return uInt(QWidget::width());
		}

		// Return the height of the PixelCanvas in pixels
		virtual uInt height() const {
			return uInt(QWidget::height());
		}

		// Return the depth of the PixelCanvas in bits
		virtual uInt depth() const {
			return itspcctbl->depth();
		}

		// Return the pixel density (in dots per inch [dpi]) of the PixelCanvas
		virtual void pixelDensity(Float &xdpi, Float &ydpi) const;


		// Get color components in range 0 to 1 without actually
		// allocating the color.  This is needed to set up other
		// devices, for example PgPlot.
		virtual Bool getColorComponents(const String &colorname,
		                                Float &r,
		                                Float &g, Float &b);

		// return the PixelCanvasColorTable
		virtual PixelCanvasColorTable * pcctbl() const {
			return itspcctbl;
		};



		// drawlist caching
		//# (uses QPictures)

		// <group>
		virtual Bool supportsLists();
		virtual uInt newList();
		virtual void endList();
		virtual void drawList(uInt list);
		virtual void deleteList(uInt list);
		virtual void deleteLists();
		virtual Bool validList(uInt list);
		// </group>





		//# METHODS OF QTPIXELCANVAS AND DERIVATIONS ONLY -- NOT GENERIC PIXELCANVAS

		// Return a QPixmap* with a copy of current widget contents.
		// Caller is responsible for deleting.
		virtual QPixmap* contents() {
			return new QPixmap(*frontBuffer_);
		}
		virtual QPixmap* getBackBuffer() {
			return backBuffer_;
		}


		/*//#dk
		//# Hold and release of refresh.  Counted; releaseAll() to release them all.
		//# Note: NOT REALLY IMPLEMENTED yet (holdcount_ test disabled) -- check
		//# interaction with (new) allowBackToFront_ first.
		// <group>
		virtual void hold() { holdcount_++;  }

		virtual void release() {
		  holdcount_ = (holdcount_>0)? holdcount_-1 : 0;
		  if(holdcount_==0 && needsRefresh_) {
		    refresh();
		    needsRefresh_ = False;  }

		virtual void releaseAll() { holdcount_=0; release();  }
		// </group>
		//*/



		// Hold / release of display widget/pixmap painting (used during
		// printing and graphics file saving.)  Be sure to return to unblocked
		// state when done.
		// <group>
		virtual void setAllowBackToFront(Bool allowed=True) {
			allowBackToFront_ = allowed;
		}

		virtual void setUpdateAllowed(Bool allowed=True) {
			if(allowed) {
				if(saveBuf_!=0) {
					delete saveBuf_;
					saveBuf_ = 0;
				}
			} else if(saveBuf_==0) saveBuf_ = contents();
		}
		// </group>



	signals:

		// Emitted in resizeEvent().
		void resizing(QResizeEvent* ev);



	private:

		// Versions of p_.begin(currentPaintDevice_()) and p_.end() ,that just
		// put p_ in desired state, without complaining if it was already there.
		// (In general, p_ will be active (on the current PaintDevice) between
		// calls to QtPixelCanvas methods...).
		// (A certain amount of painter 'state' ('graphics context') is
		// also preserved over p_end_ -- p_begin_ -- still determining this).
		// <group>
		void p_end_();
		void p_begin_();
		// </group>


		// PixelCanvas coordinate (0,0) is the bottom-left corner;
		// in Qt it is the top-left.  This translates the y coordinate.
		// (converts either way).
		Int    q_(Int pY)    {
			return QWidget::height()-1 - pY;
		}
		Float  q_(Float pY)  {
			return QWidget::height()-1 - pY;
		}
		Double q_(Double pY) {
			return QWidget::height()-1 - pY;
		}

		// Translates PC rectangle pixel coords x1,y1,x2,y2 to Qt coords qx,qy,qw,qh
		void qFromPC_(Int  x1, Int  y1,   Int x2,  Int y2,
		              Int& qx, Int& qy,   Int& qw, Int& qh) {
			qw = abs(x2-x1)+1;
			qh = abs(y2-y1)+1;
			qx = min(x1,x2);
			qy = q_(max(y1,y2));
		}

		// returns QRect from PC rectangle (corner) pixel coordinates.
		QRect qrect_(Int x1, Int y1, Int x2, Int y2) {
			Int qx,qy,qw,qh;
			qFromPC_(x1,y1,x2,y2, qx,qy,qw,qh);
			return QRect(qx,qy,qw,qh);
		}


		// translate color component in the range [0., 1.]
		// to clrByte, with 0 <= clrByte < 256
		Int clrByte_(Float f) {
			return Int(min( max(0.f,f)*256.f, 255.f ));
		}

		Bool inDrawMode_() {
			return (drawMode()!=Display::Compile);
		}

		QPixmap* currentBuffer_() {
			return (drawBuffer() == Display::FrontBuffer)?
			       frontBuffer_ : backBuffer_;
		}

		void clearBuffer_() {
			p_.eraseRect(0,0, currentBuffer_()->width(),
			             currentBuffer_()->height());
		}

		QPaintDevice* currentPaintDevice_() {
			if(inDrawMode_()) return currentBuffer_();
			else              return drawList_;
		}



		// Translate Qt KeyboardModifiers and MouseButtons to
		// mask of Display Library KeyModifiers.
		// <group>
		uInt dlKeyMods_(Qt::KeyboardModifiers);
		uInt dlKeyMods_(QMouseEvent* e);
// </group>




		//#     Data / State


		QPixmap* frontBuffer_;	// The actual buffers.  (Qt4's elimination
		QPixmap* backBuffer_;		// of resize() requires these to replaced
		// often on the heap...).


		QPainter p_;		// This Painter is used for the drawing commands, and
		// paints either to the Buffer Pixmaps ('Draw' mode)
		// or to a drawlist QPicture ('Compile' DrawMode).
		// It will remain open (painting 'begun', active) on
		// the currentPaintDevice_() between PC method calls.

		QPainter pw_;		// This painter is used only to copy the front buffer
		// to the widget during paintEvents.


		QPicture* drawList_;		// Will point to drawlist currently
		// under construction, in Compile mode.
		uInt drawListNo_;		// Number of drawlist currently being compiled.

		SimpleOrderedMap<uInt, void*> drawlists_;
		// cache of all current draw lists.



		String itsDeviceForegroundColor, itsDeviceBackgroundColor;

		//# 'painter state' or 'graphics context'

		QPen itsPen;

		QFont itsFont;

		QRect clipRect_;
		Bool clipEnabled_;

		QtPCColorTable* itspcctbl;



		//# Hold/release of refresh.  (Note: not yet used)
		//<group>
		Int holdcount_;
		Bool needsRefresh_;
		//</group>


		// other hold / release of widget/pixmap painting (used during
		// printing / graphics file saving.)
		//<group>
		bool allowBackToFront_;
		QPixmap* saveBuf_;
		//# If non-zero, saveBuf_ holds a 'frozen' state for painting the
		//# PC's widget when the rendering on frontBuffer_ is being used
		//# for another purpose (notably, when a .jpg, etc. of the PC is
		//# being saved at a  larger scale.  (It isn't effective for smaller
		//# scales; probably wasn't worth the effort, but it's there now....))
		//<group>




//############################################################################
//############################################################################
//############################################################################
//############################################################################
//################   End of the Useful Stuff   ###############################
//############################################################################
//############################################################################
//############################################################################
//############################################################################





//#       POSSIBLY USEFUL TO IMPLEMENT IN THE FUTURE


	public:



//# (for 3-channel drawing)

		// return the colorModel
		Display::ColorModel colorModel() const {
			return Display::Index;
		}

		// (Cacheable) Draw a component of a multi-channel image, storing it
		// in buffers until flushComponentImages() is called.
		virtual void drawImage(const Matrix<uInt> &/*data*/, const Int &/*x*/, const Int &/*y*/,
		                       const Display::ColorComponent &/*colorcomponent*/) {  }




		// Set the input color model for multichannel color
		void setColorModel(Display::ColorModel colorModel);

		// Fill one of the channel buffers.
		virtual void bufferComponent(const Matrix<uInt> &/*data*/,
		                             const Int &/*x*/,
		                             const Int &/*y*/,
		                             const Display::ColorComponent &/*colorcomponent*/)
		{  }

		// (NOT CACHEABLE!) Flush the component buffers.
		virtual void flushComponentBuffers() {  }






//#        RELEVANCE TO BE DETERMINED



		// return True if the refresh is active.
		//#dk PC probably can't determine this usefully, since individual
		//#   WCs may initiate their own refresh/drawing cycles....
		Bool refreshActive() const;

		// return True if refresh is allowed right now...
		virtual Bool refreshAllowed() const;

		// Draw a set of points, specifying a color per point to
		// be drawn.
		// <group>
		virtual void drawColoredPoints(const Vector<Int> &/*x1*/, const Vector<Int> &/*y1*/,
		                               const Vector<uInt> &/*colors*/) {
			// cout << "WYSISWYN " << __FILE__ << " " << __LINE__ << endl;
		}
		virtual void drawColoredPoints(const Matrix<Int> &xy,
		                               const Vector<uInt> &colors) {
			drawColoredPoints(xy.column(0), xy.column(1), colors);
		}
		// </group>

		// Cause display to flush any graphics commands not yet drawn
		virtual void flush();

		static void drawText(QPainter& painter, const QPoint& p, const QFont& font,
		                     const QPen& pen, const String& text, double angle,
		                     Display::TextAlign alignment);
		virtual void drawText(Int x, Int y, const String &text, const Float& angle,
		                      Display::TextAlign alignment = Display::AlignCenter);
		virtual void drawText(Int x, Int y, const String &text,
		                      Display::TextAlign alignment = Display::AlignCenter);

		virtual Int textWidth(const String& text);

		virtual Int textHeight(const String& text);

		virtual void setHSVColor(float /*h*/, float /*s*/, float /*v*/) {  }


		// (Cacheable) Draw a single point using current color
		// <group>
		virtual void drawPoint(Int x1, Int y1);
		virtual void drawPoint(Float x1, Float y1);
		virtual void drawPoint(Double x1, Double y1);
		// </group>

		// (Cacheable) Draw N points specified as a Nx2 matrix
		// <group>
		virtual void drawPoints(const Matrix<Int> &verts);
		virtual void drawPoints(const Matrix<Float> &verts);
		virtual void drawPoints(const Matrix<Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of points using current color
		// <group>
		virtual void drawPoints(const Vector<Int> &x1,
		                        const Vector<Int> &y1);
		virtual void drawPoints(const Vector<Float> &x1,
		                        const Vector<Float> &y1);
		virtual void drawPoints(const Vector<Double> &x1,
		                        const Vector<Double> &y1);
		// </group>

		// (Cacheable) Draw a single line using current color
		// <group>
		virtual void drawLine(Int x1, Int y1,
		                      Int x2, Int y2);
		virtual void drawLine(Float x1, Float y1,
		                      Float x2, Float y2);
		virtual void drawLine(Double x1, Double y1,
		                      Double x2, Double y2);
		// </group>

		// (Cacheable) Draw N/2 lines from an Nx2 matrix
		// <group>
		virtual void drawLines(const Matrix<Int> &verts);
		virtual void drawLines(const Matrix<Double> &verts);
		// </group>

		// (Cacheable) Draw a bunch of unrelated lines using current color
		// <group>
		virtual void drawLines(const Vector<Int> &x1,
		                       const Vector<Int> &y1,
		                       const Vector<Int> &x2,
		                       const Vector<Int> &y2);
		virtual void drawLines(const Vector<Float> &x1,
		                       const Vector<Float> &y1,
		                       const Vector<Float> &x2,
		                       const Vector<Float> &y2);
		virtual void drawLines(const Vector<Double> &x1,
		                       const Vector<Double> &y1,
		                       const Vector<Double> &x2,
		                       const Vector<Double> &y2);
		// </group>

		// (Cacheable) Draw a single connected line between the points given
		// <group>
		virtual void drawPolyline(const Vector<Int> &x1,
		                          const Vector<Int> &y1);
		virtual void drawPolyline(const Vector<Float> &x1,
		                          const Vector<Float> &y1);
		virtual void drawPolyline(const Vector<Double> &x1,
		                          const Vector<Double> &y1);
		// </group>

		// (Cacheable) Draw N-1 connected lines from Nx2 matrix of vertices
		// <group>
		virtual void drawPolyline(const Matrix<Int> &verts);
		virtual void drawPolyline(const Matrix<Float> &verts);
		virtual void drawPolyline(const Matrix<Double> &verts);
		// </group>

		// Let PixelCanvas::drawMarker take care of this.
		/*
		// Draw a "marker". See <linkto class="Display">Display</linkto>
		// for a list of available markers.
		// <group>

		 virtual void drawMarker(const Int& x1, const Int& y1,
					  const Display::Marker& marker,
				  const Int& pixelHeight);
		virtual void drawMarker(const Float& x1, const Float& y1,
					  const Display::Marker& marker,
					  const Int& pixelHeight);
		virtual void drawMarker(const Double& x1, const Double& y1,
					  const Display::Marker& marker,
					  const Int& pixelHeight);

		// </group>
		*/

		// (Cacheable) Draw a closed polygon
		// <group>
		virtual void drawPolygon(const Vector<Int> &x1,
		                         const Vector<Int> &y1);
		virtual void drawPolygon(const Vector<Double> &x1,
		                         const Vector<Double> &y1);
		// </group>

		// (Cacheable) Draw a closed N-sided polygon from Nx2 matrix of vertices
		// <group>
		virtual void drawPolygon(const Matrix<Int> &verts);
		virtual void drawPolygon(const Matrix<Float> &verts);
		virtual void drawPolygon(const Matrix<Double> &verts);
		// </group>

		// (Cacheable) Draw and fill a closed polygon
		// <group>
		virtual void drawFilledPolygon(const Vector<Int> &x1,
		                               const Vector<Int> &y1);
		virtual void drawFilledPolygon(const Vector<Double> &x1,
		                               const Vector<Double> &y1);
		// </group>

		// (Cacheable) Draw a rectangle
		// <group>
		virtual void drawRectangle(Float x1, Float y1,
		                           Float x2, Float y2);
		virtual void drawRectangle(Double x1, Double y1,
		                           Double x2, Double y2);
		// </group>

		// (Cacheable) Draw a filled rectangle
		// <group>
		virtual void drawFilledRectangle(Float x1, Float y1,
		                                 Float x2, Float y2);
		virtual void drawFilledRectangle(Double x1, Double y1,
		                                 Double x2, Double y2);
		// </group>


		// (Cacheable) Draw a set of lines, specifying a color per line to be drawn.
		// <group>
		virtual void drawColoredLines(const Vector<Int> &x1,
		                              const Vector<Int> &y1,
		                              const Vector<Int> &x2,
		                              const Vector<Int> &y2,
		                              const Vector<uInt> &colors);
		virtual void drawColoredLines(const Vector<Float> &x1,
		                              const Vector<Float> &y1,
		                              const Vector<Float> &x2,
		                              const Vector<Float> &y2,
		                              const Vector<uInt> &colors);
		virtual void drawColoredLines(const Vector<Double> &x1,
		                              const Vector<Double> &y1,
		                              const Vector<Double> &x2,
		                              const Vector<Double> &y2,
		                              const Vector<uInt> &colors);
		// </group>

		// vector primitive buffering

		// Set Graphics Attributes
		// Options for functions with enum argument
		// listed in <linkto class=Display>DisplayEnums</linkto>
		// <group>
		virtual void setDrawFunction(Display::DrawFunction function);
		virtual void setForeground(uLong color);
		virtual void setBackground(uLong color);
		virtual void setLineWidth(Float width);
		virtual void setLineStyle(Display::LineStyle style);
		virtual void setCapStyle(Display::CapStyle style);
		virtual void setJoinStyle(Display::JoinStyle style);
		virtual void setFillStyle(Display::FillStyle style);
		virtual void setFillRule(Display::FillRule rule);
		virtual void setArcMode(Display::ArcMode mode);
		// </group>

		// These method is NOT in PixelCanvas.
		virtual void setQtLineStyle(Qt::PenStyle style);
		virtual void setQtPenColor( QColor c ) {
			itsPen.setColor(c);
		}
		virtual QColor getQtPenColor( ) const {
			return itsPen.color( );
		}

		// Get Graphics Attributes
		// <group>
		virtual Display::DrawFunction getDrawFunction() const;
		virtual uLong                 getForeground()   const;
		virtual uLong                 getBackground()   const;
		virtual Float                 getLineWidth()    const;
		virtual Display::LineStyle    getLineStyle()    const;
		virtual Display::CapStyle     getCapStyle()     const;
		virtual Display::JoinStyle    getJoinStyle()    const;
		virtual Display::FillStyle    getFillStyle()    const;
		virtual Display::FillRule     getFillRule()     const;
		virtual Display::ArcMode      getArcMode()      const;
		// </group>


		// Control the image-caching strategy
		virtual void setImageCacheStrategy(Display::ImageCacheStrategy strategy);
		virtual Display::ImageCacheStrategy imageCacheStrategy() const;

		// (Cacheable) Set the color to use for clearing the display
		// <group>

		//#dk probably unneeded -- can clear using deviceBkgd(?)
		//#  (what _else_ is deviceBkgd for?...)
		virtual void setClearColor(uInt colorIndex);
		virtual void setClearColor(const String &colorname);
		virtual void setClearColor(float r, float g, float b);
		// </group>

		// (Not Cacheable) Get the current color to use for clearing the display.
		virtual uInt clearColor() const;
		virtual void getClearColor(float &r, float &g, float &b) const;

		virtual void enableMotionEvents() {  }
		virtual void disableMotionEvents() {  }
		virtual void enablePositionEvents() {  }
		virtual void disablePositionEvents() {  }

		virtual void translateAllLists(Int /*xt*/, Int /*yt*/) {  }
		virtual void translateList(uInt /*list*/, Int /*xt*/, Int /*yt*/) {  }

		virtual Bool setFont(const String &fontName);
		virtual Bool setFont(DLFont* /*font*/) {
			return False;
		}
		virtual Bool setFont(const String& fontName, const Int fontSize);

		// These setFont methods are NOT in PixelCanvas.
		// <group>
		virtual Bool setFont(const String& fontName, bool bold, bool italic);
		virtual Bool setFont(const String& fontName, const Int fontSize, bool bold,
		                     bool italic);
		// </group>

		virtual void drawImage(const Matrix<Int> &/*data*/, Int /*x*/, Int /*y*/) {  }
		virtual void drawImage(const Matrix<uLong> &/*data*/, Int /*x*/, Int /*y*/) {  }
		virtual void drawImage(const Matrix<Float> &data, Int x, Int y);
		virtual void drawImage(const Matrix<Double> &/*data*/, Int /*x*/, Int /*y*/) {  }

		virtual void drawImage(const Matrix<uInt> &/*data*/, Int /*x*/, Int /*y*/,
		                       uInt /*xzoom*/, uInt /*yzoom*/) {  }
		virtual void drawImage(const Matrix<Int> &/*data*/, Int /*x*/, Int /*y*/,
		                       uInt /*xzoom*/, uInt /*yzoom*/) {  }
		virtual void drawImage(const Matrix<uLong> &/*data*/, Int /*x*/, Int /*y*/,
		                       uInt /*xzoom*/, uInt /*yzoom*/) {  }
		virtual void drawImage(const Matrix<Float> &/*data*/, Int /*x*/, Int /*y*/,
		                       uInt /*xzoom*/, uInt /*yzoom*/) {  }
		virtual void drawImage(const Matrix<Double> &/*data*/, Int /*x*/, Int /*y*/,
		                       uInt /*xzoom*/, uInt /*yzoom*/) {  }


		// (Cacheable) Set current color (works in RGB or colormap mode)
		// <group>
		virtual void setColor(uInt colorIndex);
		// </group>

		// (Not Cacheable) Returns the current color as a color index
		virtual uInt color() const;

		// (Not Cacheable) Retuns the current color as an RGB triple
		virtual void getColor(float &r, float &g, float &b) const;

		// (Not Cacheable) Get color index value (works in RGB or colormap mode)
		// <group>
		virtual Bool getColor(Int x, Int y, uInt &color);
		virtual Bool getRGBColor(Int x, Int y,
		                         float &r, float &g, float &b);
		virtual Bool getHSVColor(Int x, Int y,
		                         float &h, float &s, float &v);
		// </group>

		// (Not Cacheable) resize request.  returns true if window was resized.
		// Will refresh if doCallbacks is True.
		//virtual Bool resize(uInt reqXSize, uInt reqYSize,
		//Bool doCallbacks = True);

		// (Not Cacheable) resize the colortable by requesting a new number of cells
		virtual Bool resizeColorTable(uInt newSize);

		// (Not Cacheable) resize the colortable by requesting a new RGB/HSV cube
		virtual Bool resizeColorTable(uInt nReds,
		                              uInt nGreens, uInt nBlues);

		// save/restore the current translation.  This is called pushMatrix because
		// eventually we may want scaling or rotation to play a modest
		// role here.
		// <group>
		virtual void pushMatrix() {  }
		virtual void popMatrix() {  }
		// </group>
		// zero the current translation
		virtual void loadIdentity() {  }

		// translation functions
		// translate applies a relative translation to the current matrix and
		// can be used to position graphics.  Together with pushMatrix and
		// popMatrix it can be used to build heirarchical scenes.
		// <group>
		virtual void translate(Int /*xt*/, Int /*yt*/) {  }
		virtual void getTranslation(Int &/*xt*/, Int &/*yt*/) const {  }
		virtual Int xTranslation() const {
			return 0;
		}
		virtual Int yTranslation() const {
			return 0;
		}
		// </group>




//#         CONFIRMED IRRELEVANT / UNUSED



		virtual void drawColoredEllipses(const Matrix<Float> &/*centres*/,
		                                 const Vector<Float> &/*smajor*/, const Vector<Float> &/*sminor*/,
		                                 const Vector<Float> &/*pangle*/, const Vector<uInt> &/*colors*/,
		                                 const Float &/*scale*/ = 1.0, const Bool &/*outline*/ = True) {  }

		virtual void drawColoredPoints(const Vector<Float> &/*x1*/,
		                               const Vector<Float> &/*y1*/, const Vector<uInt> &/*colors*/) {  }
		virtual void drawColoredPoints(const Vector<Double> &/*x1*/,
		                               const Vector<Double> &/*y1*/, const Vector<uInt> &/*colors*/) {  }
		virtual void drawColoredPoints(const Matrix<Float> &xy,
		                               const Vector<uInt> &colors) {
			drawColoredPoints(xy.column(0), xy.column(1), colors);
		}
		virtual void drawColoredPoints(const Matrix<Double> &xy,
		                               const Vector<uInt> &colors) {
			drawColoredPoints(xy.column(0), xy.column(1), colors);
		}


		virtual void getClipWindow(Int &/*x1*/, Int &/*y1*/, Int &/*x2*/, Int &/*y2*/) {  }

		virtual void setPcctbl(PixelCanvasColorTable * /*pcctbl*/) {  }

	};	// class QtPixelCanvas

}	// namespace casa

#endif
