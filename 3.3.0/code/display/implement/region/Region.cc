#include <display/region/Region.h>
#include <casa/Quanta/MVAngle.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <measures/Measures/MCDirection.h>

#include <images/Images/ImageStatistics.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <math.h>

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
	    if ( wc_ == 0 ) return;
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
	    if ( wc_ == 0 ) return;
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
	    if ( wc_ == 0 ) return;
	    PixelCanvas *pc = wc_->pixelCanvas();
	    if ( pc == 0 ) return;
	    pc->copyBackBufferToFrontBuffer();
	    pc->setDrawBuffer(Display::FrontBuffer);
	    pc->callRefreshEventHandlers(Display::BackCopiedToFront);
	}

	void Region::draw( ) {
	    if ( wc_ == 0 ) return;

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
	    drawRegion( selected( ) );
	    resetDrawingEnv( );

	    setTextEnv( );
	    drawText( );
	    resetTextEnv( );
	}

	void Region::drawText( ) { 
	    if ( wc_ == 0 ) return;
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

	void Region::getCoordinatesAndUnits( Region::Coord &c, Region::Units &u ) const {
	    c = current_region_coordsys( );
	    u = current_units( );
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
		type == MDirection::ECLIPTIC ? Region::Ecliptic : Region::Pixel;
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
	    

	static inline void convert_units( Quantum<Vector<double> > &q, Region::Units new_units ) {
	    q.convert( as_string(new_units) );
	}

	static inline void convert_units( double &x, const String &xunits, double &y, const String &yunits, Region::Units new_units ) {
	    Quantum<Vector<double> > result(Vector<double>(2), xunits);
	    Quantum<double> yq(y,yunits);
	    yq.convert(xunits);
	    result.getValue( )(0) = x;
	    result.getValue( )(1) = yq.getValue( );
	    convert_units( result, new_units );
	    x = result.getValue( )(0);
	    y = result.getValue( )(1);
	}

	static inline Quantum<Vector<double> > convert_angle( double x, const String &xunits, double y, const String &yunits,
							      MDirection::Types original_coordsys, Region::Coord new_coordsys, const String &new_units="rad" ) {
	    Quantum<double> xq(x,xunits);
	    Quantum<double> yq(y,yunits);
	    MDirection md = MDirection::Convert(MDirection(xq,yq,original_coordsys), viewer_to_casa(new_coordsys))();
	    casa::Quantum<casa::Vector<double> > result = md.getAngle("rad");
	    xq.convert("rad");
	    yq.convert("rad");
	    result.getValue( )(0) = wrap_angle(xq.getValue( ), result.getValue( )(0));
	    result.getValue( )(1) = wrap_angle(yq.getValue( ), result.getValue( )(1));
	    result.convert(new_units);
	    return result;
	}


	void Region::getPositionString( std::string &x, std::string &y, std::string &angle,
					Region::Coord coord, Region::Units new_units ) const {
	    if ( wc_ == 0 ) {
		x = "internal error";
		y = "internal error";
		angle = "internal error";
		return;
	    }

	    if ( coord == DefaultCoord ) coord = current_region_coordsys( );
	    if ( new_units == DefaultUnits ) new_units = current_units( );
	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    MDirection::Types cccs = current_casa_coordsys( );

	    if ( cccs == MDirection::N_Types ) {
		// this impiles that the coordinate system does not have a direction coordinate...
		// so it is probably a measurement set... treat as a pixel coordinate for now...
		int center_x, center_y;
		linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
		x = as_string(center_x);
		y = as_string(center_y);
		angle = as_string(0);
		return;
	    }

	    Coord cvcs = casa_to_viewer(cccs);
	    double result_x, result_y;

	    if ( coord == Pixel ) {
		int center_x, center_y;
		linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
		x = as_string(center_x);
		y = as_string(center_y);
		angle = as_string(0);
		return;
	    } else if ( coord == cvcs ) {
		linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
		const Vector<String> &units = wc_->worldAxisUnits();
		convert_units( result_x, units[0], result_y, units[1], new_units );
	    } else {
		linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
		const Vector<String> &units = wc_->worldAxisUnits();
		Quantum<Vector<double> > result = convert_angle( result_x, units[0], result_y, units[1], cccs, coord );
		convert_units( result, new_units );
		result_x = result.getValue( )(0);
		result_y = result.getValue( )(1);
	    }

	    if ( new_units == HMS ) {
		x = MVAngle(result_x)(0.0).string(MVAngle::TIME,9);
		y = MVAngle(result_y)(0.0).string(MVAngle::TIME,9);
	    } else if ( new_units == DMS ) {
		x = MVAngle(result_x)(0.0).string(MVAngle::ANGLE_CLEAN,8);
		y = MVAngle(result_y)(0.0).string(MVAngle::ANGLE_CLEAN,8);
	    } else {
		x = as_string(result_x);
		y = as_string(result_y);
	    }
	    angle = "";
	}

	void Region::movePosition( const std::string &x, const std::string &y,
				   const std::string &coord, const std::string &units ) {

	    if ( wc_ == 0 ) return;

	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    // current center of region...
	    double cur_center_x = linear_average(blc_x,trc_x);
	    double cur_center_y = linear_average(blc_y,trc_y);
	    // filled in below...
	    double new_center_x, new_center_y;

	    if ( coord == "pixel" ) {

		int pix_x = atoi(x.c_str( ));
		int pix_y = atoi(y.c_str( ));

		pixel_to_linear( wc_, pix_x, pix_y, new_center_x, new_center_y );

	    } else {
		Quantity xq, yq;

		// read in to convert HMS/DMS...
		MVAngle::read(xq,x);
		MVAngle::read(yq,y);
		// the above results in radians... fix in case of degrees...
		if ( units == "degrees" ) {
		    xq = Quantity(xq.getValue(),string_to_casa_units(units));
		    yq = Quantity(yq.getValue(),string_to_casa_units(units));
		}

		// convert to radians, for wrap_angle( ) below...
		xq.convert("rad");
		yq.convert("rad");

		// convert world coordinates to current coordinate system...
		MDirection md = MDirection::Convert(MDirection(xq,yq,string_to_casa_coordsys(coord)),current_casa_coordsys( ))( );

		// fetch world coordinates in radians...
		// and make angles consistent with input...
		casa::Quantum<casa::Vector<double> > worldq = md.getAngle("rad");
		new_center_x = wrap_angle(xq.getValue(),worldq.getValue()(0));
		new_center_y = wrap_angle(yq.getValue(),worldq.getValue()(1));

		// convert new center to linear coordinates...
		world_to_linear( wc_, new_center_x, new_center_y, new_center_x, new_center_y );

	    }

	    // move region...
	    move( new_center_x - cur_center_x, new_center_y - cur_center_y );

	    refresh( );
	}

	Region::Coord Region::current_region_coordsys( ) const {
	    return casa_to_viewer(current_casa_coordsys( ));
	}

	MDirection::Types Region::current_casa_coordsys( ) const {
	    if ( wc_ == 0 ) return MDirection::J2000;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );
	    int index = cs.findCoordinate(Coordinate::DIRECTION);
	    if ( index < 0 ) {
		// no direction coordinate...
		return MDirection::N_Types;
	    }
	    return cs.directionCoordinate(index).directionType(true);
	}

	Region::Units Region::current_units( ) const {
	    if ( wc_ == 0 ) return Degrees;
	    const Vector<String> &units = wc_->worldAxisUnits();
	    if ( units(0) == "rad" )
		return Radians;
	    if ( units(0) == "deg" )
		return Degrees;
	    return Degrees;
	}


      

	void Region::set_line_style( LineStyle linestyle ) {
	    if ( wc_ == 0 ) return;
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

	void linear_to_pixel( WorldCanvas *wc, double lin_x1, double lin_y1, int &pix_x1, int &pix_y1 ) {

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
				int &pix_x1, int &pix_y1, int &pix_x2, int &pix_y2 ) {

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

	    // BEGIN - critical section
	    static Vector<Double> worldv(2);	// avoid vector allocation for each conversion
	    static Vector<Double> linearv(2);
	    worldv(0) = world_x;
	    worldv(1) = world_y;
	    if ( ! wc->worldToLin( linearv, worldv ) )
		throw internal_error( "world to linear conversion failed" );
	    lin_x = linearv(0);
	    lin_y = linearv(1);
	    // END - critical section
	}


	void pixel_to_world( WorldCanvas *wc, int pix_x1, int pix_y1, double &world_x1, double &world_y1 ) {

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

	void pixel_to_linear( WorldCanvas *wc, int pix_x1, int pix_y1, double &lin_x1, double &lin_y1 ) {

	    if ( wc == 0 )
		throw internal_error( "coordinate transformation without coordinate system" );

	    double world_x1, world_y1;
	    pixel_to_world( wc, pix_x1, pix_y1, world_x1, world_y1 );
	    world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );
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

		    Int _axis_h_ = padd->xlatePixelAxes(3);		// get first "hidden axis
		    String zaxis = padd->zaxisStr( );

		    const CoordinateSystem& cs = image->coordinates();

		    Vector<String> axesNames = padd->worldToPixelAxisNames( cs );
		    String haxis = axesNames(_axis_h_);
		    Int hIndex = _axis_h_ - 2 + padd->uiBase( );		// uiBase( ) sets zero/one based
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


		    layerstats->push_back(RegionInfo::stats_t::value_type(haxis,hLabel));
		    layerstats->push_back(RegionInfo::stats_t::value_type("BrightnessUnit",unit));

		    Double beamArea = 0;
		    ImageInfo ii = image->imageInfo();
		    Vector<Quantum<Double> > beam = ii.restoringBeam();
		    CoordinateSystem cSys = image->coordinates();
		    String imageUnits = image->units().getName();
		    imageUnits.upcase();

		    Int afterCoord = -1;
		    Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
		    // use contains() not == so moment maps are dealt with nicely
		    if (beam.nelements()==3 && dC!=-1 && imageUnits.contains("JY/BEAM")) {
			DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
			Vector<String> units(2);
			units(0) = units(1) = "rad";
			dCoord.setWorldAxisUnits(units);
			Vector<Double> deltas = dCoord.increment();

			Double major = beam(0).getValue(Unit("rad"));
			Double minor = beam(1).getValue(Unit("rad"));
			beamArea = C::pi/(4*log(2)) * major * minor / abs(deltas(0) * deltas(1));
		    }

		    layerstats->push_back(RegionInfo::stats_t::value_type("BeamArea",String::toString(beamArea)));

		    Bool statsOk = stats.getLayerStats(*layerstats, beamArea, zPos, zIndex, hPos, hIndex);
		    if ( ! statsOk ) {
			delete layerstats;
			return 0;
		    } else {
			return layerstats;
		    }

		} catch (const casa::AipsError& err) {
		    std::string errMsg_ = err.getMesg();
		    fprintf( stderr, ">*>>*>>*>>*>>*>>*>>*>>*>>*>>*> %s\n", errMsg_.c_str() );
		    return 0;
		} catch (...) {
		    std::string errMsg_ = "Unknown error computing region statistics.";
		    fprintf( stderr, ">>>>>>>>>>*>>*>>*>>*>>*>>*>>*> %s\n", errMsg_.c_str() );
		    return 0;
		}
	}

	Int Region::getAxisIndex( ImageInterface<Float> *image, String axtype ) {

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

		    if( cs->showType(coordno) == String("Direction") ) {
			// Check for Right Ascension and Declination
			Vector<String> axnames = (cs->directionCoordinate(coordno)).axisNames(MDirection::DEFAULT);
			AlwaysAssert( axisincoord>=0 && axisincoord < axnames.nelements(), AipsError);
			if( axnames[axisincoord] == axtype ) {
			    return ax;
			}
		    } else {
			// Check for Stokes and Spectral
			if ( cs->showType(coordno)==axtype ) {
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


    }

}
