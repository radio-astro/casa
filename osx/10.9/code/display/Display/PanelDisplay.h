//# PanelDisplay.h: Provision of panelled displays for data
//# Copyright (C) 2000,2001,2002,2003
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

#ifndef TRIALDISPLAY_PANELDISPLAY_H
#define TRIALDISPLAY_PANELDISPLAY_H

#include <casa/aips.h>
#include <tr1/memory>
#include <casa/Containers/List.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/SimOrdMap.h>
#include <display/Utilities/DisplayOptions.h>
#include <display/Display/MultiWCHolder.h>
#include <display/Display/DisplayEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;
	class WorldCanvas;
	class WorldCanvasHolder;
	class Attribute;
	class AttributeBuffer;
	class MultiWCTool;

// <summary>
// A class which creates and manages "panelled" displays.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li><linkto class="PixelCanvas">PixelCanvas</linkto>
//   <li><linkto class="MultiWCHolder">MultiWCHolder</linkto>
//   <li><linkto class="DisplayData">DisplayData</linkto>
// </prerequisite>
//
// <etymology>
// The PanelDisplay class provides a display consisting of a
// number of panels.
// </etymology>
//
// <synopsis>
// This class uses the <linkto
// class="MultiWCHolder">MultiWCHolder</linkto> class to provide a
// regular set of panels for display multi-view DisplayDatas.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>

	class PanelDisplay : public MultiWCHolder, public DisplayOptions {

	public:

		enum FillOrder {
		    // left to right, then top to bottom
		    LEFT_TOP
		    // others are imaginable
		};


		// Constructor taking: a pointer to the PixelCanvas on which to lay
		// the panels, the number of panels in the x and y directions, the
		// extent of the PixelCanvas to cover with panels, and the
		// fractional spacing between the panels.
		PanelDisplay(PixelCanvas* pixelcanvas,
		             const Int nx = 3, const Int ny = 2,
		             const Float xOrigin = 0.0, const Float yOrigin = 0.0,
		             const Float xSize = 1.0, const Float ySize = 1.0,
		             const Float dx = 0.0, const Float dy = 0.0,
		             const PanelDisplay::FillOrder order = LEFT_TOP);

		// Destructor.
		virtual ~PanelDisplay();

		// Install the default options for this PanelDisplay
		virtual void setDefaultOptions();

		// Apply options stored in rec to the PanelDisplay; these get
		// forwarded to all underlying WorldCanvases
		virtual Bool setOptions(const Record& rec, Record& updatedOptions);
		virtual Record getOptions() const;
		// Set attributes on all WolrdCanvases
		virtual void setAttributes(AttributeBuffer& at);
		virtual void getAttributeValue(const String& name, Int& newValue) const;

		// Describe or set the geometry of the panels, either explicitly
		// or in RecordInterface form.
		// (Note: setGeometry() does not automatically refresh;
		//  when called within setOptions it does, though.)
		// <group>
		virtual void getGeometry(Int& nx, Int& ny, Float& xOrigin,
		                         Float& yOrigin, Float& xSize, Float& ySize,
		                         Float& dx, Float& dy,
		                         PanelDisplay::FillOrder& order) const;
		virtual void getGeometry(RecordInterface& rec) const;
		virtual void setGeometry(const Int nx, const Int ny, const Float xOrigin,
		                         const Float yOrigin, const Float xSize,
		                         const Float ySize, const Float dx, const Float dy,
		                         const PanelDisplay::FillOrder order);
		virtual void setGeometry(const RecordInterface& rec);
		// </group>

		// Clear our part of the PC.  (Clears both front and back buffers.)
		virtual void clear();


		virtual WorldCanvasHolder* wcHolder(WorldCanvas* wc) const;

		virtual PixelCanvas* pixelCanvas() {
			return itsPixelCanvas;
		}

		ConstListIter<WorldCanvas* >* myWCLI;

		virtual void addTool(const String& key, const std::tr1::shared_ptr<MultiWCTool> & );
		/* virtual void addTool(const String& key, MultiWCTool* value); */
		virtual void removeTool(const String& key);
		virtual void setToolKey(const String& toolname,
		                        const Display::KeySym& keysym);
		virtual Bool hasTools();
		virtual void updateTools(Bool remove = True, Bool add = True);
		virtual void disableTools();
		virtual void enableTools();
		virtual void enableTool(const String& toolname);
		virtual void disableTool(const String& toolname);
		virtual const std::tr1::shared_ptr<MultiWCTool> getTool(const String& key);

		virtual ListIter<WorldCanvas* > wcs() {
			//return itsWCLI;
			return ListIter<WorldCanvas* >(itsWCList);
		}

		// Is the specified DisplayData the one in charge of coordinate
		// state of the Panel's WCs?
		virtual Bool isCSmaster(const DisplayData *dd) const;
		void setCSmaster( DisplayData* dd );

		/**
		 * Returns the current mapping of PGP units to pixel units.
		 * This number varies depending on the data units of the plot.
		 */
		float getDrawUnit(  ) const;

		/**
		 * Returns the number of columns of plots that are currently
		 * being displayed.
		 */
		int getColumnCount( ) const;

		/**
		 * Returns the number of rows of plots that are currently
		 * being displayed.
		 */
		int getRowCount() const;


		static const String X_ORIGIN;
		static const String Y_ORIGIN;
		static const String X_SIZE;
		static const String Y_SIZE;




	private:

		// The PixelCanvas on which the panels are laid.
		PixelCanvas* itsPixelCanvas;

		// The number of panels in x and y.
		Int itsNX, itsNY;

		// The origin and size of the used area of the PixelCanvas.
		Float itsXOrigin, itsYOrigin, itsXSize, itsYSize;

		// The offset between each panel.
		Float itsDX, itsDY;

		//The pgmargins
		Int itslpgm, itsrpgm, itstpgm, itsbpgm;

		// What order do we fill the canvases in?
		FillOrder itsOrder;

		// Whether the geometry is currently setup?
		Bool itsGeometrySet;

		// The WorldCanvases which we made, and a convenient iterator.
		List<WorldCanvas* > itsWCList;
		//ListIter<WorldCanvas* >* itsWCLI;


		// The WorldCanvasHolders which we made, and a convenient iterator.

		// (dk note: you should create a local iterator instead of using member
		// iterators.  Otherwise, a call chain may include two methods in the
		// class stepping on each other's iterator use.  Even if you think that
		// won't happen currently, it creates a maintenance minefield).
		//
		// Also note: _exactly the same list_ (with the same _name_, even)
		// is maintained on MWCH level (to be fixed).

		List<WorldCanvasHolder* > itsWCHList;

		//ListIter<WorldCanvasHolder* >* itsWCHLI;

		SimpleOrderedMap<String, std::tr1::shared_ptr<MultiWCTool> > itsMWCTools;

		// unSetup the Geometry.
		void unSetupGeometry();




	};


} //# NAMESPACE CASA - END

#endif
