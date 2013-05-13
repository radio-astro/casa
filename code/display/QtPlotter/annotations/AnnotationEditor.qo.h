#ifndef ANNOTATIONEDITOR_QO_H
#define ANNOTATIONEDITOR_QO_H

#include <QtGui/QDialog>
#include <display/QtPlotter/annotations/AnnotationEditor.ui.h>

namespace casa {

	class PropertyListener;

	class AnnotationEditor : public QDialog {
		Q_OBJECT

	public:
		AnnotationEditor(PropertyListener* annotation, QWidget *parent = 0);
		~AnnotationEditor();
		void addSpecializedWidget( QWidget* widget );
		void setSpecializedTitle( const QString& title );
		QColor getForegroundColor() const;
	private slots:
		void addColor( );
		void editingDone();
	private:
		void setLabelColor( const QString& colorName );
		PropertyListener* propertyListener;
		const QColor DEFAULT_COLOR;
		Ui::AnnotationEditorClass ui;
	};
}
#endif // ANNOTATIONEDITOR_H
