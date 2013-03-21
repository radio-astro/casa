//# PVLine.cc: non-GUI rectangular region
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

#include <display/Display/Options.h>
#include <display/region/PVLine.qo.h>
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

#include <imageanalysis/ImageAnalysis/PVGenerator.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/region/QtRegionDock.qo.h>
#include <casa/Exceptions/Error.h>

#include <casa/Quanta/MVAngle.h>

namespace casa {
    namespace viewer {

	PVLine::PVLine( WorldCanvas *wc, QtRegionDock *d, double x1, double y1, double x2, double y2,
		bool hold_signals ) :	Region( "p/v line", wc, d, hold_signals ),
														pt1_x(x1), pt1_y(y1),
														pt2_x(x2), pt2_y(y2), sub_dpg(0) {
		center_x = linear_average(pt1_x,pt2_x);
		center_y = linear_average(pt1_y,pt2_y);
		initHistogram();
		complete = true;
	}

	// carry over from QtRegion... hopefully, removed soon...
	PVLine::PVLine( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1, double y1, double x2, double y2,
		bool hold_signals) :	Region( "p/v line", wc, rs->dock( ), hold_signals ),
														pt1_x(x1), pt1_y(y1),
														pt2_x(x2), pt2_y(y2), sub_dpg(0) {
		center_x = linear_average(pt1_x,pt2_x);
		center_y = linear_average(pt1_y,pt2_y);
		initHistogram();
		complete = true;
	}


	PVLine::~PVLine( ) { }


		unsigned int PVLine::check_handle( double x, double y ) const {
			bool pt1 = x >= (pt1_x - handle_delta_x) && x <= (pt1_x + handle_delta_x) && y >= (pt1_y - handle_delta_y) && y <= (pt1_y + handle_delta_y);
			bool pt2 = x >= (pt2_x - handle_delta_x) && x <= (pt2_x + handle_delta_x) && y >= (pt2_y - handle_delta_y) && y <= (pt2_y + handle_delta_y);
			// bool center = x >= (center_x - handle_delta_x) && x <= (center_x + handle_delta_x) && y >= (center_y - handle_delta_y) && y <= (center_y + handle_delta_y);
			return pt1 ? 1 : pt2 ? 2 : 0;
		}

		int PVLine::clickHandle( double x, double y ) const {
			if ( visible_ == false ) return 0;
			return check_handle( x, y );
		}

		struct strip_white_space {
			strip_white_space(size_t s) : size(s+1), off(0), buf(new char[size]) { }
			strip_white_space( const strip_white_space &other ) : size(other.size), off(other.off),
																  buf(new char[size])
					{ strcpy(buf,other.buf); }
			~strip_white_space( ) { delete [] buf; }
			void operator( )( char c ) { if ( ! isspace(c) ) buf[off++] = c; };
			operator std::string( ) { buf[off] = '\0'; return std::string(buf); }
			operator String( ) { buf[off] = '\0'; return String(buf); }
			size_t size;
			size_t off;
			char *buf;
		};

		std::string PVLine::worldCoordinateStrings( double x, double y ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return std::string( );

			double result_x, result_y;
			const Vector<String> &units = wc_->worldAxisUnits();
			const Vector<String> &axis_labels = wc_->worldAxisNames( );

			MDirection::Types cccs = current_casa_coordsys( );
			Quantum<Vector<double> > result = convert_angle( x, units[0], y, units[1], cccs, MDirection::J2000 );
			result_x = result.getValue("rad")(0);
			result_y = result.getValue("rad")(1);

			std::string return_value;
			if ( axis_labels(0) == "Declination" /* || (coord != region::J2000 && coord != region::B1950) */ ) {
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// D.M.S
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// MVAngle::operator(double norm) => 2*pi*norm to 2pi*norm+2pi
				//x = MVAngle(result_x)(0.0).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				// MVAngle::operator( ) => -pi to +pi
				std::string s = MVAngle(result_x)( ).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				return_value = std::for_each(s.begin(),s.end(),strip_white_space(s.size()));
			} else {
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// H:M:S
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				std::string s = MVAngle(result_x)(0.0).string(MVAngle::TIME,SEXAGPREC);
				return_value = std::for_each(s.begin(),s.end(),strip_white_space(s.size()));
			}

			if ( axis_labels(1) == "Declination" /* || (coord != region::J2000 && coord != region::B1950) */ ) {
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// D.M.S
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// MVAngle::operator(double norm) => 2*pi*norm to 2pi*norm+2pi
				//x = MVAngle(result_x)(0.0).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				// MVAngle::operator( ) => -pi to +pi
				std::string s = MVAngle(result_y)( ).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				return_value = return_value + "  " + std::for_each(s.begin(),s.end(),strip_white_space(s.size()));
			} else {
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				// H:M:S
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				std::string s = MVAngle(result_y)(0.0).string(MVAngle::TIME,SEXAGPREC);
				return_value = return_value + "  " + std::for_each(s.begin(),s.end(),strip_white_space(s.size()));
			}
			return return_value;
		}

		bool PVLine::doubleClick( double x, double y ) {
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

		bool PVLine::valid_translation( double dx, double dy, double width_delta, double height_delta ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			const double lxmin = wc_->linXMin( );
			const double lxmax = wc_->linXMax( );
			const double lymin = wc_->linYMin( );
			const double lymax = wc_->linYMax( );

			const double x_delta = width_delta  / 2.0;
			const double y_delta = height_delta  / 2.0;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

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

		void PVLine::resize( double width_delta, double height_delta ) {
			double dx = width_delta / 2.0;
			double dy = height_delta / 2.0;
			double &blc_x = pt1_x < pt2_x ? pt1_x : pt2_x;
			double &blc_y = pt1_y < pt2_y ? pt1_y : pt2_y;
			double &trc_x = pt1_x < pt2_x ? pt2_x : pt1_x;
			double &trc_y = pt1_y < pt2_y ? pt2_y : pt1_y;

			blc_x -= dx;
			blc_y -= dy;
			trc_x += dx;
			trc_y += dy;
			updateStateInfo( true, region::RegionChangeModified );
		}

		int PVLine::moveHandle( int handle, double x, double y ) {
			switch ( handle ) {
				case 1:		// end #1
					pt1_x = x;
					pt1_y = y;
					break;
				case 2:		// end #2
					pt2_x = x;
					pt2_y = y;
					break;
				// case 3:		// center handle
				// 	{
				// 		double delta_x = x - center_x;
				// 		double delta_y = y - center_y;
				// 		pt1_x += delta_x;
				// 		pt1_y += delta_y;
				// 		pt2_x += delta_x;
				// 		pt2_y += delta_y;
				// 	}
					break;
			}

			center_x = (pt1_x+pt2_x)/2.0;
			center_y = (pt1_y+pt2_y)/2.0;

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
			return handle;
		}

		void PVLine::move( double dx, double dy ) {
			pt1_x += dx;
			pt2_x += dx;
			pt1_y += dy;
			pt2_y += dy;

			center_x = linear_average(pt1_x,pt2_x);
			center_y = linear_average(pt1_y,pt2_y);

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
		}

		void PVLine::linearCenter( double &x, double &y ) const {
			x = center_x;
			y = center_y;
		}

		void PVLine::pixelCenter( double &x, double &y ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			double lx = center_x;
			double ly = center_y;

			try { linear_to_pixel( wc_, lx, ly, x, y ); } catch(...) { return; }
		}

		AnnotationBase *PVLine::annotation( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const CoordinateSystem &cs = wc_->coordinateSystem( );

			double wpt1_x, wpt1_y, wpt2_x, wpt2_y;
			try { linear_to_world( wc_, pt1_x, pt1_y, pt2_x, pt2_y, wpt1_x, wpt1_y, wpt2_x, wpt2_y ); } catch(...) { return 0; }
			const Vector<String> &units = wc_->worldAxisUnits( );

			Quantity qpt1_x( wpt1_x, units[0] );
			Quantity qpt1_y( wpt1_y, units[1] );
			Quantity qpt2_x( wpt2_x, units[0] );
			Quantity qpt2_y( wpt2_y, units[1] );

			const DisplayData *dd = wc_->displaylist().front();


			Vector<Stokes::StokesTypes> stokes;
			AnnRectBox *box = 0;

			try {
				std::vector<int> axes = dd->displayAxes( );
				IPosition shape(cs.nPixelAxes( ));
				for ( size_t i=0; i < shape.size( ); ++i )
					shape(i) = dd->dataShape( )[axes[i]];
				box = new AnnRectBox( qpt1_x, qpt1_y, qpt2_x, qpt2_y, cs, shape, stokes );
			} catch ( AipsError &e ) {
				cerr << "Error encountered creating an AnnRectBox:" << endl;
				cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
			} catch ( ... ) {
				cerr << "Error encountered creating an AnnRectBox..." << endl;
			}

			return box;
		}

		bool PVLine::flag( MSAsRaster *msar ) {
			if ( wc_ == 0 ) return false;
			return msar->flag( wc_, pt1_x, pt1_y, pt2_x, pt2_y );
		}

		void PVLine::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
											  std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::PVLineRegion;

			double wpt1_x, wpt1_y, wpt2_x, wpt2_y;
			try { linear_to_world( wc_, pt1_x, pt1_y, pt2_x, pt2_y, wpt1_x, wpt1_y, wpt2_x, wpt2_y ); } catch(...) { return; }

			double ppt1_x, ppt1_y, ppt2_x, ppt2_y;
			try { linear_to_pixel( wc_, pt1_x, pt1_y, pt2_x, pt2_y, ppt1_x, ppt1_y, ppt2_x, ppt2_y ); } catch(...) { return; }

			pixel_pts.resize(2);
			pixel_pts[0].first = static_cast<int>(ppt1_x);
			pixel_pts[0].second = static_cast<int>(ppt1_y);
			pixel_pts[1].first = static_cast<int>(ppt2_x);
			pixel_pts[1].second = static_cast<int>(ppt2_y);

			world_pts.resize(2);
			world_pts[0].first = wpt1_x;
			world_pts[0].second = wpt1_y;
			world_pts[1].first = wpt2_x;
			world_pts[1].second = wpt2_y;
		}


		void PVLine::drawRegion( bool selected ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			int x1, y1, x2, y2;
			int cx, cy;

			try {
				linear_to_screen( wc_, pt1_x, pt1_y, pt2_x, pt2_y, x1, y1, x2, y2 );
				linear_to_screen( wc_, center_x, center_y, cx, cy );
			} catch(...) { return; }

			pushDrawingEnv( region::SolidLine, 2 );
			pc->drawLine( x1, y1, x2, y2 );
			popDrawingEnv( );

			if ( selected && memory::nullptr.check( creating_region ) ) {

				int s = 3;

				double xdx, ydy;
				screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
				handle_delta_x = xdx - pt1_x;
				handle_delta_y = ydy - pt2_y;

				if (s) {
					pushDrawingEnv( region::SolidLine);
					if ( weaklySelected( ) ) {
						if ( marked_region_count( ) > 0 && mouse_in_region ) {
							pc->drawRectangle( x1-s, y1-s, x1+s, y1+s );
							pc->drawRectangle( x2-s, y2-s, x2+s, y2+s );
						} else {
							pc->drawFilledRectangle( x1-s, y1-s, x1+s, y1+s );
							pc->drawFilledRectangle( x2-s, y2-s, x2+s, y2+s );
						}
					} else if ( marked( ) ) {
						pc->drawRectangle( x1-s, y1-s, x1+s, y1+s );
						pc->drawRectangle( x2-s, y2-s, x2+s, y2+s );
					} else {
						pc->drawFilledRectangle( x1-s, y1-s, x1+s, y1+s );
						pc->drawFilledRectangle( x2-s, y2-s, x2+s, y2+s );
					}
					popDrawingEnv( );
				}
			}
		}

		bool PVLine::within_vertex_handle( double x, double y ) const {
			bool pt1 = x >= (pt1_x - handle_delta_x) && x <= (pt1_x + handle_delta_x) && y >= (pt1_y - handle_delta_y) && y <= (pt1_y + handle_delta_y);
			bool pt2 = x >= (pt2_x - handle_delta_x) && x <= (pt2_x + handle_delta_x) && y >= (pt2_y - handle_delta_y) && y <= (pt2_y + handle_delta_y);
			bool center = x >= (center_x - handle_delta_x) && x <= (center_x + handle_delta_x) && y >= (center_y - handle_delta_y) && y <= (center_y + handle_delta_y);
			return pt1 || pt2 || center;
		}

		// returns point state (region::PointLocation)
		region::PointInfo PVLine::checkPoint( double x, double y )  const {
			unsigned int result = 0;
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			if ( x > blc_x && x < trc_x && y > blc_y && y < trc_y )
				result |= region::PointInside;
			unsigned int handle = check_handle( x, y );
			if ( handle )
				result |= region::PointHandle;
			return region::PointInfo( x, y, result == 0 ? (unsigned int) region::PointOutside : result, handle );
		}

		unsigned int PVLine::mouseMovement( double x, double y, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

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


		RegionInfo::stats_t *PVLine::get_ms_stats( MSAsRaster *msar, double x, double y ) {

			RegionInfo::stats_t *result = new RegionInfo::stats_t( );

			Vector<Double> pos(2);
			try { linear_to_world( wc_, x, y, pos[0], pos[1] ); } catch(...) { return result; }

			msar->showPosition( *result, pos );
			return result;
		}

		std::list<std::tr1::shared_ptr<RegionInfo> > * PVLine::generate_dds_centers( ){

			std::list<std::tr1::shared_ptr<RegionInfo> > *region_centers = new std::list<std::tr1::shared_ptr<RegionInfo> >( );
			if( wc_==0 ) return region_centers;

			Int zindex = 0;
			if (wc_->restrictionBuffer()->exists("zIndex")) {
				wc_->restrictionBuffer()->getValue("zIndex", zindex);
			}

			DisplayData *dd = 0;
			const std::list<DisplayData*> &dds = wc_->displaylist( );
			Vector<Double> lin(2), blc(2), trc(2);

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

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

					ImageInterface<Float> *image = padd->imageinterface( );

					if ( image == 0 ) continue;

					String description = image->name(false);
					String name = image->name(true);
					std::map<String,bool>::iterator repeat = processed.find(description);
					if (repeat != processed.end()) continue;
					processed.insert(std::map<String,bool>::value_type(description,true));

					Int nAxes = image->ndim( );
					IPosition shp = image->shape( );
					const CoordinateSystem &cs = image->coordinates( );

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

					region_centers->push_back( std::tr1::shared_ptr<RegionInfo>( new PVLineRegionInfo( name, description,
																									   getLayerCenter(padd, image, *imageregion),
																									   std::vector<std::string>( ),
																									   std::vector<std::string>( ))) );

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

		ImageRegion *PVLine::get_image_region( DisplayData *dd ) const {

			if( wc_==0 ) return 0;

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
			if ( padd == 0 ) return 0;

			Vector<Double> lin(2);
			Vector<Double> blc(2);
			Vector<Double> trc(2);

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			lin(0) = blc_x;
			lin(1) = blc_y;
			if ( ! wc_->linToWorld(blc, lin)) return 0;

			lin(0) = trc_x;
			lin(1) = trc_y;
			if ( ! wc_->linToWorld(trc, lin)) return 0;

			ImageInterface<Float> *image = padd->imageinterface( );
			if ( image == 0 ) return 0;

			Vector<Int> dispAxes = padd->displayAxes( );
			dispAxes.resize(2,True);

			const Vector<String> &units = wc_->worldAxisUnits( );
			const CoordinateSystem &cs = image->coordinates( );
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


		RegionInfo *PVLine::newInfoObject( ImageInterface<Float> *image ) {
			RegionInfo::stats_t* dd_stats = new RegionInfo::stats_t();

			double ppt1_x, ppt1_y, ppt2_x, ppt2_y;
			double wpt1_x, wpt1_y, wpt2_x, wpt2_y;
			try { linear_to_pixel( wc_, pt1_x, pt1_y, pt2_x, pt2_y, ppt1_x, ppt1_y, ppt2_x, ppt2_y ); } catch(...) { return 0; }

			try { linear_to_world( wc_, pt1_x, pt1_y, pt2_x, pt2_y, wpt1_x, wpt1_y, wpt2_x, wpt2_y ); } catch(...) { return 0; }

			std::vector<std::string> pixel(2);
			std::vector<std::string> world(2);
			ostringstream pix_oss;
			pix_oss << std::fixed << std::setprecision(1) << ppt1_x << " " << ppt1_y;
			pixel[0] = pix_oss.str( );
			pix_oss.str("");
			pix_oss.clear( );
			pix_oss << std::fixed << std::setprecision(1) << ppt2_x << " " << ppt2_y;
			pixel[1] = pix_oss.str( );

			world[0] = worldCoordinateStrings(wpt1_x,wpt1_y);
			world[1] = worldCoordinateStrings(wpt2_x,wpt2_y);
			
			return new PVLineRegionInfo( image->name(true), image->name(false), dd_stats, pixel, world );
		}

		void PVLine::generate_nonimage_statistics( DisplayData *dd, std::list<RegionInfo> *region_statistics ) {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			MSAsRaster *msar =  dynamic_cast<MSAsRaster*>(dd);
			if ( msar != 0 ) {
				RegionInfo::stats_t *blc_stats = get_ms_stats( msar, blc_x, blc_y );
				RegionInfo::stats_t *trc_stats = get_ms_stats( msar, trc_x, trc_y );
				String name = msar->name( );
				region_statistics->push_back(MsRegionInfo(name,name + " [blc]",blc_stats));
				region_statistics->push_back(MsRegionInfo(name,name + " [trc]",trc_stats));
			}
		}

		void PVLine::boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const {
			// the bounding rectangle is extended here by half the size of the
			// handles in both directions...
			blcx = (pt1_x < pt2_x ? pt1_x : pt2_x) - 3;
			blcy = (pt1_y < pt2_y ? pt1_y : pt2_y) - 3;
			trcx = (pt1_x < pt2_x ? pt2_x : pt1_x) + 3;
			trcy = (pt1_y < pt2_y ? pt2_y : pt1_y) + 3;
		}

		std::string PVLine::display_element::outputPath( ) {
			if ( path_.size( ) == 0 )
				path_ = viewer::options.temporaryPath( name_ + ".pvline." );
			return path_;
		}

		ImageInterface<Float> *PVLine::generatePVImage( ImageInterface<Float> *input_image, std::string output_file, int width, bool need_result ) {
			Record dummy;
			PVGenerator pvgen( input_image, &dummy, "" /*chanInp*/, "" /*stokes*/, "" /*maskInp*/, output_file, true );
			double startx, starty, endx, endy;
			try { linear_to_pixel( wc_, pt1_x, pt1_y, pt2_x, pt2_y, startx, starty, endx, endy ); } catch(...) { return 0; }
			pvgen.setEndpoints( startx, starty, endx, endy );
			pvgen.setHalfWidth((double)((width-1)/2));
			dock_->panel( )->status( "generating temporary image: " + output_file );
			dock_->panel( )->logIO( ) << "generating temporary image \'" << output_file  << "'" << LogIO::POST;
			dock_->panel( )->logIO( ) << "generating P/V image with pixel points: (" <<
				startx << "," << starty << ") (" << endx << "," << endy << ")" << LogIO::POST;
			ImageInterface<Float> *result = 0;
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			try {
				result = pvgen.generate( need_result );
			} catch( AipsError err ) {
				dock_->panel( )->logIO( ) << LogIO::SEVERE << err.getMesg( ) << LogIO::POST;
				// fallback to P/V failure, create dummy image...
				// QtDisplayPanelGui *new_panel = dock_->panel( )->createNewPanel( );
				// new_panel->addDD( "/Users/drs/develop/casa/testing/cas-4515/SPT041847_IMAGE.image", "image", "raster", True, False );
			} catch( ... ) {
				dock_->panel( )->logIO( ) << LogIO::SEVERE << "unexpected error occurred while generating the P/V image" << LogIO::POST;
			}
			QApplication::restoreOverrideCursor();
			return result;
		}

		void PVLine::createPVImage( const std::string &name,const std::string &desc, int width ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			const std::list<DisplayData*> &dds = wc_->displaylist( );
			String casa_desc(desc);
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(*ddi);
				if ( padd == 0 ) continue;
				ImageInterface<Float> *image = padd->imageinterface( );
				if ( image->name( ) == casa_desc ) {
					display_element de( name );
					ImageInterface<Float> *new_image = generatePVImage( image, de.outputPath( ), width, true );
					if ( sub_dpg == 0 ) {
						 sub_dpg = dock_->panel( )->createNewPanel( );
						 connect( sub_dpg, SIGNAL(destroyed(QObject*)), SLOT(dpg_deleted(QObject*)) );
					}					
					sub_dpg->unregisterAllDDs( );
					display_list.push_back(de);
					sub_dpg->addDD( new_image->name(false), "image", "raster", True, True, new_image );
					sub_dpg->show( );
					sub_dpg->raise( );
					break;
				}
			}
		}

		void PVLine::dpg_deleted(QObject*) {
			sub_dpg = 0;
		}

		void PVLine::output( ds9writer &out ) const {

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

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
