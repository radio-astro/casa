//# point.h: base class for statistical regions
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


#ifndef REGION_POINT_H_
#define REGION_POINT_H_

#include <display/Display/MouseToolState.h>
#include <display/region/Rectangle.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

    class PanelDisplay;
    class AnnRegion;
    class PixelCanvas;

    namespace viewer {

	// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
	// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
	// this means that coordinate transformation is required each time the region is drawn.
	class Point : public Rectangle {
	    public:
		~Point( );
		Point( WorldCanvas *wc, double x, double y, QtMouseToolNames::PointRegionSymbols sym ) :
					Rectangle( wc, x, y, x, y ), marker_(sym){ }

		bool setMarker( QtMouseToolNames::PointRegionSymbols sym );

		int clickHandle( double /*x*/, double /*y*/ ) const { return 0; }

		bool clickWithin( double x, double y ) const;

		// returns point state (Region::PointLocation)
		PointInfo checkPoint( double x, double y ) const;
		// how much to scale the symbol used to mark point regions...
		// assumed to go from 0 to 9...
		virtual int markerScale( ) const = 0;

		// returns mouse movement state
		unsigned int mouseMovement( double x, double y, bool other_selected );
		void resize( double, double ) { }

		AnnotationBase *annotation( ) const;

		// points cannot be degenerate...
		bool degenerate( ) const { return false; }

	    protected:

		static const int radius;

		virtual void fetch_region_details( Region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
						   std::vector<std::pair<double,double> > &world_pts ) const;

		void drawRegion( bool );

		std::list<RegionInfo> *generate_dds_centers(bool /*skycomp*/);

		QtMouseToolNames::PointRegionSymbols marker_;

	    private:
		void draw_arrow( PixelCanvas *, int /*x*/, int /*y*/, int /*xsign*/, int /*ysign*/,
				 int /*scale_unit*/, int /*scale*/ );

	};
    }
}

#endif
