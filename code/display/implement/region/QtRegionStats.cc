#include <display/region/QtRegionStats.qo.h>
#include <QtGui/QStackedWidget>

namespace casa {
    namespace viewer {

      QtRegionStats::QtRegionStats( QWidget *parent ) : QGroupBox( "", parent ), container_(0), next_(0) {
	    setupUi(this);

	    fields.push_back(statfield_list::value_type(box11,text11));
	    fields.push_back(statfield_list::value_type(box12,text12));
	    fields.push_back(statfield_list::value_type(box13,text13));
	    fields.push_back(statfield_list::value_type(box14,text14));
	    fields.push_back(statfield_list::value_type(box15,text15));
	    fields.push_back(statfield_list::value_type(box21,text21));
	    fields.push_back(statfield_list::value_type(box22,text22));
	    fields.push_back(statfield_list::value_type(box23,text23));
	    fields.push_back(statfield_list::value_type(box24,text24));
	    fields.push_back(statfield_list::value_type(box25,text25));
	    fields.push_back(statfield_list::value_type(box31,text31));
	    fields.push_back(statfield_list::value_type(box32,text32));
	    fields.push_back(statfield_list::value_type(box33,text33));
	    fields.push_back(statfield_list::value_type(box34,text34));
	    fields.push_back(statfield_list::value_type(box35,text35));
	}

	QtRegionStats::~QtRegionStats( ) { }

	void QtRegionStats::updateStatistics( const String &s, std::list<std::pair<String,String> > &stats ) {

	    setTitle(QString::fromStdString(s));

	    statfield_list::iterator fiter = fields.begin( );
	    std::list<std::pair<String,String> >::iterator siter = stats.begin( );
	    while ( fiter != fields.end( ) && siter != stats.end( ) ) {
		(*fiter).first->setTitle( QString::fromStdString((*siter).first) );
		(*fiter).second->setText( QString::fromStdString((*siter).second) );
		(*fiter).second->setCursorPosition(0);
		++fiter;
		++siter;
	    }
	}

	void QtRegionStats::reset( ) {

	    // Statistics update clears fields that are not filled...
	    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
	    // clear all the key/value fields
	    // setTitle("");
	    // for (statfield_list::iterator iter = fields.begin(); iter != fields.end(); ++iter ) {
	    // 	(*iter).first->setTitle("");
	    // 	(*iter).second->setText("");
	    // }

	    next->setDisabled(true);
	    disconnect(next, 0, 0, 0);

	    // zero next state
	    container_ = 0;
	    next_ = 0;
	}

#if OLDSTUFF
	void QtRegionStats::addstats( std::list<std::pair<String,String> > *stats ) {
	    statfield_list::iterator fiter = fields.begin( );
	    std::list<std::pair<String,String> >::iterator siter = stats->begin( );
	    while ( fiter != fields.end( ) && siter != stats->end( ) ) {
		(*fiter).first->setTitle( QString::fromStdString((*siter).first) );
		(*fiter).second->setText( QString::fromStdString((*siter).second) );
		++fiter;
		++siter;
	    }
	}
#endif

	void QtRegionStats::setNext( QStackedWidget *c, QtRegionStats *n ) {
	    container_ = c;
	    next_ = n;
	    next->setDisabled(false);
	    connect(next, SIGNAL(pressed( )), SLOT(go_next( )));
	}

	void QtRegionStats::go_next( ) {
	    if ( container_ == 0 || next_ == 0 ) return;
	    container_->setCurrentWidget(next_);
	}
      
    }
}

