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
#include "ImageLoader.qo.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>

namespace casa {

ImageLoader::ImageLoader(QWidget *parent)
    : QDialog(parent), saveOutput( true ){

	ui.setupUi(this);
	this->setWindowTitle( "Histogram Image Loader");

	//Initialize the file browsing tree
	fileModel = new QFileSystemModel( ui.treeWidget );
	QString initialDir = QDir::currentPath();
	ui.treeWidget->setModel( fileModel );
	QString rootDir = QDir::rootPath();
	fileModel->setRootPath(rootDir );
	QModelIndex initialIndex = fileModel->index( initialDir );
	ui.treeWidget->setCurrentIndex( initialIndex );
	ui.treeWidget->setColumnHidden( 1, true );
	ui.treeWidget->setColumnHidden( 2, true );
	ui.treeWidget->setColumnHidden( 3, true );
	ui.directoryLineEdit->setText( initialDir );
	connect( ui.treeWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(directoryChanged(const QModelIndex&)));
	connect( ui.directoryLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validateDirectory(const QString&)));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(fileChanged()));
	connect( ui.cancelButton, SIGNAL( clicked()), this, SLOT(fileReset()));

	//TAKE OUT
	//imageFile = "/home/uniblab/casa/active/test/Antennae_NorthMoments.image";
}

void ImageLoader::keyPressEvent( QKeyEvent* event ){
	int keyCode = event->key();
	//This was written here because pressing a return on a line edit inside
	//the dialog was closing the dialog.
	if ( keyCode != Qt::Key_Return ){
		QDialog::keyPressEvent( event );
	}
}


void ImageLoader::validateDirectory( const QString& str ){
	QFile file( str );
	bool valid = file.exists();
	if ( valid ){
		QAbstractItemModel* model = ui.treeWidget->model();
		QFileSystemModel* fileModel = dynamic_cast<QFileSystemModel*>(model);
		QModelIndex pathIndex = fileModel->index( str );
		ui.treeWidget->setCurrentIndex( pathIndex );
	}
}

QString ImageLoader::getFilePath() const {
	return imageFile;
}

void ImageLoader::directoryChanged(const QModelIndex& modelIndex ){
	QString path = fileModel->filePath( modelIndex );
	//ui.treeWidget->setCurrentIndex( modelIndex );
	ui.directoryLineEdit->setText( path );
}

void ImageLoader::fileChanged( ){
	QModelIndex currentIndex = ui.treeWidget->currentIndex();
	bool validPath = currentIndex.isValid();
	if ( validPath ){
		imageFile = fileModel->filePath( currentIndex );
		QFileInfo fileInfo( imageFile );
		if ( fileInfo.isDir() ){
			emit imageFileChanged();
			this->close();
		}
		else {
			QMessageBox::warning( this, "Invalid File", "Please check that the file is a valid image.");
		}
	}
	else {
		QMessageBox::warning( this, "Invalid File", "Please check that a valid file was selected." );
	}
}

void ImageLoader::fileReset(){
	this->close();
}

ImageLoader::~ImageLoader(){
}
}
