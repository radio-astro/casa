//# QtRegionDock.cc: dockable Qt implementation of viewer region management
//# with surrounding Gui functionality
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
//# $Id: $


#include <fstream>
#include <iostream>
#include <algorithm>
#include <display/region/QtRegionDock.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/Region.qo.h>
#include <display/ds9/ds9writer.h>
#include <display/DisplayErrors.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>

extern int qInitResources_QtRegionDock();

namespace casa {
	namespace viewer {

		QtRegionDock::QtRegionDock( QtDisplayPanelGui *d, QWidget* parent ) :
			InActiveDock(parent), Ui::QtRegionDock( ),
			dpg(d), current_dd(0), current_tab_state(-1,-1),
			current_color_index(6 /*** magenta ***/ ), dismissed(false),
			mouse_in_dock(false) {
			setupUi(this);
			qInitResources_QtRegionDock();
			// there are two standard Qt, dismiss icons...
			// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
			// dismissRegion->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
			dismiss_region->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
			reset_regions->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

			// Qt Designer will not allow you to create an empty QStackedWidget (qt4.7)... all created
			// stacked widgets seem to have two elements... here we remove that elements (if they
			// exists) because we want the elements to appear as they are created by the user...
			while ( region_stack->count( ) > 0 ) {
				QWidget *c = region_stack->currentWidget( );
				region_stack->removeWidget(c);
				delete c;
			}

			connect( region_stack, SIGNAL(currentChanged(int)), SLOT(stack_changed(int)) );
			connect( region_stack, SIGNAL(widgetRemoved(int)), SLOT(stack_changed(int)) );

			connect( region_scroll, SIGNAL(valueChanged(int)), SLOT(change_stack(int)) );

			connect( dismiss_region, SIGNAL(clicked(bool)), SLOT(delete_current_region(bool)) );
			connect( reset_regions, SIGNAL(clicked(bool)), SLOT(delete_all_regions(bool)) );
			connect( this, SIGNAL(visibilityChanged(bool)), SLOT(handle_visibility(bool)) );
			connect( region_stack, SIGNAL(currentChanged(int)), SLOT(emit_region_stack_change(int)) );

		}



		QtRegionDock::~QtRegionDock() {
			// perhaps these should be deleted somewhere else?
			region_list_t dtor_list = region_list;

			// clear list to prevent side-effects...
			region_list.clear( );
			// disconnect events from regions...
			for ( region_list_t::iterator it = dtor_list.begin( ); it != dtor_list.end( ); ++it ) {
				disconnect( *it, 0, 0, 0 );
			}
			// delete regions...
			for ( region_list_t::iterator it = dtor_list.begin( ); it != dtor_list.end( ); ++it ) {
				delete *it;
			}
		}

		void QtRegionDock::updateStackOrder( int topIndex ){
			int regionStackCount = region_stack->count();
			if ( regionStackCount > 0 ){
				for ( int i = 0; i < regionStackCount; i++ ){
					QtRegionState* state = dynamic_cast<QtRegionState*>(region_stack->widget( i ));
					state->updateStackOrder( topIndex );
				}
			}
			else {
				QWidget* currentWidget= region_stack->currentWidget();
				if ( currentWidget != NULL ){
					QtRegionState* state = dynamic_cast<QtRegionState*>(currentWidget);
					state->updateStackOrder( topIndex );
				}
			}
		}

		// void QtRegionDock::showStats( const QString &stats ) { }

		void QtRegionDock::enterEvent( QEvent* ) {
			mouse_in_dock = true;
			if ( region_stack->count( ) > 0 ) {
				QtRegionState *current_selection = dynamic_cast<QtRegionState*>(region_stack->currentWidget( ));
				if ( current_selection ) {

					Region *region = current_selection->region( );
					if ( region ) {
						clearWeaklySelectedRegionSet( );
						addWeaklySelectedRegion(region);
					}
					// there seems to be a lag where due to processing time etc. the signal of the cursor
					// moving out of one or more regions is lost. This ensures that at least when the
					// cursor moves into the dock, all regions will be marked as clear of the mouse...
					for ( region_list_t::iterator it = region_list.begin( ); it != region_list.end( ); ++it ) {
						Region *region = dynamic_cast<Region*>(*it);
						if ( region ) {
							region->clearMouseInRegion( );
							region->refresh( );
						}
					}
				}
			}
		}

		void QtRegionDock::leaveEvent( QEvent* ) {
			mouse_in_dock = false;
			if ( region_stack->count( ) > 0 ) {
				QtRegionState *current_selection = dynamic_cast<QtRegionState*>(region_stack->currentWidget( ));
				if ( current_selection ) {
					// current_selection->emitRefresh( );
					Region *region = current_selection->region( );
					if ( region) {
						clearWeaklySelectedRegionSet( );
						region->refresh( );
					}
				}
			}
		}

		void QtRegionDock::emitCreate( Region *r ) {
			try {
				emit regionChange( r, "created" );
			} catch(...) {
				/*fprintf( stderr, "******\tregion selection errors - %s, %d \t******\n", __FILE__, __LINE__ );*/
			}
		}

		void QtRegionDock::addRegion(Region *r, QtRegionState *state, int index) {

			// book keeping for DataManager region access...
			region_list.push_back(r);
			region_map.insert(region_map_t::value_type(state,r));

			if ( index >= 0 ) {
				region_stack->insertWidget( index, state );
			} else {
				region_stack->addWidget(state);
			}
			int currentWidgetIndex = region_stack->indexOf( state );
			if ( currentWidgetIndex >= 0 ){
				region_stack->setCurrentWidget(state);
			}

			// emit regionChange( r, "created" );

			connect( r, SIGNAL(regionChange(viewer::Region*,std::string)), this, SIGNAL(regionChange(viewer::Region*,std::string)));
			connect( r, SIGNAL(showHistogramTool()), this, SIGNAL(showHistogramTool()));
			connect( state, SIGNAL(outputRegions(const QString &,const QString &,const QString&,const QString&)), SLOT(output_region_event(const QString &,const QString &,const QString&,const QString&)) );
			connect( state, SIGNAL(loadRegions(const QString &,const QString &)), SIGNAL(loadRegions(const QString &,const QString&)) );
			connect( this, SIGNAL(region_stack_change(QWidget*)), state, SLOT(stackChange(QWidget*)) );

			// not needed if the dock starts out as visible (or in user control)
			// if ( ! isVisible( ) && dismissed == false ) {
			// 	show( );
			// }
		}

		int QtRegionDock::indexOf(QtRegionState *state) const {
			return region_stack->indexOf(state);
		}

		void QtRegionDock::removeRegion(QtRegionState *state) {

			Region *gonner=0;
			// clean up book keeping for DataManager region access...
			region_map_t::iterator map_it = region_map.find(state);
			if ( map_it != region_map.end( ) ) {
				region_list_t::iterator list_it = std::find(region_list.begin( ), region_list.end( ), map_it->second);
				if ( list_it != region_list.end( ) ) {
					region_list.erase(list_it);
				}
				gonner = map_it->second;
				region_map.erase(map_it);
			}

			if ( region_stack->indexOf(state) != -1 ) {
				// disconnect signals from 'state' to this object...
				disconnect( state, 0, this, 0 );
			}
			region_stack->removeWidget(state);

			// At one point the equivalent revokeRegion(...), below, was performed
			// at this point, but since the unification of GUI and non-GUI regions
			// I don't think this is necessary. The regions can only be deleted via
			// the region dock or the QtDisplayPanel.

			if ( gonner ) emit regionChange( gonner, "deleted" );
		}

		void QtRegionDock::status( const std::string &msg, const std::string &type ) {
			dpg->status(msg,type);
		}

		void QtRegionDock::selectRegion(QtRegionState *state, bool scroll ) {
			if ( state != NULL ) {
				if ( scroll ){
					int stateIndex = region_stack->indexOf( state );
					if ( stateIndex >= 0 ){
						region_stack->setCurrentWidget(state);
					}
				}

				state->nowVisible( );
				Region* region = state->region();
				if ( region != NULL ) {
					try {
						emit regionSelected( region->getId());
					} catch(...) {
						/*fprintf( stderr, "******\tregion selection errors - %s, %d \t******\n", __FILE__, __LINE__ );*/
					}
				}
			}
		}

		void QtRegionDock::selectedCountUpdateNeeded( ) {
			marked_region_set_.clear( );
			region::region_list_type selected;
			for ( region_list_t::iterator it = region_list.begin( ); it != region_list.end( ); ++it ) {
				(*it)->statisticsUpdateNeeded( );
				Region *r = dynamic_cast<Region*>(*it);
				if ( r ) {
					if (  r->marked( ) ) {
						selected.insert(r);
						marked_region_set_.insert(r);
					} else if ( r->weaklySelected( ) ) {
						selected.insert(r);
					}
				}
			}

			if ( selected != selected_region_set_ ) {
				selected_region_set_ = selected;
				QtRegionState *current_state = dynamic_cast<QtRegionState*>(region_stack->currentWidget( ));
				if ( current_state && current_state->statisticsIsVisible( ) )
					update_region_statistics( );
			}

		}

		void QtRegionDock::clearWeaklySelectedRegionSet( ) {
			weakly_selected_region_set_.clear( );
			selectedCountUpdateNeeded( );
		}
		bool QtRegionDock::isWeaklySelectedRegion( const Region *region ) const {
			return weakly_selected_region_set_.find((Region*)region) == weakly_selected_region_set_.end( ) ? false : true;
		}
		void QtRegionDock::addWeaklySelectedRegion( Region *region ) {
			weakly_selected_region_set_.insert( region );
			// selectedCountUpdateNeeded( );
		}
		void QtRegionDock::removeWeaklySelectedRegion( Region *region ) {
			weakly_selected_region_set_.erase(region);
			// selectedCountUpdateNeeded( );
		}

		void QtRegionDock::dismiss( ) {
			hide( );
			dismissed = true;
		}

		// zero length string indicates OK!
		std::string QtRegionDock::outputRegions( std::list<viewer::QtRegionState*> regionstate_list, std::string file,
		        std::string format, std::string ds9_csys ) {
			if ( regionstate_list.size( ) > 0 ) {
				if ( format == "crtf" ) {
					AnnRegion::unitInit();
					RegionTextList annotation_list;
					try {
						emit saveRegions(regionstate_list,annotation_list);
					} catch (...) {
						return "encountered error";
					}
					ofstream sink;
					sink.open(file.c_str( ));
					annotation_list.print(sink);
					sink.close( );
				} else if ( format == "ds9" ) {
					ds9writer writer(file.c_str( ),ds9_csys.c_str( ));
					try {
						emit saveRegions(regionstate_list,writer);
					} catch (...) {
						return "encountered error";
					}
				} else {
					return "invalid format";
				}
				return "";
			} else {
				return "no regions to write out";
			}
		}

		void QtRegionDock::change_stack( int index ) {
			if ( mouse_in_dock == false ) return;
			int size = region_stack->count( );
			if ( index >= 0 && index <= size - 1 ) {
				region_stack->setCurrentIndex( index );
				QtRegionState *current_state = dynamic_cast<QtRegionState*>(region_stack->currentWidget( ));
				if ( current_state ) {
					Region *region = current_state->region( );
					if ( region ) {
						clearWeaklySelectedRegionSet( );
						addWeaklySelectedRegion(region);
						current_state->emitRefresh( );
						updateRegionStats( );
					}
				}
			}
		}

		void QtRegionDock::updateRegionState( QtDisplayData *dd ) {
			if ( dd == 0 && current_dd == 0 ) return;
			if ( current_dd != 0 && dd == 0 )
				region_stack->hide( );
			else if ( current_dd == 0 && dd != 0 )
				region_stack->show( );

			for ( int i = 0; i < region_stack->count( ); ++i ) {
				QWidget *widget = region_stack->widget( i );
				QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
				if ( state != 0 ) state->updateCoord( );
			}

			current_dd = dd;
		}

		void QtRegionDock::updateRegionStats( ) {
			QWidget *widget = region_stack->currentWidget( );
			QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
			if ( state != 0 ) {
				state->reloadStatistics( );
			}
		}

		void QtRegionDock::stack_changed( int index ) {

			static int last_index = -1;
			int size = region_stack->count( );
			region_scroll->setMaximum( size > 0 ? size - 1 : 0 );
			if ( size <= 0 ) {
				region_scroll->setEnabled(false);
				dismiss_region->setEnabled(false);
				reset_regions->setEnabled(false);
			} else if ( size == 1 ) {
				region_scroll->setEnabled(false);
				dismiss_region->setEnabled(true);
				reset_regions->setEnabled(true);
			} else {
				region_scroll->setEnabled(true);
				dismiss_region->setEnabled(true);
				reset_regions->setEnabled(true);
			}
			QWidget *current_widget = region_stack->currentWidget( );
			if ( current_widget == 0 ) {
				// box is empty... e.g. after being deleted while updating
				region_scroll->setEnabled(false);
				dismiss_region->setEnabled(false);
				last_index = 0;
				return;
			}

			int current_index = region_stack->currentIndex( );
			if ( current_index >= 0 )
				region_scroll->setValue(current_index);

			// QtRegionState *state = dynamic_cast<QtRegionState*>(current_widget);
			// if ( state == 0 )
			// throw internal_error("region state corruption");

			// state->justExposed( );

#if 0
			if ( Region::getWeakSelection( ) != 0 ) {
				// stack changes when new region is created... but we're only interested in
				// changes which happen due to user scrolling (in which case, the mouse has
				// entered the region dock and the weak selection has been set).
				Region::setWeakSelection(state);
				state->emitRefresh( );
			}
#endif
			last_index = index;
		}

		int QtRegionDock::numFrames( ) const {
			return dpg->numFrames( );
		}

		void QtRegionDock::updateFrameCount( int count ) {
			std::list<Region*> regionList = regions();
			for ( std::list<Region*>::iterator it = regionList.begin( );
					it != regionList.end( ); ++it ) {
				Region *rr = *it;
				if ( rr != NULL ){
					QtRegionState *state = rr->state( );
					if ( state ){
						state->updateFrameInformation( count );
					}
				}
			}
		}

		void QtRegionDock::deleteRegions( const region::region_list_type &rl ) {
			region::region_list_type listx(rl);
			for ( region::region_list_type::iterator it = listx.begin( ); it != listx.end( ); ++it ) {
				Region *rr = *it;
				if ( rr == 0 ) continue;
				Region *qr = dynamic_cast<Region*>(rr);
				if ( qr ) {
					QtRegionState *state = qr->state( );
					if ( state ) emit deleteRegion(state);
				}
			}
			weakly_selected_region_set_.clear( );
			selectedCountUpdateNeeded( );
		}

		void QtRegionDock::revokeRegion( Region *r ) {
			dpg->revokeRegion( r );
		}

		void QtRegionDock::delete_current_region(bool) {
			weakly_selected_region_set_.clear( );
			emit deleteRegion(dynamic_cast<QtRegionState*>(region_stack->currentWidget( )));
		}

		void QtRegionDock::delete_all_regions(bool) {
			weakly_selected_region_set_.clear( );
			emit deleteAllRegions( );
		}

		void QtRegionDock::output_region_event( const QString &what, const QString &where, const QString &type, const QString &csys ) {
			std::list<QtRegionState*> regionstate_list;
			if ( what == "current" ) {
				// current region, only...
				QWidget *current_widget = region_stack->currentWidget( );
				QtRegionState *current = dynamic_cast<QtRegionState*>(current_widget);
				if ( current != 0 )
					regionstate_list.push_back(current);
			} else if ( what == "marked" ) {
				// all marked regions...
				for ( int i = 0; i < region_stack->count( ); ++i ) {
					QWidget *widget = region_stack->widget( i );
					QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
					if ( state != 0 && state->marked( ) )
						regionstate_list.push_back(state);
				}
			} else {
				// ("all")... all regions...
				for ( int i = 0; i < region_stack->count( ); ++i ) {
					QWidget *widget = region_stack->widget( i );
					QtRegionState *state = dynamic_cast<QtRegionState*>(widget);
					if ( state != 0 )
						regionstate_list.push_back(state);
				}
			}

			if ( regionstate_list.size( ) > 0 ) {
				if ( type == "CASA region file" ) {
					AnnRegion::unitInit();
					RegionTextList annotation_list;
					try {
						emit saveRegions(regionstate_list,annotation_list);
					} catch (...) {
						return;
					}
					ofstream sink;
					sink.open(where.toAscii( ).constData( ));
					annotation_list.print(sink);
					sink.close( );
				} else if ( type == "DS9 region file" ) {
					ds9writer writer(where.toAscii( ).constData( ),csys.toAscii( ).constData( ));
					try {
						emit saveRegions(regionstate_list,writer);
					} catch (...) {
						return;
					}

				}
			} else {
				QWidget *current_widget = region_stack->currentWidget( );
				QtRegionState *current = dynamic_cast<QtRegionState*>(current_widget);
				if ( current != 0 )
					current->noOutputNotify( );
			}
		}

		void QtRegionDock::handle_visibility( bool visible ) {
			if ( visible && dismissed ) {
				dismissed = false;
				dpg->putrc( "visible.regions", "true" );
			}
		}

		void QtRegionDock::emit_region_stack_change( int index ) {
			emit region_stack_change(region_stack->widget(index));
		}

		void QtRegionDock::closeEvent ( QCloseEvent * event ) {
			dismissed = true;
			QDockWidget::closeEvent(event);
			dpg->putrc( "visible.regions", "false" );
		}

		void QtRegionDock::update_region_statistics( ) {
			QtRegionState *current_selection = dynamic_cast<QtRegionState*>(region_stack->currentWidget( ));
			if ( current_selection ) current_selection->updateStatistics( );
		}
	}
}
