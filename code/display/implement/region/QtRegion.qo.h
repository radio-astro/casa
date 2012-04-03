//# QtRegion.h: base class for statistical regions
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


#ifndef REGION_QTREGION_H_
#define REGION_QTREGION_H_

#include <display/region/QtRegionState.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <display/region/Region.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

    class AnnotationBase;
    class RegionTextList;

    namespace viewer {

	class QtRegionDock;
	class QtRegionState;
	class ds9writer;

	// Key points:
	//    <ul>
	//        <li> regions are produced by a factory to permit the creation of gui specific regions </li>
	//    </ul>
	class QtRegion : public QObject {
	    Q_OBJECT
	    public:

		enum RegionChanges { RegionChangeCreate, RegionChangeUpdate, RegionChangeLabel, RegionChangeDelete, RegionChangeStatsUpdate };

		// create a deginerate region just to gain access to the load regions dialog...
		QtRegion( QtRegionSourceKernel *factory );

		QtRegion( const QString &nme, QtRegionSourceKernel *factory, bool hold_signals_=false );
		virtual ~QtRegion( );

		const std::string name( ) const { return name_.toStdString( ); }

		std::string lineColor( ) const { return mystate->lineColor( ); }
		int lineWidth( ) const { return mystate->lineWidth( ); }
		Region::LineStyle lineStyle( ) const { return mystate->lineStyle( ); }

		std::string textColor( ) const { return mystate->textColor( ); }
		std::string textFont( ) const { return mystate->textFont( ); }
		int textFontSize( ) const { return mystate->textFontSize( ); }
		int textFontStyle( ) const { return mystate->textFontStyle( ); }
		std::string textValue( ) const { return mystate->textValue( ); }
		Region::TextPosition textPosition( ) const { return mystate->textPosition( ); }
		void textPositionDelta( int &x, int &y ) const { return mystate->textPositionDelta( x, y ); }

		// set attributes when loading a casa region text file...
		virtual void setLabel( const std::string &l );
		virtual void setFont( const std::string &font="", int font_size=0, int font_style=0, const std::string &font_color="" );
		virtual void setLine( const std::string &line_color="", Region::LineStyle line_style=Region::SolidLine );
		virtual void setAnnotation(bool ann);

		int numFrames( ) const;
		void zRange( int &x, int &y ) const;
		virtual int zIndex( ) const DISPLAY_PURE_VIRTUAL(Region::zIndex,0);
		virtual bool regionVisible( ) const DISPLAY_PURE_VIRTUAL(Region::regionVisible,true);

		virtual void regionCenter( double &/*x*/, double &/*y*/ ) const DISPLAY_PURE_VIRTUAL(Region::regionCenter,);

		virtual void refresh( ) DISPLAY_PURE_VIRTUAL(Region::refresh,);
		virtual AnnotationBase *annotation( ) const DISPLAY_PURE_VIRTUAL(Region::annotation,0);

		// indicates that the user has selected this rectangle...
		void selectedInCanvas( );

		// indicates that region movement requires the update of state information...
		void updateStateInfo( bool region_modified );
		void clearStatistics( );

		virtual void getCoordinatesAndUnits( Region::Coord &c, Region::Units &x_units, Region::Units &y_units,
						     std::string &width_height_units ) const = 0; //DISPLAY_PURE_VIRTUAL(Region::getCoordinatesAndUnits,);
		virtual void getPositionString( std::string &x, std::string &y, std::string &angle,
						double &bounding_width, double &bounding_height,
						Region::Coord coord = Region::DefaultCoord,
						Region::Units x_units = Region::DefaultUnits,
						Region::Units y_units = Region::DefaultUnits,
						const std::string &bounding_units = "rad" ) const = 0; //DISPLAY_PURE_VIRTUAL(Region::getPositionString,);
		virtual void movePosition( const std::string &x, const std::string &y, const std::string &coord,
					   const std::string &x_units, const std::string &y_units,
					   const std::string &width, const std::string &height, const std::string &bounding_units ) = 0; //DISPLAY_PURE_VIRTUAL(Region::movePosition,);

		void holdSignals( ) { hold_signals++; }
		void releaseSignals( );

		// functions added with the introduction of RegionToolManager and the
		// unified selection and manipulation of the various region types...
		void mark( bool set=true ) { mystate->mark( set ); }
		bool marked( ) const { return mystate->marked( ); }
		void mark_toggle( ) { mystate->mark_toggle( ); }

		virtual void output( ds9writer &out ) const = 0;

		// used to synchronize all of the RegionDock's RegionState tab configuration...
		std::pair<int,int> &tabState( );
		// used to synchronize all of the RegionDock's RegionState coordinate configuration...
		std::map<std::string,int> &coordState( );

		// called for existing regions when spectral profile tool is opened...
		void emitUpdate( );

	    public slots:
		/* void name( const QString &newname ); */
		/* void color( const QString &newcolor ); */
	    signals:
		void regionCreated( int, const QString &shape, const QString &name,
				    const QList<double> &world_x, const QList<double> &world_y,
				    const QList<int> &pixel_x, const QList<int> &pixel_y,
				    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );
		void regionUpdate( int, const QList<double> &world_x, const QList<double> &world_y,
				   const QList<int> &pixel_x, const QList<int> &pixel_y );
		// generated by emitUpdate( )...
		void regionUpdateResponse( int, const QString &shape, const QString &name,
					   const QList<double> &world_x, const QList<double> &world_y,
					   const QList<int> &pixel_x, const QList<int> &pixel_y,
					   const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		/* void updated( ); */
		/* void deleted( const QtRegion * ); */

	    protected slots:
		void refresh_canvas_event( );
		void refresh_statistics_event( bool );
		void refresh_position_event( bool );
		void position_move_event( const QString &x, const QString &y, const QString &coord,
					  const QString &x_units, const QString &y_units,
					  const QString &width, const QString &height, const QString &bounding_units );
		void refresh_zrange_event(int,int);
		void revoke_region(QtRegionState*);
		void output(std::list<QtRegionState*>,RegionTextList&);
		void output(std::list<QtRegionState*>,ds9writer&);

	    protected:
		virtual std::list<RegionInfo> *generate_dds_statistics( ) DISPLAY_PURE_VIRTUAL(Region::generate_dds_statistics,0);
		// At the base of this inheritance hierarchy is a class that uses
		// multiple inheritance. We are QtRegion is one base class, and we
		// need to be able to retrieve our peer (the non-GUI dependent)
		// Region class pointer...
		virtual Region *fetch_my_region( ) DISPLAY_PURE_VIRTUAL(Region::fetch_my_region,0);
		virtual void fetch_region_details( Region::RegionTypes &/*type*/,
		                                   std::vector<std::pair<int,int> > &/*pixel_pts*/,
						   std::vector<std::pair<double,double> > &/*world_pts*/ ) const
						DISPLAY_PURE_VIRTUAL(Region::fetch_region_details,);



		void signal_region_change( RegionChanges change );

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
		std::map<RegionChanges,bool> held_signals;
		void fetch_details( Region::RegionTypes &type, QList<int> &pixelx, QList<int> &pixely, QList<double> &worldx, QList<double> &worldy );
		void clear_signal_cache( );
		int hold_signals;
		bool z_index_within_range;
		int id_;
	};
    }
}

#endif
