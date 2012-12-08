//# QtRegion.h: base class for statistical regions
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


#ifndef REGION_QTREGION_H_
#define REGION_QTREGION_H_

#include <display/Display/MouseToolState.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/Region.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

    class AnnotationBase;
    class RegionTextList;
    class DisplayData;

    namespace viewer {

	class QtRegionDock;
	class QtRegionState;
	class ds9writer;

	class QtRegionSourceKernel;

	// Key points:
	//    <ul>
	//        <li> regions are produced by a factory to permit the creation of gui specific regions </li>
	//    </ul>
	class QtRegion : public QObject {
	    Q_OBJECT

	    public:

		// create a deginerate region just to gain access to the load regions dialog...
		QtRegion( QtRegionSourceKernel *factory );

		QtRegion( const QString &nme, QtRegionSourceKernel *factory, bool hold_signals_=false,
			  QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN );
		virtual ~QtRegion( );

		// needed for writing out a list of regions (CASA or DS9 format) because the
		// output is based upon QtRegionState pointers (because that is what is available
		// to the QtRegionDock... this should be rectified to use a list of QtRegion
		// pointers for region output...
		QtRegionState *state( ) { return mystate; }

		const std::string name( ) const { return name_.toStdString( ); }
		virtual QtMouseToolNames::PointRegionSymbols marker( ) const
				{ return QtMouseToolNames::SYM_UNKNOWN; }
		virtual bool setMarker( QtMouseToolNames::PointRegionSymbols )
				{ return false; }

		std::string lineColor( ) const { return mystate->lineColor( ); }
		std::string centerColor( ) const { return mystate->centerColor( ); }
		int lineWidth( ) const { return mystate->lineWidth( ); }
		Region::LineStyle lineStyle( ) const { return mystate->lineStyle( ); }

		int markerScale( ) const { return mystate->markerScale( ); }
		void setMarkerScale( int v ) { mystate->setMarkerScale(v); }

		std::string textColor( ) const { return mystate->textColor( ); }
		std::string textFont( ) const { return mystate->textFont( ); }
		int textFontSize( ) const { return mystate->textFontSize( ); }
		int textFontStyle( ) const { return mystate->textFontStyle( ); }
		std::string textValue( ) const { return mystate->textValue( ); }
		Region::TextPosition textPosition( ) const { return mystate->textPosition( ); }
		void textPositionDelta( int &x, int &y ) const { return mystate->textPositionDelta( x, y ); }

		// set attributes when loading a casa region text file...
		virtual void setLabel( const std::string &l );
		void setLabelPosition( Region::TextPosition );
		void setLabelDelta( const std::vector<int> & );
		virtual void setFont( const std::string &font="", int font_size=0, int font_style=0, const std::string &font_color="" );
		virtual void setLine( const std::string &line_color="", Region::LineStyle line_style=Region::SolidLine, unsigned int line_width=1 );
		virtual void setAnnotation(bool ann);

		int numFrames( ) const;
		void zRange( int &x, int &y ) const;
		virtual int zIndex( ) const DISPLAY_PURE_VIRTUAL(Region::zIndex,0);
		virtual bool regionVisible( ) const DISPLAY_PURE_VIRTUAL(Region::regionVisible,true);

		virtual void linearCenter( double &/*x*/, double &/*y*/ ) const = 0;
		  // DISPLAY_PURE_VIRTUAL(Region::linearCenter,);
		virtual void pixelCenter( double &/*x*/, double &/*y*/ ) const = 0;
		  // DISPLAY_PURE_VIRTUAL(Region::pixelCenter,);

		virtual void refresh( ) DISPLAY_PURE_VIRTUAL(Region::refresh,);
		virtual AnnotationBase *annotation( ) const DISPLAY_PURE_VIRTUAL(Region::annotation,0);

		// indicates that the user has selected this rectangle...
		void selectedInCanvas( );
		// is this region weakly or temporarily selected?
		bool weaklySelected( ) const;
		void weaklySelect( );
		void weaklyUnselect( );
		// indicates that region movement requires the update of state information...
		void updateStateInfo( bool region_modified, Region::RegionChanges );
		void clearStatistics( );
		void statisticsUpdateNeeded( ) { statistics_update_needed = true; }

		virtual void getCoordinatesAndUnits( Region::Coord &c, Region::Units &x_units, Region::Units &y_units,
						     std::string &width_height_units ) const = 0; //DISPLAY_PURE_VIRTUAL(Region::getCoordinatesAndUnits,);
		virtual void getPositionString( std::string &x, std::string &y, std::string &angle,
						double &bounding_width, double &bounding_height,
						Region::Coord coord = Region::DefaultCoord,
						Region::Units x_units = Region::DefaultUnits,
						Region::Units y_units = Region::DefaultUnits,
						const std::string &bounding_units = "rad" ) const = 0; //DISPLAY_PURE_VIRTUAL(Region::getPositionString,);

		virtual bool translateX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::movePosition,false);
		virtual bool translateY( const std::string &/*y*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::movePosition,false);
		virtual bool resizeX( const std::string &/*x*/, const std::string &/*x_units*/, const std::string &/*coordsys*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::movePosition,false);
		virtual bool resizeY( const std::string &/*y*/, const std::string &/*y_units*/, const std::string &/*coordsys*/ ) = 0; //DISPLAY_PURE_VIRTUAL(Region::movePosition,false);

		void holdSignals( ) { hold_signals++; }
		void releaseSignals( );

		// functions added with the introduction of RegionToolManager and the
		// unified selection and manipulation of the various region types...
		void mark( bool set=true );
		bool marked( ) const { return mystate->marked( ); }
		bool mark_toggle( );

		void status( const std::string &msg, const std::string &type="info" );

		bool markCenter( ) const { return mystate->markCenter( ); }

		bool skyComponent( ) const {return mystate->skyComponent();};

		virtual void output( ds9writer &out ) const = 0;

		// used to synchronize all of the RegionDock's RegionState tab configuration...
		std::pair<int,int> &tabState( );
		// used to synchronize all of the RegionDock's RegionState coordinate configuration...
		std::map<std::string,int> &coordState( );
		// used to synchronize the default color for all of the RegionDock's RegionState objects...
		int &colorIndex( );
		// forward state update information to the dock wherere a count of selected regions, information
		// is maintained. This is used to determine the corner treatment when drawing regions...
		void selectedCountUpdateNeeded( );

		// used to synchronize all region directories per QtDisplayPanelGUI...
		QString getSaveDir( );
		void putSaveDir( QString );
		QString getLoadDir( );
		void putLoadDir( QString );

		// called for existing regions when spectral profile tool is opened...
		void emitUpdate( );
		ImageRegion * getImageRegion( DisplayData* dd ) const {
					return get_image_region( dd );
				}
		int getId() const {
			return id_;
		}

	    public slots:
		/* void name( const QString &newname ); */
		/* void color( const QString &newcolor ); */
	    signals:
		void regionChange( viewer::QtRegion *, std::string );
		void regionCreated( int, const QString &shape, const QString &name,
				    const QList<double> &world_x, const QList<double> &world_y,
				    const QList<int> &pixel_x, const QList<int> &pixel_y,
				    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );
		void regionUpdate( int, viewer::Region::RegionChanges, const QList<double> &world_x, const QList<double> &world_y,
				   const QList<int> &pixel_x, const QList<int> &pixel_y );
		// generated by emitUpdate( )...
		void regionUpdateResponse( int, const QString &shape, const QString &name,
					   const QList<double> &world_x, const QList<double> &world_y,
					   const QList<int> &pixel_x, const QList<int> &pixel_y,
					   const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		void selectionChanged(viewer::QtRegion*,bool);

		/* void updated( ); */
		/* void deleted( const QtRegion * ); */

	    protected slots:
		void refresh_canvas_event( );
		void reload_statistics_event( );
		void refresh_statistics_event( bool );
		void refresh_position_event( bool );

		void translate_x( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
		void translate_y( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );
		void resize_x( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
		void resize_y( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );

		void refresh_zrange_event(int,int);
		// revoke...
		void revoke_region( );
		// revoke if our state matches parameter...
		void revoke_region(QtRegionState*);
		void output(std::list<QtRegionState*>,RegionTextList&);
		void output(std::list<QtRegionState*>,ds9writer&);
		void updateCenterInfo();
		void invalidateCenterInfo( );


	    protected:
		virtual ImageRegion *get_image_region( DisplayData* ) const = 0; /*DISPLAY_PURE_VIRTUAL(Region::get_image_region,0);*/
		virtual std::list<RegionInfo> *generate_dds_statistics( ) DISPLAY_PURE_VIRTUAL(Region::generate_dds_statistics,0);

		const std::list<Region*> &get_selected_regions( );
		size_t selected_region_count( );
		size_t marked_region_count( );
		// At the base of this inheritance hierarchy is a class that uses
		// multiple inheritance. We are QtRegion is one base class, and we
		// need to be able to retrieve our peer (the non-GUI dependent)
		// Region class pointer...
		virtual Region *fetch_my_region( ) DISPLAY_PURE_VIRTUAL(Region::fetch_my_region,0);
		/* virtual void fetch_region_details( Region::RegionTypes &/\*type*\/, */
		/*                                    std::vector<std::pair<int,int> > &/\*pixel_pts*\/, */
		/* 				   std::vector<std::pair<double,double> > &/\*world_pts*\/ ) const */
		/* 				DISPLAY_PURE_VIRTUAL(QtRegion::fetch_region_details,); */
		virtual void fetch_region_details( Region::RegionTypes &/*type*/,
		                                   std::vector<std::pair<int,int> > &/*pixel_pts*/,
						   std::vector<std::pair<double,double> > &/*world_pts*/ ) const { }

		virtual std::list<RegionInfo> *generate_dds_centers( ) DISPLAY_PURE_VIRTUAL(QtRegion::generate_dds_centers, 0);


		void signal_region_change( Region::RegionChanges change );

		bool statistics_visible;
		bool statistics_update_needed;
		bool position_visible;
		bool position_update_needed;

		QtRegionSourceKernel *source_;
		QtRegionDock *dock_;
		QtRegionState *mystate;
		QString name_;
		QString color_;

	    private:
		std::map<Region::RegionChanges,bool> held_signals;
		void fetch_details( Region::RegionTypes &type, QList<int> &pixelx, QList<int> &pixely, QList<double> &worldx, QList<double> &worldy );
		void clear_signal_cache( );
		int hold_signals;
		bool z_index_within_range;
		int id_;
	};
    }
}

#endif
