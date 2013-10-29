//# QtRegionStats.cc: the GUI table for displaying region statistics
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

#include <QDebug>
#include <display/region/QtRegionStats.qo.h>
#include <display/region/RegionInfo.h>
#include <QtGui/QStackedWidget>
#include <display/region/Region.qo.h>
#include <display/region/RegionInfo.h>
#include <display/region/Polyline.qo.h>
#include <display/DisplayErrors.h>

namespace casa {
	namespace viewer {
		namespace qt {

			void stats_t::stats_t::setLabels( const std::string &label, const std::string &desc ) {
				label_ = label;
				description_ = desc;
				setTitle(QString::fromStdString(label_));
			}

			image_stats_t::image_stats_t( statfield_list_t &fields, QWidget *parent )  : stats_t(parent) {
				setupUi(this);
				fields.push_back(statfield_list_t::value_type(box11,text11));
				fields.push_back(statfield_list_t::value_type(box12,text12));
				fields.push_back(statfield_list_t::value_type(box13,text13));
				fields.push_back(statfield_list_t::value_type(box14,text14));
				fields.push_back(statfield_list_t::value_type(box21,text21));
				fields.push_back(statfield_list_t::value_type(box22,text22));
				fields.push_back(statfield_list_t::value_type(box23,text23));
				fields.push_back(statfield_list_t::value_type(box24,text24));
				fields.push_back(statfield_list_t::value_type(box31,text31));
				fields.push_back(statfield_list_t::value_type(box32,text32));
				fields.push_back(statfield_list_t::value_type(box33,text33));
				fields.push_back(statfield_list_t::value_type(box34,text34));
				fields.push_back(statfield_list_t::value_type(box41,text41));
				fields.push_back(statfield_list_t::value_type(box42,text42));
				fields.push_back(statfield_list_t::value_type(box43,text43));
				fields.push_back(statfield_list_t::value_type(box44,text44));
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

			SliceStats::SliceStats( QWidget *parent ) : stats_t(parent) {
				setupUi(this);
				connect( sliceToolButton, SIGNAL(clicked()), this, SIGNAL(show1DSliceTool()));
			}

			pvline_stats_t::pvline_stats_t( QWidget *parent ) : stats_t(parent) {
				setupUi(this);
				// coord_box->hide( );
				// averaging_frame->hide( );
				connect( create_button, SIGNAL(clicked( )), this, SLOT(create_pv_image( )) );
				connect( pvwidth, SIGNAL(valueChanged(int)), this, SIGNAL(setPVInfo(int)) );
			}

			bool pvline_stats_t::updateStatisticsInfo( std::tr1::shared_ptr<casa::viewer::RegionInfo> info ) {
				try {
					std::tr1::shared_ptr<PVLineRegionInfo> pvinfo = std::tr1::dynamic_pointer_cast<PVLineRegionInfo>(info);
					pixel_pt1->setText(QString::fromStdString(pvinfo->pixelStrings( )[0]));
					pixel_pt2->setText(QString::fromStdString(pvinfo->pixelStrings( )[1]));
					world_pt1->setText(QString::fromStdString(pvinfo->worldStrings( )[0]));
					world_pt2->setText(QString::fromStdString(pvinfo->worldStrings( )[1]));
					position_angle->setText(QString::fromStdString(pvinfo->positionAngle( )) + QString::fromUtf8("˚"));
					separation->setText(QString::fromStdString(pvinfo->separation( )));
				} catch (...) {
					/* expect only std::bad_cast */
				}
				return true;
			}

			void pvline_stats_t::enterEvent (QEvent *e) {
				QWidget::enterEvent(e);
				emit setPVInfo(pvwidth->value( ));
			}

			void pvline_stats_t::showEvent ( QShowEvent *e ) {
				QWidget::showEvent(e);
				emit setPVInfo(pvwidth->value( ));
			}

			void pvline_stats_t::create_pv_image( ) {
				emit createPVImage(label_,description_,pvwidth->value( ));
			}

		}

		QtRegionStats::QtRegionStats( QWidget *parent ) : QWidget(parent), container_(0), next_(0), stats_box_(0) {
			layout_ = new QVBoxLayout(this);
			// stats_box_ = new qt::image_stats_t(fields,this);
		}

		QtRegionStats::~QtRegionStats( ) { }

		void QtRegionStats::updateStatistics( std::tr1::shared_ptr<casa::viewer::RegionInfo> stats, Region* region ) {

			new_stats_box(stats->type(), region, stats->label() )->setLabels( stats->label( ), stats->description( ) );

			qt::statfield_list_t::iterator fiter = fields.begin( );
			std::list<std::pair<String,String> >::iterator siter = (*stats->list()).begin( );

#if defined(__APPLE__)
			QFont stat_box_font( "Lucida Grande", 11 );
			QFont stat_field_font( "Lucida Grande", 10 );
#else
			QFont stat_box_font( "Sans Serif", 9 );
			QFont stat_field_font( "Sans Serif", 8 );
#endif
			while ( fiter != fields.end( ) && siter != (*stats->list()).end( ) ) {
				(*fiter).first->show( );
				(*fiter).first->setFont( stat_box_font );
				(*fiter).first->setTitle( QString::fromStdString((*siter).first) );
				(*fiter).second->setFont( stat_field_font );
				(*fiter).second->setText( QString::fromStdString((*siter).second) );
				// (*fiter).second->setCursorPosition(0);
				++fiter;
				++siter;
			}
			// clear out unused fields
			while ( fiter != fields.end( ) ) {
				(*fiter).first->hide( );
				// (*fiter).first->setTitle( "" );
				// (*fiter).second->setText( "" );
				++fiter;
			}

		}

		void QtRegionStats::setCenterBackground(QString background) {
            ///////// the display widget has switched from an
            ///////// un-editable entry widget to a label widget
            ///////// so it does not need to have its background
            ///////// changed...
			// qt::statfield_list_t::iterator fitter = fields.begin( );
			// while ( fitter != fields.end( )) {
			// 	QString labelStyle( "QLabel{background-color: "+background +";}");
			// 	(*fitter).second->setStyleSheet( labelStyle );
			// 	++fitter;
			// }
		}

		void QtRegionStats::disableNextButton( ) {
			if ( stats_box_ && stats_box_->next( ) ) {
				stats_box_->next( )->setDisabled(true);
				disconnect(stats_box_->next(), 0, 0, 0);
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

			if ( stats_box_ ) disableNextButton( );

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

		bool QtRegionStats::updateStatisticsInfo( std::tr1::shared_ptr<casa::viewer::RegionInfo> info ) {
			return stats_box_ ? stats_box_->updateStatisticsInfo( info ) : false;
		}

		void QtRegionStats::go_next( ) {
			if ( container_ == 0 || next_ == 0 ) return;
			int nextIndex = container_->indexOf( next_ );
			if ( nextIndex >= 0 ){
				container_->setCurrentWidget(next_);
			}
		}

		qt::stats_t *QtRegionStats::new_stats_box( RegionInfo::InfoTypes type, Region* region, const string& label ) {

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
			} else if ( type == RegionInfo::SliceInfoType ) {
				stats_box_ = new qt::SliceStats();
				if ( region != NULL ) {
					//Add the plot in place of the normal statistics
					Polyline* polylineRegion = dynamic_cast<Polyline*>(region);
					qt::SliceStats* sliceBox = dynamic_cast<qt::SliceStats*>(stats_box_);
					polylineRegion->addPlot( sliceBox->getPlotHolder(), label );
					connect( stats_box_, SIGNAL(show1DSliceTool()), region, SIGNAL(show1DSliceTool()));
				}
				layout_->addWidget(stats_box_,0,Qt::AlignLeft);
				return stats_box_;
			}
			if ( type == RegionInfo::PVLineInfoType ) {
				stats_box_ = new qt::pvline_stats_t( );
				layout_->addWidget(stats_box_,0,Qt::AlignLeft);
				if ( region != NULL ) {
					connect( stats_box_, SIGNAL(createPVImage(const std::string&,const std::string&,int)),
							 region, SLOT(createPVImage(const std::string&,const std::string&,int)) );
					connect( stats_box_, SIGNAL(setPVInfo(int)), region, SLOT(changePVInfo(int)) );
				}
				return stats_box_;
			} else {
				throw internal_error("QtRegionStats inconsistency");
			}
		}
	}
}

