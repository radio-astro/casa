//# PixelCanvasColorTable.h: abstraction of color resources/allocation
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_PIXELCANVASCOLORTABLE_H
#define TRIALDISPLAY_PIXELCANVASCOLORTABLE_H

#include <casa/Arrays/Vector.h>
#include <casa/Containers/List.h>
#include <display/Display/Colormap.h>
#include <display/Display/ColormapManager.h>
#include <display/Display/DisplayEnums.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	typedef void (*PixelCanvasColorTableResizeCB)(class PixelCanvasColorTable * pccmap, casacore::uInt newSize, void * clientData, Display::RefreshReason reason);

// <summary>
// Abstract interface to underlying graphics library's colortable
// </summary>
//
// <prerequisite>
// <li> Knowledge of hardware Colormaps
// <li> <linkto class="Colormap">Colormap</linkto>
// <li> <linkto class="ColormapManager">ColormapManager</linkto>
// </prerequisite>
//
// <etymology>
// PixelCanvas table of available colors.  Note that there is still a table
// of colors in RGB mode.
// </etymology>
//
// <synopsis>
//
// This abstract class is communicated to by the PixelCanvas to perform color
// operations.  The information in this file should not be needed to support
// applications programmers, and is provided for the future maintainer of the
// Display library.
//
// A Major role of the PixelCanvasColorTable is to dynamically allocate space
// out of its own colortable allocation from the underlying graphics library
// for its registered <linkto class="Colormap">Colormap</linkto>s.  It talks to
// its <linkto class="ColormapManager">ColormapManager</linkto> to return the
// <linkto class="Colormap">Colormap</linkto> information needed to properly
// scale data values to color.  An application must know the size of the colormap
// to determine the color resolution available to value-related data.
//
// The mapToColor functions will be needed by most image drawing and colored vector
// display drawing calls to transform quantized values into color indices.
//
// The PixelCanvasColorTable is distinguished from the <linkto class="Colormap">
// Colormap</linkto> by its functionality.  There is one and only one
// PixelCanvasColorTable for each
// <linkto class="PixelCanvas">PixelCanvas</linkto>.
// It controls the <em>allocation</em>
// of containers for colors.  It also will set the colors in the containers if the
// map is in HSV or RGB mode if necessary.
// If the map is in INDEX mode, you can install one or more
// <linkto class = "Colormap">Colormap</linkto>s that control banks
// of colors in the color table because Colormaps define the <em>colors</em>
// that go into the containers of a PixelCanvasColorTable.
//

// </synopsis>
//
// <motivation>
// Needed to abstract the concepts involved in color resource allocation from
// the
// <linkto class="PixelCanvas">PixelCanvas</linkto> user.
// </motivation>
//
// <example>
// see the <linkto class="PixelCanvas">PixelCanvas</linkto> test programs
// </example>
//
// <todo>
// <li> Implement RGB resize
// </todo>
//

	class PixelCanvasColorTable {
	public:

		// Is the hardware colormap resizeable?  ie. is it write-only?
		// Default is to return true, and derived classes should generally
		// override this function.
		virtual casacore::Bool staticSize() {
			return true;
		}

		// Resize the hardware colormap.
		// <group>
		virtual casacore::Bool resize(casacore::uInt newSize) = 0;
		virtual casacore::Bool resize(casacore::uInt nReds, casacore::uInt nGreens, casacore::uInt nBlues) = 0;
		// </group>

		// Install colors into the color table.  Offset is zero-based.  Colors
		// are installed into the PixelCanvasColorTable until the vectors run out
		// or until the end of the colortable is reached.  This has no effect if
		// in real/pseudo RGB/HSV modes.  Values are clamped to [0.0,1.0].
		virtual casacore::Bool installRGBColors(const casacore::Vector<casacore::Float> & r, const casacore::Vector<casacore::Float> & g,
		                              const casacore::Vector<casacore::Float> & b, const casacore::Vector<casacore::Float> & alpha,
		                              casacore::uInt offset = 0) = 0;

		// Return the number of colors used to make the map.
		virtual casacore::uInt nColors() const = 0;

		// Return the number of colors per component used in the map.  Fails
		// for non-HSV/RGB modes.
		virtual void nColors(casacore::uInt &n1, casacore::uInt &n2, casacore::uInt &n3) const = 0;

		// Return the depth of the map in bits
		virtual casacore::uInt depth() const = 0;

		// Return the number of colors that are still unallocated
		virtual casacore::uInt nSpareColors() const = 0;

		// Virtual destructor
		virtual ~PixelCanvasColorTable();

		// Add and remove resize callbacks
		// PixelCanvasColorTableResizeCB is of type:
		// <br>void (*)(class PixelCanvasColorTable * pcctbl, casacore::uInt newSize, void * clientData, Display::RefreshReason reason)
		//
		// <group>
		void addResizeCallback(PixelCanvasColorTableResizeCB cb, void * clientData);
		void removeResizeCallback(PixelCanvasColorTableResizeCB cb, void * clientData);
		// </group>

		// casacore::Function that issues resize callbacks
		void doResizeCallbacks(const Display::RefreshReason
		                       &reason = Display::ColorTableChange);

		// Register a colormap to be managed by the pixel canvas' color table
		void registerColormap(Colormap * cmap, casacore::Float weight = 1.0);

		// Register the <src>cmap</src> Colormap on the
		// PixelCanvasColorTable, replacing the <src>cmapToReplace</src>
		// Colormap if possible.
		void registerColormap(Colormap *cmap, Colormap *cmapToReplace);

		// Unregister a data colormap reference previously added
		void unregisterColormap(Colormap * cmap);

		// Return the allocation size of some Colormap
		casacore::uInt getColormapSize(const Colormap * cmap) const;

		// map [0,N-1] into colorpixels, where N is the current colormap size
		// The values are returned as unsigned integers in their respective
		// array.
		// <note role="tip">The choice of what type to use should be guided by
		// the number of graphics bitplanes available.  For most systems with
		// 8-bit color, casacore::uChar is optimal.  Some systems with 12 bits per pixel
		// with an alpha channel may require using the uLong. </note>
		//
		// <note role="warning">casacore::uChar type may not have enough bits
		// to hold the pixel index on some high-end graphics systems </note>
		// <note role="warning">casacore::uShort type may not have enough bits
		// to hold the pixel index on some high-end graphics systems </note>
		//
		// <group>
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uChar> & outArray,
		                        const casacore::Array<casacore::uChar> & inArray, casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uShort> & outArray,
		                        const casacore::Array<casacore::uShort> & inArray, casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uInt> & outArray,
		                        const casacore::Array<casacore::uInt> & inArray, casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uLong> & outArray,
		                        const casacore::Array<casacore::uLong> & inArray, casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColorRGB(const Colormap* map, casacore::Array<casacore::uInt>& outArray,
						const casacore::Array<casacore::uInt>& inArrayRed, const casacore::Array<casacore::uInt>& inArrayGreen, const casacore::Array<casacore::uInt>& inArrayBlue) const;
		// </group>

		// same as above except the matrix is operated on in place.  Only unsigned
		// values make sense here.  I don't really know what to include here.  Maybe
		// ask the code cop.
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uChar> & inOutArray,
		                        casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uShort> & inOutArray,
		                        casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uInt> & inOutArray,
		                        casacore::Bool rangeCheck = true) const = 0;
		virtual void mapToColor(const Colormap * map, casacore::Array<casacore::uLong> & inOutArray,
		                        casacore::Bool rangeCheck = true) const = 0;

		// Functions for dealing with multi-channel mapping
		// <group>
		// (Multichannel Color)
		// Merge separate channel data into an output image.
		// This function maps floating values between 0 and 1
		// into a output image suitable for PixelCanvas::drawImage().
		virtual void mapToColor3(casacore::Array<casacore::uLong> & out,
		                         const casacore::Array<casacore::Float> & chan1in,
		                         const casacore::Array<casacore::Float> & chan2in,
		                         const casacore::Array<casacore::Float> & chan3in) = 0;
		virtual void mapToColor3(casacore::Array<casacore::uLong> & out,
		                         const casacore::Array<casacore::Double> & chan1in,
		                         const casacore::Array<casacore::Double> & chan2in,
		                         const casacore::Array<casacore::Double> & chan3in) = 0;
		// </group>

		// This one maps values between 0 and the integer
		// maximum value for each channel into a single
		// output image suitable for PixelCanvas::drawImage().
		// <group>
		virtual void mapToColor3(casacore::Array<casacore::uLong> & out,
		                         const casacore::Array<casacore::uShort> & chan1in,
		                         const casacore::Array<casacore::uShort> & chan2in,
		                         const casacore::Array<casacore::uShort> & chan3in) = 0;
		virtual void mapToColor3(casacore::Array<casacore::uLong> & out,
		                         const casacore::Array<casacore::uInt> & chan1in,
		                         const casacore::Array<casacore::uInt> & chan2in,
		                         const casacore::Array<casacore::uInt> & chan3in) = 0;
		// </group>

		// (Multichannel Color)
		// Transform arrays from the passed color model into
		// the colormodel of the XPCCT.
		// Does nothing if colorModel is Display::Index.
		// It is assumed that input arrays are in the range of [0,1]
		virtual casacore::Bool colorSpaceMap(Display::ColorModel,
		                           const casacore::Array<casacore::Float> & chan1in,
		                           const casacore::Array<casacore::Float> & chan2in,
		                           const casacore::Array<casacore::Float> & chan3in,
		                           casacore::Array<casacore::Float> & chan1out,
		                           casacore::Array<casacore::Float> & chan2out,
		                           casacore::Array<casacore::Float> & chan3out) = 0;

		// Return whether or not a data colormap is used by this pixel canvas
		casacore::Bool member(const Colormap * cmap) const;

		// Return the default map.  This map is only used if no other colormaps
		// are registered AND if we're in INDEX mode.
		// Colormap * defaultColormap() const { return defaultColormap_; }

		// Set the default colormap
		void setDefaultColormap(const Colormap * map);

		// Return the colormapManager used by this PCCT
		ColormapManager & colormapManager() {
			return dcmapMgr_;
		}

		// Return the color model for multichannel color
		virtual Display::ColorModel colorModel() const = 0;

		/*
		// register a pixel canvas on this pcct
		void registerPixelCanvas(const class PixelCanvas * pc);
		void doRefreshCallbacks();
		*/

		Colormap* defaultColormap() {
			return defaultColormap_;
		}

	protected:

		// return the offset of a particular colormap.  This information is not
		// made available to the outside caller.  Rather a function to map
		// values to color is made available.
		casacore::uInt getColormapOffset(const Colormap * map) const {
			return dcmapMgr_.getColormapOffset(map);
		}

		// Abstract base class
		PixelCanvasColorTable();

	private:
		casacore::uInt getColorAmount( const casacore::uInt* posMatrix, const casacore::uInt* endMatrix,
					int shiftAmount, int colorCount )const;
		// Pointer to the default colormap
		Colormap* defaultColormap_;

		// The colormap manager
		ColormapManager dcmapMgr_;

		// casacore::List of resize callbacks
		casacore::List<void *> resizeCBList_;
		// casacore::List of client data for resize callbacks
		casacore::List<void *> clientDataList_;
		// casacore::List of pixelCanvases on this color table
		casacore::List<void *> pixelCanvasList_;
	};


} //# NAMESPACE CASA - END

#endif
