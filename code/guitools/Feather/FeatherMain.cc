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
	featherWorker->getFTCutSDImage( sDxOrig, sDxAmpOrig, sDyOrig, sDyAmpOrig );
	featherWorker->getFTCutIntImage(intxOrig, intxAmpOrig, intyOrig, intyAmpOrig );

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
	QDoubleValidator* doubleValidator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 10, this );
	ui.dishDiameterXLineEdit->setValidator( doubleValidator );
	ui.dishDiameterYLineEdit->setValidator( doubleValidator );
	ui.singleDishFactorLineEdit->setValidator( doubleValidator );
	connect( ui.dishDiameterXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dishDiameterXChanged(const QString&)));
	connect( ui.dishDiameterYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dishDiameterYChanged(const QString&)));


	//Takeout
	//ui.highResolutionLabel->setText( "/home/uniblab/casa/active/test/orion_vlamem.im");
	//ui.lowResolutionLabel->setText( "/home/uniblab/casa/active/test/orion_gbt.im");
	//ui.outputLabel->setText("/home/uniblab/casa/active/test/feather.image");
	ui.featherButton->setEnabled( false );

	functionColorsChanged();
	preferencesChanged();

	connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
	connect(ui.actionImageFiles, SIGNAL(triggered()), this, SLOT(openFileLoader()));
	connect(ui.actionPreferencesColor, SIGNAL(triggered()), this, SLOT(openPreferencesColor()));
	//connect(ui.actionRectangleZoom, SIGNAL(triggered()), this, SLOT(setRectangleZoomMode()));
	//connect(ui.actionDiameterSelector, SIGNAL(triggered()), this, SLOT(setDiameterSelectorMode()));
	connect( &fileLoader, SIGNAL(imageFilesChanged()), this, SLOT(imageFilesChanged()));
	connect( &preferencesColor,SIGNAL(colorsChanged()), this, SLOT(functionColorsChanged()));
	connect(ui.featherButton, SIGNAL(clicked()), this, SLOT( featherImages()));
	connect( ui.yGroupBox, SIGNAL(toggled(bool)), this, SLOT(ySupportChanged(bool)));
	connect(&preferences, SIGNAL(preferencesChanged()), this, SLOT(preferencesChanged()));
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
	plotHolder->setColors( colorMap, scatterPlotColor, dishDiameterLineColor );
}

void FeatherMain::imageFilesChanged(){
	QString lowResolutionPath = fileLoader.getFilePathLowResolution();
	QString highResolutionPath = fileLoader.getFilePathHighResolution();
	QString outputPath = fileLoader.getFilePathOutput();
	ui.lowResolutionLabel->setText( lowResolutionPath );
	ui.highResolutionLabel->setText( highResolutionPath );
	ui.outputLabel->setText( outputPath );
	ui.featherButton->setEnabled( true );
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

bool FeatherMain::isInputImagesChanged(){
	bool imagesChanged = false;
	if ( ui.lowResolutionLabel->text() != lowResImagePath ){
		imagesChanged = true;
	}
	else if ( ui.highResolutionLabel->text() != highResImagePath ){
		imagesChanged = true;
	}
	return imagesChanged;
}

void FeatherMain::featherImages() {
	bool imagesChanged = isInputImagesChanged();
	bool imagesGenerated = true;
	if ( imagesChanged ){
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
	}

	if ( imagesGenerated ){
		if ( imagesChanged ){
			//Note:  high resolution image must be defined before low resolution
			//image or we get an exception.
			featherWorker.setINTImage( *highResImage );
			featherWorker.setSDImage( *lowResImage );
		}

		Bool validDishDiameters = true;
		pair<float,float> dishDiameters = populateDishDiameters( validDishDiameters );

		if ( dishDiameters.first != DISH_DIAMETER_DEFAULT && validDishDiameters ){
			validDishDiameters = featherWorker.setEffectiveDishDiam( dishDiameters.first, dishDiameters.second );
		}
		if ( validDishDiameters ){

			float sdFactor = populateSDFactor();
			//sdFactor = 0.999f;
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
			//Getting the data should not take any time at all unless the images
			//changed.
			if ( imagesChanged ){
				progressMeter.show();
			}
		}
		else {
			QString msg("Effective dish diameter had a finer resolution than the original data.");
			QMessageBox::warning( this, "Resolution Problem", msg );
		}
	}
	else {
		QString msg( "Please check the images. They may not be appropriate to use with feather.");
		QMessageBox::warning( this, "Image Processing Problem", msg );
	}
}

void FeatherMain::featheringDone(){
	emit featherFinished();

	//Put the data into the graphs.
	plotHolder->setSingleDishWeight( thread->sDxWeight, thread->sDxAmpWeight, thread->sDyWeight, thread->sDyAmpWeight );
	plotHolder->setInterferometerWeight( thread->intxWeight, thread->intxAmpWeight, thread->intyWeight, thread->intyAmpWeight );
	plotHolder->setSingleDishData( thread->sDxCut, thread->sDxAmpCut, thread->sDyCut, thread->sDyAmpCut );
	plotHolder->setInterferometerData( thread->intxCut, thread->intxAmpCut, thread->intyCut, thread->intyAmpCut );
	plotHolder->setSingleDishDataOriginal( thread->sDxOrig, thread->sDxAmpOrig, thread->sDyOrig, thread->sDyAmpOrig );
	plotHolder->setInterferometerDataOriginal( thread->intxOrig, thread->intxAmpOrig, thread->intyOrig, thread->intyAmpOrig );
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
		bool noStokes=False;
		String outLowRes = lowResImagePath;
		String outHighRes = highResImagePath;
		logger << LogIO::NORMAL
				<< "\nFeathering together high and low resolution images.\n"
				<< LogIO::POST;

		//Get initial images
		//Deal with images that don't have stokes.
		PagedImage<Float> highResImageTemp(highResImagePath);
		PagedImage<Float> lowResImageTemp(lowResImagePath);
		if(highResImageTemp.shape().nelements() != lowResImageTemp.shape().nelements()){
			String msg( "High and low resolution images do not have the same number of axes.");
			logger << LogIO::SEVERE  << msg << LogIO::EXCEPTION;
			QMessageBox::warning( this, "Warning!", msg.c_str());
			success = false;
		}
		else {
			if ( (highResImageTemp.coordinates().findCoordinate(Coordinate::STOKES) < 0) &&
					(lowResImageTemp.coordinates().findCoordinate(Coordinate::STOKES) < 0)){
				noStokes=True;
				String msg("Making some temporary images as the inputs have no Stokes axis.\n");
				logger << LogIO::NORMAL << msg << LogIO::POST;

				PtrHolder<ImageInterface<Float> > outImage1;
				outHighRes= highResImagePath+"_stokes";
				ImageUtilities::addDegenerateAxes (logger, outImage1, highResImageTemp, outHighRes,
						False, False,
						"I", False, False,
						False);

				PtrHolder<ImageInterface<Float> > outImage2;
				outLowRes= lowResImagePath+"_stokes";
				ImageUtilities::addDegenerateAxes (logger, outImage2, lowResImageTemp, outLowRes,
						False, False,
						"I", False, False,
						False);

			}

			lowResImage = new PagedImage<Float>(outLowRes);
			highResImage = new PagedImage<Float>(outHighRes);
			if(noStokes){
				Table::deleteTable(outHighRes);
				Table::deleteTable(outLowRes);
			}

		}
	}
	catch (AipsError x) {
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
