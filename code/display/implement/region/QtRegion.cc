
#include <casa/BasicSL/String.h>
#include <display/DisplayEvents/DTVisible.h>
#include <display/region/QtRegion.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <display/region/QtRegionDock.qo.h>
#include <imageanalysis/Annotations/AnnRegion.h>
#include <casaqt/QtUtilities/QtId.h>

namespace casa {
    namespace viewer {

	QtRegion::freestate_list *QtRegion::freestates = 0;

	QtRegion::QtRegion( const QString &nme, QtRegionSource *factory, bool hold_signals_ ) :
			source_(factory), dock_(factory->dock()), name_(nme), hold_signals(hold_signals_ ? 1 : 0),
			z_index_within_range(true), id_(QtId::get_id( )) {
	    statistics_visible = position_visible = false;
	    statistics_update_needed = position_update_needed = true;

	    static DTVisible aipsrc;
	    color_ = QString::fromStdString(aipsrc.drawColor( ));

	    if ( dock_ == 0 )
		throw internal_error( "no dock widget is available" );

	    if ( freestates == 0 )
		freestates = new freestate_list( );

	    // BEGIN - critical section
	    if ( freestates->size() > 0 ) {
		mystate = freestates->back( );
		freestates->pop_back( );
		// END - critical section
		mystate->reset(name_,this);
	    } else {
		mystate = new QtRegionState(name_,this);
	    }

	    connect( mystate, SIGNAL(refreshCanvas( )), SLOT(refresh_canvas_event( )) );
	    connect( mystate, SIGNAL(statisticsVisible(bool)), SLOT(refresh_statistics_event(bool)) );
	    connect( mystate, SIGNAL(positionVisible(bool)), SLOT(refresh_position_event(bool)) );
	    connect( mystate, SIGNAL(positionMove( const QString &, const QString &, const QString &, const QString &)),
		     SLOT(position_move_event( const QString &, const QString &, const QString &, const QString &)) );
				     
	    connect( mystate, SIGNAL(zRange(int,int)), SLOT(refresh_zrange_event(int,int)) );
	    connect( dock_, SIGNAL(deleteRegion(QtRegionState*)), SLOT(revoke_region(QtRegionState*)) );
	    connect( dock_, SIGNAL(outputRegions(std::list<QtRegionState*>, std::ostream&)), SLOT(output(std::list<QtRegionState*>,std::ostream&)) );

	    dock_->addRegion(mystate);
	    signal_region_change( RegionChangeCreate );
	}

	QtRegion::~QtRegion( ) {
	    dock_->removeRegion(mystate);
	    disconnect(mystate, 0, 0, 0);
	    freestates->push_back(mystate);
	}

	int QtRegion::numFrames( ) const { return source_->numFrames( ); }

	void QtRegion::zRange( int &min, int &max ) const {
	    min = mystate->zMin( );
	    max = mystate->zMax( );
	}

#if OLDSTUFF
	void QtRegion::clearstats( ) { mystate->clearstats( ); }
	void QtRegion::addstats( const std::string &name, std::list<std::pair<String,String> > *stats )
	    { mystate->addstats( name, stats ); }

#endif
	void QtRegion::selectedInCanvas( ) { dock_->selectRegion(mystate); }

        // indicates that region movement requires that the statistcs be updated...
	void QtRegion::updateStateInfo( bool region_modified ) {

	    signal_region_change( RegionChangeUpdate );

	    // update statistics, when needed...
	    if ( statistics_visible == false ) {
		if ( region_modified ) statistics_update_needed = true;
	    } else if ( (statistics_update_needed || region_modified) && regionVisible( ) ) {
		statistics_update_needed = false;
		Region::StatisticsList *rl = generate_statistics_list( );
		// send statistics to region state object...
		mystate->updateStatistics(rl);
	    }

	    // update position, when needed...
	    if ( position_visible == false ) {
		if ( region_modified ) position_update_needed = true;
	    } else if ( (position_update_needed || region_modified) && regionVisible( ) ) {
		Region::Coord c;
		Region::Units u;
		std::string x, y, angle;
		static bool first_time_through = true;
		if ( first_time_through ) {
		    getCoordinatesAndUnits( c, u );
		    mystate->setCoordinatesAndUnits( c, u );
		    first_time_through = false;
		}

		mystate->getCoordinatesAndUnits( c, u );
		getPositionString( x, y, angle, c, u );
		mystate->updatePosition( QString::fromStdString(x),
					 QString::fromStdString(y),
					 QString::fromStdString(angle) );
	    }
	      
	}

	void QtRegion::clearStatistics( ) {
	    statistics_update_needed = true;
	    mystate->clearStatistics( );
	}

	void QtRegion::refresh_statistics_event( bool visible ) {
	    statistics_visible = visible;
	    updateStateInfo( false );
	}

	void QtRegion::refresh_position_event( bool visible ) {
	    position_visible = visible;
	    updateStateInfo( false );
	}

	void QtRegion::position_move_event( const QString &x, const QString &y, const QString &coord, const QString &units ) {
	    movePosition( x.toStdString( ), y.toStdString( ), coord.toStdString( ), units.toStdString( ) );
	}

	void QtRegion::refresh_zrange_event( int min, int max ) {
	    int index = zIndex( );
	    if ( z_index_within_range == true && (index < min || index > max) ) {
		z_index_within_range = false;
		refresh( );
	    } else if ( z_index_within_range == false && index >= min && index <= max ) {
		z_index_within_range = true;
		refresh( );
	    }
	}
	void QtRegion::refresh_canvas_event( ) { refresh( ); }

	void QtRegion::revoke_region( QtRegionState *redacted_state ) {
	    if ( redacted_state == mystate ) {
		source_->revokeRegion(fetch_my_region( ));
	    }
	}

	static inline AnnotationBase::LineStyle viewer_to_annotation( Region::LineStyle ls ) {
	    return ls == Region::SolidLine ? AnnotationBase::SOLID : ls == Region::DotLine ? AnnotationBase::DOTTED : AnnotationBase::DASHED;
	}

	void QtRegion::output( std::list<QtRegionState*> ol, std::ostream &out ) {
	    std::list<QtRegionState*>::iterator iter = find( ol.begin(), ol.end( ), mystate );
	    if ( iter != ol.end( ) ) {

		AnnRegion *ann = annotation( );

		int number_frames = (*iter)->numFrames( );
		ann->setLabel( (*iter)->textValue( ) );

		ann->setColor( (*iter)->lineColor( ) );
		ann->setLineStyle( viewer_to_annotation((*iter)->lineStyle( )) );
		ann->setLineWidth( (*iter)->lineWidth( ) );

		ann->setFont( (*iter)->textFont( ) );
		ann->setFontSize( (*iter)->textFontSize( ) );
		int font_style = (*iter)->textFontStyle( );
		ann->setFontStyle( font_style | Region::ItalicText && font_style | Region::BoldText ? AnnotationBase::ITALIC_BOLD :
				   font_style | Region::ItalicText ? AnnotationBase::ITALIC : 
				   font_style | Region::BoldText ? AnnotationBase::BOLD : AnnotationBase::NORMAL );
		// if ( (*iter)->zMin( ) != 0 || (*iter)->zMax( ) < number_frames ) {
		//     ann->setFreqRange( (*iter)->zMin( ), (*iter)->zMax( ) );
		// }
		ann->print(out);
		delete ann;

	    }
	}

	void QtRegion::fetch_details( Region::RegionTypes &type, QList<int> &pixelx, QList<int> &pixely, QList<double> &worldx, QList<double> &worldy ) {

	    std::vector<std::pair<int,int> > pixel_pts;
	    std::vector<std::pair<double,double> > world_pts;

	    fetch_region_details(type, pixel_pts, world_pts);

	    for ( int i=0; i < pixel_pts.size(); ++i ) {
		pixelx.push_back(pixel_pts[i].first);
		pixely.push_back(pixel_pts[i].second);
	    }

	    for ( int i=0; i < world_pts.size(); ++i ) {
		worldx.push_back(world_pts[i].first);
		worldy.push_back(world_pts[i].second);
	    }

	}

	void QtRegion::signal_region_change( RegionChanges change ) {

	    if ( hold_signals > 0 ) {
		held_signals[change] = true;
		return;
	    }


	    switch ( change ) {
		case RegionChangeUpdate:
		case RegionChangeCreate:
		    {
			Region::RegionTypes type;
			QList<int> pixelx, pixely;
			QList<double> worldx, worldy;

			fetch_details( type, pixelx, pixely, worldx, worldy );

			if ( pixelx.size() == 0 || pixely.size() == 0 || worldx.size() == 0 || worldy.size() == 0 ) return;

			if ( change == RegionChangeCreate )
			    emit regionCreated( id_, QString( type == Region::RectRegion ? "rectangle" : type == Region::PointRegion ? "point" : 
							      type == Region::EllipseRegion ? "ellipse" : type == Region::PolyRegion ? "polygon" : "error"),
						QString::fromStdString(name( )), worldx, worldy, pixelx, pixely, QString::fromStdString(lineColor( )), QString::fromStdString(textValue( )),
						QString::fromStdString(textFont( )), textFontSize( ), textFontStyle( ) );
			else
			    emit regionUpdate( id_, worldx, worldy, pixelx, pixely );
		    }
		    break;
		case RegionChangeLabel:
		    fprintf( stderr, "====>> labelRegion( %d [id], %s [line color], %s [text], %s [font], %d [style], %d [size] )\n",
			     id_, lineColor( ).c_str( ), textValue( ).c_str( ), textFont( ).c_str( ), textFontStyle( ), textFontSize( ) );
		    break;
	    }
	}

	void QtRegion::releaseSignals( ) {

	    if ( --hold_signals > 0 ) return;
	    hold_signals = 0;

	    if ( held_signals[RegionChangeCreate] ) {
		signal_region_change(RegionChangeCreate);
	    } else {
		if ( held_signals[RegionChangeUpdate] ) {
		    signal_region_change(RegionChangeUpdate);
		}
		if ( held_signals[RegionChangeLabel] ) {
		    signal_region_change(RegionChangeLabel);
		}
	    }
	    clear_signal_cache( );
	}

	void QtRegion::clear_signal_cache( ) {
	    held_signals[RegionChangeCreate] = false;
	    held_signals[RegionChangeUpdate] = false;
	    held_signals[RegionChangeLabel] = false;
	}

    }
}
