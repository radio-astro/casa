//# QtExportBrowser.cc: Qt implementation of viewer data manager widget.
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


#include <display/QtViewer/QtExportBrowser.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QDirModel>
#include <QMessageBox>
#include <QPushButton>
#include <graphics/X11/X_exit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	QtExportBrowser::QtExportBrowser(QWidget *parent, QString filePath) :
		QDialog(parent) {
		setupUi(this);

		setExportFilePath_(filePath);
		buildDirTree();

		// Note: The hidden "apply" button is the scapegoat to get
		//       'return' signals from the lineEditor after 'returnPressed()'
		//       of this class has been passed and the input was NOT accepted.
		//       This way the widget stays open and the user can provide a
		//       correct input. Seems to be complicated but I (MK) could not
		//       find a better way.
		QPushButton *applyBtn = buttonBox->button(QDialogButtonBox::Apply);
		if (applyBtn) {
			applyBtn->setAutoDefault(true);
			applyBtn->setDefault(true);
			applyBtn->hide();
		}
		//QPushButton *saveBtn = buttonBox->button(QDialogButtonBox::Save);
		//if (saveBtn){saveBtn->setAutoDefault(false); saveBtn->setDefault(false);}
		//QPushButton *closeBtn = buttonBox->button(QDialogButtonBox::Close);
		//if (closeBtn){closeBtn->setAutoDefault(false); closeBtn->setDefault(false);}

		connect(treeWidget_,    SIGNAL(itemClicked(QTreeWidgetItem*,int)),
		        SLOT(clickItem(QTreeWidgetItem*)));
		connect(treeWidget_,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		        SLOT(doubleClickItem(QTreeWidgetItem*)));
		connect(lineEdit_,   SIGNAL(returnPressed()),
		        SLOT(returnPressed()));
	}


	QtExportBrowser::~QtExportBrowser() {
	}
	void QtExportBrowser::buildDirTree() {

		// make the tree header
		treeWidget_->clear();
		QStringList lbl;
		lbl << "Name" << "Type";
		treeWidget_->setColumnCount(2);
		treeWidget_->setHeaderLabels(lbl);

		QTreeWidgetItem *dirItem;

		// create and add an item for the home directory
		dirItem = new QTreeWidgetItem();
		dirItem->setIcon(0, QIcon(":/icons/home_folder.png"));
		dirItem->setToolTip(0, QString("Home directory"));
		dirItem->setText(1, "Directory");
		//dirItem->setTextColor(1, getDirColor(dType));
		treeWidget_->insertTopLevelItem (0, dirItem );

		// create and add an item for the root directory
		dirItem = new QTreeWidgetItem();
		dirItem->setIcon(0, QIcon(":/icons/root_folder.png"));
		dirItem->setToolTip(0, QString("Root directory"));
		dirItem->setText(1, "Directory");
		//dirItem->setTextColor(1, getDirColor(dType));
		treeWidget_->insertTopLevelItem (1, dirItem );

		// list the content of dir
		// go over each item
		exportDir_.makeAbsolute();
		QStringList entryList = exportDir_.entryList();
		for (int i = 0; i < entryList.size(); i++) {

			QString it = entryList.at(i);
			if (it.compare(".") > 0) {

				/// generate new item
				dirItem = new QTreeWidgetItem(treeWidget_);
				dirItem->setText(0, it);

				// put the full path into the tooltip;
				// just differentiate between "Directories" and "Files"
				QFileInfo fileInfo = QFileInfo(exportDir_.path() + "/" +  entryList.at(i));
				if (fileInfo.isDir())
					dirItem->setText(1, "Directory");
				else
					dirItem->setText(1, "File");
			}
		}

		// resize the tree widget
		treeWidget_->resizeColumnToContents(0);
	}

	void QtExportBrowser::doubleClickItem(QTreeWidgetItem* item) {
		// make sure a directory was clicked
		if(item!=0 && item->text(1)=="Directory") {

			// correct the 'clickItem' action
			if (buffExpFile_.size()>0) {
				exportFile_=buffExpFile_;
				buffExpFile_.clear();
			}

			// get the text
			QString iText = item->text(0);

			// if there is a text go to that directory
			if (iText.size()>0) {
				updateDirectory(item->text(0));
			}
			// if there is no text
			else if (iText.size()==0) {

				// get the top-level index
				int index = treeWidget_->indexOfTopLevelItem (item );

				// go to "home" or "root"
				if (index == 0)
					updateDirectory(QDir::homePath());
				else if (index == 1)
					updateDirectory(QDir::rootPath());
			}
		}
	}

	void QtExportBrowser::clickItem(QTreeWidgetItem* item) {

		// check whether the item is usable
		if(item!=0 && item->text(0).size()>0) {

			// save a directory name
			if (item->text(1) =="Directory")
				buffExpFile_=exportFile_;

			// save the marked item
			exportFile_=item->text(0);
		}

		// put the new path in the editor
		lineEdit_->setText(exportDir_.path()+"/"+exportFile_);
	}

	void QtExportBrowser::updateDirectory(QString newDir) {
		// try to go to the new dir;
		// if not possible, message an go back to old dir
		QDir saved = exportDir_;
		if(!exportDir_.cd(newDir)) {
			QMessageBox::warning(this, tr("QtExportBrowser"),
			                     tr("No such directory:\n %1").arg(newDir));
			exportDir_ = saved;
		}

		// update the treewidget
		buildDirTree();

		// refresh the line editor
		exportDir_.makeAbsolute();
		lineEdit_->setText(exportDir_.path()+"/"+exportFile_);
	}

	void QtExportBrowser::returnPressed() {

		// get the save path and cut a trailing "/"
		QString str = lineEdit_->text();
		while (str.endsWith("/") && str.size()>1)
			str.chop(1);

		// check the directory exists, saving
		// thus would be possible
		QFileInfo fileInfo(str);
		QDir dir = QDir(fileInfo.path());
		if (!dir.exists()) {
			// go back to what existed
			// give a message and return
			exportFile_ = fileInfo.fileName();
			lineEdit_->setText(exportDir_.path()+"/"+exportFile_);
			QString msg = "Directory: " + dir.path()  + " does not exist!\nCan NOT save " + fileInfo.fileName();
			QMessageBox qMsg;
			qMsg.setText(msg);
			qMsg.exec();
			return;
		} else {
			// store path and dir, exit with "accept"
			exportFile_ = fileInfo.fileName();
			exportDir_ =  dir;
			lineEdit_->setText(exportDir_.path()+"/"+exportFile_);
			done(QDialog::Accepted);
		}
	}

	void QtExportBrowser::setExportFilePath_(QString filePath) {
		// chop trailing "/"
		while (filePath.endsWith("/") && filePath.size()>1)
			filePath.chop(1);

		// split the path to dir and file
		QFileInfo fileInfo(filePath);
		exportFile_ = fileInfo.fileName();
		exportDir_ =  QDir(fileInfo.path());

		// if the dir does not exist, take $HOME
		if (!exportDir_.exists())
			exportDir_ = QDir::homePath ();

		// set dir+file in the editor
		lineEdit_->setText(exportDir_.path()+"/"+exportFile_);
	}
} //# NAMESPACE CASA - END
