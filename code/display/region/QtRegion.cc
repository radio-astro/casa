//# QtRegion.cc: GUI base class for all regions
//# Copyright (C) 2012
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
//# $Id: $

#include <casa/BasicSL/String.h>
#include <display/DisplayEvents/DTVisible.h>
#include <display/region/QtRegion.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <display/region/QtRegionDock.qo.h>
#include <imageanalysis/Annotations/AnnRegion.h>
#include <imageanalysis/Annotations/RegionTextList.h>
#include <display/DisplayErrors.h>
#include <casaqt/QtUtilities/QtId.h>
#include <QDir>

namespace casa {
    namespace viewer {

	QtRegion::QtRegion( QtRegionSourceKernel *factory ) :
			source_(factory), dock_(factory->dock( )), name_(""), hold_signals(0),
			z_index_within_range(true), id_(QtId::get_id( )) {
	    statistics_visible = position_visible = false;
	    statistics_update_needed = position_update_needed = true;

	    static DTVisible aipsrc;
	    color_ = QString::fromStdString(aipsrc.drawColor( ));

	    if ( dock_ == 0 )
		throw internal_error( "no dock widget is available" );

	    dock_->addRegion(this,mystate);
	}

      QtRegion::QtRegion( const QString &nme, QtRegionSourceKernel *factory, bool hold_signals_,
			  QtMouseToolNames::PointRegionSymbols sym ) :
			source_(factory), dock_(factory->dock()), name_(nme), hold_signals(hold_signals_ ? 1 : 0),
			z_index_within_range(true), id_(QtId::get_id( )) {
	    statistics_visible = position_visible = false;
	    statistics_update_needed = position_update_needed = true;

	    static DTVisible aipsrc;
	    color_ = QString::fromStdString(aipsrc.drawColor( ));

	    if ( dock_ == 0 )
		throw internal_error( "no dock widget is available" );

	    mystate = new QtRegionState( name_, this, sym );

	    connect( mystate, SIGNAL(regionChange(viewer::QtRegion*,std::string)), SIGNAL(regionChange(viewer::QtRegion*,std::string)) );

	    connect( mystate, SIGNAL(refreshCanvas( )), SLOT(refresh_canvas_event( )) );
	    connect( mystate, SIGNAL(statisticsVisible(bool)), SLOT(refresh_statistics_event(bool)) );
	    connect( mystate, SIGNAL(collectStatistics( )), SLOT(reload_statistics_event( )) );
	    connect( mystate, SIGNAL(positionVisible(bool)), SLOT(refresh_position_event(bool)) );

	    connect( mystate, SIGNAL(translateX(const QString &, const QString &, const QString &)), SLOT(translate_x(const QString&,const QString&, const QString &)) );
	    connect( mystate, SIGNAL(translateY(const QString &, const QString &, const QString &)), SLOT(translate_y(const QString&,const QString&, const QString &)) );
	    connect( mystate, SIGNAL(resizeX(const QString &, const QString &, const QString &)), SLOT(resize_x(const QString&,const QString&, const QString &)) );
	    connect( mystate, SIGNAL(resizeY(const QString &, const QString &, const QString &)), SLOT(resize_y(const QString&,const QString&, const QString &)) );

	    connect (mystate->getFitButton(), SIGNAL(clicked()), this, SLOT(updateCenterInfo()));

	    connect( mystate, SIGNAL(zRange(int,int)), SLOT(refresh_zrange_event(int,int)) );
	    connect( dock_, SIGNAL(deleteRegion(QtRegionState*)), SLOT(revoke_region(QtRegionState*)) );
	    connect( dock_, SIGNAL(deleteAllRegions( )), SLOT(revoke_region( )) );
	    connect( dock_, SIGNAL(saveRegions(std::list<QtRegionState*>, RegionTextList &)), SLOT(output(std::list<QtRegionState*>, RegionTextList &)) );
	    connect( dock_, SIGNAL(saveRegions(std::list<QtRegionState*>, ds9writer &)), SLOT(output(std::list<QtRegionState*>, ds9writer &)) );

	    dock_->addRegion(this,mystate);
	    signal_region_change( Region::RegionChangeCreate );
	}

	QtRegion::~QtRegion( ) {
	    dock_->removeRegion(mystate);
	    disconnect(mystate, 0, 0, 0);
	}

	void QtRegion::mark(bool set) {
	    if ( set != mystate->marked( ) ) {
		mystate->mark(set);
		emit selectionChanged(this,set);
	    }
	}

		void QtRegion::status( const std::string &msg, const std::string &type ) { dock_->status(msg,type); }

	bool QtRegion::mark_toggle( ) {
	    bool result = mystate->mark_toggle( );
	    emit selectionChanged(this,result);
	    return result;
	}

	void QtRegion::setLabel( const std::string &l ) {  mystate->setTextValue(l); }
	void QtRegion::setLabelPosition( Region::TextPosition pos ) { mystate->setTextPosition( pos ); }
	void QtRegion::setLabelDelta( const std::vector<int> &delta ) { mystate->setTextDelta( delta ); }

	void QtRegion::setFont( const std::string &font, int font_size, int font_style, const std::string &font_color ) {
	    if ( font != "" ) mystate->setTextFont(font);
	    if ( font_size >= 0 ) mystate->setTextFontSize(font_size);
	    mystate->setTextFontStyle( font_style );
	    if ( font_color != "" ) mystate->setTextColor( font_color );
	}

	void QtRegion::setLine( const std::string &line_color, Region::LineStyle line_style, unsigned int line_width ) {
	    if ( line_color != "" ) mystate->setLineColor( line_color );
	    mystate->setLineStyle( line_style );
	    mystate->setLineWidth( line_width );
	}

	void QtRegion::setAnnotation(bool ann) { mystate->setAnnotation(ann); }

	int QtRegion::numFrames( ) const { return source_->numFrames( ); }

	void QtRegion::zRange( int &min, int &max ) const {
	    min = mystate->zMin( );
	    max = mystate->zMax( );
	}

	void QtRegion::selectedInCanvas( ) { dock_->selectRegion(mystate); }

	void QtRegion::emitUpdate( ) {
	    Region::RegionTypes type;
	    QList<int> pixelx, pixely;
	    QList<double> worldx, worldy;

	    fetch_details( type, pixelx, pixely, worldx, worldy );

	    emit regionUpdateResponse( id_, QString( type == Region::RectRegion ? "rectangle" : type == Region::PointRegion ? "point" :
						     type == Region::EllipseRegion ? "ellipse" : type == Region::PolyRegion ? "polygon" : "error"),
				       QString::fromStdString(name( )), worldx, worldy, pixelx, pixely, QString::fromStdString(lineColor( )), QString::fromStdString(textValue( )),
				       QString::fromStdString(textFont( )), textFontSize( ), textFontStyle( ) );
	}

	std::pair<int,int> &QtRegion::tabState( ) { return dock_->tabState( ); }
	std::map<std::string,int> &QtRegion::coordState( ) { return dock_->coordState( ); }
	int &QtRegion::colorIndex( ) { return dock_->colorIndex( ); }
	void QtRegion::selectedCountUpdateNeeded( ) { dock_->selectedCountUpdateNeeded( ); }

	QString QtRegion::getSaveDir( ) {
	    if ( dock_->saveDir( ).isNull( ) ) {
		if ( ! dock_->loadDir( ).isNull( ) )
		    dock_->saveDir( ) = dock_->loadDir( );
		else
		    dock_->saveDir( ) = QDir::currentPath();
	    }
	    return dock_->saveDir( );
	}
	void QtRegion::putSaveDir( QString dir ) {
	    dock_->saveDir( ) = dir;
	}
	QString QtRegion::getLoadDir( ) {
	    if ( dock_->loadDir( ).isNull( ) ) {
		if ( ! dock_->saveDir( ).isNull( ) )
		    dock_->loadDir( ) = dock_->saveDir( );
		else
		    dock_->loadDir( ) = QDir::currentPath();
	    }
	    return dock_->loadDir( );
	}
	void QtRegion::putLoadDir( QString dir ) {
	    dock_->loadDir( ) = dir;
	}

	void QtRegion::updateCenterInfo() {
		std::list<RegionInfo> *rc = generate_dds_centers( );
		mystate->updateCenters(rc);
		// set the background to standard color which is some kind of grey
		mystate->setCenterBackground(QString("#e8e8e8"));
	}

	void QtRegion::invalidateCenterInfo( ){
		// set the background to "darkgrey"
		mystate->setCenterBackground(QString("#a9a9a9"));
	}

	bool QtRegion::weaklySelected( ) const {
		return dock_->isWeaklySelectedRegion(this);
	}
	void QtRegion::weaklySelect( ) {
		dock_->addWeaklySelectedRegion(this);
		dock_->selectRegion(mystate);
	}
	void QtRegion::weaklyUnselect( ) {
		dock_->removeWeaklySelectedRegion(this);
		const Region::region_list_type &weak = dock_->weaklySelectedRegionSet( );
		if ( weak.size( ) > 0 ) {
			QtRegion *region = dynamic_cast<QtRegion*>(*weak.begin( ));
			if ( region ) dock_->selectRegion(region->state( ));
		} else {
			const Region::region_list_type &marked = dock_->selectedRegionSet( );
			if ( marked.size( ) > 0 ) {
				QtRegion *region = dynamic_cast<QtRegion*>(*marked.begin( ));
				if ( region ) dock_->selectRegion(region->state( ));
			} else {
				updateStateInfo( false, Region::RegionChangeFocus );
			}
		}
	}

        // indicates that region movement requires that the statistcs be updated...
	void QtRegion::updateStateInfo( bool region_modified, Region::RegionChanges change ) {

		signal_region_change( change );

		// update statistics, when needed...
		if ( statistics_visible == false ) {
			if ( region_modified ) statistics_update_needed = true;
		} else if ( (statistics_update_needed || region_modified ) && regionVisible( ) ) {
			reload_statistics_event( );
		}

		// update position, when needed...
		if ( position_visible == false ) {
			if ( region_modified ) position_update_needed = true;
		} else if ( (position_update_needed || region_modified) && regionVisible( ) ) {
			Region::Coord c;
			Region::Units xu,yu;
			std::string whu;
			std::string x, y, angle;
			double width, height;
			mystate->getCoordinatesAndUnits( c, xu, yu, whu );
			getPositionString( x, y, angle, width, height, c, xu, yu, whu );

			QString qwidth;
			QString qheight;
			if ( width < 0.001 && height < 0.001 ) {
				qwidth = QString("%1").arg(width,0,'g',5);
				qheight = QString("%1").arg(height,0,'g',5);
			} else {
				qwidth = QString("%1").arg(width);
				qheight = QString("%1").arg(height);
			}

			mystate->updatePosition( QString::fromStdString(x),
									 QString::fromStdString(y),
									 QString::fromStdString(angle),
									 qwidth, qheight );

		}

	}

	void QtRegion::clearStatistics( ) {
	    statistics_update_needed = true;
	    mystate->clearStatistics( );
	}

	void QtRegion::refresh_statistics_event( bool visible ) {
	    statistics_visible = visible;
	    if ( hold_signals ) {
		held_signals[Region::RegionChangeStatsUpdate] = true;
		return;
	    }
	    updateStateInfo( false, Region::RegionChangeFocus );
	}

	void QtRegion::reload_statistics_event( ) {
	    statistics_update_needed = false;
	    std::list<RegionInfo> *rl = generate_dds_statistics( );
	    // send statistics to region state object...
	    mystate->updateStatistics(rl);
	    delete rl;
	}

	void QtRegion::refresh_position_event( bool visible ) {
	    position_visible = visible;
	    updateStateInfo( false, Region::RegionChangeUpdate );
	}

	void QtRegion::translate_x( const QString &x, const QString &x_units, const QString &coordsys ) {
	    if ( translateX( x.toStdString( ), x_units.toStdString( ), coordsys.toStdString( ) ) ) {
		refresh( );
	    }
	}
	void QtRegion::translate_y( const QString &y, const QString &y_units, const QString &coordsys ) {
	    if ( translateY( y.toStdString( ), y_units.toStdString( ), coordsys.toStdString( ) ) ) {
		refresh( );
	    }
	}
	void QtRegion::resize_x( const QString &x, const QString &x_units, const QString &coordsys ) {
	    resizeX( x.toStdString( ), x_units.toStdString( ), coordsys.toStdString( ) );
	}
	void QtRegion::resize_y( const QString &y, const QString &y_units, const QString &coordsys ) {
	    resizeY( y.toStdString( ), y_units.toStdString( ), coordsys.toStdString( ) );
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

	void QtRegion::revoke_region( ) {
	    source_->revokeRegion(fetch_my_region( ));
	}

	void QtRegion::revoke_region( QtRegionState *redacted_state ) {
	    if ( redacted_state == mystate ) { revoke_region( ); }
	}

	static inline AnnotationBase::LineStyle viewer_to_annotation( Region::LineStyle ls ) {
	    return ls == Region::SolidLine ? AnnotationBase::SOLID : ls == Region::DotLine ? AnnotationBase::DOTTED : AnnotationBase::DASHED;
	}

	void QtRegion::output( std::list<QtRegionState*> ol, RegionTextList &regionlist ) {
	    std::list<QtRegionState*>::iterator iter = find( ol.begin(), ol.end( ), mystate );
	    if ( iter != ol.end( ) ) {

		AnnotationBase *ann = annotation( );

		if ( ann == 0 ) {
		    fprintf( stderr, "Failed to create region annotation...\n" );
		    return;
		}

		AnnRegion *reg = dynamic_cast<AnnRegion*>(ann);
		if ( reg ) reg->setAnnotationOnly((*iter)->isAnnotation( ));

		// int number_frames = (*iter)->numFrames( );
		ann->setLabel( (*iter)->textValue( ) );

		ann->setColor( (*iter)->lineColor( ) );
		ann->setLineStyle( viewer_to_annotation((*iter)->lineStyle( )) );
		ann->setLineWidth( (*iter)->lineWidth( ) );

		ann->setFont( (*iter)->textFont( ) );
		ann->setFontSize( (*iter)->textFontSize( ) );
		int font_style = (*iter)->textFontStyle( );

		switch ( textPosition( ) ) {
		case Region::BottomText: ann->setLabelPosition("bottom"); break;
		    case Region::LeftText: ann->setLabelPosition("left"); break;
		    case Region::RightText: ann->setLabelPosition("right"); break;
		    default: ann->setLabelPosition("top");
		}
		ann->setLabelColor(textColor( ));

		vector<int> delta(2);
		textPositionDelta( delta[0], delta[1] );
		if ( delta[0] != 0 || delta[1] != 0 ) {
		    ann->setLabelOffset(delta);
		}


		ann->setFontStyle( font_style & Region::ItalicText && font_style & Region::BoldText ? AnnotationBase::ITALIC_BOLD :
				   font_style & Region::ItalicText ? AnnotationBase::ITALIC :
				   font_style & Region::BoldText ? AnnotationBase::BOLD : AnnotationBase::NORMAL );
		// if ( (*iter)->zMin( ) != 0 || (*iter)->zMax( ) < number_frames ) {
		//     ann->setFreqRange( (*iter)->zMin( ), (*iter)->zMax( ) );
		// }

		regionlist.addLine(AsciiAnnotationFileLine(ann));

	    }
	}

	void QtRegion::output( std::list<QtRegionState*> ol, ds9writer &out ) {
	    std::list<QtRegionState*>::iterator iter = find( ol.begin(), ol.end( ), mystate );
	    if ( iter != ol.end( ) ) {
		output(out);
	    }
	}

	void QtRegion::fetch_details( Region::RegionTypes &type, QList<int> &pixelx, QList<int> &pixely, QList<double> &worldx, QList<double> &worldy ) {

	    std::vector<std::pair<int,int> > pixel_pts;
	    std::vector<std::pair<double,double> > world_pts;

	    fetch_region_details(type, pixel_pts, world_pts);

	    for ( unsigned int i=0; i < pixel_pts.size(); ++i ) {
		pixelx.push_back(pixel_pts[i].first);
		pixely.push_back(pixel_pts[i].second);
	    }

	    for ( unsigned int i=0; i < world_pts.size(); ++i ) {
		worldx.push_back(world_pts[i].first);
		worldy.push_back(world_pts[i].second);
	    }

	}

	const std::list<Region*> &QtRegion::get_selected_regions( ) {
		// std::list<QtRegion*> regions = dock_->regions( );
		// for ( std::list<QtRegion*>::iterator it=regions.begin( ); it != regions.end( ); ++it ) {
		// 	if ( (*it)->marked( ) ) {
		// 		Region *r = dynamic_cast<Region*>(*it);
		// 		if ( r ) result->push_back(r);
		// 	}
		// }
		// return result;
		return dock_->selectedRegions( );
	}

	size_t QtRegion::selected_region_count( ) { return dock_->selectedRegionCount( ); }
	size_t QtRegion::marked_region_count( ) { return dock_->markedRegionCount( ); }

	void QtRegion::signal_region_change( Region::RegionChanges change ) {

	    if ( hold_signals > 0 ) {
		held_signals[change] = true;
		return;
	    }


	    switch ( change ) {
		case Region::RegionChangeUpdate:
		case Region::RegionChangeCreate:
		case Region::RegionChangeReset:
		case Region::RegionChangeFocus:
		case Region::RegionChangeModified:
		case Region::RegionChangeNewChannel:
		    {
			Region::RegionTypes type;
			QList<int> pixelx, pixely;
			QList<double> worldx, worldy;

			fetch_details( type, pixelx, pixely, worldx, worldy );

			if ( pixelx.size() == 0 || pixely.size() == 0 || worldx.size() == 0 || worldy.size() == 0 ) return;

			if ( change == Region::RegionChangeCreate ) {
			    dock_->emitCreate( this );
			    emit regionCreated( id_, QString( type == Region::RectRegion ? "rectangle" : type == Region::PointRegion ? "point" :
							      type == Region::EllipseRegion ? "ellipse" : type == Region::PolyRegion ? "polygon" : "error"),
						QString::fromStdString(name( )), worldx, worldy, pixelx, pixely, QString::fromStdString(lineColor( )), QString::fromStdString(textValue( )),
						QString::fromStdString(textFont( )), textFontSize( ), textFontStyle( ) );
			} else
			    emit regionUpdate( id_, change, worldx, worldy, pixelx, pixely );
		    }
		    break;

		case Region::RegionChangeDelete:
		case Region::RegionChangeStatsUpdate:
		case Region::RegionChangeLabel:
		    // fprintf( stderr, "====>> labelRegion( %d [id], %s [line color], %s [text], %s [font], %d [style], %d [size] )\n",
		    // 	     id_, lineColor( ).c_str( ), textValue( ).c_str( ), textFont( ).c_str( ), textFontStyle( ), textFontSize( ) );
		    break;
	    }
	}

	void QtRegion::releaseSignals( ) {

	    if ( --hold_signals > 0 ) return;
	    hold_signals = 0;

	    if ( held_signals[Region::RegionChangeCreate] ) {
		signal_region_change(Region::RegionChangeCreate);
	    } else {
		if ( held_signals[Region::RegionChangeUpdate] ) {
		    signal_region_change(Region::RegionChangeUpdate);
		}
		if ( held_signals[Region::RegionChangeLabel] ) {
		    signal_region_change(Region::RegionChangeLabel);
		}
	    }

	    if ( held_signals[Region::RegionChangeStatsUpdate] )
		updateStateInfo( false, Region::RegionChangeCreate );

	    clear_signal_cache( );
	}

	void QtRegion::clear_signal_cache( ) {
	    held_signals[Region::RegionChangeCreate] = false;
	    held_signals[Region::RegionChangeUpdate] = false;
	    held_signals[Region::RegionChangeLabel] = false;
	}

    }
}
