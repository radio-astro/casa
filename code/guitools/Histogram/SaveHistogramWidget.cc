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
#include "SaveHistogramWidget.qo.h"
#include <QFileSystemModel>

#include <QMessageBox>
#include <QDebug>

namespace casa {

const QString SaveHistogramWidget::SAVE_ASCII ="ascii";
const QString SaveHistogramWidget::SAVE_PNG ="png";

SaveHistogramWidget::SaveHistogramWidget(QWidget *parent)
    : QDialog(parent){

	ui.setupUi(this);

	//Initialize the save type combo box
	ui.suffixComboBox->addItem( SAVE_PNG );
	ui.suffixComboBox->addItem( SAVE_ASCII );

	//Initialize the file browsing tree
	fileModel = new QFileSystemModel( ui.directoryTreeWidget );
	QString initialDir = QDir::currentPath();
	ui.directoryTreeWidget->setModel( fileModel );
	QString rootDir = QDir::rootPath();
	fileModel->setRootPath(rootDir );
	QModelIndex initialIndex = fileModel->index( initialDir );
	ui.directoryTreeWidget->setCurrentIndex( initialIndex );
	ui.directoryTreeWidget->setColumnHidden( 1, true );
	ui.directoryTreeWidget->setColumnHidden( 2, true );
	ui.directoryTreeWidget->setColumnHidden( 3, true );
	ui.directoryLineEdit->setText( initialDir );
	connect( ui.directoryTreeWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(directoryChanged(const QModelIndex&)));
	connect( ui.directoryLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validateDirectory(const QString&)));

	connect( ui.saveButton, SIGNAL(clicked()), this, SLOT(save()));
	connect( ui.cancelButton, SIGNAL( clicked()), this, SLOT(saveCanceled()));
}

void SaveHistogramWidget::saveCanceled(){
	close();
}

void SaveHistogramWidget::save(){

	//Directory
	QString saveFileName = ui.directoryLineEdit->text();
	QString directorySeparator = QDir::separator();
	if ( saveFileName.endsWith( directorySeparator )){
		saveFileName.append( directorySeparator );
	}

	//File
	QString fileName = ui.nameLineEdit->text();
	if ( fileName.length() == 0 ){
		QMessageBox::warning(this,"Missing File Name", "Please specify a name for the file.");
		return;
	}
	int dotIndex = fileName.indexOf( ".");
	if ( dotIndex > 0 ){
		fileName = fileName.left( dotIndex );
	}
	saveFileName = saveFileName + fileName;

	//Suffix
	QString suffix = ui.suffixComboBox->currentText();
	saveFileName = saveFileName + "." + suffix;
	if ( suffix == SAVE_PNG ){
		emit savePing( saveFileName );
	}
	else if ( suffix == SAVE_ASCII ){
		emit saveAscii( saveFileName );
	}
	else {
		qWarning()<<"Unsupported save file type" << suffix;
	}
	close();
}

void SaveHistogramWidget::validateDirectory( const QString& str ){
	QFile file( str );
	bool valid = file.exists();
	if ( valid ){
		QAbstractItemModel* model = ui.directoryTreeWidget->model();
		QFileSystemModel* fileModel = dynamic_cast<QFileSystemModel*>(model);
		QModelIndex pathIndex = fileModel->index( str );
		ui.directoryTreeWidget->setCurrentIndex( pathIndex );
	}
}

void SaveHistogramWidget::directoryChanged(const QModelIndex& modelIndex ){
	QString path = fileModel->filePath( modelIndex );
	//ui.treeWidget->setCurrentIndex( modelIndex );
	ui.directoryLineEdit->setText( path );
}



SaveHistogramWidget::~SaveHistogramWidget(){

}
}
