//# QtMouseToolState.cc: constants and [global] mouse-button state
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


#include <display/QtViewer/QtMouseToolState.qo.h>
#include <stdio.h>		/***for sprintf( )***/
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

	namespace QtMouseToolNames {

		const std::string ZOOM = "zoom";
		const std::string PAN = "pan";
		const std::string SHIFTSLOPE = "shiftslope";
		const std::string BRIGHTCONTRAST = "brightcontrast";
		const std::string POINT =  "point";
		const std::string RECTANGLE = "rectangle";
		const std::string ELLIPSE = "ellipse";
		const std::string POLYGON = "polygon";
		const std::string POLYLINE = "polyline";
		const std::string RULERLINE = "rulerline";
		const std::string POSITIONVELOCITY = "pvtool";
		const std::string MULTICROSSHAIR = "multicrosshair";
		const std::string ANNOTATIONS = "annotations";
		const std::string NONE = "";


		//# the final elements stand for "none" (or "invalid");

		const std::string tools[] = { ZOOM, PAN, SHIFTSLOPE, BRIGHTCONTRAST,
		                              POINT, RECTANGLE, ELLIPSE,  POLYGON, POLYLINE,
		                              RULERLINE, POSITIONVELOCITY, MULTICROSSHAIR, ANNOTATIONS, NONE
		                            };

		const std::string longnames[] = { "Zooming", "Panning",
		                                  "Colormap fiddling - shift/slope",
		                                  "Colormap fiddling - brightness/contrast",
		                                  "Point marking", "Rectangle drawing", "Ellipse drawing", "Polygon drawing",
		                                  "Polyline drawing", "Ruler drawing", "P/V tool", "Multipanel crosshair", "Annotations",  ""
		                                };

		std::string iconnames[] = { "magnifyb", "handb", "arrowcrossb",
		                            "brightcontrastb", "symdotb", "rectregionb", "ellregionb", "polyregionb",
		                            "polylineb", "rulerb", "pvtoolb", "mpcrosshairb", "dontuseb",  ""
		                          };

		const std::string helptexts[] = {
			"Use the assigned mouse button to drag out a rectangle."
			"\nUse handles to resize."
			"\nDouble click inside rectangle-> zoom in"
			"\nDouble click outside rectangle -> zoom out"
			"\n<Esc> to cancel",

			"Drag tool using the assigned mouse button.",

			"Drag tool using the assigned mouse button.",

			"Drag tool using the assigned mouse button.",

			"Click here and hold for menu of other symbol markers..."
			"\nClick assigned mouse button to mark a point region at cursor position."
			"\n<Esc> to cancel",

			"Use the assigned mouse button to drag out a rectangle."
			"\nUsehandles to resize.\nDouble click inside to execute."
			"\n<Esc> to cancel",

			"Use the assigned mouse button to drag out an ellipse."
			"\nUsehandles to resize.\nDouble click inside to execute."
			"\n<Esc> to cancel",

			"Place polygon points by clicking the assigned mouse button."
			"\nDoubleclick on last point to finish polygon."
			"\nUse handles to resize."
			"\nDouble click inside to execute."
			"\n<Esc> to cancel",

			"Place polyline points by clicking the assigned mouse button."
			"\nDoubleclick on last point to finish the polyline."
			"\nUse handles to rearrange points."
			"\n<Esc> to cancel",

			"Press assigned mouse button and draw ruler line by dragging.",

			"Draw a line to generate a P/V slice...",

			"Select a shape to draw and then click / drag on screen to place it."
			"\nSelect \"more\" to show more options"
			"\n<Esc> to cancel",


			""
		};

		static std::vector<std::string> point_symbol_icons;
		static std::vector<std::string> point_symbol_names;
		static std::vector<std::string> point_symbol_rc;

		static void init_point_region_symbol_rc( ) {
			if ( point_symbol_rc.size( ) == 0 ) {
				// these must be added to the vector in the order
				// implied by the PointRegionSymbols enum...
				point_symbol_rc.push_back("dot");
				point_symbol_rc.push_back("drarrow");
				point_symbol_rc.push_back("dlarrow");
				point_symbol_rc.push_back("urarrow");
				point_symbol_rc.push_back("ularrow");
				point_symbol_rc.push_back("plus");
				point_symbol_rc.push_back("x");
				point_symbol_rc.push_back("circle");
				point_symbol_rc.push_back("diamond");
				point_symbol_rc.push_back("square");
			}
		}


		static void init_point_region_symbol_names( ) {
			if ( point_symbol_names.size( ) == 0 ) {
				init_point_region_symbol_rc( );
				for ( unsigned int i=0; i < point_symbol_rc.size( ); ++i ) {
					point_symbol_names.push_back("sym" + point_symbol_rc[i]);
				}
			}
		}

		static void init_point_region_symbol_icons( ) {
			if ( point_symbol_icons.size( ) == 0 ) {
				init_point_region_symbol_names( );
				for ( unsigned int i=0; i < point_symbol_names.size( ); ++i ) {
					point_symbol_icons.push_back(std::string(":/icons/") + point_symbol_names[i] + "%s.png");
				}
			}
		}

		std::string pointRegionSymbolIcon( PointRegionSymbols sym, int button ) {
			if ( point_symbol_icons.size( ) == 0 ) init_point_region_symbol_icons( );
			if ( sym < 0 || sym >= SYM_POINT_REGION_COUNT ) return std::string( );
			std::string pattern = point_symbol_icons[(int)sym];
			char buf[pattern.size( )+40];
			if ( button < 0 || button > 3 )
				sprintf( buf, pattern.c_str( ), "" );
			else {
				char num[30];
				sprintf( num, "b%d", button );
				sprintf( buf, pattern.c_str( ), num );
			}
			return std::string(buf);
		}


		std::string pointRegionSymbolRc( PointRegionSymbols sym ) {
			if ( point_symbol_rc.size( ) == 0 ) init_point_region_symbol_rc( );
			return point_symbol_rc[sym];
		}

	}  // namespace QtMouseToolNames




	Int QtMouseToolState::mousebtns_[] =  { 1, 0, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 };

	QtMouseToolState::QtMouseToolState( ) : rc(viewer::getrc( )) {
		initToolState( );
	}

	int QtMouseToolState::getButtonState(const std::string &tool) const {
		std::map<std::string,int>::const_iterator it = tool_state.find(tool);
		return it == tool_state.end( ) ? -1 : it->second;
	}

	void QtMouseToolState::chgMouseBtn(String tool, Int mousebtn) {

		// Request reassignment of a given mouse button to a tool.
		// NB: this is where guis, etc. should request a button change, so that
		// all stay on same page (not directly to tool or displaypanel, e.g.).

		using namespace QtMouseToolNames;
		// Constants (nTools, tool names, etc.) used by Qt mouse tools.

		if(mousebtn<0 || mousebtn>3) return;	// (Invalid mouse button).

		Int ti = toolIndex(tool);

		if(mousebtn!=0) {

			Int oldti = toolIndexOnButton_(mousebtn);
			// oldti is tool formerly assigned to requested mouse button.

			if(ti==oldti) return;		// (already where we want to be).

			if(oldti!=nTools) {
				mousebtns_[oldti] = 0;		// assign old tool to no button.
				emit mouseBtnChg(toolName(oldti), mousebtns_[oldti]);
				// broadcast that change.
			}
		}

		if(ti!=nTools) {
			bool saved[3] = { false, false, false };
			mousebtns_[ti] = mousebtn;	// assign requested tool to requested button.
			emit mouseBtnChg(tool, mousebtn);	// broadcast that change.
			if ( mousebtn != 0 ) {
				char buf[40];
				sprintf( buf, "viewer.mouse.button.%s", (mousebtn == 1 ? "one" : mousebtn == 2 ? "two" : "three") );
				saved[mousebtn] = true;
				rc.put( buf, toolName(ti) );
				for ( int i=0; i < nTools; ++i ) {
					if ( i != ti && mousebtns_[i] != 0 ) {
						sprintf( buf, "viewer.mouse.button.%s", (mousebtns_[i] == 1 ? "one" : mousebtns_[i] == 2 ? "two" : "three") );
						saved[mousebtns_[i]] = true;
						rc.put( buf, toolName(i) );
					}
				}
				for ( int i=0; i < 3; ++i ) {
					if ( saved[i] == false ) {
						sprintf( buf, "viewer.mouse.button.%s", (i == 1 ? "one" : i == 2 ? "two" : "three") );
						rc.put( buf, "unbound" );
					}
				}

			}
		}
	}

	void QtMouseToolState::initButtonState( std::string numstr, int num ) {
		using namespace QtMouseToolNames;
		std::string val = rc.get("viewer.mouse.button." + numstr);
		if ( val != "" ) {
			Int ti = toolIndex(val);
			if ( ti != nTools ) {
				for ( int i=0; i < nTools; ++i )
					if ( mousebtns_[i] == num ) {
						mousebtns_[i] = 0;
						break;
					}
				mousebtns_[ti] = num;
			}
		}
	}

	void QtMouseToolState::initToolState( ) {
		using namespace QtMouseToolNames;
		std::string val = rc.get( "viewer.mouse.state.point" );
		init_point_region_symbol_rc( );

		if ( val == "" ) {
			rc.put("viewer.mouse.state.point",point_symbol_rc[SYM_DOT]);
			return;
		}

		int ti = -1;
		for( int i=0; i<nTools; ++i ) {
			if ( tools[i] == POINT ) {
				ti = i;
				break;
			}
		}
		if ( ti < 0 ) return;

		for ( unsigned int i=0; i < point_symbol_rc.size( ); ++i ) {
			if ( val == point_symbol_rc[i] ) {
				if ( point_symbol_names.size() == 0 ) init_point_region_symbol_names( );
				iconnames[ti] = point_symbol_names[i] + "b";
				tool_state[POINT] = i;
				break;
			}
		}

		initButtonState("one",1);
		initButtonState("two",2);
		initButtonState("three",3);
	}

	void QtMouseToolState::mouseBtnStateChg(String /*tool*/, Int sym) {
		using namespace QtMouseToolNames;

		if ( sym < 0 || sym >= SYM_POINT_REGION_COUNT ) return;
		int ti = -1;
		for( int i=0; i<nTools; ++i ) {
			if ( tools[i] == POINT ) {
				ti = i;
				break;
			}
		}
		if ( ti < 0 ) return;
		if ( point_symbol_icons.size() == 0 ) init_point_region_symbol_icons( );
		iconnames[ti] = point_symbol_names[sym] + "b";

		tool_state[POINT] = sym;
		emit mouseBtnChg(POINT, mousebtns_[ti]);	// force update

	}

	void QtMouseToolState::emitBtns() {
		// Request signalling of the current mouse button setting for every
		// type of tool.  Call this if you want to assure that all objects are
		// up-to-date on mouse button settings.

		using namespace QtMouseToolNames;
		// Constants (nTools, tool names, etc.) used by Qt mouse tools

		for(Int i=0; i<nTools; i++) emit mouseBtnChg(tools[i], mousebtns_[i]);
	}



	Int QtMouseToolState::toolIndexOnButton_(Int mousebtn) {
		// Returns index of tool currently assigned to a button (1, 2, or 3).
		// (Always returns QtMouseToolNames::nTools for button 0).

		using namespace QtMouseToolNames;

		if(mousebtn<=0 || mousebtn>3) return nTools;

		Int ti = 0;
		while(ti<nTools && mousebtn!=mousebtns_[ti]) ti++;
		return ti;
	}

} //# NAMESPACE CASA - END
