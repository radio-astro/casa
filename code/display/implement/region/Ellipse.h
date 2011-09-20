//# ellipse.h: base class for statistical regions
//# Copyright (C) 2011
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


#ifndef REGION_ELLIPSE_H_
#define REGION_ELLIPSE_H_

#include <display/region/Rectangle.h>
#include <casa/BasicSL/String.h>
#include <casadbus/types/ptr.h>
#include <list>

namespace casa {

    class PanelDisplay;
    class AnnRegion;

    namespace viewer {

	// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
	// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
	// this means that coordinate transformation is required each time the region is drawn.
	class Ellipse : public Rectangle {
	    public:
		~Ellipse( );
		Ellipse( WorldCanvas *wc, double x1, double y1, double x2, double y2) :
		    Rectangle( wc, x1, y1, x2, y2 ) { }

		// returns mouse movement state
		int mouseMovement( double x, double y, bool other_selected );

		AnnRegion *annotation( ) const;

	    protected:

		StatisticsList *generate_statistics_list( );

		virtual void fetch_region_details( RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
						   std::vector<std::pair<double,double> > &world_pts ) const;

		void drawRegion( bool );
		/* void drawHandles( ); */

	};
    }
}

#endif
