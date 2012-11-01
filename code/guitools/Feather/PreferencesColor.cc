#include "PreferencesColor.qo.h"
#include <guitools/Feather/Preferences.qo.h>
#include <QColorDialog>

namespace casa {

const QString PreferencesColor::FUNCTION_COLOR = "Function Color";

PreferencesColor::PreferencesColor(QWidget *parent)
    : QDialog(parent), SCATTER_INDEX(-1){

	ui.setupUi(this);
	setWindowTitle( "Feather Plot Color Preferences");

	//Default colors
	colorMap.insert( SD_WEIGHT_COLOR, QColor( "#0000FF"));
	colorMap.insert( SD_ORIGINAL_COLOR, QColor("#1E90FF"));
	colorMap.insert( SD_OUTPUT_COLOR, QColor("#87CEEB"));
	colorMap.insert( INT_WEIGHT_COLOR, QColor("#FF0000"));
	colorMap.insert( INT_ORIGINAL_COLOR, QColor("#8B0000"));
	colorMap.insert( INT_OUTPUT_COLOR, QColor("#FF1493"));
	scatterPlotColor = Qt::black;

	initializeUserColors();
	resetColors();

	connect( ui.scatterColorButton, SIGNAL(clicked()), this, SLOT(selectScatterPlotColor()));
	connect( ui.singleDishWeightColorButton, SIGNAL(clicked()), this, SLOT(selectSDWeightColor()));
	connect( ui.singleDishOriginalColorButton, SIGNAL(clicked()), this, SLOT(selectSDOriginalColor()));
	connect( ui.singleDishOutputColorButton, SIGNAL(clicked()), this, SLOT(selectSDOutputColor()));
	connect( ui.interferometerWeightColorButton, SIGNAL(clicked()), this, SLOT(selectINTWeightColor()));
	connect( ui.interferometerOriginalColorButton, SIGNAL(clicked()), this, SLOT(selectINTOriginalColor()));
	connect( ui.interferometerOutputColorButton, SIGNAL(clicked()), this, SLOT(selectINTOutputColor()));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void PreferencesColor::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	for ( int i = 0; i < END_COLOR; i++ ){
		QString colorName = readCustomColor( settings, i );
		if ( colorName.length() > 0 ){
			QColor customColor( colorName );
			colorMap[static_cast<FunctionColor>(i)] = customColor;
		}
	}
	QString scatterColorName = readCustomColor( settings, SCATTER_INDEX );
	if ( scatterColorName.length() > 0 ){
		scatterPlotColor = QColor( scatterColorName );
	}
}

QMap<PreferencesColor::FunctionColor,QColor> PreferencesColor::getFunctionColors( ) const {
	return colorMap;
}

QColor PreferencesColor::getScatterPlotColor() const{
	return scatterPlotColor;
}

void PreferencesColor::storeCustomColor( QSettings& settings, FunctionColor index ){
	QString storageKey = FUNCTION_COLOR + QString::number( index );
	QString colorName = colorMap[index].name();
	settings.setValue( storageKey, colorName );
}

QString PreferencesColor::readCustomColor( QSettings& settings, int index){
	QString lookupStr = FUNCTION_COLOR + QString::number(index);
	QString colorName = settings.value( lookupStr, "" ).toString();
	return colorName;
}

void PreferencesColor::storeMapColor( QPushButton* button, FunctionColor colorType ){
	QColor buttonColor = getButtonColor( button );
	QString buttonColorName = buttonColor.name();
	colorMap[colorType] = buttonColorName;
}

void PreferencesColor::persistColors(){
	//Copy the colors from the buttons into the map.
	storeMapColor( ui.singleDishWeightColorButton, SD_WEIGHT_COLOR);
	storeMapColor( ui.singleDishOriginalColorButton, SD_ORIGINAL_COLOR);
	storeMapColor( ui.singleDishOutputColorButton, SD_OUTPUT_COLOR);
	storeMapColor( ui.interferometerWeightColorButton, INT_WEIGHT_COLOR);
	storeMapColor( ui.interferometerOriginalColorButton, INT_ORIGINAL_COLOR);
	storeMapColor( ui.interferometerOutputColorButton, INT_OUTPUT_COLOR);
	scatterPlotColor = getButtonColor( ui.scatterColorButton );


	//Save the colors in the map
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	settings.clear();
	for ( int i = 0; i < END_COLOR; i++ ){
		storeCustomColor( settings, static_cast<FunctionColor>(i) );
	}
	QString scatterKey = FUNCTION_COLOR + QString::number( SCATTER_INDEX );
	settings.setValue( scatterKey, scatterPlotColor.name() );
}

void PreferencesColor::colorsAccepted(){
	persistColors();
	QDialog::close();
	emit colorsChanged();
}

void PreferencesColor::colorsRejected(){
	resetColors();
	QDialog::close();
}

void PreferencesColor::resetColors(){
	setButtonColor( ui.scatterColorButton, Qt::black );
	setButtonColor( ui.singleDishWeightColorButton, colorMap[SD_WEIGHT_COLOR]);
	setButtonColor( ui.singleDishOriginalColorButton, colorMap[SD_ORIGINAL_COLOR]);
	setButtonColor( ui.singleDishOutputColorButton, colorMap[SD_OUTPUT_COLOR]);
	setButtonColor( ui.interferometerWeightColorButton, colorMap[INT_WEIGHT_COLOR]);
	setButtonColor( ui.interferometerOriginalColorButton, colorMap[INT_ORIGINAL_COLOR]);
	setButtonColor( ui.interferometerOutputColorButton, colorMap[INT_OUTPUT_COLOR]);
	setButtonColor( ui.scatterColorButton, scatterPlotColor );
}

void PreferencesColor::setButtonColor( QPushButton* button, QColor color ){
	QPalette p = button->palette();
	p.setBrush(QPalette::Button, color);
	button->setPalette( p );
}

QColor PreferencesColor::getButtonColor( QPushButton* button ) const {
	QPalette p = button->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void PreferencesColor::showColorDialog( QPushButton* source ){
	QColor initialColor = getButtonColor( source );
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( source, selectedColor );
	}
}

void PreferencesColor::selectSDWeightColor(){
	showColorDialog( ui.singleDishWeightColorButton );
}

void PreferencesColor::selectSDOriginalColor(){
	showColorDialog( ui.singleDishOriginalColorButton );
}

void PreferencesColor::selectSDOutputColor(){
	showColorDialog( ui.singleDishOutputColorButton );
}

void PreferencesColor::selectINTWeightColor(){
	showColorDialog( ui.interferometerWeightColorButton );
}

void PreferencesColor::selectINTOriginalColor(){
	showColorDialog( ui.interferometerOriginalColorButton );
}

void PreferencesColor::selectINTOutputColor(){
	showColorDialog( ui.interferometerOutputColorButton );
}

void PreferencesColor::selectScatterPlotColor(){
	showColorDialog( ui.scatterColorButton );
}

PreferencesColor::~PreferencesColor(){
}

}
