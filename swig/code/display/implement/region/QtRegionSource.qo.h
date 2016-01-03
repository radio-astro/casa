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

#include <map>
#include <QObject>
#include <display/region/RegionSource.h>

class QStackedWidget;

namespace casa {

    class QtDisplayPanelGui;
    class QtDisplayData;

    class AnnRectBox;
    class AnnEllipse;
    class AnnSymbol;
    class AnnPolygon;

    namespace viewer {

	class QtRegion;
	class QtRegionDock;

	class QtRegionSourceKernel : public QObject, public RegionSourceKernel {
	    Q_OBJECT
	    public:
		QtRegionSourceKernel( QtDisplayPanelGui *panel );

		QtRegionDock *dock( );
		int numFrames( ) const;

		~QtRegionSourceKernel( );

		void revokeRegion( Region *r );

		// inherited pure-virtual from dtorNotifiee, removes deleted regions...
		void dtorCalled( const dtorNotifier * );

	    signals:
		void regionCreated( int, const QString &shape, const QString &name,
				    const QList<double> &world_x, const QList<double> &world_y,
				    const QList<int> &pixel_x, const QList<int> &pixel_y,
				    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		void regionUpdate( int, const QList<double> &world_x, const QList<double> &world_y,
				   const QList<int> &pixel_x, const QList<int> &pixel_y );

		void regionUpdateResponse( int, const QString &shape, const QString &name,
					   const QList<double> &world_x, const QList<double> &world_y,
					   const QList<int> &pixel_x, const QList<int> &pixel_y,
					   const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

	    protected:
		friend class QtRegionSource;

		/* std::tr1::shared_ptr<Rectangle> rectangle( int blc_x, int blc_y, int trc_x, int trc_y ); */
		std::tr1::shared_ptr<Rectangle> rectangle( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
		std::tr1::shared_ptr<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 );
		std::tr1::shared_ptr<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts );

		// ellipse is derived from rectangle... so while this should be "std::tr1::shared_ptr<Ellipse>" this would preclude
		// the direct reuse of the Rectangle code (which only differs by region creation)... perhaps a case where
		// smart pointers are not so smart (in not mirroring the inheritance hiearchy)... though perhaps it can be
		// generalized to "std::tr1::shared_ptr<Region>"...
		std::tr1::shared_ptr<Rectangle> ellipse( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
		std::tr1::shared_ptr<Rectangle> point( RegionCreator *rc, WorldCanvas *wc, double x, double y );

	    protected slots:
		void loadRegions( bool &handled, const QString &path, const QString &type );
		void updateRegionState(QtDisplayData*);

	    private: 

		void load_crtf_regions( WorldCanvas *, const QString &path );
		void load_crtf_rectangle( WorldCanvas *wc, MDirection::Types cstype, const AnnRectBox *box );
		void load_crtf_ellipse( WorldCanvas *wc, MDirection::Types cstype, const AnnEllipse *ellipse );
		void load_crtf_point( WorldCanvas *wc, MDirection::Types cstype, const AnnSymbol *symbol );
		void load_crtf_polygon( WorldCanvas *wc, MDirection::Types cstype, const AnnPolygon *polygon );

		QtDisplayPanelGui *panel_;

		std::map<Region*,RegionCreator*> creator_of_region;

	};

	class QtRegionSource : public RegionSource {
	    public:
		QtRegionSource( RegionCreator *rc, QtDisplayPanelGui *panel );
	    protected:
		friend class QtRegionSourceFactory;
		QtRegionSource( RegionCreator *rc, QtDisplayPanelGui *panel, const shared_kernel_ptr_type &kernel );
	};

    }
}

#endif