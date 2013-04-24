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
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <QUuid>
#include <QDrag>
#include <QColorDialog>

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
	ui.selectCheckBox->setChecked( selected );
	connect( ui.selectCheckBox, SIGNAL(toggled(bool)),
			this, SLOT(imageSelectionChanged(bool)));
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

	//Display type of the image
	initDisplayType();

	//Coloring the image
	setButtonColor( Qt::gray );
	setImageColorsEnabled( false );
	connect( ui.colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

	//Open close the image view.
	minimizeDisplay();
	connect( ui.openCloseButton, SIGNAL(clicked()), this, SLOT(openCloseDisplay()));
}

void ImageView::initDisplayType(){
	displayGroup = new QButtonGroup( this );
	displayGroup->addButton( ui.contourRadio, DISPLAY_CONTOUR );
	displayGroup->addButton( ui.rasterRadio, DISPLAY_RASTER );
	displayGroup->addButton( ui.vectorRadio, DISPLAY_VECTOR );
	displayGroup->addButton( ui.markerRadio, DISPLAY_MARKER );
	if ( imageData != NULL ){
		if ( imageData->isContour() ){
			ui.contourRadio->setChecked(true);
		}
		else if ( imageData->isRaster() ){
			ui.rasterRadio->setChecked( true );
		}
		else if ( imageData->isMarker()){
			ui.markerRadio->setChecked( true );
		}
		else if ( imageData->isVector()){
			ui.vectorRadio->setChecked( true );
		}
	}
	connect( ui.contourRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
	connect( ui.rasterRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
	connect( ui.vectorRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
	connect( ui.markerRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
}

QtDisplayData* ImageView::getData() const {
	return imageData;
}

void ImageView::setImageSelected( bool selected ){
	ui.selectCheckBox->setChecked( selected );
}

bool ImageView::isImageSelected() const {
	return selected;
}

void ImageView::setImageColorsEnabled( bool enabled ){
	ui.colorButton->setEnabled( enabled );
}

void ImageView::setBackgroundColor( QColor color ){
	QPalette pal = palette();
	pal.setColor( QPalette::Background, color );
	setPalette( pal );
}

void ImageView::setButtonColor( QColor color ){
	QPalette p = ui.colorButton->palette();
	p.setBrush(QPalette::Button, color);
	ui.colorButton->setPalette( p );
}

QColor ImageView::getButtonColor() const {
	QPalette p = ui.colorButton->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void ImageView::showColorDialog(){
	QColor initialColor = getButtonColor();
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( selectedColor );
		emit displayColorsChanged( this );
	}
}

void ImageView::emitDisplayColorsChanged(){
	emit displayColorsChanged( this );
}

ImageView::DisplayType ImageView::getDataDisplayType() const {
	DisplayType ddDisplay = DISPLAY_MARKER;
	if ( imageData->isRaster()){
		ddDisplay = DISPLAY_RASTER;
	}
	else if ( imageData->isContour()){
		ddDisplay = DISPLAY_CONTOUR;
	}
	else if ( imageData->isVector()){
		ddDisplay = DISPLAY_VECTOR;
	}
	return ddDisplay;
}

QString ImageView::getDataDisplayTypeName() const {
	QAbstractButton* button = displayGroup->checkedButton();
	return button->text();
}

void ImageView::setDisplayedColor(QColor color){
	setButtonColor( color );
}

QColor ImageView::getDisplayedColor() const {
	return getButtonColor();
}

void ImageView::displayTypeChanged() {
	//Decide if there has been a change to
	//Raster/Color/Contour/Vector
	DisplayType guiDisplay = static_cast<DisplayType>(displayGroup->checkedId());
	DisplayType dataDisplay = getDataDisplayType();
	if ( guiDisplay != dataDisplay ){
		//Need to change the type.
		emit displayTypeChanged( this );
	}


}

void ImageView::imageSelectionChanged( bool selected ) {
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
	imageLayout->removeItem( ui.colorLayout);

	ui.colorLayout->removeWidget( ui.colorButton );
	ui.colorLayout->removeWidget( ui.colorLabel );
	ui.colorLayout->removeItem( ui.colorSpacer1 );
	ui.colorLayout->removeItem( ui.colorSpacer2 );
	ui.colorLayout->setParent( NULL );
	ui.colorButton->setParent( NULL );
	ui.colorLabel->setParent( NULL );
	//ui.colorSpacer1->setParent( NULL );
	//ui.colorSpacer2->setParent( NULL );
	ui.displayGroupBox->setParent( NULL );

	this->setMaximumHeight( 50 );
	this->setMinimumHeight( 50 );
	QIcon openIcon( ":/icons/imageMaximize.png");
	ui.openCloseButton->setIcon( openIcon );
	minimized = true;
}

void ImageView::maximizeDisplay(){
	QLayout* imageLayout = this->layout();
	imageLayout->addWidget( ui.displayGroupBox );
	ui.colorLayout->addItem( ui.colorSpacer1 );
	ui.colorLayout->addWidget( ui.colorLabel );
	ui.colorLayout->addWidget( ui.colorButton );
	ui.colorLayout->addItem( ui.colorSpacer2 );
	QVBoxLayout* verticalLayout = dynamic_cast<QVBoxLayout*>(imageLayout);
	verticalLayout->addLayout( ui.colorLayout );
	this->setMinimumHeight( 200 );
	this->setMaximumHeight( 500 );
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

void ImageView::mouseMoveEvent( QMouseEvent* /*event*/ ){
	makeDrag();
}

ImageView::~ImageView()
{

}
}
