#include "PreferencesFunction.qo.h"
#include <QColorDialog>

namespace casa {

PreferencesFunction::PreferencesFunction(int index, QWidget *parent )
    : QWidget(parent){
	ui.setupUi(this);
	id = index;
	connect( ui.visibleCheckBox, SIGNAL( stateChanged(int)), this, SLOT(visibilityChanged()));
	connect( ui.colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
}

QString PreferencesFunction::readCustomColor( QSettings& settings, const QString& baseLookup){
	QString lookupStr = baseLookup + QString::number(id);
	QString colorName = settings.value( lookupStr, "" ).toString();
	return colorName;
}

void PreferencesFunction::storeCustomColor( QSettings& settings, const QString& baseLookup ){
	QString storageKey = baseLookup + QString::number( id );
	QString colorName = defaultColor.name();
	settings.setValue( storageKey, colorName );
}

void PreferencesFunction::showColorDialog(){
	QColor initialColor = getButtonColor();
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( selectedColor );
	}
}

QColor PreferencesFunction::getColor() const {
	return defaultColor;
}

void PreferencesFunction::setColor( QColor other ){
	defaultColor = other;
}

void PreferencesFunction::storeColor(){
	defaultColor = getButtonColor();
}

void PreferencesFunction::resetColor(){
	setButtonColor( defaultColor );
}

QColor PreferencesFunction::getButtonColor() const {
	QPalette p = ui.colorButton->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void PreferencesFunction::setButtonColor( QColor color ){
	QPalette p = ui.colorButton->palette();
	p.setBrush(QPalette::Button, color);
	ui.colorButton->setPalette( p );
}

void PreferencesFunction::visibilityChanged(){
	ui.colorButton->setEnabled( ui.visibleCheckBox->isChecked());
}

PreferencesFunction::~PreferencesFunction()
{

}
}
