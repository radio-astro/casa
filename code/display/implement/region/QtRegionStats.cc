#include <display/region/QtRegionStats.qo.h>
#include <QtGui/QStackedWidget>

namespace casa {
    namespace viewer {

	QtRegionStats::QtRegionStats( QWidget *parent ) : QWidget(parent), container_(0), next_(0) {

	    image_stats_box = new qt::image_stats_t(this);
	    
	    fields.push_back(statfield_list::value_type(image_stats_box->box11,image_stats_box->text11));
	    fields.push_back(statfield_list::value_type(image_stats_box->box12,image_stats_box->text12));
	    fields.push_back(statfield_list::value_type(image_stats_box->box13,image_stats_box->text13));
	    fields.push_back(statfield_list::value_type(image_stats_box->box14,image_stats_box->text14));
	    fields.push_back(statfield_list::value_type(image_stats_box->box15,image_stats_box->text15));
	    fields.push_back(statfield_list::value_type(image_stats_box->box21,image_stats_box->text21));
	    fields.push_back(statfield_list::value_type(image_stats_box->box22,image_stats_box->text22));
	    fields.push_back(statfield_list::value_type(image_stats_box->box23,image_stats_box->text23));
	    fields.push_back(statfield_list::value_type(image_stats_box->box24,image_stats_box->text24));
	    fields.push_back(statfield_list::value_type(image_stats_box->box25,image_stats_box->text25));
	    fields.push_back(statfield_list::value_type(image_stats_box->box31,image_stats_box->text31));
	    fields.push_back(statfield_list::value_type(image_stats_box->box32,image_stats_box->text32));
	    fields.push_back(statfield_list::value_type(image_stats_box->box33,image_stats_box->text33));
	    fields.push_back(statfield_list::value_type(image_stats_box->box34,image_stats_box->text34));
	    fields.push_back(statfield_list::value_type(image_stats_box->box35,image_stats_box->text35));
	}

	QtRegionStats::~QtRegionStats( ) { }

	void QtRegionStats::updateStatistics( const String &s, std::list<std::pair<String,String> > &stats ) {

	    image_stats_box->setTitle(QString::fromStdString(s));

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

	    image_stats_box->next->setDisabled(true);
	    disconnect(image_stats_box->next, 0, 0, 0);

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
	    image_stats_box->next->setDisabled(false);
	    connect(image_stats_box->next, SIGNAL(pressed( )), SLOT(go_next( )));
	}

	void QtRegionStats::go_next( ) {
	    if ( container_ == 0 || next_ == 0 ) return;
	    container_->setCurrentWidget(next_);
	}
      
    }
}

