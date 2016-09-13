//# PanelDisplay.h: A 3d Slice Application
//# Copyright (C) 2000,2001
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

#ifndef TRIALDISPLAY_SLICEPANELDISPLAY_H
#define TRIALDISPLAY_SLICEPANELDISPLAY_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/SimOrdMap.h>
#include <display/DisplayEvents/DTVisible.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/MWCAnimator.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;
	class MultiWCHolder;
	class PanelDisplay;

// <summary>
// An event handler to negotiate between three different PanelDisplays
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> EventHandlers
//   <li> SlicePanelDisplay
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
//
// </synopsis>
//

	class SliceEH : public WCMotionEH,public WCPositionEH, public DTVisible {
	public:
		SliceEH(PanelDisplay* mwch, uInt axis=0, uInt daxis=0);
		virtual ~SliceEH();
		virtual void draw(const WCMotionEvent &ev);
		virtual void operator()(const WCMotionEvent &ev);
		virtual void operator()(const WCPositionEvent &ev);
		virtual void matchKey(Display::KeySym key);
	private:
		PanelDisplay* itsPD;
		MWCAnimator itsAni;
		uInt itsAxis;
		uInt itsDrawAxis;
		Display::KeySym itsPositionKey;
		Display::KeyModifier itsMotionKey;
	};
// <summary> An application provide displays of three axis
// combinations of data in a cube
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> PanelDisplay
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Slicing is a handy tool to ananlyse data cubes
// </motivation>
//
// <todo>
// </todo>
//

	class SlicePanelDisplay : public WCMotionEH {
	public:
		SlicePanelDisplay(PixelCanvas* pcanvas);
		virtual ~SlicePanelDisplay();

		virtual Record getOptions() const;
		virtual Bool setOptions(const Record& rec, Record& updatedOptions);

		virtual void hold();
		virtual void release();

		virtual void refresh(const Display::RefreshReason &reason = Display::UserCommand);

		virtual PanelDisplay* getPanelDisplay(const String& pdname);

		virtual void setToolKey(const String& toolname,
		                        const Display::KeySym& keysym);
		virtual void disableTools();
		virtual void enableTools();

		virtual void operator()(const WCMotionEvent &ev);
		virtual PanelDisplay* activePanelDisplay() {
			return itsActivePD;
		};

		virtual void precompute();

	private:
		SimpleOrderedMap<String, PanelDisplay* > itsPanelDisplays;
		SimpleOrderedMap<String, SliceEH* > itsSliceEHs;
		PanelDisplay* itsActivePD;
		void installEHs();
		void removeEHs();
	};


} //# NAMESPACE CASA - END

#endif
