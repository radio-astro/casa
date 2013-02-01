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

		CleanGui::CleanGui(QWidget *parent) : QDialog(parent) {
			setupUi(this);
			const QString WINDOW_TITLE( "Clean Tool");
			this->setWindowTitle( WINDOW_TITLE );

			QStringList label_list;
			label_list << "name";
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
					}
				}
			} catch( ... ) { }

			//****
			//**** setup signals here
			//****
			connect( clean_processes, SIGNAL(itemSelectionChanged()), SLOT(selection_change()) );

			if ( current_item != 0 )
				clean_processes->setCurrentItem( current_item, 0 );

		}

		void CleanGui::selection_change( ) {
			fprintf( stderr, "\t>>>>>>>>>> selection change!\n" );
			QList<QTreeWidgetItem *> lst = clean_processes->selectedItems( );
			if (!lst.empty()) {
				QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
				QString process_id = item->text(0);
				fprintf( stderr, "\t\tentering try block...\n" );
				fflush(stderr);
				try {
					fprintf( stderr, "\t\tcreating imager control...\n" );
					fflush(stderr);
					ImagerControl ic( IMAGER_NAME + process_id.toStdString( ) );
					fprintf( stderr, "\t\tdone... calling function...\n" );
					fflush(stderr);
					try {
						fprintf( stderr, "\t\there we go...\n" );
						fflush(stderr);
						std::map<std::string,dbus::variant> details = ic.getDetails( );
						fprintf( stderr, "\t\tsize: %lu\n", details.size() );
						fflush(stderr);
						std::ostream_iterator<std::string> out( cout, ", " );
						std::transform( details.begin( ), details.end( ), out,
										(std::string (*)(const std::pair<std::string,dbus::variant>&)) dbus::asString );
					} catch ( ... ) {
						fprintf( stderr, "\t\toops...\n" );
						fflush( stderr );
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

		CleanGui::~CleanGui( ) { }

	}
}
