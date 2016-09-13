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
#include <display/region/Region.qo.h>

class QStackedWidget;

namespace casa {

	class QtDisplayPanelGui;
	class QtDisplayData;

	class AnnRectBox;
	class AnnEllipse;
	class AnnSymbol;
	class AnnPolygon;
	class AnnPolyline;

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
			void regionUpdate( int, viewer::region::RegionChanges, const QList<double> &world_x, const QList<double> &world_y,
			                   const QList<int> &pixel_x, const QList<int> &pixel_y );
			void regionUpdateResponse( int, const QString &shape, const QString &name,
			                           const QList<double> &world_x, const QList<double> &world_y,
			                           const QList<int> &pixel_x, const QList<int> &pixel_y,
			                           const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );
			void newCorners( double, double, double, double);
			void show1DSliceTool();

		public slots:
//Used to change the position of the source.
			void adjustPosition( double blcx, double blcy, double trcx, double trcy );

		protected:
			friend class QtRegionSource;

			/* SHARED_PTR<Rectangle> rectangle( int blc_x, int blc_y, int trc_x, int trc_y ); */
			SHARED_PTR<Rectangle> rectangle( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
			SHARED_PTR<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 );
			SHARED_PTR<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts );
			SHARED_PTR<Polyline> polyline( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 );
			SHARED_PTR<Polyline> polyline( RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts );
			SHARED_PTR<PVLine> pvline( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );

// ellipse is derived from rectangle... so while this should be "SHARED_PTR<Ellipse>" this would preclude
// the direct reuse of the Rectangle code (which only differs by region creation)... perhaps a case where
// smart pointers are not so smart (in not mirroring the inheritance hiearchy)... though perhaps it can be
// generalized to "SHARED_PTR<Region>"...
			SHARED_PTR<Rectangle> ellipse( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );
			SHARED_PTR<Rectangle> point( RegionCreator *rc, WorldCanvas *wc, double x, double y, QtMouseToolNames::PointRegionSymbols sym, int size );

			QtMouseToolNames::PointRegionSymbols currentPointSymbolType( ) const;


		protected slots:
			void loadRegions( const QString &path, const QString &type );
			void updateRegionState(QtDisplayData*);



		private:

			void load_crtf_regions( WorldCanvas *, const QString &path );
			void load_crtf_rectangle( WorldCanvas *wc, MDirection::Types cstype, const AnnRectBox *box );
			void load_crtf_ellipse( WorldCanvas *wc, MDirection::Types cstype, const AnnEllipse *ellipse );
			void load_crtf_point( WorldCanvas *wc, MDirection::Types cstype, const AnnSymbol *symbol );
			void load_crtf_polygon( WorldCanvas *wc, MDirection::Types cstype, const AnnPolygon *polygon );
			void load_crtf_polyline( WorldCanvas *wc, MDirection::Types cstype, const AnnPolyline *polyline );

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
