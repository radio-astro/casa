//# PSPixelCanvas.cc: Subclass of PixelCanvas for PostScript.
//# Copyright (C) 1996,1997,1999,2000,2001,2002,2003
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
#include <casa/stdio.h>
#include <casa/stdlib.h>
#include <strings.h>
#include <display/Display/PSPixelCanvas.h>
#include <display/Display/ColorConversion.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////////////////////////////////////////////
//	Some internal constants

// FILL/NOFILL a polygon, close/don't close a polygon.
	static const int FILL=1, NOFILL=0, CLOSE=1, NOCLOSE=0;

	static const char *DEFAULTFOREGROUNDCOLORNAME = "black";
	static const char *DEFAULTBACKGROUNDCOLORNAME = "white";
	static const char *FOREGROUND = "foreground";
	static const char *BACKGROUND = "background";

////////////////////////////////////////////////////////
// Conversion between points and "pixels". PostScript doesn't deal with
// pixels so they are invented.
// This is arbitrary as long as it's at least as large as that of the best
// printer to be used and doesn't overflow an int for large paper sizes.
// Declared resolution in pixels/dimension. (eg. 1200 dots per inch).
	static const float RESOLUTION = 1200.0;
	static const PSDriver::Dimension PIXELDIMENSION = PSDriver::INCHES;
// (Valid values are INCHES, POINTS and MM).
// Note: 1200*1200*8.5*11 is about 128M pixels.
////////////////////////////////////////////////////////////////


	PSPixelCanvas::PSPixelCanvas(PSDriver *ps, PSPixelCanvasColorTable *pcct) :
		PixelCanvas(pcct) {
		pspcinit(ps, pcct);
	}

	void PSPixelCanvas::pspcinit(PSDriver *psd, PSPixelCanvasColorTable *pcct) {
		annotate(True);
		ps = psd;
		if(ps == NULL)
			throw(AipsError("PSPixelCanvas passed NULL PSDriver."));
		pcct_ = pcct;

		// We allow a different resolution in the X & Y axes, but,
		// currently, there is no way to specify.
		// Stored as Points per pixel;
		xppp_ = yppp_ = 0.0;
		setResolution(RESOLUTION, RESOLUTION, PIXELDIMENSION);
		lineWidth(1.0);
		// Default device color names.
		setDeviceForegroundColor(DEFAULTFOREGROUNDCOLORNAME);
		setDeviceBackgroundColor(DEFAULTBACKGROUNDCOLORNAME);
		// Initialize foreground (current), background and clear colors.
		setColor(DEFAULTFOREGROUNDCOLORNAME);
		int colorindex;
		pcct_->allocColor(DEFAULTBACKGROUNDCOLORNAME, colorindex);
		setBackground((uLong)colorindex);
		setClearColor(deviceBackgroundColor());

		// For now, we're just making something up.
		capStyle_ = Display::CSNotLast;		// ????
		joinStyle_ = Display::JSMiter;
		fillStyle_ = Display::FSSolid;
		fillRule_ = Display::FREvenOdd;
		arcMode_ = Display::AMChord;
		xt_ = yt_ = 0.0;
		clipx0_ = clipy0_ = clipx1_ = clipy1_ = 0;
		haveClipInfo_ = False;
		clipEnabled_ = False;
		smooth_ = False;
		itsComponentWidth = itsComponentHeight = itsComponentBPC = 0;
		itsComponents = 0;
	}

	PSPixelCanvas::~PSPixelCanvas() {
		ps->finish();
	}

	// enabling/disabling of event tracking

	void PSPixelCanvas::enableMotionEvents() {
	}

	void PSPixelCanvas::disableMotionEvents() {
	}

	void PSPixelCanvas::enablePositionEvents() {
	}

	void PSPixelCanvas::disablePositionEvents() {
	}



	// begin caching display commands - return list ID
	uInt PSPixelCanvas::newList() {
		return 0;
	}

	// end caching display commands
	void PSPixelCanvas::endList() {
	}

	// (Cacheable) recall cached display commands
	void PSPixelCanvas::drawList(uInt) {
	}

	// translate all lists
	void PSPixelCanvas::translateAllLists(Int, Int) {
	}

	// translate the list
	void PSPixelCanvas::translateList(uInt, Int, Int) {
	}

	// remove list from cache
	void PSPixelCanvas::deleteList(uInt) {
	}

	// flush all lists from the cache
	void PSPixelCanvas::deleteLists() {
	}

	// return True if the list exists
	Bool PSPixelCanvas::validList(uInt) {
		return False;
	}

// (Cacheable) Set the font to the recognizable font name and size
// Always returns True since we can't know if fontName exists.
	Bool PSPixelCanvas::setFont(const String& fontName, const Int size) {
		ps->findFont(fontName.chars(), (float)size);

		return True;
	}

// (Cacheable) Set the font to the recognizable font name
// Always returns True since we can't know if fontName exists.
	Bool PSPixelCanvas::setFont(const String & fontName) {
		ps->setFont(fontName.chars());
		return True;
	}

	// (Cacheable) Draw text using that font aligned in some way to the
	// position
	void PSPixelCanvas::drawText(Int x, Int y, const String & text,
	                             Display::TextAlign alignment) {
		PSDriver::TextAlign al = (PSDriver::TextAlign)alignment;

		ps->drawText((float)x, (float)y, text.chars(), al);
	}

	void PSPixelCanvas::drawText(Int x, Int y, const String & text,
	                             const Float& angle,
	                             Display::TextAlign alignment) {

		PSDriver::TextAlign al = (PSDriver::TextAlign)alignment;
		ps->drawText((float)x, (float)y, text.chars(), al, (float)angle);
	}

//
// drawImage
//
	void PSPixelCanvas::drawImage(const Matrix<uInt> & data, Int X, Int Y) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;

		if(mult == 1) {	// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		} else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Int> & data, Int X, Int Y) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<uLong> & data, Int X, Int Y) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Float> & data, Int X, Int Y) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Double> & data, Int X, Int Y) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

//
// drawImage with masking.
//
	void PSPixelCanvas::drawImage(const Int &X, const Int &Y,
	                              const Matrix<uInt> &data,
	                              const Matrix<Bool> &mask,
	                              Bool /*opaqueMask*/) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;

		if(mult == 1) {	// Index
			uShort backgroundColor = (uShort)getBackground();
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v;
					if(mask(x, y))
						v = (uShort)data(x, y);
					else
						v = backgroundColor;
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		} else {	// RGB/HSV
			// Assumes, probably incorrectly, background was set with a packed pixel.
			uLong backgroundColor = getBackground();
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v;
					if(mask(x, y))
						v = (uLong)data(x, y);
					else
						v = backgroundColor;

					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, 1.0, 1.0, d, bpc, cm);
		delete [] d;
	}

//
// drawImage (zoom version)
//
	void PSPixelCanvas::drawImage(const Matrix<uInt> & data, Int X, Int Y,
	                              uInt xzoom, uInt yzoom) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, xzoom, yzoom, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Int> & data, Int X, Int Y,
	                              uInt xzoom, uInt yzoom) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, xzoom, yzoom, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<uLong> & data, Int X, Int Y,
	                              uInt xzoom, uInt yzoom) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, xzoom, yzoom, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Float> & data, Int X, Int Y,
	                              uInt xzoom, uInt yzoom) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, xzoom, yzoom, d, bpc, cm);
		delete [] d;
	}

	void PSPixelCanvas::drawImage(const Matrix<Double> & data, Int X, Int Y,
	                              uInt xzoom, uInt yzoom) {
		uInt nx = data.nrow();
		uInt ny = data.ncolumn();
		Display::ColorModel cm = PSpcctbl()->colorModel();
		uInt mult = PSpcctbl()->numComponents();
		uShort *d = new uShort[nx*ny*mult], *dptr;
		int bpc = 8;
		dptr = d;
		if(mult == 1)		// Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
					if(v > 255) bpc = 12;
				}
			}
		else {	// RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					uShort a, b, c;
					// Unpack color information.
					pcct_->pixelToComponents( v, a, b, c);
					if( (a > 255) || (b > 255) || (c > 255))
						bpc = 12;
					*dptr++ = a;
					*dptr++ = b;
					*dptr++ = c;
				}
			}
		}
		drawPSImage(X, Y, nx, ny, xzoom, yzoom, d, bpc, cm);
		delete [] d;
	}

// Copy component to buffer.
	void PSPixelCanvas::drawImage(const Matrix<uInt> &data, const Int &x,
	                              const Int &y,
	                              const Display::ColorComponent &colorcomponent) {
		if(itsComponents == NULL)	{
			itsComponentX = static_cast<Float>(x);
			itsComponentY = static_cast<Float>(y);
			itsComponentWidth = data.nrow();
			itsComponentHeight = data.ncolumn();
			itsComponents = new uShort [itsComponentHeight*itsComponentWidth*3];
			itsComponentBPC = 8;
		}
		uShort *ptr = itsComponents;
		// Offsets: Red=0, Green=1, Blue=2.
		switch(colorcomponent) {
		case Display::Red:
		case Display::Hue:
			break;
		case Display::Green:
		case Display::Saturation:
			ptr += 1;
			break;
		case Display::Blue:
		case Display::Value:
			ptr += 2;
			break;
		}

		for(uInt y = 0; y < itsComponentHeight; y++)
			for(uInt x = 0; x < itsComponentWidth; x++) {
				uShort v = (uShort)data(x,y);
				*ptr = v;
				if(v > 255)
					itsComponentBPC = 12;
				ptr += 3;
			}
	}

	void PSPixelCanvas::bufferComponent(const Matrix<uInt> &/*data*/,
	                                    const Int &/*x*/, const Int &/*y*/,
	                                    const Display::ColorComponent
	                                    &/*colorcomponent*/) {
		cerr << "PSPixelCanvas::bufferComponent(..., colorcomponent) NYI" << endl;
	}

	void PSPixelCanvas::flushComponentBuffers() {
		if( itsComponents != NULL) {
			if(PSpcctbl()->colorModel() == Display::HSV) {
				// do HSV - RGB
				uInt nelements = itsComponentWidth*itsComponentHeight*3;
				uInt maxcolor1, maxcolor2, maxcolor3;
				PSpcctbl()->nColors(maxcolor1, maxcolor2, maxcolor3);
				maxcolor1 -= 1;
				maxcolor2 -= 1;
				maxcolor3 -= 1;
				for(uInt i=0; i < nelements; i += 3) {
					float h, s, v, r, g, b;
					// Convert to floats in range of [0..1].
					h = (float)(itsComponents[i])/maxcolor1;
					s = (float)(itsComponents[i+1])/maxcolor2;
					v = (float)(itsComponents[i+2])/maxcolor3;
					hsvToRgb(h, s, v, r, g, b);
					itsComponents[i] = (uShort)(r*maxcolor1);
					itsComponents[i+1] = (uShort)(g*maxcolor2);
					itsComponents[i+2] = (uShort)(b*maxcolor3);
				}
			}
			drawPSImage(static_cast<int>(itsComponentX), static_cast<int>(itsComponentY),
					itsComponentWidth, itsComponentHeight, 1.0, 1.0,
			        itsComponents, itsComponentBPC, PSpcctbl()->colorModel());
			itsComponentWidth = itsComponentHeight = 0;
			delete [] itsComponents;
			itsComponents = NULL;
		}
	}



//
// drawPSImage
//
	void PSPixelCanvas::drawPSImage(const int x, const int y,
	                                const int iwidth, const int iheight,
	                                const float xzoom, const float yzoom,
	                                const uShort *data, const int bpc,
	                                const Display::ColorModel colormodel) {
		float x0 = (float)x, y0 = (float) y;

		float iwidthf = (float)iwidth;
		float iheightf = (float)iheight;

		// If zoom is < 0, we'll shrink the image.
		float xscl, yscl;
		if(xzoom < 0)
			xscl = -1.0/xzoom;
		else if(xzoom == 0.0)
			xscl = 1.0;
		else
			xscl = xzoom;

		if(yzoom < 0)
			yscl = -1.0/yzoom;
		else if(yzoom == 0.0)
			yscl = 1.0;
		else
			yscl = yzoom;

		float owidth = iwidthf*xscl;		// Output size.
		float oheight = iheightf*yscl;

		PSDriver::ColorSpace cs = PSDriver::INDEXED;

		switch(colormodel) {
		case Display::Index:
			cs = PSDriver::INDEXED;
			break;
		case Display::RGB:
			cs = PSDriver::RGB;
			break;
		case Display::HSV:
			cs = PSDriver::HSV;
			break;
		default:
			cerr << "PSPixelCanvas::drawPSImage: illegal colormodel\n";
			break;
		}

		ps->drawImage(x0, y0, iwidth, iheight, owidth, oheight,
		              data, bpc, cs, smooth_);
	}

// Set/get declared resolution in pixels per supplied dimension.
// Computes approximate scale factors needed to be applied to the
// transformation matrix to map 0..pagesize( in points)
// to 0..(pagesize*res(in points).
#if 0
	void PSPixelCanvas::setResolution(const float xresUC, const float yresUC,
	                                  const PSDriver::Dimension dim) {
		if((xresUC <= 0.0) || (yresUC <= 0.0))
			return;

		// (points/pixel) = (points/dim)/(pixels/dim)
		float xresp = ps->toPoints(1.0, dim)/ xresUC;
		float yresp = ps->toPoints(1.0, dim)/yresUC;
		float widthp, heightp, widthu, heightu;
		float xscale, yscale;

		// This isn't precisely correct, but seems close enough.
		ps->pageSize(widthp, heightp, False);	// pagesize in points.
		ps->pageSize(widthu, heightu, True);	// in user coords.
		xscale = widthu/(xresp*widthp);
		yscale = heightu/(yresp*heightp);
		ps->scale( xscale, yscale);
		xppp_ = xresp;
		yppp_ = yresp;
	}

	void PSPixelCanvas::getResolution(float &xres, float &yres,
	                                  const PSDriver::Dimension dim)const {
		// (pixels/dim) = 1.0 / ((points/pixel) * dim/points))
		xres = 1.0 / ps->fromPoints(xppp_, dim);
		yres = 1.0 / ps->fromPoints(yppp_, dim);
	}
#else
// Adjust transformation matrix so 'resUC' pixels converts into the
// proper # of points.
//		points = T*pixels
	void PSPixelCanvas::setResolution(const float xresUC, const float yresUC,
	                                  const PSDriver::Dimension dim) {
		if((xresUC <= 0.0) || (yresUC <= 0.0))
			return;

		// Convert new resolutions to points.
		// (points/pixel) = (points/dim)/(pixels/dim).
		float ppdim = ps->toPoints(1.0, dim);	// points/<unit dim>.
		float pntPerPixlx = ppdim/xresUC;
		float pntPerPixly = ppdim/yresUC;
		float widthp, heightp, widthu, heightu;
		float cur_pntPerPixlx, cur_pntPerPixly;

		ps->pageSize(widthp, heightp, False);	// pagesize in points.
		ps->pageSize(widthu, heightu, True);	// in user coords.
		// Current scale in points/pixel.
		cur_pntPerPixlx = widthp/widthu;
		cur_pntPerPixly = heightp/heightu;

		// Can't 'unset' current scale, so compute the conversion to go
		// from old to new scale.
		// newscale = currentscale*s; s = newscale/currentscale.
		float scalex = pntPerPixlx/cur_pntPerPixlx;
		float scaley = pntPerPixly/cur_pntPerPixly;
		ps->scale( scalex, scaley);
		xppp_ = pntPerPixlx;
		yppp_ = pntPerPixly;
	}

	void PSPixelCanvas::getResolution(float &xres, float &yres,
	                                  const PSDriver::Dimension dim)const {
		// (pixels/dim) = 1.0 / ((points/pixel) * dim/points))
		xres = 1.0 / ps->fromPoints(xppp_, dim);
		yres = 1.0 / ps->fromPoints(yppp_, dim);
	}
#endif

	// Set Graphics Attributes
	// Options for functions with enum argument
	// listed in <linkto class=Display>DisplayEnums</linkto>

	void PSPixelCanvas::setDrawFunction(Display::DrawFunction) {
	}

// The set color routines are only implemented for INDEX mode.
	void PSPixelCanvas::setForeground(uLong color) {
		foreground_ = color;
		if(annotate())
			note("setForeground: ", color);
//	if (PSpcctbl()->colorModel() == Display::Index)
		ps->setColor((float)foreground_);
	}

	void PSPixelCanvas::setBackground(uLong color) {
		if(annotate())
			note("setBackground: ", color);
		background_ = color;
		ps->setBackgroundColor(background_);
	}

//void PSPixelCanvas::setLineWidth(uInt width)
	void PSPixelCanvas::setLineWidth(Float width) {
		//lineWidth((float)width);
		lineWidth(width);
	}

	void PSPixelCanvas::lineWidth(Float width) {
		lineWidth_ = width;
		// Attempt to compensate for scaling.
		ps->lineWidth(lineWidth_);
	}

	void PSPixelCanvas::setLineStyle(Display::LineStyle style) {
		lineStyle_ = style;
	}

	void PSPixelCanvas::setCapStyle(Display::CapStyle style) {
		capStyle_ = style;
	}

	void PSPixelCanvas::setJoinStyle(Display::JoinStyle style) {
		joinStyle_ = style;
	}

	void PSPixelCanvas::setFillStyle(Display::FillStyle style) {
		fillStyle_ = style;
	}

	void PSPixelCanvas::setFillRule(Display::FillRule rule) {
		fillRule_ = rule;
	}

	void PSPixelCanvas::setArcMode(Display::ArcMode mode) {
		arcMode_ = mode;
	}



	// Get Graphics Attributes

	Display::DrawFunction PSPixelCanvas::getDrawFunction() const {
		return Display::DFCopy;		// The only one supported.
	}

	uLong PSPixelCanvas::getForeground()   const {
		return foreground_;
	}

	uLong PSPixelCanvas::getBackground()   const {
		return background_;
	}

	/*
	uInt PSPixelCanvas::getLineWidth() const
	{
		return (uInt)lineWidth_;
	}
	*/
	Float PSPixelCanvas::getLineWidth() const {
		return lineWidth_;
	}

	Display::LineStyle PSPixelCanvas::getLineStyle() const {
		return lineStyle_;
	}

	Display::CapStyle PSPixelCanvas::getCapStyle()     const {
		return capStyle_;
	}

	Display::JoinStyle PSPixelCanvas::getJoinStyle()    const {
		return joinStyle_;
	}

	Display::FillStyle PSPixelCanvas::getFillStyle() const {
		return fillStyle_;
	}

	Display::FillRule PSPixelCanvas::getFillRule() const {
		return fillRule_;
	}

	Display::ArcMode PSPixelCanvas::getArcMode() const {
		return arcMode_;
	}



	// (Cacheable) Option Control
	// Options listed in <linkto class=Display>DisplayEnums</linkto>

	Bool PSPixelCanvas::enable(Display::Option option) {
		Bool ok;

		switch(option) {
		case Display::ClipWindow:
#if 1
			clipEnabled_ = True;
			ps->clipRect(	clipx0_, clipy0_,
			                clipx1_-clipx0_, clipy1_-clipy0_);
			ok = True;
#else
			ok = False;	// Disabled!!
#endif
			break;
		default:
			ok = False;
		}
		return ok;
	}

	Bool PSPixelCanvas::disable(Display::Option option) {
		Bool ok;

		switch(option) {
		case Display::ClipWindow:
			clipEnabled_ = False;
			// no mechanism for turning it off.
			ok = True;
		default:
			ok = False;
		}
		return ok;
	}



	// Control the image-caching strategy
	void PSPixelCanvas::setImageCacheStrategy(Display::ImageCacheStrategy) {
	}

	Display::ImageCacheStrategy PSPixelCanvas::imageCacheStrategy() const {
		// This doesn't make sense for PostScript, so we'll return nonsense.
		return Display::ServerAlways;
	}

	// (Cacheable) Setup the clip window.  The clip window, when enabled, allows
	// a user to clip all graphics output to a rectangular region on
	// the screen.

	void PSPixelCanvas::setClipWindow(Int x1, Int y1, Int x2, Int y2) {
		clipx0_ = (Float)x1;
		clipy0_ = (Float)y1;
		clipx1_ = (Float)x2;
		clipy1_ = (Float)y2;
		haveClipInfo_ = True;
	}

	void PSPixelCanvas::getClipWindow(Int & x1, Int & y1, Int & x2, Int & y2) {
		x1 = (Int)clipx0_;
		y1 = (Int)clipy0_;
		x2 = (Int)clipx1_;
		y2 = (Int)clipy1_;
	}

	// (Not Cacheable) Redraw the window

	void PSPixelCanvas::refresh(const Display::RefreshReason &reason,
	                            const Bool &explicitrequest) {
		// call upon the client to draw.
		//callRefreshEventHandlers(reason);

		if (explicitrequest && (reason != Display::BackCopiedToFront)) {
			setDrawBuffer(Display::BackBuffer);
		}
		callRefreshEventHandlers(reason);
		if (explicitrequest && (reason != Display::BackCopiedToFront)) {
			copyBackBufferToFrontBuffer();
			setDrawBuffer(Display::FrontBuffer);
			callRefreshEventHandlers(Display::BackCopiedToFront);
		}

	}



	// Cause display to flush any graphics commands not yet drawn
	void PSPixelCanvas::flush() {
		ps->flush();
	}

	// (Cacheable) Clear the window using the current clear color.

	void PSPixelCanvas::clear() {
		if(annotate()) {
			switch (PSpcctbl()->colorModel()) {
			case Display::Index:
				note("clear window using index: ", clearColor_);
				break;
			case Display::RGB:
				note("clear window using RGB ");
				break;
			case Display::HSV:
				note("clear window using HSV: ");
				break;
			}
		}
#if 0
		float x1 = pointsToX(ps->width(PSDriver::POINTS)-1);
		float y1 = pointsToY(ps->height(PSDriver::POINTS)-1);
#else
		float x1, y1;
		ps->pageSize(x1, y1);
#endif
		clearRectangle(0.0, 0.0, x1, y1);
	}

	void PSPixelCanvas::clearRectangle(const float x0, const float y0,
	                                   const float x1, const float y1) {
		ps->gsave();
		ps->setColor((float)clearColor_);
		drawRectangle( x0, y0, x1, y1, FILL);
		ps->grestore();
	}

	void PSPixelCanvas::clear(Int x1, Int y1, Int x2, Int y2) {
		if(annotate()) {
			char buf[128];
			sprintf(buf, "clearRectangle: %d/%d, %d/%d using index %ld",
			        x1, y1, x2, y2, clearColor_);
			ps->comment(buf);
		}

		clearRectangle((float)x1, (float)y1, (float)x2, (float) y2);
	}



	// (Cacheable) Set the color to use for clearing the display

	void PSPixelCanvas::setClearColor(uInt colorIndex) {
		clearColor_ = colorIndex;
	}

	void PSPixelCanvas::setClearColor(const String & colorname) {
		int index;

		if(pcct_->allocColor(colorname, index))
			clearColor_ = (uLong)index;
		else
			throw(AipsError("Attempt to set color to undefined clear color"));
	}

	void PSPixelCanvas::setClearColor(float r, float g, float b) {
		int index;

		if(pcct_->allocColor(r, g, b, index))
			clearColor_ = (uLong)index;
	}



	// (Not Cacheable) Get the current color to use for clearing the display.
	uInt PSPixelCanvas::clearColor() const {
		return (uInt)clearColor_;
	}

	void PSPixelCanvas::getClearColor(float & r, float & g, float & b) const {
		pcct_->queryColor(clearColor_, r, g, b);
	}

	// Return the width of the PSPixelCanvas in pixels
	uInt PSPixelCanvas::width() const {
		float w, height;
		ps->pageSize(w, height, True);	// Page size in user coords.
		return (uInt)w;
	}

	// Return the height of the PSPixelCanvas in pixels
	uInt PSPixelCanvas::height() const {
		float width, h;
		ps->pageSize(width, h);
		return uInt(h);
	}

	// Return the depth of the PSPixelCanvas in bits
	uInt PSPixelCanvas::depth() const {
		return pcct_->depth();
	}

	// get the pixel density
	void PSPixelCanvas::pixelDensity(Float &xdpi, Float &ydpi) const {
		getResolution(xdpi, ydpi);
	}

	// Set current color.

	void PSPixelCanvas::setColor(uInt colorIndex) {
		if(annotate())
			note("setColor: ", colorIndex);
		foreground_ = colorIndex;
		ps->setColor((float)foreground_);
	}

	void PSPixelCanvas::setColor(const String & colorname) {
		int index;
		String precolor = colorname;
		if (precolor == FOREGROUND) {
			precolor = deviceForegroundColor();
		} else if (precolor == BACKGROUND) {
			precolor = deviceBackgroundColor();
		}

		if(pcct_->allocColor(precolor, index)) {
			foreground_ = index;
			if(annotate()) {
				char buf[128];
				sprintf(buf, "setColor (%s) to index %d",
				        precolor.chars(), index);
				ps->comment(buf);
			}
			ps->setColor((float)foreground_);
		} else
			throw(AipsError("Attempt to set color to undefined color"));
	}

	void PSPixelCanvas::setRGBColor(float r, float g, float b) {
		int index;

		if(pcct_->allocColor(r, g, b, index)) {
			foreground_ = (uLong)index;
			ps->setColor((float)foreground_);
		}
	}

// Not right.
	void PSPixelCanvas::setHSVColor(float h, float s, float v) {
		ps->setHSBColor( h, s, v);
	}



	// (Not Cacheable) Returns the current color as a color index
	uInt PSPixelCanvas::color() const {
		return (uInt)foreground_;
	}

// (Not Cacheable) Retuns the current color as an RGB triple
	void PSPixelCanvas::getColor(float & r, float & g, float & b) const {
		pcct_->queryColor(foreground_, r, g, b);
	}

	// (Not Cacheable) Get color index value (works in RGB or colormap mode)
	// Not supported.

	Bool PSPixelCanvas::getColor(Int, Int, uInt & color) {
		color = 0;
		return False;
	}

// Not supported
	Bool PSPixelCanvas::getRGBColor(Int, Int, float & r, float & g, float & b) {
		r = g = b = 0.0;
		return False;
	}

// Not supported
	Bool PSPixelCanvas::getHSVColor(Int, Int, float & h, float & s, float & v) {
		h = s = v = 0.0;
		return False;
	}

	Bool PSPixelCanvas::getColorComponents( const String &colorname,
	                                        Float &r, Float &g, Float &b) {
		// Check for special cases.
		if(colorname == FOREGROUND)
			return pcct_->parseColor(deviceForegroundColor(), r, g, b);
		else if(colorname == BACKGROUND)
			return pcct_->parseColor(deviceBackgroundColor(), r, g, b);
		else
			return pcct_->parseColor(colorname, r, g, b);
	}


	// Get/set the current foreground/background colors.  These colors
	// should be used when the special Strings "foreground" and "background"
	// are given for a color.

	void PSPixelCanvas::setDeviceForegroundColor(const String colorname) {
		defaultForegroundColorName_ = colorname;
	}

	String PSPixelCanvas::deviceForegroundColor() const {
		return defaultForegroundColorName_;
	}

	void PSPixelCanvas::setDeviceBackgroundColor(const String colorname) {
		defaultBackgroundColorName_ = colorname;
	}

	String PSPixelCanvas::deviceBackgroundColor() const {
		return defaultBackgroundColorName_;
	}




	// (Not Cacheable) resize request.  returns true if window was resized.
	// Will refresh if doCallbacks is True.
	Bool PSPixelCanvas::resize(uInt reqXSize, uInt reqYSize, Bool doCallbacks) {
		// Compute new conversion constants in pixels/point.
#if 0
		Float xpp = (float)reqXSize/ps->width(PSDriver::POINTS);
		Float ypp = (float)reqYSize/ps->height(PSDriver::POINTS);
		setResolution(xpp, ypp, PSDriver::POINTS);
#else
		setResolution((Float)reqXSize, (Float)reqYSize);
#endif
		if (doCallbacks) refresh();
		return True;
	}

	// (Not Cacheable) resize the colortable by requesting a new number of cells
	Bool PSPixelCanvas::resizeColorTable(uInt newSize) {
		return pcct_->resize(newSize);
	}

	// (Not Cacheable) resize the colortable by requesting a new RGB/HSV cube
	Bool PSPixelCanvas::resizeColorTable(uInt nReds, uInt nGreens, uInt nBlues) {
		return pcct_->resize(nReds, nGreens, nBlues);
	}

	// Need a mechanism to return the PixelCanvasColorTable so
	// drawing functions within classes can operate.
	PixelCanvasColorTable *PSPixelCanvas::pcctbl() const {
		return pcct_;
	}

	void PSPixelCanvas::setPcctbl(PixelCanvasColorTable * pcctbl) {
		// This may not be a good idea since this & the colortable
		// need to be talking to the same PSDriver.
		// This is bad.
		pcct_ = (PSPixelCanvasColorTable *)pcctbl;
	}



	// save/restore the current translation.  This is called pushMatrix because
	// eventually we may want scaling or rotation to play a modest
	// role here.
	// NOTE: This implemented as leaving a copy of the current transform
	// 	matrix on the stack, then popping it off. Bad things will happen
	//	if anything else is left on the stack when the pop happens.
	//	The save/restore are safer, but they save/restore all the graphics
	//	state.

	void PSPixelCanvas::pushMatrix() {
#define USEPUSHMATRIX
#if defined(USEPUSHMATRIX)
		ps->pushMatrix();
#else
		ps->gsave();
#endif
	}

	void PSPixelCanvas::popMatrix() {
#if defined(USEPUSHMATRIX)
		ps->popMatrix();
#else
		ps->grestore();
#endif
	}


	// zero the current translation
	void PSPixelCanvas::loadIdentity() {
		translate(Int(-xt_), Int(-yt_));
		xt_ = yt_ = 0.0;
	}

	// translation functions
	// translate applies a relative translation to the current matrix and
	// can be used to position graphics.  Together with pushMatrix and
	// popMatrix it can be used to build heirarchical scenes.

	void PSPixelCanvas::translate(Int xt, Int yt) {
		xt_ += (float)xt;
		yt_ += (float)yt;
		ps->translate(xt_, yt_);
	}

	void PSPixelCanvas::getTranslation(Int & xt, Int & yt) const {
		xt = (Int)xt_;
		yt = (Int)yt_;
	}

	Int PSPixelCanvas::xTranslation() const {
		return (Int)xt_;
	}

	Int PSPixelCanvas::yTranslation() const {
		return (Int)yt_;
	}



	// (Not cacheable) set the draw buffer
	// Not supported.
//void PSPixelCanvas::setDrawBuffer(Display::DrawBuffer buf)
	void PSPixelCanvas::setDrawBuffer(Display::DrawBuffer buf) {
		PixelCanvas::setDrawBuffer_(buf);
	}

	// buffer memory exchanges
	// (Not cacheable)

	void PSPixelCanvas::copyBackBufferToFrontBuffer() {
	}

	void PSPixelCanvas::copyFrontBufferToBackBuffer() {
	}

	void PSPixelCanvas::swapBuffers() {
	}



	// partial buffer memory exchanges.  (x1,y1 are blc, x2,y2 are trc)

	void PSPixelCanvas::copyBackBufferToFrontBuffer(Int, Int, Int, Int) {
	}

	void PSPixelCanvas::copyFrontBufferToBackBuffer(Int, Int, Int, Int) {
	}

	void PSPixelCanvas::swapBuffers(Int, Int, Int, Int) {
	}


//
// drawLine
//
	void PSPixelCanvas::drawLine(Int x1, Int y1, Int x2, Int y2) {
		drawLine((Float)x1, (Float)y1, (Float)x2, (Float)y2);
	}
	void PSPixelCanvas::drawLine(Float x1, Float y1, Float x2, Float y2) {
		ps->drawLine(x1, y1, x2, y2);
	}
	void PSPixelCanvas::drawLine(Double x1, Double y1, Double x2, Double y2) {
		drawLine((Float)x1, (Float)y1, (Float)x2, (Float)y2);
	}

//
// drawLines (Matrix version)
//
	void PSPixelCanvas::drawLines(const Matrix<Int> & verts) {
		uInt nVerts = verts.nrow();
		uInt nlines = nVerts/2;
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		uInt k = 0;
		for(uInt i=0; i < nlines; i++) {
			x1[i] = (float)verts(k,0);
			y1[i] = (float)verts(k++,1);
			x2[i] = (float)verts(k,0);
			y2[i] = (float)verts(k++,1);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

	void PSPixelCanvas::drawLines(const Matrix<Float> & verts) {
		uInt nVerts = verts.nrow();
		uInt nlines = nVerts/2;
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		uInt k = 0;
		for(uInt i=0; i < nlines; i++) {
			x1[i] = verts(k,0);
			y1[i] = verts(k++,1);
			x2[i] = verts(k,0);
			y2[i] = verts(k++,1);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

	void PSPixelCanvas::drawLines(const Matrix<Double> & verts) {
		uInt nVerts = verts.nrow();
		uInt nlines = nVerts/2;
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		uInt k = 0;
		for(uInt i=0; i < nlines; i++) {
			x1[i] = (float)verts(k,0);
			y1[i] = (float)verts(k++,1);
			x2[i] = (float)verts(k,0);
			y2[i] = (float)verts(k++,1);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

//
// drawLines (Vector version)
//
	void PSPixelCanvas::drawLines(const Vector<Int> & X1, const Vector<Int> & Y1,
	                              const Vector<Int> & X2, const Vector<Int> & Y2) {
		uInt nlines = X1.nelements();
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		for(uInt i=0; i < nlines; i++) {
			x1[i] = (float)X1(i);
			y1[i] = (float)Y1(i);
			x2[i] = (float)X2(i);
			y2[i] = (float)Y2(i);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

	void PSPixelCanvas::drawLines(const Vector<Float> & X1,
	                              const Vector<Float> & Y1,
	                              const Vector<Float> & X2,
	                              const Vector<Float> & Y2) {
		uInt nlines = X1.nelements();
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		for(uInt i=0; i < nlines; i++) {
			x1[i] = (float)X1(i);
			y1[i] = (float)Y1(i);
			x2[i] = (float)X2(i);
			y2[i] = (float)Y2(i);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

	void PSPixelCanvas::drawLines(const Vector<Double> & X1,
	                              const Vector<Double> & Y1,
	                              const Vector<Double> & X2,
	                              const Vector<Double> & Y2) {
		uInt nlines = X1.nelements();
		float *x1 = new float[nlines];
		float *y1 = new float[nlines];
		float *x2 = new float[nlines];
		float *y2 = new float[nlines];
		for(uInt i=0; i < nlines; i++) {
			x1[i] = (float)X1(i);
			y1[i] = (float)Y1(i);
			x2[i] = (float)X2(i);
			y2[i] = (float)Y2(i);
		}
		ps->drawLines(nlines, x1, y1, x2, y2);
		delete [] x1;
		delete [] y1;
		delete [] x2;
		delete [] y2;
	}

//
// drawPoint
//
	void PSPixelCanvas::drawPoint(Int x1, Int y1) {
		ps->drawPoint((float) x1, (float)y1);
	}
	void PSPixelCanvas::drawPoint(Float x1, Float y1) {
		ps->drawPoint((float) x1, (float)y1);
	}
	void PSPixelCanvas::drawPoint(Double x1, Double y1) {
		ps->drawPoint((float) x1, (float)y1);
	}

//
// drawPoints (Matrix version)
//
	void PSPixelCanvas::drawPoints(const Matrix<Int> & verts) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPoints(const Matrix<Float> & verts) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPoints(const Matrix<Double> & verts) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

//
// drawPoints (Vector version)
//
	void PSPixelCanvas::drawPoints(const Vector<Int> & x1,
	                               const Vector<Int> & y1) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPoints(const Vector<Float> & x1,
	                               const Vector<Float> & y1) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPoints(const Vector<Double> & x1,
	                               const Vector<Double> & y1) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPoints(nPoints, x, y);
		delete [] x;
		delete [] y;
	}

//
// drawPolylines (Vector version)
	void PSPixelCanvas::drawPolylines(const Vector<Int> & x1,
	                                  const Vector<Int> & y1, const int close,
	                                  const int fill) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPolylines(const Vector<Float> & x1,
	                                  const Vector<Float> & y1, const int close,
	                                  const int fill) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPolylines(const Vector<Double> & x1,
	                                  const Vector<Double> & y1, const int close,
	                                  const int fill) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

//
// drawPolylines (Matrix version)
	void PSPixelCanvas::drawPolylines(const Matrix<Int> & verts, const int close,
	                                  const int fill) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPolylines(const Matrix<Float> & verts,
	                                  const int close, const int fill) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

	void PSPixelCanvas::drawPolylines(const Matrix<Double> & verts,
	                                  const int close, const int fill) {
		uInt nPoints = verts.nrow();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		for(uInt i=0; i < nPoints; i++) {
			x[i] = (float)verts(i,0);
			y[i] = (float)verts(i,1);
		}
		ps->drawPolyline(nPoints, x, y, close, fill);
		delete [] x;
		delete [] y;
	}

//
// drawPolyline (Vector version)
//
	void PSPixelCanvas::drawPolyline(const Vector<Int> & x1,
	                                 const Vector<Int> & y1) {
		drawPolylines(x1, y1, NOCLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolyline(const Vector<Float> & x1,
	                                 const Vector<Float> & y1) {
		drawPolylines(x1, y1, NOCLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolyline(const Vector<Double> & x1,
	                                 const Vector<Double> & y1) {
		drawPolylines(x1, y1, NOCLOSE, NOFILL);
	}

//
// drawPolyline (Matrix version)
	void PSPixelCanvas::drawPolyline(const Matrix<Int> & verts) {
		drawPolylines(verts, NOCLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolyline(const Matrix<Float> & verts) {
		drawPolylines(verts, NOCLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolyline(const Matrix<Double> & verts) {
		drawPolylines(verts, NOCLOSE, NOFILL);
	}

//
// drawPolygon (Vector version)
	void PSPixelCanvas::drawPolygon(const Vector<Int> & x1,
	                                const Vector<Int> & y1) {
		drawPolylines(x1, y1, CLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolygon(const Vector<Float> & x1,
	                                const Vector<Float> & y1) {
		drawPolylines(x1, y1, CLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolygon(const Vector<Double> & x1,
	                                const Vector<Double> & y1) {
		drawPolylines(x1, y1, CLOSE, NOFILL);
	}

//
// drawFilledPolygon (Vector version)
//
	void PSPixelCanvas::drawFilledPolygon(const Vector<Int> & x1,
	                                      const Vector<Int> & y1) {
		drawPolylines(x1, y1, CLOSE, FILL);
	}
	void PSPixelCanvas::drawFilledPolygon(const Vector<Float> & x1,
	                                      const Vector<Float> & y1) {
		drawPolylines(x1, y1, CLOSE, FILL);
	}
	void PSPixelCanvas::drawFilledPolygon(const Vector<Double> & x1,
	                                      const Vector<Double> & y1) {
		drawPolylines(x1, y1, CLOSE, FILL);
	}

//
// drawPolygon (Matrix version)
//
	void PSPixelCanvas::drawPolygon(const Matrix<Int> & verts) {
		drawPolylines(verts, CLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolygon(const Matrix<Float> & verts) {
		drawPolylines(verts, CLOSE, NOFILL);
	}
	void PSPixelCanvas::drawPolygon(const Matrix<Double> & verts) {
		drawPolylines(verts, CLOSE, NOFILL);
	}

//
// drawRectangle (fill version)
//
	void PSPixelCanvas::drawRectangle(Int x1, Int y1,
	                                  Int x2, Int y2, const int fill) {
		ps->drawRectangle( (float)x1, (float)y1,
		                   (float)x2, (float)y2, fill);
	}
	void PSPixelCanvas::drawRectangle(Float x1, Float y1,
	                                  Float x2, Float y2, const int fill) {
		ps->drawRectangle( (float)x1, (float)y1,
		                   (float)x2, (float)y2, fill);
	}
	void PSPixelCanvas::drawRectangle(Double x1, Double y1,
	                                  Double x2, Double y2, const int fill) {
		ps->drawRectangle( (float)x1, (float)y1,
		                   (float)x2, (float)y2, fill);
	}

//
// drawRectangle
//
	void PSPixelCanvas::drawRectangle(Int x1, Int y1, Int x2, Int y2) {
		drawRectangle(x1, y1, x2, y2, NOFILL);
	}
	void PSPixelCanvas::drawRectangle(Float x1, Float y1, Float x2, Float y2) {
		drawRectangle(x1, y1, x2, y2, NOFILL);
	}
	void PSPixelCanvas::drawRectangle(Double x1, Double y1, Double x2, Double y2) {
		drawRectangle(x1, y1, x2, y2, NOFILL);
	}

//
// drawFilledRectangle
//
	void PSPixelCanvas::drawFilledRectangle(Int x1, Int y1, Int x2, Int y2) {
		drawRectangle(x1, y1, x2, y2, FILL);
	}
	void PSPixelCanvas::drawFilledRectangle(Float x1, Float y1, Float x2,
	                                        Float y2) {
		drawRectangle(x1, y1, x2, y2, FILL);
	}
	void PSPixelCanvas::drawFilledRectangle(Double x1, Double y1, Double x2,
	                                        Double y2) {
		drawRectangle(x1, y1, x2, y2, FILL);
	}

	void PSPixelCanvas::drawColoredPoints(const Vector<Int> & x1,
	                                      const Vector<Int> & y1,
	                                      const Vector<uInt> & colors) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		float *c = new float[nPoints];
		for (unsigned int i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
			c[i] = (float)colors(i);
		}
		ps->drawColoredPoints( nPoints, x, y, c);
		delete [] x;
		delete [] y;
		delete [] c;
	}

	void PSPixelCanvas::drawColoredPoints(const Vector<Float> & x1,
	                                      const Vector<Float> & y1,
	                                      const Vector<uInt> & colors) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		float *c = new float[nPoints];
		for (unsigned int i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
			c[i] = (float)colors(i);
		}
		ps->drawColoredPoints( nPoints, x, y, c);
		delete [] x;
		delete [] y;
		delete [] c;
	}

	void PSPixelCanvas::drawColoredPoints(const Vector<Double> & x1,
	                                      const Vector<Double> & y1,
	                                      const Vector<uInt> & colors) {
		uInt nPoints = x1.nelements();
		float *x = new float[nPoints];
		float *y = new float[nPoints];
		float *c = new float[nPoints];
		for (unsigned int i=0; i < nPoints; i++) {
			x[i] = (float)x1(i);
			y[i] = (float)y1(i);
			c[i] = (float)colors(i);
		}
		ps->drawColoredPoints( nPoints, x, y, c);
		delete [] x;
		delete [] y;
		delete [] c;
	}

	void PSPixelCanvas::drawColoredPoints(const Matrix<Int> &xy,
	                                      const Vector<uInt> &colors) {
		PixelCanvas::drawColoredPoints(xy, colors);
	}
	void PSPixelCanvas::drawColoredPoints(const Matrix<Float> &xy,
	                                      const Vector<uInt> &colors) {
		PixelCanvas::drawColoredPoints(xy, colors);
	}
	void PSPixelCanvas::drawColoredPoints(const Matrix<Double> &xy,
	                                      const Vector<uInt> &colors) {
		PixelCanvas::drawColoredPoints(xy, colors);
	}

//
// drawColoredLines
//
	void PSPixelCanvas::drawColoredLines(const Vector<Int> & x1,
	                                     const Vector<Int> & y1,
	                                     const Vector<Int> & x2,
	                                     const Vector<Int> & y2,
	                                     const Vector<uInt> & colors) {
		uInt nLines = x1.nelements();
		ps->gsave();	// Retain current color.
		for (unsigned int i = 0; i< nLines; i++) {
			ps->setColor( (float)colors(i));
			ps->drawLine((float)x1(i), (float)y1(i),
			             (float)x2(i), (float)y2(i));
		}
		ps->grestore();
	}

	void PSPixelCanvas::drawColoredLines(const Vector<Float> & x1,
	                                     const Vector<Float> & y1,
	                                     const Vector<Float> & x2,
	                                     const Vector<Float> & y2,
	                                     const Vector<uInt> & colors) {
		uInt nLines = x1.nelements();
		ps->gsave();	// Retain current color.
		for (unsigned int i = 0; i< nLines; i++) {
			ps->setColor( (float)colors(i));
			ps->drawLine((float)x1(i), (float)y1(i),
			             (float)x2(i), (float)y2(i));
		}
		ps->grestore();
	}

	void PSPixelCanvas::drawColoredLines(const Vector<Double> & x1,
	                                     const Vector<Double> & y1,
	                                     const Vector<Double> & x2,
	                                     const Vector<Double> & y2,
	                                     const Vector<uInt> & colors) {
		uInt nLines = x1.nelements();
		ps->gsave();	// Retain current color.
		for (unsigned int i = 0; i< nLines; i++) {
			ps->setColor( (float)colors(i));
			ps->drawLine((float)x1(i), (float)y1(i),
			             (float)x2(i), (float)y2(i));
		}
		ps->grestore();
	}

	void PSPixelCanvas::note(const char *str) {
		ps->comment(str);
	}

	void PSPixelCanvas::note(const char *str, const uLong val) {
		char buf[128];
		sprintf(buf, "%s%ld", str, val);
		note(buf);
	}

	Float PSPixelCanvas::pixelScaling() const {
		Float x,y;
		getResolution(x,y);
		x /= 72.0;
		return x;
	}

} //# NAMESPACE CASA - END

