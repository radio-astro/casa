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

#include <list>
#include <string>
#include <casadbus/types/ptr.h>
#include <images/Images/ImageStatistics.h>
#include <measures/Measures/MDirection.h>

namespace casa {

    class WorldCanvas;
    class PrincipalAxesDD;
    template <class T> class ImageInterface;
    class ImageRegion;

    namespace viewer {

	struct internal_error : public std::exception {
	    internal_error( const char *err="internal viewer inconsistency" ) : str(err) { }
	    ~internal_error( ) throw( ) { }
	    const char* what() const throw() { return str.c_str( ); }
	    const std::string str;
	};

	// convert linear coordinates to viewer screen coordinates...
	void linear_to_screen( WorldCanvas *wc_, double, double, int &, int & );
	void linear_to_screen( WorldCanvas *wc_, double, double, double, double, int &, int &, int &, int & );
	// convert linear coordinates to casa pixel coordinates...
	void linear_to_pixel( WorldCanvas *wc_, double, double, int &, int & );
	void linear_to_pixel( WorldCanvas *wc_, double, double, double, double, int &, int &, int &, int & );
	// convert viewer screen coordinates to linear coordinates...
	void screen_to_linear( WorldCanvas *wc_, int, int, double &, double & );
	void screen_to_linear( WorldCanvas *wc_, int, int, int, int, double &, double &, double &, double & );
	// convert linear coordinates to world coordinates...
	void linear_to_world( WorldCanvas *wc_, double, double, double &, double & );
	void linear_to_world( WorldCanvas *wc_, double, double, double, double, double &, double &, double &, double & );
	// convert world coordinates to linear coordinates...
	void world_to_linear( WorldCanvas *wc_, double, double, double &, double & );
	// convert casa pixel coordinates to world coordinates...
	void pixel_to_world( WorldCanvas *wc_, int, int, double &, double & );
	// convert casa pixel coordinates to linear coordinates...
	void pixel_to_linear( WorldCanvas *wc_, int, int, double &, double & );

	// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
	// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
	// this means that coordinate transformation is required each time the region is drawn.
	class Region {
	    public:

		/* enum states { undisplayed, inactive, highlighted, selected }; */
		// LSDoubleDashed is only used to preserve state (it is a Display::LineStyle option)
		enum LineStyle { SolidLine, DashLine, DotLine, LSDoubleDashed };
		enum TextPosition { TopText, RightText, BottomText, LeftText };
		enum TextFontStyle { ItalicText = 1 << 0, BoldText = 1 << 1 };
		enum Coord { J2000, B1950, Galactic, SuperGalactic, Ecliptic, Pixel, DefaultCoord };
		enum Units { Degrees, Radians, HMS, DMS, DefaultUnits };

		// state returned from mouse functions for regions...
		enum MouseState { MouseRefresh = 1 << 0, MouseSelected = 1 << 1, MouseHandle = 1 << 2 };
		// functions to query results from mouse functions...
		static bool refreshNeeded( int v ) { return v & MouseRefresh ? true : false; }
		static bool regionSelected( int v ) { return v & MouseSelected ? true : false; }
		static bool handleSelected( int v ) { return v & MouseHandle ? true : false; }

		// user specified name
		virtual const std::string name( ) const = 0;
		virtual bool modified( ) const = 0;

		virtual std::string lineColor( ) const = 0;
		virtual int lineWidth( ) const = 0;
		virtual LineStyle lineStyle( ) const = 0;


		virtual std::string textColor( ) const = 0;
		virtual std::string textFont( ) const = 0;
		virtual int textFontSize( ) const = 0;
		virtual int textFontStyle( ) const = 0;
		virtual std::string textValue( ) const = 0;
		virtual TextPosition textPosition( ) const = 0;
		virtual void textPositionDelta( int &x, int &y ) const = 0;

		void getCoordinatesAndUnits( Region::Coord &c, Region::Units &u ) const;
		void getPositionString( std::string &x, std::string &y, std::string &angle,
					Region::Coord coord = Region::DefaultCoord,
					Region::Units units = Region::DefaultUnits ) const;
		void movePosition( const std::string &x, const std::string &y,
				   const std::string &coord, const std::string &units );


		// one would expect the "number of frames" in our composite cube (including
		// multiple images) to be contained in the non-GUI portion of the viewer
		// hierarchy, but rather it is within the Qt portion... thus this function
		// to fetch it...   <drs>
		virtual int numFrames( ) const = 0;
		virtual void zRange( int &min, int &max ) const = 0;
		int zIndex( ) const;
		bool regionVisible( ) const { return visible_; }

		virtual ~Region( ){ }

		Region( ) : wc_(0), selected_(false) { }
		Region( WorldCanvas *wc );

		void setDrawingEnv( );
		void resetDrawingEnv( );
		void setTextEnv( );
		void resetTextEnv( );
		void pushDrawingEnv( LineStyle ls );
		void popDrawingEnv( );

		// duplicate of MultiWCTool::refresh( )
		void refresh( );

		// returns true when refresh is needed
		virtual bool mouseMovement( int x, int y ) { return false; }
		virtual void move( double dx, double dy ) = 0;

		typedef ImageStatistics<Float>::stat_list getLayerStats_t;
		typedef std::list<std::pair<String,memory::cptr<getLayerStats_t> > > StatisticsList;
		
		virtual void draw( );

		// indicates that region movement requires that the statistcs be updated...
		virtual void updateStateInfo( bool region_modified ) = 0;

		bool selected( ) const { return selected_; }

		// blank out the statistics for this region
		virtual void clearStatistics( ) = 0;

	    protected:

		inline double linear_average( double a, double b ) const { return (a + b) / 2.0; }

		getLayerStats_t *getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg );

		Units current_units( ) const;
		Coord current_region_coordsys( ) const;
		MDirection::Types current_casa_coordsys( ) const;

		// in "linear" coordinates...
		virtual void boundingRectangle( double &blc_x, double &blc_y, double &trc_x, double &trc_y ) const = 0;

		virtual void drawRegion( bool selected ) = 0;
		virtual void drawText( );

		LineStyle current_ls;
		std::list<LineStyle> ls_stack;
		WorldCanvas *wc_;

		int last_z_index;
		bool selected_;

		bool visible_;

	    private:
		void set_line_style( LineStyle linestyle );
	};
    }
}

#endif
