//# Region.qo.h: base class for statistical regions
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
#include <map>
#include <list>
#include <string>
#include <images/Images/ImageStatistics.h>
#include <measures/Measures/MDirection.h>
#include <display/region/RegionInfo.h>
#include <display/Utilities/dtor.h>
#include <display/Display/MouseToolState.h>
#include <display/Utilities/VOID.h>
#include <casaqt/QtUtilities/QtId.h>
#include <stdexcept>
#include <QObject>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/RegionEnums.h>
#include <casadbus/types/nullptr.h>

#include <casa/Arrays/Vector.h>

#include <tr1/memory>

extern "C" void casa_viewer_pure_virtual( const char *file, int line, const char *func );
#define DISPLAY_PURE_VIRTUAL(FUNCTION,RESULT) \
  { casa_viewer_pure_virtual( __FILE__, __LINE__, #FUNCTION ); return RESULT; }

namespace casa {

	class WorldCanvas;
	class PrincipalAxesDD;
	template <class T> class ImageInterface;

	class ImageRegion;
	class DisplayData;
	class HistogramTab;

	class RegionTextList;
	class AnnotationBase;

	namespace viewer {

		class ds9writer;
		class QtRegionDock;

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
		// convert world coordantes to screen coordinates
		void world_to_screen( WorldCanvas *wc_, double, double, int &, int & );
		void world_to_screen( WorldCanvas *wc_, double, double, double, double, int &, int &, int &, int & );
		// convert world to pixel coordinates
		void world_to_pixel( WorldCanvas *wc_, double, double, int &, int & );
		void world_to_pixel( WorldCanvas *wc_, double, double, double, double, int &, int &, int &, int & );

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

		MDirection::Types get_coordinate_type( const DisplayCoordinateSystem &wc );

		class ImageRegion_state {
		public:
			ImageRegion_state( ) : count_(0) { }
			ImageRegion_state( ImageRegion *ir, size_t region_count ) : imageregion(ir), count_(region_count) { }
			ImageRegion_state( const ImageRegion_state &other ) : imageregion(other.imageregion), count_(other.count_) { }
			operator std::tr1::shared_ptr<ImageRegion>( ) {
				return imageregion;
			}
			size_t regionCount( ) const {
				return count_;
			}
		private:
			void* operator new (std::size_t) throw (std::logic_error) {
				throw std::logic_error("allocating an object not intended for dynamic allocation");
			}
			std::tr1::shared_ptr<ImageRegion> imageregion;
			size_t count_;
		};

		// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
		// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
		// this means that coordinate transformation is required each time the region is drawn.
		class Region : public QObject, public dtorNotifier {
			Q_OBJECT
		public:

			// functions to query results from mouse functions...
			static bool refreshNeeded( int v ) {
				return v & region::MouseRefresh ? true : false;
			}
			static bool regionSelected( int v ) {
				return v & region::MouseSelected ? true : false;
			}
			static bool handleSelected( int v ) {
				return v & region::MouseHandle ? true : false;
			}

			// user specified name
			const std::string name( ) const {
				return name_;
			}

			std::string lineColor( ) const {
				return mystate->lineColor( );
			}
			std::string centerColor( ) const {
				return mystate->centerColor( );
			}
			int lineWidth( ) const {
				return mystate->lineWidth( );
			}
			region::LineStyle lineStyle( ) const {
				return mystate->lineStyle( );
			}

			std::string textColor( ) const {
				return mystate->textColor( );
			}
			std::string textFont( ) const {
				return mystate->textFont( );
			}
			int textFontSize( ) const {
				return mystate->textFontSize( );
			}
			int textFontStyle( ) const {
				return mystate->textFontStyle( );
			}
			std::string textValue( ) const {
				return mystate->textValue( );
			}
			region::TextPosition textPosition( ) const {
				return mystate->textPosition( );
			}
			void textPositionDelta( int &x, int &y ) const {
				return mystate->textPositionDelta( x, y );
			}

			virtual void setLabel( const std::string &l );
			virtual void setLabelPosition( region::TextPosition );
			virtual void setLabelDelta( const std::vector<int> & );
			virtual void setFont( const std::string &font="", int font_size=-1, int font_style=0, const std::string &font_color="" );
			virtual void setLine( const std::string &line_color="", region::LineStyle line_style=region::SolidLine, unsigned int line_width=1 );
			virtual void setAnnotation(bool);

			void getCoordinatesAndUnits( region::Coord &c, region::Units &x_units, region::Units &y_units,
			                             std::string &width_height_units ) const;
			void getPositionString( std::string &x, std::string &y, std::string &angle,
			                        double &bounding_width, double &bounding_height,
			                        region::Coord coord = region::DefaultCoord,
			                        region::Units x_units = region::DefaultUnits,
			                        region::Units y_units = region::DefaultUnits,
			                        const std::string &bounding_units = "rad" ) const;

			bool translateX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ );
			bool translateY( const std::string &/*x*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ );
			bool resizeX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ );
			bool resizeY( const std::string &/*x*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ );

			int numFrames( ) const;
			void zRange( int &x, int &y ) const;
			int zIndex( ) const;
			bool regionVisible( ) const {
				return visible_;
			}

			bool worldBoundingRectangle( double &, double &, const std::string & ) const;

			virtual ~Region( );

			Region( ) : dock_(0), id_(QtId::get_id( )), wc_(0), selected_(false), visible_(true),
				mouse_in_region(false), z_index_within_range(true) { }
			Region( const std::string &name, WorldCanvas *wc,  QtRegionDock *,
					bool hold_signals_=false, QtRegionState *supplied_state=0,
			        QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN );

			// is this region degenerate?
			virtual bool degenerate( ) const;

			void setDrawingEnv( );
			void resetDrawingEnv( );
			void setTextEnv( );
			void resetTextEnv( );
			void pushDrawingEnv( region::LineStyle ls, int thickness=-1 );
			void popDrawingEnv( );

			void setDrawCenter(bool draw_center) {
				draw_center_=draw_center;
			};
			bool getDrawCenter() {
				return draw_center_;
			};

			// duplicate of MultiWCTool::refresh( )
			void refresh( );

			virtual region::PointInfo checkPoint( double x, double y ) const = 0;

			// returns OR'ed set of MouseState...
			virtual unsigned int mouseMovement( double /*x*/, double /*y*/, bool /*other_selected*/ )
			DISPLAY_PURE_VIRTUAL(Region::mouseMovement,0);

			virtual void draw( bool other_selected );

			// indicates that the center info is no longer valid
			void invalidateCenterInfo( );

			bool selected( ) const {
				return selected_;
			}

			// is this region weakly or temporarily selected?
			virtual bool weaklySelected( ) const;
			virtual void weaklySelect( bool scroll_dock=true );
			// weaklyUnselectLimited( ) plus extra processing...
			virtual void weaklyUnselect( );
			// just unmark this region (called from RegionToolManager); do not
			// do the rest of the reorganization that weaklyUnselect( ) does...
			virtual void weaklyUnselectLimited( );

			// indicates that the user has selected this rectangle...
			// ...may need to scroll region dock
			virtual void selectedInCanvas( );
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
			virtual void mark( bool set=true );
			virtual bool marked( ) const {
				return mystate->marked( );
			}
			// returns the new state...
			virtual bool mark_toggle( );
			size_t selected_region_count( );
			size_t marked_region_count( );

			void clearMouseInRegion( ) {
				mouse_in_region = false;
			}

			// update status information...
			virtual void status( const std::string &msg, const std::string &type="info" );

			virtual bool markCenter( ) const {
				return mystate->markCenter( );
			}

			virtual bool skyComponent( ) const DISPLAY_PURE_VIRTUAL(Region::skyComponent,true);

			virtual void output( ds9writer &out ) const = 0;

			// in "linear" coordinates...
			virtual void boundingRectangle (double &/*blc_x*/, double &/*blc_y*/, double &/*trc_x*/, double &/*trc_y*/) const
			DISPLAY_PURE_VIRTUAL(Region::boundingRectangle,);

			void emitUpdate( );

			// postpone signal processing (important for signals generated during construction)...
			void holdSignals( ) {
				hold_signals++;
			}
			virtual void releaseSignals( ) {
				if ( --hold_signals == 0 ) process_held_signals( );
				if ( hold_signals < 0 ) hold_signals = 0;
			}

			virtual std::list<std::tr1::shared_ptr<RegionInfo> > *statistics( ) {
				return generate_dds_statistics( );
			}

			// called when creating regions to allow suppression of corner-handle drawing...
			static std::tr1::shared_ptr<viewer::Region> creatingRegion( ) {
				return creating_region;
			}
			static void creatingRegionBegin( std::tr1::shared_ptr<viewer::Region> r ) {
				creating_region = r;
			}
			static void creatingRegionEnd( ) {
				creating_region = memory::nullptr;
			}

			//Histogram functionality
			int getId() const {
				return id_;
			}
			ImageRegion *getImageRegion( DisplayData* dd ) const {
				return get_image_region( dd );
			}

// --------------------==================== from old QtRegion ====================--------------------

			// query region type... to avoid dynamic cast ladder...
			virtual region::RegionTypes type( ) const = 0;

			virtual void pixelCenter( double &/*x*/, double &/*y*/ ) const = 0;
			// DISPLAY_PURE_VIRTUAL(Region::pixelCenter,);

			// needed for writing out a list of regions (CASA or DS9 format) because the
			// output is based upon QtRegionState pointers (because that is what is available
			// to the QtRegionDock... this should be rectified to use a list of QtRegion
			// pointers for region output...
			QtRegionState *state( ) {
				return mystate;
			}

			// how much to scale the symbol used to mark point regions...
			// assumed to go from 0 to 9...
			int markerScale( ) const {
				return mystate->markerScale( );
			}
			void setMarkerScale( int v ) {
				mystate->setMarkerScale(v);
			}

			// indicates that region movement requires the update of state information...
			void updateStateInfo( bool region_modified, region::RegionChanges );
			void refresh_state_gui( );
			// blank out the statistics for this region
			void clearStatistics( );
			void statisticsUpdateNeeded( ) {
				statistics_update_needed = true;
			}

			// used to synchronize the default color for all of the RegionDock's RegionState objects...
			int &colorIndex( );
			// used to synchronize all of the RegionDock's RegionState tab configuration...
			std::pair<int,int> &tabState( );
			// used to synchronize all of the RegionDock's RegionState coordinate configuration...
			std::map<std::string,int> &coordState( );

			// forward state update information to the dock wherere a count of selected regions, information
			// is maintained. This is used to determine the corner treatment when drawing regions...
			void selectedCountUpdateNeeded( );

			// used to synchronize all region directories per QtDisplayPanelGUI...
			QString getSaveDir( );
			void putSaveDir( QString );
			QString getLoadDir( );
			void putLoadDir( QString );

			virtual bool setMarker( QtMouseToolNames::PointRegionSymbols ) {
				return false;
			}

			virtual AnnotationBase *annotation( ) const DISPLAY_PURE_VIRTUAL(Region::annotation,0);

			static Quantum< ::casa::Vector<double> > convert_angle( double x, const std::string &xunits, double y, const std::string &yunits,
			        MDirection::Types original_coordsys, MDirection::Types new_coordsys, const std::string &new_units="rad" );

		signals:
			void selectionChanged(viewer::Region*,bool);

			void regionCreated( int, const QString &shape, const QString &name, const QList<double> &world_x,
			                    const QList<double> &world_y, const QList<int> &pixel_x, const QList<int> &pixel_y,
			                    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );
			void regionUpdate( int, viewer::region::RegionChanges, const QList<double> &world_x, const QList<double> &world_y,
			                   const QList<int> &pixel_x, const QList<int> &pixel_y );
			void regionChange( viewer::Region *, std::string );

			// generated by emitUpdate( )...
			void regionUpdateResponse( int, const QString &shape, const QString &name,
			                           const QList<double> &world_x, const QList<double> &world_y,
			                           const QList<int> &pixel_x, const QList<int> &pixel_y,
			                           const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		protected slots:
			void refresh_canvas_event( );
			void refresh_statistics_event( bool );
			void refresh_position_event( bool );

			void translate_x( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
			void translate_y( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );
			void resize_x( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
			void resize_y( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );
			void updateCenterInfo( );
			void adjustCorners( double, double, double, double);
			void refresh_zrange_event(int,int);

			// revoke...
			void revoke_region( );
			// revoke if our state matches parameter...
			void revoke_region(QtRegionState*);

			void reload_statistics_event( );

			void output(std::list<QtRegionState*>,RegionTextList&);
			void output(std::list<QtRegionState*>,ds9writer&);

		protected:

			static const int SEXAGPREC;

			QtRegionDock *dock_;
			QtRegionState *mystate;
			HistogramTab* histogram;

			bool statistics_update_needed;
			bool position_visible;
			bool position_update_needed;

			int id_;
			bool statistics_visible;

			int hold_signals;
			std::map<region::RegionChanges,bool> held_signals;

			void fetch_details( region::RegionTypes &type, QList<int> &pixelx,
			                    QList<int> &pixely, QList<double> &worldx, QList<double> &worldy );
			virtual void fetch_region_details( region::RegionTypes &type,
			                                   std::vector<std::pair<int,int> > &pixel_pts,
			                                   std::vector<std::pair<double,double> > &world_pts ) const {
				type = region::NonRegion;
				pixel_pts.clear( );
				world_pts.clear( );
				DISPLAY_PURE_VIRTUAL(Region::fetch_region_details,);
			}

			void signal_region_change( region::RegionChanges );

			// --------------------==================== from old QtRegion ====================--------------------

		protected:
			void initHistogram();
			virtual std::list<std::tr1::shared_ptr<RegionInfo> > *generate_dds_statistics( );

			// hook to allow generate_dds_statistics( ) to generate statistics
			// for rectangular measurement set regions...
			virtual void generate_nonimage_statistics( DisplayData*, std::list<std::tr1::shared_ptr<RegionInfo> > * ) { }
			// newInfoObject(...) is currently only used for PVLine regions, but it should be used for
			// other regions to allow for specialized creation of the region info objects for display
			// in "statistics"...
			virtual RegionInfo *newInfoObject( ImageInterface<Float> *, PrincipalAxesDD * ) {
				return 0;
			}

			virtual ImageRegion *get_image_region( DisplayData* ) const
			DISPLAY_PURE_VIRTUAL(Region::get_image_region,0);

			virtual const std::set<Region*> &get_selected_regions( );
			virtual ImageRegion_state get_image_selected_region( DisplayData* );

			virtual std::list<std::tr1::shared_ptr<RegionInfo> > *generate_dds_centers( ) = 0;

			static Int getAxisIndex( ImageInterface<Float> *image, std::string axtype );

			inline double linear_average( double a, double b ) const {
				return (a + b) / 2.0;
			}
			RegionInfo::center_t *getLayerCenter( PrincipalAxesDD *padd, std::tr1::shared_ptr<ImageInterface<Float> > image, ImageRegion& imgReg);
			RegionInfo::stats_t  *getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg );

			region::Units current_xunits( ) const;
			region::Units current_yunits( ) const;
			region::Coord current_region_coordsys( ) const;
			MDirection::Types current_casa_coordsys( ) const;

			virtual void drawRegion( bool /*selected*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::drawRegion,);
			virtual void drawText( );

			virtual void setCenter(double &, double &, double &, double &) DISPLAY_PURE_VIRTUAL(Region::setCenter,);

			virtual void drawCenter(double &x, double &y );
			virtual void drawCenter(double &x, double &y, double &deltx, double &delty);


			virtual bool within_drawing_area( );

			region::LineStyle current_ls;
			typedef std::pair<region::LineStyle,int> ls_ele;
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

			// Holds a pointer to the region currently being created...
			static std::tr1::shared_ptr<Region> creating_region;

		private slots:
			//Called when the histogram stack needs to be changed due
			//to the add/removal of an image.
			void update_histogram_event();

		private:
			// common state-setup for constructors...
			void init( );

			void updateHistogramRegion();
			void clearHistograms();
			void process_held_signals( );
			void clear_signal_cache( );
			bool z_index_within_range;
			void set_line_style(const ls_ele&);
			bool draw_center_;
			std::string name_;
		};

		// used to pass point specific marker information (marker type and scaling)
		// to generic region creation routines RegionTool::create(...)
		class PointMarkerState : public VOID {
		public:
			PointMarkerState( QtMouseToolNames::PointRegionSymbols t, int s ) : VOID("viewer.PointMarkerState"), type_(t), scale_(s) { }
			QtMouseToolNames::PointRegionSymbols type( ) const {
				return type_;
			}
			int scale( ) const {
				return scale_;
			}
		private:
			QtMouseToolNames::PointRegionSymbols type_;
			int scale_;
		};
	}
}

#endif
