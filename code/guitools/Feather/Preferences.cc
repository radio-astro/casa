#include "Preferences.qo.h"
#include <QSettings>

namespace casa {

const QString Preferences::ORGANIZATION = "NRAO/CASA";
const QString Preferences::APPLICATION = "Feather";
const QString Preferences::LINE_THICKNESS = "Plot Line Thickness";
const QString Preferences::DISPLAY_ORIGINAL_FUNCTIONS = "Display Original Functions";
const QString Preferences::DISPLAY_ORIGINAL_LEGEND = "Display Original Legend";
const QString Preferences::DISPLAY_OUTPUT_FUNCTIONS = "Display Output Functions";
const QString Preferences::DISPLAY_OUTPUT_LEGEND = "Display Output Legend";

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
      lineThickness( 1 ),
      displayOriginalFunctions(false),
      displayOriginalLegend(true),
      displayOutputFunctions( true ),
      displayOutputLegend( true ){

	ui.setupUi(this);
	setWindowTitle( "Feather Plot Display");

	ui.lineThicknessSpinBox->setMinimum( 0 );
	ui.lineThicknessSpinBox->setMaximum( 5 );

	initializeCustomSettings();
	reset();

	connect( ui.originalCheckBox, SIGNAL(stateChanged(int)), this, SLOT(originalPlotVisibilityChanged(int)));
	connect( ui.outputCheckBox, SIGNAL(stateChanged(int)), this, SLOT(outputPlotVisibilityChanged(int)));
	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(preferencesAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(preferencesRejected()));
}

void Preferences::initializeCustomSettings(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );
	lineThickness = settings.value( LINE_THICKNESS, lineThickness).toInt();
	displayOriginalFunctions = settings.value( DISPLAY_ORIGINAL_FUNCTIONS, displayOriginalFunctions).toBool();
	displayOriginalLegend = settings.value( DISPLAY_ORIGINAL_LEGEND, displayOriginalLegend ).toBool();
	displayOutputFunctions = settings.value( DISPLAY_OUTPUT_FUNCTIONS, displayOutputFunctions ).toBool();
	displayOutputLegend = settings.value( DISPLAY_OUTPUT_LEGEND, displayOutputLegend ).toBool();
	originalPlotVisibilityChanged( displayOriginalFunctions );
	outputPlotVisibilityChanged( displayOutputFunctions );
}

void Preferences::originalPlotVisibilityChanged( int checked ){
	ui.originalLegendCheckBox->setEnabled( checked );
}

void Preferences::outputPlotVisibilityChanged( int checked ){
	ui.outputLegendCheckBox->setEnabled( checked );
}

bool Preferences::isDisplayOriginalFunctions() const {
	return displayOriginalFunctions;
}

bool Preferences::isDisplayOriginalLegend() const {
	return displayOriginalLegend;
}

bool Preferences::isDisplayOutputFunctions() const {
	return displayOutputFunctions;
}

bool Preferences::isDisplayOutputLegend() const {
	return displayOutputLegend;
}

int Preferences::getLineThickness() const {
	return lineThickness;
}


void Preferences::preferencesAccepted(){
	persist();
	emit preferencesChanged();
	this->close();
}

void Preferences::preferencesRejected(){
	reset();
	this->close();
}

void Preferences::reset(){
	ui.lineThicknessSpinBox->setValue( lineThickness );
	ui.originalCheckBox->setChecked( displayOriginalFunctions );
	ui.originalLegendCheckBox->setChecked( displayOriginalLegend );
	ui.outputCheckBox->setChecked( displayOutputFunctions );
	ui.outputLegendCheckBox->setChecked( displayOutputLegend );
	originalPlotVisibilityChanged( displayOriginalFunctions );
	outputPlotVisibilityChanged( displayOutputFunctions );
}

void Preferences::persist(){
	QSettings settings( ORGANIZATION, APPLICATION );

	lineThickness = ui.lineThicknessSpinBox->value();
	settings.setValue( LINE_THICKNESS, lineThickness );

	displayOriginalFunctions = ui.originalCheckBox->isChecked();
	settings.setValue( DISPLAY_ORIGINAL_FUNCTIONS, displayOriginalFunctions );

	displayOriginalLegend = ui.originalLegendCheckBox->isChecked();
	settings.setValue( DISPLAY_ORIGINAL_LEGEND, displayOriginalLegend );

	displayOutputFunctions = ui.outputCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_FUNCTIONS, displayOutputFunctions );

	displayOutputLegend = ui.outputLegendCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_LEGEND, displayOutputLegend );
}

Preferences::~Preferences(){

}
}
