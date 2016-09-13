#include "AnnotationEditor.qo.h"
#include <display/QtPlotter/annotations/PropertyListener.h>
#include <QColorDialog>
namespace casa {
	AnnotationEditor::AnnotationEditor(PropertyListener* propertyListener, QWidget *parent)
		: QDialog(parent),DEFAULT_COLOR(Qt::black) {
		ui.setupUi(this);
		this->propertyListener = propertyListener;


		setLabelColor( DEFAULT_COLOR.name() );

		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(editingDone()));
		connect( ui.browseColorButton, SIGNAL(clicked()), this, SLOT(addColor()));

	}

	void AnnotationEditor::addSpecializedWidget( QWidget* widget ) {
		QHBoxLayout* layout = new QHBoxLayout();
		layout->addWidget( widget );
		ui.specializedGroupBox->setLayout( layout );
	}
	void AnnotationEditor::setSpecializedTitle( const QString& title ) {
		ui.specializedGroupBox->setTitle( title );
	}

	QColor AnnotationEditor::getForegroundColor() const {
		QPalette pal = ui.colorLabel->palette();
		QColor bkColor = pal.color(ui.colorLabel->backgroundRole());
		return bkColor;
	}

	void AnnotationEditor::setLabelColor( const QString& colorName ) {
		QString styleColor =  "QLabel { background-color: " + colorName + "; }";
		ui.colorLabel->setStyleSheet( styleColor );
	}

	void AnnotationEditor::editingDone() {
		if ( propertyListener != NULL ) {
			propertyListener->propertiesChanged();
		}
		this->close();
	}

	void AnnotationEditor::addColor( ) {
		QColor currentColor = getForegroundColor();
		QColor selectedColor = QColorDialog::getColor( currentColor, this );
		if ( selectedColor.isValid() ) {
			setLabelColor( selectedColor.name() );
		}
	}

	AnnotationEditor::~AnnotationEditor() {

	}
}
