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
#include "ImageScroll.qo.h"
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/DisplayDataHolder.h>

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QSpacerItem>

namespace casa {

ImageScroll::ImageScroll(QWidget *parent)
    : QWidget(parent),
      LAYOUT_SPACING(5), LAYOUT_MARGIN(5),
      managedImages(NULL){

	QVBoxLayout* scrollLayout = new QVBoxLayout();
	scrollLayout->setSpacing( LAYOUT_SPACING );
	scrollLayout->setContentsMargins(LAYOUT_MARGIN,LAYOUT_MARGIN,LAYOUT_MARGIN,LAYOUT_MARGIN);
	spacer = new QSpacerItem( 0,900);
	setLayout( scrollLayout );
	setAutoFillBackground( true );
	QPalette pal = palette();
	pal.setColor( QPalette::Background, Qt::white );
	setPalette( pal );

	setAcceptDrops( true );
	imageColorsEnabled = false;
}

void ImageScroll::setImageHolder( DisplayDataHolder* holder ){
	managedImages = holder;
	managedImages->setImageTracker( this );
	for ( DisplayDataHolder::DisplayDataIterator iter = managedImages->beginDD();
			iter != managedImages->endDD(); iter++ ){
		ImageView* view = new ImageView( *iter, this );
		addImage( view );
	}
}

void ImageScroll::setControllingDD( QtDisplayData* dd ){
	if ( managedImages != NULL ){
		managedImages->setDDControlling( dd );
	}
}

//**************************************************************
//         Gui Methods for manipulating images
//**************************************************************

QList<ImageView*> ImageScroll::getSelectedViews() {
	QList<ImageView*>::iterator iter = images.begin();
	QList<ImageView*> selectedImages;
	while ( iter != images.end() ){
		if ( (*iter)->isImageSelected()){
			selectedImages.append( *iter );
		}
		iter++;
	}
	return selectedImages;
}


void ImageScroll::removeImageViews( QList<ImageView*>& views ){
	QList<ImageView*>::iterator iter = views.begin();
	while ( iter != views.end()){
		closeImage( *iter, false );
		iter++;
	}
}

void ImageScroll::closeImages(){
	QList<ImageView*> closeImages = getSelectedViews();
	while ( !closeImages.isEmpty()){
		ImageView* imageView = closeImages.takeLast();
		closeImage( imageView );
	}
}


void ImageScroll::closeImage( ImageView* imageView, bool deleteImage ){
	QLayout* scrollLayout = layout();
	scrollLayout->removeWidget( imageView );
	images.removeOne( imageView );
	QtDisplayData* dd = imageView->getData();
	managedImages->discardDD( dd, false );
	disconnect( imageView, SIGNAL(displayTypeChanged(ImageView*)),
			this, SLOT(displayTypeChanged(ImageView*)));
	if ( deleteImage ){
		delete imageView;
	}
}

void ImageScroll::addImageViews(QList<ImageView*>& imageViews){
	QLayout* scrollLayout = layout();
	scrollLayout->removeItem( spacer );
	QList<ImageView*>::iterator iter = imageViews.begin();
	while ( iter != imageViews.end()){
		images.append( *iter );
		scrollLayout->addWidget( *iter);
		QtDisplayData* dd = (*iter)->getData();
		managedImages->insertDD( dd, -1 );
		iter++;
	}
	scrollLayout->addItem( spacer );
}

void ImageScroll::setSelectAll( bool select ){
	QList<ImageView*>::iterator iter = images.begin();
	while ( iter != images.end()){
		(*iter)->setImageSelected( select );
		iter++;
	}
}

void ImageScroll::setImageColorsEnabled( bool enabled ){
	imageColorsEnabled = enabled;
	QList<ImageView*>::iterator iter = images.begin();
	while ( iter != images.end()){
		(*iter)->setImageColorsEnabled( enabled );
		iter++;
	}
}

void ImageScroll::applyColorChangesIndividually(){
	for ( QList<ImageView*>::iterator iter = images.begin(); iter != images.end(); iter++ ){
		(*iter)->emitDisplayColorsChanged();
	}
}

//**************************************************************************
//                Drag and Drop
//**************************************************************************

void ImageScroll::dropEvent( QDropEvent* dropEvent ){

	//Use the position to estimate where int the list the imageView
	//should be placed.
	QPoint dropPosition = dropEvent->pos();
	int dropY = dropPosition.y();

	//Since some images may be open others not so we don't have
	//a uniform width.  Have to go through and ask each one what their
	//size is.
	int dropIndex = -1;
	int height = LAYOUT_MARGIN;
	int i = 0;
	QList<ImageView*>::iterator iter = images.begin();
	while ( iter != images.end() ){
		if (dropY < height ){
			dropIndex = i;
			break;
		}
		QSize imageSize = (*iter)->size();
		height = height + imageSize.height() + 2*LAYOUT_SPACING;
		iter++;
		i++;
	}
	if ( dropIndex == -1 ){
		if ( dropY < height){
			dropIndex = images.size();
		}
	}

	if ( dropIndex >= 0 ){
		const QMimeData* dropData = dropEvent->mimeData();
		ImageView* droppedView = getMimeImageView( dropData );
		if ( droppedView != NULL ){
			int droppedImageIndex = images.indexOf( droppedView );
			if ( droppedImageIndex < dropIndex && dropIndex != 0 ){
				dropIndex = dropIndex - 1;
			}
			QLayout* scrollLayout = layout();
			scrollLayout->removeWidget( droppedView );
			QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(scrollLayout);
			boxLayout->insertWidget(dropIndex, droppedView );

			//Update the order of the data list.
			QtDisplayData* displayData = droppedView->getData();
			managedImages->discardDD( displayData, false );
			managedImages->insertDD( displayData, dropIndex );
		}
	}
}


ImageView* ImageScroll::getMimeImageView( const QMimeData* mimeData ){
	//Find the image view that was dropped.
	QString targetName = mimeData->text();
	ImageView* droppedView = findImageView( targetName );
	return droppedView;
}

void ImageScroll::dragMoveEvent( QDragMoveEvent* dragMoveEvent ){
	dragMoveEvent->accept();
}

void ImageScroll::dragEnterEvent( QDragEnterEvent* enterEvent ){
	const QMimeData* mimeData = enterEvent->mimeData();
	ImageView* draggedImageView = getMimeImageView( mimeData );
	if ( draggedImageView != NULL ){
		enterEvent->acceptProposedAction();
	}
}

//---------------------------------------------------------------------
//                    Image Tracker Interface
//---------------------------------------------------------------------
void ImageScroll::masterImageSelected( QtDisplayData* /*image*/ ){

}
void ImageScroll::imageAdded( QtDisplayData* image ){
	ImageView* view = new ImageView( image, this );
	view->setImageColorsEnabled( imageColorsEnabled );
	addImage( view );
	emit displayDataAdded( image );
}

bool ImageScroll::isManaged( QtDisplayData* displayData ) const {
	return managedImages->exists( displayData );
}

void ImageScroll::imageRemoved( QtDisplayData* image ){
	ImageView* deleteView = findImageView( image->name().c_str() );
	if ( deleteView != NULL ){
		closeImage( deleteView );
		emit displayDataRemoved( image );
	}
}

//-------------------------------------------------------------------------
//                  Utility
//-------------------------------------------------------------------------

void ImageScroll::addImage( ImageView* viewerImage ){
	images.append( viewerImage );
	connect( viewerImage, SIGNAL( displayTypeChanged( ImageView* )),
				this, SIGNAL(displayTypeChanged(ImageView*)));
	connect( viewerImage, SIGNAL( displayColorsChanged( ImageView*)),
				this, SIGNAL(displayColorsChanged(ImageView*)));
	QLayout* scrollLayout = layout();
	scrollLayout->removeItem( spacer );
	scrollLayout->addWidget( viewerImage );
	scrollLayout->addItem( spacer );
}


ImageView* ImageScroll::findImageView( const QString& targetName ){
	ImageView* targetView = NULL;
	QList<ImageView*>::iterator iter = images.begin();
	while ( iter != images.end() ){
		QString viewName = (*iter)->getName();
		if ( viewName == targetName ){
			targetView =  *iter;
			break;
		}
		iter++;
	}
	return targetView;
}


ImageScroll::~ImageScroll(){
	while( images.isEmpty()){
		ImageView* viewerImage = images.takeLast();
		delete viewerImage;
	}
	delete managedImages;
	delete spacer;
}
}
