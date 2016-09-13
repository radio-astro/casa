//# ColormapManager.h: dynamic mapping of Colormaps onto ColorTables
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

#ifndef TRIALDISPLAY_COLORMAPMANAGER_H
#define TRIALDISPLAY_COLORMAPMANAGER_H

#include <casa/aips.h>
#include <casa/Containers/SimOrdMap.h>
#include <display/Display/Colormap.h>

//# Forward declarations
#include <casa/iosfwd.h>
namespace casa { //# NAMESPACE CASA - BEGIN

	class ColormapInfo;
	class PixelCanvasColorTable;

// <summary>
// Class to manage registration of colormaps on PixelCanvasColorTables
// </summary>

// <use visibility=local>

// <reviewed reviewer="None yet" date="yyyy/mm/dd" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class="Colormap">Colormap</linkto>
// <li> <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// </prerequisite>

// <etymology>
// This class manages a set of colormaps and dynamically maintains
// both their allocation on the colortable and the colortable's
// colors.
// </etymology>

// <synopsis>

// The colormap manager is a management tool which facilitates the
// dynamic mapping of 1 or more Colormaps to a single
// PixelCanvasColorTable.
//
// The colormap manager manages the assignment of data colormaps,
// which define the function that generates the color spectrum to use,
// to the hardware colormap which defines how many cells are
// available.  As Colormaps are registerd and unregistered, the
// manager reorganizes the colors on the colormap and reinstalls the
// maps in response.  The colormap manager also registers a resize
// callback with the pixel canvas colormap.  This allows the
// application to resize the hardware colormap and have all colormaps
// adjusted as appropriate.
//
// The calling program can at any time ask for the size of a given
// colormap and use that size to map real-world values into a range
// appropriate for display (see the <linkto
// class="PixelCanvas">PixelCanvas</linkto> class' getColormapSize()
// and mapToColor() functions.
//
// Because the colormap manager can control the distribution of its
// Colormaps, application code must issue a call to redistribute if it
// decides to change the size of the colortable on its own.
// </synopsis>

// <motivation>
// Needed to have tool available to help manage more than one colormap.
// </motivation>

	class ColormapManager {

	public:

		// Constructor requires pointer to the PixelCanvasColorTable which it
		// will manage.
		ColormapManager(PixelCanvasColorTable *pcctbl);

		// Destructor.
		virtual ~ColormapManager();

		// Register a Colormap with this ColormapManager, and optionally
		// pass a weight - colormaps are distributed proportionally
		// according to their weight.
		void registerColormap(Colormap * dcmap, Float weight = 1.0);

		// Register the <src>cmap</src> Colormap with this manager,
		// replacing the <src>cmapToReplace</src> Colormap if possible.
		void registerColormap(Colormap *cmap, Colormap *cmapToReplace);

		// Unregister a Colormap with this ColormapManager.
		Bool unregisterColormap(Colormap * dcmap);

		// Return the current size of the colormap.
		uInt getColormapSize(const Colormap * map) const;

		// Return the current offset of the colormap.
		uInt getColormapOffset(const Colormap * map) const;

		// Redistribute the available colorcells to the registered
		// colormaps.
		void redistributeColormaps();

		// Reinstall the colorcell values.
		void reinstallColormaps();

		// Is the given Colormap registered on this
		// ColormapManager/PixelCanvasColorTable combination?
		Bool member(const Colormap * map) const;

		// Return the number of registered Colormaps.
		uInt nMaps() const {
			return itsInfoMap.ndefined();
		}

		// Return a pointer to a Colormap by number.
		const Colormap *getMap(const uInt mapnum) const;

		// Stream output operator.
		friend ostream & operator << (ostream & os, const ColormapManager & cm);

	private:

		// Pointer to managed PixelCanvasColorTable.
		PixelCanvasColorTable * itsPCColorTable;

		// Map which associates Colormap pointers with ColormapInfo.
		SimpleOrderedMap<const Colormap *, ColormapInfo *> itsInfoMap;

	};


} //# NAMESPACE CASA - END

#endif
