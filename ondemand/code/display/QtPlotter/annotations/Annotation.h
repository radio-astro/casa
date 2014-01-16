/*
 * Annotation.h
 *
 *  Created on: Sep 27, 2012
 *      Author: slovelan
 */

#ifndef ANNOTATION_H_
#define ANNOTATION_H_
#include <display/QtPlotter/annotations/AnnotationEditor.qo.h>
#include <display/QtPlotter/annotations/PropertyListener.h>
#include <QColor>
#include <QObject>
class QPainter;

namespace casa {

	class Annotation : public PropertyListener {
	public:
		Annotation( QWidget* parent = NULL );

		/**
		 * Changes the size of the annotation.  (locationX, locationY)
		 * are assumed to be the newer bottom right corner of the annotation.
		 */
		void setDimensionsPosition( int locationX, int locationY );
		void setDimensions( int width, int height );

		/**
		 * Moves the top left corner of the annotation to (positionX, positionY);
		 */
		void move( int positionX, int positionY );

		/**
		 * Stores the passed in values as reference points so that they can be
		 * later used to calculate how much to move the annotation.  Returns true
		 * if the mouse location has changed, say from a top right corner to the
		 * interior region of the annotation.
		 */
		bool registerMousePosition( int mouseX, int mouseY );

		/**
		 * (mouseX,mouseY) are the new position of the mouse. The difference between
		 * the stored position and the new position is used to calculate how much to
		 * move the top left corner of the annotation.
		 */
		void mousePositionMoved( int mouseX, int mouseY );
		void draw( QPainter* painter );
		void setSelected( bool selected );
		bool isSelected() const;
		bool contains( int posX, int posY) const;
		virtual void drawAnnotation( QPainter* painter ) = 0;
		void showEditor( );
		virtual ~Annotation();
		void propertiesChanged();

	protected:
		AnnotationEditor annotationEditor;
		void init();
		//Top left corner of the annotation
		int pixelX;
		int pixelY;
		int width;
		int height;

		//Stored reference point for later calculation move amounts
		int mouseDownX;
		int mouseDownY;

	private:
		enum MouseLocation {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, INTERIOR };
		bool isInCorner( int cornerX, int cornerY, int mouseX, int mouseY ) const;
		void drawCorner( MouseLocation cornerLocation, QPainter* painter, int cornerX, int cornerY );

		bool selected;
		QColor foregroundColor;
		const int CORNER_SIZE;
		const int MIN_WIDTH;
		const int MIN_HEIGHT;
		MouseLocation mouseLocation;
	};

} /* namespace casa */
#endif /* ANNOTATION_H_ */
