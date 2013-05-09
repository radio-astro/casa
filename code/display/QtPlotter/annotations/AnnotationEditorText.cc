#include "AnnotationEditorText.qo.h"
namespace casa {
	AnnotationEditorText::AnnotationEditorText(QWidget *parent)
		: QWidget(parent) {
		ui.setupUi(this);

		ui.fontSizeSpinBox->setMinimum( 1 );
		ui.fontSizeSpinBox->setMaximum( 20 );
		ui.fontSizeSpinBox->setValue( 10 );
	}

	QString AnnotationEditorText::getLabel() const {
		return ui.lineEdit->text();
	}

	QFont AnnotationEditorText::getFontFamily() const {
		return ui.fontComboBox->currentFont();
	}

	int AnnotationEditorText::getFontSize() const {
		return ui.fontSizeSpinBox->value();
	}

	bool AnnotationEditorText::isBold() const {
		bool boldText = ui.boldCheckBox->isChecked();
		return boldText;
	}

	bool AnnotationEditorText::isItalic() const {
		bool italicText = ui.italicCheckBox->isChecked();
		return italicText;
	}

	AnnotationEditorText::~AnnotationEditorText() {

	}
}
