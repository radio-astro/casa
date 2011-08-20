//# rectangle.h: base class for statistical regions
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


#ifndef REGION_RECTANGLE_H_
#define REGION_RECTANGLE_H_

#include <display/region/Region.h>
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
	class Rectangle : public Region {
	    public:
		~Rectangle( );
		Rectangle( ) : blc_x(0), blc_y(0), trc_x(0), trc_y(0) { }
		Rectangle( double x1, double y1, double x2, double y2 ) : Region( ),
		    blc_x(x1<x2?x1:x2), blc_y(y1<y2?y1:y2), trc_x(x1<x2?x2:x1), trc_y(y1<y2?y2:y1) { }
		Rectangle( WorldCanvas *wc, double x1, double y1, double x2, double y2) : Region( wc ),
		    blc_x(x1<x2?x1:x2), blc_y(y1<y2?y1:y2), trc_x(x1<x2?x2:x1), trc_y(y1<y2?y2:y1) { }

		bool clickWithin( double x, double y ) const
		    { return x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y; }

		int clickHandle( double x, double y ) const;

#if OLDSTUFF
		void update( double x1, double y1, double x2, double y2 )
		    { blc_x = x1<x2?x1:x2; blc_y = y1<y2?y1:y2; trc_x = x1<x2?x2:x1; trc_y = y1<y2?y2:y1; }

		virtual void clearstats( ) { }
		virtual void addstats( const std::string &name, std::list<std::pair<String,String> > *stats ) { }
#endif

		// returns mouse movement state
		int mouseMovement( double x, double y, bool other_selected );

		// indicates that the user has selected this rectangle...
		virtual void selectedInCanvas( ) = 0;

		// for rectangles, resizing can change the handle...
		int resize( int handle, double x, double y );
		void move( double dx, double dy )
		    { blc_x += dx; trc_x += dx; blc_y += dy; trc_y += dy; updateStateInfo( true ); }

		void regionCenter( double &x, double &y ) const;

		AnnRegion *annotation( ) const;

	    protected:
		StatisticsList *generate_statistics_list( );

		// in "linear" coordinates...
		void boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const;

		void drawRegion( bool );
		void drawHandles( );

		double blc_x, blc_y;
		double trc_x, trc_y;
		double handle_delta_x, handle_delta_y;

	};
    }
}

#endif
