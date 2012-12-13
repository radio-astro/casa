//# Region.h: base class for statistical regions
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


#ifndef REGION_REGION_H_
#define REGION_REGION_H_

#include <set>
#include <list>
#include <string>
#include <images/Images/ImageStatistics.h>
#include <measures/Measures/MDirection.h>
#include <display/region/RegionInfo.h>
#include <display/Utilities/dtor.h>
#include <display/Display/MouseToolState.h>
#include <display/Utilities/VOID.h>
#include <stdexcept>


extern "C" void casa_viewer_pure_virtual( const char *file, int line, const char *func );
#define DISPLAY_PURE_VIRTUAL(FUNCTION,RESULT) \
  { casa_viewer_pure_virtual( __FILE__, __LINE__, #FUNCTION ); return RESULT; }

namespace casa {

    class WorldCanvas;
    class PrincipalAxesDD;
    template <class T> class ImageInterface;
    class ImageRegion;
	class DisplayData;

    namespace viewer {

	// convert linear coordinates to viewer screen coordinates...
	void linear_to_screen( WorldCanvas *wc_, double, double, int &, int & );
	void linear_to_screen( WorldCanvas *wc_, double, double, double, double, int &, int &, int &, int & );
	// convert linear coordinates to casa pixel coordinates...
	void linear_to_pixel( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_pixel( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	// convert viewer screen coordinates to linear coordinates...
	void screen_to_linear( WorldCanvas *wc_, int, int, double &, double & );
	void screen_to_linear( WorldCanvas *wc_, int, int, int, int, double &, double &, double &, double & );
	// convert linear coordinates to world coordinates...
	void linear_to_world( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_world( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	// convert world coordinates to linear coordinates...
	void world_to_linear( WorldCanvas *wc_, double, double, double &, double & );
	void world_to_linear( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double& );
	// convert casa pixel coordinates to world coordinates...
	void pixel_to_world( WorldCanvas *wc_, double, double, double &, double & );
	void pixel_to_world( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	// convert casa pixel coordinates to linear coordinates...
	void pixel_to_linear( WorldCanvas *wc_, double, double, double &, double & );
	void pixel_to_linear( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );

	// convert linear coordinates to specific coordinate systems...
	void linear_to_j2000( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_j2000( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	void linear_to_b1950( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_b1950( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	void linear_to_galactic( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_galactic( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	void linear_to_ecliptic( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_ecliptic( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );

	void to_linear( WorldCanvas *, MDirection::Types in_type, double, double, double &, double & );
	void to_linear( WorldCanvas *, MDirection::Types in_type, double, double, double, double, double &, double &, double &, double & );
	void to_linear_offset( WorldCanvas *, MDirection::Types in_type, double, double, double &, double & );

	void screen_offset_to_linear_offset( WorldCanvas *wc_, int, int, double &, double & );
	void pixel_offset_to_linear_offset( WorldCanvas *wc_, double, double, double &, double & );
	void linear_offset_to_pixel_offset( WorldCanvas *wc_, double, double, double &, double & );

	MDirection::Types get_coordinate_type( const CoordinateSystem &wc );


	class ImageRegion_state {
		public:
			ImageRegion_state( ImageRegion *ir, size_t region_count ) : imageregion(ir), count_(region_count) { }
			ImageRegion_state( const ImageRegion_state &other ) : imageregion(other.imageregion), count_(other.count_) { }
			operator ImageRegion *( ) { return imageregion; }
			size_t regionCount( ) const { return count_; }
		private:
			void* operator new (std::size_t) throw (std::logic_error)
						{ throw std::logic_error("allocating an object not intended for dynamic allocation"); }
			ImageRegion *imageregion;
			size_t count_;
	};

	// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
	// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
	// this means that coordinate transformation is required each time the region is drawn.
	class Region : public dtorNotifier {
	    public:
		typedef std::set<Region*> region_list_type;

		/* enum states { undisplayed, inactive, highlighted, selected }; */
		// LSDoubleDashed is only used to preserve state (it is a Display::LineStyle option)
		enum LineStyle { SolidLine, DashLine, DotLine, LSDoubleDashed };
		enum TextPosition { TopText, RightText, BottomText, LeftText };
		enum TextFontStyle { ItalicText = 1 << 0, BoldText = 1 << 1 };
		enum Coord { J2000, B1950, Galactic, SuperGalactic, Ecliptic, DefaultCoord };
		enum Units { Degrees, Radians, Sexagesimal, Pixel, DefaultUnits };

		// state returned from mouse functions for regions...
		enum MouseState { MouseRefresh = 1 << 0, MouseSelected = 1 << 1, MouseHandle = 1 << 2 };

		enum RegionTypes { RectRegion, PointRegion, EllipseRegion, PolyRegion };

		enum PointLocation { PointInside = 1 << 0, PointHandle = 1 << 1, PointOutside = 1 << 2 };

		enum RegionChanges { RegionChangeCreate, RegionChangeUpdate, RegionChangeReset, RegionChangeFocus, RegionChangeModified, RegionChangeLabel, RegionChangeDelete, RegionChangeStatsUpdate, RegionChangeNewChannel };

		class PointInfo {
		    public:
			PointInfo( double x, double y, unsigned int location, unsigned int handle=0 ) :
							x_(x), y_(y), location_(location), handle_(handle) { }
			PointInfo( const PointInfo &other) : x_(other.x_), y_(other.y_), location_(other.location_), handle_(other.handle_) { }
			unsigned int handle( ) const { return handle_; }
			unsigned int &handle( ) { return handle_; }
			unsigned int location( ) const { return location_; }
			unsigned int operator&( PointLocation mask ) const { return location_ & mask; }
			const PointInfo &operator=( const PointInfo &other ) {
			    x_ = other.x_;
			    y_ = other.y_;
			    location_ = other.location_;
			    handle_ = other.handle_;
			    return *this;
			}
			double x( ) const { return x_; }
			double y( ) const { return y_; }
			double &x( ) { return x_; }
			double &y( ) { return y_; }
		    private:
			double x_, y_;
			unsigned int location_;
			unsigned int handle_;
		};

		// functions to query results from mouse functions...
		static bool refreshNeeded( int v ) { return v & MouseRefresh ? true : false; }
		static bool regionSelected( int v ) { return v & MouseSelected ? true : false; }
		static bool handleSelected( int v ) { return v & MouseHandle ? true : false; }

		// user specified name
		virtual const std::string name( ) const DISPLAY_PURE_VIRTUAL(Region::name,"");

		virtual std::string lineColor( ) const DISPLAY_PURE_VIRTUAL(Region::lineColor,"cyan");
		virtual std::string centerColor( ) const DISPLAY_PURE_VIRTUAL(Region::centerColor,"cyan");
		virtual int lineWidth( ) const DISPLAY_PURE_VIRTUAL(Region::lineWidth,1);
		virtual LineStyle lineStyle( ) const DISPLAY_PURE_VIRTUAL(Region::lineStyle,SolidLine);

		virtual std::string textColor( ) const DISPLAY_PURE_VIRTUAL(Region::textColor,"cyan");
		virtual std::string textFont( ) const DISPLAY_PURE_VIRTUAL(Region::textFont,"Courier");
		virtual int textFontSize( ) const DISPLAY_PURE_VIRTUAL(Region::textFontSize,12);
		virtual int textFontStyle( ) const DISPLAY_PURE_VIRTUAL(Region::textFontStyle,0);
		virtual std::string textValue( ) const DISPLAY_PURE_VIRTUAL(Region::textValue,"");
		virtual TextPosition textPosition( ) const DISPLAY_PURE_VIRTUAL(Region::textPosition,BottomText);
		virtual void textPositionDelta( int &/*x*/, int &/*y*/ ) const DISPLAY_PURE_VIRTUAL(Region::textPositionDelta,);

		virtual void setLabel( const std::string &l ) = 0;
		virtual void setLabelPosition( TextPosition ) = 0;
		virtual void setLabelDelta( const std::vector<int> & ) = 0;
		virtual void setFont( const std::string &font="", int font_size=-1, int font_style=0, const std::string &font_color="" ) = 0;
		virtual void setLine( const std::string &line_color="", Region::LineStyle line_style=Region::SolidLine, unsigned int line_width=1 ) = 0;
		virtual void setAnnotation(bool) = 0;

		void getCoordinatesAndUnits( Region::Coord &c, Region::Units &x_units, Region::Units &y_units,
					     std::string &width_height_units ) const;
		void getPositionString( std::string &x, std::string &y, std::string &angle,
					double &bounding_width, double &bounding_height,
					Region::Coord coord = Region::DefaultCoord,
					Region::Units x_units = Region::DefaultUnits,
					Region::Units y_units = Region::DefaultUnits,
					const std::string &bounding_units = "rad" ) const;

		bool translateX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ );
		bool translateY( const std::string &/*x*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ );
		bool resizeX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ );
		bool resizeY( const std::string &/*x*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ );

		// one would expect the "number of frames" in our composite cube (including
		// multiple images) to be contained in the non-GUI portion of the viewer
		// hierarchy, but rather it is within the Qt portion... thus this function
		// to fetch it...   <drs>
		virtual int numFrames( ) const DISPLAY_PURE_VIRTUAL(Region::numFrames,0);
		virtual void zRange( int &/*min*/, int &/*max*/ ) const DISPLAY_PURE_VIRTUAL(Region::zRange,);
		int zIndex( ) const;
		bool regionVisible( ) const { return visible_; }

		bool worldBoundingRectangle( double &, double &, const std::string & ) const;

		virtual ~Region( ) { }

		Region( ) : wc_(0), selected_(false), visible_(true), mouse_in_region(false) { }
		Region( WorldCanvas *wc );

		// is this region degenerate?
		virtual bool degenerate( ) const;

		void setDrawingEnv( );
		void resetDrawingEnv( );
		void setTextEnv( );
		void resetTextEnv( );
		void pushDrawingEnv( LineStyle ls, int thickness=-1 );
		void popDrawingEnv( );

		void setDrawCenter(bool draw_center){draw_center_=draw_center;};
		bool getDrawCenter(){return draw_center_;};

		// duplicate of MultiWCTool::refresh( )
		void refresh( );

		virtual PointInfo checkPoint( double x, double y ) const = 0;

		// returns OR'ed set of MouseState...
		virtual unsigned int mouseMovement( double /*x*/, double /*y*/, bool /*other_selected*/ )
		DISPLAY_PURE_VIRTUAL(Region::mouseMovement,0);

		virtual void draw( bool other_selected );

		// indicates that region movement requires that the statistcs be updated...
		virtual void updateStateInfo( bool /*region_modified*/, Region::RegionChanges /*change*/ ) DISPLAY_PURE_VIRTUAL(Region::updateStateInfo,);

		// indicates that the center info is no longer valid
		virtual void invalidateCenterInfo( ) DISPLAY_PURE_VIRTUAL(Region::invalidateCenterInfo,);

		bool selected( ) const { return selected_; }

		virtual bool weaklySelected( ) const = 0;
		virtual void weaklySelect( ) = 0;
		virtual void weaklyUnselect( ) = 0;
		// indicates that the user has selected this rectangle...
		// ...may need to scroll region dock
		virtual void selectedInCanvas( ) DISPLAY_PURE_VIRTUAL(Region::selectedInCanvas,);

		// blank out the statistics for this region
		virtual void clearStatistics( ) DISPLAY_PURE_VIRTUAL(Region::clearStatistics,);

		virtual bool clickWithin( double /*x*/, double /*y*/ ) const DISPLAY_PURE_VIRTUAL(Region::clickWithin,false);
		virtual int clickHandle( double /*x*/, double /*y*/ ) const DISPLAY_PURE_VIRTUAL(Region::clickHandle,0);
		// return value indicates if any data was flagged...
		virtual bool doubleClick( double /*x*/, double /*y*/ );
		// for rectangles, resizing can change the handle...
		// for rectangles, moving a handle is resizing...
		virtual int moveHandle( int handle, double /*x*/, double /*y*/ ) DISPLAY_PURE_VIRTUAL(Region::moveHandle,handle);
		virtual void move( double /*dx*/, double /*dy*/ ) DISPLAY_PURE_VIRTUAL(Region::move,);
		virtual void resize( double /*width_delta*/, double /*height_delta*/ ) = 0;
		virtual bool valid_translation( double dx, double dy, double width_delta, double height_delta ) = 0;

		// functions added with the introduction of RegionToolManager and the
		// unified selection and manipulation of the various region types...
		virtual void mark( bool set=true ) = 0;
		virtual bool marked( ) const = 0;
		// returns the new state...
		virtual bool mark_toggle( ) = 0;

		void clearMouseInRegion( ) { mouse_in_region = false; }

		// update status information...
		virtual void status( const std::string &msg, const std::string &type="info" ) = 0;

		virtual bool markCenter( ) const DISPLAY_PURE_VIRTUAL(Region::markCenter,true);

		virtual bool skyComponent( ) const DISPLAY_PURE_VIRTUAL(Region::skyComponent,true);

		// in "linear" coordinates...
		virtual void boundingRectangle (double &/*blc_x*/, double &/*blc_y*/, double &/*trc_x*/,
		                                double &/*trc_y*/) const
		DISPLAY_PURE_VIRTUAL(Region::boundingRectangle,);

		virtual void emitUpdate( )
			DISPLAY_PURE_VIRTUAL(Region::emitUpdate,);
	    protected:
		virtual std::list<RegionInfo> *generate_dds_statistics( );
		// hook to allow generate_dds_statistics( ) to generate statistics
		// for rectangular measurement set regions...
		virtual void generate_nonimage_statistics( DisplayData*, std::list<RegionInfo> * ) { }

		virtual ImageRegion *get_image_region( DisplayData* ) const
			DISPLAY_PURE_VIRTUAL(Region::get_image_region,0);
		virtual ImageRegion_state get_image_selected_region( DisplayData* );

		virtual const std::list<Region*> &get_selected_regions( ) = 0;
		virtual size_t selected_region_count( )
			DISPLAY_PURE_VIRTUAL(Region::selected_region_count,0);
		virtual size_t marked_region_count( ) = 0;
			/* DISPLAY_PURE_VIRTUAL(Region::marked_region_count,0); */

		virtual std::list<RegionInfo> *generate_dds_centers(bool )
			DISPLAY_PURE_VIRTUAL(Region::generate_dds_centers, new std::list<RegionInfo>( ));

		static Int getAxisIndex( ImageInterface<Float> *image, std::string axtype );

		inline double linear_average( double a, double b ) const { return (a + b) / 2.0; }
		RegionInfo::center_t *getLayerCenter( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg);
		RegionInfo::stats_t  *getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg );

		Units current_xunits( ) const;
		Units current_yunits( ) const;
		Coord current_region_coordsys( ) const;
		MDirection::Types current_casa_coordsys( ) const;

		virtual void drawRegion( bool /*selected*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::drawRegion,);
		virtual void drawText( );

		virtual void setCenter(double &, double &, double &, double &) DISPLAY_PURE_VIRTUAL(Region::setCenter,);

		virtual void drawCenter(double &x, double &y );
		virtual void drawCenter(double &x, double &y, double &deltx, double &delty);


		virtual bool within_drawing_area( );

		LineStyle current_ls;
		typedef std::pair<LineStyle,int> ls_ele;
		std::list<ls_ele> ls_stack;
		WorldCanvas *wc_;

		int last_z_index;
		bool selected_;

		bool visible_;

		// Should this region be considered complete?
		// Set to true by derived classes...
		bool complete;

		// Derived classes set this to true and clear it when the mouse
		// enters and exits this regions bounding box...
		bool mouse_in_region;

	    private:
		void set_line_style(const ls_ele&);
		bool draw_center_;

	};

	// used to pass point specific marker information (marker type and scaling)
	// to generic region creation routines RegionTool::create(...)
	class PointMarkerState : public VOID {
	    public:
		PointMarkerState( QtMouseToolNames::PointRegionSymbols t, int s ) : VOID("viewer.PointMarkerState"), type_(t), scale_(s) { }
		QtMouseToolNames::PointRegionSymbols type( ) const { return type_; }
		int scale( ) const { return scale_; }
	    private:
		QtMouseToolNames::PointRegionSymbols type_;
		int scale_;
	};
    }
}

#endif
