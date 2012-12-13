//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include "FeatherMain.qo.h"
#include <guitools/Feather/PlotHolder.qo.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageUtilities.h>
#include <casa/Utilities/PtrHolder.h>
#include <QDebug>
#include <QMap>
#include <QMessageBox>

namespace casa {

const int FeatherMain::DISH_DIAMETER_DEFAULT = -1;
const int FeatherMain::SINGLE_DISH_FACTOR_DEFAULT = 1;

void FeatherThread::run(){

	//Initialize the data
	featherWorker->getFeatherSD( sDxWeight, sDxAmpWeight, sDyWeight, sDyAmpWeight );
	featherWorker->getFeatherINT( intxWeight, intxAmpWeight, intyWeight, intyAmpWeight );

	featherWorker->getFeatheredCutSD( sDxCut, sDxAmpCut, sDyCut, sDyAmpCut );
	featherWorker->getFeatheredCutINT( intxCut, intxAmpCut, intyCut, intyAmpCut );

	if ( saveOutput ){
		fileSaved = featherWorker->saveFeatheredImage( saveFilePath.toStdString() );
	}
}

FeatherMain::FeatherMain(QWidget *parent)
    : QMainWindow(parent), fileLoader( this ),
      preferences( this ), preferencesColor( this ),
      lowResImage( NULL ), highResImage( NULL ),
      thread(NULL), plotHolder(NULL), progressMeter(this), logger(LogOrigin("CASA", "Feather")){

	ui.setupUi(this);

	plotHolder = new PlotHolder( this );
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget( plotHolder );
	ui.plotHolderWidget->setLayout( layout );
	connect( plotHolder, SIGNAL(dishDiameterChangedX(double)), this, SLOT(dishDiameterXChanged(double)));
	connect( plotHolder, SIGNAL(dishDiameterChangedY(double)), this, SLOT(dishDiameterYChanged(double)));

	progressMeter.setWindowTitle( "Feather");
	progressMeter.setLabelText( "Feathering Images...");
	progressMeter.setWindowModality( Qt::WindowModal );
	progressMeter.setCancelButton( 0 );
	progressMeter.setMinimum( 0 );
	progressMeter.setMaximum( 0 );
	connect( this, SIGNAL( featherFinished()), &progressMeter, SLOT(cancel()));

	//Put a validator on the effective dish diameter.
	QDoubleValidator* validator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 10, this );
	ui.dishDiameterXLineEdit->setValidator( validator );
	ui.dishDiameterYLineEdit->setValidator( validator );
	ui.singleDishFactorLineEdit->setValidator( validator );
	initializeDishDiameterLimit( ui.dishDiameterXLimitLabel );
	initializeDishDiameterLimit( ui.dishDiameterYLimitLabel );
	connect( ui.dishDiameterXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dishDiameterXChanged(const QString&)));
	connect( ui.dishDiameterYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dishDiameterYChanged(const QString&)));

	ui.featherButton->setEnabled( false );

	functionColorsChanged();
	preferencesChanged();

	QActionGroup* paletteGroup = new QActionGroup(this );
	ui.actionDiameterSelector->setActionGroup( paletteGroup );
	ui.actionRectangleZoom->setActionGroup( paletteGroup );
	ui.actionDiameterSelector->setCheckable( true );
	ui.actionRectangleZoom->setCheckable( true );
	ui.actionRectangleZoom->setChecked( true );
	connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
	connect(ui.actionImageFiles, SIGNAL(triggered()), this, SLOT(openFileLoader()));
	connect(ui.actionPreferencesColor, SIGNAL(triggered()), this, SLOT(openPreferencesColor()));
	connect(ui.actionRectangleZoom, SIGNAL(triggered()), plotHolder, SLOT(setRectangleZoomMode()));
	connect(ui.actionDiameterSelector, SIGNAL(triggered()), plotHolder, SLOT(setDiameterSelectorMode()));

	connect( &fileLoader, SIGNAL(imageFilesChanged()), this, SLOT(imageFilesChanged()));
	connect( &preferencesColor,SIGNAL(colorsChanged()), this, SLOT(functionColorsChanged()));
	connect(ui.featherButton, SIGNAL(clicked()), this, SLOT( featherImages()));
	connect( ui.yGroupBox, SIGNAL(toggled(bool)), this, SLOT(ySupportChanged(bool)));
	connect(&preferences, SIGNAL(preferencesChanged()), this, SLOT(preferencesChanged()));
}

void FeatherMain::initializeDishDiameterLimit( QLabel* diamLimitLabel ){
	QFont font = diamLimitLabel->font();
	font.setBold( true );
	diamLimitLabel->setFont( font );
	QPalette palette = diamLimitLabel->palette();
	palette.setColor(QPalette::Foreground, Qt::red );
	diamLimitLabel->setPalette( palette );
	diamLimitLabel->setVisible( false );
}


void FeatherMain::ySupportChanged( bool ySupport ){
	if ( !ySupport ){
		ui.dishDiameterYLineEdit->setText( ui.dishDiameterXLineEdit->text());
	}
}

void FeatherMain::dishDiameterXChanged( double value ){
	QString diameterStr = QString::number( value );
	ui.dishDiameterXLineEdit->setText( diameterStr );
}

void FeatherMain::dishDiameterYChanged( double value ){
	QString diameterStr = QString::number( value );
	if ( ui.dishDiameterYLineEdit->isEnabled() ){
		ui.dishDiameterYLineEdit->setText( diameterStr );
	}
	else {
		ui.dishDiameterXLineEdit->setText( diameterStr );
	}
}

void FeatherMain::dishDiameterXChanged( const QString& xDiameter ){
	if ( !ui.dishDiameterYLineEdit->isEnabled() ){
		ui.dishDiameterYLineEdit->setText( xDiameter );
	}
	double xDiamVal = xDiameter.toDouble();
	plotHolder->dishDiameterXChanged( xDiamVal );
}

void FeatherMain::dishDiameterYChanged( const QString& yDiameter ){
	double yDiamVal = yDiameter.toDouble();
	plotHolder->dishDiameterYChanged( yDiamVal );
}



void FeatherMain::preferencesChanged(){

	//Line thickness
	int lineThickness = preferences.getLineThickness();
	plotHolder->setLineThickness( lineThickness );

	int dotSize = preferences.getDotSize();
	plotHolder->setDotSize( dotSize );

	//Legend visibility
	bool legendVisible = preferences.isDisplayLegend();
	plotHolder->setLegendVisibility( legendVisible );

	plotHolder->setDisplayScatterPlot( preferences.isDisplayOutputScatterPlot());
	plotHolder->setDisplayOriginalSlice( preferences.isDisplayOriginalFunctions());
	plotHolder->setDisplayOutputSlice( preferences.isDisplayOutputFunctions());
	plotHolder->setDisplayYGraphs( !preferences.isDisplayXOnly() );
	plotHolder->layoutPlotWidgets();
}

void FeatherMain::functionColorsChanged(){
	QMap<PreferencesColor::FunctionColor,QColor> colorMap = preferencesColor.getFunctionColors();
	QColor scatterPlotColor = preferencesColor.getScatterPlotColor();
	QColor dishDiameterLineColor = preferencesColor.getDishDiameterLineColor();
	QColor zoomRectColor = preferencesColor.getZoomRectColor();
	plotHolder->setColors( colorMap, scatterPlotColor, dishDiameterLineColor, zoomRectColor );
}

void FeatherMain::imageFilesChanged(){
	QString lowResolutionPath = fileLoader.getFilePathLowResolution();
	QString highResolutionPath = fileLoader.getFilePathHighResolution();
	QString outputPath = fileLoader.getFilePathOutput();

	ui.lowResolutionLabel->setText( lowResolutionPath );
	ui.highResolutionLabel->setText( highResolutionPath );
	ui.outputLabel->setText( outputPath );
	bool imagesOK = loadImages();
	ui.featherButton->setEnabled( imagesOK );
	if ( !imagesOK ){
		QString msg( "Please check the images. They may not be appropriate to use with feather.");
		QMessageBox::warning( this, "Image Processing Problem", msg );
	}
	else {
		clearPlots();

		//Load the original data into the plot
		addOriginalDataToPlots();

		//Reset the dish diameters from the images
		resetDishDiameters();
	}
}

void FeatherMain::resetDishDiameters(){
	const int DEFAULT_DISH = -1;
	Float xDiam = DEFAULT_DISH;
	Float yDiam = DEFAULT_DISH;
	featherWorker.getEffectiveDishDiam( xDiam, yDiam );
	resetDishDiameter( ui.dishDiameterXLineEdit,
			ui.dishDiameterXLimitLabel, xDiam, DEFAULT_DISH);
	resetDishDiameter( ui.dishDiameterYLineEdit,
			ui.dishDiameterYLimitLabel, yDiam, DEFAULT_DISH);

	if ( xDiam != yDiam ){
		ui.yGroupBox->setChecked( true );
	}
}

void FeatherMain::resetDishDiameter( QLineEdit* dishEdit, QLabel* diamLimit,
		float value, float defaultValue ){
	if ( value != defaultValue ){
		QString diamValue = QString::number( value );
		dishEdit->setText( diamValue );
		diamLimit->setText( "<="+diamValue );
		diamLimit->setVisible( true );
	}
	else {
		dishEdit->setText( "" );
		diamLimit->setText( "" );
		diamLimit->setVisible( false );
	}
}

void FeatherMain::openFileLoader(){
	fileLoader.exec();
}

void FeatherMain::openPreferences(){
	preferences.exec();
}

void FeatherMain::openPreferencesColor(){
	preferencesColor.exec();
}

bool FeatherMain::loadImages(){
	bool imagesGenerated = true;
	lowResImagePath = ui.lowResolutionLabel->text();
	highResImagePath = ui.highResolutionLabel->text();
	if ( lowResImage != NULL ){
		delete lowResImage;
		lowResImage = NULL;
	}
	if ( highResImage != NULL ){
		delete highResImage;
		highResImage = NULL;
	}
	try {
		imagesGenerated = generateInputImage( lowResImagePath.toStdString(), highResImagePath.toStdString(), lowResImage, highResImage );
	}
	catch( AipsError& error ){
		imagesGenerated = false;
	}

	if ( imagesGenerated ){
		//Note:  high resolution image must be defined before low resolution
		//image or we get an exception.
		if ( highResImage != NULL && lowResImage != NULL ){
			featherWorker.setINTImage( *highResImage );
			featherWorker.setSDImage( *lowResImage );
		}
		else {
			imagesGenerated = false;
		}
	}
	return imagesGenerated;
}

void FeatherMain::addOriginalDataToPlots(){
	Vector<Float> sDxOrig;
	Vector<Float> sDxAmpOrig;
	Vector<Float> sDyOrig;
	Vector<Float> sDyAmpOrig;
	featherWorker.getFTCutSDImage( sDxOrig, sDxAmpOrig, sDyOrig, sDyAmpOrig );
	plotHolder->setSingleDishDataOriginal(sDxOrig, sDxAmpOrig, sDyOrig, sDyAmpOrig );

	Vector<Float> intxOrig;
	Vector<Float> intxAmpOrig;
	Vector<Float> intyOrig;
	Vector<Float> intyAmpOrig;
	featherWorker.getFTCutIntImage(intxOrig, intxAmpOrig, intyOrig, intyAmpOrig );
	plotHolder->setInterferometerDataOriginal( intxOrig, intxAmpOrig, intyOrig, intyAmpOrig );
}

void FeatherMain::clearPlots(){
	plotHolder->clearPlots();
}

void FeatherMain::featherImages() {

	Bool validDishDiameters = true;
	pair<float,float> dishDiameters = populateDishDiameters( validDishDiameters );

	if ( dishDiameters.first != DISH_DIAMETER_DEFAULT && validDishDiameters ){
		try {
			//At the moment, Feather.cc setEffectiveDishDiam always returns true,
			//and instead throws an exception if something is wrong, but we will
			//check the return value in case the code changes.
			validDishDiameters = featherWorker.setEffectiveDishDiam( dishDiameters.first, dishDiameters.second );
		}
		catch( AipsError& error ){
			String msg = error.getMesg();
			QString userMessage( msg.c_str() );
			QMessageBox::warning( this, "Invalid Dish Diameter", userMessage );
			return;
		}
	}
	if ( validDishDiameters ){

		float sdFactor = populateSDFactor();
		featherWorker.setSDScale( sdFactor );

		if ( thread != NULL ){
			delete thread;
			thread = NULL;
		}
		thread = new FeatherThread();
		connect( thread, SIGNAL( finished() ), this, SLOT(featheringDone()));
		thread->setFeatherWorker( &featherWorker );
		QString outputImagePath = ui.outputLabel->text();
		thread->setSaveOutput( fileLoader.isOutputSaved(), outputImagePath );
		thread->start();
		progressMeter.show();
	}
	else {
		QString msg("Effective dish diameter had a finer resolution than the original data.");
		QMessageBox::warning( this, "Resolution Problem", msg );
	}

}

void FeatherMain::featheringDone(){
	emit featherFinished();

	//Put the data into the graphs.
	plotHolder->clearPlots();
	plotHolder->setSingleDishWeight( thread->sDxWeight, thread->sDxAmpWeight, thread->sDyWeight, thread->sDyAmpWeight );
	plotHolder->setInterferometerWeight( thread->intxWeight, thread->intxAmpWeight, thread->intyWeight, thread->intyAmpWeight );
	plotHolder->setSingleDishData( thread->sDxCut, thread->sDxAmpCut, thread->sDyCut, thread->sDyAmpCut );
	plotHolder->setInterferometerData( thread->intxCut, thread->intxAmpCut, thread->intyCut, thread->intyAmpCut );
	//In case we are zoomed on the original data, this will reload it, unzoomed.
	addOriginalDataToPlots();
	plotHolder->updateScatterData();

	//Post a message if we could not save the output image.
	if ( fileLoader.isOutputSaved() ){
		bool fileSaved = thread->fileSaved;
		if ( !fileSaved ){
			QString msg( "There was a problem saving the image to "+ui.outputLabel->text());
			QMessageBox::warning( this, "Save Problem", msg);
		}
	}
}

float FeatherMain::populateSDFactor() const {
	QString sdFactorStr = ui.singleDishFactorLineEdit->text().trimmed();
	float sdFactor = SINGLE_DISH_FACTOR_DEFAULT;
	if ( sdFactorStr.length() > 0 ){
		sdFactor = sdFactorStr.toFloat();
	}
	return sdFactor;
}

pair<float,float> FeatherMain::populateDishDiameters( Bool& validDiameters ) {
	QString xDiameterStr = ui.dishDiameterXLineEdit->text().trimmed();
	QString yDiameterStr = ui.dishDiameterYLineEdit->text().trimmed();
	float xDiameter = DISH_DIAMETER_DEFAULT;
	float yDiameter = DISH_DIAMETER_DEFAULT;
	validDiameters = true;
	int xDiamLength = xDiameterStr.length();
	int yDiamLength = yDiameterStr.length();
	if ( xDiamLength > 0 && yDiamLength > 0 ){
		xDiameter = xDiameterStr.toFloat();
		yDiameter = yDiameterStr.toFloat();
	}
	else if ( xDiamLength * yDiamLength == 0 && (xDiamLength > 0 || yDiamLength > 0 )){
		QString warningMsg( "Effective dish diameters must be left blank or specified in BOTH directions");
		QMessageBox::warning( this, "Invalid Dish Diameters", warningMsg );
		validDiameters = false;
	}
	pair<float,float> diameters( xDiameter, yDiameter );
	return diameters;
}

bool FeatherMain::generateInputImage( const String& lowResImagePath, const String& highResImagePath,
		ImageInterface<Float>*& lowResImage, ImageInterface<Float>*& highResImage ){
	bool success = true;
	try {
		logger << LogIO::NORMAL
				<< "\nFeathering together high and low resolution images.\n"
				<< LogIO::POST;

		//Get initial images
		PagedImage<Float> highResImageTemp(highResImagePath);
		PagedImage<Float> lowResImageTemp(lowResImagePath);
		if(highResImageTemp.shape().nelements() != lowResImageTemp.shape().nelements()){
			String msg( "High and low resolution images do not have the same number of axes.");
			logger << LogIO::SEVERE  << msg << LogIO::EXCEPTION;
			QMessageBox::warning( this, "Warning!", msg.c_str());
			success = false;
		}
		else {
			lowResImage = new PagedImage<Float>(lowResImagePath);
			highResImage = new PagedImage<Float>(highResImagePath);
		}
	}
	catch (AipsError& x) {
		String msg = x.getMesg();
		logger << LogIO::SEVERE << "Caught exception: " << msg<< LogIO::EXCEPTION;
		success = false;;
	}
	return success;
}


FeatherMain::~FeatherMain(){

	delete lowResImage;
	delete highResImage;

	delete thread;
}
}
