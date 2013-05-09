//# regionsource.h: regionsource producing persistent regions used within the casa viewer
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


#ifndef REGION_REGIONSOURCE_H_
#define REGION_REGIONSOURCE_H_
#include <list>
#include <tr1/memory>
#include <display/region/RegionCreator.h>
#include <display/Utilities/dtor.h>
#include <display/Display/MouseToolState.h>

namespace casa {

	class WorldCanvas;

	namespace viewer {

		class Rectangle;
		class Polygon;
		class Polyline;
		class Ellipse;
		class PVLine;

		class QtRegionDock;
		class RegionSource;
		class RegionCreator;

		class RegionSourceKernel : public dtorNotifiee {
		public:
			typedef std::tr1::shared_ptr<RegionSourceKernel> shared_kernel_ptr_type;

			RegionSourceKernel( ) { }
			virtual ~RegionSourceKernel( );

			// inherited pure-virtual from dtorNotifiee, removes deleted regions...
			void dtorCalled( const dtorNotifier * );

			// re-generate regionUpdateResponse( ) signals for existing regions...
			// with *same* arguments as regionCreated( ), for the benefit of a newly created (e.g. QtProfile) tool...
			virtual void generateExistingRegionUpdates( );

			virtual QtRegionDock *dock( ) {
				return 0;
			}
			virtual int numFrames( ) const {
				return -1;
			}

			virtual void revokeRegion( Region *r ) = 0;

		protected:
			friend class RegionSource;
			virtual std::tr1::shared_ptr<Rectangle> rectangle( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) = 0;
			virtual std::tr1::shared_ptr<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 ) = 0;
			virtual std::tr1::shared_ptr<Polygon> polygon( RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) = 0;
			virtual std::tr1::shared_ptr<Polyline> polyline( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 ) = 0;
			virtual std::tr1::shared_ptr<Polyline> polyline( RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) = 0;
			virtual std::tr1::shared_ptr<Rectangle> ellipse( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) = 0;
			virtual std::tr1::shared_ptr<Rectangle> point( RegionCreator *rc, WorldCanvas *wc, double x, double y, QtMouseToolNames::PointRegionSymbols sym, int size ) = 0;
			virtual std::tr1::shared_ptr<PVLine> pvline( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) = 0;

			virtual QtMouseToolNames::PointRegionSymbols currentPointSymbolType( ) const = 0;

			// register region for dtor callback, and add to list of created regions...
			void register_new_region( Region * );

			std::list<Region*> created_regions;
		};

		class RegionSource {
		public:
			typedef RegionSourceKernel::shared_kernel_ptr_type shared_kernel_ptr_type;

			std::tr1::shared_ptr<Rectangle> rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
				return kernel_->rectangle(region_creator,wc,blc_x,blc_y,trc_x,trc_y);
			}
			virtual std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, double x1, double y1 ) {
				return kernel_->polygon(region_creator,wc,x1,y1);
			}
			virtual std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
				return kernel_->polygon(region_creator,wc,pts);
			}
			virtual std::tr1::shared_ptr<Polyline> polyline( WorldCanvas *wc, double x1, double y1 ) {
				return kernel_->polyline(region_creator,wc,x1,y1);
			}
			virtual std::tr1::shared_ptr<Polyline> polyline( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
				return kernel_->polyline(region_creator,wc,pts);
			}
			virtual std::tr1::shared_ptr<Rectangle> ellipse( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
				return kernel_->ellipse(region_creator,wc,blc_x,blc_y,trc_x,trc_y);
			}
			virtual std::tr1::shared_ptr<Rectangle> point( WorldCanvas *wc, double x, double y, QtMouseToolNames::PointRegionSymbols sym, int size ) {
				return kernel_->point(region_creator,wc,x,y,sym,size);
			}
			std::tr1::shared_ptr<PVLine> pvline( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
				return kernel_->pvline(region_creator,wc,blc_x,blc_y,trc_x,trc_y);
			}

			RegionSource( RegionCreator *rc, const shared_kernel_ptr_type &k ) :  kernel_(k), region_creator(rc) { }
			RegionSource( const RegionSource &other ) : kernel_(other.kernel_), region_creator(other.region_creator) { }

			void revokeRegion( Region *r ) {
				kernel_->revokeRegion(r);
			}

			shared_kernel_ptr_type kernel( ) {
				return kernel_;
			}

			QtRegionDock *dock( ) {
				return kernel_->dock( );
			}
			int numFrames( ) const {
				return kernel_->numFrames( );
			}

			QtMouseToolNames::PointRegionSymbols currentPointSymbolType( ) const {
				return kernel_->currentPointSymbolType( );
			}
			virtual ~RegionSource( ) { }

		private:
			shared_kernel_ptr_type kernel_;
			RegionCreator *region_creator;
		};
	}
}

#endif
