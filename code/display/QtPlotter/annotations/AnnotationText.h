/*
 * AnnotationText.h
 *
 *  Created on: Sep 27, 2012
 *      Author: slovelan
 */

#ifndef ANNOTATIONTEXT_H_
#define ANNOTATIONTEXT_H_

#include <display/QtPlotter/annotations/Annotation.h>

namespace casa {

	class AnnotationEditorText;

	class AnnotationText : public Annotation {
	public:
		AnnotationText( QWidget* parent);
		void drawAnnotation( QPainter* painter );
		virtual ~AnnotationText();
		virtual void propertiesChanged();

	private:

		QString label;
		QFont fontFamily;
		AnnotationEditorText* annotationEditorText;
	};

} /* namespace casa */
#endif /* ANNOTATIONTEXT_H_ */
