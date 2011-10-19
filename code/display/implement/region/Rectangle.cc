#include <display/region/Rectangle.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <vector>

#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/PanelDisplay.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Regions/WCBox.h>

#include <imageanalysis/Annotations/AnnRectBox.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

#include <display/DisplayDatas/MSAsRaster.h>

namespace casa {
    namespace viewer {

	Rectangle::~Rectangle( ) { }


	int Rectangle::clickHandle( double x, double y ) const {
	    if ( visible_ == false ) return 0;
	    bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
	    bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
	    bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
	    bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;
	    return trc ? 1 : brc ? 2 : blc ? 3 : tlc ? 4 : 0;
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
	    updateStateInfo( true );
	    return handle;
	}

	void Rectangle::regionCenter( double &x, double &y ) const {
	    x = linear_average(blc_x,trc_x);
	    y = linear_average(blc_y,trc_y);
	}

	AnnotationBase *Rectangle::annotation( ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );

	    double wblc_x, wblc_y, wtrc_x, wtrc_y;
	    linear_to_world( wc_, blc_x, blc_y, trc_x, trc_y, wblc_x, wblc_y, wtrc_x, wtrc_y );
	    const Vector<String> &units = wc_->worldAxisUnits( );

	    Quantity qblc_x( wblc_x, units[0] );
	    Quantity qblc_y( wblc_y, units[1] );
	    Quantity qtrc_x( wtrc_x, units[0] );
	    Quantity qtrc_y( wtrc_y, units[1] );

	    const DisplayData *dd = wc_->displaylist().front();


	    Vector<Stokes::StokesTypes> stokes;
	    Int polaxis = CoordinateUtil::findStokesAxis(stokes, cs);

	    AnnRectBox *box = 0;

	    try {
		std::vector<int> axes = dd->displayAxes( );
		IPosition shape(cs.nPixelAxes( ));
		for ( int i=0; i < shape.size( ); ++i )
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

	void Rectangle::fetch_region_details( RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
					      std::vector<std::pair<double,double> > &world_pts ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    type = RectRegion;

	    double wblc_x, wblc_y, wtrc_x, wtrc_y;
	    linear_to_world( wc_, blc_x, blc_y, trc_x, trc_y, wblc_x, wblc_y, wtrc_x, wtrc_y );

	    int pblc_x, pblc_y, ptrc_x, ptrc_y;
	    linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, pblc_x, pblc_y, ptrc_x, ptrc_y );

	    pixel_pts.resize(2);
	    pixel_pts[0].first = pblc_x;
	    pixel_pts[0].second = pblc_y;
	    pixel_pts[1].first = ptrc_x;
	    pixel_pts[1].second = ptrc_y;

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
	    linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );
	    pc->drawRectangle( x1, y1, x2, y2 );

	    if ( selected ) {
		Int w = x2 - x1;
		Int h = y2 - y1;

		Int s = 0;		// handle size
		if (w>=35 && h>=35) s = 6;
		else if (w>=20 && h>=20) s = 4;
		else if (w>= 9 && h>= 9) s = 3;

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
		  pushDrawingEnv( Region::SolidLine);
		  pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
		  pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
		  pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
		  pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
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

	int Rectangle::mouseMovement( double x, double y, bool other_selected ) {
	    int result = 0;

	    if ( visible_ == false ) return result;

	    //if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y ) {
	    if ( x > blc_x && x < trc_x && y > blc_y && y < trc_y || within_vertex_handle( x, y )) {
		result |= MouseSelected;
		result |= MouseRefresh;
		selected_ = true;
		draw( );
		if ( other_selected == false ) {
		    // mark flag as this is the region (how to mix in other shapes)
		    // of interest for statistics updates...
		    selectedInCanvas( );
		}
	    } else if ( selected_ == true ) {
		selected_ = false;
		draw( );
		result |= MouseRefresh;
	    }
	    return result;
	}


	RegionInfo::stats_t *Rectangle::get_ms_stats( MSAsRaster *msar, double x, double y ) {

	    RegionInfo::stats_t *result = new RegionInfo::stats_t( );
	  
	    Vector<Double> pos(2);
	    linear_to_world( wc_, x, y, pos[0], pos[1] );

	    bool ok = msar->showPosition( *result, pos );
	    return result;
	}


	std::list<RegionInfo> *Rectangle::generate_dds_statistics(  ) {
	    std::list<RegionInfo> *region_statistics = new std::list<RegionInfo>( );
	    if( wc_==0 ) return region_statistics;

	    Int zindex = 0;
	    if (wc_->restrictionBuffer()->exists("zIndex")) {
		wc_->restrictionBuffer()->getValue("zIndex", zindex);
	    }

	    DisplayData *dd = 0;
	    const std::list<DisplayData*> &dds = wc_->displaylist( );
	    Vector<Double> lin(2), blc(2), trc(2);

	    lin(0) = blc_x;
	    lin(1) = blc_y;
	    if ( ! wc_->linToWorld(blc, lin)) return region_statistics;

	    lin(0) = trc_x;
	    lin(1) = trc_y;
	    if ( ! wc_->linToWorld(trc, lin)) return region_statistics;

	    std::string errMsg_;
	    std::map<String,bool> processed;
	    for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
		dd = *ddi;

		PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
		if (padd==0) {
		    MSAsRaster *msar =  dynamic_cast<MSAsRaster*>(dd);
		    if ( msar != 0 ) {
			RegionInfo::stats_t *blc_stats = get_ms_stats( msar, blc_x, blc_y );
			RegionInfo::stats_t *trc_stats = get_ms_stats( msar, trc_x, trc_y );
			String full_ms_name = msar->name( );
			region_statistics->push_back(MsRegionInfo(full_ms_name + " [blc]",blc_stats));
			region_statistics->push_back(MsRegionInfo(full_ms_name + " [trc]",trc_stats));
		    }
		    continue;
		}

		try {
		    if ( ! padd->conformsTo(*wc_) ) continue;

		    ImageInterface<Float> *image = padd->imageinterface( );

		    if ( image == 0 ) continue;

		    String full_image_name = image->name(false);
		    std::map<String,bool>::iterator repeat = processed.find(full_image_name);
		    if (repeat != processed.end()) continue;
		    processed.insert(std::map<String,bool>::value_type(full_image_name,true));

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

		    Int spaxis = getAxisIndex( image, String("Spectral") );
		    for (Int ax = 0; ax < nAxes; ax++) {
			if ( ax == dispAxes[0] || ax == dispAxes[1] || ax == spaxis) {
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

		    region_statistics->push_back(ImageRegionInfo(full_image_name,getLayerStats(padd,image,*imageregion)));
		    delete imageregion;

		} catch (const casa::AipsError& err) {
		    errMsg_ = err.getMesg();
		    continue;
		} catch (...) {
		    errMsg_ = "Unknown error converting region";
		    continue;
		}
	    }
	    return region_statistics;
	}

	void Rectangle::boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const {
	    blcx = blc_x;
	    blcy = blc_y;
	    trcx = trc_x;
	    trcy = trc_y;
	}

    }

}
