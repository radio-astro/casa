//# QtRegionState.cc: the GUI state of Region/QtRegion objects
//# with surrounding Gui functionality
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

#include <fcntl.h>
#include <unistd.h>
#include <QDebug>
#include <QFileInfo>
#include <display/region/Region.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/region/QtRegionStats.qo.h>
#include <display/region/HistogramTab.qo.h>
#include <casadbus/types/nullptr.h>
#include <QFileDialog>
#include <display/DisplayErrors.h>

namespace casa {
	namespace viewer {

		static const char casa_ext[] = ".crtf";
		static const char ds9_ext[] = ".reg";
		static const char *default_ext = casa_ext;


		QtRegionState::freestat_list *QtRegionState::freestats = 0;
		QtRegionState::freestat_list *QtRegionState::freecenters = 0;

		void QtRegionState::init( ) {
			QString cat = categories->tabText(categories->currentIndex( ));
			if ( cat == STATISTICS_MODE( ) )
				emit statisticsVisible( true );
			else if ( cat == PROPERTIES_MODE( ) ) {
				QString state = states->tabText(categories->currentIndex( ));
				if ( state == "coordinates" )
					emit positionVisible( true );
			}
		}


		QtRegionState::QtRegionState( const QString &n,
		                              QtMouseToolNames::PointRegionSymbols sym, Region *r, QWidget *parent ) :
			QFrame(parent), LINE_COLOR_CHANGE("state: line color"),
			selected_statistics(-1), region_(r), setting_combo_box(0),
			pre_dd_change_statistics_count(-1) {
			setupUi(this);
#if defined(__APPLE__)
			QFont font( "Lucida Grande", 11 );
			QFont fontp( "Lucida Grande", 11 );
#else
			QFont font( "Sans Serif", 9 );
			QFont fontp( "Sans Serif", 8 );
#endif
			region_type->setFont(font);
			label->setFont(font);
			frame_min->setFont(font);
			frame_max->setFont(font);
			region_mark->setFont(fontp);
			region_annotation->setFont(fontp);
			histogramTab = NULL;

			if (  sym == QtMouseToolNames::SYM_UNKNOWN || sym == QtMouseToolNames::SYM_POINT_REGION_COUNT )
				marker_group->hide( );
			else {
				int current_item = -1;
				for ( int i=0; i < QtMouseToolNames::SYM_POINT_REGION_COUNT; ++i ) {
					marker->addItem(QIcon(QString::fromStdString(QtMouseToolNames::pointRegionSymbolIcon((QtMouseToolNames::PointRegionSymbols)i))),
					                QString(),QVariant(i));
					if ( (int) sym == i ) current_item = i;
				}
				if ( current_item < 0 ) {
					marker->setDisabled(true);
				} else {
					marker->setCurrentIndex(current_item);
				}
			}


			if ( region_ ) initRegionState( );

			//setLineWidth(0);
			setFrameShape(QFrame::NoFrame);

			coordinate_angle_box->hide( );

			if ( freestats == 0 )
				freestats = new freestat_list( );
			if ( freecenters == 0 )
				freecenters = new freestat_list( );

			text_position->setWrapping(true);

			region_type->setText(QApplication::translate("QtRegionState", n.toAscii( ).constData( ), 0, QApplication::UnicodeUTF8));

			csys_box->hide( );

			// deprecated...
			coordinates_apply->hide( );
			coordinates_reset->hide( );

			connect( center_x, SIGNAL(editingFinished( )), SLOT(translate_x( )) );
			connect( center_y, SIGNAL(editingFinished( )), SLOT(translate_y( )) );
			connect( bounding_width, SIGNAL(editingFinished( )), SLOT(resize_x( )) );
			connect( bounding_height, SIGNAL(editingFinished( )), SLOT(resize_y( )) );

			// update line characteristics...
			connect( line_color, SIGNAL(currentIndexChanged(int)), SLOT(color_state_change(int)) );
			connect( line_style, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
			connect( line_width, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
			connect( text_position, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
			connect( text_color, SIGNAL(currentIndexChanged(int)), SLOT(color_state_change(int)) );
			connect( font_name, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
			connect( font_size, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
			connect( x_off, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
			connect( y_off, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
			connect( font_italic, SIGNAL(clicked(bool)), SLOT(state_change(bool)) );
			connect( font_bold, SIGNAL(clicked(bool)), SLOT(state_change(bool)) );
			connect( region_mark, SIGNAL(stateChanged(int)), SLOT(state_change_region_mark(int)) );
			connect( marker_scale, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );

			connect( save_file_name_browse, SIGNAL(clicked(bool)), SLOT(save_browser(bool)) );
			connect( load_file_name_browse, SIGNAL(clicked(bool)), SLOT(load_browser(bool)) );

			connect( save_now, SIGNAL(clicked(bool)), SLOT(save_region(bool)) );
			connect( save_file_type, SIGNAL(currentIndexChanged(const QString&)), SLOT(update_default_file_extension(const QString&)) );
			connect( save_filename, SIGNAL(textChanged(const QString&)), SLOT(update_save_type(const QString &)) );

			connect( load_now, SIGNAL(clicked(bool)), SLOT(load_regions(bool)) );
			connect( load_filename, SIGNAL(textChanged(const QString&)), SLOT(update_load_type(const QString &)) );

			connect( frame_min, SIGNAL(valueChanged(int)), SLOT(frame_min_change(int)) );
			connect( frame_max, SIGNAL(valueChanged(int)), SLOT(frame_max_change(int)) );

			connect( text, SIGNAL(textChanged(const QString&)), SLOT(state_change(const QString&)) );

			connect( states, SIGNAL(currentChanged(int)), SLOT(states_change(int)) );
			connect( file_tab, SIGNAL(currentChanged(int)), SLOT(filetab_change(int)) );

			connect( coordinate_system, SIGNAL(currentIndexChanged(const QString &)), SLOT(coordsys_change(const QString &)) );
			connect( x_units, SIGNAL(currentIndexChanged(int)), SLOT(states_val_change(int)) );
			connect( y_units, SIGNAL(currentIndexChanged(int)), SLOT(states_val_change(int)) );
			connect( dim_units, SIGNAL(currentIndexChanged(int)), SLOT(states_val_change(int)) );

			last_line_color = line_color->currentText( );
			connect( line_color, SIGNAL(currentIndexChanged(const QString&)), SLOT(line_color_change(const QString&)) );

			connect( marker, SIGNAL(currentIndexChanged(int)), SLOT(set_point_region_marker(int)) );


		}

		void QtRegionState::updateStackOrder( int firstImage ){
			statistics_group->setCurrentIndex( firstImage );

			if ( histogramTab != NULL ){
				histogramTab->showGraph( firstImage );
			}
		}

		int QtRegionState::getStackIndex() const {
			return statistics_group->currentIndex();
		}

		void QtRegionState::initRegionState( ) {
			int current_color_index = region_->colorIndex( );
			line_color->setCurrentIndex(current_color_index);
			text_color->setCurrentIndex(current_color_index);

			// use common tab state from dock...
			std::pair<int,int> &tab_state = region_->tabState( );
			if ( tab_state.first < 0 ) {
				QList<QTabWidget*> tabs = categories->currentWidget( )->findChildren<QTabWidget*>( );
				tab_state.first = categories->currentIndex( );
				if ( tabs.size( ) > 0 )
					tab_state.second = tabs.first( )->currentIndex( );
				else
					tab_state.second = -1;
			} else {
				categories->setCurrentIndex(tab_state.first);
				if ( tab_state.second >= 0 ) {
					QList<QTabWidget*> tabs = categories->currentWidget( )->findChildren<QTabWidget*>( );
					if ( tabs.size( ) > 0 )
						tabs.first( )->setCurrentIndex(tab_state.second);
				}
			}

			// use common coordinate state from dock...
			std::map<std::string,int> &coordinate_state = region_->coordState( );
			QList<QComboBox*> combos = coordinateTab->findChildren<QComboBox*>( );
			if ( coordinate_state.size( ) == 0 ) {
				for (int i = 0; i < combos.size(); ++i) {
					const QComboBox *combo = combos.at(i);
					coordinate_state.insert( std::map<std::string,int>::value_type(combo->objectName( ).toStdString( ), combo->currentIndex( ) ) );
				}
			} else {
				for (int i = 0; i < combos.size(); ++i) {
					QComboBox *combo = combos[i];
					std::map<std::string,int>::iterator it = coordinate_state.find(combo->objectName( ).toStdString( ));
					if ( it == coordinate_state.end( ) ) continue;
					combo->setCurrentIndex(it->second);
				}
			}

			int z_max = region_->numFrames( );
			updateFrameInformation(z_max);

			connect( categories, SIGNAL(currentChanged(int)), SLOT(category_change(int)) );
		}

		void QtRegionState::updateFrameInformation( int count ){
			frame_min->setMaximum( count );
			frame_max->setMaximum( count );
			frame_max->setValue( count );
		}

		QtRegionState::~QtRegionState( ) {
			if ( region_ ) region_->selectedCountUpdateNeeded( );
		}

		void QtRegionState::reset( const QString &n, Region *r ) {
			region_type->setText(QApplication::translate("QtRegionState", n.toAscii( ).constData( ), 0, QApplication::UnicodeUTF8));
			region_ = r;

			int z_max = region_->numFrames( );
			frame_min->setMaximum(z_max);
			frame_max->setMaximum(z_max);
			frame_max->setValue(z_max);
		}

		void QtRegionState::addHistogram(QWidget* histogram ) {
			categories->addTab( histogram, HISTOGRAM_MODE( ));
			histogramTab = dynamic_cast<HistogramTab*>(histogram);
		}

		void QtRegionState::setRegion( Region *r ) {
			region_ = r;
			initRegionState( );
		}

		void QtRegionState::updateCoord( ) {
			emit positionVisible(true);
		}

		void QtRegionState::updateStatistics(  ) {
			updateStatistics( region( )->statistics( ) );
		}

		void QtRegionState::updateStatistics( std::list<std::tr1::shared_ptr<RegionInfo> > *stats ) {
			if ( stats == 0 || stats->size() == 0 ) {
				if ( pre_dd_change_statistics_count != -1 ) clearStatistics( );
				pre_dd_change_statistics_count = -1;
				return;
			}
			int currentIndex = statistics_group->currentIndex();
			if ( currentIndex < 0 ){
				currentIndex = 0;
			}
			clearStatistics();

			while ( (int)stats->size() > statistics_group->count() ) {
				QtRegionStats *mystat;
				// BEGIN - critical section
				if ( freestats->size() > 0 ) {
					mystat = freestats->back( );
					freestats->pop_back( );
					// END - critical section
					mystat->reset( );
				} else {
					mystat = new QtRegionStats( );
				}
				statistics_group->insertWidget(statistics_group->count( ),mystat);
			}

			int num = statistics_group->count( );
			int firstIndex = 0;
			QtRegionStats *first = dynamic_cast<QtRegionStats*>(statistics_group->widget(firstIndex));
			if ( first == 0 ) throw internal_error( );
			std::list<std::tr1::shared_ptr<RegionInfo> >::iterator stat_iter = stats->begin();
			if ( ! memory::anullptr.check((*stat_iter)->list( )) ) {
				if ( first->updateStatisticsInfo( *stat_iter ) == false ){
					statisticsUpdate( first, *stat_iter );
				}
			}

			if ( num < 2 ) {
				first->disableNextButton( );
				return;
			}

			QtRegionStats *prev = first;

			for ( int i=1; i < statistics_group->count() && ++stat_iter != stats->end(); ++i ) {
				QtRegionStats *cur = dynamic_cast<QtRegionStats*>(statistics_group->widget(i));
				if ( cur == 0 ) throw internal_error( );
				if ( ! memory::anullptr.check((*stat_iter)->list( )) ) {
					if ( cur->updateStatisticsInfo( *stat_iter ) == false ){
						statisticsUpdate( cur, *stat_iter );
					}
				}
				prev->setNext( statistics_group, cur );
				prev = cur;
			}
			prev->setNext( statistics_group, first );

			int statistics_count = statistics_group->count( );
			if ( pre_dd_change_statistics_count < statistics_count && statistics_count > 0 ){
				if ( currentIndex >= 0 && currentIndex < statistics_count){
					statistics_group->setCurrentIndex( currentIndex );
				}
				else {
					statistics_group->setCurrentIndex( 0 );
				}
			}

			pre_dd_change_statistics_count = -1;
		}

		void QtRegionState::statisticsUpdate( QtRegionStats *regionStats, std::tr1::shared_ptr<casa::viewer::RegionInfo> regionInfo ) {
			if ( region_->type( ) != region::PolylineRegion &&
			        region_->type( ) != region::PVLineRegion ) {
				regionStats->updateStatistics( regionInfo );
			}
			else {
				regionStats->updateStatistics( regionInfo, region_ );
			}
		}

		void QtRegionState::reloadStatistics( ) {
			QString cat = categories->tabText(categories->currentIndex( ));
			if ( cat == STATISTICS_MODE( ) ) {
				pre_dd_change_statistics_count = statistics_group->count( );
				emit collectStatistics( );
			}
			if ( cat == HISTOGRAM_MODE() ) {
				emit updateHistogram();
			}
		}

		void QtRegionState::updateCenters( std::list<std::tr1::shared_ptr<RegionInfo> > *centers ) {
			// check if something can be done at all
			if ( centers == 0 || centers->size() == 0 ) return;

			while ((int)centers->size() < centers_group->count() ) {
				QtRegionStats *w = dynamic_cast<QtRegionStats*>(centers_group->widget(0));
				if ( w == 0 ) throw internal_error( );
				centers_group->removeWidget(w);
				freecenters->push_back(w);
			}
			while ((int)centers->size() > centers_group->count() ) {
				QtRegionStats *mycenter;
				// BEGIN - critical section
				if ( freecenters->size() > 0 ) {
					mycenter = freecenters->back( );
					freecenters->pop_back( );
					// END - critical section
					mycenter->reset( );
				} else {
					mycenter = new QtRegionStats( );
				}
				centers_group->insertWidget(centers_group->count( ),mycenter);
			}
			int num = centers_group->count( );
			QtRegionStats *first = dynamic_cast<QtRegionStats*>(centers_group->widget(0));
			if ( first == 0 ) throw internal_error( );
			std::list<std::tr1::shared_ptr<RegionInfo> >::iterator center_iter = centers->begin();
			if ( ! memory::anullptr.check((*center_iter)->list( )) ) {
				if ( first->updateStatisticsInfo( *center_iter ) == false )
					statisticsUpdate( first, *center_iter );
			}
			if ( num < 2 ) return;

			QtRegionStats *prev = first;
			for ( int i=1; i < centers_group->count() && ++center_iter != centers->end(); ++i ) {
				QtRegionStats *cur = dynamic_cast<QtRegionStats*>(centers_group->widget(i));
				if ( cur == 0 ) throw internal_error( );
				if ( ! memory::anullptr.check((*center_iter)->list( )) ) {
					if ( first->updateStatisticsInfo( *center_iter ) == false )
						statisticsUpdate(cur, *center_iter );
				}
				prev->setNext( centers_group, cur );
				prev = cur;
			}
			prev->setNext( centers_group, first );
		}

		void QtRegionState::setCenterBackground(QString background) {
			for ( int i=0; i < centers_group->count(); ++i ) {
				QtRegionStats *cur = dynamic_cast<QtRegionStats*>(centers_group->widget(i));
				if ( cur == 0 ) {
					throw internal_error( );
				}
				cur->setCenterBackground(background);
			}
			centers_group->update();
		}

		void QtRegionState::clearStatistics( ) {
			while ( statistics_group->count() > 0 ) {
				QtRegionStats *w = dynamic_cast<QtRegionStats*>(statistics_group->widget(0));
				if ( w == 0 ) throw internal_error( );
				statistics_group->removeWidget(w);
				freestats->push_back(w);
			}
		}

		std::string QtRegionState::lineColor( ) const {
			QString lc = line_color->currentText( );
			return lc.toStdString( );
		}

		std::string QtRegionState::centerColor( ) const {
			QString cc = center_color->currentText( );
			return cc.toStdString( );
		}

		region::LineStyle QtRegionState::lineStyle( ) const {
			QString ls = line_style->currentText( );
			if ( ls == "dashed" ) return region::DashLine;
			else if ( ls == "dotted" ) return region::DotLine;
			else return region::SolidLine;
		}

		void QtRegionState::setMarkerScale( int v ) {
			if ( v <= 0 ) marker_scale->setValue(0);
			else if ( v >= 9 ) marker_scale->setValue(9);
			else marker_scale->setValue(v);
		}

		std::string QtRegionState::textColor( ) const {
			QString tc = text_color->currentText( );
			return tc.toStdString( );
		}

		std::string QtRegionState::textFont( ) const {
			QString tfn = font_name->currentText( );
			return tfn.toStdString( );
		}

		int QtRegionState::textFontStyle( ) const {
			int result = 0;
			if ( font_italic->isChecked( ) ) result = result | region::ItalicText;
			if ( font_bold->isChecked( ) ) result = result | region::BoldText;
			return result;
		}

		std::string QtRegionState::textValue( ) const {
			QString txt = text->text( );
			return txt.toStdString( );
		}

		region::TextPosition QtRegionState::textPosition( ) const {
			int pos = text_position->value( );
			switch ( pos ) {
			case 1:
				return region::LeftText;
			case 2:
				return region::TopText;
			case 3:
				return region::RightText;
			default:
				return region::BottomText;
			}
		}

		void QtRegionState::textPositionDelta( int &x, int &y ) const {
			x = x_off->value( );
			y = y_off->value( );
		}


		void QtRegionState::setTextValue( const std::string &l ) {
			text->setText( QString::fromStdString(l) );
		}
		void QtRegionState::setTextPosition( region::TextPosition pos ) {
			switch ( pos ) {
			case region::TopText:
				text_position->setValue(2);
				break;
			case region::RightText:
				text_position->setValue(3);
				break;
			case region::BottomText:
				text_position->setValue(4);
				break;
			case region::LeftText:
				text_position->setValue(1);
				break;
			}
		}
		void QtRegionState::setTextDelta( const std::vector<int> &delta ) {
			if ( delta.size( ) == 2 ) {
				x_off->setValue(delta[0]);
				y_off->setValue(delta[1]);
			}
		}

		void QtRegionState::setTextFont( const std::string &f ) {
			QString font( QString::fromStdString(f) );
			for ( int i = 0; i < font_name->count( ); ++i ) {
				if ( ! font.compare( font_name->itemText(i), Qt::CaseInsensitive ) ) {
					font_name->setCurrentIndex(i);
					break;
				}
			}
		}

		void QtRegionState::setTextFontSize( int s ) {
			// limits specified in QtRegionState.ui
			if ( s >= 7 && s <= 99 ) font_size->setValue(s);
		}

		void QtRegionState::setTextFontStyle( int s ) {
			if ( s & region::BoldText ) font_bold->setCheckState(Qt::Checked);
			if ( s & region::ItalicText ) font_italic->setCheckState(Qt::Checked);
		}

		void QtRegionState::setTextColor( const std::string &c ) {
			QString color(QString::fromStdString(c));
			for ( int i = 0; i < text_color->count( ); ++i ) {
				if ( ! color.compare( text_color->itemText(i), Qt::CaseInsensitive ) ) {
					text_color->setCurrentIndex(i);
					break;
				}
			}
		}

		void QtRegionState::setLineColor( const std::string &c ) {
			QString color(QString::fromStdString(c));
			for ( int i = 0; i < line_color->count( ); ++i ) {
				if ( ! color.compare( line_color->itemText(i), Qt::CaseInsensitive ) ) {
					line_color->setCurrentIndex(i);
					break;
				}
			}
		}

		void QtRegionState::setLineStyle( region::LineStyle s ) {
			switch ( s ) {
			case region::SolidLine:
				line_style->setCurrentIndex(0);
				break;
			case region::LSDoubleDashed:
			case region::DashLine:
				line_style->setCurrentIndex(1);
				break;
			case region::DotLine:
				line_style->setCurrentIndex(2);
				break;
			}
		}

		void QtRegionState::setLineWidth( unsigned int width ) {
			line_width->setValue(width);
		}

		void QtRegionState::setAnnotation( bool ann ) {
			region_annotation->setChecked(ann);
		}

		void QtRegionState::disableAnnotation( bool ann ) {
			region_annotation->setChecked(ann);
			region_annotation->setDisabled(true);
		}

		int QtRegionState::zMin( ) const {
			return frame_min->value( );
		}
		int QtRegionState::zMax( ) const {
			return frame_max->value( );
		}
		int QtRegionState::numFrames( ) const {
			return region_->numFrames( );
		}

		bool QtRegionState::isAnnotation( ) const {
			return region_annotation->isChecked( ) ? true : false;
		}

		void QtRegionState::stackChange( QWidget *top ) {
			if ( top == this ) {
				QString state = states->tabText(categories->currentIndex( ));
				if ( state == "coordinates" ) {
					bool updated_comboboxes = false;
					std::map<std::string,int> &coordinate_state = region_->coordState( );
					QList<QComboBox*> combos = coordinateTab->findChildren<QComboBox*>( );
					for (int i = 0; i < combos.size(); ++i) {
						QComboBox *combo = combos[i];
						std::map<std::string,int>::iterator it = coordinate_state.find(combo->objectName( ).toStdString( ));
						if ( it == coordinate_state.end( ) ) continue;
						if ( it->second != combo->currentIndex( ) ) {
							++setting_combo_box;
							combo->setCurrentIndex(it->second);
							updated_comboboxes = true;
							--setting_combo_box;
						}
					}
					if ( updated_comboboxes )
						emit positionVisible( true );
				}
			}
		}

		void QtRegionState::state_change( int ) {
			emit refreshCanvas( );
			// type of state change could be made specific (when needed)
			// with QObject::sender( ) as in color_state_change...
			emit regionChange( region_, "state" );
		}
		void QtRegionState::state_change_region_mark( int ) {
			region_->selectedCountUpdateNeeded( );
			emit refreshCanvas( );
			// type of state change could be made specific (when needed)
			// with QObject::sender( ) as in color_state_change...
			emit regionChange( region_, "state" );
		}
		void QtRegionState::color_state_change( int index ) {
			emit refreshCanvas( );
			region_->colorIndex( ) = index;

			QObject *sender = QObject::sender( );
			emit regionChange( region_, sender == line_color ? LINE_COLOR_CHANGE : sender == text_color ? "state: text color" : "state" );
		}
		void QtRegionState::state_change( bool ) {
			emit refreshCanvas( );
		}
		void QtRegionState::state_change( const QString & ) {
			emit refreshCanvas( );
		}

		void QtRegionState::line_color_change(const QString &s ) {
			if ( text_color->currentText() == last_line_color &&
			        text_color->itemText(line_color->currentIndex()) == s ) {
				text_color->setCurrentIndex(line_color->currentIndex());
			}
			last_line_color = s;
		}

		void QtRegionState::update_save_type(const QString &txt) {
			QFileInfo fi(txt);
			if ( fi.suffix( ) == "reg" )
				save_file_type->setCurrentIndex(1);
			else if ( fi.suffix( ) == "crtf" )
				save_file_type->setCurrentIndex(0);
		}

		void QtRegionState::update_load_type(const QString &txt) {
			QFileInfo fi(txt);
			if ( fi.exists( ) ) {
				const int buffer_size = 1024;
				char buffer[buffer_size];
				FILE *fh = fopen( txt.toAscii( ).constData( ), "r" );
				if ( fh ) {
					if (fgets( buffer, buffer_size-1, fh )) {
						const char ds9header[] = "# Region file format: DS9";
						const char casaheader[] = "#CRTFv0 CASA Region Text Format";
						if ( strncmp(ds9header, buffer, sizeof(ds9header)-1) == 0 ) {
							load_file_type->setCurrentIndex(1);
						} else if ( strncmp(casaheader, buffer, sizeof(casaheader)-1) == 0 ) {
							load_file_type->setCurrentIndex(0);
						}
					}
					fclose(fh);
				}
			}

		}

		void QtRegionState::load_regions( bool ) {
			QString path = load_filename->text( );
			if ( path == "" ) {
#if QT_VERSION >= 0x040700
				load_filename->setPlaceholderText(QApplication::translate("QtRegionState", "please enter a file name or use 'browse' button", 0, QApplication::UnicodeUTF8));
#endif
				load_now->setFocus(Qt::OtherFocusReason);
				return;
			}

			bool found = false;
			QFileInfo fi(path);
			if ( fi.exists( ) ) {
				found = true;
			} else {
				if ( load_file_type->currentText( ).compare("CASA region file") == 0 ) {
					QString newpath = path + casa_ext;
					QFileInfo nfi(newpath);
					if ( nfi.exists( ) ) {
						path = newpath;
						found = true;
					}
				} else if ( load_file_type->currentText( ).compare("DS9 region file") == 0 ) {
					QString newpath = path + ds9_ext;
					QFileInfo nfi(newpath);
					if ( nfi.exists( ) ) {
						path = newpath;
						found = true;
					}
				}
			}

			if ( found ) {
				int fd = open( path.toAscii( ).constData( ), O_RDONLY );
				if ( fd == -1 ) {
					char *buf = (char*) malloc((strlen(path.toAscii( ).constData( )) + 50) * sizeof(char));
					sprintf( buf, "could not read %s", path.toAscii( ).constData( ) );
					load_filename->clear( );
#if QT_VERSION >= 0x040700
					load_filename->setPlaceholderText(QApplication::translate("QtRegionState", buf, 0, QApplication::UnicodeUTF8));
#endif
					load_now->setFocus(Qt::OtherFocusReason);
					free(buf);
					return;
				} else {
					::close(fd);
				}
			} else {
				char *buf = (char*) malloc((strlen(path.toAscii( ).constData( )) + 50) * sizeof(char));
				sprintf( buf, "file '%s' does not exist", path.toAscii( ).constData( ) );
				load_filename->clear( );
#if QT_VERSION >= 0x040700
				load_filename->setPlaceholderText(QApplication::translate("QtRegionState", buf, 0, QApplication::UnicodeUTF8));
#endif
				load_now->setFocus(Qt::OtherFocusReason);
				free(buf);
				return;
			}

			emit loadRegions( path, load_file_type->currentText( ) == "CASA region file" ? "casa region" : "ds9 region" );
		}

		void QtRegionState::update_default_file_extension(const QString &txt) {
			if ( txt.compare("CASA region file") == 0 ) {
				default_ext = casa_ext;
				csys_box->hide( );
			} else if ( txt.compare("DS9 region file") == 0 ) {
				default_ext = ds9_ext;
				csys_box->show( );
			}
		}

		QString QtRegionState::default_extension( const QString &base ) {
			if ( base.contains('.') ) return base;
			else return base + QString(default_ext);
		}

		void QtRegionState::save_region( bool ) {
			QString path = save_filename->text( );
			if ( path == "" ) {
#if QT_VERSION >= 0x040700
				save_filename->setPlaceholderText(QApplication::translate("QtRegionState", "please enter a file name or use 'browse' button", 0, QApplication::UnicodeUTF8));
#endif
				save_now->setFocus(Qt::OtherFocusReason);
				return;
			}

			QString name = default_extension(path);

			bool do_unlink = false;
			int fd = open( name.toAscii( ).constData( ), O_WRONLY | O_APPEND );
			if ( fd == -1 ) {
				fd = open( name.toAscii( ).constData( ), O_WRONLY | O_APPEND | O_CREAT, 0644 );
				if ( fd != -1 ) do_unlink = true;
			}
			if ( fd == -1 ) {
				char *buf = (char*) malloc((strlen(name.toAscii( ).constData( )) + 50) * sizeof(char));
				sprintf( buf, "unable to write to %s", name.toAscii( ).constData( ) );
				save_filename->clear( );
#if QT_VERSION >= 0x040700
				save_filename->setPlaceholderText(QApplication::translate("QtRegionState", buf, 0, QApplication::UnicodeUTF8));
#endif
				save_now->setFocus(Qt::OtherFocusReason);
				free(buf);
				return;
			} else {
				::close(fd);
				if ( do_unlink ) unlink(name.toAscii( ).constData( ));
			}

			QString what( save_current_region->isChecked( ) ? "current" :
			              save_marked_regions->isChecked( ) ? "marked" : "all" );
			emit outputRegions( what, name, save_file_type->currentText( ), save_csys_type->currentText( ) );
		}

// this gets called for each mouse movement as the mouse crosses the region...
// it would be nice if there were a better way to update the tab state...
		void QtRegionState::nowVisible( ) {
			if ( region_ == 0 ) return;
			std::pair<int,int> &tab_state = region_->tabState( );
			categories->setCurrentIndex(tab_state.first);
			if ( tab_state.second >= 0 ) {
				QList<QTabWidget*> tabs = categories->currentWidget( )->findChildren<QTabWidget*>( );
				if ( tabs.size( ) > 0 )
					tabs.first( )->setCurrentIndex(tab_state.second);
			}
		}

		std::string QtRegionState::mode( ) const {
			QString cat = categories->tabText(categories->currentIndex( ));
			if ( cat == PROPERTIES_MODE( ) ) {
				QString state = states->tabText(categories->currentIndex( ));
				if ( state == "coordinates" ) return "position";
				if ( state == "line" ) return "line";
				if ( state == "text" ) return "text";
			} else if ( cat == STATISTICS_MODE( ) ) {
				return STATISTICS_MODE( ).toStdString();
			} else if ( cat == FIT_MODE( ) ) {
				return FIT_MODE( ).toStdString();
			} else if ( cat == FILE_MODE( ) ) {
				return "output";
			} else if ( cat == HISTOGRAM_MODE( )) {
				return HISTOGRAM_MODE( ).toStdString();
			}
			return "";
		}

		void QtRegionState::category_change( int index ) {
			// this (something of a hack) is required to allow the configuraiton
			// (i.e. with the same context & state as the user configured here)
			// to be set for newly created regions...
			std::pair<int,int> &tab_state = region_->tabState( );
			QList<QTabWidget*> tabs = categories->currentWidget( )->findChildren<QTabWidget*>( );
			tab_state.first = index;
			region_->refresh_state_gui( ); /***updateStatistics***/
		}

		void QtRegionState::states_change( int ) {
			// this callback is called for all of the "state" combo-boxes too...
			// this assignment really only needs to be done when the "states" tab changes...
			std::pair<int,int> &tab_state = region_->tabState( );
			tab_state.second = states->currentIndex( );

			// coordinates tab selected or not...
			QString state = states->tabText(categories->currentIndex( ));
			if ( state == "coordinates" )
				emit positionVisible( true );
			else
				emit positionVisible( false );
		}

		void QtRegionState::states_val_change( int ) {
			if ( setting_combo_box ) return;
			// coordinates tab selected or not...
			QString state = states->tabText(categories->currentIndex( ));
			if ( state == "coordinates" ) {
				std::map<std::string,int> &coordinate_state = region_->coordState( );
				QList<QComboBox*> combos = coordinateTab->findChildren<QComboBox*>( );
				for (int i = 0; i < combos.size(); ++i) {
					const QComboBox *combo = combos.at(i);
					std::map<std::string,int>::iterator it = coordinate_state.find(combo->objectName( ).toStdString( ));
					it->second = combo->currentIndex( );
				}
				emit positionVisible( true );
			} else {
				emit positionVisible( false );
			}
		}

		void QtRegionState::filetab_change( int index ) {
			std::pair<int,int> &tab_state = region_->tabState( );
			tab_state.second = index;
		}

		void QtRegionState::coordsys_change( const QString &text ) {
			std::map<std::string,int> &coordinate_state = region_->coordState( );
			coordinate_state["coordinate_system"] = coordinate_system->currentIndex( );

			// pixels are unitless...
			if ( text == "pixel" ) {
				x_units->setDisabled(true);
				y_units->setDisabled(true);
				dim_units->setDisabled(true);
			} else {
				x_units->setDisabled(false);
				y_units->setDisabled(false);
				dim_units->setDisabled(false);
			}

			// coordinates tab selected or not...
			QString state = states->tabText(categories->currentIndex( ));
			if ( state == "coordinates" )
				emit positionVisible( true );
			else
				emit positionVisible( false );
		}

		void QtRegionState::translate_x( ) {
			emit translateX( center_x->displayText( ), x_units->currentText( ), coordinate_system->currentText( ) );
		}
		void QtRegionState::translate_y( ) {
			emit translateY( center_y->displayText( ), y_units->currentText( ), coordinate_system->currentText( ) );
		}
		void QtRegionState::resize_x( ) {
			emit resizeX( bounding_width->displayText( ), dim_units->currentText( ), coordinate_system->currentText( ) );
		}
		void QtRegionState::resize_y( ) {
			emit resizeY( bounding_height->displayText( ), dim_units->currentText( ), coordinate_system->currentText( ) );
		}

		void QtRegionState::frame_min_change( int v ) {
			frame_max->setMinimum(v);
			emit zRange( v, frame_max->value( ) );
		}
		void QtRegionState::frame_max_change( int v ) {
			frame_min->setMaximum(v);
			emit zRange( frame_min->value( ), v );
		}

// invoked from QtRegionDock...
// void QtRegionState::justExposed( ) {
// 	QString cat = categories->tabText(categories->currentIndex( ));
// 	if ( cat == "stats" )
// 		emit statisticsVisible( true );
// 	else
// 		emit statisticsVisible( false );
// }

		std::string QtRegionState::bounding_index_to_string( int index ) const {
			switch ( index ) {
			case 0:
				return("rad");
			case 1:
				return("deg");
			case 2:
				return("arcsec");
			case 3:
				return("arcmin");
			case 4:
				return("pixel");
			default:
				return("rad");
			}
			return("rad");
		}

		void QtRegionState::getCoordinatesAndUnits( region::Coord &c, region::Units &xu, region::Units &yu, std::string &bounding_units ) const {
			switch ( coordinate_system->currentIndex( ) ) {
			case 0:
				c = region::J2000;
				break;
			case 1:
				c = region::B1950;
				break;
			case 2:
				c = region::Galactic;
				break;
			case 3:
				c = region::SuperGalactic;
				break;
			case 4:
				c = region::Ecliptic;
				break;
			default:
				c = region::J2000;
				break;
			}
			switch ( x_units->currentIndex( ) ) {
			case 0:
				xu = region::Radians;
				break;
			case 1:
				xu = region::Degrees;
				break;
			case 2:
				xu = region::Sexagesimal;
				break;
			case 3:
				xu = region::Pixel;
				break;
			default:
				xu = region::Radians;
				break;
			}
			switch ( y_units->currentIndex( ) ) {
			case 0:
				yu = region::Radians;
				break;
			case 1:
				yu = region::Degrees;
				break;
			case 2:
				yu = region::Sexagesimal;
				break;
			case 3:
				yu = region::Pixel;
				break;
			default:
				yu = region::Radians;
				break;
			}

			bounding_units = bounding_index_to_string(dim_units->currentIndex( ));

		}

		void QtRegionState::setCoordinatesAndUnits( region::Coord c, region::Units xu, region::Units yu, const std::string &bounding_units ) {
			switch( c ) {
			case region::J2000:
				coordinate_system->setCurrentIndex( 0 );
				break;
			case region::B1950:
				coordinate_system->setCurrentIndex( 1 );
				break;
			case region::Galactic:
				coordinate_system->setCurrentIndex( 2 );
				break;
			case region::SuperGalactic:
				coordinate_system->setCurrentIndex( 3 );
				break;
			case region::Ecliptic:
				coordinate_system->setCurrentIndex( 4 );
				break;
			default:
				coordinate_system->setCurrentIndex( 0 );
				break;
			}
			switch ( xu ) {
			case region::Radians:
				x_units->setCurrentIndex( 0 );
				break;
			case region::Degrees:
				x_units->setCurrentIndex( 1 );
				break;
			case region::Sexagesimal:
				x_units->setCurrentIndex( 2 );
				break;
			case region::Pixel:
				x_units->setCurrentIndex( 3 );
				break;
			default:
				x_units->setCurrentIndex( 0 );
				break;
			}
			switch ( yu ) {
			case region::Radians:
				y_units->setCurrentIndex( 0 );
				break;
			case region::Degrees:
				y_units->setCurrentIndex( 1 );
				break;
			case region::Sexagesimal:
				y_units->setCurrentIndex( 2 );
				break;
			case region::Pixel:
				y_units->setCurrentIndex( 3 );
				break;
			default:
				y_units->setCurrentIndex( 0 );
				break;
			}

			if ( bounding_units == "deg" )
				dim_units->setCurrentIndex(1);
			else if ( bounding_units == "arcsec" )
				dim_units->setCurrentIndex(2);
			else if ( bounding_units == "arcmin" )
				dim_units->setCurrentIndex(3);
			else if ( bounding_units == "pixel" )
				dim_units->setCurrentIndex(4);
			else
				dim_units->setCurrentIndex(0);
		}

		void QtRegionState::updatePosition( const QString &x, const QString &y, const QString &angle, const QString &width, const QString &height ) {
			center_x->setText(x);
			center_y->setText(y);
			center_angle->setText(angle);
			bounding_width->setText(width);
			bounding_height->setText(height);
		}

		void QtRegionState::noOutputNotify( ) {
			save_filename->clear( );
#if QT_VERSION >= 0x040700
			save_filename->setPlaceholderText(QApplication::translate("QtRegionState", "no regions were selected for output...", 0, QApplication::UnicodeUTF8));
#endif
			save_now->setFocus(Qt::OtherFocusReason);
		}

		void QtRegionState::mark( bool set ) {
			region_->selectedCountUpdateNeeded( );
			region_mark->setChecked( set );
		}

		bool QtRegionState::mark_toggle( ) {
			region_->selectedCountUpdateNeeded( );
			bool newstate = region_mark->isChecked( ) ? false : true;
			region_mark->setChecked( newstate );
			return newstate;
		}

		void QtRegionState::save_browser(bool) {

			QFileDialog *dlg = new QFileDialog( 0, "Select a Region File for Output", region_->getSaveDir( ) );
			dlg->setModal(true);
			dlg->setFileMode(QFileDialog::AnyFile);
			dlg->setOption(QFileDialog::DontUseNativeDialog, true);

			if ( dlg->exec( ) == QDialog::Accepted ) {
				QStringList selected = dlg->selectedFiles();
				if ( selected.count( ) > 0 ) {
					save_filename->setText(selected.first( ));
				}
				region_->putSaveDir( dlg->directory( ).canonicalPath( ) );
			}
			delete dlg;

		}

		void QtRegionState::load_browser(bool) {

			QFileDialog *dlg = new QFileDialog( 0, "Select a Region File to Load", region_->getLoadDir( ) );
			dlg->setModal(true);
			dlg->setFileMode(QFileDialog::ExistingFile);
			dlg->setOption(QFileDialog::DontUseNativeDialog, true);

			if ( dlg->exec( ) == QDialog::Accepted ) {
				QStringList selected = dlg->selectedFiles();
				if ( selected.count( ) > 0 ) {
					load_filename->setText(selected.first( ));
				}
				region_->putLoadDir( dlg->directory( ).canonicalPath( ) );
			}
			delete dlg;

		}

		void QtRegionState::set_point_region_marker( int index ) {
			QVariant qv = marker->itemData(index);
			if ( qv.type( ) != QVariant::Int ) return;
			bool ok;
			int sym = qv.toInt(&ok);
			if ( ok == false ) return;
			if ( sym < 0 || sym > QtMouseToolNames::SYM_POINT_REGION_COUNT ) return;
			if ( region_->setMarker( (QtMouseToolNames::PointRegionSymbols) sym ) )
				emit refreshCanvas( );
		}


		QtPVLineState::QtPVLineState( const QString &name,
		                              QtMouseToolNames::PointRegionSymbols sym, Region *region, QWidget *parent ) :
			QtRegionState(name,sym,region,parent) {
			categories->removeTab(categories->indexOf(imfit_tab));
			QString old_label = QtRegionState::STATISTICS_MODE( );
			QString new_label = STATISTICS_MODE( );
			for ( int i=0; i < categories->count( ); ++i )
				if ( categories->tabText(i) == old_label ) {
					categories->setTabText(i,new_label);
					categories->setCurrentIndex(i);
					break;
				}
		}

		QtSliceCutState::QtSliceCutState( const QString &name,
		                                  QtMouseToolNames::PointRegionSymbols sym, Region *region, QWidget *parent ) :
			QtRegionState(name,sym,region,parent) {
			categories->removeTab(categories->indexOf(imfit_tab));
			QString old_label = QtRegionState::STATISTICS_MODE( );
			QString new_label = STATISTICS_MODE( );
			for ( int i=0; i < categories->count( ); ++i )
				if ( categories->tabText(i) == old_label ) {
					categories->setTabText(i,new_label);
					categories->setCurrentIndex(i);
					break;
				}
		}

		QtPointState::QtPointState( const QString &name, QtMouseToolNames::PointRegionSymbols sym,
			                        Region *region, QWidget *parent ) : QtRegionState(name,sym,region,parent) {
			categories->removeTab(categories->indexOf(imfit_tab));
		}

	}
}

