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
#include "ImageManagerDialog.qo.h"
#include <display/QtViewer/ImageView.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/ImageScroll.qo.h>


namespace casa {

ImageManagerDialog::ImageManagerDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	initializeScrollArea( ui.openHolder, openScroll );
	initializeScrollArea( ui.displayedHolder, displayedScroll );

	//For testing add a few images
	addImage( NULL );
	addImage( NULL );
	addImage( NULL );

	connect( ui.openToRegisteredButton, SIGNAL(clicked()), this, SLOT(openToDisplayed()));
	connect( ui.registerToOpenButton, SIGNAL(clicked()), this, SLOT(displayedToOpen()));
	connect( ui.closeImageButton, SIGNAL(clicked()), this, SLOT(closeImage()));
}

void ImageManagerDialog::initializeScrollArea( QWidget* holder, ImageScroll*& scrollArea ){
	scrollArea = new ImageScroll( this );
	QHBoxLayout* holderLayout = new QHBoxLayout();
	holderLayout->addWidget( scrollArea );
	holder->setLayout( holderLayout );
}


void ImageManagerDialog::openToDisplayed(){
	QList<ImageView*> movedImages = openScroll->getSelectedViews();
	openScroll->removeImageViews( movedImages );
	displayedScroll->addImageViews( movedImages );
}

void ImageManagerDialog::displayedToOpen(){
	QList<ImageView*> movedImages = displayedScroll->getSelectedViews();
	displayedScroll->removeImageViews( movedImages );
	openScroll->addImageViews( movedImages );
}

void ImageManagerDialog::addImage( QtDisplayData* displayData ){
	if ( displayData != NULL ){
		QString imageName = displayData->name().c_str();
		int imageIndex = ui.masterImageComboBox->findText( imageName );
		if ( imageIndex == -1 ){
			openScroll->addImage( displayData );
			ui.masterImageComboBox->addItem( imageName );
		}
	}
	//**************************************************************
	//         Testing Only - Take Out Later
	//**************************************************************
	else {
		openScroll->addImage( NULL );
	}
}

void ImageManagerDialog::closeImage(){
	openScroll->closeImages();
	displayedScroll->closeImages();
}

ImageManagerDialog::~ImageManagerDialog()
{

}
}
