#include "PreferencesFunction.qo.h"
#include <QColorDialog>

namespace casa {

const QString PreferencesFunction::FUNCTION_COLOR = "Function Color";

PreferencesFunction::PreferencesFunction(int index,  QWidget *parent )
    : QWidget(parent), COLOR_KEY("Color"), VISIBILITY_KEY("Visibility"){
	ui.setupUi(this);

	id = index;
	scatterEligible = true;

	connect( ui.visibleCheckBox, SIGNAL( clicked()), this, SLOT(visibilityChanged()));
	connect( ui.colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
}

void PreferencesFunction::setScatterEligible( bool eligible ){
	scatterEligible = eligible;
}

bool PreferencesFunction::isScatterEligible() const {
	return scatterEligible;
}

void PreferencesFunction::setColor( QColor baseColor ){
	curveSettings.setColor( baseColor );
}

void PreferencesFunction::setDisplayed( bool displayed ){
	curveSettings.setVisibility( displayed );
}

bool PreferencesFunction::isDisplayed() const {
	return curveSettings.isDisplayed();
}

bool PreferencesFunction::isDisplayedGUI() const {
	return ui.visibleCheckBox->isChecked();
}

void PreferencesFunction::setName( const QString& name ){
	curveSettings.setName( name );
}

QString PreferencesFunction::getName() const {
	return curveSettings.getName();
}

void PreferencesFunction::setDisplayHidden(){
	ui.visibleCheckBox->hide();
}

QString PreferencesFunction::getBaseStorageId() const {
	return FUNCTION_COLOR + QString::number(id);
}

const CurveDisplay PreferencesFunction::getFunctionPreferences() const {
	return curveSettings;
}

void PreferencesFunction::initialize( QSettings& settings){
	QString lookupStr = getBaseStorageId();
	QString colorLookupStr = lookupStr+COLOR_KEY;
	QString defaultColorName = curveSettings.getColor().name();
	QString colorName = settings.value( colorLookupStr, defaultColorName ).toString();
	curveSettings.setColor( QColor( colorName ));

	QString visibilityLookupStr = lookupStr+VISIBILITY_KEY;
	bool defaultVisibility = curveSettings.isDisplayed();
	QString defaultVisStr = "true";
	if ( !defaultVisibility ){
		defaultVisStr = "false";
	}
	bool vis = settings.value( visibilityLookupStr,defaultVisStr).toBool();
	curveSettings.setVisibility( vis );
	reset();
}

void PreferencesFunction::persist( QSettings& settings){
	//Copy the displayed preferences to the default ones.
	curveSettings.setColor( getButtonColor() );
	curveSettings.setVisibility(ui.visibleCheckBox->isChecked());

	//Write the settings
	QString storageKey = getBaseStorageId();
	QString colorLookupStr = storageKey+COLOR_KEY;
	QString colorName = curveSettings.getColor().name();
	settings.setValue( colorLookupStr, colorName );
	QString visibilityLookupStr = storageKey + VISIBILITY_KEY;
	QString visStr = "true";
	if ( !curveSettings.isDisplayed()){
		visStr = "false";
	}
	settings.setValue( visibilityLookupStr, visStr );
}

void PreferencesFunction::showColorDialog(){
	QColor initialColor = getButtonColor();
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( selectedColor );
	}
}


void PreferencesFunction::reset(){
	setButtonColor( curveSettings.getColor() );
	ui.visibleCheckBox->setChecked( curveSettings.isDisplayed() );
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
	emit displayStatusChanged();
}

PreferencesFunction::~PreferencesFunction()
{

}
}
