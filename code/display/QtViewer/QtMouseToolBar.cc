//# QtMouseToolBar.cc: 'mouse-tool' toolbar for qtviewer display panel.
//# Copyright (C) 2005
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
//# $Id$
//#

#include <list>
#include <display/QtViewer/QtMouseToolBar.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>

namespace casa { //# NAMESPACE CASA - BEGIN


	QtMouseToolButton::QtMouseToolButton( const std::string &type, QWidget *parent ) : QToolButton(parent), tool_(type) { }

	void QtMouseToolButton::mousePressEvent( QMouseEvent *event ) {
		Int btn = (event->button() == Qt::LeftButton)?   1 :
		          (event->button() == Qt::MidButton)?    2 :
		          (event->button() == Qt::RightButton)?  3 :   0;

		if(btn!=0) emit mouseToolBtnPress(text().toStdString(), btn);
	}

	std::string QtMouseToolButton::getIconStr(Int button) const {
		using namespace QtMouseToolNames;
		char buf[128];
		sprintf(buf,"%d",button);
		std::string result = std::string(":/icons/") + std::string(iconName(tool_)) + std::string(buf) + std::string(".png");
		return result;
	}


	QtPointToolButton::QtPointToolButton( QWidget *parent ) : QtMouseToolButton(QtMouseToolNames::POINT,parent),
		rc(viewer::getrc( )), timer(new QTimer( )) {
		connect( timer, SIGNAL(timeout( )), SLOT(popup_options_menu( )) );
	}

	void QtPointToolButton::popup_options_menu( ) {
		timer->stop( );
		show_context_menu(popup_menu_pos);
	}

	void QtPointToolButton::mousePressEvent( QMouseEvent *event ) {
		popup_menu_pos = event->globalPos( );
		timer->start( 750 );	// popup menu in 3/4 of a second
		QtMouseToolButton::mousePressEvent(event);
	}

	void QtPointToolButton::mouseReleaseEvent( QMouseEvent *event ) {
		timer->stop( );
		QtMouseToolButton::mouseReleaseEvent(event);
	}

	class PopupMenu : public QMenu {
	public:
		PopupMenu( ) : QMenu( ) { }
	protected:
		void leaveEvent(QEvent*);
	};

	void PopupMenu::leaveEvent( QEvent */*ev*/ ) {
		// pop down the menu upon exit...
		QKeyEvent kev( QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier );
		event(&kev);
	}

	void QtPointToolButton::show_context_menu( const QPoint &global_pos ) {
		using namespace QtMouseToolNames;

		QMenu *options = new PopupMenu( );
		for ( int i = 0; i < SYM_POINT_REGION_COUNT; ++i ) {
			QAction *act = options->addAction(QIcon(QString::fromStdString(pointRegionSymbolIcon((PointRegionSymbols)i))), QString( ));
			act->setData(QVariant(i));
		}

		QAction *selectedItem = options->exec(global_pos);
		if ( selectedItem ) {
			int item = selectedItem->data( ).toInt( );
			if ( item >= 0 && item < SYM_POINT_REGION_COUNT ) {
				emit mouseToolBtnState(POINT,item);
				rc.put("viewer.mouse.state.point",pointRegionSymbolRc((PointRegionSymbols)item));
			}
		}
		delete options;
	}


	QtMouseToolBar::QtMouseToolBar(QtMouseToolState* msbtns, QtDisplayPanel* qdp, QWidget* parent) : QToolBar("Mouse Toolbar", parent), msbtns_(msbtns) {

		// Use a default set of mouse tools, unless specified by a qdp.

		using namespace QtMouseToolNames;
		// Constants (nTools, names, etc.) used by Qt mouse tools.
		// Definitions in QtMouseToolState.cc.

		Vector<String> tools;
		// (Overrides a def. in namespace above: these are the tool names
		// for this particular toolbar, not necessarily all tools).

		if(qdp==0 || qdp->mouseToolNames().nelements()==0) {
			tools.resize(9);
			tools[0]=ZOOM;
			tools[1]=PAN;
			tools[2]=SHIFTSLOPE;
			tools[3]=BRIGHTCONTRAST;
			tools[4]=POINT;
			tools[5]=RECTANGLE;
			tools[6]=ELLIPSE;
			tools[7]=POLYGON;
			tools[8]=POLYLINE;
		} else tools = qdp->mouseToolNames();


		// Create tool buttons ('actions') within the toolbar.

		for(uInt i=0; i<tools.nelements(); i++) {

			std::string tool = tools[i];

			QtMouseToolButton* mtb = ( tool == POINT ? new QtPointToolButton(this) :
			                           new QtMouseToolButton(tool,this));
			buttons_.insert(std::pair<std::string,QtMouseToolButton*>(tool,mtb));
			addWidget(mtb);

			mtb->setObjectName(tool.c_str( ));
			mtb->setText(tool.c_str( ));
			mtb->setToolTip( ( "Click here with the desired mouse button "
			                   "to assign that button to \n\'" + longName(tool) +
			                   "\'\n" + help(tool)) . c_str( ) );

			// Originally, tool buttons are created as unassigned to mouse buttons.
			// This will change via calls to chgMouseBtn_(), even in initialization.

			mtb->setIcon(QIcon( (":/icons/" + iconName(tool) + "0.png").c_str( ) ));
			mtb->setCheckable(True);

			// Pressing a button will order a button assignment change from the
			// central registry.
			connect( mtb,       SIGNAL(mouseToolBtnPress(String, Int)),
			         msbtns_,   SLOT(chgMouseBtn        (String, Int)) );
			connect( mtb,       SIGNAL(mouseToolBtnState(String, Int)),
			         msbtns_,   SLOT(mouseBtnStateChg   (String, Int)) );
		}


		// Keeps this toolbar up-to-date with central button-state registry.
		connect( msbtns_, SIGNAL(mouseBtnChg (std::string, Int)),
		         SLOT(chgMouseBtn_(std::string, Int)) );

		msbtns_->emitBtns();
	}

	QtMouseToolButton *QtMouseToolBar::button( const std::string &name ) {
		std::map<std::string,QtMouseToolButton*>::iterator it = buttons_.find( name );
		if ( it != buttons_.end( ) )
			return it->second;
		return 0;
	}


	void QtMouseToolBar::chgMouseBtn_(std::string tool, Int button) {
		// Connected to the QtMouseToolState::mouseBtnChg() signal.  Changes the
		// tool button's (QAction's) state (icon, whether checked), to reflect
		// the [new] mouse button assignment of a given mouse tool.

		using namespace QtMouseToolNames;

		QToolButton* mtb = findChild<QToolButton*>(tool.c_str( ));
		if(mtb==0) return;	// (shouldn't happen).
		ostringstream os;
		os << button;
		mtb->setIcon(QIcon(QString::fromStdString( ":/icons/" + iconName(tool) + os.str( ) + ".png" )));
		mtb->setChecked(button!=0);
	}

} //# NAMESPACE CASA - END
