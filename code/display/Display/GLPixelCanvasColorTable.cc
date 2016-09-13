//# GLPCColorTable.cc:	Implementation of PixelCanvasColorTable
//#			for GLPixelCanvas.
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

#if defined(OGL)
// GLPixelCanvasColorTable was derived from X11PixelCanvasColorTable.

#include <casa/Logging/LogIO.h>
#include <display/Display/ColorConversion.h>
#include <display/Display/ColorDistribution.h>
#include <graphics/X11/X11Util.h>
#include <display/Display/X11ResourceManager.h>
#include <display/Display/GLPixelCanvasColorTable.h>
#include <GL/glx.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/BasicMath/Math.h>
#include <math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	uInt GLPixelCanvasColorTable::nColors() const {
		return nColors_;
	}
	uInt GLPixelCanvasColorTable::depth() const {
		return depth_;
	}
	uInt GLPixelCanvasColorTable::nSpareColors() const {
		return QueryColorsAvailable(False);
	}
	::XDisplay * GLPixelCanvasColorTable::display() const {
		return display_;
	}
	Screen * GLPixelCanvasColorTable::screen() const {
		return screen_;
	}
	Visual * GLPixelCanvasColorTable::visual() const {
		return visualInfo_->visual;
	}
	XColormap GLPixelCanvasColorTable::xcmap() const {
		return xcmap_;
	}

// Return the number of colors per component in the map.
	void GLPixelCanvasColorTable::nColors(uInt &n1, uInt &n2, uInt &n3) const {
		if ((colorModel_ == Display::RGB) || (colorModel_ == Display::HSV)) {
			if( decomposedIndex()) {		// Use actual HW value.
				n1 = red_max_+1;
				n2 = green_max_+1;
				n3 = blue_max_+1;
				return;
			} else {			// Use values computed from colormap.
				n1 = n1_;
				n2 = n2_;
				n3 = n3_;
				return;
			}
		} else {
			throw(AipsError("Cannot count component colors in non-HSV/RGB "
			                "GLPixelCanvasColorTable"));
		}
	}

// Count the # of trailing 0 bits in a #. (Amount needed to shift a value).
	static inline unsigned short shiftcount(const unsigned long v) {
		unsigned int mask = 1;
		const short nbits = sizeof(v)*8;
		register int i;
		for(i=0; i<nbits; i++,mask += mask)
			if(mask&v)
				break;
		return i;
	}

// Length of virtual color table to use when dealing with RO colormaps.
// The size is chosen to match PostScript capabilities.
	static const uInt VIRTUAL_CMAP_SIZE = 4096;

// Initialize info for virtual colormap decomposed index handling.
	void GLPixelCanvasColorTable::checkVisual() {
		Visual *visual = visualInfo_->visual;

		switch( visual->c_class) {
		case StaticGray:
			readOnly_ = True;
			decomposedIndex_ = False;
			break;
		case GrayScale:
			readOnly_ = False;
			decomposedIndex_ = False;
			break;
		case StaticColor:
			readOnly_ = True;
			decomposedIndex_ = False;
			break;
		case TrueColor:
			readOnly_ = True;
			decomposedIndex_ = True;
			break;
		case DirectColor:
			readOnly_ = False;
			decomposedIndex_ = True;
			break;
		case PseudoColor:
		default:
			readOnly_ = False;
			decomposedIndex_ = False;
			break;
		}
		// Initialize the virtual colormap. Possibly not the best place to
		// do it, but I don't have a better place yet.
		if(colors_ == static_cast<uLong *>(0)) {
			uInt len;
			if(!readOnly_)
				len = visual->map_entries;
			else
				len = VIRTUAL_CMAP_SIZE;

			colors_ = new uLong[len];
			vcmap_ = new GLVColorTableEntry[len];
			vcmapLength_ = len;
			// Set initial defaults.
			for(uInt i =0; i< len; i++) {
				colors_[i] = i;
				vcmap_[i].setPixel(i);
				// To go from pixel/rgb to vIndex.
				vcmap_[i].setIndex(i);
			}
		}

		// Initialize TC pixel mask, shift & max values.
		if(decomposedIndex_) {
			red_mask_ = visual->red_mask;
			red_shift_ = shiftcount(red_mask_);
			if(red_shift_ > 0)
				red_max_ = (int)(red_mask_ >> red_shift_);
			else
				red_max_ = (int)red_mask_;

			green_mask_ = visual->green_mask;
			green_shift_ = shiftcount(green_mask_);
			if(green_shift_ > 0)
				green_max_ = (int)(green_mask_ >> green_shift_);
			else
				green_max_ = (int)green_mask_;

			blue_mask_ = visual->blue_mask;
			blue_shift_ = shiftcount(blue_mask_);
			if(blue_shift_ > 0)
				blue_max_ = (int)(blue_mask_ >> blue_shift_);
			else
				blue_max_ = (int)blue_mask_;
		} else {
			red_shift_ = green_shift_ = blue_shift_ = 0;
			red_max_ = green_max_ = blue_max_ = 0;
			red_mask_ = green_mask_ = blue_mask_ = 0;
		}
	}

	Bool GLPixelCanvasColorTable::installRGBColors(const Vector<Float> & r,
	        const Vector<Float> & g,
	        const Vector<Float> & b,
	        uInt offset) {
		if (offset + r.nelements() > nColors_)
			throw(AipsError("GLPixelCanvasColorTable::installRGBColors: offset + vector length > nColors in"));

		for (uInt i = 0; i < r.nelements(); i++) {
			storeColor(i+offset, r(i), g(i), b(i));
		}
		return True;
	}

	Bool GLPixelCanvasColorTable::resize(uInt newSize) {
		uInt oldSize = nColors();

		// If the new size is smaller, we will always succede.  If the new size
		// is bigger, we may not have the space we require
		if (newSize > oldSize) {

			uInt availColors = QueryColorsAvailable(True);
			if (availColors < newSize - oldSize) {
				LogIO os;
				os << LogIO::WARN << LogOrigin("GLPixelCanvasColorTable",
				                               "resize", WHERE)
				   << "There were not enough colors to satisfy the resize request"
				   << LogIO::POST;
				return False;
			}
		}

		// Ok now we can try a reallocation using the current map.  It may
		// still fail if someone else alloc'd cells between the calls to
		// X11QueryColorsAvailable() and the allocCells() below.
		doResizeCallbacks(Display::ClearPriorToColorChange);
		deallocCells();

		// Realloc using Custom
		Bool ok = allocCells(newSize);
		if (ok) {
			colormapManager().redistributeColormaps();
			return True;
		}

		// Uh oh.  Someone alloc'ed cells during that short time.  Try to
		// get our old size back and recover.
		ok = allocCells(oldSize);
		if (ok) {
			LogIO os;
			os << LogIO::WARN << LogOrigin("GLPixelCanvasColorTable", "resize",
			                               WHERE)
			   << "Could not allocate enough colors to satisfy the resize,\n"
			   << "number of colors returned to " << oldSize
			   << LogIO::POST;
			colormapManager().redistributeColormaps();
			return False;
		}

		// If we get here, we were unable to restore the old size, so
		// throw an exception.
		throw(AipsError("Color cells have been lost - unsupported situation"));

		return False;  // to stop compiler warning
	}

	Bool GLPixelCanvasColorTable::resize(uInt n1, uInt n2, uInt n3) {
		// correct for zero entries if present
		if (n1 == 0) n1 = 1;
		if (n2 == 0) n2 = 1;
		if (n3 == 0) n3 = 1;

		// Return True immediately if no change needed
		if (n1_ == n1 && n2_ == n2 & n3_ == n3) return True;

		// Test to see if there is a change in the number of colors needed.  Resize
		// if that is so.
		if (n1*n2*n3 != n1_*n2_*n3_) {
			// Fail if rigid
			if (rigid_) {
				// This GLPixelCanvasColorTable cannot be resized
				return False;
			}

			uInt n1o = n1_;
			uInt n2o = n2_;
			uInt n3o = n3_;

			n1_ = n1;
			n2_ = n2;
			n3_ = n3;

			doResizeCallbacks(Display::ClearPriorToColorChange);
			// resize to suit
			deallocCells();

			// Try to allocate space for the new cube
			if (!allocColorCube(n1, n2, n3)) {
				// failed, fallback to original
				n1_ = n1o;
				n2_ = n2o;
				n3_ = n3o;

				if (!allocColorCube(n1o, n2o, n3o)) {
					// failed to recover
					throw(AipsError("GLPixelCanvasColorTable::buildMapRGBBest(xpcct):\n"
					                "could not allocate color cube on passed colormap"));
				}
			}
		} else {
			// don't need to realloc cells
			n1_ = n1;
			n2_ = n2;
			n3_ = n3;
		}

		setupColorCube(n1_, n2_, n3_, 1, n1_, n1_*n2_);

		switch(colorModel_) {
		case Display::RGB:
			fillColorCubeRGB();
			break;
		case Display::HSV:
			fillColorCubeHSV();
			break;
		case Display::Index:
			// won't get here
			break;
		}

		doResizeCallbacks();
		return True;
	}

// 5
////////////////////////////////////////////////////////////////
// (called from SimpleWorldGLCanvasApp)
	GLPixelCanvasColorTable::GLPixelCanvasColorTable(::XDisplay *dpy,
	        Display::ColorModel colorModel,
	        Float percent,
	        XVisualInfo *vi)
		: display_(dpy),
		  screen_(0),
		  visual_(0),
		  xcmap_(0),
		  depth_(0),
		  nColors_(0),
		  colors_(0),
		  rigid_(False),
		  pow2Mapping_(False),
		  colorModel_(colorModel),
		  n1_(0),
		  n2_(0),
		  n3_(0) {

		if(!initVisual(vi))
			throw(AipsError(
			          "GLPixelCanvasColorTable:Could not get requested GL visual."));
		checkVisual();

		uInt totalCells = QueryColorsAvailable(True);
		uInt nCells = 0;

		// Compute # of color cells to use.
		// Special hack: If percent is <= -2, treat it as the negative of the
		// number of cells to use.
		if(percent <= -2.0) {
			nCells = -(uInt)percent;
			if(nCells > totalCells)
				nCells = totalCells;
		} else {
			if(percent <= 0.0)
				percent = 90.0;
			else if(percent > 100.0)
				percent = 100.0;
			nCells = (uInt)(totalCells * percent / 100.0);
		}
		Bool ok = False;
		uInt nr,ng,nb;

		if (!getRGBDistribution(nCells, False, nr, ng, nb))
			throw(AipsError("GLPixelCanvasColorTable::GLPixelCanvasColorTable -\n"
			                "Can't allocate color cube in shared colortable."));
		uInt nAlloc = nr*ng*nb;
		ok = allocCells(nAlloc);

		setupColorCube(nr, ng, nb, 1, nr, nr*ng);

		switch(colorModel_) {
		case Display::Index:
			fillColorCubeRGB();
			break;
		case Display::RGB:
			fillColorCubeRGB();
			break;
		case Display::HSV:
			fillColorCubeHSV();
			break;
		}
	}

// Saves information from the supplied visual or creates one if necessary.
// Creates X colormap.
// Returns T of OK, else F if no available visualInfo.
	Bool GLPixelCanvasColorTable::initVisual(XVisualInfo *vi) {
		if(vi != NULL)
			visualInfo_ = vi;
		else if((visualInfo_ = getVisualInfo(display_, colorModel_)) == NULL)
			return False;

		visual_ = visualInfo_->visual;
		screen_ = ScreenOfDisplay(display_, visualInfo_->screen);

		depth_ = visualInfo_->depth;
		xcmap_ = XCreateColormap(display_, RootWindowOfScreen(screen_),
		                         visual_, AllocNone);
		X11ResourceManager::refColormap(screen_, xcmap_);
		return True;
	}

	GLPixelCanvasColorTable::~GLPixelCanvasColorTable() {
		deallocCells();
		X11ResourceManager::unrefColormap(screen_, xcmap_);
	}

//-------------------------------------------------------------------------
//
//  Index Colormaps
//
//--------------------------------------------------------------------------

// Deallocate color cells. Really do it if we're using a HW map. Fake it
// for RO maps.
	Bool GLPixelCanvasColorTable::deallocCells() {
		if(!readOnly() && (nColors_ > 0))
			XFreeColors(display_, xcmap_, colors_, nColors_, (uLong) 0);

		nColors_ = 0;
		return True;
	}

// Allocate RW color cells from colormap. For read only maps, we're faking
// so request always succeeds as long as the request is within range.
// On entry, it is assumed no RW colors are currently allocated.
// If the request fails, nColors_ will be set to 0.
// colors_ is assumed to have been allocated during initialization.
	Bool GLPixelCanvasColorTable::allocCells(uInt nCells) {
		Bool ok;

		// Assume max colormap length if 0. (Probably only done if
		// using RO color maps so we have to succeed).
		if(nCells == 0)
			nCells = vcmapLength_;

		if(readOnly_) {
			if(nCells <= vcmapLength_) {
				ok = True;
			} else
				ok = False;
		} else {
			uLong *colors = new uLong[nCells];
			uLong planeMask[1];

			ok = (XAllocColorCells(display_, xcmap_, 1, planeMask, 0,
			                       colors, nCells) > 0);
			if(ok) {
				// Initialize mapping array (colors_)
				for(uLong i = 0; i< nCells; i++) {
					uLong indx = colors[i];
					colors_[i] = indx;
					vcmap_[indx].setIndex(i);
				}
			}

			delete [] colors;
		}

		if(ok)
			nColors_ = nCells;
		else
			nColors_ = 0;

		return ok;
	}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//
//  Multi-Channel support functions
//
//--------------------------------------------------------------------------
	Bool GLPixelCanvasColorTable::isPow2(uInt n, uInt & log2n) {
		if (n == 0) {
			log2n = 0;
			return True;
		}
		for (uInt r = 0; r < 31; r++)
			if (n - (1 << r) == 0) {
				log2n = r;
				return True;
			}
		return False;
	}

	Bool GLPixelCanvasColorTable::allocColorCube() {
		// Try to make a colour cube with remaining colors in xcmap_
		Bool ok = False;
		uInt nr,ng,nb;
		while (!ok) {
			uInt nCells = QueryColorsAvailable(True);
			if (!getRGBDistribution(nCells, False, nr, ng, nb))
				throw(AipsError("GLPixelCanvasColorTable::allocColorCube(xpcct) -\n"
				                "Can't allocate color cube in shared colortable."));
			uInt nAlloc = nr*ng*nb;
			ok = allocCells(nAlloc);
		}

		setupColorCube(nr, ng, nb, 1, nr, nr*ng);
		return ok;
	}

	Bool GLPixelCanvasColorTable::allocColorCube(uInt n1, uInt n2, uInt n3) {
		uInt nCells = QueryColorsAvailable(True);
		uInt nAlloc = n1*n2*n3;
		if (nCells < (n1*n2*n3))
			return False;

		if (!allocCells(nAlloc))
			return False;

		setupColorCube(n1,n2,n3,1,n1,n1*n2);

		return True;
	}

	Bool GLPixelCanvasColorTable::allocColorCubeMinMax(
	    uInt n1min, uInt n2min, uInt n3min,
	    uInt n1max, uInt n2max, uInt n3max) {
		Bool ok = False;

		uInt n1 = n1max;
		uInt n2 = n2max;
		uInt n3 = n3max;

		Bool c1,c2,c3;

		ok = allocColorCube(n1,n2,n3);
		while (!ok) {
			if (n1 == n1min && n2 == n2min && n3 == n3min)
				return False;

			// pick component to decrement
			c1 = (n1 > n1min);
			c2 = (n2 > n2min);
			c3 = (n3 > n3min);

			if (c3 && c2)
				if (n3 >= n2) c2 = False;
				else c3 = False;
			if (c3 && c1)
				if (n3 >= n1) c1 = False;
				else c3 = False;
			if (c2 && c1)
				if (n2 >= n1) c1 = False;
				else c2 = False;

			if (c1) n1--;
			if (c2) n2--;
			if (c3) n3--;

			ok = allocColorCube(n1,n2,n3);
		}
		return ok;
	}

	void GLPixelCanvasColorTable::copyColorCube(
	    const GLPixelCanvasColorTable & mapRef) {
		pow2Mapping_ = mapRef.pow2Mapping_;
		baseColor_ = mapRef.baseColor_;
		n1_ = mapRef.n1_;
		n2_ = mapRef.n2_;
		n3_ = mapRef.n3_;
		n1Mult_ = mapRef.n1Mult_;
		n2Mult_ = mapRef.n2Mult_;
		n3Mult_ = mapRef.n3Mult_;
		n1Shift_ = mapRef.n1Shift_;
		n2Shift_ = mapRef.n2Shift_;
		n3Shift_ = mapRef.n3Shift_;
		X11ResourceManager::refColormap(screen_, xcmap_);
	}

// assumes color cube has been setup
	void GLPixelCanvasColorTable::fillColorCubeRGB() {
		for (uInt b = 0; b < n3_; b++)
			for (uInt g = 0; g < n2_; g++)
				for (uInt r = 0; r < n1_; r++) {
					uInt colorCubeIndex;
					if (pow2Mapping_)
						colorCubeIndex = (r << n1Shift_) | (g << n2Shift_)
						                 | (b << n3Shift_);
					else
						colorCubeIndex = n1Mult_*r + n2Mult_*g + n3Mult_*b;

					float red, green, blue;

					red = (n1_>1) ? ((float)r / (n1_-1)) : 0.0;
					green = (n2_>1) ? ((float)g / (n2_-1)) : 0.0;
					blue = (n3_>1) ? ((float)b / (n3_-1)) : 0.0;
					storeColor(colorCubeIndex, red, green, blue);
				}
	}

	void GLPixelCanvasColorTable::colorFillRGB(
	    Vector<Float> &r, Vector<Float> &g,
	    Vector<Float> &b,
	    uInt nr, uInt ng, uInt nb,
	    FILLMODE mode) {
		float rs, gs, bs;	// Scale.
		uInt  rm, gm, bm;	// Multiplier.

		if(nr == 0) nr = 1;
		if(ng == 0) ng = 1;
		if(nb == 0) nb = 1;

		rs = (nr > 1) ? 1.0/(nr-1) : 0.0;
		gs = (ng > 1) ? 1.0/(ng-1) : 0.0;
		bs = (nb > 1) ? 1.0/(nb-1) : 0.0;

		// Order in which colors are loaded into tables.
		switch(mode) {
		default:
		case FILLRGB:
			rm = 1;
			gm = nr;
			bm = nr*ng;
			break;
		case FILLRBG:
			rm = 1;
			bm = nr;
			gm = nr*nb;
			break;
		case FILLGRB:
			gm = 1;
			rm = ng;
			bm = ng*nr;
			break;
		case FILLGBR:
			gm = 1;
			bm = ng;
			rm = ng*nb;
			break;
		case FILLBRG:
			bm = 1;
			rm = nb;
			gm = nb*nr;
			break;
		case FILLBGR:
			bm = 1;
			gm = nb;
			rm = nb*ng;
		}

		for (uInt bi = 0; bi < nb; bi++) {
			float blue = bi*bs;
			for (uInt gi = 0; gi < ng; gi++) {
				float green = gi*gs;
				for (uInt ri = 0; ri < nr; ri++) {
					float red = ri*rs;
					uInt index;

					index = ri*rm + gi*gm + bi*bm;
					r(index) = red;
					g(index) = green;
					b(index) = blue;
				}
			}
		}
	}

// Store the RGB values.
	void GLPixelCanvasColorTable::storeColor(const uInt vindex,
	        const float r, const float g, const float b) {
		if(vindex >= nColors_) {
			char buf[128];
			sprintf(buf,
			        "GLPixelCanvasColorTable::storeColor: bad index %d",
			        nColors_);
			throw(AipsError(buf));
		} else {
			uLong pindex = colors_[vindex];
			vcmap_[pindex].put(r, g, b);
			storeHWColor(pindex, r, g, b);
		}
	}

// Convert a float (0..1) to an X11 color component value.
	static inline unsigned long floatToComponent(const float v,
	        const unsigned short max, const short shift,
	        const unsigned long mask) {
		unsigned long p = (((unsigned long) (v*max))<< shift) & mask;
		return p;
	}

// Write an RGB color value to HW colormap if it's RW. For decomposed index
// colormaps, the pixel representation of (r/g/b) is generated and placed
// in the virtual colormap.
	void GLPixelCanvasColorTable::storeHWColor(const uLong pindex,
	        const float r, const float g, const float b) {
		// pindex is the index into the virtual colormap. For PseudoColor
		// colormaps, this is also the pixel value. For decomposedIndexes
		// ( TrueColor & DirectColor), it is necessary to generate the
		// pixel value.
		uLong pixel = pindex;
		if(decomposedIndex()) {
			unsigned long ri, gi, bi;
			ri = floatToComponent(r, red_max_, red_shift_, red_mask_);
			gi = floatToComponent(g,
			                      green_max_, green_shift_, green_mask_);
			bi = floatToComponent(b,blue_max_, blue_shift_, blue_mask_);
			pixel = ri | bi | gi;
			vcmap_[pindex].setPixel(pixel);
		}

		if(!readOnly_) {
			XColor xc;
			const float	scl =  65535.;
			xc.pixel = pixel;
			xc.red = (uShort)(r*scl);
			xc.green = (uShort)(g*scl);
			xc.blue = (uShort)(b*scl);
			xc.flags = DoRed | DoGreen | DoBlue;
			XStoreColor(display_, xcmap_, &xc);
		}
	}

	void GLPixelCanvasColorTable::indexToRGB(const uInt index,
	        float &r, float &g, float &b) {
		if(index >= nColors_) {
			throw(AipsError("GLPixelCanvasColorTable::indexToRGB: bad index"));
		}
		vcmap_[index].get(r, g, b);
	}


	void GLPixelCanvasColorTable::fillColorCubeHSV() {
		for (uInt v = 0; v < n3_; v++)
			for (uInt s = 0; s < n2_; s++)
				for (uInt h = 0; h < n1_; h++) {
					uInt colorCubeIndex;
					if (pow2Mapping_)
						colorCubeIndex = (h << n1Shift_) | (s << n2Shift_) | (v << n3Shift_);
					else
						colorCubeIndex = n1Mult_*h + n2Mult_*s + n3Mult_*v;

					Float hf = (n1_>1) ? ((Float) h) / (n1_-1) : 0.0;
					Float sf = (n2_>1) ? ((Float) s) / (n2_-1) : 1.0;
					Float vf = (n3_>1) ? ((Float) v) / (n3_-1) : 1.0;

					Float r,g,b;
					hsvToRgb(hf,sf,vf,r,g,b);

					storeColor(colorCubeIndex, r, g, b);
				}
	}

	void GLPixelCanvasColorTable::setupColorCube(uLong n1,
	        uLong n2,
	        uLong n3,
	        uLong n1mult,
	        uLong n2mult,
	        uLong n3mult) {
		uInt log2n1, log2n2, log2n3;
		uInt log2n1mult, log2n2mult, log2n3mult;

		pow2Mapping_ = (isPow2(n1, log2n1)
		                && isPow2(n2, log2n2)
		                && isPow2(n3, log2n3)
		                && isPow2(n1mult, log2n1mult)
		                && isPow2(n2mult, log2n2mult)
		                && isPow2(n3mult, log2n3mult));

		n1_ = n1;
		n2_ = n2;
		n3_ = n3;
		n1Mult_ = n1mult;
		n2Mult_ = n2mult;
		n3Mult_ = n3mult;

		if (pow2Mapping_) {
			n1Shift_ = log2n1mult;
			n2Shift_ = log2n2mult;
			n3Shift_ = log2n3mult;
		}

		// If HW color cells have already been allocated some may never get
		// deallocated.
		uInt newnColors = n1_*n2_*n3_;
		if(!readOnly() && (newnColors < nColors_)) {
			LogIO os;
			os << LogIO::WARN << LogOrigin("GLPixelCanvasColorTable:",
			                               "setupColorCube", WHERE)
			   << "Losing " << nColors_ - n1_*n2_*n3_ << " colors from HW map"
			   << LogIO::POST;
		}

		nColors_ = n1_*n2_*n3_;
	}

// Converts data between
// Inputs:  Three Float arrays representing each of the three channels
//          representing color triples in the range of <0,0,0> to <1,1,1>
//          Color model of inputs
// Outputs: float arrays representing values from <0,0,0> to <1,1,1> in
//          the outputColorModel_ associated with the X11PCColorTable
	Bool GLPixelCanvasColorTable::colorSpaceMap(Display::ColorModel inputColorModel,
	        const Array<Float> & chan1in,
	        const Array<Float> & chan2in,
	        const Array<Float> & chan3in,
	        Array<Float> & chan1out,
	        Array<Float> & chan2out,
	        Array<Float> & chan3out) {
		Bool ok = False;

		// [ ] CONFORM TEST

		switch(inputColorModel) {
		case Display::RGB:
			switch(colorModel_) {
			case Display::RGB:
				chan1out = chan1in;
				chan2out = chan2in;
				chan3out = chan3in;
				ok = True;
				break;
			case Display::HSV:
				rgbToHsv(chan1in, chan2in, chan3in,
				         chan1out, chan2out, chan3out);
				ok = True;
				break;
			case Display::Index:
				// won't get here
				break;
			}
			break;
		case Display::HSV:
			switch(colorModel_) {
			case Display::RGB:
				hsvToRgb(chan1in, chan2in, chan3in,
				         chan1out, chan2out, chan3out);
				ok = True;
				break;
			case Display::HSV:
				chan1out = chan1in;
				chan2out = chan2in;
				chan3out = chan3in;
				ok = True;
				break;
			case Display::Index:
				// won't get here
				break;
			}
			break;
		case Display::Index:
			// won't get here
			break;
		}
		return ok;
	}

// map [0-1] float data to <[0-(n1_-1)], [0-(n2_-1)], [0-(n3_-1)]>
	void GLPixelCanvasColorTable::mapToColor3(Array<uLong> & outImage,
	        const Array<Float> & chan1in,
	        const Array<Float> & chan2in,
	        const Array<Float> & chan3in) {
		if (colorModel_ == Display::Index)
			throw(AipsError("Should not get to GLPixelCanvasColorTable::mapToColor3 in Index mode"));

		Bool ch1Del;
		const Float * ch1p = chan1in.getStorage(ch1Del);
		Bool ch2Del;
		const Float * ch2p = chan2in.getStorage(ch2Del);
		Bool ch3Del;
		const Float * ch3p = chan3in.getStorage(ch3Del);

		Bool outDel;
		uLong * outp = outImage.getStorage(outDel);

		uLong * endp = outp + outImage.nelements();

		const Float * ch1q = ch1p;
		const Float * ch2q = ch2p;
		const Float * ch3q = ch3p;

		uLong * outq = outp;

		uLong t1, t2, t3;

		if (pow2Mapping_) {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					// Use shifts/adds to get color index, use table lookup to get
					// pixel value.
					t1 = (uLong)((*ch1q++)*n1_);
					if (t1 >= n1_) t1 = n1_-1;
					t2 = (uLong)((*ch2q++)*n2_);
					if (t2 >= n2_) t2 = n2_-1;
					t3 = (uLong)((*ch3q++)*n3_);
					if (t3 >= n3_) t3 = n3_-1;
					*outq++ = colors_[(t1 << n1Shift_) | (t2 << n2Shift_) | (t3 << n3Shift_)];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (uLong)(*ch1q++ * red_max_);
						if(red > red_max_) red = red_max_;
						red = (red << red_shift_)&red_mask_;
						uLong green = (uLong)(*ch2q++ * green_max_);
						if(green > green_max_) green = green_max_;
						green = (green << green_shift_)&green_mask_;
						uLong blue = (uLong)(*ch3q++ * blue_max_);
						if(blue > blue_max_) blue = blue_max_;
						blue = (blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ = pixel;
					}
				else	// HSV
					while (outq < endp) {
						Float r, g, b;
						// Convert HSV -> RGB.
						hsvToRgb(*ch1q++, *ch2q++, *ch3q++, r, g, b);
						uLong Red = (uLong)(r*red_max_);
						uLong red = (Red << red_shift_)&red_mask_;
						uLong Green = (uLong)(g*green_max_);
						uLong green =
						    (Green << green_shift_)&green_mask_;
						uLong Blue = (uLong)(b*blue_max_);
						uLong blue = (Blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		} else {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					// Cube not aligned to bitplane boundaries, so have to
					// multiply/and add to get color index, use table lookup to get
					// pixel value.
					t1 = (uLong)((*ch1q++)*n1_);
					if (t1 >= n1_) t1 = n1_-1;
					t2 = (uLong)((*ch2q++)*n2_);
					if (t2 >= n2_) t2 = n2_-1;
					t3 = (uLong)((*ch3q++)*n3_);
					if (t3 >= n3_) t3 = n3_-1;
					*outq++ = colors_[t1*n1Mult_ + t2*n2Mult_ + t3*n3Mult_];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (uLong)(*ch1q++ * red_max_);
						if(red > red_max_) red = red_max_;
						red = (red << red_shift_)&red_mask_;
						uLong green = (uLong)(*ch2q++ * green_max_);
						if(green > green_max_) green = green_max_;
						green = (green << green_shift_)&green_mask_;
						uLong blue = (uLong)(*ch3q++ * blue_max_);
						if(blue > blue_max_) blue = blue_max_;
						blue = (blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ = pixel;
					}
				else	// HSV
					while (outq < endp) {
						Float r, g, b;
						// Convert HSV -> RGB.
						hsvToRgb(*ch1q++, *ch2q++, *ch3q++, r, g, b);
						uLong Red = (uLong)(r*red_max_);
						uLong red = (Red << red_shift_)&red_mask_;
						uLong Green = (uLong)(g*green_max_);
						uLong green =
						    (Green << green_shift_)&green_mask_;
						uLong Blue = (uLong)(b*blue_max_);
						uLong blue = (Blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		}


		chan1in.freeStorage(ch1p, ch1Del);
		chan2in.freeStorage(ch2p, ch2Del);
		chan3in.freeStorage(ch3p, ch3Del);

		outImage.putStorage(outp, outDel);
	}

// map [0-1] float data to <[0-(n1_-1)], [0-(n2_-1)], [0-(n3_-1)]>
	void GLPixelCanvasColorTable::mapToColor3(Array<uLong> & outImage,
	        const Array<Double> & chan1in,
	        const Array<Double> & chan2in,
	        const Array<Double> & chan3in) {
		if (colorModel_ == Display::Index)
			throw(AipsError("Should not get to GLPixelCanvasColorTable::mapToColor3 in Index mode"));

		Bool ch1Del;
		const Double * ch1p = chan1in.getStorage(ch1Del);
		Bool ch2Del;
		const Double * ch2p = chan2in.getStorage(ch2Del);
		Bool ch3Del;
		const Double * ch3p = chan3in.getStorage(ch3Del);

		Bool outDel;
		uLong * outp = outImage.getStorage(outDel);

		uLong * endp = outp + outImage.nelements();

		const Double * ch1q = ch1p;
		const Double * ch2q = ch2p;
		const Double * ch3q = ch3p;

		uLong * outq = outp;

		uLong t1, t2, t3;

		if (pow2Mapping_) {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					// Use shifts/adds to get color index, use table lookup to get
					// pixel value.
					t1 = (uLong)((*ch1q++)*n1_);
					if (t1 >= n1_) t1 = n1_-1;
					t2 = (uLong)((*ch2q++)*n2_);
					if (t2 >= n2_) t2 = n2_-1;
					t3 = (uLong)((*ch3q++)*n3_);
					if (t3 >= n3_) t3 = n3_-1;
					*outq++ = colors_[(t1 << n1Shift_) | (t2 << n2Shift_) | (t3 << n3Shift_)];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (uLong)(*ch1q++ * red_max_);
						if(red > red_max_) red = red_max_;
						red = (red << red_shift_)&red_mask_;
						uLong green = (uLong)(*ch2q++ * green_max_);
						if(green > green_max_) green = green_max_;
						green = (green << green_shift_)&green_mask_;
						uLong blue = (uLong)(*ch3q++ * blue_max_);
						if(blue > blue_max_) blue = blue_max_;
						blue = (blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ = pixel;
					}
				else	// HSV
					while (outq < endp) {
						Float r, g, b;
						// Convert HSV -> RGB.
						hsvToRgb(*ch1q++, *ch2q++, *ch3q++, r, g, b);
						uLong Red = (uLong)(r*red_max_);
						uLong red = (Red << red_shift_)&red_mask_;
						uLong Green = (uLong)(g*green_max_);
						uLong green = (Green << green_shift_)&green_mask_;
						uLong Blue = (uLong)(b*blue_max_);
						uLong blue = (Blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		} else {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					// Cube not aligned to bitplane boundaries, so have to
					// multiply/and add to get color index, use table lookup to get
					// pixel value.
					t1 = (uLong)((*ch1q++)*n1_);
					if (t1 >= n1_) t1 = n1_-1;
					t2 = (uLong)((*ch2q++)*n2_);
					if (t2 >= n2_) t2 = n2_-1;
					t3 = (uLong)((*ch3q++)*n3_);
					if (t3 >= n3_) t3 = n3_-1;
					*outq++ = colors_[t1*n1Mult_ + t2*n2Mult_ + t3*n3Mult_];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (uLong)(*ch1q++ * red_max_);
						if(red > red_max_) red = red_max_;
						red = (red << red_shift_)&red_mask_;
						uLong green = (uLong)(*ch2q++ * green_max_);
						if(green > green_max_) green = green_max_;
						green = (green << green_shift_)&green_mask_;
						uLong blue = (uLong)(*ch3q++ * blue_max_);
						if(blue > blue_max_) blue = blue_max_;
						blue = (blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ = pixel;
					}
				else	// HSV
					while (outq < endp) {
						Float r, g, b;
						// Convert HSV -> RGB.
						hsvToRgb(*ch1q++, *ch2q++, *ch3q++, r, g, b);
						uLong Red = (uLong)(r*red_max_);
						uLong red = (Red << red_shift_)&red_mask_;
						uLong Green = (uLong)(g*green_max_);
						uLong green = (Green << green_shift_)&green_mask_;
						uLong Blue = (uLong)(b*blue_max_);
						uLong blue = (Blue << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		}

		chan1in.freeStorage(ch1p, ch1Del);
		chan2in.freeStorage(ch2p, ch2Del);
		chan3in.freeStorage(ch3p, ch3Del);

		outImage.putStorage(outp, outDel);
	}

// merge separate channels into an output image.  It is assumed the
// values in the channel images are within the ranges per channel
	void GLPixelCanvasColorTable::mapToColor3(Array<uLong> & outImage,
	        const Array<uShort> & chan1in,
	        const Array<uShort> & chan2in,
	        const Array<uShort> & chan3in) {
		if (colorModel_ == Display::Index)
			throw(AipsError("Should not get to GLPixelCanvasColorTable::mapToColor3 in Index mode"));

		Bool ch1Del;
		const uShort * ch1p = chan1in.getStorage(ch1Del);
		Bool ch2Del;
		const uShort * ch2p = chan2in.getStorage(ch2Del);
		Bool ch3Del;
		const uShort * ch3p = chan3in.getStorage(ch3Del);

		Bool outDel;
		uLong * outp = outImage.getStorage(outDel);

		uLong * endp = outp + outImage.nelements();

		const uShort * ch1q = ch1p;
		const uShort * ch2q = ch2p;
		const uShort * ch3q = ch3p;

		uLong * outq = outp;

		if (pow2Mapping_) {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					*outq++ = colors_[((*ch1q++) << n1Shift_) | ((*ch2q++) << n2Shift_) | ((*ch3q++) << n3Shift_)];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (*ch1q++ << red_shift_)&red_mask_;
						uLong green = (*ch2q++ << green_shift_)&green_mask_;
						uLong blue = (*ch3q++ << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
				else	// HSV
					while (outq < endp) {
						uLong r, g, b;
						// Convert HSV -> RGB.
						HSV2RGB((uLong)*ch1q++, (uLong)*ch2q++, (uLong)*ch3q++,
						        r, g, b);
						uLong red = (r << red_shift_)&red_mask_;
						uLong green = (g << green_shift_)&green_mask_;
						uLong blue = (b << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		} else {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					*outq++ = colors_[(*ch1q++)*n1Mult_ + (*ch2q++)*n2Mult_ + (*ch3q++)*n3Mult_];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (*ch1q++ << red_shift_)&red_mask_;
						uLong green = (*ch2q++ << green_shift_)&green_mask_;
						uLong blue = (*ch3q++ << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
				else	// HSV
					while (outq < endp) {
						uLong r, g, b;
						// Convert HSV -> RGB.
						HSV2RGB((uLong)*ch1q++, (uLong)*ch2q++, (uLong)*ch3q++,
						        r, g, b);
						uLong red = (r << red_shift_)&red_mask_;
						uLong green = (g << green_shift_)&green_mask_;
						uLong blue = (b << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		}

		chan1in.freeStorage(ch1p, ch1Del);
		chan2in.freeStorage(ch2p, ch2Del);
		chan3in.freeStorage(ch3p, ch3Del);

		outImage.putStorage(outp, outDel);
	}

	void
	GLPixelCanvasColorTable::HSV2RGB(const uLong H, const uLong S, const uLong V,
	                                 uLong &R, uLong &G, uLong &B) {
		float h, s, v, r, g, b;
		// Convert to floats in range of [0..1].
		h = ((float)H)/red_max_;
		s = ((float)S)/green_max_;
		v = ((float)V)/blue_max_;
		hsvToRgb(h, s, v, r, g, b);
		R = (uLong)(r*red_max_);
		G = (uLong)(g*green_max_);
		B = (uLong)(b*blue_max_);
	}

// merge separate channels into an output image.  It is assumed the
// values in the channel images are within the ranges per channel
	void GLPixelCanvasColorTable::mapToColor3(Array<uLong> & outImage,
	        const Array<uInt> & chan1in,
	        const Array<uInt> & chan2in,
	        const Array<uInt> & chan3in) {
		if (colorModel_ == Display::Index)
			throw(AipsError("Should not get to GLPixelCanvasColorTable::mapToColor3 in Index mode"));

		Bool ch1Del;
		const uInt * ch1p = chan1in.getStorage(ch1Del);
		Bool ch2Del;
		const uInt * ch2p = chan2in.getStorage(ch2Del);
		Bool ch3Del;
		const uInt * ch3p = chan3in.getStorage(ch3Del);

		Bool outDel;
		uLong * outp = outImage.getStorage(outDel);

		uLong * endp = outp + outImage.nelements();

		const uInt * ch1q = ch1p;
		const uInt * ch2q = ch2p;
		const uInt * ch3q = ch3p;

		uLong * outq = outp;

		if (pow2Mapping_) {
			if( !decomposedIndex()) {
				while (outq < endp) {
					*outq++ = colors_[((*ch1q++) << n1Shift_) | ((*ch2q++) << n2Shift_) | ((*ch3q++) << n3Shift_)];
				}
			} else {	// TrueColor, DirectColor.
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (*ch1q++ << red_shift_)&red_mask_;
						uLong green = (*ch2q++ << green_shift_)&green_mask_;
						uLong blue = (*ch3q++ << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
				else	// HSV
					while (outq < endp) {
						uLong r, g, b;
						// Convert HSV -> RGB.
						HSV2RGB((uLong)*ch1q++, (uLong)*ch2q++, (uLong)*ch3q++,
						        r, g, b);
						uLong red = (r << red_shift_)&red_mask_;
						uLong green = (g << green_shift_)&green_mask_;
						uLong blue = (b << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		} else {
			if (visual_->c_class == PseudoColor || visual_->c_class == StaticColor) {
				while (outq < endp) {
					*outq++ = colors_[(*ch1q++)*n1Mult_ + (*ch2q++)*n2Mult_ + (*ch3q++)*n3Mult_];
				}
			} else {
				if(colorModel_ == Display::RGB)
					while (outq < endp) {
						uLong red = (*ch1q++ << red_shift_)&red_mask_;
						uLong green = (*ch2q++ << green_shift_)&green_mask_;
						uLong blue = (*ch3q++ << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
				else	// HSV
					while (outq < endp) {
						uLong r, g, b;
						// Convert HSV -> RGB.
						HSV2RGB((uLong)*ch1q++, (uLong)*ch2q++, (uLong)*ch3q++,
						        r, g, b);
						uLong red = (r << red_shift_)&red_mask_;
						uLong green = (g << green_shift_)&green_mask_;
						uLong blue = (b << blue_shift_)&blue_mask_;
						uLong pixel = red | green | blue;
						*outq++ =  pixel;
					}
			}
		}

		chan1in.freeStorage(ch1p, ch1Del);
		chan2in.freeStorage(ch2p, ch2Del);
		chan3in.freeStorage(ch3p, ch3Del);

		outImage.putStorage(outp, outDel);
	}

	void GLPixelCanvasColorTable::setupStandardMapping(const XStandardColormap * mapInfo) {
		setupColorCube(mapInfo->red_max+1,
		               mapInfo->green_max+1,
		               mapInfo->blue_max+1,
		               mapInfo->red_mult,
		               mapInfo->green_mult,
		               mapInfo->blue_mult);
		baseColor_ = mapInfo->base_pixel;
		xcmap_ = mapInfo->colormap;
	}

//-------------------------------------------------------------------------

	ostream & operator << (ostream & os, const GLPixelCanvasColorTable & pcc) {
		os << "--------------------- GLPixelCanvasColorTable -----------------\n";
		//os << "Display  : " << pcc.display_ << endl;
		//os << "Screen   : " << pcc.screen_ << endl;
		os << "Visual   : " << pcc.visual_ << endl;
		os << "XColormap : " << pcc.xcmap_ << endl;
		os << "Depth    : " << pcc.depth_ << endl;
		os << "nColors  : " << pcc.nColors_ << endl;
		os << "Rigid    : " << (pcc.rigid_ ? "Yes" : "No") << endl;

		os << "color model      : " << pcc.colorModel_ << endl;
		os << "pow2Mapping      : " << pcc.pow2Mapping_ << endl;
		os << "baseColor        : " << pcc.baseColor_ << endl;
		os << "Color Cube -" << endl;
		os << "  resolution    : <" << pcc.n1_ << "," << pcc.n2_ << "," << pcc.n3_ << ">\n";
		os << "  multipliers   : <" << pcc.n1Mult_ << "," << pcc.n2Mult_ << "," << pcc.n3Mult_ << ">\n";
		os << "  shift         : <" << pcc.n1Shift_ << "," << pcc.n2Shift_ << "," << pcc.n3Shift_ << ">\n";
		os << "------------------- END GLPixelCanvasColorTable ---------------\n";
		return os;
	}

	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uChar> & outArray,
	        const Array<uChar> & inArray,
	        Bool rangeCheck) const {
		Bool inDel;
		const uChar * inp = inArray.getStorage(inDel);

		uInt offset = getColormapOffset(map);
		uInt cmapsize = getColormapSize(map);
		uChar maxc   = offset + cmapsize - 1;

		Bool outDel;
		uChar * outp = outArray.getStorage(outDel);
		uChar * endp = outp + outArray.nelements();

		const uChar * inq = inp;
		uChar * outq = outp;

		if (rangeCheck) {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = (*inq >= cmapsize) ? maxc : offset + *inq;
				if(virtualToPhysical(vIndex, pIndex))
					*outq++ = (uChar)pIndex;
				else
					*outq++ = 0;	// Shouldn't happen.
				inq++;
			}
		} else {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = offset + *inq++;
				virtualToPhysical(vIndex, pIndex);
				*outq++ = (uChar)pIndex;
			}
		}

		inArray.freeStorage(inp, inDel);
		outArray.putStorage(outp, outDel);
	}

	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uShort> & outArray,
	        const Array<uShort> & inArray,
	        Bool rangeCheck) const {
		Bool inDel;
		const uShort * inp = inArray.getStorage(inDel);

		uInt offset = getColormapOffset(map);
		uInt cmapsize = getColormapSize(map);
		uShort maxc   = offset + cmapsize - 1;

		Bool outDel;
		uShort * outp = outArray.getStorage(outDel);
		uShort * endp = outp + outArray.nelements();

		const uShort * inq = inp;
		uShort * outq = outp;

		if (rangeCheck) {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = (*inq >= cmapsize) ? maxc : offset + *inq;
				if(virtualToPhysical(vIndex, pIndex))
					*outq++ = (uShort)pIndex;
				else
					*outq++ = 0;	// Shouldn't happen.
				inq++;
			}
		} else {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = offset + *inq++;
				virtualToPhysical(vIndex, pIndex);
				*outq++ = (uShort)pIndex;
			}
		}

		inArray.freeStorage(inp, inDel);
		outArray.putStorage(outp, outDel);
	}

	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uInt> & outArray,
	        const Array<uInt> & inArray,
	        Bool rangeCheck) const {
		Bool inDel;
		const uInt * inp = inArray.getStorage(inDel);

		Bool outDel;
		uInt * outp = outArray.getStorage(outDel);
		uInt * endp = outp + outArray.nelements();

		uInt offset = getColormapOffset(map);
		uInt cmapsize = getColormapSize(map);
		uInt maxc   = offset + cmapsize - 1;

		const uInt * inq = inp;
		uInt * outq = outp;

		if (rangeCheck) {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = (*inq >= cmapsize) ? maxc : offset + *inq;
				if(virtualToPhysical(vIndex, pIndex))
					*outq++ = (uInt)pIndex;
				else
					*outq++ = 0;	// Shouldn't happen.
				inq++;
			}
		} else {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = offset + *inq++;
				virtualToPhysical(vIndex, pIndex);
				*outq++ = (uInt)pIndex;
			}
		}

		inArray.freeStorage(inp, inDel);
		outArray.putStorage(outp, outDel);
	}

	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uLong> & outArray,
	        const Array<uLong> & inArray,
	        Bool rangeCheck) const {
		Bool inDel;
		const uLong * inp = inArray.getStorage(inDel);

		uInt offset = getColormapOffset(map);
		uInt cmapsize = getColormapSize(map);
		uLong maxc   = offset + cmapsize - 1;

		Bool outDel;
		uLong * outp = outArray.getStorage(outDel);
		uLong * endp = outp + outArray.nelements();

		const uLong * inq = inp;
		uLong * outq = outp;

		if (rangeCheck) {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = (*inq >= cmapsize) ? maxc : offset + *inq;
				if(virtualToPhysical(vIndex, pIndex))
					*outq++ = pIndex;
				else
					*outq++ = 0;	// Shouldn't happen.
				inq++;
			}
		} else {
			while( outq < endp) {
				uLong pIndex, vIndex;

				vIndex = offset + *inq++;
				virtualToPhysical(vIndex, pIndex);
				*outq++ = pIndex;
			}
		}

		inArray.freeStorage(inp, inDel);
		outArray.putStorage(outp, outDel);
	}

	template <class T>
	void GLPixelCanvasColorTable_mapToColor(uLong * table,
	                                        uInt tableSize,
	                                        uInt mapOffset,
	                                        Array<T> & inOutArray,
	                                        Bool rangeCheck);

	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uChar> & inOutArray,
	        Bool rangeCheck) const {
		GLPixelCanvasColorTable_mapToColor(colors_, nColors_, getColormapOffset(map),
		                                   inOutArray, rangeCheck);
	}
	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uShort> & inOutArray,
	        Bool rangeCheck) const {
		GLPixelCanvasColorTable_mapToColor(colors_, nColors_, getColormapOffset(map),
		                                   inOutArray, rangeCheck);
	}
	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uInt> & inOutArray,
	        Bool rangeCheck) const {
		GLPixelCanvasColorTable_mapToColor(colors_, nColors_, getColormapOffset(map),
		                                   inOutArray, rangeCheck);
	}
	void GLPixelCanvasColorTable::mapToColor(const Colormap * map,
	        Array<uLong> & inOutArray,
	        Bool rangeCheck) const {
		GLPixelCanvasColorTable_mapToColor(colors_, nColors_, getColormapOffset(map),
		                                   inOutArray, rangeCheck);
	}

	GLVColorTableEntry::GLVColorTableEntry() {
		red_ = green_ = blue_ = 0.0;
		index_ = 0;
		pixel_ = 0;
	}

	void GLVColorTableEntry::operator=(const GLVColorTableEntry &e) {
		index_ = e.index_;
		pixel_ = e.pixel_;
		red_ = e.red_;
		green_ = e.green_;
		blue_ = e.blue_;
	}

	ostream &operator<<(ostream &s, const GLVColorTableEntry &e) {
		s << " Index = " << e.getIndex();
		s << " Pixel = " << e.getPixel();
		float r, g, b;
		e.get(r, g, b);
		s << " RGB = " << r << "/" << g << "/" << b;
		return s;
	}

// # of colors that can still be allocated RW.
	uInt GLPixelCanvasColorTable::QueryColorsAvailable(const Bool contig)const {
		if(readOnly())
			return (uInt)vcmapLength_ - nColors_;
		else
			return QueryHWColorsAvailable(contig);
	}

	uInt GLPixelCanvasColorTable::QueryHWColorsAvailable(const Bool contig)const {
		return X11QueryColorsAvailable(display_, xcmap_, contig);
	}

// Convert a virtual index to a physical pixel. Valid range is 0 to nColors_ -1.
// Returns False if vindex is out of range. Otherwise, True.
	Bool GLPixelCanvasColorTable::virtualToPhysical(const uLong vindex,
	        uLong &pindex)const {
		if(vindex >= nColors_)
			return False;
		else {
			int index = colors_[vindex];
			pindex = vcmap_[index].getPixel();
			return True;
		}
	}

// Convert a pixel to color components. If decomposed index (eg TC.)
// the pixel contains the color information. Otherwise (eg. PseudoColor),
// The information is looked up in the virtual colormap.
	void GLPixelCanvasColorTable::pixelToComponents(const uLong pixel,
	        Float &r, Float &g, Float &b) {
		if(decomposedIndex_) {
			r = (float)((pixel >> red_shift_) & red_mask_)/red_max_;
			g = (float)((pixel >> green_shift_) & green_mask_)/green_max_;
			b = (float)((pixel >> blue_shift_) & blue_mask_)/blue_max_;
		} else {
			if(pixel >= vcmapLength_)
				return;
			vcmap_[pixel].get(r, g, b);
		}
	}

// Try to get a visual that uses the requested colormodel. If there
// isn't one, get the other kind and we'll fake it.
	static XVisualInfo *tryForRGBVisual(::XDisplay *xdpy) {
		Display *dpy = (Display *) xdpy;
		XVisualInfo *vi;
		int screennum = DefaultScreen(dpy);
		static int attributes[] = {GLX_DOUBLEBUFFER, GLX_RGBA,
		                           // Chooses the largest buffer sizes >= the given number.
		                           GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
		                           None
		                          };


		// First try double buffer, then single buffer.
		if((vi = glXChooseVisual(dpy, screennum, attributes)) != NULL)
			return vi;

		// The first entry in attributes is GLX_DOUBLEBUFFER.
		return glXChooseVisual(dpy, screennum, attributes+1);
	}

// Try for an index visual.
// First try all sizes with double buffering, then try again with single.
	static XVisualInfo *tryForIndexedVisual(::XDisplay *xdpy) {
		XVisualInfo *vi;
		Display *dpy = (Display *) xdpy;
		int screennum = DefaultScreen(dpy);
		int attributes[] = {GLX_DOUBLEBUFFER, GLX_BUFFER_SIZE, 0, None};
		static const int BSINDEX = sizeof(attributes)/sizeof(*attributes)-2;
		static int buffer_sizes[] = {12, 10, 8, 4};
		static const int NSIZES = sizeof(buffer_sizes)/sizeof(*buffer_sizes);

		// Try double buffer, then single buffer.
		for(int offset = 0; offset < 2; offset++) {
			// Try each colormap size.
			for(int bs = 0; bs < NSIZES; bs++) {
				attributes[BSINDEX] = buffer_sizes[bs];
				vi = glXChooseVisual(dpy, screennum,
				                     attributes+offset);
				if(vi != NULL)
					return vi;
			}
		}
		return NULL;
	}

	XVisualInfo *GLPixelCanvasColorTable::getVisualInfo(::XDisplay *xdpy,
	        const Display::ColorModel colormodel) {
		XVisualInfo *vi = NULL;

		switch(colormodel) {
		case Display::Index:
			if((vi = tryForIndexedVisual(xdpy)) == NULL)
				vi = tryForRGBVisual(xdpy);
			break;
		case Display::RGB:
		case Display::HSV:
			if((vi = tryForRGBVisual(xdpy)) == NULL)
				vi = tryForIndexedVisual(xdpy);
		default:
			break;
		}

		return vi;
	}

} //# NAMESPACE CASA - END

#endif // OGL
