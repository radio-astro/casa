//# Copyright (C) 2013
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
#include <casadbus/session/DBusSession.h>
#include <casadbus/synthesis/ImagerControl.h>
#include <casadbus/utilities/io.h>
#include <display/Clean/CleanGui.qo.h>
#include <iostream>
#include <iterator>
#include <QDebug>

namespace casa {

	namespace viewer {

		#define IMAGER_NAME "SynthesisImager"
		static const char imager_address[] = "edu.nrao.casa." IMAGER_NAME;

		CleanGui::CleanGui(QWidget *parent) : QDialog(parent), ic(0), current_process_index(-1),
											  current_process_state(UNDEFINED) {
			setupUi(this);
			const QString WINDOW_TITLE( "Clean Tool");
			this->setWindowTitle( WINDOW_TITLE );

			QStringList label_list;
			label_list << "id";
			clean_processes->setColumnCount(1);
			clean_processes->setHeaderLabels(label_list);

			QTreeWidgetItem *current_item = 0;
			try {
				casa::DBusSession &session = casa::DBusSession::instance( );
				typedef std::vector<std::string> namelist_t;
				namelist_t names = session.listNames( );
				for ( namelist_t::iterator it = names.begin( ); it != names.end( ); ++it ) {
					if ( it->compare( 0, sizeof(imager_address)-1, imager_address ) == 0 ) {
						std::string value = *it;
						value.erase( 0, sizeof(imager_address)-1 );
						QTreeWidgetItem *item = new QTreeWidgetItem(clean_processes);
						if ( current_item == 0 ) current_item = item;
						item->setText( 0, QString::fromStdString(value) );
						item->setCheckState( 0, Qt::Unchecked );
					}
				}
			} catch( ... ) { }

			//****
			//**** setup signals here
			//****
			connect( clean_processes, SIGNAL(itemSelectionChanged()), SLOT(selection_change()) );
			connect( clean_processes, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(check_box_change(QTreeWidgetItem*,int)) );
			connect( stop_button, SIGNAL(pressed( )), SLOT(stop_button_event( )) );
			connect( play_button, SIGNAL(pressed( )), SLOT(play_button_event( )) );
			connect( pause_button, SIGNAL(pressed( )), SLOT(pause_button_event( )) );
			connect( refresh_button, SIGNAL(pressed( )), SLOT(refresh_button_event( )) );

			basic_frame->setEnabled(false);
			advanced_frame->setEnabled(false);
			if ( current_item != 0 ) {
				clean_processes->setCurrentItem( current_item, 0 );
			}

		}

		void CleanGui::play_button_event( ) {
			if ( ic == NULL ) selection_change( );
			if ( ic == NULL ) return;
			/****
			***** assume that a stopped process cannot be paused...
			****/
			if ( current_process_state == RUNNING ) return;
			for ( int i=0; i < 3 && current_process_state != RUNNING; ++i ) {
				if ( current_process_state == PAUSED )
					ic->changePauseFlag( false );
				if ( current_process_state == STOPPED )
					ic->changeStopFlag( false );
				refresh( );
			}
		}

		void CleanGui::pause_button_event( ) {
			if ( ic == NULL ) selection_change( );
			if ( ic == NULL ) return;
			/****
			***** assume that a stopped process cannot be paused...
			****/
			if ( current_process_state == PAUSED ||
				 current_process_state == STOPPED ) return;

			ic->changePauseFlag( true );
			refresh( );
		}

		void CleanGui::stop_button_event( ) {
			if ( ic == NULL ) selection_change( );
			if ( ic == NULL ) return;
			if ( current_process_state == STOPPED ) return;
			ic->changeStopFlag( true );
			refresh( );
		}

		void CleanGui::refresh_button_event( ) {
			if ( ic == NULL )
				selection_change( );
			else
				refresh( );
		}

		void CleanGui::check_box_change( QTreeWidgetItem *item, int index ) {
			if ( ic == NULL ) selection_change( );
			if ( ic == NULL ) return;
			if ( item->checkState(index) != Qt::Unchecked ) {
				ic->incrementController( );
				basic_frame->setEnabled(true);
				advanced_frame->setEnabled(true);
			} else {
				basic_frame->setEnabled(false);
				advanced_frame->setEnabled(false);
				ic->decrementController( );
			}
		}

		void CleanGui::selection_change( ) {
			current_process_state = RUNNING;
			/***********************************************************************
			****** Checking tree item's checkbox makes this viewer a controller ****
			***********************************************************************/
			basic_frame->setEnabled(false);
			advanced_frame->setEnabled(false);

			QList<QTreeWidgetItem *> lst = clean_processes->selectedItems( );
			if (!lst.empty()) {
				QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
				int index = clean_processes->indexOfTopLevelItem(item);
				if ( index < 0 || index == current_process_index ) return;
				current_process_index = -1;
				QString process_id = item->text(0);
				try {
					if ( ic != NULL ) {
						delete ic;
						ic = 0;
					}
					ic = new ImagerControl( IMAGER_NAME + process_id.toStdString( ) );
					current_process_index = index;
					if ( ic ) refresh( );
				} catch ( const AipsError &err ) {
					std::cerr << "loading clean info failed (at " << __FILE__ << ":" << __LINE__ <<  "): " <<
						err.getMesg( ) << std::endl;
				} catch ( ... ) {
					std::cerr << "loading clean info failed (at " << __FILE__ << ":" << __LINE__ <<  ")..." <<
						std::endl;
				}
			}
		}

		void CleanGui::refresh( ) {
			try {
				fflush(stderr);
				typedef std::map<std::string,dbus::variant> details_t;
				details_t details = ic->getDetails( );
#if DEBUG
				std::ostream_iterator<std::string> out( cerr, ", " );
				std::transform( details.begin( ), details.end( ), out,
								(std::string (*)(const std::pair<std::string,dbus::variant>&)) dbus::asString );
				fflush(stderr);  //*** catches cerr too...
#endif
				/***********************************************************************
				******  Is interaction currently enabled?                         ******
				***********************************************************************/
				details_t::iterator it = details.find("interactivemode");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::BOOL ) {
					if ( it->second.getBool( ) ) {
						interactive_label->setText("interactive");
					} else {
						interactive_label->setText("not interactive");
					}
				}
				/***********************************************************************
				******  Fill in the threshold...                                  ******
				***********************************************************************/
				it = details.find("threshold");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double threshold = it->second.getDouble( );
					threshold_entry->setText(QString::number(threshold));
				} else {
					threshold_entry->clear( );
				}
				/***********************************************************************
				******  Fill in the niter...                                      ******
				***********************************************************************/
				it = details.find("niter");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::INT ) {
					int niter = it->second.getInt( );
					niter_entry->setText(QString::number(niter));
				} else {
					niter_entry->clear( );
				}

				/***********************************************************************
				******  Fill in clean process description...                      ******
				***********************************************************************/
				std::string description = ic->getDescription( );
				process_description->setPlainText( QString::fromStdString(description) );

				/***********************************************************************
				******  Fill in cycle niter...                                    ******
				***********************************************************************/
				it = details.find("cycleniter");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::INT ) {
					int niter = it->second.getInt( );
					cycle_niter_entry->setText(QString::number(niter));
				} else {
					cycle_niter_entry->clear( );
				}

				/***********************************************************************
				******  Fill in cycle threshold...                                ******
				***********************************************************************/
				it = details.find("cyclethreshold");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double threshold = it->second.getDouble( );
					cycle_threshold_entry->setText(QString::number(threshold));
				} else {
					cycle_threshold_entry->clear( );
				}

				/***********************************************************************
				******  Fill in interactive niter...                              ******
				***********************************************************************/
				it = details.find("interactiveniter");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::INT ) {
					int niter = it->second.getInt( );
					interactive_niter_entry->setText(QString::number(niter));
				} else {
					interactive_niter_entry->clear( );
				}

				/***********************************************************************
				******  Fill in interactive threshold...                          ******
				***********************************************************************/
				it = details.find("interactivethreshold");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double threshold = it->second.getDouble( );
					interactive_threshold_entry->setText(QString::number(threshold));
				} else {
					interactive_threshold_entry->clear( );
				}

				/***********************************************************************
				******  Fill in cycle factor entry...                             ******
				***********************************************************************/
				it = details.find("cyclefactor");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double factor = it->second.getDouble( );
					cycle_factor_entry->setText(QString::number(factor));
				} else {
					cycle_factor_entry->clear( );
				}


				/***********************************************************************
				******  Fill in loop gain entry...                                ******
				***********************************************************************/
				it = details.find("loopgain");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double gain = it->second.getDouble( );
					loop_gain_entry->setText(QString::number(gain));
				} else {
					loop_gain_entry->clear( );
				}

				/***********************************************************************
				******  Fill in current clean state...                            ******
				***********************************************************************/
				it = details.find("cleanstate");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::STRING ) {
					std::string state = it->second.getString( );
					clean_state_label->setText(QString::fromStdString(state));
					if ( state == "paused" )
						current_process_state = PAUSED;
					else if ( state == "stopped" )
						current_process_state = STOPPED;
					else
						current_process_state = RUNNING;
				} else {
					clean_state_label->setText("error");
					current_process_state = UNDEFINED;
				}

			} catch ( ... ) {
				fprintf( stderr, "\t\toops...\n" );
				fflush( stderr );
			}

		}

		CleanGui::~CleanGui( ) {  }

	}
}
