//# QtPolygon.cc: GUI polygon
//# Copyright (C) 2012
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
//# $Id: $

#include <display/region/QtPolygon.qo.h>
#include <display/region/RegionSource.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {

	QtPolygon::QtPolygon( QtRegionSourceKernel *factory, WorldCanvas *wc, double x1, double y1, bool hold_signals ) :
				QtRegion("polygon",factory, hold_signals), Polygon(wc, x1, y1) { mystate->init( ); }

	QtPolygon::QtPolygon( QtRegionSourceKernel *factory, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, bool hold_signals ) :
				QtRegion("polygon",factory, hold_signals), Polygon(wc, pts) { mystate->init( ); }

 	void QtPolygon::output( ds9writer &out ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    out.polygon(wc_,drawing_points( ));
	}

	void QtPolygon::polygonComplete( ) {
	    // for polygons signals remain blocked until polygon is complete...
	    releaseSignals( );
	}
    }
}
