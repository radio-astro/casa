//# qtregionsource.qo.h: qtregionsource producing persistent regions used within the casa viewer
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


#ifndef REGION_QTREGIONSOURCE_H_
#define REGION_QTREGIONSOURCE_H_

#include <QObject>
#include <display/region/RegionSource.h>

class QStackedWidget;

namespace casa {
    class QtDisplayPanelGui;

    namespace viewer {

	class QtRegion;
	class QtRegionDock;

	class QtRegionSource : public QObject, public RegionSource {
	    Q_OBJECT
	    public:
		QtRegionSource( RegionCreator *rc, QtDisplayPanelGui *panel ) : RegionSource(rc), panel_(panel) { }

		/* memory::cptr<Rectangle> rectangle( int blc_x, int blc_y, int trc_x, int trc_y ); */
		memory::cptr<Rectangle> rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
		memory::cptr<Polygon> polygon( WorldCanvas *wc, double x1, double y1 );

#if 0
		const Ellipse &ellipse( const std::vector<int> &x, const std::vector<int> &y ) = 0;
#endif

		QtRegionDock *dock( );
		int numFrames( ) const;

		~QtRegionSource( );

	    signals:
		void created( QtRegion * );

	    private: 
		QtDisplayPanelGui *panel_;
	};
    }
}

#endif
