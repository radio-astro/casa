#include "ImageScroll.qo.h"
#include <display/QtViewer/ImageView.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

namespace casa {

ImageScroll::ImageScroll(QWidget *parent)
    : QScrollArea(parent)
{
	ui.setupUi(this);

	QVBoxLayout* scrollLayout = new QVBoxLayout();
	setLayout( scrollLayout );
	setAutoFillBackground( true );
	QPalette pal = palette();
	pal.setColor( QPalette::Background, Qt::white );
	setPalette( pal );

	setAcceptDrops( true );
}

void ImageScroll::addImage( QtDisplayData* displayData ){
	ImageView* viewerImage = new ImageView( displayData, this );
	QLayout* scrollLayout = layout();
	scrollLayout->addWidget( viewerImage );

	images.append( viewerImage );
}

void ImageScroll::addImageViews( QList<ImageView*>& views ){
	QList<ImageView*>::iterator iter = views.begin();
	while ( iter != views.end()){
		images.append( *iter );
		iter++;
	}
	resetScroll();
}

void ImageScroll::removeImageViews( QList<ImageView*>& views ){
	QList<ImageView*>::iterator iter = views.begin();
	while ( iter != views.end()){
		images.removeOne( *iter );
		iter++;
	}
	resetScroll();
}

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

void ImageScroll::resetScroll(){
	QLayout* scrollLayout = layout();
	while( ! scrollLayout->isEmpty()){
		scrollLayout->takeAt(0);

	}
	QList<ImageView*>::iterator iter = images.begin();
	while( iter != images.end()){
		scrollLayout->addWidget( *iter);
		iter++;
	}
}

void ImageScroll::closeImages(){
	QList<ImageView*> closeImages = getSelectedViews();
	while ( !closeImages.isEmpty()){
		ImageView* imageView = closeImages.takeLast();
		images.removeOne( imageView );
		delete imageView;
	}
	resetScroll();
}

//**************************************************************************
//                Drag and Drop
//**************************************************************************

void ImageScroll::dropEvent( QDropEvent* dropEvent ){
	qDebug() << "Items dropped";

	//Use the position to estimate where int the list the imageView
	//should be placed.
	QPoint dropPosition = dropEvent->pos();
	int dropY = dropPosition.y();
	//PROBLEM!!!!! Some images may be open others not so we don't have
	//a uniform width.  Have to go through and ask each one what their
	//size is.
	int dropIndex = -1;
	int MARGIN = 10;
	int height = MARGIN;
	int i = 0;
	QList<ImageView*>::iterator iter = images.begin();
	while ( iter != images.end() ){
		if (dropY < height ){
			dropIndex = i;
			break;
		}
		QSize imageSize = (*iter)->size();
		height = height + imageSize.height() + MARGIN;
		iter++;
		i++;
	}

	if ( dropIndex >= 0 ){
		//Find the image view that was dropped.
		const QMimeData* dropData = dropEvent->mimeData();
		QString targetName = dropData->text();
		QList<ImageView*>::iterator iter = images.begin();
		ImageView* droppedView = NULL;
		while ( iter != images.end() ){
			QString viewName = (*iter)->getName();
			if ( viewName == targetName ){
				droppedView =  *iter;
				break;
			}
			iter++;
		}
		if ( droppedView != NULL ){
			QLayout* scrollLayout = layout();
			scrollLayout->removeWidget( droppedView );
			QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(scrollLayout);
			boxLayout->insertWidget(dropIndex, droppedView );
		}
	}
}

void ImageScroll::dragMoveEvent( QDragMoveEvent* dragMoveEvent ){
	dragMoveEvent->accept();
}



void ImageScroll::dragEnterEvent( QDragEnterEvent* enterEvent ){
	qDebug() << "Item entered.";
	enterEvent->acceptProposedAction();
}

ImageScroll::~ImageScroll(){
	while( images.isEmpty()){
		ImageView* viewerImage = images.takeLast();
		delete viewerImage;
	}
}
}
