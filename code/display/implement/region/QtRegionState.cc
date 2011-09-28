#include <fcntl.h>
#include <unistd.h>
#include <QDebug>
#include <display/region/QtRegion.qo.h>
#include <display/region/QtRegionState.qo.h>

namespace casa {
    namespace viewer {

	QtRegionState::freestat_list *QtRegionState::freestats = 0;

	QtRegionState::QtRegionState( const QString &n, QtRegion *r, QWidget *parent ) :
					QFrame(parent), selected_statistics(-1), region_(r) {
	    setupUi(this);

	    //setLineWidth(0);
	    setFrameShape(QFrame::NoFrame);

	    if ( action_tab->count( ) == 2 && action_tab->tabText(1) == "Tab 2" )
		action_tab->removeTab(1);

	    coordinate_angle_box->hide( );

	    if ( freestats == 0 )
		freestats = new freestat_list( );

	    text_position->setWrapping(true);

	    name->setPlaceholderText(QApplication::translate("QtRegionState", n.toAscii( ).constData( ), 0, QApplication::UnicodeUTF8));


	    // update line characteristics...
	    connect( line_color, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
	    connect( line_style, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
	    connect( line_width, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
	    connect( text_position, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
	    connect( text_color, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
	    connect( font_name, SIGNAL(currentIndexChanged(int)), SLOT(state_change(int)) );
	    connect( font_size, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
	    connect( x_off, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
	    connect( y_off, SIGNAL(valueChanged(int)), SLOT(state_change(int)) );
	    connect( font_italic, SIGNAL(clicked(bool)), SLOT(state_change(bool)) );
	    connect( font_bold, SIGNAL(clicked(bool)), SLOT(state_change(bool)) );
	    connect( save_now, SIGNAL(clicked(bool)), SLOT(save_region(bool)) );

	    int z_max = region_->numFrames( );
	    frame_min->setMaximum(z_max);
	    frame_max->setMaximum(z_max);
	    frame_max->setValue(z_max);

	    connect( frame_min, SIGNAL(valueChanged(int)), SLOT(frame_min_change(int)) );
	    connect( frame_max, SIGNAL(valueChanged(int)), SLOT(frame_max_change(int)) );

	    connect( text, SIGNAL(textChanged(const QString&)), SLOT(state_change(const QString&)) );

	    connect( categories, SIGNAL(currentChanged(int)), SLOT(category_change(int)) );
	    connect( states, SIGNAL(currentChanged(int)), SLOT(states_change(int)) );

	    connect( coordinate_system, SIGNAL(currentIndexChanged(const QString &)), SLOT(coordsys_change(const QString &)) );
	    connect( coordinate_units, SIGNAL(currentIndexChanged(int)), SLOT(states_change(int)) );
	    connect( coordinates_reset, SIGNAL(clicked(bool)), SLOT(coordinates_reset_event(bool)) );
	    connect( coordinates_apply, SIGNAL(clicked(bool)), SLOT(coordinates_apply_event(bool)) );


	    last_line_color = line_color->currentText( );
	    connect( line_color, SIGNAL(currentIndexChanged(const QString&)), SLOT(line_color_change(const QString&)) );

	}

	QtRegionState::~QtRegionState( ) {
	}

	void QtRegionState::reset( const QString &n, QtRegion *r ) {
	    name->setText(QString());
	    name->setPlaceholderText(QApplication::translate("QtRegionState", n.toAscii( ).constData( ), 0, QApplication::UnicodeUTF8));
	    region_ = r;

	    int z_max = region_->numFrames( );
	    frame_min->setMaximum(z_max);
	    frame_max->setMaximum(z_max);
	    frame_max->setValue(z_max);
	}
#if OLDSTUFF
	void QtRegionState::clearstats( ) {
	    selected_statistics = statistics_group->currentIndex( );
	    while ( statistics_group->count() > 0 ) {
		QtRegionStats *w = dynamic_cast<QtRegionStats*>(statistics_group->widget(0));
		if ( w == 0 ) throw internal_error( );
		statistics_group->removeWidget(w);
		freestats->push_back(w);
	    }
	}

	void QtRegionState::addstats( const std::string &name, std::list<std::pair<String,String> > *stats ) {
	    if ( ! stats ) return;
	    QtRegionStats *mystat;
	    // BEGIN - critical section
	    if ( freestats->size() > 0 ) {
		mystat = freestats->back( );
		freestats->pop_back( );
		// END - critical section
		mystat->reset(QString::fromStdString(name),stats->size( ));
	    } else {
		mystat = new QtRegionStats(QString::fromStdString(name));
	    }

	    mystat->addstats( stats );
	    statistics_group->insertWidget(statistics_group->count( ),mystat);

	    if ( selected_statistics >= 0 && selected_statistics == statistics_group->count( ) - 1 )
		statistics_group->setCurrentIndex(selected_statistics);

	    int num = statistics_group->count( );
	    if ( num < 2 ) return;

	    QtRegionStats *first = dynamic_cast<QtRegionStats*>(statistics_group->widget(0));
	    QtRegionStats *prev = first;
	    if ( prev == 0 ) throw internal_error( );

	    for ( int i=1; i < statistics_group->count(); ++i ) {
		QtRegionStats *cur = dynamic_cast<QtRegionStats*>(statistics_group->widget(i));
		if ( cur == 0 ) throw internal_error( );
		prev->setNext( statistics_group, cur );
		prev = cur;
	    }
	    prev->setNext( statistics_group, first );
	}
#endif

	void QtRegionState::updateStatistics( std::list<RegionInfo> *stats ) {
	    if ( stats == 0 || stats->size() == 0 ) return;

	    while ( stats->size() < statistics_group->count() ) {
		QtRegionStats *w = dynamic_cast<QtRegionStats*>(statistics_group->widget(0));
		if ( w == 0 ) throw internal_error( );
		statistics_group->removeWidget(w);
		freestats->push_back(w);
	    }
	    while ( stats->size() > statistics_group->count() ) {
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
	    QtRegionStats *first = dynamic_cast<QtRegionStats*>(statistics_group->widget(0));
	    if ( first == 0 ) throw internal_error( );
	    std::list<RegionInfo>::iterator stat_iter = stats->begin();
	    if ( stat_iter->list( ).isNull( ) ) {
		fprintf( stderr, "YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1YESYES1\n" );
	    } else {
		first->updateStatistics(*stat_iter);
	    }
	    if ( num < 2 ) return;

	    QtRegionStats *prev = first;

	    for ( int i=1; i < statistics_group->count() && ++stat_iter != stats->end(); ++i ) {
		QtRegionStats *cur = dynamic_cast<QtRegionStats*>(statistics_group->widget(i));
		if ( cur == 0 ) throw internal_error( );
		if ( stat_iter->list( ).isNull( ) ) {
		    fprintf( stderr, "YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2YESYES2\n" );
		} else {
		    cur->updateStatistics(*stat_iter);
		}
		prev->setNext( statistics_group, cur );
		prev = cur;
	    }
	    prev->setNext( statistics_group, first );

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

	Region::LineStyle QtRegionState::lineStyle( ) const {
	    QString ls = line_style->currentText( );
	    if ( ls == "dashed" ) return Region::DashLine;
	    else if ( ls == "dotted" ) return Region::DotLine;
	    else return Region::SolidLine;
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
	    if ( font_italic->isChecked( ) ) result = result | Region::ItalicText;
	    if ( font_bold->isChecked( ) ) result = result | Region::BoldText;
	    return result;
	}

	std::string QtRegionState::textValue( ) const {
	    QString txt = text->text( );
	    return txt.toStdString( );
	}

	Region::TextPosition QtRegionState::textPosition( ) const {
	    int pos = text_position->value( );
	    switch ( pos ) {
		case 1:
		    return Region::LeftText;
		case 2:
		    return Region::TopText;
		case 3:
		    return Region::RightText;
		default:
		    return Region::BottomText;
	    }
	}

	void QtRegionState::textPositionDelta( int &x, int &y ) const {
	    x = x_off->value( );
	    y = y_off->value( );
	}

	int QtRegionState::zMin( ) const { return frame_min->value( ); }
	int QtRegionState::zMax( ) const { return frame_max->value( ); }
	int QtRegionState::numFrames( ) const { return region_->numFrames( ); }


	void QtRegionState::state_change( int ) { emit refreshCanvas( ); }
	void QtRegionState::state_change( bool ) { emit refreshCanvas( ); }
	void QtRegionState::state_change( const QString & ) { emit refreshCanvas( ); }

	void QtRegionState::line_color_change(const QString &s ) {
	    if ( text_color->currentText() == last_line_color &&
		 text_color->itemText(line_color->currentIndex()) == s ) {
		text_color->setCurrentIndex(line_color->currentIndex());
	    }
	    last_line_color = s;
	}

	void QtRegionState::save_region( bool ) {
	    if ( save_filename->text() == "" ) {
		save_filename->setPlaceholderText(QApplication::translate("QtRegionState", "please enter a file name or use 'browse' button", 0, QApplication::UnicodeUTF8));
		save_now->setFocus(Qt::OtherFocusReason);
		return;
	    }
	    QString name = save_filename->text();

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
		save_filename->setPlaceholderText(QApplication::translate("QtRegionState", buf, 0, QApplication::UnicodeUTF8));
		save_now->setFocus(Qt::OtherFocusReason);
		return;
	    } else {
		::close(fd);
		if ( do_unlink ) unlink(name.toAscii( ).constData( ));
	    }

	    QString what( save_current_region->isChecked( ) ? "current" :
			  save_marked_regions->isChecked( ) ? "marked" : "all" );
	    emit outputRegion( what, name );
	}

	void QtRegionState::category_change( int ) {
	    QString cat = categories->tabText(categories->currentIndex( ));
	    if ( cat == "stats" )
		emit statisticsVisible( true );
	    else
		emit statisticsVisible( false );
	}

	void QtRegionState::states_change( int ) {
	    // coordinates tab selected or not...
	    QString state = states->tabText(categories->currentIndex( ));
	    if ( state == "coordinates" )
		emit positionVisible( true );
	    else
		emit positionVisible( false );
	}

	void QtRegionState::coordsys_change( const QString &text ) {
	    // pixels are unitless...
	    if ( text == "pixel" )
		coordinate_units->setDisabled(true);
	    else
		coordinate_units->setDisabled(false);
	    // coordinates tab selected or not...
	    QString state = states->tabText(categories->currentIndex( ));
	    if ( state == "coordinates" )
		emit positionVisible( true );
	    else
		emit positionVisible( false );
	}

	void QtRegionState::coordinates_reset_event(bool) {
	    emit positionVisible(true);
	}

	void QtRegionState::coordinates_apply_event(bool) {
	    emit positionMove(center_x->displayText( ),center_y->displayText( ),coordinate_system->currentText( ),coordinate_units->currentText( ));
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
	void QtRegionState::justExposed( ) {
	    QString cat = categories->tabText(categories->currentIndex( ));
	    if ( cat == "stats" )
		emit statisticsVisible( true );
	    else
		emit statisticsVisible( false );
	}

	void QtRegionState::getCoordinatesAndUnits( Region::Coord &c, Region::Units &u ) const {
	    switch ( coordinate_system->currentIndex( ) ) {
		case 0: c = Region::J2000; break;
		case 1: c = Region::B1950; break;
		case 2: c = Region::Galactic; break;
		case 3: c = Region::SuperGalactic; break;
		case 4: c = Region::Ecliptic; break;
		case 5: c = Region::Pixel; break;
		default: c = Region::J2000; break;
	    }
	    switch ( coordinate_units->currentIndex( ) ) {
		case 0: u = Region::Radians; break;
		case 1: u = Region::Degrees; break;
		case 2: u = Region::HMS; break;
		case 3: u = Region::DMS; break;
		default: u = Region::Radians; break;
	    }
	}

	void QtRegionState::setCoordinatesAndUnits( Region::Coord c, Region::Units u ) {
	    switch( c ) {
		case Region::J2000: coordinate_system->setCurrentIndex( 0 ); break;
		case Region::B1950: coordinate_system->setCurrentIndex( 1 ); break;
		case Region::Galactic: coordinate_system->setCurrentIndex( 2 ); break;
		case Region::SuperGalactic: coordinate_system->setCurrentIndex( 3 ); break;
		case Region::Ecliptic: coordinate_system->setCurrentIndex( 4 ); break;
		case Region::Pixel: coordinate_system->setCurrentIndex( 5 ); break;
		default: coordinate_system->setCurrentIndex( 0 ); break;
	    }
	    switch ( u ) {
		case Region::Radians: coordinate_units->setCurrentIndex( 0 ); break;
		case Region::Degrees: coordinate_units->setCurrentIndex( 1 ); break;
		case Region::HMS: coordinate_units->setCurrentIndex( 2 ); break;
		case Region::DMS: coordinate_units->setCurrentIndex( 3 ); break;
		default: coordinate_units->setCurrentIndex( 0 ); break;
	    }
	}

	void QtRegionState::updatePosition( const QString &x, const QString &y, const QString &angle ) {
	    center_x->setText(x);
	    center_y->setText(y);
	    center_angle->setText(angle);
	}

	void QtRegionState::noOutputNotify( ) {
	    save_filename->clear( );
	    save_filename->setPlaceholderText(QApplication::translate("QtRegionState", "no regions were selected for output...", 0, QApplication::UnicodeUTF8));
	    save_now->setFocus(Qt::OtherFocusReason);
	}

    }
}

