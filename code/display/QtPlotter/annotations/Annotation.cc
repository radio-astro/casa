/*
 * Annotation.cc
 *
 *  Created on: Sep 27, 2012
 *      Author: slovelan
 */

#include "Annotation.h"
#include <QPainter>
#include <QDebug>
namespace casa {

	Annotation::Annotation( QWidget* parent ):
		annotationEditor( this,parent ),
		selected( true ),
		CORNER_SIZE( 5 ), MIN_WIDTH( 10 ), MIN_HEIGHT( 10 ) {
		init();
	}


	void Annotation::init() {
		pixelX = 0;
		pixelY = 0;
		width = MIN_WIDTH;
		height = MIN_HEIGHT;
		foregroundColor = Qt::black;
	}
	void Annotation::propertiesChanged() {
		foregroundColor = annotationEditor.getForegroundColor();
		selected = false;
	}

	void Annotation::showEditor() {
		annotationEditor.show();
	}

	bool Annotation::isSelected() const {
		return selected;
	}
	void Annotation::setSelected( bool selected ) {
		this->selected = selected;
	}
	bool Annotation::contains( int posX, int posY) const {
		bool insideRect = false;
		if ( pixelX <= posX && posX <= pixelX + width ) {
			if ( pixelY <= posY && posY <= pixelY + height ) {
				insideRect = true;

			}
		}
		return insideRect;
	}

	void Annotation::drawCorner( MouseLocation cornerLocation, QPainter* painter,
	                             int cornerX, int cornerY ) {
		if ( mouseLocation == cornerLocation ) {
			painter->fillRect( cornerX, cornerY, CORNER_SIZE, CORNER_SIZE, foregroundColor );
		} else {
			painter->drawRect( cornerX, cornerY, CORNER_SIZE, CORNER_SIZE );
		}
	}

	void Annotation::draw( QPainter* painter ) {
		if ( pixelX > 0 ) {
			QPen storedPen = painter->pen();
			QPen newPen = storedPen;

			newPen.setColor( foregroundColor );
			newPen.setWidth( 2 );
			painter->setPen( newPen );
			if ( selected ) {
				QBrush brush( Qt::white );
				painter->fillRect( pixelX, pixelY, width, height, brush );
				painter->drawRect( pixelX, pixelY, width, height );
				drawCorner( TOP_LEFT, painter, pixelX, pixelY );
				drawCorner( TOP_RIGHT, painter, pixelX+width-CORNER_SIZE, pixelY );
				drawCorner( BOTTOM_LEFT, painter, pixelX, pixelY+height-CORNER_SIZE );
				drawCorner( BOTTOM_RIGHT, painter, pixelX+width-CORNER_SIZE, pixelY+height-CORNER_SIZE );
			}
			drawAnnotation( painter );
			painter->setPen( storedPen );
		}
	}



	void Annotation::move( int positionX, int positionY ) {
		pixelX = positionX;
		pixelY = positionY;
	}

	void Annotation::setDimensionsPosition( int positionX, int positionY ) {
		this->width = qMax( qAbs( positionX - pixelX ), MIN_WIDTH);
		this->height = qMax( qAbs( positionY - pixelY ), MIN_HEIGHT);
	}

	void Annotation::setDimensions( int newWidth, int newHeight ) {
		width = qMax( newWidth, MIN_WIDTH);
		height = qMax( newHeight, MIN_HEIGHT );
	}


	bool Annotation::isInCorner( int cornerX, int cornerY, int mouseX, int mouseY ) const {
		bool inCorner = false;
		if ( cornerX - CORNER_SIZE <= mouseX && mouseX <= cornerX + CORNER_SIZE ) {
			if ( cornerY - CORNER_SIZE <= mouseY && mouseY <= cornerY + CORNER_SIZE ) {
				inCorner = true;
			}
		}
		return inCorner;
	}

	bool Annotation::registerMousePosition( int mouseX, int mouseY ) {

		mouseDownX = mouseX;
		mouseDownY = mouseY;

		int oldMouseLocation = mouseLocation;
		bool changedLocation = false;
		if ( isInCorner( pixelX, pixelY, mouseX, mouseY )) {
			mouseLocation = TOP_LEFT;
		} else if ( isInCorner( pixelX + width, pixelY, mouseX, mouseY )) {
			mouseLocation = TOP_RIGHT;
		} else if ( isInCorner( pixelX, pixelY+height, mouseX, mouseY )) {
			mouseLocation = BOTTOM_LEFT;
		} else if ( isInCorner( pixelX + width, pixelY+height, mouseX, mouseY )) {
			mouseLocation = BOTTOM_RIGHT;
		} else {
			mouseLocation = INTERIOR;
		}
		if ( oldMouseLocation != mouseLocation ) {
			changedLocation = true;
		}
		return changedLocation;
	}

	void Annotation::mousePositionMoved( int mouseX, int mouseY ) {
		int mouseMoveAmountX = mouseX - mouseDownX;
		int mouseMoveAmountY = mouseY - mouseDownY;

		if ( mouseLocation == INTERIOR ) {
			pixelX = pixelX + mouseMoveAmountX;
			pixelY = pixelY + mouseMoveAmountY;
		} else if ( mouseLocation == TOP_LEFT ) {
			pixelX = pixelX + mouseMoveAmountX;
			pixelY = pixelY + mouseMoveAmountY;
			setDimensions(width - mouseMoveAmountX, height - mouseMoveAmountY);
		} else if ( mouseLocation == TOP_RIGHT ) {
			setDimensions( width + mouseMoveAmountX, height - mouseMoveAmountY );
			pixelY = pixelY + mouseMoveAmountY;
		} else if ( mouseLocation == BOTTOM_LEFT ) {
			setDimensions( width - mouseMoveAmountX, height + mouseMoveAmountY );
			pixelX = pixelX + mouseMoveAmountX;
		} else if ( mouseLocation == BOTTOM_RIGHT ) {
			setDimensions( width + mouseMoveAmountX, height + mouseMoveAmountY );
		} else {
			qDebug()<<"unrecognized mouse location: "<<mouseLocation;
		}
		mouseDownX = mouseX;
		mouseDownY = mouseY;
	}


	Annotation::~Annotation() {
	}

} /* namespace casa */
