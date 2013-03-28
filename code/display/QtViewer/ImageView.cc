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
#include "ImageView.qo.h"
#include <casa/BasicSL/String.h>
#include <display/QtViewer/ImageView.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <QUuid>
#include <QDrag>

namespace casa {

ImageView::ImageView(QtDisplayData* data, QWidget *parent)
    : QFrame(parent),
      selectedColor("#C0C0C0"), normalColor("#D3D3D3"),
      imageData( NULL ){

	ui.setupUi(this);

	if ( data != NULL ){
		imageData = data;
	}

	//Selection/Colors
	selected = false;
    setAutoFillBackground( true );
    setBackgroundColor( normalColor );

	QString name;
	if ( imageData == NULL ){
		QUuid uid = QUuid::createUuid();
		name = uid.toString();
	}
	else {
		String imageName = imageData->name();
		name = imageName.c_str();
	}
	ui.imageNameLabel->setText( name );

	//Open close the image view.
	minimizeDisplay();
	connect( ui.openCloseButton, SIGNAL(clicked()), this, SLOT(openCloseDisplay()));
}

bool ImageView::isImageSelected() const {
	return selected;
}

void ImageView::setBackgroundColor( QColor color ){
	QPalette pal = palette();
	pal.setColor( QPalette::Background, color );
	setPalette( pal );
}

void ImageView::setImageSelected( bool selected ) {
	this->selected = selected;
	if ( selected ){
		setFrameShadow( QFrame::Sunken );
		setBackgroundColor( selectedColor );
	}
	else {
		setFrameShadow( QFrame::Raised );
		setBackgroundColor( normalColor );
	}
}

void ImageView::mousePressEvent( QMouseEvent* /*event*/ ){
	qDebug() << "Got a mouse press";
	selected = !selected;
	setImageSelected( selected );
	//emit imageSelected(this);
}

QString ImageView::getName() const {
	QString nameStr = ui.imageNameLabel->text();
	return nameStr;
}

//****************************************************************
//                     Opening and Closing the View
//****************************************************************

void ImageView::openCloseDisplay(){
	if ( minimized ){
		maximizeDisplay();
	}
	else {
		minimizeDisplay();
	}
}

void ImageView::minimizeDisplay(){
	QLayout* imageLayout = this->layout();
	imageLayout->removeWidget( ui.displayGroupBox );
	imageLayout->removeWidget( ui.colorGroupBox );
	ui.displayGroupBox->setParent( NULL );
	ui.colorGroupBox->setParent( NULL );
	QIcon openIcon( ":/icons/imageMaximize.png");
	ui.openCloseButton->setIcon( openIcon );
	minimized = true;
}

void ImageView::maximizeDisplay(){
	QLayout* imageLayout = this->layout();
	imageLayout->addWidget( ui.displayGroupBox );
	imageLayout->addWidget( ui.colorGroupBox );
	QIcon closeIcon( ":/icons/imageMinimize.png");
	ui.openCloseButton->setIcon( closeIcon );
	minimized = false;
}

//**********************************************************************
//                   Drag and Drop
//**********************************************************************

void ImageView::makeDrag(){
	QDrag* drag = new QDrag( this );
	QMimeData* data = new QMimeData();
	data->setText(getName());
	drag->setMimeData( data );
	drag->start();
}

void ImageView::mouseMoveEvent( QMouseEvent* event ){
	if ( selected ){
		makeDrag();
	}
}

ImageView::~ImageView()
{

}
}
