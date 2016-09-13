//# GLPixelCanvasColorTable.h: color table provision for X11 devices
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
//# $Id$

#ifndef TRIALDISPLAY_GLPIXELCANVASCOLORTABLE_H
#define TRIALDISPLAY_GLPIXELCANVASCOLORTABLE_H

#include <casa/aips.h>
#include <graphics/X11/X11Util.h>

#include <graphics/X11/X_enter.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <graphics/X11/X_exit.h>

#include <casa/Arrays/Vector.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/PixelCanvasColorTable.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of PixelCanvasColorTable for OpenGL.
// </summary>
// <synopsis>
// GLPixelCanvasColorTable is an X11PixelCanvasColorTable that has been
// modified to work within the constraints of the GLPixelCanvas. (e.g. Not
// assuming the default visual).
//
// Colormap values are written to a virtual colortable (an internal array).
// For Indexed (PseudoColor) windows, the values are sent to the hardware
// colormap. For RGB (trueColor) windows, the virtual colortable is used
// to emulate a hardware lookup table.
// </synopsis>
//
// <prerequisite>
// <li> <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// <li> How to get the X Screen pointer.
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <thrown>
//    AipsError
// </thrown>
// <use visibility=export>

// Colormap entry for the virtual colormap.
	class GLVColorTableEntry {
	public:
		GLVColorTableEntry();

		void operator=(const GLVColorTableEntry &);

		// Store/get the values.
		void put(const float red, const float green, const float blue) {
			red_ = red;
			green_ = green;
			blue_ = blue;
		}

		void get(float &red, float &green, float &blue)const {
			red = red_;
			green = green_;
			blue = blue_;
		}

		casacore::uInt	getIndex()const {
			return index_;
		}
		void	setIndex(const casacore::uInt i) {
			index_ = i;
		}

		casacore::uLong getPixel()const {
			return pixel_;
		}
		void setPixel(const unsigned long p) {
			pixel_ = p;
		}

		friend ostream &operator<<(ostream &s, const GLVColorTableEntry &x);

	private:
		casacore::uInt	index_;			// Index into map for this entry.
		casacore::uLong	pixel_;			// Real index or color value.
		float	red_, green_, blue_;	// Color component values. (0..1).
	};

	class GLPixelCanvasColorTable : public PixelCanvasColorTable {
	public:

		// <group>
		GLPixelCanvasColorTable();

		~GLPixelCanvasColorTable();

		GLPixelCanvasColorTable(::XDisplay *dpy,
		                        Display::ColorModel mapType = Display::RGB,
		                        casacore::Float percent=90.0,
		                        XVisualInfo *visInfo=NULL);
		// </group>

		// allocate cells for colormaps
		casacore::Bool allocCells(casacore::uInt nCells);

		// deallocate cells for colormaps
		casacore::Bool deallocCells();

		// Allocate the best color cube given the map
		casacore::Bool allocColorCube();
		// Allocate a color cube of a specific size
		casacore::Bool allocColorCube(casacore::uInt n1, casacore::uInt n2, casacore::uInt n3);
		// Allocate a color cube within the ranges of sizes
		casacore::Bool allocColorCubeMinMax(casacore::uInt n1min, casacore::uInt n2min, casacore::uInt n3min,
		                          casacore::uInt n1max, casacore::uInt n2max, casacore::uInt n3max);
		// Copy color cube info from the mapRef
		void copyColorCube(const GLPixelCanvasColorTable & mapRef);
		// Fill a color cube with an RGB spectrum
		void fillColorCubeRGB();
		enum FILLMODE {FILLRGB, FILLRBG, FILLGRB, FILLGBR, FILLBRG, FILLBGR};
		static void colorFillRGB(
		    casacore::Vector<casacore::Float> &r, casacore::Vector<casacore::Float> &g, casacore::Vector<casacore::Float> &b,
		    casacore::uInt nr, casacore::uInt ng, casacore::uInt nb,
		    FILLMODE mode = FILLRGB);

		static void colorFillRGB(
		    casacore::Vector<casacore::Float> &r, casacore::Vector<casacore::Float> &g, casacore::Vector<casacore::Float> &b,
		    casacore::uInt ncolors, FILLMODE mode= FILLRGB);
		// Fill a color cube with an HSV spectrum
		void fillColorCubeHSV();

		// Merge separate channel data into an output image.
		// This function maps floating values between 0 and 1
		// into a output image suitable for PixelCanvas::drawImage().
		// <group>
		void mapToColor3(casacore::Array<casacore::uLong> & out,
		                 const casacore::Array<casacore::Float> & chan1in,
		                 const casacore::Array<casacore::Float> & chan2in,
		                 const casacore::Array<casacore::Float> & chan3in);
		void mapToColor3(casacore::Array<casacore::uLong> & out,
		                 const casacore::Array<casacore::Double> & chan1in,
		                 const casacore::Array<casacore::Double> & chan2in,
		                 const casacore::Array<casacore::Double> & chan3in);
		// </group>

		// This one maps values between 0 and the integer
		// maximum value for each channel into a single
		// output image suitable for PixelCanvas::drawImage().
		// <group>
		void mapToColor3(casacore::Array<casacore::uLong> & out,
		                 const casacore::Array<casacore::uShort> & chan1in,
		                 const casacore::Array<casacore::uShort> & chan2in,
		                 const casacore::Array<casacore::uShort> & chan3in);
		void mapToColor3(casacore::Array<casacore::uLong> & out,
		                 const casacore::Array<casacore::uInt> & chan1in,
		                 const casacore::Array<casacore::uInt> & chan2in,
		                 const casacore::Array<casacore::uInt> & chan3in);
		// </group>

		// (Multichannel Color)
		// Transform arrays from the passed color model into
		// the colormodel of the XPCCT.
		// Does nothing if colorModel is Display::Index.
		// It is assumed that input arrays are in the range of [0,1]
		casacore::Bool colorSpaceMap(Display::ColorModel,
		                   const casacore::Array<casacore::Float> & chan1in,
		                   const casacore::Array<casacore::Float> & chan2in,
		                   const casacore::Array<casacore::Float> & chan3in,
		                   casacore::Array<casacore::Float> & chan1out,
		                   casacore::Array<casacore::Float> & chan2out,
		                   casacore::Array<casacore::Float> & chan3out);

		// map [0,N-1] into colorpixels, where N is the current colormap size
		// The values are returned as unsigned integers in their respective
		// array.
		// <note role="warning">casacore::uChar type may not have enough bits
		// to hold the pixel index on some high-end graphics systems </note>
		// <note role="warning">casacore::uShort type may not have enough bits
		// to hold the pixel index on some high-end graphics systems </note>
		// <group>
		void mapToColor(const Colormap * map, casacore::Array<casacore::uChar> & outArray,
		                const casacore::Array<casacore::uChar> & inArray, casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uShort> & outArray,
		                const casacore::Array<casacore::uShort> & inArray, casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uInt> & outArray,
		                const casacore::Array<casacore::uInt> & inArray, casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uLong> & outArray,
		                const casacore::Array<casacore::uLong> & inArray, casacore::Bool rangeCheck = true) const;
		// </group>

		// same as above except the matrix is operated on in place.  Only unsigned
		// values make sense here.
		// <group>
		void mapToColor(const Colormap * map, casacore::Array<casacore::uChar> & inOutArray,
		                casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uShort> & inOutArray,
		                casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uInt> & inOutArray,
		                casacore::Bool rangeCheck = true) const;
		void mapToColor(const Colormap * map, casacore::Array<casacore::uLong> & inOutArray,
		                casacore::Bool rangeCheck = true) const;
		// </group>

		// print details of class to ostream
		friend ostream & operator << (ostream & os, const GLPixelCanvasColorTable & pcc);

		// Is the hardware colormap resizeable?  ie. is it write-only?
		virtual casacore::Bool staticSize() {
			return (readOnly_ && decomposedIndex_);
		}

		// resize the map if allowed.  Returns true if resize was accepted
		// <group>
		casacore::Bool resize(casacore::uInt newSize);
		casacore::Bool resize(casacore::uInt nReds, casacore::uInt nGreens, casacore::uInt nBlues);
		// </group>

		// Install colors into the color table. Offset is zero-based. Colors
		// are installed into the PixelCanvasColorTable until the Arrays run out
		// or until the end of the colortable is reached.  This only has an
		// effect if the ColorModel is Index.  Values are clamped to [0.0,1.0].
		casacore::Bool installRGBColors(const casacore::Vector<casacore::Float> & r, const casacore::Vector<casacore::Float> & g,
		                      const casacore::Vector<casacore::Float> & b, casacore::uInt offset = 0);

		// Return the total number of RW colors currently in use.
		casacore::uInt nColors() const;

		// Return the number of colors per component in the map.  Throws
		// an exception if this is not an HSV or RGB ColorTable.
		virtual void nColors(casacore::uInt &n1, casacore::uInt &n2, casacore::uInt &n3) const;

		// Return the depth in bits of the colors
		casacore::uInt depth() const;

		// Return the number of colors that are still unallocated
		casacore::uInt nSpareColors() const;

		// Return pointer to display that is being used
		::XDisplay * display() const;
		// Return pointer to screen that is being used
		Screen * screen() const;
		// Return pointer to visual that is being used
		Visual * visual() const;
		// Return pointer to visual info that is being used
		XVisualInfo *visualInfo() const {
			return visualInfo_;
		}
		// Return XID of X "virtual colormap" being used
		XColormap xcmap() const;

		// Return true if the table is in colorIndex mode
		casacore::Bool indexMode() const {
			return (colorModel_ == Display::Index);
		}
		// Return true if the table is in RGB mode
		casacore::Bool rgbMode() const {
			return (colorModel_ == Display::RGB);
		}
		// Return true if the table is in HSV mode
		casacore::Bool hsvMode() const {
			return (colorModel_ == Display::HSV);
		}

		// Return true if the colortable can be resized.
		casacore::Bool rigid() const {
			return rigid_;
		}

		// Return the color model for multichannel color
		Display::ColorModel colorModel() const {
			return colorModel_;
		}
		casacore::Bool readOnly()const {
			return readOnly_;
		}
		casacore::Bool decomposedIndex()const {
			return decomposedIndex_;
		}
		// Return the number of currently unallocated cells that can be allocated RW.
		casacore::uInt QueryColorsAvailable(const casacore::Bool contig)const;
		virtual casacore::uInt QueryHWColorsAvailable(const casacore::Bool contig)const;
		// Convert a virtual index to a physical pixel.
		casacore::Bool virtualToPhysical(const unsigned long vindex,
		                       unsigned long &pindex)const;
		// Store an RGB value at index. For RGV visuals, only the virtual colormap
		// is updated.
		void storeColor(const casacore::uInt index,
		                const float r, const float g, const float b);
		// Convert a pixel to color components. If decomposed index (eg TC),
		// the pixel contains the color information. Otherwise, (eg. PseudoColor),
		// The information is looked up in the virtual colormap.
		void pixelToComponents(const casacore::uLong pixel, casacore::Float &r, casacore::Float &g, casacore::Float &b);
		// Return a GL capable visual that supports the colormodel or NULL.
		static XVisualInfo *getVisualInfo(::XDisplay *dpy,
		                                  const Display::ColorModel colormodel);
		// Returns the color values for the index. (Inverse of installRGBColors()).
		void indexToRGB(const casacore::uInt index, float &r, float &g, float &b);
	private:

		// Return the log power 2 of n and return true if n is
		// a power of two.  Otherwise return false.
		casacore::Bool isPow2(casacore::uInt n, casacore::uInt & log2n);

		// (Multi-Channel)
		void setupColorCube(casacore::uLong n1, casacore::uLong n2, casacore::uLong n3,
		                    casacore::uLong n1m, casacore::uLong n2m, casacore::uLong n3m);
		// (Multi-Channel)
		void setupStandardMapping(const XStandardColormap * mapInfo);

		casacore::Bool initVisual(XVisualInfo *vi=NULL);
		// A pointer to the XDisplay
		::XDisplay * display_;
		// A pointer the the X Screen
		Screen * screen_;
		// A pointer to the X Visual
		Visual * visual_;
		XVisualInfo *visualInfo_;
		// A pointer to the XColormap (X Hardware colormap)
		XColormap xcmap_;

		// (Valid Always) number of bits of depth
		casacore::uInt depth_;
		// (Valid Always) number of total colors available for RW.
		casacore::uInt nColors_;
		// (Valid only when implementation uses a PseudoColor or a
		// StaticColor visual).  casacore::Table of color indices available.
		casacore::uLong * colors_;	// converts pixel index into vcmap index. Always valid.
		casacore::uShort vcmapLength_;	// Length of virtual colormap.
		// (& HW map if it exists)
		GLVColorTableEntry *vcmap_;
		// (Valid Always)
		// true if the table may not be resized, such as when a
		// standard XColormap is used.  Generally this is set to true
		// unless the visual is PseudoColor/StaticColor and the color
		// model is Display::Index.
		casacore::Bool rigid_;
		////////////////////////////////////////////////////////////////
		// true if the colormap is read only.
		casacore::Bool	readOnly_;
		// true if trueColor or DirectColor.
		casacore::Bool  decomposedIndex_;
		void checkVisual();
		// Write an RGB value to hardware colormap at physical index.
		// (Called by storeColor()).
		virtual void storeHWColor(const casacore::uLong pindex,
		                          const float r, const float g, const float b);
		// Shift counts, masks, and max values used to build pixels for
		// decomposed index colormaps.
		unsigned short red_shift_, green_shift_, blue_shift_;
		unsigned short red_max_, green_max_, blue_max_;
		unsigned long red_mask_, green_mask_, blue_mask_;
		// Convert from integer HSV components to RGB pixel components.
		void HSV2RGB(const casacore::uLong H, const casacore::uLong S, const casacore::uLong V,
		             casacore::uLong &R, casacore::uLong &G, casacore::uLong &B);
		casacore::uInt HSV2Index(float h, float s, float v);

		////////////////////////////////////////////////////////////////
		// (Valid only for multi-channel color modes (RGB, HSV, etc))
		// If true, then the color cube's sides are powers of two,
		// making indexing possible with shift/add using the values of
		// <nBits1_, nBits2_, nBits3_>.  If not true, indexing
		// using <n1_,n2_,n3_> and multiplication is required.
		casacore::Bool pow2Mapping_;

		// (Valid Always)
		// The colormodel that this GLPixelCanvasColorTable has been
		// configured as.
		Display::ColorModel colorModel_;

		// (Valid only for multi-channel color modes (RGB, HSV, etc))
		// Represents the first cell used for the color cube.
		// baseColor_ is zero for PseudoColor/StaticColor implementations
		// because they use a table.
		casacore::uLong baseColor_;

		// (Valid only for multi-channel color modes (RGB, HSV, etc))
		// Specifies the color resolution for each side of the
		// color cube.
		// index = n1Mult_*R + n2Mult_*G + n3Mult_*B for RGB in
		// the range of <[0,n1_-1],[0,n2_-1],[0,n3_-1]>
		// <group>
		casacore::uInt n1_;
		casacore::uInt n2_;
		casacore::uInt n3_;

		casacore::uInt n1Mult_;
		casacore::uInt n2Mult_;
		casacore::uInt n3Mult_;
		// </group>

		// (Valid only for multi-channel color modes (RGB, HSV, etc))
		// and when pow2Mapping is true.
		// index = (R << n1Shift_) | (G << n2Shift_) | (B << n3Shift_)
		// for RGB the range of <[0,n1_-1],[0,n2_-1],[0,n3_-1]>
		// <group>
		casacore::uInt n1Shift_;
		casacore::uInt n2Shift_;
		casacore::uInt n3Shift_;
		// </group>
	};


} //# NAMESPACE CASA - END
#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/GLPCColTblTemplates.tcc>
#endif
#endif
