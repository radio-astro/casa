/*
 * AnnotationText.cc
 *
 *  Created on: Sep 27, 2012
 *      Author: slovelan
 */

#include "AnnotationText.h"
#include <display/QtPlotter/annotations/AnnotationEditorText.qo.h>
#include <QPainter>
#include <QDebug>
namespace casa {

	AnnotationText::AnnotationText(QWidget* parent) :
		Annotation(parent ), annotationEditorText( NULL ) {
		annotationEditorText = new AnnotationEditorText( &annotationEditor );
		annotationEditor.setSpecializedTitle( "Text" );
		annotationEditor.addSpecializedWidget( annotationEditorText );

	}

	void AnnotationText::drawAnnotation( QPainter* painter ) {
		int yCenter = pixelY + height / 2;
		QPoint startPoint( pixelX, yCenter );
		QFont oldFont = painter->font();
		painter->setFont( fontFamily );
		painter->drawText( startPoint, label );
		painter->setFont( oldFont );
	}



	void AnnotationText::propertiesChanged() {
		//Set up my properties
		label = annotationEditorText->getLabel();
		fontFamily = annotationEditorText->getFontFamily();
		fontFamily.setPointSize(annotationEditorText->getFontSize());
		fontFamily.setItalic( annotationEditorText->isItalic());
		fontFamily.setBold( annotationEditorText->isBold());

		//Tell my parent about it
		Annotation::propertiesChanged();
	}

	AnnotationText::~AnnotationText() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
