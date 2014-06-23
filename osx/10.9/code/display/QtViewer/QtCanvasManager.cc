//# QtCanvasManager.cc: GUI for managing the panel state
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
//# $Id: $

#include <display/QtViewer/QtCanvasManager.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewer.qo.h>

namespace casa {

	QtCanvasManager::QtCanvasManager( QtDisplayPanelGui *dpg ) : dpg_(dpg) {
		setupUi(this);
		options = new QtCanvasManagerOptions(dpg_->displayPanel( ),options_box);
		options_box->setLayout(new QVBoxLayout(options_box));
		options_box->setWidget(options);
		QBoxLayout *layout = dynamic_cast<QBoxLayout*>(link_frame->layout( ));
		if ( layout ) layout->setDirection(QBoxLayout::BottomToTop);
		link_frame->show( );
	}

	void QtCanvasManager::showEvent( QShowEvent *e ) {
		QDialog::showEvent(e);

		// Save state of current widgets... and reset widgets to original state...
		std::map<std::string,bool> current_state;
		for ( std::list<CursorLink*>::iterator iter=link_widgets.begin( ); iter !=  link_widgets.end( ); ++iter ) {
			// only keep state for widgets which are visible...
			if ( (*iter)->isVisible( ) == false ) break;
			(*iter)->hide( );
			(*iter)->setColor(QColor(Qt::black));
			current_state.insert( std::map<std::string,bool>::value_type((*iter)->name( ),(*iter)->isChecked( )) );
		}

		// Configure widgets...
		std::list<CursorLink*>::iterator old_links = link_widgets.begin( );
		std::list<QtDisplayPanelGui*> dpgs = dpg_->viewer( )->openDPs( );
		for ( std::list<QtDisplayPanelGui*>::iterator iter=dpgs.begin( ); iter != dpgs.end( ); ++iter ) {
			if ( *iter == dpg_ ) continue;
			std::string txt = (*iter)->id( );
			CursorLink *cur = 0;
			if ( old_links == link_widgets.end( ) ) {
				link_widgets.push_front( (cur = new CursorLink(txt,this)) );
				old_links = link_widgets.end( );
				link_frame->layout( )->addWidget(cur);
			} else {
				cur = *old_links++;
				cur->setText(txt);
				cur->show( );
			}
			if ( cur ) {
				cur->setDPG(*iter);
				std::map<std::string,bool>::const_iterator state = current_state.find(txt);
				cur->setChecked( state == current_state.end( ) ? false : state->second ) ;
			}
		}
	}

	CursorLink::CursorLink( const std::string &name, QtCanvasManager *m, QWidget *parent ) : QFrame(parent), mgr(m),
		name_(name), current_color(Qt::black) {
		setupUi(this);
		link->setText(QString::fromStdString(name_));
		connect( color, SIGNAL(clicked()), this, SLOT(setColor( )) );
		connect( link, SIGNAL(stateChanged(int)), SLOT(linkChange(int)) );
	}

	void CursorLink::setColor( QColor new_color ) {
		if ( new_color.isValid( ) ) {
			current_color = new_color;
			QPalette palette = color->palette( );
			palette.setColor(QPalette::ButtonText,current_color);
			color->setPalette(palette);
			if ( link->checkState( ) != Qt::Unchecked ) {
				// update color...
				dpg->linkCursorTracking(mgr->dpg( ),current_color);
			}
		}
	}
	void CursorLink::setColor( ) {
		setColor(QColorDialog::getColor(current_color, this));
	}

	void CursorLink::linkChange( int state ) {
		if ( state == Qt::Unchecked )
			dpg->unlinkCursorTracking(mgr->dpg( ));
		else
			dpg->linkCursorTracking(mgr->dpg( ),current_color);
	}

}
