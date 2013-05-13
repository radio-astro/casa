//# Copyright (C) 2005,2013
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

#ifndef QTCanvasManager_H
#define QTCanvasManager_H

#include <casa/aips.h>
#include <display/QtAutoGui/QtAutoGui.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtCanvasManager.ui.h>
#include <display/QtViewer/CursorLink.ui.h>
#include <casa/Containers/Record.h>

#include <graphics/X11/X_enter.h>
/* #  include <QDir> */
/* #  include <QColor> */
/* #  include <QHash> */
#include <QFrame>
#include <QSpacerItem>
#include <graphics/X11/X_exit.h>


namespace casa {

	class QtViewer;
	class QtCanvasManager;

// <summary>
// Options widget for single DisplayPanel.
// </summary>

// <synopsis>
// </synopsis>

	class QtCanvasManagerOptions : public QtAutoGui {
		Q_OBJECT
	public:
		QtCanvasManagerOptions( QtDisplayPanel* qdp, QWidget *parent ) : QtAutoGui(parent) {
			loadRecord(qdp->getOptions());
			connect( this, SIGNAL(setOptions(Record)), qdp, SLOT(setOptions(Record)) );
			connect( qdp, SIGNAL(optionsChanged(Record)), SLOT(changeOptions(Record)) );
		}
		~QtCanvasManagerOptions() {  }
	};

	class CursorLink : public QFrame, protected Ui::CursorLink {
		Q_OBJECT
	public:
		CursorLink( const std::string &name, QtCanvasManager *mgr, QWidget *parent=0 );
		std::string name( ) const {
			return name_;
		}
		bool isChecked( ) const {
			return link->checkState( ) == Qt::Unchecked ? false : true;
		}
		void setChecked( bool val ) {
			link->setCheckState( val ? Qt::Checked : Qt::Unchecked );
		}
		void setText( const std::string &txt ) {
			link->setText(QString::fromStdString(txt));
		}
		void setDPG( QtDisplayPanelGui *g ) {
			dpg = g;
		}
		QtDisplayPanelGui *getDPG( ) {
			return dpg;
		}
		void setColor( QColor c );

	private slots:
		void setColor( );
		void linkChange(int);

	private:
		QtCanvasManager *mgr;
		QtDisplayPanelGui *dpg;
		std::string name_;
		QColor current_color;
	};

	class QtCanvasManager : public QDialog, protected Ui::QtCanvasManager {
		Q_OBJECT
	public:

		QtCanvasManager(QtDisplayPanelGui *dpg);
		~QtCanvasManager( ) { }
		QtDisplayPanelGui *dpg( ) {
			return dpg_;
		}

	protected:

		void showEvent( QShowEvent* );
		QtCanvasManagerOptions *options;
		QtDisplayPanelGui *dpg_;
	private:
		QVBoxLayout *layout;
		QSpacerItem *spacer;
		std::list<CursorLink*> link_widgets;

	};

}

#endif

