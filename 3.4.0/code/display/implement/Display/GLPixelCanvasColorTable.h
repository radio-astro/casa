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
// colormap. For RGB (TrueColor) windows, the virtual colortable is used
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
	void put(const float red, const float green, const float blue)
	{	red_ = red; green_ = green; blue_ = blue;
	}

	void get(float &red, float &green, float &blue)const
	{	red = red_; green = green_; blue = blue_;
	}

	uInt	getIndex()const{return index_;}
	void	setIndex(const uInt i){index_ = i;}

	uLong getPixel()const{return pixel_;}
	void setPixel(const unsigned long p){pixel_ = p;}

	friend ostream &operator<<(ostream &s, const GLVColorTableEntry &x);

  private:
	uInt	index_;			// Index into map for this entry.
	uLong	pixel_;			// Real index or color value.
	float	red_, green_, blue_;	// Color component values. (0..1).
};

class GLPixelCanvasColorTable : public PixelCanvasColorTable
{
public:

  // <group>
  GLPixelCanvasColorTable();

  ~GLPixelCanvasColorTable();

  GLPixelCanvasColorTable(::XDisplay *dpy,
			  Display::ColorModel mapType = Display::RGB,
			  Float percent=90.0,
			  XVisualInfo *visInfo=NULL);
  // </group>

  // allocate cells for colormaps
  Bool allocCells(uInt nCells);

  // deallocate cells for colormaps
  Bool deallocCells();

  // Allocate the best color cube given the map
  Bool allocColorCube();
  // Allocate a color cube of a specific size
  Bool allocColorCube(uInt n1, uInt n2, uInt n3);
  // Allocate a color cube within the ranges of sizes
  Bool allocColorCubeMinMax(uInt n1min, uInt n2min, uInt n3min,
			    uInt n1max, uInt n2max, uInt n3max);
  // Copy color cube info from the mapRef
  void copyColorCube(const GLPixelCanvasColorTable & mapRef);
  // Fill a color cube with an RGB spectrum
  void fillColorCubeRGB();
  enum FILLMODE{FILLRGB, FILLRBG, FILLGRB, FILLGBR, FILLBRG, FILLBGR};
  static void colorFillRGB(
			Vector<Float> &r, Vector<Float> &g, Vector<Float> &b,
			uInt nr, uInt ng, uInt nb,
			FILLMODE mode = FILLRGB);

  static void colorFillRGB(
			Vector<Float> &r, Vector<Float> &g, Vector<Float> &b,
			uInt ncolors, FILLMODE mode= FILLRGB);
  // Fill a color cube with an HSV spectrum
  void fillColorCubeHSV();

  // Merge separate channel data into an output image.
  // This function maps floating values between 0 and 1
  // into a output image suitable for PixelCanvas::drawImage().
  // <group>
  void mapToColor3(Array<uLong> & out,
		   const Array<Float> & chan1in,
		   const Array<Float> & chan2in,
		   const Array<Float> & chan3in);
  void mapToColor3(Array<uLong> & out,
		   const Array<Double> & chan1in,
		   const Array<Double> & chan2in,
		   const Array<Double> & chan3in);
  // </group>

  // This one maps values between 0 and the integer
  // maximum value for each channel into a single
  // output image suitable for PixelCanvas::drawImage().
  // <group>
  void mapToColor3(Array<uLong> & out,
		   const Array<uShort> & chan1in,
		   const Array<uShort> & chan2in,
		   const Array<uShort> & chan3in);
  void mapToColor3(Array<uLong> & out,
		   const Array<uInt> & chan1in,
		   const Array<uInt> & chan2in,
		   const Array<uInt> & chan3in);
  // </group>  

  // (Multichannel Color)
  // Transform arrays from the passed color model into
  // the colormodel of the XPCCT.
  // Does nothing if colorModel is Display::Index.
  // It is assumed that input arrays are in the range of [0,1]
  Bool colorSpaceMap(Display::ColorModel, 
		     const Array<Float> & chan1in, 
		     const Array<Float> & chan2in, 
		     const Array<Float> & chan3in, 
		     Array<Float> & chan1out, 
		     Array<Float> & chan2out, 
		     Array<Float> & chan3out);  

  // map [0,N-1] into colorpixels, where N is the current colormap size
  // The values are returned as unsigned integers in their respective 
  // array.  
  // <note role="warning">uChar type may not have enough bits
  // to hold the pixel index on some high-end graphics systems </note>
  // <note role="warning">uShort type may not have enough bits
  // to hold the pixel index on some high-end graphics systems </note>
  // <group>
  void mapToColor(const Colormap * map, Array<uChar> & outArray, 
		  const Array<uChar> & inArray, Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uShort> & outArray, 
		  const Array<uShort> & inArray, Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uInt> & outArray, 
		  const Array<uInt> & inArray, Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uLong> & outArray, 
		  const Array<uLong> & inArray, Bool rangeCheck = True) const;
  // </group>

  // same as above except the matrix is operated on in place.  Only unsigned
  // values make sense here.
  // <group>
  void mapToColor(const Colormap * map, Array<uChar> & inOutArray, 
		  Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uShort> & inOutArray, 
		  Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uInt> & inOutArray, 
		  Bool rangeCheck = True) const;
  void mapToColor(const Colormap * map, Array<uLong> & inOutArray, 
		  Bool rangeCheck = True) const;
  // </group>
  
  // print details of class to ostream
  friend ostream & operator << (ostream & os, const GLPixelCanvasColorTable & pcc);

  // Is the hardware colormap resizeable?  ie. is it write-only?
  virtual Bool staticSize() 
    { return (readOnly_ && decomposedIndex_); }

  // resize the map if allowed.  Returns True if resize was accepted
  // <group>
  Bool resize(uInt newSize);
  Bool resize(uInt nReds, uInt nGreens, uInt nBlues);
  // </group>

  // Install colors into the color table. Offset is zero-based. Colors
  // are installed into the PixelCanvasColorTable until the Arrays run out
  // or until the end of the colortable is reached.  This only has an
  // effect if the ColorModel is Index.  Values are clamped to [0.0,1.0].
  Bool installRGBColors(const Vector<Float> & r, const Vector<Float> & g, 
			const Vector<Float> & b, uInt offset = 0);

  // Return the total number of RW colors currently in use.
  uInt nColors() const;

  // Return the number of colors per component in the map.  Throws
  // an exception if this is not an HSV or RGB ColorTable.
  virtual void nColors(uInt &n1, uInt &n2, uInt &n3) const;

  // Return the depth in bits of the colors
  uInt depth() const;
  
  // Return the number of colors that are still unallocated
  uInt nSpareColors() const;

  // Return pointer to display that is being used
  ::XDisplay * display() const;
  // Return pointer to screen that is being used
  Screen * screen() const;
  // Return pointer to visual that is being used
  Visual * visual() const;
  // Return pointer to visual info that is being used
  XVisualInfo *visualInfo() const{ return visualInfo_;}
  // Return XID of X "virtual colormap" being used
  XColormap xcmap() const;
  
  // Return True if the table is in colorIndex mode
  Bool indexMode() const { return (colorModel_ == Display::Index); }
  // Return True if the table is in RGB mode
  Bool rgbMode() const { return (colorModel_ == Display::RGB); }
  // Return True if the table is in HSV mode
  Bool hsvMode() const { return (colorModel_ == Display::HSV); }

  // Return True if the colortable can be resized.
  Bool rigid() const { return rigid_; }

  // Return the color model for multichannel color
  Display::ColorModel colorModel() const { return colorModel_; }
  Bool readOnly()const{return readOnly_;}
  Bool decomposedIndex()const{return decomposedIndex_;}
  // Return the number of currently unallocated cells that can be allocated RW.
  uInt QueryColorsAvailable(const Bool contig)const;
  virtual uInt QueryHWColorsAvailable(const Bool contig)const;
  // Convert a virtual index to a physical pixel.
  Bool virtualToPhysical(const unsigned long vindex,
			 unsigned long &pindex)const;
  // Store an RGB value at index. For RGV visuals, only the virtual colormap
  // is updated.
  void storeColor(const uInt index,
		  const float r, const float g, const float b);
  // Convert a pixel to color components. If decomposed index (eg TC),
  // the pixel contains the color information. Otherwise, (eg. PseudoColor),
  // The information is looked up in the virtual colormap.
  void pixelToComponents(const uLong pixel, Float &r, Float &g, Float &b);
  // Return a GL capable visual that supports the colormodel or NULL.
  static XVisualInfo *getVisualInfo(::XDisplay *dpy,
				    const Display::ColorModel colormodel);
  // Returns the color values for the index. (Inverse of installRGBColors()).
  void indexToRGB(const uInt index, float &r, float &g, float &b);
private:

  // Return the log power 2 of n and return True if n is
  // a power of two.  Otherwise return false.
  Bool isPow2(uInt n, uInt & log2n);

  // (Multi-Channel)
  void setupColorCube(uLong n1, uLong n2, uLong n3,
		      uLong n1m, uLong n2m, uLong n3m);
  // (Multi-Channel)
  void setupStandardMapping(const XStandardColormap * mapInfo);

  Bool initVisual(XVisualInfo *vi=NULL);
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
  uInt depth_;
  // (Valid Always) number of total colors available for RW.
  uInt nColors_;
  // (Valid only when implementation uses a PseudoColor or a
  // StaticColor visual).  Table of color indices available.
  uLong * colors_;	// converts pixel index into vcmap index. Always valid.
  uShort vcmapLength_;	// Length of virtual colormap.
			// (& HW map if it exists)
  GLVColorTableEntry *vcmap_;
  // (Valid Always)
  // True if the table may not be resized, such as when a
  // standard XColormap is used.  Generally this is set to True
  // unless the visual is PseudoColor/StaticColor and the color
  // model is Display::Index.
  Bool rigid_;
  ////////////////////////////////////////////////////////////////
  // True if the colormap is read only.
  Bool	readOnly_;
  // True if TrueColor or DirectColor.
  Bool  decomposedIndex_;
  void checkVisual();
  // Write an RGB value to hardware colormap at physical index.
  // (Called by storeColor()).
  virtual void storeHWColor(const uLong pindex,
			    const float r, const float g, const float b);
  // Shift counts, masks, and max values used to build pixels for
  // decomposed index colormaps.
  unsigned short red_shift_, green_shift_, blue_shift_;
  unsigned short red_max_, green_max_, blue_max_;
  unsigned long red_mask_, green_mask_, blue_mask_;
  // Convert from integer HSV components to RGB pixel components.
  void HSV2RGB(const uLong H, const uLong S, const uLong V,
	       uLong &R, uLong &G, uLong &B);
  uInt HSV2Index(float h, float s, float v);

  ////////////////////////////////////////////////////////////////
  // (Valid only for multi-channel color modes (RGB, HSV, etc))
  // If true, then the color cube's sides are powers of two,
  // making indexing possible with shift/add using the values of
  // <nBits1_, nBits2_, nBits3_>.  If not true, indexing
  // using <n1_,n2_,n3_> and multiplication is required. 
  Bool pow2Mapping_;

  // (Valid Always)
  // The colormodel that this GLPixelCanvasColorTable has been 
  // configured as.
  Display::ColorModel colorModel_;

  // (Valid only for multi-channel color modes (RGB, HSV, etc))
  // Represents the first cell used for the color cube.
  // baseColor_ is zero for PseudoColor/StaticColor implementations
  // because they use a table.
  uLong baseColor_;

  // (Valid only for multi-channel color modes (RGB, HSV, etc))
  // Specifies the color resolution for each side of the
  // color cube.
  // index = n1Mult_*R + n2Mult_*G + n3Mult_*B for RGB in
  // the range of <[0,n1_-1],[0,n2_-1],[0,n3_-1]>
  // <group>
  uInt n1_;
  uInt n2_;
  uInt n3_;

  uInt n1Mult_;
  uInt n2Mult_;
  uInt n3Mult_;
  // </group>

  // (Valid only for multi-channel color modes (RGB, HSV, etc))
  // and when pow2Mapping is true.
  // index = (R << n1Shift_) | (G << n2Shift_) | (B << n3Shift_)
  // for RGB the range of <[0,n1_-1],[0,n2_-1],[0,n3_-1]>
  // <group>
  uInt n1Shift_;
  uInt n2Shift_;
  uInt n3Shift_;
  // </group>
};


} //# NAMESPACE CASA - END
#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/GLPCColTblTemplates.tcc>
#endif
#endif
