//# QtDataManager.qo.h: Qt implementation of viewer data manager widget.
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

#ifndef QTEXPORTBROWSER_H_
#define QTEXPORTBROWSER_H_
// (Trailing underscore is not a typo -- do not remove it;
// QtExportBrowser.ui.h uses the QTEXPORTBROWSER_H symbol).

#include <graphics/X11/X_enter.h>
#include <QString>
#include <QDir>

//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#include <display/QtViewer/QtExportBrowser.ui.h>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	class QtExportBrowser: public QDialog, private Ui::QtExportBrowser {

		Q_OBJECT

	public:

		QtExportBrowser(QWidget* parent=0, QString filepath="");
		~QtExportBrowser();

	public:
		QString exportFilePath(void) {
			return lineEdit_->text();
		};
		void updateDirectory(const QString);


	protected slots:
		void buildDirTree();
		void doubleClickItem(QTreeWidgetItem* item);
		void clickItem(QTreeWidgetItem* item);
		void returnPressed();

	private:
		void setExportFilePath_(QString filePath);

		QString exportFile_;
		QString buffExpFile_;
		QDir exportDir_;
	};
} //# NAMESPACE CASA - END

#endif
