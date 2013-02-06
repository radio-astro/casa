#include "SliceColorPreferences.qo.h"
#include <QColorDialog>

namespace casa {

const QString SliceColorPreferences::APPLICATION = "1D Slice Tool";
const QString SliceColorPreferences::ORGANIZATION = "NRAO/CASA";
const QString SliceColorPreferences::SLICE_COLOR = "Slice Color";

SliceColorPreferences::SliceColorPreferences(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle( "1D Slice Color Preferences");
	sliceColor=Qt::blue;

	initializeUserColors();
	resetColors();

	connect( ui.sliceColorButton, SIGNAL(clicked()), this, SLOT(selectSliceColor()));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

QColor SliceColorPreferences::getSliceColor() const {
	return sliceColor;
}

QColor SliceColorPreferences::getButtonColor( QPushButton* button ) const {
	QPalette p = button->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void SliceColorPreferences::showColorDialog( QPushButton* source ){
	QColor initialColor = getButtonColor( source );
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( source, selectedColor );
	}
}

void SliceColorPreferences::selectSliceColor(){
	showColorDialog( ui.sliceColorButton );
}

void SliceColorPreferences::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	QString sliceColorName = readCustomColor( settings, SLICE_COLOR, sliceColor.name() );
	if ( sliceColorName.length() > 0 ){
		sliceColor = QColor( sliceColorName );
	}
}

QString SliceColorPreferences::readCustomColor( QSettings& settings,
		const QString& identifier, const QString& defaultColor){
	QString colorName = settings.value( identifier, defaultColor ).toString();
	return colorName;
}

void SliceColorPreferences::resetColors(){
	setButtonColor( ui.sliceColorButton, sliceColor );
}

void SliceColorPreferences::setButtonColor( QPushButton* button, QColor color ){
	QPalette p = button->palette();
	p.setBrush(QPalette::Button, color);
	button->setPalette( p );
}

void SliceColorPreferences::colorsAccepted(){
	persistColors();
	QDialog::close();
	emit colorsChanged();
}

void SliceColorPreferences::colorsRejected(){
	resetColors();
	QDialog::close();
}

void SliceColorPreferences::persistColors(){
	//Copy the colors from the buttons into color variables
	sliceColor = getButtonColor( ui.sliceColorButton );

	//Save the colors in the persistent settings.
	QSettings settings( ORGANIZATION, APPLICATION );
	settings.clear();
	settings.setValue( SLICE_COLOR, sliceColor.name() );
}

SliceColorPreferences::~SliceColorPreferences()
{

}
}
