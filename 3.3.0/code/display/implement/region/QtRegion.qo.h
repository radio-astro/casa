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
#include <display/region/Region.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

    class AnnRegion;

    namespace viewer {

	class QtRegionDock;
	class QtRegionSource;
	class QtRegionState;

	// Key points:
	//    <ul>
	//        <li> regions are produced by a factory to permit the creation of gui specific regions </li>
	//    </ul>
	class QtRegion : public QObject {
	    Q_OBJECT
	    public:

		enum RegionChanges { RegionChangeCreate, RegionChangeUpdate, RegionChangeLabel };

		QtRegion( const QString &nme, QtRegionSource *factory, bool hold_signals_=false );
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

		int numFrames( ) const;
		void zRange( int &x, int &y ) const;
		virtual int zIndex( ) const DISPLAY_PURE_VIRTUAL(Region::zIndex,0);
		virtual bool regionVisible( ) const DISPLAY_PURE_VIRTUAL(Region::regionVisible,true);

		virtual void regionCenter( double &x, double &y ) const DISPLAY_PURE_VIRTUAL(Region::regionCenter,);

		virtual void refresh( ) DISPLAY_PURE_VIRTUAL(Region::refresh,);
		virtual AnnRegion *annotation( ) const DISPLAY_PURE_VIRTUAL(Region::annotation,0);

		// indicates that the user has selected this rectangle...
		void selectedInCanvas( );

		// indicates that region movement requires the update of state information...
		void updateStateInfo( bool region_modified );
		void clearStatistics( );

		virtual void getCoordinatesAndUnits( Region::Coord &c, Region::Units &u ) const DISPLAY_PURE_VIRTUAL(Region::getCoordinatesAndUnits,);
		virtual void getPositionString( std::string &x, std::string &y, std::string &angle,
						Region::Coord coord = Region::DefaultCoord,
						Region::Units units = Region::DefaultUnits ) const DISPLAY_PURE_VIRTUAL(Region::getPositionString,);
		virtual void movePosition( const std::string &x, const std::string &y,
					   const std::string &coord, const std::string &units ) DISPLAY_PURE_VIRTUAL(Region::movePosition,);

		void holdSignals( ) { hold_signals++; }
		void releaseSignals( );

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

		/* void updated( ); */
		/* void deleted( const QtRegion * ); */

	    protected slots:
		void refresh_canvas_event( );
		void refresh_statistics_event( bool );
		void refresh_position_event( bool );
		void position_move_event( const QString &x, const QString &y, const QString &coord, const QString &units );
		void refresh_zrange_event(int,int);
		void revoke_region(QtRegionState*);
		void output(std::list<QtRegionState*>,std::ostream&);

	    protected:
		virtual std::list<RegionInfo> *generate_dds_statistics( ) DISPLAY_PURE_VIRTUAL(Region::generate_dds_statistics,0);
		// At the base of this inheritance hierarchy is a class that uses
		// multiple inheritance. We are QtRegion is one base class, and we
		// need to be able to retrieve our peer (the non-GUI dependent)
		// Region class pointer...
		virtual Region *fetch_my_region( ) DISPLAY_PURE_VIRTUAL(Region::fetch_my_region,0);
		virtual void fetch_region_details( Region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
						   std::vector<std::pair<double,double> > &world_pts ) const 
						DISPLAY_PURE_VIRTUAL(Region::fetch_region_details,);



		void signal_region_change( RegionChanges change );

		bool statistics_visible;
		bool statistics_update_needed;
		bool position_visible;
		bool position_update_needed;

		QtRegionSource *source_;
		QtRegionDock *dock_;
		QtRegionState *mystate;
		typedef std::list<QtRegionState*> freestate_list;
		static freestate_list *freestates;
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
