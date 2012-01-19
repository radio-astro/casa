//# qtregionsource.cc: Qt regionfactory for generating regions
//# with surrounding Gui functionality
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
//# $Id: $

#include <display/region/QtRegionSource.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/region/QtRectangle.qo.h>
#include <display/region/QtPolygon.qo.h>
#include <display/region/QtEllipse.qo.h>
#include <display/region/QtPoint.qo.h>

namespace casa {
    namespace viewer {

	QtRegionSource::~QtRegionSource( ) { }

	// std::tr1::shared_ptr<Rectangle> QtRegionSource::rectangle( int blc_x, int blc_y, int trc_x, int trc_y ) {
	//     return std::tr1::shared_ptr<Rectangle>(new QtRectangle( this, blc_x, blc_y, trc_x, trc_y ));
	// }
	std::tr1::shared_ptr<Rectangle> QtRegionSource::rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
	    QtRectangle *result = new QtRectangle( this, wc, blc_x, blc_y, trc_x, trc_y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	std::tr1::shared_ptr<Polygon> QtRegionSource::polygon( WorldCanvas *wc, double x1, double y1 ) {
	    QtPolygon *result = new QtPolygon( this, wc, x1, y1, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Polygon>(result);
	    // return std::tr1::shared_ptr<Polygon>( );
	}

	std::tr1::shared_ptr<Polygon> QtRegionSource::polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
	    QtPolygon *result = new QtPolygon( this, wc, pts, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Polygon>(result);
	    // return std::tr1::shared_ptr<Polygon>( );
	}

	std::tr1::shared_ptr<Rectangle> QtRegionSource::ellipse( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
	    QtEllipse *result = new QtEllipse( this, wc, blc_x, blc_y, trc_x, trc_y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	std::tr1::shared_ptr<Rectangle> QtRegionSource::point( WorldCanvas *wc, double x, double y ) {
	    QtPoint *result = new QtPoint( this, wc, x, y, true );

	    connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						    const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
		     this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
	    connect( result, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
		     this, SIGNAL( regionUpdate( int, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

	    result->releaseSignals( );
	    return std::tr1::shared_ptr<Rectangle>(result);
	}

	QtRegionDock *QtRegionSource::dock( ) { return panel_->regionDock( ); }
	int QtRegionSource::numFrames( ) const { return panel_->numFrames( ); }

    }
}

