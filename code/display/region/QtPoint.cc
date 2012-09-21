//# QtPoint.cc: GUI point
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

#include <display/region/QtPoint.qo.h>
#include <display/region/RegionSource.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>
#include <QDebug>

namespace casa {
    namespace viewer {
	// QtPoint::QtPoint( RegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("point",factory), Point(blc_x, blc_y, trc_x, trc_y) { }

	QtPoint::QtPoint( QtRegionSourceKernel *factory, WorldCanvas *wc, double x, double y, QtMouseToolNames::PointRegionSymbols sym, bool hold_signals ) :
				QtRegion("point", factory, hold_signals, sym), Point(wc, x, y, sym) {
	    mystate->init( );
	    mystate->disableAnnotation(true);
	}

	QtPoint::~QtPoint( ) { }

	void QtPoint::adjustCorners( double blcx, double blcy, double trcx, double trcy ){
		world_to_linear(wc_, blcx, blcy, blc_x, blc_y );
		world_to_linear( wc_, trcx, trcy, trc_x, trc_y);
		updateStateInfo( true, Region::RegionChangeUpdate );
		//setDrawCenter(false);
		//invalidateCenterInfo();
		refresh();
	}

	void QtPoint::output( ds9writer &out ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    std::vector<std::pair<double,double> > pts(2);
	    pts[0].first = blc_x;
	    pts[0].second = blc_y;
	    pts[1].first = trc_x;
	    pts[1].second = trc_y;
	    out.point(wc_,pts);

	}
    }
}
