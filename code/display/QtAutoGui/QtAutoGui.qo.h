//# QtAutoGui.qo.h: General-purpose Qt options panel widget created from
//#              an options Record.
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

#ifndef QTAUTOGUI_H
#define QTAUTOGUI_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QtCore>
#  include <QtGui>
#  include <QtXml/QDomDocument>
#include <graphics/X11/X_exit.h>

#include "QtGuiEntry.qo.h"
#include "QtXmlRecord.h"

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// General-purpose Qt options widget created from Record
// </summary>

// <synopsis>
// This class serves the same general purpose as the old glish
// autogui.g: it creates a gui from a record whose fields describe
// 'parameters' (or 'options') to be controlled by the gui.  The record
// used to construct the gui is passed in construction, or soon after,
// via loadRecord().  Each option passed contains an initial value as
// well as meta-data describing the type of input widget to be used
// ('popt' field), labels, limits, choices, help texts, etc., as applicable.
//
// When the user operates the control for a gui option, a setOptions
// signal is emitted with a similar Record indicating which option was
// changed, and new value.  If the 'Apply' button of the gui is pressed,
// the setOptions signal contains the current value of every option.
// </synopsis>

//////////////////////////  QtAutoGui  /////////////////////////
	class QtAutoGui : public QWidget {
		Q_OBJECT

	public:

		QtAutoGui(QWidget *parent = 0);
		QtAutoGui(Record rec, String dataName = "",
		          String dataType = "Unknown", QWidget *parent = 0);
		~QtAutoGui();

		void setFileName(const QString &file_name);
		QString fileName() const;
		void setDataType(const QString &dType);
		QString dataType() const;
		bool load(QDomDocument &doc);
		bool load(); //from file
		bool save();  //to file
		void apply(); //to the canvas
		void restore();
		void setMemory(); //take a snapshot
		void setOriginal(); //restore to the saved
		void setDefault(); //use the preset values
		void dismiss();
		enum ACTIONTYPE {Set, Copy, Paste, Default, Original, Memorize, Command};
		// call once only during initialization, to populate the gui,
		// if you used the QtAutoGui(QWidget=0) constructor.
		void loadRecord(Record rec);
		static const QString HISTOGRAM_SHOW_KEY;
		static const QString HISTOGRAM_RANGE_KEY;

	signals:

		void setOptions(Record options);
		//# dk note to hye:
		//#
		//# This is the signal that needs to be sent whenever a value
		//# is changed on the gui, or when 'apply' is pressed.  When
		//# a gui value change occurs, send only that option; when
		//# 'apply' is pressed, send the current value of all options.
		//# The record format should be as for getOptions, although not
		//# all fields are always sent.  You need only send the 'value'
		//# field of the option[s], although you can send the other fields
		//# too if you want.
		//#
		//# Look at the format of setoptions events sent in the current
		//# viewer; this is the format the C++ display library expects.
		//# An easy way to do this is to copy this diagnostic-version
		//# glish file to the directory where you run aips++:
		//#
		//#  /users/dking/testcode/g/viewerdisplaydata.g
		//#
		//# Then run the aips++ viewer normally there.  Each time you
		//# change an 'adjust' panel option or press its 'apply', it will
		//# print the record emitted by the setoptions event.

	protected:
		void contextMenuEvent(QContextMenuEvent *e);
	private slots:

	public slots:
		void itemValueChanged(QString name, QString value, int action, bool apply);
		//#dk virtual void paintEvent ( QPaintEvent * event );
		void adjustHeight();

		virtual void changeOptions(Record changedOptions);
		//# dk note to hye:
		//#
		//# This slot needs to be implemented to handle changes
		//# requested by the C++ library to current values, allowed data
		//# range('pmin, pmax') and display
		//# label ('listname'), as appropriate, for any existing options.
		//#
		//# It would be useful to allow changes to slider increment
		//# ('presolution'), available choices ('popt') and the default
		//# value as well.  Also, it would be useful to support
		//# (new) boolean subfields 'enabled' and 'visible'.  These
		//# features can be postponed, if necessary, though, and we will not
		//# need to support changes to some of the fields sent in these
		//# records from the library (such as help, editable, dlformat...).
		//#
		//# If you're really ambitious, you could try supporting addition of
		//# new option entries, too.
		//#
		//# the 'modifygui' method in autogui.g is the glish equivalent
		//# of what we're trying to do here, although modifygui itself is
		//# rather incomplete.


		// (called only once, during or soon after construction.  Sets up contents of
		// the gui from rec).

		void autoApplyState( bool auto_apply ) {
			auto_apply_ = auto_apply;
		}

	protected:
		bool auto_apply_;

	private:
		QDomDocument m_doc;
		QString m_file_name;
		QString m_data_type;
		QWidget *contents_;
		QVBoxLayout *contentsLayout_;
		QVBoxLayout *vboxLayout;
		//#dk QFrame *line;
		QSpacerItem *spacerItem;
		//QScrollArea *scrollArea;

		Bool recordLoaded_;
		QString m_lockItem;
		QMutex mutex;
		void initialize();

	};

} //# NAMESPACE CASA - END

#endif
