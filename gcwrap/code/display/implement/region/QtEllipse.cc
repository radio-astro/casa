//# QtEllipse.cc: GUI eliptical region
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

#include <display/region/QtEllipse.qo.h>
#include <display/region/RegionSource.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {
	// QtEllipse::QtEllipse( RegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("ellipse",factory), Ellipse(blc_x, blc_y, trc_x, trc_y) { }

	QtEllipse::QtEllipse( QtRegionSourceKernel *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y, bool hold_signals ) :
				QtRegion("ellipse",factory, hold_signals), Ellipse(wc, blc_x, blc_y, trc_x, trc_y) { mystate->init( ); }

	QtEllipse::~QtEllipse( ) { }

	void QtEllipse::output( ds9writer &out ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    std::vector<std::pair<double,double> > pts(2);
	    pts[0].first = blc_x;
	    pts[0].second = blc_y;
	    pts[1].first = trc_x;
	    pts[1].second = trc_y;
	    out.ellipse(wc_,pts);
	}
    }
}
