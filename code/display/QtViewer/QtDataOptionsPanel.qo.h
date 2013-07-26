//# QtDataOptionsPanel.qo.h: Qt implementation of viewer data manager widget.
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

#ifndef QTDATAOPTIONSPANEL_H_
#define QTDATAOPTIONSPANEL_H_

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QWidget>
#  include <QResizeEvent>
//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include <display/QtViewer/QtDataOptionsPanel.ui.h>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	class QtDisplayPanelGui;
	class QtDisplayData;

	class QtDataOptionsPanel : public QWidget, protected Ui::QtDataOptionsPanel {

		Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.


	public:

		QtDataOptionsPanel(QtDisplayPanelGui* panel=0, QWidget* parent=0 );
		~QtDataOptionsPanel();
		void removeDD( QtDisplayData* data ){
					removeDDTab_( data );
				}

	signals:
		void setAutoApply(bool);
		void globalColorSettingsChanged( bool );
		void dataOptionsTabChanged( const QString& name );

	protected:

		//void paintEvent(QPaintEvent* event);	//#dk (hye-type trick)

		void resizeEvent (QResizeEvent* ev);		//#diag -- to show size

		QWidget *parent_;
		QtDisplayPanelGui *panel_;

	protected slots:

		// These respond to DD creation/removal signals from viewer_
		// <group>
		virtual void createDDTab_(QtDisplayData*, Bool autoRegister, int insertPosition);
		virtual void removeDDTab_(QtDisplayData*);
		// </group>
		virtual void auto_apply_state_change(bool);

	private slots:
		void tabChanged( int index );

	private:

		QtDataOptionsPanel() {  }	// (not intended for use).

	};

} //# NAMESPACE CASA - END

#endif
