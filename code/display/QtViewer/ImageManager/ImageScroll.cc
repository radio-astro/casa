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
#include <display/QtViewer/QtDisplayData.qo.h>

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QSpacerItem>
#include <QFrame>

namespace casa {

	ImageScroll::ImageScroll(QWidget *parent)
		: QWidget(parent),
		  LAYOUT_SPACING(5),
		  LAYOUT_MARGIN(5){

		ui.setupUi(this);
		scrollWidget = new QWidget( ui.scrollArea );
		ui.scrollArea->setWidget( scrollWidget );
		ui.scrollArea->setWidgetResizable( true );
		QVBoxLayout* scrollLayout = new QVBoxLayout();
		scrollLayout->setSpacing( LAYOUT_SPACING );
		scrollLayout->setContentsMargins(LAYOUT_MARGIN,LAYOUT_MARGIN,LAYOUT_MARGIN,LAYOUT_MARGIN);
		scrollWidget->setLayout( scrollLayout );

		spacer = new QSpacerItem( 0,20, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		dropMarker = new QFrame();
		dropMarker->setFrameShape( QFrame::HLine );
		dropMarker->setLineWidth( 5 );
		dropMarker->setStyleSheet("color:white");


		setAutoFillBackground( true );
		QPalette pal = palette();
		QColor bgColor( Qt::black );
		pal.setColor( QPalette::Background, bgColor );
		setPalette( pal );

		setAcceptDrops( true );
	}

	//----------------------------------------------------------------------
	//                      Accessors
	//----------------------------------------------------------------------

	QtDisplayData* ImageScroll::getHueMaster() const {
		QtDisplayData* target = NULL;
		for ( QList<ImageView*>::const_iterator iter = images.begin();
			iter != images.end(); iter++ ) {
			if ( (*iter)->isMasterHue() ){
				target = (*iter)->getData();
				break;
			}
		}
		return target;
	}
	QtDisplayData* ImageScroll::getSaturationMaster() const {
		QtDisplayData* target = NULL;
		for ( QList<ImageView*>::const_iterator iter = images.begin();
			iter != images.end(); iter++ ) {
			if ( (*iter)->isMasterSaturation() ){
				target = (*iter)->getData();
				break;
			}
		}
		return target;
	}

	QtDisplayData* ImageScroll::getCoordinateMaster() const {
		QtDisplayData* target = NULL;
		for ( QList<ImageView*>::const_iterator iter = images.begin();
			iter != images.end(); iter++ ) {
			if ( (*iter)->isMasterCoordinate() ){
				target = (*iter)->getData();
				break;
			}
		}
		return target;
	}

	QList<ImageView*> ImageScroll::getViews() {
		return images;
	}

	int ImageScroll::getImageCount() const {
		return images.size();
	}

	int ImageScroll::getIndex( ImageView* view ) {
		QString viewName = view->getName();
		int orderIndex = findImageView( viewName );
		return orderIndex;
	}

	int ImageScroll::getRegisteredIndex( int dropIndex )const{
		int count = 0;
		int totalCount = 0;
		for( QList<ImageView*>::const_iterator iter = images.begin();
							iter != images.end(); iter++ ) {
			if ( totalCount == dropIndex ){
				break;
			}
			if ( (*iter)->isRegistered() ){
				count++;
			}
			totalCount++;
		}
		return count;
	}


	int ImageScroll::getRegisteredCount() const {
		int count = 0;
		for( QList<ImageView*>::const_iterator iter = images.begin();
				iter != images.end(); iter++ ) {
			if ( (*iter)->isRegistered()){
				count++;
			}
		}
		return count;
	}


	//-------------------------------------------------------------
	//            Opening/Closing/Registering Images
	//-------------------------------------------------------------

	void ImageScroll::closeImages() {
		while ( !images.isEmpty()) {
			int index = images.size() - 1;
			closeImage( images[index], true );
		}
	}

	void ImageScroll::closeImage( ImageView* imageView, bool deleteImage ) {
		QLayout* scrollLayout = scrollWidget->layout();
		scrollLayout->removeWidget( imageView );
		imageView->setParent( NULL );
		bool masterCoordinate = imageView->isMasterCoordinate();
		images.removeOne( imageView );
		disconnect( imageView, SIGNAL(displayTypeChanged(ImageView*)),
			this, SIGNAL(displayTypeChanged(ImageView*)));
		disconnect( imageView, SIGNAL( close(ImageView*)),
			this, SLOT( closeImage(ImageView*)));
		disconnect( imageView, SIGNAL( masterCoordinateImageSelected( ImageView*)),
			this, SLOT( coordinateSystemChanged( ImageView*)));
		disconnect( imageView, SIGNAL( masterCoordinateImageClear()),
			this, SLOT( masterCoordinateClear()));
		disconnect( imageView, SIGNAL( masterHueImageSelected( ImageView*)),
			this, SLOT( hueImageChanged( ImageView*)));
		disconnect( imageView, SIGNAL( masterSaturationImageSelected( ImageView*)),
			this, SLOT( saturationImageChanged( ImageView*)));
		disconnect( imageView, SIGNAL( showDataDisplayOptions( QtDisplayData*)),
			this, SIGNAL( showDataDisplayOptions( QtDisplayData*)));
		disconnect( imageView, SIGNAL( viewImage( ImageView*)),
				this, SLOT( viewImage(ImageView*)));
		QtDisplayData* dd = imageView->getData();
		emit displayDataRemoved( dd, masterCoordinate );
		if ( deleteImage ) {
			delete imageView;
		}
	}

	void ImageScroll::addImageView( QtDisplayData* data, bool registered,
			ImageView::ColorCombinationMode mode,
			int dropIndex, bool masterCoordinate,
			bool masterSaturation, bool masterHue, QColor rgbColor ){
		ImageView* view = NULL;
		String dataName = data->name();
		int index = findImageView( dataName.c_str(), false );
		if ( index >= 0 ){
			view = images[index];
			view->setData( data );
		}
		if ( view == NULL ){
			view = new ImageView( data );
			view->setRegistered( registered );
			view->setColorCombinationMode( mode );
			if ( masterCoordinate && view->isControlEligible()){
				view->setMasterCoordinateImage( true );
			}
			if ( masterSaturation ){
				view->setMasterSaturationImage( true );
			}
			if ( masterHue ){
				view->setMasterHueImage( true );
			}
			view->setDisplayedColor( rgbColor );
			addImage( view, dropIndex );
		}
	}

	void ImageScroll::removeImageView( QtDisplayData* displayData ){
		int index = findImageView( displayData->name().c_str());
		if ( index >= 0 ){
			this->closeImage( images[index], true );
		}
	}

	void ImageScroll::addImage( ImageView* viewerImage, int dropIndex ) {
		QString newName = viewerImage->getName();
		int index = findImageView( newName );
		if ( index < 0 ){
			connect( viewerImage, SIGNAL( displayTypeChanged( ImageView* )),
				 this, SIGNAL(displayTypeChanged(ImageView*)));
			connect( viewerImage, SIGNAL( close(ImageView*)),
					this, SLOT( closeImage(ImageView*)));
			connect( viewerImage, SIGNAL( masterCoordinateImageSelected( ImageView*)),
					this, SLOT( coordinateSystemChanged( ImageView*)));
			connect( viewerImage, SIGNAL( masterCoordinateImageClear()),
						this, SLOT( masterCoordinateClear()));
			connect( viewerImage, SIGNAL( masterHueImageSelected( ImageView*)),
								this, SLOT( hueImageChanged( ImageView*)));
			connect( viewerImage, SIGNAL( masterSaturationImageSelected( ImageView*)),
								this, SLOT( saturationImageChanged( ImageView*)));
			connect( viewerImage, SIGNAL( showDataDisplayOptions( QtDisplayData*)),
					this, SIGNAL( showDataDisplayOptions( QtDisplayData*)));
			connect( viewerImage, SIGNAL( imageSelected(ImageView*)),
					this, SIGNAL(registrationChange(ImageView*)));
			connect( viewerImage, SIGNAL( viewImage(ImageView*)),
					this, SLOT( viewImage(ImageView*)));

			//Add to GUI
			QLayout* scrollLayout = scrollWidget->layout();
			scrollLayout->removeItem( spacer );
			if ( dropIndex == -1 ){
				images.append( viewerImage );
				scrollLayout->addWidget( viewerImage );
			}
			else {
				QVBoxLayout* boxLayout = dynamic_cast<QVBoxLayout*>(scrollLayout);
				boxLayout->insertWidget( dropIndex, viewerImage );
				images.insert( dropIndex, viewerImage );
			}
			scrollLayout->addItem( spacer );
		}
	}

	void ImageScroll::setRegisterAll( bool select ) {
		for ( int i = 0; i < images.size(); i++ ) {
			images[i]->setRegistered( select );
		}
	}


	//--------------------------------------------------------------
	//       Master Images (Coordinate,Hue,Saturation)
	//--------------------------------------------------------------

	void ImageScroll::resetMasterCoordinate( ImageView* newMaster ){
		if ( newMaster != NULL ){
			QString masterName = newMaster->getName();
			for ( QList<ImageView*>::iterator iter = images.begin();
					iter != images.end(); iter++ ) {
				if ( (*iter) != NULL && masterName != (*iter)->getName() ){
					(*iter)->setMasterCoordinateImage( false );
				}
			}
		}
	}

	void ImageScroll::masterCoordinateClear(){
		emit masterCoordinateImageChanged( NULL );
	}

	void ImageScroll::coordinateSystemChanged( ImageView* newMaster ){
		resetMasterCoordinate( newMaster );
		emit masterCoordinateImageChanged( newMaster->getData());
	}

	void ImageScroll::hueImageChanged( ImageView* imageData ){
		for ( QList<ImageView*>::iterator iter = images.begin();
							iter != images.end(); iter++ ) {
			if ( imageData != (*iter) ){
				(*iter)->setMasterHueImage( false );
			}
		}
	}

	void ImageScroll::saturationImageChanged( ImageView* imageData ){
		for ( QList<ImageView*>::iterator iter = images.begin();
									iter != images.end(); iter++ ) {
			if ( imageData != (*iter) ){
				(*iter)->setMasterSaturationImage( false );
			}
		}
	}

	void ImageScroll::setMasterCoordinateImage( QString coordinateImageName ){
		int masterIndex = findImageView( coordinateImageName );
		if ( masterIndex >= 0 ){
			images[masterIndex]->setMasterCoordinateImage( true );
			resetMasterCoordinate( images[masterIndex] );
		}
	}


	//--------------------------------------------------------------------------------
	//                      Setters
	//----------------------------------------------------------------------------------

	void ImageScroll::setColorCombinationMode( ImageView::ColorCombinationMode mode ) {
		QList<ImageView*>::iterator iter = images.begin();
		while ( iter != images.end()) {
			(*iter)->setColorCombinationMode( mode );
			iter++;
		}
	}




	void ImageScroll::setViewedImage( int registrationIndex ){
		int regIndex = 0;
		//We are in channel mode.  Viewed image will be the
		//last registered image.
		if ( registrationIndex < 0 ){
			int registrationCount = getRegisteredCount();
			registrationIndex = registrationCount - 1;
		}
		for ( QList<ImageView*>::iterator iter = images.begin(); iter != images.end(); iter++ ) {
			bool currentlyViewed = false;
			if ( (*iter)->isRegistered() ){
				if ( regIndex == registrationIndex ){
					currentlyViewed = true;
				}
				regIndex++;
			}
			(*iter)->setViewedImage( currentlyViewed );
		}
	}

	void ImageScroll::viewImage( ImageView* imageView ){
		int dropIndex = this->getIndex( imageView );
		int registrationIndex = this->getRegisteredIndex( dropIndex );
		emit animateToImage( registrationIndex );
	}


	//**************************************************************************
	//                Drag and Drop
	//**************************************************************************

	int ImageScroll::getDropIndex( int dropY  ){
		//Since some images may be open others not so we don't have
		//a uniform width.  Have to go through and ask each one what their
		//size is.
		int dropIndex = -1;
		int height = LAYOUT_MARGIN;
		int i = 0;
		QList<ImageView*>::iterator iter = images.begin();
		while ( iter != images.end() ) {
			QSize imageSize = (*iter)->size();
			height = height + imageSize.height() + 2*LAYOUT_SPACING;
			if (dropY < height ) {
				dropIndex = i;
				break;
			}

			iter++;
			i++;
		}
		if ( dropIndex == -1 ) {
			int imageCount = images.size();
			if ( dropY < height) {
				dropIndex = imageCount - 1;
			}
			else if ( dropY > height ){
				dropIndex = imageCount;
			}
		}
		return dropIndex;

	}


	void ImageScroll::dropEvent( QDropEvent* dropEvent ) {
		//Use the position to estimate where int the list the imageView
		//should be placed.

		QPoint dropPosition = dropEvent->pos();
		int dropY = dropPosition.y();
		int dropIndex = getDropIndex( dropY );
		removeDragMarker();

		if ( dropIndex >= 0 ) {
			const QMimeData* dropData = dropEvent->mimeData();
			ImageView* droppedView = getMimeImageView( dropData );
			if ( droppedView != NULL ) {
				int droppedImageIndex = images.indexOf( droppedView );

				//The image we are moving will be deleted from the list.
				//The list will be one item shorter when we insert it again.
				if ( droppedImageIndex < dropIndex && dropIndex != 0 && droppedImageIndex != images.size() -1) {
					dropIndex = dropIndex - 1;
				}

				//Update the order of the data holder list
				QtDisplayData* displayData = droppedView->getData();
				bool registered = droppedView->isRegistered();
				bool masterCoordinate = droppedView->isMasterCoordinate();
				bool masterSaturation = droppedView->isMasterSaturation();
				bool masterHue = droppedView->isMasterHue();
				QColor rgbColor = droppedView->getDisplayedColor();
				emit imageOrderingChanged( displayData, dropIndex, registered,
						masterCoordinate, masterSaturation, masterHue, rgbColor);
			}
		}
	}


	ImageView* ImageScroll::getMimeImageView( const QMimeData* mimeData ) {
		//Find the image view that was dropped.
		//QString targetName = mimeData->text();
		QByteArray titleBytes = mimeData->data( ImageView::DROP_ID );
		QString targetName(titleBytes);
		ImageView* droppedView = NULL;
		int index = findImageView( targetName );
		if ( index >= 0 ){
			droppedView = images[index];
		}
		return droppedView;
	}

	void ImageScroll::removeDragMarker(){
		QLayout* iLayout = scrollWidget->layout();
		if ( iLayout != NULL && iLayout->indexOf( dropMarker) >= 0 ){
			QVBoxLayout* scrollLayout = dynamic_cast<QVBoxLayout*>(iLayout);
			scrollLayout->removeWidget( dropMarker );
			dropMarker->setParent( NULL );
		}
	}

	void ImageScroll::insertDragMarker( int position ){
		QLayout* iLayout = scrollWidget->layout();
		QVBoxLayout* scrollLayout = dynamic_cast<QVBoxLayout*>(iLayout);
		scrollLayout->insertWidget( position, dropMarker );
	}

	int ImageScroll::getDragMarkerLayoutIndex() const {
		QLayout* iLayout = scrollWidget->layout();
		int layoutIndex = -1;
		if ( iLayout != NULL ){
			layoutIndex = iLayout->indexOf( dropMarker);
		}
		return layoutIndex;
	}

	void ImageScroll::dragMoveEvent( QDragMoveEvent* dragMoveEvent ) {
		QPoint dragPoint = dragMoveEvent->pos();
		int dragY = dragPoint.y();
		int dropIndex = getDropIndex(dragY);
		if ( dropIndex >= 0 ){
			int layoutIndex = getDragMarkerLayoutIndex();
			if ( dropIndex != layoutIndex ){
				if ( layoutIndex >= 0  ){
					removeDragMarker();
				}
				insertDragMarker( dropIndex );
			}
		}
		dragMoveEvent->accept();
	}

	void ImageScroll::dragEnterEvent( QDragEnterEvent* enterEvent ) {
		const QMimeData* mimeData = enterEvent->mimeData();
		ImageView* draggedImageView = getMimeImageView( mimeData );
		if ( draggedImageView != NULL ) {
			enterEvent->acceptProposedAction();
		}
	}

	void ImageScroll::dragLeaveEvent( QDragLeaveEvent* /*leaveEvent*/ ){
		removeDragMarker();
	}


	//-------------------------------------------------------------------------
	//                  Utility
	//-------------------------------------------------------------------------

	QString ImageScroll::stripBold( QString name ) const {
		QString result = name.remove( "<b>");
		result = result.remove( "</b>");
		return result;
	}

	int ImageScroll::findImageView( QString targetName, bool exactMatch ) {
		int index = 0;
		int targetIndex = -1;
		targetName = stripBold( targetName );
		for ( QList<ImageView*>::iterator iter =images.begin();
				iter != images.end(); iter++ ) {
			QString viewName = (*iter)->getName();
			viewName = stripBold( viewName );
			if ( viewName == targetName ) {
				targetIndex = index;
				break;
			}
			index++;
		}
		//The data could have just changed type.  Look for a view
		//with no data
		if ( targetIndex == - 1 && !exactMatch ){
			QString prefix = removeSuffixes( targetName );
			index = 0;
			for ( QList<ImageView*>::iterator iter =images.begin();
							iter != images.end(); iter++ ) {
				QString viewName = stripBold( (*iter)->getName());
				QString imagePrefix = removeSuffixes( viewName );
				if ( prefix == imagePrefix && (*iter)->isEmpty()){
					targetIndex = index;
					break;
				}
				index++;
			}
		}
		return targetIndex;
	}

	QString ImageScroll::removeSuffixes( QString name ) const {
		QString cleaned = name;

		//Raster images have the word raster appended to their names
		//while other types of images do not.
		const QString RASTER( "-raster");
		cleaned = cleaned.remove(RASTER );

		//When duplicate images of the same name are loaded they get
		//a suffix of the form <n>.
		int dupIndex = cleaned.indexOf( "<");
		cleaned = cleaned.right( dupIndex );

		//extra space
		cleaned = cleaned.trimmed();
		return cleaned;
	}


	ImageScroll::~ImageScroll() {
	}
}
