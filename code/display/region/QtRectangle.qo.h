//# QtRectangle.h: base class for statistical regions
//# Copyright (C) 2011,2012
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


#ifndef REGION_QTRECTANGLE_H_
#define REGION_QTRECTANGLE_H_

#include <display/region/QtRegion.qo.h>
#include <display/region/Rectangle.h>

namespace casa {
    namespace viewer {

	// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
	// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
	// this means that coordinate transformation is required each time the region is drawn.
	//
	// Key points:
	//    <ul>
	//        <li> regions are produced by a factory to permit the creation of gui specific regions </li>
	//    </ul>
	class QtRectangle : public QtRegion, public Rectangle {
	    Q_OBJECT
	    public:
		~QtRectangle( );

		const std::string name( ) const { return QtRegion::name( ); }

		std::string lineColor( ) const   { return QtRegion::lineColor( ); }
		std::string centerColor( ) const { return QtRegion::centerColor( ); }
		int lineWidth( ) const { return QtRegion::lineWidth( ); }
		Region::LineStyle lineStyle( ) const { return QtRegion::lineStyle( ); }


		std::string textColor( ) const { return QtRegion::textColor( ); }
		std::string textFont( ) const { return QtRegion::textFont( ); }
		int textFontSize( ) const { return QtRegion::textFontSize( ); }
		int textFontStyle( ) const { return QtRegion::textFontStyle( ); }
		std::string textValue( ) const { return QtRegion::textValue( ); }
		Region::TextPosition textPosition( ) const { return QtRegion::textPosition( ); }
		void textPositionDelta( int &x, int &y ) const { QtRegion::textPositionDelta( x, y ); }

		void getCoordinatesAndUnits( Region::Coord &c, Region::Units &x_units, Region::Units &y_units, std::string &width_height_units ) const
			{ Region::getCoordinatesAndUnits( c, x_units, y_units, width_height_units ); }
		void getPositionString( std::string &x, std::string &y, std::string &angle,
					double &bounding_width, double &bounding_height,
					Region::Coord coord = Region::DefaultCoord,
					Region::Units x_units = Region::DefaultUnits,
					Region::Units y_units = Region::DefaultUnits,
					const std::string &bounding_units = "rad" ) const
			{ Region::getPositionString( x, y, angle, bounding_width, bounding_height, coord, x_units, y_units, bounding_units ); }

		bool translateX( const std::string &x, const std::string &x_units, const std::string &coordsys )
			{ return Region::translateX( x, x_units, coordsys ); }
		bool translateY( const std::string &y, const std::string &y_units, const std::string &coordsys )
			{ return Region::translateY( y, y_units, coordsys ); }
		bool resizeX( const std::string &x, const std::string &x_units, const std::string &coordsys )
			{ return Region::resizeX( x, x_units, coordsys ); }
		bool resizeY( const std::string &y, const std::string &y_units, const std::string &coordsys )
			{ return Region::resizeY( y, y_units, coordsys ); }

		int numFrames( ) const { return QtRegion::numFrames( ); }
		void zRange( int &min, int &max ) const { QtRegion::zRange(min,max); }
		int zIndex( ) const { return Region::zIndex( ); }

		bool regionVisible( ) const { return Region::regionVisible( ); }
		void linearCenter( double &x, double &y ) const { Rectangle::linearCenter( x, y ); }
		void pixelCenter( double &x, double &y ) const { Rectangle::pixelCenter( x, y ); }

		QtRectangle( QtRegionSourceKernel *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y, bool hold_signals=false );

		// qt-event -> QtRegion -> QtRectangle -> Region::refresh( )
		void refresh( ) { Rectangle::refresh( ); }
		AnnotationBase *annotation( ) const { return Rectangle::annotation( ); }

		// indicates that the user has selected this rectangle...
		void selectedInCanvas( ) { QtRegion::selectedInCanvas( ); }
		// is this region weakly or temporarily selected?
		bool weaklySelected( ) const { return QtRegion::weaklySelected( ); }
		void weaklySelect( ) { QtRegion::weaklySelect( ); }
		void weaklyUnselect( ) { QtRegion::weaklyUnselect( ); }

		// indicates that region movement requires that the statistcs be updated...
		void updateStateInfo( bool region_modified, Region::RegionChanges change ) { QtRegion::updateStateInfo( region_modified, change ); }

		// indicates that the center info is no longer valid
		void invalidateCenterInfo( ) {QtRegion::invalidateCenterInfo();};

		void clearStatistics( ) { QtRegion::clearStatistics( ); }

		void setLabel( const std::string &l ) { QtRegion::setLabel(l); }
		void setLabelPosition( TextPosition pos ) { QtRegion::setLabelPosition(pos); }
		void setLabelDelta( const std::vector<int> &delta ) { QtRegion::setLabelDelta(delta); }
		void setFont( const std::string &font="", int font_size=-1, int font_style=0, const std::string &font_color="" )
				{ QtRegion::setFont( font, font_size, font_style, font_color ); }
		void setLine( const std::string &line_color="", Region::LineStyle line_style=SolidLine, unsigned int line_width=1 )
				{ QtRegion::setLine( line_color, line_style, line_width ); }
		void setAnnotation(bool ann) { QtRegion::setAnnotation(ann); }

		// functions added with the introduction of RegionToolManager and the
		// unified selection and manipulation of the various region types...
		void mark( bool set=true ) { QtRegion::mark( set ); }
		bool marked( ) const { return QtRegion::marked( ); }
		bool mark_toggle( ) { return QtRegion::mark_toggle( ); }

		void status( const std::string &msg, const std::string &type="info" ) { QtRegion::status(msg,type); }

		bool markCenter() const { return QtRegion::markCenter( ); }

		bool skyComponent() const { return QtRegion::skyComponent( ); }

		void output( ds9writer &out ) const;

		void emitUpdate( ) { QtRegion::emitUpdate( ); }

		public slots:
			void adjustCorners( double, double, double, double );

	    protected:
		std::list<RegionInfo> *generate_dds_statistics( ) { return Rectangle::generate_dds_statistics( ); }
		ImageRegion *get_image_region( DisplayData *dd ) const { return Rectangle::get_image_region( dd ); }
		const std::list<Region*> &get_selected_regions( ) { return QtRegion::get_selected_regions( ); }
		size_t selected_region_count( ) { return QtRegion::selected_region_count( ); }
		size_t marked_region_count( ) { return QtRegion::marked_region_count( ); }
		virtual Region *fetch_my_region( ) { return (Region*) this; }
		virtual void fetch_region_details( RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
						   std::vector<std::pair<double,double> > &world_pts ) const 
				{ return Rectangle::fetch_region_details( type, pixel_pts, world_pts ); }
		std::list<RegionInfo> *generate_dds_centers( ) { return Rectangle::generate_dds_centers( ); };

	};
    }
}

#endif
