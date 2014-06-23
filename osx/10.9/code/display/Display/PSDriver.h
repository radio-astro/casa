//# PSDriver.h: Defines PSDriver, a low-level PostScript interface for the DL.
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

#ifndef TRIALDISPLAY_PSDRIVER_H
#define TRIALDISPLAY_PSDRIVER_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <casa/BasicSL/String.h>

//# Forward declarations
#include <casa/iosfwd.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Low level interface between PSPixelCanvas{ColorTable} and PostScript.
// </summary>

// <use visibility=local>

//<a name="Synopsis"></a>
// <synopsis>
// PSDriver takes graphics requests from
// <linkto class="PSPixelCanvas"> PSPixelCanvas </linkto> and
// <linkto class="PSPixelCanvasColorTable"> PSPixelCanvasColorTable </linkto>
// and generates PostScript code. It might be thought of as playing an
// analogous role to what Xlib does for the X11PixelCanvas{ColorTable}.
// It could be used separately, but mostly programers will just
// create a PSDriver object to pass to PSPixelCanvas{ColorTable}.
//
// While there are many constructors, PSDrivers will probably most frequently
// be created using something like:
//
// <srcblock>
//	PSdriver *ps = new PSDriver("filename", PSDriver::LETTER,
//				    PSDriver::PORTRAIT);
// </srcblock>
//
// If it were desired to use RGB colormode rather than INDEXED,
// the following could be added.
//
//<srcblock>
//	ps->setColorSpace(PSDriver::RGB);
//</srcblock>
//
// PSDriver is logically split into two sections. The 'upper' level
// contains public functions, such as drawLine(). These do various
// amounts of bookkeeping, such as bounding box calculations, then call
// the 'lower' level routines, typically with names starting with emit.
// (eg. emitLine() in this example).
//
// There are many public routines that will rarely be called from outside
// of PSDriver. Typical of these are the various xxxToAscii85 routines
// that are used internally to generate image data.

// </synopsis>
//
// <thrown>
// PSDriver does not currently explicitly throw any exceptions.
// </thrown>
//
//<a name="ToDo"></a>
// <todo>
// <ol>
// <li>	EPS support is preliminary.
// <li>	Vertical text justification doesn't look correct, presumably because
//	the character's bounding box includes interline space.
// <li>	No display list support.
// <li>	Clipping support is disabled since it is currently possible to
//	only reduce the size of the clipping path, not replace it.
// <li>	Points are drawn as filled circles of fixed size.
// </ol>
// </todo>
//
// Notes:
// <ol>
// <li> It is possible to switch between Index, RGB and HSV modes at will.
// <li>	The length of the color table is 4096 entries.
// <li> PostScript doesn't deal with pixels.
// <li>	The transform matrix affects virtually everything.
// <li> Font names must be known by the PostScript interpreter.
// </ol>

	class PSDriver {
	public:
		// The various color spaces PSDriver knows about.
		enum ColorSpace { UNKNOWNSPACE=-1, INDEXED, GRAY, RGB, HSV, HSB=HSV};
		// Different line types that can be drawn.
		enum LineStyle { UNKNOWNSTYPE=-1, SOLID, DASHED, DASHDASH};
		// Constants describing the the length of the indexed color table and the number of bits per color component.
		enum {NUMCOLORS = 4096, BITSPERCOMPONENT=12};
		enum Dimension { POINTS, INCHES, MM};
		enum Layout {	PORTRAIT=1, LANDSCAPE=2, EPS=0x4, EPS_PORTRAIT=0x5,
		                EPS_LANDSCAPE=0x6
		            };

//<srcblock>
		//		Options for drawImage.
		// SMOOTH	 Pixels are interpolated. Not supported by all
		//		interpreters. Old versions of ghostscript may core
		//		dump if this is selected.
//</srcblock>
		enum ImageOptions { NOIMGOPTS=0, SMOOTH=0x1};

		enum MediaSize {
		    // USERPAGE is used internally. It is not a valid media type.
		    USERPAGE=-1,
		    // The following lists the known media types (paper sizes).
		    LETTER=0, NA_LETTER=LETTER, LEGAL, NA_LEGAL=LEGAL,
		    NA_10X13_ENVELOPE, NA_9X12_ENVELOPE,
		    NA_NUMBER_10_ENVELOPE, NA_7X9_ENVELOPE, NA_9X11_ENVELOPE,
		    NA_10X14_ENVELOPE, NA_6X9_ENVELOPE, NA_10X15_ENVELOPE,
		    A, B, C, D, E,
		    ISO_A0, ISO_A1, ISO_A2, ISO_A3, ISO_A4, A4=ISO_A4, ISO_A5, ISO_A6,
		    ISO_A7, ISO_A8, ISO_A9, ISO_A10, ISO_B0, ISO_B1, ISO_B2, ISO_B3,
		    ISO_B4, ISO_B5, ISO_B6, ISO_B7, ISO_B8, ISO_B9, ISO_B10, ISO_C0,
		    ISO_C1, ISO_C2, ISO_C3, ISO_C4, ISO_C5, ISO_C6, ISO_C7, ISO_C8,
		    ISO_DESIGNATED,
		    JIS_B0, JIS_B1, JIS_B2, JIS_B3, JIS_B4, JIS_B5, JIS_B6, JIS_B7,
		    JIS_B8, JIS_B9, JIS_B10
		};
//
		enum TextAlign {
		    AlignCenter = Display::AlignCenter,
		    AlignLeft = Display::AlignLeft,
		    AlignTop = Display::AlignTop,
		    AlignRight = Display::AlignRight,
		    AlignBottom = Display::AlignBottom,
		    AlignTopLeft = Display::AlignTopLeft,
		    AlignTopRight = Display::AlignTopRight,
		    AlignBottomLeft = Display::AlignBottomLeft,
		    AlignBottomRight = Display::AlignBottomRight
		};

//	PSInfo is used to supply comment fields for the PostScript header
//	and other information that needs to be available when the PSDriver
//	object is created. Its use is optional.
		class PSInfo {
		public:
			PSInfo();
			~PSInfo();
			void Creator(const char *);
			void Creator(const String &);
			const String &Creator()const {
				return creator_;
			}
			void For(const char *);
			void For(const String &);
			const String &For()const {
				return for_;
			}
			void Title(const char *);
			void Title(const String &);
			const String &Title()const {
				return title_;
			}
			void Comment(const String &);
			void Comment(const char *);
			const String &Comment()const {
				return comment_;
			}

			void setMargins(const float lm, const float rm,
			                const float tm, const float bm,
			                const Dimension dim= PSDriver::INCHES);
			// If setMargins has been called, returns True and the
			// margin values. Otherwise, returns False and does not
			// change the arguments.
			Bool getMargins(float &lm, float &rm,
			                float &tm, float &bm);
		private:
			String	for_;
			String	title_;
			String	creator_;
			String	comment_;
			float	lm_, rm_, tm_, bm_;	// Margins.
			Bool	haveMargins_;		// setMargins has been called.
		};
		// Generic PostScript

	public:
// Arguments to the various constructors:
// <ul>
// <li> out 		is an ostream that the PostScript code is written to.
// <li> fname		it the name of the output file to be written to.
// <li> MediaSize	is the paper size. Each paper size has default margins.
// <li> Dimension	is POINTS, INCHES or MM. Describes values of x0,y0,x1,y1.
// <li> x0,y0,x1,y1	are used instead of MediaSize to explicitly give
//			the lower left and upper right corners of the
//			drawing surface.
// <li> Layout		is PORTRAIT or LANDSCAPE.
// <li> PSInfo		supplies optional header comments or margins.
//			Optional argument.
// </ul>
//
		// INDEXED, PORTRAIT to a default file.
		PSDriver();
		PSDriver(ostream &out);
		PSDriver(const char *filename);
		PSDriver(ostream &out, const MediaSize, const Layout=PORTRAIT,
		         PSInfo *info=NULL);
		PSDriver(const String &fname, const MediaSize, const Layout=PORTRAIT,
		         PSInfo *info=NULL);
		PSDriver(const char *fname, const MediaSize, const Layout=PORTRAIT,
		         PSInfo *info=NULL);

		PSDriver(ostream &out, const Dimension dim,
		         const float x0, const float y0,
		         const float x1, const float y1,
		         const Layout=PORTRAIT, PSInfo *info=NULL);
		PSDriver(const String &outname, const Dimension dim,
		         const float x0, const float y0,
		         const float x1, const float y1,
		         const Layout=PORTRAIT, PSInfo *info=NULL);
		PSDriver(const String &outname, const MediaSize,
		         const Dimension dim,
		         const float x0, const float y0,
		         const float x1, const float y1,
		         const Layout=PORTRAIT, PSInfo *info=NULL);
		PSDriver(const char *outname, const Dimension dim,
		         const float x0, const float y0,
		         const float x1, const float y1,
		         const Layout=PORTRAIT, PSInfo *info=NULL);
		~PSDriver();

//#	////////////////////////////////////////////////////////////////
//
		// Set/get desired color space. (Default is INDEXED).
//<group>
		void setColorSpace(const ColorSpace);
		inline ColorSpace colorSpace()const {
			return colorSpace_;
		}
//</group>
		// Set current color used for text and vectors. Only the first
		// argument is used for indexed & gray.
		// For Indexed mode, color should be in the range (0..ncolors-1)
		// where ncolors is the number of colors in the table. color
		// is truncated to an integer.
		// For the others, color should be in the range (0..1).
		void setColor(const float rh, const float gs, const float bv);
		// Set color value for indexed and gray scale modes.
		// If called when not in indexed or gray modes, the same value
		// is used for all three components.
		void setColor(const float color);
		// Color to be used for dashes.
		void setBackgroundColor(const float rh, const float gs,
		                        const float bv, const ColorSpace=RGB);
		// Set color value for indexed and gray scale modes.
		void setBackgroundColor(const float color, const ColorSpace=INDEXED);

		// Change color space then set color.
		void setIndexColor(const float indx);
		void setRGBColor(const float r, const float g, const float b);
		void setHSVColor(const float h, const float s, const float v) {
			setHSBColor(h, s, v);
		}
		void setHSBColor(const float h, const float s, const float b);

		// Load color tables with contents of a, b & c. Typically, a holds
		// red, b holds green & c holds blue. Values should be in the
		// range (0..1).
		// May be called anytime, but values are only used in INDEXED mode.
		// Start is starting index in the color table. Len is the
		// number of entries to set.
		void storeColors(const int start, const int len,
		                 const float *r, const float *g, const float *b);
		// Scatter load the colortable. Much more effecient if indexes are
		// consecutive.
		void storeColorValues(const int len, const int *indexes,
		                      const float *a, const float *b, const float *c);
		// Store 1 color.
		void storeColor(const int index,
		                const float r, const float g, const float b);
		// Load linear ramps (0..1.0) into the first ncolors entries.
		void setLinearRamps(const int ncolors);
//#	////////////////////////////////////////////////////////////////
//#	//		Coordinate commands.
		// Current transform matrix is changed by the given values, not
		// replaced.
// <group>
		// Change origin to x/y.
		void translate(const float x, const float y);
		// Change scale by x/y.
		void scale(const float x, const float y);
		// Rotate by # degrees.
		void rotate(const float degrees);
// </group>

		// Set the length of dashes.
		void setDashLength(const float length);
	public:
//#	////////////////////////////////////////////////////////////////
//<srcblock>
		//			Draw an Image
		// x0,y0	Image is drawn with lower left corner at x0, y0.
		//		(In current user coordinates).
		// width,height	Size of source image in elements.
		// xsize,ysize	Size of output image. (Eg. If current user
		//		coordinates were in inches, a size of 2 would create
		//		a two inch output square regardless of the size of the
		//		input image).
		// data		Data for an image that is width pixels wide by
		//		height pixels high. The actual length of the array
		//		depends on the ColorSpace argument:
		//		INDEXED:	The array contains width*height
		//				indexes in the range 0.. 2^bpc -1.
		//				(0..255 or 0..4095).
		//		RGB or HSV:	The array contains width*height
		//				RGB (HSV) triples in the range
		//				0..255 for bpc=8 or 0..1023 otherwise.
		//				The length of the array	is
		//				width*height*3.
		// bpc		Used to determing how data is encoded.
		//		If (0 < bpc <= 8) 8 bit encoding is used.
		//		If bpc > 8, 12 bit encoding is used.
		//		If bpc <= 0, the array is scanned to determine the
		//		encoding method.
		// ColorSpace	Whether image is INDEXED, RGB or HSV.
		// imageopts	Bitwise OR of IMAGEOPTS.
		//		 Currently only whether to smooth image.
//</srcblock>

		void drawImage( const float x0, const float y0,
		                const int width, const int height,
		                const float xsize, const float ysize,
		                const uShort *data,
		                const int bpc=0, const ColorSpace=INDEXED,
		                const int imageopts=0);

		// Save/restore graphics state.
//<group>
		void gsave();
		void grestore();
//</group>
//
//<srcblock>
		//			 Graphics
//</srcblock>
//

		void setLineStyle(const LineStyle);
		// w = 1.0 means 0.005".
		void lineWidth(const float w);

		void moveTo(const float x, const float y);
		void lineTo(const float x, const float y, const int stroke=1);

		void drawPolygon(const int len, const float *x, const float *y,
		                 const int fill=0);
		void drawPolyline(const int len, const float *x, const float *y,
		                  const int close=0, const int fill=0);
		void drawLine(  const float x0, const float y0,
		                const float x1, const float y1);
		void drawLines(const int len,   const float *x1, const float *y1,
		               const float *x2, const float *y2);
		void drawRectangle(const float x0, const float y0,
		                   const float x1, const float y1, const int fill=0);
		void drawFilledRectangle(const float x0, const float y0,
		                         const float x1, const float y1);
		void drawPoint( const float x, const float y, const float radius=1.0);
		void drawPoints(const int len, const float *x, const float *y,
		                const float radius=1.0);
		void drawColoredPoints( const int len, const float *x, const float *y,
		                        const float *colors, const float radius=1.0);


//#	////////////////////////////////////////////////////////////////
//
//<srcblock>
		//				Text
//</srcblock>
//#

//   Display a line of text. Tabs, newlines, etc. are not handled.
		void drawText(const float x, const float y, const char *str,
		              const TextAlign algn=AlignBottomLeft,
		              const float angle = 0);
//<note role=caution> PSDriver just passes the name to the output file.
//	It can't know whether the font really exists on the output device.
//</note>
		// Set font using current font scaling.
		void setFont(const char *fn);
		// Use font with supplied scaling.
		void findFont(const char *fn, const float scale);

		// Set/Get font scaling used with setFont.
		void setDefaultFontScale(const float scl);
		float getDefaultFontScale()const {
			return defaultFontSize_;
		}
//
//#	////////////////////////////////////////////////////////////////
//<srcblock>
		//				Misc.
//</srcblock>
//
		// Return page size (drawable area - margins).
		// If userCoords is True, values are in current user coordinates.
		// Otherwise, they are in transformed coordinates ( points).
		void pageSize(float &width, float &height,
		              const Bool userCoords=True);

		// Return bounding box.
		// Return is True if a bounding box was declared initially. Otherwise,
		// False and the current value of the bounding box will be returned.
		// If userCoords is True, values are in current user coordinates.
		// Otherwise, transformed coords.
		Bool getBoundingBox( float &x0, float &y0, float &x1, float &y1,
		                     const Bool userCoords=True)const;

		// Returns True if bounding box size was given to the constructor.
		inline Bool haveBoundingBox()const {
			return haveBoundingBox_;
		}

		// Put a comment in the output file.
		// The comment may include newlines.
//<group>
		void comment(const char *);
		void comment(const String &);
//</group>
		// Push/pop transform matrix.
//<group>
		void pushMatrix();
		void popMatrix();
//</group>
		void newPage();
		// Flush the output.
		void flush();
		// Done. Called by the destructor.
		void finish();
		// Set clipping rectangle.
		// Subsequent calls can only make the clipping rectangle smaller.
		void clipRect(  const float x0, const float y0,
		                const float width, const float height);
	public:
//
//<srcblock>
//		The following, while public, will almost will almost
// never be called from outside of PSDriver.
//</srcblock>

		// Routines to encode non negative binary numbers in ASCII85 format.
		// There will be upto 5/4 as many output characters as input bytes.
		// Returns the number of bytes written to out.

		static int bytesToAscii85(const char *in, const int inlength,
		                          char *out);
		static int uShorts8ToAscii85(const uShort *in,
		                             const int inlength, char *out);


		// Converts in to ASCII85 bytes using only lower 12 bits of each
		// integer. Out must be at least 1.75*inlength rounded up to
		// a multiple of 5 bytes long.
		// ( <1.5bytes/entry>*<inlength entries>*<1.25 expansion>).
		// If inlength is odd, a 0 valued entry will be implicitly added.
		static int uShorts12ToAscii85(const uShort *in,
		                              const int width,
		                              const int height, char *out);

		// Encode data as ASCII85 chars. Function return is an array of
		// chars. (use delete [] to free). outlen is set to the number of
		// bytes in the array.
		// On input, if bpc is 8, data is treated as containing 1 byte per
		// element. If bpc is 12, data is assumed to contain 12 bits/element.
		// If bpc is 0, the array is scanned.
		// On return, bpc is set the the value actually used.
		static char *encodeUShorts(const int width, const int height,
		                           int &bpc, const uShort *data,
		                           int &outlen);

//#	////////////////////////////////////////////////////////////////
		// Miscellaneous functions to convert between typical scalings.
		static inline float pointsToInches(const float in) {
			return in/72.0;
		}
		static inline float pointsToMM(const float in) {
			return in*(25.4/72.0);
		}
		static inline float inchesToPoints(const float in) {
			return in*72.0;
		}
		static inline float mmToPoints(const float in) {
			return in*(72./25.4);
		}

		static float toPoints(const float in, const Dimension indem);
		static float fromPoints(const float in, const Dimension indem);
//#	////////////////////////////////////////////////////////////////

		// Do forward or reverse transformation on a point.
		// If absolute is False, the translations aren't done.
		void toPoints(const float xin, const float yin,
		              float &xout, float &yout,
		              const Bool absolute=True)const {
			state_.toPoints(xin, yin, xout, yout, absolute);
		}

		void fromPoints(const float xin, const float yin,
		                float &xout, float &yout,
		                const Bool absolute=True)const {
			state_.fromPoints(xin, yin, xout, yout, absolute);
		}


		// Return a string representation of a dimension.
		static const char *dimensionToString(const PSDriver::Dimension dim);
		const char *dimensionToString()const {
			return dimensionToString(dimension_);
		}
		Dimension dimension()const {
			return dimension_;
		}

		// Is the output EPS? Portrait?
//<group>
		Bool isEPS()const {
			return eps_;
		}
		Bool isPortrait()const {
			return portrait_;
		}
//</group>

		// The PageInfo struct is used internally to supply descriptions
		// of the various page types.
		typedef struct PageInfo {
			PSDriver::MediaSize	media;		// A4, LETTER, etc.
			float			width;
			float			height;
			float			lrmargin;
			float			tbmargin;
			PSDriver::Dimension	dimension;	// MM or INCHES.
			char			*name; //"North American letter", etc.
			char			*alias1;	// "NA_LETTER", etc.
			char			*alias2;	// "LETTER", etc.
		} pageinfo_;
		// Given a paper type, return a pointer to its description.
		static const PSDriver::PageInfo *pageInfo(const PSDriver::MediaSize);
		// Return page description for index indx.
		static const PSDriver::PageInfo *getPageInfo(const int indx);
		// Lookup for a page description using the name field.
		static const PSDriver::PageInfo *lookupPageInfo(const char *name);
		static int numPageTypes();			// # of page types.
	private:
//<srcblock>
		// Similar to the public drawImage but uses a transform matrix instead of
		// position and scaling.
		// imagematrix		6 element array that defines transformation
		//			from user space to image space:
		//			m = [ a b c d tx ty ]
		//			 x' = ax + cy + tx
		//			 y' = bx + dy + ty
		//	The transformation is such that [ width 0 0 height 0 0] will
		//	map the image into a unit square which would then be scaled
		//	by the current scale() values.
		//	To invert the image, use: [ width 0 0 -height 0 height].
		//	If imagematrix is NULL, the image will be mapped into the
		//	unit square.
		// bpc is as above.
		// If smooth is 0, no smoothing is done. Otherwise, pixels are
		// interpolated.
		// Caution: This routine bypasses boundary box checks! If
		// BB checks are not important, then this routine could be public.
//</srcblock>
#if 0
		void drawImage(const float imagematrix[6],
		               const int width, const int height,
		               const uShort *data,
		               const int bpc=-1, const ColorSpace cs=INDEXED,
		               const int smooth=0);
#endif
//#	////////////////////////////////////////////////////////////////
		void newPath();		// These are used internally.
		void stroke();
		void closePath();
//#	////////////////////////////////////////////////////////////////
		void init(ostream &output, const ColorSpace cs,
		          const PageInfo *, const Layout, PSInfo *);
		void initname(const String &name, const ColorSpace cs,
		              const PageInfo *, const Layout, PSInfo *);
		// Compare x/y to current bounding box. x and y are in current
		// user coordinates unless userCoords is False in which case they
		// have already been transformed.
		void bbCheck(const float x, const float y, const Bool userCoords=True);
		void bbCheck();	// Do bbCheck on current position.

		void setCurrXY( const float x, const float y,
		                const Bool userCoords = True) {
			state_.setXY(x, y, userCoords);
		}
		void getCurrXY( float &x, float &y, const Bool userCoords=True) {
			state_.getXY(x, y, userCoords);
		}
	private:
		void startDocument();
		// Code writers.
		void emitHeader();
		void emitProlog();
		void emitTrailer();
		void emitShowPage(const int force=0);
		void emitPageNum();
		void emitSetup();

		void emitColorSpace(const ColorSpace);
		void emitLineStyle(const LineStyle);
		void emitDashLength(const float length);
		void emitLineWidth(const float w);
		void emitMoveTo(const float x, const float y);
		void emitLineTo(const float x, const float y, const int stroke=1);
		void emitPolyline(const int len, const float *x, const float *y,
		                  const int close, const int fill);
		void emitPolygon(const int len, const float *x, const float *y,
		                 const int fill);
		void emitLine(const float x0, const float y0,
		              const float x1, const float y1);
		void emitLines(const int len, const float *x1, const float *y1,
		               const float *x2, const float *y2);
		void emitRectangle(const float x, const float y,
		                   const float width, const float height,
		                   const int fill);
		void emitPoint(const float x, const float y, const float radius=1.0);
		void emitPoints(const int len, const float *x, const float *y,
		                const float radius=1.0);
		// Rotate by degrees.
		void emitRotate(const float degrees);
		// Scale. (new scale = <old scale>*<new scale>.
		void emitScale(const float x, const float y);
		// Change origin.
		void emitTranslate(const float x, const float y);

		void emitFindFont(const char *fn, const float scale);	// Load font.
		// Draw text starting at point x, y.
		// Newlines, tabs, etc. are not handled.
		void emitText(const float x, const float y, const char *str,
		              const TextAlign=AlignBottomLeft,
		              const float angle = 0);
		// Just throw a string on the stack and leave it there.
		void emitString(const char *str);

		// Draw a width x height PS image.
		// matrix		Matrix to convert between user & image space.
		// width, height	size of input image in pixels.
		// bpc			bits per color component. ( 8 or 12)
		// smooth		0 - no smoothing, 1 - smooth pixels.
		//			ghostscript may core dump if not 0.
		//  ascii85		data encoded as ASCII85 chars.
		//  len			length of data array.
		void emitImage( const float matrix[6],
		                const int width, const int height, const int bpc,
		                const int smooth, const char *ascii85, const int len,
		                const ColorSpace=INDEXED);
		// Load color tables with contents of a, b & c. Typically, a holds
		// red, b holds green & c holds blue. Values should be in the
		// range (0..1).
		void emitStoreColors(const int start, const int len,
		                     const float *a, const float *b, const float *c);
		// Load color tables given an array of colors and their corresponding
		// indexes. Much more effecient if indexes are consecutive.
		void emitStoreColorValues(const int len, const int *indexes,
		                          const float *a, const float *b, const float *c);
		// Internal helper function.
		void emitStoreColors(const int color, const int start, const int len,
		                     const float *ary);

		// Load one value into color table.
		void emitStoreColor(const int index,
		                    const float r, const float g, const float b);
		// Set curren`t color.
		void emitSetColor(const ColorSpace cs, const int index);
		void emitSetColor(const ColorSpace cs,
		                  const float a, const float b, const float c);
		// Load color first ncolors entries in the tables with linear
		// ramps scaled 0..1.
		void emitSetLinearRamps(const int ncolors);
		void emitGSave();
		void emitGRestore();
		void emitPushMatrix();
		void emitPopMatrix();
		void emitClipRect(const float x0, const float y0,
		                  const float width, const float height);
		void emitBackgroundColor(const float a, const float b, const float c,
		                         const ColorSpace cs);
		// Write value of bounding box to output.
		void emitBoundingBox();
	private:
		// Holds our copy of the current transformation matrix. Used for
		// bounding box computations.
		class PSState {
		public:
			PSState();
			~PSState();
			void scale( const float x, const float y);
			void translate( const float x, const float y);
			void rotate(const float degrees);
			// [ x' y' 1 ] = [ x y 1] * CTM
			inline void toPoints(const float xin, const float yin,
			                     float &xout, float &yout,
			                     const Bool absolute=True)const {
				xout = (float)(a_*xin + c_*yin);
				yout = (float)(b_*xin + d_*yin);
				if(absolute) {
					xout += (float)tx_;
					yout += (float)ty_;
				}
			}

			// [ x y 1 ] = [ x' y' 1] * inverse(CTM);
			inline void fromPoints(const float xin, const float yin,
			                       float &xout, float &yout,
			                       const Bool absolute=True)const {
				xout = (float)(ai_*xin + ci_*yin);
				yout = (float)(bi_*xin + di_*yin);
				if(absolute) {
					xout += (float)txi_;
					yout += (float)tyi_;
				}
			}

			void setIdentity();
			void operator=(const PSState &);
			// Set/Get the current position.
			void setXY(const float x, const float y,
			           const Bool userCoords=True);
			void getXY(float &x, float &y, const Bool userCoords=True);
		private:
			void invert();

			double	a_, b_, c_, d_, tx_, ty_;	// Forward
			double	ai_, bi_, ci_, di_, txi_, tyi_;	// Inverse
			float	currX_, currY_;			// Current x/y in pnts

		};
		void pushState();
		void popState();
		// Length of internal state(transform) stack.
		enum {STATESTACKLENGTH=16};
		int		statestackindex_;
		PSState		state_;
		PSState		statestack_[STATESTACKLENGTH];
		ostream		*out;
		std::fstream	*mystream_;
		ColorSpace	colorSpace_;
		Dimension	dimension_;
		Bool		portrait_;
		Bool		eps_;
		LineStyle	lineStyle_;
		float		defaultFontSize_;
		float		xscale_, yscale_; // Paper size scale.
		// Bounding box in points.
		float		bbx0_, bby0_;
		float		bbx1_, bby1_;
		Bool		boxCheck0_;		// Has anything been checked?
		Bool		haveBoundingBox_;
		Bool		checkBoundingBox_;
		// user coords.
		// Page boundaries in points;
		float		xll_, yll_;
		float		xur_, yur_;

		// Clipping rectangle in points.
		float		clipXll_, clipXur_;
		float		clipYll_, clipYur_;

		unsigned int	pageNum_;		// Current page number.
		int		finished_;		// Set when finished called.
		PSInfo		*info_;
	};


} //# NAMESPACE CASA - END

#endif
