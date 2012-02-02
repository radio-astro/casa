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

		/* std::tr1::shared_ptr<Rectangle> rectangle( int blc_x, int blc_y, int trc_x, int trc_y ); */
		std::tr1::shared_ptr<Rectangle> rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
		std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, double x1, double y1 );
		std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts );

		// ellipse is derived from rectangle... so while this should be "std::tr1::shared_ptr<Ellipse>" this would preclude
		// the direct reuse of the Rectangle code (which only differs by region creation)... perhaps a case where
		// smart pointers are not so smart (in not mirroring the inheritance hiearchy)... though perhaps it can be
		// generalized to "std::tr1::shared_ptr<Region>"...
		std::tr1::shared_ptr<Rectangle> ellipse( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
		std::tr1::shared_ptr<Rectangle> point( WorldCanvas *wc, double x, double y );

		QtRegionDock *dock( );
		int numFrames( ) const;

		~QtRegionSource( );

	    signals:
		void regionCreated( int, const QString &shape, const QString &name,
				    const QList<double> &world_x, const QList<double> &world_y,
				    const QList<int> &pixel_x, const QList<int> &pixel_y,
				    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		void regionUpdate( int, const QList<double> &world_x, const QList<double> &world_y,
				   const QList<int> &pixel_x, const QList<int> &pixel_y );

	    private: 
		QtDisplayPanelGui *panel_;
	};
    }
}

#endif
