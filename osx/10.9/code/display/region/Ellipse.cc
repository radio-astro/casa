//# Ellipse.cc: non-GUI elliptical region
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
//# $Id$
#include <display/region/Ellipse.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/PanelDisplay.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Regions/WCEllipsoid.h>
#include <images/Images/SubImage.h>

#include <imageanalysis/Annotations/AnnEllipse.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/ds9/ds9writer.h>

#include <casa/cppconfig.h>

namespace casa {
	namespace viewer {

		Ellipse::Ellipse( WorldCanvas *wc, QtRegionDock *d, double x1, double y1, double x2, double y2) :
			Rectangle( wc, d, x1, y1, x2, y2 ) {
			initHistogram();
		}

		// carry over from QtRegion... hopefully, removed soon...
		Ellipse::Ellipse( QtRegionSourceKernel *factory, WorldCanvas *wc, double x1, double y1, double x2, double y2, bool hold_signals) :
			Rectangle( "ellipse", wc, factory->dock( ), x1, y1, x2, y2, hold_signals ) {
			initHistogram();
		}

		Ellipse::~Ellipse( ) { }


		AnnotationBase *Ellipse::annotation( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );

			std::pair<Vector<Quantity>,Vector<Quantity> > p_r = posAndRadii( );
			if ( p_r.second.size( ) != 2 ) return 0;

			const DisplayData *dd = wc_->displaylist().front();

			Vector<Stokes::StokesTypes> stokes;
			/*Int polaxis =*/
			CoordinateUtil::findStokesAxis(stokes, cs);

			AnnEllipse *ellipse = 0;
			try {
				std::vector<int> axes = dd->displayAxes( );
				IPosition shape(cs.nPixelAxes( ));
				for ( unsigned int i=0; i < shape.size( ); ++i )
					shape(i) = dd->dataShape( )[axes[i]];
				Quantity rot( p_r.second(1) > p_r.second(0) ? Quantity(0,"deg") : Quantity(90,"deg") );
				//  Can't call them major and minor because of gcc macros which render them:
				//  ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- -------
				//  Quantity gnu_dev_minor (p_r.second(1) > p_r.second(0) ? p_r.second(0) : p_r.second(1));
				//  Quantity gnu_dev_major (p_r.second(1) < p_r.second(0) ? p_r.second(0) : p_r.second(1));
				Quantity minor_( p_r.second(1) > p_r.second(0) ? p_r.second(0) : p_r.second(1) );
				Quantity major_( p_r.second(1) < p_r.second(0) ? p_r.second(0) : p_r.second(1) );
				ellipse = new AnnEllipse( p_r.first(0), p_r.first(1), major_, minor_, rot, cs, shape, stokes );
			} catch ( AipsError &e ) {
				cerr << "Error encountered creating an AnnEllipse:" << endl;
				cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
			} catch ( ... ) {
				cerr << "Error encountered creating an AnnEllipse..." << endl;
			}

			return ellipse;
		}

		void Ellipse::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
		                                    std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::EllipseRegion;
			region::RegionTypes x;
			Rectangle::fetch_region_details( x, pixel_pts, world_pts );
		}


		void Ellipse::drawRegion( bool selected ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			double center_x, center_y;
			linearCenter( center_x, center_y );

			int x1, y1, x2, y2;
			int cx, cy;
			try {
				linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );
			} catch(...) {
				return;
			}
			try {
				linear_to_screen( wc_, center_x, center_y, cx, cy );
			} catch(...) {
				return;
			}

			pc->drawEllipse(cx, cy, cx - x1, cy - y1, 0.0, True, 1.0, 1.0);

			if (getDrawCenter())
				drawCenter( center_x_, center_y_, center_delta_x_, center_delta_y_);

			if ( selected && ! creating_region ) {

				// draw outline rectangle for resizing the ellipse...
				pushDrawingEnv(region::DotLine);
				pc->drawRectangle( x1, y1, x2, y2 );
				popDrawingEnv( );

				Int w = x2 - x1;
				Int h = y2 - y1;

				Int s = 0;		// handle size
				if (w>=18 && h>=18) s = 6;
				else if (w>=15 && h>=15) s = 5;
				else if (w>=12 && h>=12) s = 4;
				else if (w>=9 && h>=9) s = 3;

				double xdx, ydy;
				try {
					screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
				} catch(...) {
					return;
				}
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

		unsigned int Ellipse::mouseMovement( double x, double y, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

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
			} else if ( selected_ == true ) {
				weaklyUnselect( );
				mouse_in_region = false;
				selected_ = false;
				draw( other_selected );
				result |= region::MouseRefresh;
			}
			return result;
		}

		std::list<shared_ptr<RegionInfo> > *Ellipse::generate_dds_centers( ) {
			std::list<shared_ptr<RegionInfo> > *region_centers = new std::list<shared_ptr<RegionInfo> >( );

			if( wc_==0 ) return region_centers;

			Int zindex = 0;
			if (wc_->restrictionBuffer()->exists("zIndex")) {
				wc_->restrictionBuffer()->getValue("zIndex", zindex);
			}

			double blcx, blcy, trcx, trcy;
			boundingRectangle( blcx, blcy, trcx, trcy );

			DisplayData *dd = 0;
			const std::list<DisplayData*> &dds = wc_->displaylist( );
			Vector<Double> lin(2), blc(2), center(2);

			lin(0) = blcx;
			lin(1) = blcy;
			if ( ! wc_->linToWorld(blc, lin)) return region_centers;

			double center_x, center_y;
			linearCenter( center_x, center_y );
			lin(0) = center_x;
			lin(1) = center_y;
			if ( ! wc_->linToWorld(center, lin)) return region_centers;

			std::string errMsg_;
			std::map<String,bool> processed;
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				dd = *ddi;

				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
				if (padd==0) continue;

				try {
					if ( ! padd->conformsTo(*wc_) ) continue;

					shared_ptr<ImageInterface<Float> > image (padd->imageinterface( ));

					if ( ! image ) continue;

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

					// select the visible layer in the third and all
					// hidden axes with a WCBox and a SubImage
					Quantum<Double> px0(0.,"pix");
					Vector<Quantum<Double> > blcq(nAxes,px0), trcq(nAxes,px0);
					for (Int ax = 0; ax < nAxes; ax++) {
						if ( ax == dispAxes[0] || ax == dispAxes[1]) {
							trcq[ax].setValue(shp[ax]-1);
						} else  {
							blcq[ax].setValue(pos[ax]);
							trcq[ax].setValue(pos[ax]);
						}
					}
					WCBox box(blcq, trcq, cs, Vector<Int>());
					ImageRegion     *imgbox = new ImageRegion(box);
					shared_ptr<SubImage<Float> > boxImg(new SubImage<Float>(*image, *imgbox));

					// generate the WCEllipsoide
					//Quantum<Double> px0(0.,"pix");
					Vector<Quantum<Double> > centerq(2,px0), radiiq(2,px0);
					const Vector<String> &units = wc_->worldAxisUnits( );

					centerq[0].setValue(center[0]);
					centerq[0].setUnit(units[0]);
					centerq[1].setValue(center[1]);
					centerq[1].setUnit(units[1]);

					Quantum<Double> _blc_1_(blc[0],units[0]);
					radiiq[0] = centerq[0] - _blc_1_;
					radiiq[0].setValue(fabs(radiiq[0].getValue( )));

					Quantum<Double> _blc_2_(blc[1],units[1]);
					radiiq[1] = centerq[1] - _blc_2_;
					radiiq[1].setValue(fabs(radiiq[1].getValue( )));

					cout << "centerq: " << centerq << endl;
					cout << "radiiq:  " << radiiq << endl;
					// This is a 2D ellipse (which is the same sort of ellipse that is created via
					// the new annotaitons). I don't know how one creates an elliptical column (which
					// extends the 2D ellipse through all spectral channels) that is analogous to
					// what is done for rectangles... must consult the delphic oracle when the
					// need arises... <drs>
					WCEllipsoid ellipse( centerq, radiiq, IPosition(dispAxes), cs);
					ImageRegion *imageregion = new ImageRegion(ellipse);

					region_centers->push_back(
						shared_ptr<RegionInfo>(
							new ImageRegionInfo(
								name,description,
								getLayerCenter(padd,boxImg,*imageregion)
							)
						)
					);
					delete imgbox;
					delete imageregion;
				} catch (const casa::AipsError& err) {
					errMsg_ = err.getMesg();
					fprintf( stderr, "Ellipse::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				} catch (...) {
					errMsg_ = "Unknown error converting region";
					fprintf( stderr, "Ellipse::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				}
			}
			return region_centers;
		}

		std::pair<Vector<Quantity>,Vector<Quantity> > Ellipse::posAndRadii( ) const {

			Vector<Double> lin(2), blc(2), center(2);
			double blcx, blcy, trcx, trcy;
			boundingRectangle( blcx, blcy, trcx, trcy );

			lin(0) = blcx;
			lin(1) = blcy;
			if ( ! wc_->linToWorld(blc, lin)) return std::pair<Vector<Quantity>,Vector<Quantity> >( );

			double center_x, center_y;
			linearCenter( center_x, center_y );
			lin(0) = center_x;
			lin(1) = center_y;
			if ( ! wc_->linToWorld(center, lin)) return std::pair<Vector<Quantity>,Vector<Quantity> >( );

			Vector<Quantum<Double> > qcenter, qblc;

			qcenter.resize(2);
			qblc.resize(2);

			const Vector<String> &units = wc_->worldAxisUnits( );

			qcenter[0] = Quantum<Double>(center[0], units[0]);
			qcenter[1] = Quantum<Double>(center[1], units[1]);

			qblc[0] = Quantum<Double>(blc[0], units[0]);
			qblc[1] = Quantum<Double>(blc[1], units[1]);

			Vector<Quantity> radii_(2);

			MDirection::Types cccs = current_casa_coordsys( );
			if ( cccs == MDirection::N_Types ) {
				// no direction coordinate was found...
				radii_[0] = qcenter[0] > qblc[0] ? qcenter[0] - qblc[0] : qblc[0] - qcenter[0];
				radii_[1] = qcenter[1] > qblc[1] ? qcenter[1] - qblc[1] : qblc[1] - qcenter[1];

			} else {
				// a direction coordinate was found...

				Vector<Double> center_rad(2);
				center_rad[0] = qcenter[0].getValue("rad");
				center_rad[1] = qcenter[1].getValue("rad");
				MDirection mdcenter( Quantum<Vector<Double> >(center_rad,"rad"), cccs );

				Vector<Double> blc_rad_x(2);
				blc_rad_x[0] = qblc[0].getValue("rad");
				blc_rad_x[1] = qcenter[1].getValue("rad");
				MDirection mdblc_x( Quantum<Vector<Double> >(blc_rad_x,"rad"),cccs );

				Vector<Double> blc_rad_y(2);
				blc_rad_y[0] = qcenter[0].getValue("rad");
				blc_rad_y[1] = qblc[1].getValue("rad");
				MDirection mdblc_y( Quantum<Vector<Double> >(blc_rad_y,"rad"),cccs );

				double xdistance = mdcenter.getValue( ).separation(mdblc_x.getValue( ));
				double ydistance = mdcenter.getValue( ).separation(mdblc_y.getValue( ));
				radii_[0] = Quantity(xdistance,"rad");
				radii_[1] = Quantity(ydistance,"rad");
            }

			return std::make_pair(qcenter,radii_);
		}

		ImageRegion *Ellipse::get_image_region( DisplayData *dd ) const {

			if( wc_==0 ) return 0;

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
			if ( padd == 0 ) return 0;

			shared_ptr<ImageInterface<Float> > image( padd->imageinterface( ));
			Vector<Int> dispAxes = padd->displayAxes( );
			dispAxes.resize(2,True);

			std::pair<Vector<Quantity>,Vector<Quantity> > p_r = posAndRadii( );
			if ( p_r.second.size( ) != 2 ) return 0;

			ImageRegion *result = 0;
			try {
				WCEllipsoid ellipse( p_r.first, p_r.second, IPosition(dispAxes), wc_->coordinateSystem( ));
				result = new ImageRegion(ellipse);
			} catch(...) { }
			return result;
		}

		bool Ellipse::output_region( ds9writer &out, WorldCanvas *, const std::vector<std::pair<double,double> > &pts ) const {
			return out.ellipse(wc_,pts);
		}

	}

}
