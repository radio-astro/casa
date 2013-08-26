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
#include <display/Clean/ConfirmStop.ui.h>
#include <casadbus/utilities/io.h>
#include <display/Clean/CleanGui.qo.h>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <QDebug>
#define DEBUG 1
namespace casa {

	namespace viewer {

		CleanGui::CleanGui(QWidget *parent) : QDialog(parent), ic(0), current_process_index(-1),
			current_process_state(UNDEFINED) {
			setupUi(this);
			const QString WINDOW_TITLE( "Clean Tool");
			this->setWindowTitle( WINDOW_TITLE );
			default_send_tooltip = send_button->toolTip( );

			// these are freed when 'this' is freed...
			QDoubleValidator *double_validator = new QDoubleValidator(this);
			QIntValidator *int_validator = new QIntValidator(this);

			threshold_entry->setValidator(double_validator);
			niter_entry->setValidator(int_validator);
			cycle_niter_entry->setValidator(int_validator);
			cycle_threshold_entry->setValidator(double_validator);
			interactive_niter_entry->setValidator(int_validator);
			interactive_threshold_entry->setValidator(double_validator);
			cycle_factor_entry->setValidator(double_validator);
			loop_gain_entry->setValidator(double_validator);

			QStringList label_list;
			label_list << "id";
			clean_processes->setColumnCount(1);
			clean_processes->setHeaderLabels(label_list);

			QTreeWidgetItem *current_item = 0;
			try {
				casa::DBusSession &session = casa::DBusSession::instance( );
				typedef std::vector<std::string> namelist_t;
				namelist_t names = session.listNames( );
#if DEBUG
				fprintf( stderr, "<CleanGui::CleanGui>: finding existing clean services...\n" );
				fflush(stderr);
#endif
				for ( namelist_t::iterator it = names.begin( ); it != names.end( ); ++it ) {
#if DEBUG
					cout << std::setw(25) << "<CleanGui::CleanGui>:" << std::setw(50) << it->c_str( ) << 
						std::setw(50) << ImagerControl::interface_name( );
#endif
					if ( it->compare( 0, ImagerControl::interface_name( ).size( ), ImagerControl::interface_name( ) ) == 0 ) {
#if DEBUG
						cout << std::setw(7) << "(yes)" << endl;
#endif
						std::string value = *it;
						value.erase( 0, ImagerControl::interface_name( ).size( ) );
						QTreeWidgetItem *item = new QTreeWidgetItem(clean_processes);
						if ( current_item == 0 ) current_item = item;
						item->setText( 0, QString::fromStdString(value) );
						item->setCheckState( 0, Qt::Unchecked );
					}
#if DEBUG
					else { cout << std::setw(7) << "(no)" << endl; }
#endif
				}
			} catch( ... ) { }

			//****
			//**** setup signals here
			//****
			connect( send_button, SIGNAL(pressed( )), SLOT(send_state_event( )) );

			connect( clean_processes, SIGNAL(itemSelectionChanged()), SLOT(selection_change()) );
			connect( clean_processes, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(check_box_change(QTreeWidgetItem*,int)) );
			connect( stop_button, SIGNAL(pressed( )), SLOT(stop_button_event( )) );
			connect( play_button, SIGNAL(pressed( )), SLOT(play_button_event( )) );
			// connect( pause_button, SIGNAL(pressed( )), SLOT(pause_button_event( )) );
			connect( refresh_button, SIGNAL(pressed( )), SLOT(refresh_button_event( )) );

			connect( niter_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( threshold_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( cycle_niter_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( interactive_niter_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( cycle_threshold_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( interactive_threshold_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( cycle_factor_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );
			connect( loop_gain_entry, SIGNAL(textEdited(const QString&)), SLOT(entry_changed_event(const QString&)) );

			allow_editing(false);

			if ( current_item != 0 ) {
				clean_processes->setCurrentItem( current_item, 0 );
			}

		}

		void CleanGui::send_state_event( ) {
			if ( ic == NULL ) return;
			ic->controlUpdate(collect( ));
			set_send_needed(false);
			refresh( );
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
			Ui::ConfirmStop setup;
			QDialog *popup = new QDialog(this);
			setup.setupUi(popup);
			int result = popup->exec( );
			delete popup;
			if ( result == 0 ) return;
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
				allow_editing(true);
			} else {
				allow_editing(false);
				refresh( );
				ic->decrementController( );
			}
		}

		void CleanGui::set_send_needed( bool needed ) {
			if ( needed ) {
				send_button->setStyleSheet("color: red");
				send_button->setToolTip("values changed, update needed...");
			} else {
				send_button->setStyleSheet("color: black");
				send_button->setToolTip(default_send_tooltip);
			}
		}

		void CleanGui::allow_editing( bool ok ) {
			if ( ok ) {
				basic_frame->setEnabled(true);
				advanced_frame->setEnabled(true);
				send_button->setEnabled(true);
			} else {
				basic_frame->setEnabled(false);
				advanced_frame->setEnabled(false);
				send_button->setEnabled(false);
			}
		}

		void CleanGui::entry_changed_event( const QString &str ) {
			std::map<QObject*,QString>::const_iterator it = current_clean_state.find(QObject::sender( ));
			if ( it != current_clean_state.end( ) ) {
				if ( str != it->second ) {
					set_send_needed(true);
					return;
				}
			}

			// here we know that the current entry has been returned to its "stored" value...
			// or this is an event from some entry not stored in our map... either way, we
			// will traverse and check all the values...
			for ( std::map<QObject*,QString>::const_iterator it = current_clean_state.begin( );
			        it != current_clean_state.end( ); ++it ) {
				QLineEdit *ptr = dynamic_cast<QLineEdit*>(it->first);
				if ( it->second != ptr->text( ) ) {
					set_send_needed(true);
					return;
				}
			}
			set_send_needed(false);
		}

		void CleanGui::selection_change( ) {
			current_process_state = RUNNING;
			/***********************************************************************
			****** Checking tree item's checkbox makes this viewer a controller ****
			***********************************************************************/
			allow_editing(false);

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
#ifdef DEBUG
					cout << "<CleanGui::selection_change>: creating imager control object:\t" << ImagerControl::name( ) + process_id.toStdString( ) << endl;
#endif
					ic = new ImagerControl( "edu.nrao.casa." + ImagerControl::name( ) + process_id.toStdString( ) );
					current_process_index = index;

					if ( ic ) {
						refresh( );
						if ( item->checkState(0) != Qt::Unchecked )
							allow_editing(true);
					}
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
#if DEBUG
				fprintf( stderr, "<CleanGui::refresh>: getting details...\n" );
#endif
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
					QString val = QString::number(threshold);
					threshold_entry->setText(val);
					current_clean_state[threshold_entry] = val;
				} else {
					threshold_entry->clear( );
					current_clean_state[threshold_entry] = "";
				}
				/***********************************************************************
				******  Fill in the niter...                                      ******
				***********************************************************************/
				it = details.find("niter");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::INT ) {
					int niter = it->second.getInt( );
					QString val = QString::number(niter);
					niter_entry->setText(val);
					current_clean_state[niter_entry] = val;
				} else {
					niter_entry->clear( );
					current_clean_state[niter_entry] = "";
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
					QString val = QString::number(niter);
					cycle_niter_entry->setText(val);
					current_clean_state[cycle_niter_entry] = val;
				} else {
					cycle_niter_entry->clear( );
					current_clean_state[cycle_niter_entry] = "";
				}

				/***********************************************************************
				******  Fill in cycle threshold...                                ******
				***********************************************************************/
				it = details.find("cyclethreshold");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double threshold = it->second.getDouble( );
					QString val = QString::number(threshold);
					cycle_threshold_entry->setText(val);
					current_clean_state[cycle_threshold_entry] = val;
				} else {
					cycle_threshold_entry->clear( );
					current_clean_state[cycle_threshold_entry] = "";
				}

				/***********************************************************************
				******  Fill in interactive niter...                              ******
				***********************************************************************/
				it = details.find("interactiveniter");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::INT ) {
					int niter = it->second.getInt( );
					QString val = QString::number(niter);
					interactive_niter_entry->setText(val);
					current_clean_state[interactive_niter_entry] = val;
				} else {
					interactive_niter_entry->clear( );
					current_clean_state[interactive_niter_entry] = "";
				}

				/***********************************************************************
				******  Fill in interactive threshold...                          ******
				***********************************************************************/
				it = details.find("interactivethreshold");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double threshold = it->second.getDouble( );
					QString val = QString::number(threshold);
					interactive_threshold_entry->setText(val);
					current_clean_state[interactive_threshold_entry] = val;
				} else {
					interactive_threshold_entry->clear( );
					current_clean_state[interactive_threshold_entry] = "";
				}

				/***********************************************************************
				******  Fill in cycle factor entry...                             ******
				***********************************************************************/
				it = details.find("cyclefactor");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double factor = it->second.getDouble( );
					QString val = QString::number(factor);
					cycle_factor_entry->setText(val);
					current_clean_state[cycle_factor_entry] = val;
				} else {
					cycle_factor_entry->clear( );
					current_clean_state[cycle_factor_entry] = "";
				}


				/***********************************************************************
				******  Fill in loop gain entry...                                ******
				***********************************************************************/
				it = details.find("loopgain");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::DOUBLE ) {
					double gain = it->second.getDouble( );
					QString val = QString::number(gain);
					loop_gain_entry->setText(val);
					current_clean_state[loop_gain_entry] = val;
				} else {
					loop_gain_entry->clear( );
					current_clean_state[loop_gain_entry] = "";
				}

				/***********************************************************************
				******  Fill in current clean state...                            ******
				***********************************************************************/
				it = details.find("cleanstate");
				if ( it != details.end( ) && it->second.type( ) == dbus::variant::STRING ) {
					std::string state = it->second.getString( );
					clean_state_label->setText(QString::fromStdString(state));
					if ( state == "paused" ) {
						current_process_state = PAUSED;
						play_button->setIcon(QIcon(":/icons/Anim4_Play.png"));
					} else if ( state == "stopped" ) {
						current_process_state = STOPPED;
						play_button->setIcon(QIcon(":/icons/Anim4_Play.png"));
						play_button->setEnabled(false);
					} else {
						play_button->setIcon(QIcon(":/icons/Anim7_Pause.png"));
						current_process_state = RUNNING;
					}
				} else {
					clean_state_label->setText("error");
					current_process_state = UNDEFINED;
				}

				set_send_needed(false);

			} catch ( std::exception e ) {
				fprintf( stderr, "error CleanGui::refresh( ): %s\n", e.what( ) );
				fflush( stderr );
			} catch ( ... ) {
				fprintf( stderr, "unexpected error in CleanGui::refresh( )\n" );
				fflush( stderr );
			}

		}

		std::map<std::string,dbus::variant> CleanGui::collect( ) {
			typedef std::map<std::string,dbus::variant> details_t;
			details_t result;
#if DEBUG
			cerr << "CleanGui::collect( )\n" << endl;
#endif
			try {
				/***********************************************************************
				******  Is interaction currently enabled?                         ******
				***********************************************************************/
				// interactive_label->text( );

				/***********************************************************************
				******  Fill in the threshold...                                  ******
				***********************************************************************/
				result.insert(details_t::value_type( "threshold",
				                                     threshold_entry->text( ).toDouble( ) ));

				/***********************************************************************
				******  Fill in the niter...                                      ******
				***********************************************************************/
				result.insert(details_t::value_type( "niter",
				                                     niter_entry->text( ).toInt( ) ));

				/***********************************************************************
				******  Fill in cycle niter...                                    ******
				***********************************************************************/
				result.insert(details_t::value_type( "cycleniter",
				                                     cycle_niter_entry->text( ).toInt( ) ));

				/***********************************************************************
				******  Fill in cycle threshold...                                ******
				***********************************************************************/
				result.insert(details_t::value_type( "cyclethreshold",
				                                     cycle_threshold_entry->text( ).toDouble( ) ));

				/***********************************************************************
				******  Fill in interactive niter...                              ******
				***********************************************************************/
				result.insert(details_t::value_type( "interactiveniter",
				                                     interactive_niter_entry->text( ).toInt( ) ));

				/***********************************************************************
				******  Fill in interactive threshold...                          ******
				***********************************************************************/
				result.insert(details_t::value_type( "interactivethreshold",
				                                     interactive_threshold_entry->text( ).toDouble( ) ));

				/***********************************************************************
				******  Fill in cycle factor entry...                             ******
				***********************************************************************/
				result.insert(details_t::value_type( "cyclefactor",
				                                     cycle_factor_entry->text( ).toDouble( ) ));

				/***********************************************************************
				******  Fill in loop gain entry...                                ******
				***********************************************************************/
				result.insert(details_t::value_type( "loopgain",
				                                     loop_gain_entry->text( ).toDouble( ) ));

				/***********************************************************************
				******  Fill in current clean state...                            ******
				***********************************************************************/
				//it = details.find("cleanstate");

			} catch ( std::exception e ) {
				fprintf( stderr, "error CleanGui::collect( ): %s\n", e.what( ) );
				fflush( stderr );
			} catch ( ... ) {
				fprintf( stderr, "unexpected error in CleanGui::collect( )\n" );
				fflush( stderr );
			}
			return result;
		}

		CleanGui::~CleanGui( ) {  }

	}
}
