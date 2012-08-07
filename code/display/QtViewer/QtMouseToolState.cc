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
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace QtMouseToolNames {

  const String ZOOM = "zoom";
  const String PAN = "pan";
  const String SHIFTSLOPE = "shiftslope";
  const String BRIGHTCONTRAST = "brightcontrast";
  const String POINT =  "point";
  const String RECTANGLE = "rectangle";
  const String ELLIPSE = "ellipse";
  const String POLYGON = "polygon";
  const String POLYLINE = "polyline";
  const String RULERLINE = "rulerline";
  const String MULTICROSSHAIR = "multicrosshair";
  const String ANNOTATIONS = "annotations";
  const String NONE = "";

  
  //# the final elements stand for "none" (or "invalid");
  
  const String tools[] = { ZOOM, PAN, SHIFTSLOPE, BRIGHTCONTRAST,
			   POINT, RECTANGLE, ELLIPSE,  POLYGON, POLYLINE,
			   RULERLINE, MULTICROSSHAIR, ANNOTATIONS, NONE };

  const String longnames[] = { "Zooming", "Panning",
    "Colormap fiddling - shift/slope",
    "Colormap fiddling - brightness/contrast",
    "Positioning", "Rectangle drawing", "Ellipse drawing", "Polygon drawing",
    "Polyline drawing", "Ruler drawing", "Multipanel crosshair", "Annotations",  "" };
    
  String iconnames[] = { "magnifyb", "handb", "arrowcrossb",
   "brightcontrastb", "symdotb", "rectregionb", "ellregionb", "polyregionb",
   "polylineb", "rulerb",  "mpcrosshairb", "dontuseb",  "" };
    
  const String helptexts[] = {
    "Use the assigned mouse button to drag out a rectangle."
    "\nUsehandles to resize."
    "\nDouble click inside rectangle-> zoom in"
    "\nDouble click outside rectangle -> zoom out"
    "\n<Esc> to cancel",
     
    "Drag tool using the assigned mouse button.",
     
    "Drag tool using the assigned mouse button.",
     
    "Drag tool using the assigned mouse button.",
     
    "Click assigned mouse button to drop cursor at that position."
    "\nDouble click inside to execute."
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
       
    "Press assigned mouse button and draw ruler line by dragging."

    "Select a shape to draw and then click / drag on screen to place it."
    "\nSelect \"more\" to show more options"
    "\n<Esc> to cancel",

         
     ""  };

    static std::vector<std::string> point_symbol_icons;
    static std::vector<std::string> point_symbol_names;

    static void init_point_region_symbol_names( ) {
	if ( point_symbol_names.size( ) == 0 ) {
	    // these must be added to the vector in the order
	    // implied by the PointRegionSymbols enum...
	    point_symbol_names.push_back("symdot");
	    point_symbol_names.push_back("symdrarrow");
	    point_symbol_names.push_back("symdlarrow");
	    point_symbol_names.push_back("symurarrow");
	    point_symbol_names.push_back("symularrow");
	    point_symbol_names.push_back("symplus");
	    point_symbol_names.push_back("symx");
	    point_symbol_names.push_back("symcircle");
	    point_symbol_names.push_back("symdiamond");
	    point_symbol_names.push_back("symsquare");
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

    QString pointRegionSymbolIcon( PointRegionSymbols sym, int button ){
	if ( point_symbol_names.size( ) == 0 ) init_point_region_symbol_icons( );
	if ( sym < 0 || sym >= SYM_POINT_REGION_COUNT ) return QString( );
	std::string pattern = point_symbol_icons[(int)sym];
	char buf[pattern.size( )+40];
	if ( button < 0 || button > 3 )
	    sprintf( buf, pattern.c_str( ), "" );
	else {
	    char num[30];
	    sprintf( num, "b%d", button );
	    sprintf( buf, pattern.c_str( ), num );
	}
	return QString(buf);
    }

}  // namespace QtMouseToolNames




Int QtMouseToolState::mousebtns_[] =  { 1, 0, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0,  0 };


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
    mousebtns_[ti] = mousebtn;	// assign requested tool to requested button.
    emit mouseBtnChg(tool, mousebtn);	// broadcast that change.
  }
}
 

void QtMouseToolState::mouseBtnStateChg(String /*tool*/, Int sym) {
    using namespace QtMouseToolNames;

    if ( sym < 0 || sym >= SYM_POINT_REGION_COUNT ) return;
    int ti = -1;
    for( int i=0; i<nTools; ++i ) {
	if ( tools[i] == POINT ) { ti = i; break; }
    }
    if ( ti < 0 ) return;
    if ( point_symbol_icons.size() == 0 ) init_point_region_symbol_icons( );
    iconnames[ti] = point_symbol_names[sym] + "b";

    emit mouseBtnChg(POINT, mousebtns_[ti]);	// force update

}

void QtMouseToolState::emitBtns() {
  // Request signalling of the current mouse button setting for every
  // type of tool.  Call this if you want to assure that all objects are
  // up-to-date on mouse button settings.
  
  using namespace QtMouseToolNames;
  // Constants (nTools, tool names, etc.) used by Qt mouse tools
  
  for(Int i=0; i<nTools; i++) emit mouseBtnChg(tools[i], mousebtns_[i]);  }

  

Int QtMouseToolState::toolIndexOnButton_(Int mousebtn) {
  // Returns index of tool currently assigned to a button (1, 2, or 3).
  // (Always returns QtMouseToolNames::nTools for button 0).
  
  using namespace QtMouseToolNames;
  
  if(mousebtn<=0 || mousebtn>3) return nTools;
  
  Int ti = 0;   
  while(ti<nTools && mousebtn!=mousebtns_[ti]) ti++;
  return ti;  }


  
bool QtMouseToolState::selectPointRegionSymbolIcon( QtMouseToolNames::PointRegionSymbols sym ) {
    using namespace QtMouseToolNames;
    if ( sym < 0 || sym >= SYM_POINT_REGION_COUNT ) return false;
    int offset = -1;
    for( int i=0; i<nTools; ++i ) {
	if ( tools[i] == POINT ) offset = i;
    }
    if ( offset < 0 ) return false;
    iconnames[offset] = point_symbol_icons[sym];
    emit mouseBtnChg(tools[offset], mousebtns_[offset]);
    return true;
}
  

} //# NAMESPACE CASA - END
