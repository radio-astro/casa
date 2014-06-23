//# MultiEllipseTool.cc: Base class for MultiWorldCanvas event-based ellipse tools
//# Copyright (C) 2000,2001,2002
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

#include <display/DisplayEvents/MultiEllipseTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	shared_ptr<viewer::Rectangle> MultiEllipseTool::allocate_region( WorldCanvas *wc, double x1, double y1, double x2, double y2, VOID * ) const {
		return rfactory->ellipse( wc, x1, y1, x2, y2 );
	}

	static std::set<viewer::region::RegionTypes> multi_ellipse_tool_region_set;
	const std::set<viewer::region::RegionTypes> &MultiEllipseTool::regionsCreated( ) const {
		if ( multi_ellipse_tool_region_set.size( ) == 0 ) {
			multi_ellipse_tool_region_set.insert( viewer::region::EllipseRegion );
		}
		return multi_ellipse_tool_region_set;
	}


} //# NAMESPACE CASA - END
