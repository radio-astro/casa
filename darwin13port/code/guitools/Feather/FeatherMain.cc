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
#include <guitools/Feather/FeatherDataType.h>
#include <guitools/Feather/FeatherManager.qo.h>

#include <images/Images/ImageUtilities.h>
#include <casa/Utilities/PtrHolder.h>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QMessageBox>
#include <limits>

namespace casa {

const int FeatherMain::DISH_DIAMETER_DEFAULT = -1;
const int FeatherMain::SINGLE_DISH_FACTOR_DEFAULT = 1;


FeatherMain::FeatherMain(QWidget *parent)
    : QMainWindow(parent), fileLoader( this ),
      preferences( this ), preferencesColor( this ),
      plotHolder(NULL), progressMeter(this), overWriteFileDialog( this ),
      logger(LogOrigin("CASA", "Feather")){

	ui.setupUi(this);
	ui.outputLabel->setText("");

	dataManager = new FeatherManager();
	connect( dataManager, SIGNAL(featheringDone()), this, SLOT( featheringDone()));

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

	connect( &overWriteFileDialog, SIGNAL( overWriteOK()), this, SLOT( overWriteOK()));

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

	//What to view
	bool legendVisible = preferences.isDisplayLegend();
	plotHolder->setLegendVisibility( legendVisible );
	plotHolder->setDisplayScatterPlot( preferences.isDisplayOutputScatterPlot());
	plotHolder->setDisplayOutputSlice( preferences.isDisplayOutputFunctions());
	plotHolder->setDisplayYGraphs( preferences.isDisplayY() );
	plotHolder->setDisplayXGraphs( preferences.isDisplayX() );

	//Whether to use a u/v or radial axis
	bool xAxisUV = preferences.isXAxisUV();
	bool oldAxisUV = !dataManager->isRadial();
	if ( xAxisUV != oldAxisUV ){
		//We only need one dish diameter box if we are plotting in
		//radial distance.
		if ( xAxisUV ){
			ui.yGroupBox->show();
			ui.xGroupBox->setTitle( "U");
		}
		else {
			ui.yGroupBox->hide();
			ui.xGroupBox->setTitle( "Distance");
		}
		plotHolder->setXAxisUV( xAxisUV );
		preferencesColor.setRadialPlot( !xAxisUV );
		//We have to reset the data if we are changing from a uv to a
		//radial plot
		dataManager->setRadial( !xAxisUV );
		featherImages();
	}

	//Single channel or averaged plane
	bool averaged = preferences.isPlaneAveraged();
	int planeIndex = preferences.getPlaneIndex();
	bool oldAveraged = dataManager->isChannelsAveraged();
	int oldPlaneIndex = dataManager->getChannelIndex();

	if ( averaged != oldAveraged ){
		dataManager->setChannelsAveraged( averaged );
	}
	if ( !averaged ){
		dataManager->setChannelIndex( planeIndex );
	}
	if ( (oldAveraged != averaged) ||
			(!averaged && (oldPlaneIndex != planeIndex)) ){
		plotHolder->clearPlots();
		resetData();
	}


	plotHolder->setLogScale( preferences.isLogUV(), preferences.isLogAmplitude() );
	plotHolder->refreshPlots();
	plotHolder->layoutPlotWidgets();
}

void FeatherMain::resetData(){
	if ( dataManager != NULL && dataManager->isReady()){
		addOriginalDataToPlots();
		addFeatheredDataToPlots();
		plotHolder->addSumData();
		plotHolder->updateScatterData();
	}
}

void FeatherMain::functionColorsChanged(){
	FeatherCurveType::CurveType xScatter = preferencesColor.getScatterXCurve();
	QList<FeatherCurveType::CurveType> yScatters = preferencesColor.getScatterYCurve();
	plotHolder->setScatterCurves( xScatter, yScatters );

	QMap<PreferencesColor::CurveType,CurveDisplay> colorMap = preferencesColor.getFunctionColors();
	plotHolder->setColors( colorMap );

	//Enable/disable the toggle button for the dish diameter
	bool dishDiameterEnabled = colorMap[FeatherCurveType::DISH_DIAMETER].isDisplayed();
	ui.actionDiameterSelector->setEnabled( dishDiameterEnabled );
}

QString FeatherMain::getFileName( QString path ) const {
	QString fileName = path;
	int slashIndex = path.lastIndexOf( QDir::separator() );
	if ( slashIndex >= 0 ){
		fileName = fileName.mid(slashIndex + 1);
	}
	return fileName;
}



void FeatherMain::updatePlaneInformation(){
	int planeCount = dataManager->getPlaneCount();
	preferences.setPlaneCount( planeCount );

}

void FeatherMain::imageFilesChanged(){
	//Store the full path
	lowResImagePath = fileLoader.getFilePathLowResolution();
	highResImagePath = fileLoader.getFilePathHighResolution();
	outputImagePath = fileLoader.getFilePathOutput();
	dirtyImagePath = fileLoader.getFileDirty();

	//Only show the file names on the GUI.
	ui.lowResolutionLabel->setText( getFileName(lowResImagePath) );
	ui.highResolutionLabel->setText( getFileName( highResImagePath ));
	ui.outputLabel->setText( getFileName( outputImagePath ));

	if ( highResImagePath.trimmed().length() > 0 && lowResImagePath.trimmed().length() > 0 ){



		bool imagesOK = dataManager->loadImages( highResImagePath, lowResImagePath, &logger );
		bool dirtyImageLoaded = false;
		if ( imagesOK && dirtyImagePath.trimmed().length() > 0 ){
			dirtyImageLoaded = dataManager->loadDirtyImage( dirtyImagePath);
		}
		preferencesColor.setDirtyEnabled( dirtyImageLoaded );

		//Feathering
		ui.featherButton->setEnabled( imagesOK );
		if ( imagesOK ){

			updatePlaneInformation();

			clearPlots();

			//Reset the dish diameters from the images
			resetDishDiameters();
		}
		else {
			QString error = dataManager->getError();
			QMessageBox::warning( this, "Warning!", error );
		}
	}
}

void FeatherMain::resetDishDiameters(){
	const int DEFAULT_DISH = -1;
	Float xDiam = DEFAULT_DISH;
	Float yDiam = DEFAULT_DISH;
	dataManager->getEffectiveDishDiameter( xDiam, yDiam );
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
	preferences.show();
}

void FeatherMain::openPreferencesColor(){
	//We call show rather than exec here so that users can keep this one up
	//and turn on/off curves
	preferencesColor.show();
}

void FeatherMain::clearPlots(){
	plotHolder->clearPlots();
}


void FeatherMain::featherImages( bool checkOutput ) {

	if ( lowResImagePath.trimmed().length() == 0 ){
		return;
	}

	//If we are supposed to save output, we need to check that there is not another
	//file there.  If there is, we need to put up a warning asking if we should overwrite
	//the file our save with another name.
	bool saveOutput = fileLoader.isOutputSaved();
	if ( saveOutput && checkOutput ){
		QFile outputFile( outputImagePath );
		bool outputOverwrite = outputFile.exists();
		if ( outputOverwrite ){
			int lastSlash = outputImagePath.lastIndexOf( QDir::separator());
			QString fileName = "";
			QString directory = outputImagePath;
			if ( lastSlash >= 0 ){
				directory = outputImagePath.mid(lastSlash+1);
				fileName = outputImagePath.left( lastSlash );
			}
			overWriteFileDialog.setFile( fileName );
			overWriteFileDialog.setDirectory( directory );
			overWriteFileDialog.show();
			return;
		}
	}

	Bool validDishDiameters = true;
	pair<float,float> dishDiameters = populateDishDiameters( validDishDiameters );

	if ( dishDiameters.first != DISH_DIAMETER_DEFAULT && validDishDiameters ){
		try {
			//At the moment, Feather.cc setEffectiveDishDiam always returns true,
			//and instead throws an exception if something is wrong, but we will
			//check the return value in case the code changes.
			validDishDiameters = dataManager->setEffectiveDishDiameter( dishDiameters.first, dishDiameters.second );
		}
		catch( AipsError& error ){
			String msg = error.getMesg();
			QString userMessage( msg.c_str() );
			QMessageBox::warning( this, "Invalid Dish Diameter", userMessage );
			return;
		}
	}
	if ( validDishDiameters ){
		plotHolder->clearData();
		float sdFactor = populateSDFactor();
		dataManager->setSDScale( sdFactor );
		progressMeter.show();
		dataManager->applyFeather( fileLoader.isOutputSaved(), outputImagePath );
	}
	else {
		QString msg("Effective dish diameter had a finer resolution than the original data.");
		QMessageBox::warning( this, "Resolution Problem", msg );
	}

}



void FeatherMain::addOriginalDataToPlots(){

	FeatheredData sDOrig = dataManager->getSDOrig();
	plotHolder->setData(sDOrig.getUX(), sDOrig.getUY(),
				sDOrig.getVX(), sDOrig.getVY(), FeatherDataType::LOW );
	if ( sDOrig.isEmpty()){
		QMessageBox::warning(this,"Single Dish Data Missing", "There was a  loading single dish data.");
	}


	FeatheredData intOrig = dataManager->getIntOrig();
	plotHolder->setData( intOrig.getUX(), intOrig.getUY(),
				intOrig.getVX(), intOrig.getVY(), FeatherDataType::HIGH );
	if ( intOrig.isEmpty() ){
		QMessageBox::warning(this,"Interferometer Data Missing", "There was a problem loading interferometer data.");
	}

	FeatheredData dirtyOrig = dataManager->getDirtyOrig();
	plotHolder->setData( dirtyOrig.getUX(), dirtyOrig.getUY(),
		dirtyOrig.getVX(), dirtyOrig.getVY(), FeatherDataType::DIRTY );
	if ( dirtyOrig.isEmpty() && dirtyImagePath.length() > 0 ){
		QMessageBox::warning(this,"Dirty Data Missing", "There was a problem loading the data.");
	}

}

void FeatherMain::addFeatheredDataToPlots( ){
	//Put the data calculated from feather into the graph.
	FeatheredData sdWeight = dataManager->getSDWeight();
	plotHolder->setData( sdWeight.getUX(), sdWeight.getUY(),
			sdWeight.getVX(), sdWeight.getVY(), FeatherDataType::WEIGHT_SD );

	FeatheredData intWeight = dataManager->getIntWeight();
	plotHolder->setData( intWeight.getUX(), intWeight.getUY(),
			intWeight.getVX(), intWeight.getVY(), FeatherDataType::WEIGHT_INT );

	FeatheredData sdCut = dataManager->getSDCut();
	plotHolder->setData( sdCut.getUX(), sdCut.getUY(), sdCut.getVX(), sdCut.getVY(), FeatherDataType::LOW_WEIGHTED );

	FeatheredData intCut = dataManager->getIntCut();
	plotHolder->setData( intCut.getUX(), intCut.getUY(), intCut.getVX(), intCut.getVY(), FeatherDataType::HIGH_WEIGHTED );

	FeatheredData dirtyCut = dataManager->getDirtyCut();
	plotHolder->setData( dirtyCut.getUX(), dirtyCut.getUY(), dirtyCut.getVX(), dirtyCut.getVY(), FeatherDataType::DIRTY_WEIGHTED );

	FeatheredData intConvolvedSD = dataManager->getIntConvolvedSDOrig();
	plotHolder->setData( intConvolvedSD.getUX(), intConvolvedSD.getUY(),
		intConvolvedSD.getVX(), intConvolvedSD.getVY(), FeatherDataType::HIGH_CONVOLVED_LOW );

	FeatheredData intConvolvedSDCut = dataManager->getIntConvolvedSDCut();
	plotHolder->setData( intConvolvedSDCut.getUX(), intConvolvedSDCut.getUY(),
		intConvolvedSDCut.getVX(), intConvolvedSDCut.getVY(), FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED );

	FeatheredData dirtyConvolvedSD = dataManager->getDirtyConvolvedSDOrig();
	plotHolder->setData( dirtyConvolvedSD.getUX(), dirtyConvolvedSD.getUY(),
		dirtyConvolvedSD.getVX(), dirtyConvolvedSD.getVY(), FeatherDataType::DIRTY_CONVOLVED_LOW );

	FeatheredData dirtyConvolvedSDCut = dataManager->getDirtyConvolvedSDCut();
	plotHolder->setData( dirtyConvolvedSDCut.getUX(), dirtyConvolvedSDCut.getUY(),
		dirtyConvolvedSDCut.getVX(), dirtyConvolvedSDCut.getVY(), FeatherDataType::DIRTY_CONVOLVED_LOW_WEIGHTED );

	FeatheredData sdConvolvedInt = dataManager->getSDConvolvedIntOrig();
	plotHolder->setData( sdConvolvedInt.getUX(), sdConvolvedInt.getUY(),
			sdConvolvedInt.getVX(), sdConvolvedInt.getVY(), FeatherDataType::LOW_CONVOLVED_HIGH );

	FeatheredData sdConvolvedIntCut = dataManager->getSDConvolvedIntCut();
	plotHolder->setData( sdConvolvedIntCut.getUX(), sdConvolvedIntCut.getUY(),
			sdConvolvedIntCut.getVX(), sdConvolvedIntCut.getVY(), FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED );

	/*FeatheredData sdConvolvedDirty = dataManager->getSDConvolvedDirtyOrig();
	plotHolder->setData( sdConvolvedDirty.getUX(), sdConvolvedDirty.getUY(),
				sdConvolvedDirty.getVX(), sdConvolvedDirty.getVY(), FeatherDataType::LOW_CONVOLVED_DIRTY );

	FeatheredData sdConvolvedDirtyCut = dataManager->getSDConvolvedDirtyCut();
	plotHolder->setData( sdConvolvedDirtyCut.getUX(), sdConvolvedDirtyCut.getUY(),
				sdConvolvedDirtyCut.getVX(), sdConvolvedDirtyCut.getVY(), FeatherDataType::LOW_CONVOLVED_DIRTY_WEIGHTED );
	*/
}


void FeatherMain::featheringDone(){
	emit featherFinished();

	bool successful = dataManager->isSuccess();
	if ( !successful ){
		QString errorMessage = dataManager->getError();
		QMessageBox::warning( this, "Feathering Problem", errorMessage );
	}
	else {

		//Remove all plots.
		plotHolder->clearPlots();

		resetData();

		plotHolder->layoutPlotWidgets();
		plotHolder->refreshPlots();

		//Post a message if we could not save the output image.
		if ( fileLoader.isOutputSaved() ){
			bool fileSaved = dataManager->isFileSaved();
			if ( !fileSaved ){
				QString msg( "There was a problem saving the image to "+ui.outputLabel->text());
				QMessageBox::warning( this, "Save Problem", msg);
			}
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

void FeatherMain::overWriteOK() {
	QString outputDirectory = overWriteFileDialog.getDirectory();
	QString outputFileName = overWriteFileDialog.getFile();
	fileLoader.updateOutput( outputDirectory, outputFileName );
	if ( !outputDirectory.endsWith( QDir::separator())){
		outputDirectory = outputDirectory + QDir::separator();
	}
	outputImagePath = outputDirectory + outputFileName;
	ui.outputLabel->setText( outputFileName );
	//Reapply feather now without doing the output file check.
	featherImages( false );
}

FeatherMain::~FeatherMain(){
	delete dataManager;
}
}
