//# Rectangle.cc: non-GUI rectangular region
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

#include <display/region/Rectangle.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <vector>

#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/PanelDisplay.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <images/Regions/WCBox.h>
#include <display/ds9/ds9writer.h>

#include <imageanalysis/Annotations/AnnRectBox.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <display/DisplayDatas/MSAsRaster.h>
#include <display/DisplayErrors.h>

#include <tr1/memory>

namespace casa {
	namespace viewer {

		Rectangle::Rectangle( WorldCanvas *wc, QtRegionDock *d, double x1, double y1, double x2, double y2,
		                      bool hold_signals ) :	Region( "rectangle", wc, d, hold_signals ),
			blc_x(x1<x2?x1:x2),
			blc_y(y1<y2?y1:y2),
			trc_x(x1<x2?x2:x1),
			trc_y(y1<y2?y2:y1) {
			initHistogram();
			complete = true;
		}

		// carry over from QtRegion... hopefully, removed soon...
		Rectangle::Rectangle( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1, double y1, double x2, double y2,
		                      bool hold_signals) :	Region( "rectangle", wc, rs->dock( ), hold_signals ),
			blc_x(x1<x2?x1:x2),
			blc_y(y1<y2?y1:y2),
			trc_x(x1<x2?x2:x1),
			trc_y(y1<y2?y2:y1) {
			initHistogram();
			complete = true;
		}


		Rectangle::~Rectangle( ) { }


		unsigned int Rectangle::check_handle( double x, double y ) const {
			bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
			bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;
			return trc ? 1 : brc ? 2 : blc ? 3 : tlc ? 4 : 0;
		}

		int Rectangle::clickHandle( double x, double y ) const {
			if ( visible_ == false ) return 0;
			return check_handle( x, y );
		}

		bool Rectangle::doubleClick( double x, double y ) {
			bool flagged_ms = false;
			const std::list<DisplayData*> &dds = wc_->displaylist( );
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				DisplayData *dd = *ddi;
				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
				if (padd==0) {
					// if this DisplayData is not an image, attempt to flag (measurement set)
					MSAsRaster *msar =  dynamic_cast<MSAsRaster*>(dd);
					if ( msar != 0 ) {
						flagged_ms = true;
						flag(msar);
					}
					continue;
				}
			}
			if ( flagged_ms ) return true;

			// if no MeasurementSet was found (to flag), generate statistics output to the terminal
			return Region::doubleClick( x, y );
		}

		bool Rectangle::valid_translation( double dx, double dy, double width_delta, double height_delta ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			const double lxmin = wc_->linXMin( );
			const double lxmax = wc_->linXMax( );
			const double lymin = wc_->linYMin( );
			const double lymax = wc_->linYMax( );

			const double x_delta = width_delta  / 2.0;
			const double y_delta = height_delta  / 2.0;

			double pt = blc_x + dx - x_delta;
			if ( pt < lxmin || pt > lxmax ) return false;
			pt = trc_x + dx + x_delta;
			if ( pt < lxmin || pt > lxmax ) return false;
			pt = blc_y + dy - y_delta;
			if ( pt < lymin || pt > lymax ) return false;
			pt = trc_y + dy + y_delta;
			if ( pt < lymin || pt > lymax ) return false;
			return true;
		}

		void Rectangle::resize( double width_delta, double height_delta ) {
			double dx = width_delta / 2.0;
			double dy = height_delta / 2.0;
			blc_x -= dx;
			blc_y -= dy;
			trc_x += dx;
			trc_y += dy;
			updateStateInfo( true, region::RegionChangeModified );
		}


		int Rectangle::moveHandle( int handle, double x, double y ) {
			switch ( handle ) {
			case 1:		// trc handle
				if ( x < blc_x ) {
					if ( y < blc_y ) {
						trc_x = blc_x;
						trc_y = blc_y;
						blc_x = x;
						blc_y = y;
						handle = 3;
					} else {
						trc_x = blc_x;
						trc_y = y;
						blc_x = x;
						handle = 4;
					}
				} else if ( y < blc_y ) {
					trc_y = blc_y;
					blc_y = y;
					handle = 2;
				} else {
					trc_x = x;
					trc_y = y;
				}
				break;
			case 2:		// brc handle
				if ( x < blc_x ) {
					if ( y > trc_y ) {
						blc_y = trc_y;
						trc_x = blc_x;
						trc_y = y;
						blc_x = x;
						handle = 4;
					} else {
						trc_x = blc_x;
						blc_x = x;
						blc_y = y;
						handle = 3;
					}
				} else if ( y > trc_y ) {
					blc_y = trc_y;
					trc_x = x;
					trc_y = y;
					handle = 1;
				} else {
					trc_x = x;
					blc_y = y;
				}
				break;
			case 3:		// blc handle
				if ( x > trc_x ) {
					if ( y > trc_y ) {
						blc_x = trc_x;
						blc_y = trc_y;
						trc_x = x;
						trc_y = y;
						handle = 1;
					} else {
						blc_x = trc_x;
						trc_x = x;
						blc_y = y;
						handle = 2;
					}
				} else if ( y > trc_y ) {
					blc_y = trc_y;
					blc_x = x;
					trc_y = y;
					handle = 4;
				} else {
					blc_x = x;
					blc_y = y;
				}
				break;
			case 4:		// tlc handle
				if ( x > trc_x ) {
					if ( y < blc_y ) {
						blc_x = trc_x;
						trc_y = blc_y;
						blc_y = y;
						trc_x = x;
						handle = 2;
					} else {
						blc_x = trc_x;
						trc_x = x;
						trc_y = y;
						handle = 1;
					}
				} else if ( y < blc_y ) {
					trc_y = blc_y;
					blc_x = x;
					blc_y = y;
					handle = 3;
				} else {
					blc_x = x;
					trc_y = y;
				}
				break;
			}

			if ( blc_x > trc_x || blc_y > trc_y ) throw internal_error("rectangle inconsistency");
			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
			return handle;
		}

		void Rectangle::move( double dx, double dy ) {
			blc_x += dx;
			trc_x += dx;
			blc_y += dy;
			trc_y += dy;
			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
		}

		void Rectangle::linearCenter( double &x, double &y ) const {
			x = linear_average(blc_x,trc_x);
			y = linear_average(blc_y,trc_y);
		}

		void Rectangle::pixelCenter( double &x, double &y ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			double lx = linear_average(blc_x,trc_x);
			double ly = linear_average(blc_y,trc_y);

			try {
				linear_to_pixel( wc_, lx, ly, x, y );
			} catch(...) {
				return;
			}
		}

		AnnotationBase *Rectangle::annotation( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );

			double wblc_x, wblc_y, wtrc_x, wtrc_y;
			try {
				linear_to_world( wc_, blc_x, blc_y, trc_x, trc_y, wblc_x, wblc_y, wtrc_x, wtrc_y );
			} catch(...) {
				return 0;
			}
			const Vector<String> &units = wc_->worldAxisUnits( );

			Quantity qblc_x( wblc_x, units[0] );
			Quantity qblc_y( wblc_y, units[1] );
			Quantity qtrc_x( wtrc_x, units[0] );
			Quantity qtrc_y( wtrc_y, units[1] );

			const DisplayData *dd = wc_->displaylist().front();


			Vector<Stokes::StokesTypes> stokes;
			AnnRectBox *box = 0;

			try {
				std::vector<int> axes = dd->displayAxes( );
				IPosition shape(cs.nPixelAxes( ));
				for ( size_t i=0; i < shape.size( ); ++i )
					shape(i) = dd->dataShape( )[axes[i]];
				box = new AnnRectBox( qblc_x, qblc_y, qtrc_x, qtrc_y, cs, shape, stokes );
			} catch ( AipsError &e ) {
				cerr << "Error encountered creating an AnnRectBox:" << endl;
				cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
			} catch ( ... ) {
				cerr << "Error encountered creating an AnnRectBox..." << endl;
			}

			return box;
		}

		bool Rectangle::flag( MSAsRaster *msar ) {
			if ( wc_ == 0 ) return false;
			return msar->flag( wc_, blc_x, blc_y, trc_x, trc_y );
		}

		void Rectangle::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
		                                      std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::RectRegion;

			double wblc_x, wblc_y, wtrc_x, wtrc_y;
			try {
				linear_to_world( wc_, blc_x, blc_y, trc_x, trc_y, wblc_x, wblc_y, wtrc_x, wtrc_y );
			} catch(...) {
				return;
			}

			double pblc_x, pblc_y, ptrc_x, ptrc_y;
			try {
				linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, pblc_x, pblc_y, ptrc_x, ptrc_y );
			} catch(...) {
				return;
			}

			pixel_pts.resize(2);
			pixel_pts[0].first = static_cast<int>(pblc_x);
			pixel_pts[0].second = static_cast<int>(pblc_y);
			pixel_pts[1].first = static_cast<int>(ptrc_x);
			pixel_pts[1].second = static_cast<int>(ptrc_y);

			world_pts.resize(2);
			world_pts[0].first = wblc_x;
			world_pts[0].second = wblc_y;
			world_pts[1].first = wtrc_x;
			world_pts[1].second = wtrc_y;
		}


		void Rectangle::drawRegion( bool selected ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			int x1, y1, x2, y2;
			try {
				linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );
			} catch(...) {
				return;
			}
			pc->drawRectangle( x1, y1, x2, y2 );

			// draw the center
			if (getDrawCenter())// && markCenter())
				drawCenter( center_x_, center_y_, center_delta_x_, center_delta_y_);

			if ( selected && memory::nullptr.check( creating_region ) ) {
				Int w = x2 - x1;
				Int h = y2 - y1;

				Int s = 0;		// handle size
				if (w>=18 && h>=18) s = 6;
				else if (w>=15 && h>=15) s = 5;
				else if (w>=12 && h>=12) s = 4;
				else if (w>=9 && h>=9) s = 3;

				double xdx, ydy;
				screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
				handle_delta_x = xdx - blc_x;
				handle_delta_y = ydy - blc_y;

				int hx0 = x1;
				int hx1 = x1 + s;
				int hx2 = x2 - s;
				int hx3 = x2;
				int hy0 = y1;
				int hy1 = y1 + s;
				int hy2 = y2 - s;
				int hy3 = y2;	// set handle coordinates
				if (s) {
					pushDrawingEnv( region::SolidLine);
					if ( weaklySelected( ) ) {
						if ( marked_region_count( ) > 0 && mouse_in_region ) {
							pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						} else {
							pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						}
					} else if ( marked( ) ) {
						pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
					} else {
						pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
					}
					popDrawingEnv( );
				}
			}
		}

		bool Rectangle::within_vertex_handle( double x, double y ) const {
			bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
			bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;
			return trc || brc || blc || tlc;
		}

		// returns point state (region::PointLocation)
		region::PointInfo Rectangle::checkPoint( double x, double y )  const {
			unsigned int result = 0;
			if ( x > blc_x && x < trc_x && y > blc_y && y < trc_y )
				result |= region::PointInside;
			unsigned int handle = check_handle( x, y );
			if ( handle )
				result |= region::PointHandle;
			return region::PointInfo( x, y, result == 0 ? (unsigned int) region::PointOutside : result, handle );
		}

		unsigned int Rectangle::mouseMovement( double x, double y, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

			//if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y ) {
			if ( x > blc_x && x < trc_x && y > blc_y && y < trc_y ) {
				weaklySelect( mouse_in_region == false );
				mouse_in_region = true;
				result |= region::MouseSelected;
				result |= region::MouseRefresh;
				selected_ = true;
				draw( other_selected );
				if ( other_selected == false ) {
					// mark flag as this is the region (how to mix in other shapes)
					// of interest for statistics updates...
					selectedInCanvas( );
				}
			} else {
				weaklyUnselect( );
				mouse_in_region = false;
				if ( selected_ == true ) {
					selected_ = false;
					draw( other_selected );
					result |= region::MouseRefresh;
				}
			}
			return result;
		}


		RegionInfo::stats_t *Rectangle::get_ms_stats( MSAsRaster *msar, double x, double y ) {

			RegionInfo::stats_t *result = new RegionInfo::stats_t( );

			Vector<Double> pos(2);
			try {
				linear_to_world( wc_, x, y, pos[0], pos[1] );
			} catch(...) {
				return result;
			}

			msar->showPosition( *result, pos );
			return result;
		}

		std::list<std::tr1::shared_ptr<RegionInfo> > *Rectangle::generate_dds_centers( ) {

			std::list<std::tr1::shared_ptr<RegionInfo> > *region_centers = new std::list<std::tr1::shared_ptr<RegionInfo> >( );
			if( wc_==0 ) return region_centers;

			Int zindex = 0;
			if (wc_->restrictionBuffer()->exists("zIndex")) {
				wc_->restrictionBuffer()->getValue("zIndex", zindex);
			}

			DisplayData *dd = 0;
			const std::list<DisplayData*> &dds = wc_->displaylist( );
			Vector<Double> lin(2), blc(2), trc(2);

			lin(0) = blc_x;
			lin(1) = blc_y;
			if ( ! wc_->linToWorld(blc, lin)) return region_centers;

			lin(0) = trc_x;
			lin(1) = trc_y;
			if ( ! wc_->linToWorld(trc, lin)) return region_centers;

			std::string errMsg_;
			std::map<String,bool> processed;
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				dd = *ddi;

				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
				if (padd==0) {
					MSAsRaster *msar =  dynamic_cast<MSAsRaster*>(dd);
					if ( msar != 0 ) {
						cout << "No centering on MS!" <<endl;
					}
					continue;
				}

				try {
					if ( ! padd->conformsTo(*wc_) ) continue;

					std::tr1::shared_ptr<ImageInterface<Float> > image(padd->imageinterface( ));

					if ( ! image  ) continue;

					String description = image->name(false);
					String name = image->name(true);
					std::map<String,bool>::iterator repeat = processed.find(description);
					if (repeat != processed.end()) continue;
					processed.insert(std::map<String,bool>::value_type(description,true));

					Int nAxes = image->ndim( );
					IPosition shp = image->shape( );
					const DisplayCoordinateSystem &cs = image->coordinates( );

					int zIndex = padd->activeZIndex( );
					IPosition pos = padd->fixedPosition( );
					Vector<Int> dispAxes = padd->displayAxes( );

					if ( nAxes == 2 ) dispAxes.resize(2,True);

					if ( nAxes < 2 || Int(shp.nelements()) != nAxes ||
					        Int(pos.nelements()) != nAxes ||
					        anyLT(dispAxes,0) || anyGE(dispAxes,nAxes) )
						continue;

					if ( dispAxes.nelements() > 2u )
						pos[dispAxes[2]] = zIndex;

					dispAxes.resize(2,True);

					// WCBox dummy;
					Quantum<Double> px0(0.,"pix");
					Vector<Quantum<Double> > blcq(nAxes,px0), trcq(nAxes,px0);

					//Int spaxis = getAxisIndex( image, String("Spectral") );
					for (Int ax = 0; ax < nAxes; ax++) {
						//if ( ax == dispAxes[0] || ax == dispAxes[1] || ax == spaxis) {
						if ( ax == dispAxes[0] || ax == dispAxes[1]) {
							trcq[ax].setValue(shp[ax]-1);
						} else  {
							blcq[ax].setValue(pos[ax]);
							trcq[ax].setValue(pos[ax]);
						}
					}

					// technically (I guess), WorldCanvasHolder::worldAxisUnits( ) should be
					// used here, because it references the "CSmaster" DisplayData which all
					// of the display options are referenced from... lets hope all of the
					// coordinate systems are kept in sync...      <drs>
					const Vector<String> &units = wc_->worldAxisUnits( );

					for (Int i = 0; i < 2; i++) {
						Int ax = dispAxes[i];

						blcq[ax].setValue(blc[i]);
						trcq[ax].setValue(trc[i]);

						blcq[ax].setUnit(units[i]);
						trcq[ax].setUnit(units[i]);
					}

					WCBox box(blcq, trcq, cs, Vector<Int>());
					ImageRegion *imageregion = new ImageRegion(box);

					region_centers->push_back(std::tr1::shared_ptr<RegionInfo>(new ImageRegionInfo(name,description,getLayerCenter(padd, image, *imageregion))));

					delete imageregion;
				} catch (const casa::AipsError& err) {
					errMsg_ = err.getMesg();
					continue;
				} catch (...) {
					errMsg_ = "Unknown error centering region";
					continue;
				}
			}
			return region_centers;
		}

		ImageRegion *Rectangle::get_image_region( DisplayData *dd ) const {

			if( wc_==0 ) return 0;

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
			if ( padd == 0 ) return 0;

			Vector<Double> lin(2);
			Vector<Double> blc(2);
			Vector<Double> trc(2);

			lin(0) = blc_x;
			lin(1) = blc_y;
			if ( ! wc_->linToWorld(blc, lin)) return 0;

			lin(0) = trc_x;
			lin(1) = trc_y;
			if ( ! wc_->linToWorld(trc, lin)) return 0;

			std::tr1::shared_ptr<ImageInterface<Float> > image(padd->imageinterface( ));
			if ( image == 0 ) return 0;

			Vector<Int> dispAxes = padd->displayAxes( );
			dispAxes.resize(2,True);

			const Vector<String> &units = wc_->worldAxisUnits( );
			const DisplayCoordinateSystem &cs = image->coordinates( );
			Vector<Quantum<Double> > qblc, qtrc;
			qblc.resize(2);
			qtrc.resize(2);

			qblc[0] = Quantum<Double>(blc[0], units[0]);
			qblc[1] = Quantum<Double>(blc[1], units[1]);
			qtrc[0] = Quantum<Double>(trc[0], units[0]);
			qtrc[1] = Quantum<Double>(trc[1], units[1]);

			ImageRegion *result = 0;
			try {
				WCBox box( qblc, qtrc, IPosition(dispAxes), cs, Vector<Int>() );
				result = new ImageRegion(box);

			} catch(...) { }
			return result;
		}

		void Rectangle::generate_nonimage_statistics( DisplayData *dd, std::list<RegionInfo> *region_statistics ) {
			MSAsRaster *msar =  dynamic_cast<MSAsRaster*>(dd);
			if ( msar != 0 ) {
				RegionInfo::stats_t *blc_stats = get_ms_stats( msar, blc_x, blc_y );
				RegionInfo::stats_t *trc_stats = get_ms_stats( msar, trc_x, trc_y );
				String name = msar->name( );
				region_statistics->push_back(MsRegionInfo(name,name + " [blc]",blc_stats));
				region_statistics->push_back(MsRegionInfo(name,name + " [trc]",trc_stats));
			}
		}

		void Rectangle::boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const {
			blcx = blc_x;
			blcy = blc_y;
			trcx = trc_x;
			trcy = trc_y;
		}

		void Rectangle::output( ds9writer &out ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			std::string path = QtDisplayData::path(wc_->csMaster());
			out.setCsysSource(path.c_str( ));
			std::vector<std::pair<double,double> > pts(2);
			pts[0].first = blc_x;
			pts[0].second = blc_y;
			pts[1].first = trc_x;
			pts[1].second = trc_y;
			out.rectangle(wc_,pts);
		}

	}

}
