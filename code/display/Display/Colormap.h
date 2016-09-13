//# Colormap.h: generating and selecting colors from a look-up map
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2002,2005
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

#ifndef TRIALDISPLAY_COLORMAP_H
#define TRIALDISPLAY_COLORMAP_H

//# Includes

#include <casa/aips.h>
#include <casa/Containers/BlockIO.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward declarations

	template <class T> class Vector;
	class ColormapDefinition;
	class PixelCanvasColorTable;
	template <class T, class U> class Function1D;
	class LogIO;

// <summary>
// Describes a method of generating a table of colors.
// </summary>
//
// <etymology>
// The name of Colormap comes from ...
// </etymology>
//
// <synopsis>
// A Colormap is a class which is capable of generating a table
// of colormaps for the ColormapManager.  The Colormap's duties
// are to fill a Vector of colors of a specific size.
//
// A Colormap can operate in two modes:
// <ul>
// <li>Dynamic - The Colormap is treated as a function which
// can be arbitrarily descretized into some number of cells.
// <dd>Static - The Colormap is treated as a rigid table of colors and
// may not be resized by, for example, the ColormapManager.
//
// The Colormap generates colors by composing a ColormapShapeFunc
// with a ColormapDefinition to provide the colors for the
// ColormapManager.
//
// Typically the ColormapDefinition is selected from a menu,
// and the ColormapShapeFunc what is changed by a gui.
//
// The ColormapDefinition and ColormapShapeFunc may be derived from
// to provide specialized colortable treatment.
//
// </synopsis>
//
// <motivation>
// Needed to satisfy many simultaneous wishes:
// <ol>
// <li> Wanted to increase application portability by providing for
//    dynamic resize of the colortable(s) used by the application.
// <li> Needed a way to specialize colormaps (e.g. Ron & Renzo map
//    which requires knowledge of the dataset being viewed).
// <li> Needed a colormap to be sharable across multiple displays.  This
//    implies its existence can vary from colortable to colortable.
// </ol>
// </motivation>
//
// <example>
// see the Display test directory
// </example>
//
// <todo>
// </todo>
//

	class Colormap {

	public:

		// Default Constructor Required
		Colormap();

		// User Constructor
		explicit Colormap(const String& name);

		// Destructor.
		virtual ~Colormap();

		// If rigid is True, the colormap must be installed at a
		// specific size.
		// <group>
		Bool rigid() const {
			return itsIsRigid;
		}
		void setRigid(Bool b) {
			itsIsRigid = b;
		}
		// </group>

		// What is the size enforced by the rigid requirement?
		// <group>
		uInt rigidSize()
		const {
			return itsRigidSize;
		}
		void setRigidSize(uInt s) {
			itsRigidSize = s;
		}
		// </group>

		// Compute RGB values using the definition and shape function
		virtual Bool calcRGBMaps(uInt reqSize,
		                         Vector<Float> & redMap,
		                         Vector<Float> & greenMap,
		                         Vector<Float> & blueMap,
		                         Vector<Float> & alphaMap) const;

		// return the name of the map
		const String & name() const {
			return itsName;
		}
		void setName( const String& mapName ) {
			itsName = mapName;
		}

		// Register/Unregister a PixelCanvasColorTable that uses this Colormap
		// <group>
		void registerPCColorTable(PixelCanvasColorTable *pcctbl);
		void unregisterPCColorTable(PixelCanvasColorTable *pcctbl);
		// </group>

		// set/get the colormap brightness level in range 0 to 1
		// <group>
		void setBrightness(const Float &brightness, const Bool &doReinstall = True);
		Float getBrightness() const {
			return itsBrightness;
		};
		// </group>

		// set/get the colormap alpha level in range 0 to 1
		// <group>
		void setAlpha(const Float &brightness, const Bool &doReinstall = True);
		Float getAlpha() const {
			return itsAlpha;
		};
		// </group>

		// set/get the colormap contrast level
		// <group>
		void setContrast(const Float &contrast, const Bool &doReinstall = True);
		Float getContrast() const {
			return itsContrast;
		};
		// </group>

		// set/get the inverse flags
		// <group>
		void setInvertFlags(const Bool &red, const Bool &green, const Bool &blue,
		                    const Bool &doReinstall = True);
		void getInvertFlags(Bool &red, Bool &green, Bool &blue) const;
		// </group>

		// Set whether or not the colormap should use a log scale.
		void setLogScale( const Int & logScale, const Bool & doReinstall = True);

		// Do resizeCallbacks on the PixelCanvasColorTables that use this
		// Colormap
		void doResizeCallbacks();

		// Set the Colormap shaping function.  If the argument is 0, then
		// resort to using the default shaping function, which happens to
		// be a polynomial of order 1.
		void setShapingFunction(Function1D<Float, Float> *shapingfunc = 0);

		// Set and retrieve the coefficients of the shaping function.
		// <group>
		void setShapingCoefficients(const Vector<Float> &params,
		                            const Bool &doReinstall = True);
		const Vector<Float> getShapingCoefficients() const;
		// </group>

		// Write a Colormap to an ostream in a simple text form.
		friend ostream & operator << (ostream & os, const Colormap & c);

		// Write a Colormap to an AipsIO stream in a binary format.
		friend AipsIO &operator<<(AipsIO &aio, const Colormap & c);

		// Write a Colormap to a LogIO stream.
		friend LogIO &operator<<(LogIO &lio, const Colormap & c);

		// Read a Colormap from an AipsIO stream in a binary format.
		// Will throw an AipsError if the current Colormap Version does not match
		// that of the one on disk.
		friend AipsIO &operator>>(AipsIO &aio, Colormap & c);

		// Provide access to the colormap definition.
		ColormapDefinition *definition() {
			return itsColormapDefinition;
		}
		void setColormapDefinition( ColormapDefinition* definition );

	protected:

		// reinstall this Colormap on the registered PixelCanvasColorTables
		void reinstall();

	private:

		// name of this Colormap.
		String itsName;

		// is this Colormap rigid?
		Bool itsIsRigid;

		// what is its rigid size?
		uInt itsRigidSize;
		//Transparency
		Float itsAlpha;

		// levels
		Float itsBrightness, itsBrightnessScale;
		Float itsContrast, itsContrastScale;
		// invert flags
		Bool itsInvertRed, itsInvertGreen, itsInvertBlue;
		Int itsLogScale;

		ColormapDefinition *itsColormapDefinition;

		// function for shaping the colormap
		Function1D<Float, Float> *itsShapingFunction;
		Bool itsOwnShapingFunction;

		SimpleOrderedMap<PixelCanvasColorTable *, uInt> itsPCColorTables;

		enum { ColormapVersion = 1 };
	};


} //# NAMESPACE CASA - END

#endif


