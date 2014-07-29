//# QtLEL.qo.h: Helper class for viewer data manager widget (QtDataManager).
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

#ifndef QTLELEDIT_H
#define QTLELEDIT_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QLineEdit>
#  include <QColor>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN


	class QtLELEdit : public QLineEdit {
		//# Used within QtDataManager's designer-created gui (QtDataManager.ui[.h]).
		//# A QLineEdit which also signals whenever it gets focus.

		Q_OBJECT


	public:

		QtLELEdit(QWidget* parent=0) : QLineEdit(parent) {

			inactivePalette_ = activePalette_ = palette();

			//# The inactive pallette shows text in the 'disabled' color, even
			//# if the edit widget is not really disabled.  This is to provide
			//# feedback that the LEL expression is / is not being used.

			QColor dsbldText = palette().color(QPalette::Disabled, QPalette::Text);
			inactivePalette_.setColor(QPalette::Active, QPalette::Text, dsbldText);
			inactivePalette_.setColor(QPalette::Inactive, QPalette::Text, dsbldText);

			deactivate();
		}


		Bool isActive() {
			return active_;
		}


	signals:

		void gotFocus(QFocusEvent* ev);


	public slots:

		virtual void  activate() {
			setPalette(activePalette_);
			active_ = True;
		}

		virtual void deactivate() {
			setPalette(inactivePalette_);
			active_ = False;
		}



	protected slots:

		virtual void focusInEvent(QFocusEvent* ev) {
			QLineEdit::focusInEvent(ev);
			activate();
			emit gotFocus(ev);
		}


	protected:

		QPalette activePalette_;
		QPalette inactivePalette_;
		Bool active_;

	};



} //# NAMESPACE CASA - END

#endif
