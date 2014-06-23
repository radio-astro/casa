//# QtMouseToolState.qo.h: constants and [global] mouse-button state
//# for the qtviewer 'mouse-tools' used by its display panel[s].
//# Copyright (C) 2005
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


#ifndef MOUSETOOLSTATE_H_
#define MOUSETOOLSTATE_H_

#include <string>

// <synopsis>
// QtMouseToolNames holds static constants for Qt mouse tools (with
// some access methods for them).  There is an ordered list of 'mouse tool'
// types, names and reference indices for them, etc.
// </synopsis>
namespace casa {
	namespace QtMouseToolNames {

		enum { nTools = 13 };

		extern const std::string ZOOM, PAN, SHIFTSLOPE, BRIGHTCONTRAST,	POINT, RECTANGLE, ELLIPSE, POLYGON, POLYLINE,
		       RULERLINE, POSITIONVELOCITY, MULTICROSSHAIR, ANNOTATIONS, NONE;

		//# nTools is an invalid tool index (or stands for "none") in these arrays.
		extern const std::string     tools[nTools+1], longnames[nTools+1], helptexts[nTools+1];
		extern std::string           iconnames[nTools+1];

		// Return index of named tool within the master list.
		// (i==nTools means 'not a tool').
		inline int toolIndex(std::string tool) {
			for(int i=0;; i++) if (tools[i]==tool || i==nTools) return i;
			//# i==nTools means 'not a tool'.
		}

		inline std::string toolName(int toolindex) {
			if(toolindex<0 || toolindex>=nTools) return NONE;
			return tools[toolindex];
		}

		inline std::string longName(std::string tool) {
			return longnames[toolIndex(tool)];
		}
		inline std::string iconName(std::string tool) {
			return iconnames[toolIndex(tool)];
		}
		inline std::string help(std::string tool)     {
			return helptexts[toolIndex(tool)];
		}

		enum PointRegionSymbols { SYM_DOT=0, SYM_DOWN_RIGHT_ARROW=1, SYM_DOWN_LEFT_ARROW=2,
		                          SYM_UP_RIGHT_ARROW=3, SYM_UP_LEFT_ARROW=4, SYM_PLUS=5,
		                          SYM_X=6, SYM_CIRCLE=7, SYM_DIAMOND=8, SYM_SQUARE=9,
		                          SYM_POINT_REGION_COUNT=10, SYM_UNKNOWN
		                        };
		std::string pointRegionSymbolIcon( PointRegionSymbols, int button=-1 );
		std::string pointRegionSymbolRc(PointRegionSymbols);

	};
};

#endif
