#include <display/region/Region.h>
#include <casa/Quanta/MVAngle.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <images/Images/SubImage.h>
#include <measures/Measures/MCDirection.h>

#include <images/Images/ImageStatistics.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <math.h>
#include <algorithm>

extern "C" void casa_viewer_pure_virtual( const char *file, int line, const char *func ) {
    fprintf( stderr, "%s:%d pure virtual '%s( )' called...\n", file, line, func );
}

namespace casa {
    namespace viewer {

	Region::Region( WorldCanvas *wc ) :  wc_(wc), selected_(false), visible_(true) {
	    last_z_index = wc_ == 0 ? 0 : wc_->zIndex( );
	    // if ( wc_->restrictionBuffer()->exists("zIndex")) {
	    // 	wc_->restrictionBuffer()->getValue("zIndex", last_z_index);
	    // }
	}

	int Region::zIndex( ) const { return wc_ == 0 ? last_z_index : wc_->zIndex( ); }

	void Region::setDrawingEnv( ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if(pc==0) return;

	    Int x0 = wc_->canvasXOffset();  Int x1 = x0 + wc_->canvasXSize() - 1;
	    Int y0 = wc_->canvasYOffset();  Int y1 = y0 + wc_->canvasYSize() - 1;

	    pc->setClipWindow(x0,y0, x1,y1);
	    pc->enable(Display::ClipWindow);

	    pc->setLineWidth(1);
	    pc->setCapStyle(Display::CSRound);
	    pc->setColor(lineColor());
	    pc->setLineWidth(lineWidth());

	    Display::LineStyle current_ls = pc->getLineStyle( );
	    switch ( current_ls ) {
		case Display::LSSolid:
		    ls_stack.push_back(SolidLine);
		    break;
		case Display::LSDashed:
		    ls_stack.push_back(DashLine);
		    break;
		case Display::LSDoubleDashed:
		    ls_stack.push_back(LSDoubleDashed);
		    break;
	    }

	    LineStyle linestyle = lineStyle( );
	    set_line_style( linestyle );

	    pc->setDrawFunction(Display::DFCopy);
	}

	void Region::resetDrawingEnv( ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if ( pc == 0 ) return;

	    pc->disable(Display::ClipWindow);

	    set_line_style( ls_stack.back( ) );
	    ls_stack.pop_back( );

	    if ( ls_stack.size( ) != 0 )
		throw internal_error( "Region stack inconsistency" );
	}

	void Region::setTextEnv( ) {
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if ( pc == 0 ) return;
	    pc->setColor(textColor( ));
	    QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
	    if ( qpc != 0 ) {
		int text_mod = textFontStyle( );
		qpc->setFont( textFont( ), textFontSize( ),
			      text_mod & BoldText ? true : false,
			      text_mod & ItalicText ? true : false );
	    }
	}

	void Region::resetTextEnv( ) {
	}

	void Region::pushDrawingEnv( LineStyle ls ) {
	    ls_stack.push_back(current_ls);
	    set_line_style( ls );
	}

	void Region::popDrawingEnv( ) {
	    set_line_style( ls_stack.back( ) );
	    ls_stack.pop_back( );
	}

	void Region::refresh( ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if ( pc == 0 ) return;
	    pc->copyBackBufferToFrontBuffer();
	    pc->setDrawBuffer(Display::FrontBuffer);
	    pc->callRefreshEventHandlers(Display::BackCopiedToFront);
	}

	void Region::draw( ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    // When stepping through a cube, this detects that a different plane is being displayed...
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    int new_z_index = wc_->zIndex( );
	    int z_min, z_max;
	    zRange(z_min,z_max);

	    if ( new_z_index < z_min || new_z_index > z_max ) {
		visible_ = false;
		clearStatistics( );
		return;
	    } else {
		visible_ = true;
	    }

	    if ( new_z_index != last_z_index ) {
		updateStateInfo(true);
	    }
	    last_z_index = new_z_index;
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

	    setDrawingEnv( );
	    drawRegion( selected( ) || marked( ) );
	    resetDrawingEnv( );

	    setTextEnv( );
	    drawText( );
	    resetTextEnv( );
	}

	void Region::drawText( ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();

	    double lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y;
	    boundingRectangle( lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y );
	    int blc_x, blc_y, trc_x, trc_y;
	    linear_to_screen( wc_, lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y, blc_x, blc_y, trc_x, trc_y );

	    int x, y;
	    int dx, dy;
	    textPositionDelta( dx, dy );

	    int m_angle = 0;
	    TextPosition tp = textPosition( );
	    std::string text = textValue( );
	    int text_height = pc->textHeight(text);
	    int text_width = pc->textWidth(text);


	    Display::TextAlign alignment = Display::AlignCenter;
	    int midx, midy;

	    const int offset = 5;
	    switch ( tp ) {
		case TopText:
		    alignment = Display::AlignBottomLeft;
		    y = trc_y + offset;
		    x = (int) (((double) (blc_x + trc_x - text_width) + 0.5) / 2.0);
		    break;
		case RightText:
		    alignment = Display::AlignBottomLeft;
		    x = trc_x + offset;
		    y = (int) (((double) (blc_y + trc_y - text_height) + 0.5) / 2.0);
		    break;
		case BottomText:
		    alignment = Display::AlignTopLeft;
		    y = blc_y - offset;
		    x = (int) (((double) (blc_x + trc_x - text_width) + 0.5) / 2.0);
		    break;
		case LeftText:
		    alignment = Display::AlignBottomRight;
		    x = blc_x - offset;
		    y = (int) (((double) (blc_y + trc_y - text_height) + 0.5) / 2.0);
		    break;
	    }

	    pc->drawText( x + dx, y + dy, text, m_angle, alignment );
	}

	static std::string as_string( double v ) {
	    char buf[256];
	    sprintf( buf, "%g", v );
	    return std::string(buf);
	}

	static std::string as_string( int v ) {
	    char buf[256];
	    sprintf( buf, "%d", v );
	    return std::string(buf);
	}

	// region units as string... note, for HMS, DMS radian conversion is first done...
	static inline const char *as_string( Region::Units units ) {
	    return units == Region::Degrees ? "deg" : "rad";
	}

      void Region::getCoordinatesAndUnits( Region::Coord &c, Region::Units &x_units, Region::Units &y_units, std::string &width_height_units ) const {
	    c = current_region_coordsys( );
	    x_units = current_xunits( );
	    y_units = current_yunits( );
	    width_height_units = (x_units == Radians ? "rad" : "deg");
	}

	static inline double wrap_angle( double before, double after ) {
	    const double UNIT_WRAPAROUND = 2.0 * M_PI;
	    if ( after < 0 && before > 0 )
		return after + UNIT_WRAPAROUND;
	    else if ( after > 0 && before < 0 )
		return after - UNIT_WRAPAROUND;
	    return after;
	}

	static inline Region::Coord casa_to_viewer( MDirection::Types type ) {
	    return type == MDirection::J2000 ? Region::J2000 :
		type == MDirection::B1950 ? Region::B1950 :
		type == MDirection::GALACTIC ? Region::Galactic :
		type == MDirection::SUPERGAL ? Region::SuperGalactic :
		type == MDirection::ECLIPTIC ? Region::Ecliptic : Region::J2000;
	}

	static inline MDirection::Types viewer_to_casa( Region::Coord type ) {
	    return type == Region::J2000 ?  MDirection::J2000 :
		type == Region::B1950 ?  MDirection::B1950 :
		type == Region::Galactic ? MDirection::GALACTIC :
		type == Region::SuperGalactic ? MDirection::SUPERGAL :
		type == Region::Ecliptic ? MDirection::ECLIPTIC : MDirection::J2000;
	}

	static inline MDirection::Types string_to_casa_coordsys( const std::string &s ) {
	    return s == "J2000" ? MDirection::J2000 :
		   s == "B1950" ? MDirection::B1950 :
		   s == "galactic" ? MDirection::GALACTIC :
		   s == "super galactic" ? MDirection::SUPERGAL :
		   s == "ecliptic" ? MDirection::ECLIPTIC : MDirection::J2000;
	}

	static inline String string_to_casa_units( const std::string &s ) {
	    return s == "degrees" ? "deg" : "rad";
	}


	// static inline void convert_units( Quantum<Vector<double> > &q, Region::Units new_units ) {
	//     q.convert( as_string(new_units) );
	// }

	static inline void convert_units( double &x, const std::string &xunits, Region::Units new_x_units, double &y, const std::string &yunits, Region::Units new_y_units ) {
	    Quantum<double> resultx(x, xunits.c_str( ));
	    Quantum<double> resulty(y, yunits.c_str( ));
	    x = resultx.getValue(as_string(new_x_units));
	    y = resulty.getValue(as_string(new_y_units));
	}

	static inline Quantum<Vector<double> > convert_angle( double x, const std::string &xunits, double y, const std::string &yunits,
							      MDirection::Types original_coordsys, MDirection::Types new_coordsys, const std::string &new_units="rad" ) {
	    Quantum<double> xq(x,String(xunits));
	    Quantum<double> yq(y,String(yunits));
	    MDirection md = MDirection::Convert(MDirection(xq,yq,original_coordsys), new_coordsys)();
	    casa::Quantum<casa::Vector<double> > result = md.getAngle("rad");
	    xq.convert("rad");
	    yq.convert("rad");
	    result.getValue( )(0) = wrap_angle(xq.getValue( ), result.getValue( )(0));
	    result.getValue( )(1) = wrap_angle(yq.getValue( ), result.getValue( )(1));
	    result.convert(String(new_units));
	    return result;
	}


	void Region::getPositionString( std::string &x, std::string &y, std::string &angle,
					double &bounding_width, double &bounding_height, Region::Coord coord,
					Region::Units new_x_units, Region::Units new_y_units, const std::string &bounding_units ) const {
	    if ( wc_ == 0 ) {
		x = y = angle = "internal error";
		return;
	    }

	    if ( wc_->csMaster() == 0 ) {
		x = y = angle = "no wcs";
		return;
	    }

	    if ( coord == DefaultCoord ) coord = current_region_coordsys( );
	    if ( new_x_units == DefaultUnits ) new_x_units = current_xunits( );
	    if ( new_y_units == DefaultUnits ) new_y_units = current_xunits( );
	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    MDirection::Types cccs = current_casa_coordsys( );

	    if ( cccs == MDirection::N_Types ) {
		// this impiles that the coordinate system does not have a direction coordinate...
		// so it is probably a measurement set... treat as a pixel coordinate for now...
		double center_x, center_y;
		linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
		x = as_string(center_x);
		y = as_string(center_y);

		// set bounding width/height
		double p_blc_x, p_blc_y, p_trc_x, p_trc_y;
		linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, p_blc_x, p_blc_y, p_trc_x, p_trc_y );
		bounding_width = fabs(p_trc_x-p_blc_x);
		bounding_height = fabs(p_trc_y-p_blc_y);

		angle = as_string(0);
		return;
	    }

	    Coord cvcs = casa_to_viewer(cccs);
	    double result_x, result_y;
	    const Vector<String> &units = wc_->worldAxisUnits();

	    // seting bounding units
	    if ( bounding_units == "pixel" ) {
		double p_blc_x, p_blc_y, p_trc_x, p_trc_y;
		linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, p_blc_x, p_blc_y, p_trc_x, p_trc_y );
		bounding_width = fabs(p_trc_x-p_blc_x);
		bounding_height = fabs(p_trc_y-p_blc_y);
	    } else {
		linear_offset_to_world_offset(wc_,fabs(trc_x-blc_x),fabs(trc_y-blc_y), viewer_to_casa(coord), bounding_units, bounding_width, bounding_height);
	    }

	    if ( coord == cvcs ) {
		linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
		convert_units( result_x, units[0], new_x_units, result_y, units[1], new_y_units );

	    } else {
		linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
		Quantum<Vector<double> > result = convert_angle( result_x, units[0], result_y, units[1], cccs, viewer_to_casa(coord) );
		result_x = result.getValue(as_string(new_x_units))(0);
		result_y = result.getValue(as_string(new_y_units))(1);

	    }

	    const Vector<String> &axis_labels = wc_->worldAxisNames( );
	    if ( new_x_units == Pixel ) {
		double center_x, center_y;
		linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
		x = as_string(center_x);
	    } else if ( new_x_units == Sexagesimal ) {
		if ( axis_labels(0) == "Declination" )
		    x = MVAngle(result_x)(0.0).string(MVAngle::ANGLE_CLEAN,8);
		else
		    x = MVAngle(result_x)(0.0).string(MVAngle::TIME,8);
	    } else {
		x = as_string(result_x);
	    }

	    if ( new_y_units == Pixel ) {
		double center_x, center_y;
		linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
		y = as_string(center_y);
	    } else if ( new_y_units == Sexagesimal ) {
		if ( axis_labels(1) == "Declination" )
		    y = MVAngle(result_y)(0.0).string(MVAngle::ANGLE_CLEAN,8);
		else
		    y = MVAngle(result_y)(0.0).string(MVAngle::TIME,8);
	    } else {
		y = as_string(result_y);
	    }
	    angle = "";

	}

	void Region::movePosition( const std::string &x, const std::string &y, const std::string &coord,
				   const std::string &x_units, const std::string &y_units,
				   const std::string &width, const std::string &height, const std::string &bounding_units ) {

	    const double dim_threshold = 0.5;

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    // current center of region...
	    double cur_center_x = linear_average(blc_x,trc_x);
	    double cur_center_y = linear_average(blc_y,trc_y);
	    double cur_width = fabs(trc_x-blc_x);
	    double cur_height = fabs(trc_y-blc_y);
	    // filled in below...
	    double new_center_x, new_center_y;
	    double new_width_inc=0, new_height_inc=0;

	    if ( coord == "pixel" ) {

		double pix_x = atof(x.c_str( ));
		double pix_y = atof(y.c_str( ));

		pixel_to_linear( wc_, pix_x, pix_y, new_center_x, new_center_y );

		double x_off = atof(width.c_str( ))/2.0;
		double y_off = atof(height.c_str( ))/2.0;

		double dd_blc_x, dd_blc_y, dd_trc_x, dd_trc_y;

		pixel_to_linear( wc_, cur_center_x - x_off, cur_center_y - y_off, cur_center_x + x_off, cur_center_y + y_off, dd_blc_x, dd_blc_y, dd_trc_x, dd_trc_y );
		double new_blc_x = (dd_blc_x <= dd_trc_x ? dd_blc_x : dd_trc_x);
		double new_blc_y = (dd_blc_y <= dd_trc_y ? dd_blc_y : dd_trc_y);
		double new_trc_x = (dd_blc_x >= dd_trc_x ? dd_blc_x : dd_trc_x);
		double new_trc_y = (dd_blc_y >= dd_trc_y ? dd_blc_y : dd_trc_y);

		double x_change = fabs(new_trc_x - new_blc_x) - fabs(trc_x - blc_x);
		double y_change = fabs(new_trc_y - new_blc_y) - fabs(trc_y - blc_y);
		if ( fabs(x_change) >= dim_threshold )
		    new_width_inc = x_change;
		if ( fabs(y_change) >= dim_threshold )
		    new_height_inc = y_change;

	    } else {
		Quantity xq, yq;

		// read in to convert HMS/DMS...
		MVAngle::read(xq,x);
		MVAngle::read(yq,y);
		// the above results in radians... fix in case of degrees...
		if ( x_units == "degrees" )
		    xq = Quantity(xq.getValue(),string_to_casa_units(x_units));
		if ( y_units == "degrees" )
		    yq = Quantity(yq.getValue(),string_to_casa_units(y_units));

		// convert to radians, for wrap_angle( ) below...
		xq.convert("rad");
		yq.convert("rad");

		// center specified by the user...
		MDirection orig_direction(xq,yq,string_to_casa_coordsys(coord));
		Quantum<Vector<double> > orig_center = orig_direction.getAngle("rad");
		Quantity xoff(atof(width.c_str( ))/2.0,bounding_units.c_str( ));
		Quantity yoff(atof(height.c_str( ))/2.0,bounding_units.c_str( ));

		double orig_center_x = orig_center.getValue("rad")(0);
		double orig_center_y = orig_center.getValue("rad")(1);

		MDirection orig_blc_md( Quantity(orig_center_x - xoff.getValue("rad"),"rad"), Quantity(orig_center_y - yoff.getValue("rad"),"rad") );
		MDirection orig_trc_md( Quantity(orig_center_x + xoff.getValue("rad"),"rad"), Quantity(orig_center_y + yoff.getValue("rad"),"rad") );
		MDirection blc_md = MDirection::Convert(orig_blc_md,current_casa_coordsys( ))( );
		casa::Quantum<casa::Vector<double> > blc = blc_md.getAngle("rad");
		MDirection trc_md = MDirection::Convert(orig_trc_md,current_casa_coordsys( ))( );
		casa::Quantum<casa::Vector<double> > trc = trc_md.getAngle("rad");

		double dd_blc_x, dd_blc_y, dd_trc_x, dd_trc_y;
		try {
		    world_to_linear( wc_, blc.getValue("rad")(0), blc.getValue("rad")(1), trc.getValue("rad")(0), trc.getValue("rad")(1), dd_blc_x, dd_blc_y, dd_trc_x, dd_trc_y );
		} catch(...) {
		    // conversion failed...
		    // need a viewer status line...
		    updateStateInfo( true );
		    return;
		}

		double new_blc_x = (dd_blc_x <= dd_trc_x ? dd_blc_x : dd_trc_x);
		double new_blc_y = (dd_blc_y <= dd_trc_y ? dd_blc_y : dd_trc_y);
		double new_trc_x = (dd_blc_x >= dd_trc_x ? dd_blc_x : dd_trc_x);
		double new_trc_y = (dd_blc_y >= dd_trc_y ? dd_blc_y : dd_trc_y);

		double x_change = fabs(new_trc_x - new_blc_x) - fabs(trc_x - blc_x);
		double y_change = fabs(new_trc_y - new_blc_y) - fabs(trc_y - blc_y);
		if ( fabs(x_change) >= dim_threshold )
		    new_width_inc = x_change;
		if ( fabs(y_change) >= dim_threshold )
		    new_height_inc = y_change;

		// convert world coordinates to current coordinate system...
		MDirection md = MDirection::Convert(orig_direction,current_casa_coordsys( ))( );

		// cout << "Viewer MDirection:\t" << md << endl;

		// fetch world coordinates in radians...
		// and make angles consistent with input...
		casa::Quantum<casa::Vector<double> > worldq = md.getAngle("rad");
		new_center_x = wrap_angle(xq.getValue("rad"),worldq.getValue()(0));
		new_center_y = wrap_angle(yq.getValue("rad"),worldq.getValue()(1));

		// convert new center to linear coordinates...
		world_to_linear( wc_, new_center_x, new_center_y, new_center_x, new_center_y );

	    }

	    // trap attempts to move region out of visible area...
	    if ( ! valid_translation( new_center_x - cur_center_x, new_center_y - cur_center_y,
				      new_width_inc, new_height_inc ) ) {
		updateStateInfo( true ); 
		return;
	    }

	    // move region...
	    move( new_center_x - cur_center_x, new_center_y - cur_center_y );

	    if ( new_width_inc != 0 || new_height_inc != 0 ) {
		resize( new_width_inc, new_height_inc );
	    }

	    refresh( );
	}

	Region::Coord Region::current_region_coordsys( ) const {
	    return casa_to_viewer(current_casa_coordsys( ));
	}

	MDirection::Types Region::current_casa_coordsys( ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return MDirection::J2000;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );
	    int index = cs.findCoordinate(Coordinate::DIRECTION);
	    if ( index < 0 ) {
		// no direction coordinate...
		return MDirection::N_Types;
	    }
	    return cs.directionCoordinate(index).directionType(true);
	}

	Region::Units Region::current_xunits( ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return Degrees;
	    const Vector<String> &units = wc_->worldAxisUnits();
	    if ( units(0) == "rad" )
		return Radians;
	    if ( units(0) == "deg" )
		return Degrees;
	    return Degrees;
	}

	Region::Units Region::current_yunits( ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return Degrees;
	    const Vector<String> &units = wc_->worldAxisUnits();
	    if ( units(1) == "rad" )
		return Radians;
	    if ( units(1) == "deg" )
		return Degrees;
	    return Degrees;
	}



	void Region::set_line_style( LineStyle linestyle ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if ( pc == 0 ) return;
	    switch ( linestyle ) {
		case DashLine:
		    pc->setLineStyle( Display::LSDashed );
		    current_ls = DashLine;
		    break;
		case DotLine:
		    {	QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
			if(qpc != NULL) {
			    qpc->setQtLineStyle(Qt::DotLine);
			    current_ls = DotLine;
			} else {
			    pc->setLineStyle( Display::LSDashed );
			    current_ls = DashLine;
			}
		    }
		    break;
		case LSDoubleDashed:
		    pc->setLineStyle(Display::LSDoubleDashed );
		    current_ls = DashLine;
		    break;
		default:
		    pc->setLineStyle(Display::LSSolid);
		    current_ls = SolidLine;
		    break;
	    }
	}

	void linear_to_screen( WorldCanvas *wc, double lin_x1, double lin_y1, int &scr_x1, int &scr_y1 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);

	    // BEGIN - critical section
	    linearv(0) = lin_x1;
	    linearv(1) = lin_y1;
	    if ( ! wc->linToPix( pixelv, linearv ) )
		throw internal_error( "linear to screen conversion failed" );
	    scr_x1 = pixelv(0);
	    scr_y1 = pixelv(1);
	    // END - critical section
	}

	void linear_to_screen( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
				int &scr_x1, int &scr_y1, int &scr_x2, int &scr_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);

	    // BEGIN - critical section
	    linearv(0) = lin_x1;
	    linearv(1) = lin_y1;
	    if ( ! wc->linToPix( pixelv, linearv ) )
		throw internal_error( "linear to screen conversion failed" );
	    scr_x1 = pixelv(0);
	    scr_y1 = pixelv(1);

	    linearv(0) = lin_x2;
	    linearv(1) = lin_y2;
	    if ( ! wc->linToPix( pixelv, linearv ) )
		throw internal_error( "linear to screen conversion failed" );
	    scr_x2 = pixelv(0);
	    scr_y2 = pixelv(1);
	    // END - critical section
	}

	void linear_to_pixel( WorldCanvas *wc, double lin_x1, double lin_y1, double &pix_x1, double &pix_y1 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    double world_x1, world_y1;
	    linear_to_world( wc, lin_x1, lin_y1, world_x1, world_y1 );

	    const CoordinateSystem &cs = wc->coordinateSystem( );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);

	    // BEGIN - critical section
	    if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
		worldv.resize(cs.nWorldAxes( ));
		worldv = cs.referenceValue( );
	    }

	    if ( cs.nPixelAxes( ) != pixelv.nelements( ) ) {
		pixelv.resize(cs.nPixelAxes( ));
		pixelv = cs.referencePixel( );
	    }

	    worldv(0) = world_x1;
	    worldv(1) = world_y1;

	    if ( ! cs.toPixel( pixelv, worldv ) )
		throw internal_error( "linear to pixel conversion failed" );

	    pix_x1 = pixelv(0);
	    pix_y1 = pixelv(1);
	    // END - critical section
	}

	void linear_to_pixel( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
				double &pix_x1, double &pix_y1, double &pix_x2, double &pix_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    double world_x1, world_y1, world_x2, world_y2;
	    linear_to_world( wc, lin_x1, lin_y1, lin_x2, lin_y2, world_x1, world_y1, world_x2, world_y2 );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);

	    const CoordinateSystem &cs = wc->coordinateSystem( );

	    // BEGIN - critical section

	    if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
		worldv.resize(cs.nWorldAxes( ));
		worldv = cs.referenceValue( );
	    }

	    if ( cs.nPixelAxes( ) != pixelv.nelements( ) ) {
		pixelv.resize(cs.nPixelAxes( ));
		pixelv = cs.referencePixel( );
	    }

	    worldv(0) = world_x1;
	    worldv(1) = world_y1;

	    if ( ! cs.toPixel( pixelv, worldv ) )
		throw internal_error( "linear to pixel conversion failed" );

	    pix_x1 = pixelv(0);
	    pix_y1 = pixelv(1);

	    worldv(0) = world_x2;
	    worldv(1) = world_y2;
	    if ( ! cs.toPixel( pixelv, worldv ) )
		throw internal_error( "linear to pixel conversion failed" );

	    pix_x2 = pixelv(0);
	    pix_y2 = pixelv(1);
	    // END - critical section
	}

	void screen_to_linear( WorldCanvas *wc, int scr_x, int scr_y, double &lin_x, double &lin_y ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    // BEGIN - critical section
	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);
	    pixelv(0) = scr_x;
	    pixelv(1) = scr_y;
	    if ( ! wc->pixToLin( linearv, pixelv ) )
		throw internal_error( "pixel to linear conversion failed" );
	    lin_x = linearv(0);
	    lin_y = linearv(1);
	    // END - critical section
	}

	void screen_to_linear( WorldCanvas *wc, int scr_x1, int scr_y1, int scr_x2, int scr_y2,
				 double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);

	    // BEGIN - critical section
	    pixelv(0) = scr_x1;
	    pixelv(1) = scr_y1;
	    if ( ! wc->pixToLin( linearv, pixelv ) )
		throw internal_error( "pixel to linear conversion failed" );

	    lin_x1 = linearv(0);
	    lin_y1 = linearv(1);

	    pixelv(0) = scr_x2;
	    pixelv(1) = scr_y2;
	    if ( ! wc->pixToLin( linearv, pixelv ) )
		throw internal_error( "pixel to linear conversion failed" );
	    lin_x2 = linearv(0);
	    lin_y2 = linearv(1);
	    // END - critical section
	}

	void screen_offset_to_linear_offset( WorldCanvas *wc_, int sx, int sy, double &lx, double &ly ) {
	    const int base = 200;
	    double blcx, blcy, trcx, trcy;
	    screen_to_linear( wc_, base, base, base+sx, base+sy, blcx, blcy, trcx, trcy );
	    lx = trcx - blcx;
	    ly = trcy - blcy;
	}

	void pixel_offset_to_linear_offset( WorldCanvas *wc, double sx, double sy, double &lx, double &ly ) {
	    const int base = 10;
	    double blcx, blcy, trcx, trcy;
	    pixel_to_linear( wc, base, base, base+sx, base+sy, blcx, blcy, trcx, trcy );
	    lx = trcx - blcx;
	    ly = trcy - blcy;
	    return;
	}

	void linear_offset_to_pixel_offset( WorldCanvas *wc_, double lx, double ly, double &px, double &py ) {
	    const int base = 0;
	    double blcx, blcy, trcx, trcy;

	    linear_to_pixel( wc_, base, base, base+lx, base+ly, blcx, blcy, trcx, trcy );
	    px = trcx - blcx;
	    py = trcy - blcy;
	}

	void linear_offset_to_world_offset( WorldCanvas *wc_, double lx, double ly, MDirection::Types coordsys, const std::string &units, double &wx, double &wy ) {
	    const int base = 0;
	    double blcx, blcy, trcx, trcy;

	    linear_to_world( wc_, base, base, base+lx, base+ly, blcx, blcy, trcx, trcy );
	    MDirection::Types cccs = get_coordinate_type( wc_->coordinateSystem( ) );
	    Region::Coord crcs = casa_to_viewer(cccs);
	    if ( coordsys == cccs && units == "rad" ) {
		wx = fabs(trcx-blcx);
		wy = fabs(trcy-blcy);
		return;
	    }

	    Quantum<Vector<double> > blc, trc;
	    if ( coordsys == cccs ) {
		const Vector<String> &units = wc_->worldAxisUnits();
		Vector<double> pts(2);
		pts[0] = blcx;
		pts[1] = blcy;
		blc = Quantum<Vector<double> >(pts,units[0]);
		pts[0] = trcx;
		pts[1] = trcy;
		trc = Quantum<Vector<double> >(pts,units[1]);
	    } else {
		const Vector<String> &units = wc_->worldAxisUnits();
		blc = convert_angle( blcx, units[0], blcy, units[1], cccs, coordsys );
		trc = convert_angle( trcx, units[0], trcy, units[1], cccs, coordsys );
	    }

	    wx = fabs(trc.getValue(units.c_str( ))(0) - blc.getValue(units.c_str( ))(0));
	    wy = fabs(trc.getValue(units.c_str( ))(1) - blc.getValue(units.c_str( ))(1));
	}


	void world_offset_to_linear_offset( WorldCanvas *wc_, MDirection::Types coordsys, const std::string &units, double wx, double wy, double &lx, double &ly ) {

	    double basex = 0;
	    double basey = 0;

	    MDirection::Types cccs = get_coordinate_type( wc_->coordinateSystem( ) );
	    if ( coordsys == cccs && units == "rad" ) {
		double blcx, blcy, trcx, trcy;
		world_to_linear( wc_, basex, basey, basex+wx, basey+wy, blcx, blcy, trcx, trcy );
		lx = fabs(trcx-blcx);
		ly = fabs(trcy-blcy);
		return;
	    }

	    // create blc, trc from offset...
	    Quantum<Vector<double> > blc, trc;

	    // start with offset in specified units...
	    Quantity xoff(wx,units.c_str( ));
	    Quantity yoff(wy,units.c_str( ));

	    // create blc and trc in radians (with converted offsets)...
	    Vector<double> pts(2);
	    pts[0] = basex;
	    pts[1] = basey;
	    blc = Quantum<Vector<double> >(pts,"rad");
	    pts[0] = basex + xoff.getValue("rad");
	    pts[1] = basey + yoff.getValue("rad");
	    trc = Quantum<Vector<double> >(pts,"rad");

	    // convert from the specified coordinate system to the viewer's coordinate system...
	    if ( coordsys != cccs ) {
		blc = convert_angle( blc.getValue("rad")(0), "rad", blc.getValue("rad")(1), "rad", coordsys, cccs );
		trc = convert_angle( trc.getValue("rad")(0), "rad", trc.getValue("rad")(1), "rad", coordsys, cccs );
	    }

	    double blcx, blcy, trcx, trcy;
	    world_to_linear( wc_, blc.getValue("rad")(0), blc.getValue("rad")(1), trc.getValue("rad")(0), trc.getValue("rad")(1), blcx, blcy, trcx, trcy );
	    lx = fabs(trcx-blcx);
	    ly = fabs(trcy-blcy);

	}


	void linear_to_world( WorldCanvas *wc, double lin_x, double lin_y, double &world_x, double &world_y ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    // BEGIN - critical section
	    static Vector<Double> linearv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);
	    linearv(0) = lin_x;
	    linearv(1) = lin_y;
	    if ( ! wc->linToWorld( worldv, linearv ) )
		throw internal_error( "linear to world conversion failed" );
	    world_x = worldv(0);
	    world_y = worldv(1);
	    // END - critical section
	}

	void linear_to_world( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
				 double &world_x1, double &world_y1, double &world_x2, double &world_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    static Vector<Double> linearv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);

	    // BEGIN - critical section
	    linearv(0) = lin_x1;
	    linearv(1) = lin_y1;
	    if ( ! wc->linToWorld( worldv, linearv ) )
		throw internal_error( "linear to world conversion failed" );

	    world_x1 = worldv(0);
	    world_y1 = worldv(1);

	    linearv(0) = lin_x2;
	    linearv(1) = lin_y2;
	    if ( ! wc->linToWorld( worldv, linearv ) )
		throw internal_error( "linear to world conversion failed" );
	    world_x2 = worldv(0);
	    world_y2 = worldv(1);
	    // END - critical section
	}

	void world_to_linear( WorldCanvas *wc, double world_x, double world_y, double &lin_x, double &lin_y ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    static Vector<Double> worldv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);

	    const CoordinateSystem &cs = wc->coordinateSystem( );

	    // BEGIN - critical section

	    // if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
	    // 	worldv.resize(cs.nWorldAxes( ));
	    // 	worldv = cs.referenceValue( );
	    // }

	    worldv(0) = world_x;
	    worldv(1) = world_y;

	    if ( ! wc->worldToLin( linearv, worldv ) )
		throw internal_error( "world to linear conversion failed" );
	    lin_x = linearv(0);
	    lin_y = linearv(1);

	    // END - critical section
	}

	void world_to_linear( WorldCanvas *wc, double world_x1, double world_y1, double world_x2, double world_y2,
			      double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    // BEGIN - critical section
	    static Vector<Double> worldv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);
	    worldv(0) = world_x1;
	    worldv(1) = world_y1;
	    if ( ! wc->worldToLin( linearv, worldv ) )
		throw internal_error( "world to linear conversion failed" );
	    lin_x1 = linearv(0);
	    lin_y1 = linearv(1);

	    worldv(0) = world_x2;
	    worldv(1) = world_y2;
	    if ( ! wc->worldToLin( linearv, worldv ) )
		throw internal_error( "world to linear conversion failed" );
	    lin_x2 = linearv(0);
	    lin_y2 = linearv(1);
	    // END - critical section
	}

	void pixel_to_world( WorldCanvas *wc, double pix_x1, double pix_y1, double &world_x1, double &world_y1 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    const CoordinateSystem &cs = wc->coordinateSystem( );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);

	    // BEGIN - critical section
	    if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
		worldv.resize(cs.nWorldAxes( ));
		worldv = cs.referenceValue( );
	    }

	    if ( cs.nPixelAxes( ) != pixelv.nelements( ) ) {
		pixelv.resize(cs.nPixelAxes( ));
		pixelv = cs.referencePixel( );
	    }

	    pixelv(0) = pix_x1;
	    pixelv(1) = pix_y1;

	    if ( ! cs.toWorld( worldv, pixelv ) )
		throw internal_error( "pixel to world conversion failed" );

	    world_x1 = worldv(0);
	    world_y1 = worldv(1);
	    // END - critical section
	}

	void pixel_to_world( WorldCanvas *wc, double pix_x1, double pix_y1, double pix_x2, double pix_y2,
			     double &world_x1, double &world_y1, double &world_x2, double &world_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    const CoordinateSystem &cs = wc->coordinateSystem( );

	    static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> worldv(2);

	    // BEGIN - critical section
	    if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
		worldv.resize(cs.nWorldAxes( ));
		worldv = cs.referenceValue( );
	    }

	    if ( cs.nPixelAxes( ) != pixelv.nelements( ) ) {
		pixelv.resize(cs.nPixelAxes( ));
		pixelv = cs.referencePixel( );
	    }

	    pixelv(0) = pix_x1;
	    pixelv(1) = pix_y1;

	    if ( ! cs.toWorld( worldv, pixelv ) )
		throw internal_error( "pixel to world conversion failed" );

	    world_x1 = worldv(0);
	    world_y1 = worldv(1);


	    pixelv(0) = pix_x2;
	    pixelv(1) = pix_y2;

	    if ( ! cs.toWorld( worldv, pixelv ) )
		throw internal_error( "pixel to world conversion failed" );

	    world_x2 = worldv(0);
	    world_y2 = worldv(1);
	    // END - critical section
	}

	void pixel_to_linear( WorldCanvas *wc, double pix_x1, double pix_y1, double &lin_x1, double &lin_y1 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    double world_x1, world_y1;
	    pixel_to_world( wc, pix_x1, pix_y1, world_x1, world_y1 );
	    world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );
	}

      void pixel_to_linear( WorldCanvas *wc, double pix_x1, double pix_y1, double pix_x2, double pix_y2,
			    double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    double world_x1, world_y1;
	    pixel_to_world( wc, pix_x1, pix_y1, world_x1, world_y1 );
	    world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );

	    pixel_to_world( wc, pix_x2, pix_y2, world_x1, world_y1 );
	    world_to_linear( wc, world_x1, world_y1, lin_x2, lin_y2 );
	}

        MDirection::Types get_coordinate_type( const CoordinateSystem &cs ) {
	    for ( uInt i=0; i < cs.nCoordinates(); ++i )
		if ( cs.type(i) == Coordinate::DIRECTION )
		    return cs.directionCoordinate(i).directionType(true);
	    return MDirection::EXTRA;
	}

	RegionInfo::stats_t *Region::getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg ) {

	    // Compute and print statistics on DD's image for
	    // given region in all layers.

	    //there are several possible path here
	    //(1) modify ImageRegion record then create SubImage of 1 plane
	    //(2) modify ImageRegion directly then creage SubImage of 1 plane
	    //(3) make SubImage of SubImage to get one plane
	    //(4) pass layer index to LatticeStatistcis
	    //(5) do single plane statistic right here

	    if( image==0 || padd == 0 ) return 0;

		try {

		    SubImage<Float> subImg(*image, imgReg);
		    IPosition shp = image->shape();
		    IPosition sshp = subImg.shape();

		    Vector<Int> dispAxes = padd->displayAxes();

		    Vector<Int> cursorAxes(2);
		    cursorAxes(0) = dispAxes[0];	//display axis 1
		    cursorAxes(1) = dispAxes[1];	//display axis 2

		    Int nAxes = image->ndim();
		    Vector<int> otherAxes(0);
		    otherAxes = IPosition::otherAxes(nAxes, cursorAxes).asVector();

		    IPosition start(nAxes);
		    IPosition stride(nAxes);
		    IPosition end(sshp);
		    start = 0;
		    stride = 1;

		    Int _axis_h_ = shp.size( ) > 3 ? padd->xlatePixelAxes(3) : -1;		// get first "hidden axis
		    String zaxis = padd->zaxisStr( );

		    const CoordinateSystem& cs = image->coordinates();

		    Vector<String> axesNames = padd->worldToPixelAxisNames( cs );
		    String haxis = _axis_h_ >= 0 ? axesNames(_axis_h_) : "";
		    // uiBase( ) sets zero/one based:
		    Int hIndex = _axis_h_ >= 0 ? padd->xlateFixedPixelAxes(_axis_h_) + padd->uiBase( ) : -1;
		    Int zIndex = padd->activeZIndex();

		    String zUnit, zspKey, zspVal;
		    zUnit = padd->spectralunitStr( );

		    String unit =  image->units().getName();

		    IPosition pos = padd->fixedPosition();

		    ImageStatistics<Float> stats(subImg, False);
		    if ( ! stats.setAxes(cursorAxes) ) return 0;
		    stats.setList(True);
		    String layerStats;
		    Vector<String> nm = cs.worldAxisNames();

		    Int zPos = -1;
		    Int hPos = -1;
		    for (Int k = 0; k < nm.nelements(); k++) {
			if (nm(k) == zaxis)
			    zPos = k;
			if (nm(k) == haxis)
			    hPos = k;
		    }

		    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		    //   begin collecting statistics...
		    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		    RegionInfo::stats_t *layerstats = new RegionInfo::stats_t( );

		    String zLabel="";
		    String hLabel="";
		    Vector<Double> tPix,tWrld;
		    tPix = cs.referencePixel();
		    String tStr;
		    if (zPos > -1) {
			tPix(zPos) = zIndex;
			if (!cs.toWorld(tWrld,tPix)) {
			} else {
			    zLabel = ((CoordinateSystem)cs).format(tStr, Coordinate::DEFAULT, tWrld(zPos), zPos);
			    layerstats->push_back(RegionInfo::stats_t::value_type(zaxis,zLabel + tStr));

			    if (zUnit.length()>0) {
				zspKey = "Spectral_Vale";
				zspVal = ((CoordinateSystem)cs).format(zUnit,Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit;
			    }
			}
		    }

		    if (hPos > -1) {
			tPix(hPos) = hIndex;

			if (!cs.toWorld(tWrld,tPix)) {
			} else {
			    hLabel = ((CoordinateSystem)cs).format(tStr, Coordinate::DEFAULT, tWrld(hPos), hPos);
			    if (zUnit.length()>0) {
				zspKey = "Spectral_Vale";
				zspVal = ((CoordinateSystem)cs).format(zUnit, Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit;
			    }
			}
		    }

		    Int spInd = cs.findCoordinate(Coordinate::SPECTRAL);
		    SpectralCoordinate spCoord;
		    Int wSp=-1;
		    if ( spInd>=0 ){
			wSp= (cs.worldAxes(spInd))[0];
			spCoord=cs.spectralCoordinate(spInd);
			spCoord.setVelocity();
			Double vel;
			Double restFreq = spCoord.restFrequency();
			if (downcase(zaxis).contains("freq")) {
			    if (spCoord.pixelToVelocity(vel, zIndex)) {
				if (restFreq >0)
				    layerstats->push_back(RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
				else
				    layerstats->push_back(RegionInfo::stats_t::value_type(zspKey,zspVal));

				// --- this line was executed, but was a NOP in the old code --- <drs>
				// layerstats->push_back(RegionInfo::image_stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
			    }
			}

			if (downcase(haxis).contains("freq")) {
			    if (spCoord.pixelToVelocity(vel, hIndex)) {
				if (restFreq >0)
				    layerstats->push_back(RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
				else
				    layerstats->push_back(RegionInfo::stats_t::value_type(zspKey,zspVal));

				layerstats->push_back(RegionInfo::stats_t::value_type("Frame",MFrequency::showType(spCoord.frequencySystem())));
				layerstats->push_back(RegionInfo::stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
			    }
			}
		    }


		    if ( hLabel != "" ) layerstats->push_back(RegionInfo::stats_t::value_type(haxis,hLabel));

		    // strip out extra quotes, e.g. '"ELECTRONS"'
		    std::string unitval(unit.c_str( ));
		    std::string::size_type p = 0;
		    while( (p = unitval.find('"',p)) != unitval.npos ) {
			unitval.erase(p, 1);
		    }
		    layerstats->push_back(RegionInfo::stats_t::value_type("BrightnessUnit",unitval));

		    Double beamArea = 0;
		    ImageInfo ii = image->imageInfo();
		    Vector<Quantum<Double> > beam = ii.restoringBeam();
		    CoordinateSystem cSys = image->coordinates();
		    std::string imageUnits = image->units().getName();
		    std::transform( imageUnits.begin(), imageUnits.end(), imageUnits.begin(), ::toupper );

		    Int afterCoord = -1;
		    Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
		    // use contains() not == so moment maps are dealt with nicely
		    if ( beam.nelements()==3 && dC!=-1 && imageUnits.find("JY/BEAM") != std::string::npos ) {
			DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
			Vector<String> units(2);
			units(0) = units(1) = "rad";
			dCoord.setWorldAxisUnits(units);
			Vector<Double> deltas = dCoord.increment();

			Double major = beam(0).getValue(Unit("rad"));
			Double minor = beam(1).getValue(Unit("rad"));
			beamArea = C::pi/(4*log(2)) * major * minor / abs(deltas(0) * deltas(1));
		    }

		    if ( beamArea > 0 ) layerstats->push_back(RegionInfo::stats_t::value_type("BeamArea",String::toString(beamArea)));

		    Bool statsOk = stats.getLayerStats(*layerstats, beamArea, zPos, zIndex, hPos, hIndex);
		    if ( ! statsOk ) {
			delete layerstats;
			return 0;
		    } else {
			return layerstats;
		    }

		} catch (const casa::AipsError& err) {
		    std::string errMsg_ = err.getMesg();
		    // fprintf( stderr, "Region::getLayerStats( ): %s\n", errMsg_.c_str() );
		    return 0;
		} catch (...) {
		    std::string errMsg_ = "Unknown error computing region statistics.";
		    // fprintf( stderr, "Region::getLayerStats( ): %s\n", errMsg_.c_str() );
		    return 0;
		}
	}

	Int Region::getAxisIndex( ImageInterface<Float> *image, std::string axtype ) {

	    if( image == 0 ) return -1;

	    const CoordinateSystem* cs=0;
	    try {
		cs = &(image->coordinates());
	    } catch(...) { cs = 0;  }	// (necessity of try-catch is doubtful...).

	    if (cs==0) return -1;

	    try {
		Int nAxes = image->ndim();
		for(Int ax=0; ax<nAxes && ax<Int(cs->nWorldAxes()); ax++) {
		    // coordno : type of coordinate
		    // axisincoord : index within the coordinate list defined by coordno
		    Int coordno, axisincoord;
		    cs->findWorldAxis(coordno, axisincoord, ax);

		    //cout << "coordno=" << coordno << "  axisincoord : " << axisincoord << "  type : " << cs->showType(coordno) << endl;

		    if( cs->showType(coordno) == "Direction" ) {
			// Check for Right Ascension and Declination
			Vector<String> axnames = (cs->directionCoordinate(coordno)).axisNames(MDirection::DEFAULT);
			AlwaysAssert( axisincoord>=0 && axisincoord < axnames.nelements(), AipsError);
			if( axnames[axisincoord] == axtype.c_str() ) {
			    return ax;
			}
		    } else {
			// Check for Stokes and Spectral
			if ( cs->showType(coordno) == axtype.c_str() ) {
			    return ax;
			}
		    }
		}
	    } catch (...) {
		std::string errMsg_ = "Unknown error converting region ***";
		// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
		// emit qddRegnError(errMsg_);
		return -1;
	    }
	    return -1;
	}

	void linear_to_j2000( WorldCanvas *wc, double lx, double ly, double &wx, double &wy ) {
	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    linear_to_world( wc, lx, ly, wx, wy );
	    if ( type == MDirection::J2000 ) return;
	    Vector<double> pts(2);
	    pts[0] = wx;
	    pts[1] = wy;
	    MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
	    casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
	    wx = newpts.getValue( )(0);
	    wy = newpts.getValue( )(1);
	}

	void linear_to_j2000( WorldCanvas *wc, double lx1, double ly1, double lx2, double ly2,
			      double &wx1, double &wy1, double &wx2, double &wy2 ) {

	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    linear_to_world( wc, lx1, ly1, lx2, ly2, wx1, wy1, wx2, wy2 );
	    if ( type == MDirection::J2000 ) return;

	    Vector<double> pts(2);
	    pts[0] = wx1;
	    pts[1] = wy1;
	    MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
	    casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
	    wx1 = newpts.getValue( )(0);
	    wy1 = newpts.getValue( )(1);

	    pts[0] = wx2;
	    pts[1] = wy2;
	    direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
	    newpts = direction.getAngle("rad");
	    wx2 = newpts.getValue( )(0);
	    wy2 = newpts.getValue( )(1);

	}

	void to_linear( WorldCanvas *wc, MDirection::Types in_type, double x, double y, double &lx, double &ly ) {
	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    if ( type == in_type ) {
		world_to_linear( wc, x, y, lx, ly );
		return;
	    } else {
		Vector<double> pts(2);
		pts[0] = x;
		pts[1] = y;
		MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
		casa::Vector<double> newpts = direction.getAngle("rad").getValue( );
		world_to_linear( wc, newpts[0], newpts[1], lx, ly );
	    }
	}

	void to_linear( WorldCanvas *wc, MDirection::Types in_type, double x1, double y1, double x2, double y2, double &lx1, double &ly1, double &lx2, double &ly2 ) {
	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    if ( type == in_type ) {
		world_to_linear( wc, x1, y1, lx1, ly1 );
		world_to_linear( wc, x2, y2, lx2, ly2 );
		return;
	    } else {
		Vector<double> pts(2);
		pts[0] = x1;
		pts[1] = y1;
		MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
		casa::Vector<double> newpts = direction.getAngle("rad").getValue( );
		world_to_linear( wc, newpts[0], newpts[1], lx1, ly1 );
		pts[0] = x2;
		pts[1] = y2;
		direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
		newpts = direction.getAngle("rad").getValue( );
		world_to_linear( wc, newpts[0], newpts[1], lx1, ly2 );
	    }
	}

	void to_linear_offset( WorldCanvas *wc, MDirection::Types in_type, double xoff, double yoff, double &lxoff, double &lyoff ) {
	    const int base = 10;
	    double blcx, blcy, trcx, trcy;
	    to_linear( wc, in_type, base, base, base+xoff, base+yoff, blcx, blcy, trcx, trcy );
	    lxoff = trcx - blcx;
	    lyoff = trcy - blcy;
	}

	void linear_to_b1950( WorldCanvas *wc, double lx, double ly, double &wx, double &wy ) {
	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    linear_to_world( wc, lx, ly, wx, wy );
	    if ( type == MDirection::B1950 ) return;

	    Vector<double> pts;
	    pts[0] = wx;
	    pts[1] = wy;
	    MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
	    casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
	    wx = newpts.getValue( )(0);
	    wy = newpts.getValue( )(1);
	}

	void linear_to_b1950( WorldCanvas *wc, double lx1, double ly1, double lx2, double ly2,
			      double &wx1, double &wy1, double &wx2, double &wy2 ) {

	    MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
	    linear_to_world( wc, lx1, ly1, lx2, ly2, wx1, wy1, wx2, wy2 );
	    if ( type == MDirection::B1950 ) return;

	    Vector<double> pts(2);
	    pts[0] = wx1;
	    pts[1] = wy1;
	    MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
	    casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
	    wx1 = newpts.getValue( )(0);
	    wy1 = newpts.getValue( )(1);

	    pts[0] = wx2;
	    pts[1] = wy2;
	    direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
	    newpts = direction.getAngle("rad");
	    wx2 = newpts.getValue( )(0);
	    wy2 = newpts.getValue( )(1);

	}
    }

}
