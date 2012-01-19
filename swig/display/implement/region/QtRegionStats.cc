#include <display/region/QtRegionStats.qo.h>
#include <QtGui/QStackedWidget>
#include <display/region/Region.h>
#include <display/region/RegionInfo.h>

namespace casa {
    namespace viewer {
	namespace qt {
	    image_stats_t::image_stats_t( statfield_list_t &fields, QWidget *parent )  : stats_t(parent) {
		setupUi(this);
		fields.push_back(statfield_list_t::value_type(box11,text11));
		fields.push_back(statfield_list_t::value_type(box12,text12));
		fields.push_back(statfield_list_t::value_type(box13,text13));
		fields.push_back(statfield_list_t::value_type(box14,text14));
		fields.push_back(statfield_list_t::value_type(box15,text15));
		fields.push_back(statfield_list_t::value_type(box21,text21));
		fields.push_back(statfield_list_t::value_type(box22,text22));
		fields.push_back(statfield_list_t::value_type(box23,text23));
		fields.push_back(statfield_list_t::value_type(box24,text24));
		fields.push_back(statfield_list_t::value_type(box25,text25));
		fields.push_back(statfield_list_t::value_type(box31,text31));
		fields.push_back(statfield_list_t::value_type(box32,text32));
		fields.push_back(statfield_list_t::value_type(box33,text33));
		fields.push_back(statfield_list_t::value_type(box34,text34));
		fields.push_back(statfield_list_t::value_type(box35,text35));
	    }

	    ms_stats_t::ms_stats_t( statfield_list_t &fields, QWidget *parent ) : stats_t(parent) {
		setupUi(this);
		fields.push_back(statfield_list_t::value_type(box11,text11));
		fields.push_back(statfield_list_t::value_type(box12,text12));
		fields.push_back(statfield_list_t::value_type(box21,text21));
		fields.push_back(statfield_list_t::value_type(box22,text22));
		fields.push_back(statfield_list_t::value_type(box31,text31));
		fields.push_back(statfield_list_t::value_type(box32,text32));
		fields.push_back(statfield_list_t::value_type(box41,text41));
		fields.push_back(statfield_list_t::value_type(box42,text42));
	    }

	}

	QtRegionStats::QtRegionStats( QWidget *parent ) : QWidget(parent), container_(0), next_(0), stats_box_(0) {
		layout_ = new QVBoxLayout(this);
		// stats_box_ = new qt::image_stats_t(fields,this);
	}

	QtRegionStats::~QtRegionStats( ) { }

	void QtRegionStats::updateStatistics( RegionInfo &stats ) {

	    new_stats_box(stats.type())->setTitle(QString::fromStdString(stats.label()));

	    qt::statfield_list_t::iterator fiter = fields.begin( );
	    std::list<std::pair<String,String> >::iterator siter = (*stats.list()).begin( );
	    while ( fiter != fields.end( ) && siter != (*stats.list()).end( ) ) {
		(*fiter).first->setTitle( QString::fromStdString((*siter).first) );
		(*fiter).second->setText( QString::fromStdString((*siter).second) );
		(*fiter).second->setCursorPosition(0);
		++fiter;
		++siter;
	    }
	    // clear out unused fields
	    while ( fiter != fields.end( ) ) {
		(*fiter).first->setTitle( "" );
		(*fiter).second->setText( "" );
		++fiter;
	    }
	      
	}

	void QtRegionStats::reset( ) {

	    // Statistics update clears fields that are not filled...
	    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
	    // clear all the key/value fields
	    // setTitle("");
	    // for (statfield_list_t::iterator iter = fields.begin(); iter != fields.end(); ++iter ) {
	    // 	(*iter).first->setTitle("");
	    // 	(*iter).second->setText("");
	    // }

	    if ( stats_box_ ) {
		stats_box_->next( )->setDisabled(true);
		disconnect(stats_box_->next(), 0, 0, 0);
	    }

	    // zero next state
	    container_ = 0;
	    next_ = 0;
	}

#if OLDSTUFF
	void QtRegionStats::addstats( std::list<std::pair<String,String> > *stats ) {
	    statfield_list_t::iterator fiter = fields.begin( );
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
	    if ( stats_box_ ) {
		stats_box_->next( )->setDisabled(false);
		connect(stats_box_->next(), SIGNAL(pressed( )), SLOT(go_next( )));
	    }
	}

	void QtRegionStats::go_next( ) {
	    if ( container_ == 0 || next_ == 0 ) return;
	    container_->setCurrentWidget(next_);
	}
      
	qt::stats_t *QtRegionStats::new_stats_box( RegionInfo::InfoTypes type ) {

	    if ( stats_box_ && stats_box_->type() == type ) return stats_box_;

	    if ( stats_box_ ) {
		// if we make it here, it means that stats_box_ is the wrong type...
		layout_->removeWidget(stats_box_);
		fields.erase(fields.begin(),fields.end());
		delete stats_box_;
	    }

	    if ( type == RegionInfo::ImageInfoType ) {
		stats_box_ = new qt::image_stats_t(fields);
		layout_->addWidget(stats_box_,0,Qt::AlignLeft);
		return stats_box_;
	    } else if ( type == RegionInfo::MsInfoType ) {
		stats_box_ = new qt::ms_stats_t(fields);
		layout_->addWidget(stats_box_,0,Qt::AlignLeft);
		return stats_box_;
	    } else {
		throw internal_error("QtRegionStats inconsistency");
	    }
	}
    }
}

