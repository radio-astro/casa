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
#include "HistogramMain.qo.h"
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <guitools/Histogram/ColorPreferences.qo.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <QMessageBox>
#include <QCloseEvent>

namespace casa {

HistogramMain::HistogramMain(bool showFileLoader, bool fitControls,
		bool rangeControls, bool plotModeControls, QWidget *parent)
    : QMainWindow(parent), fileLoader(this),
      plotWidget( NULL ),
      logger(LogOrigin("CASA", "Histogram")){

	ui.setupUi(this);
	setWindowTitle( "Histogram Tool");

	QLayout* mainLayout = ui.centralwidget->layout();
	if ( mainLayout == NULL ){
		mainLayout = new QHBoxLayout();
	}
	mainLayout->setContentsMargins(5,1,1,1);
	plotWidget = new BinPlotWidget( fitControls, rangeControls,
			plotModeControls, this );
	mainLayout->addWidget( plotWidget );
	ui.centralwidget->setLayout( mainLayout );

	//Set-up the initial colors
	preferencesColor = new ColorPreferences( this );
	preferencesColor->setFitColorsVisible( fitControls );
	colorsChanged();

	if ( !showFileLoader ){
		ui.toolBar->removeAction( ui.actionImageFile );
	}
	else {
		connect(ui.actionImageFile, SIGNAL(triggered()), this, SLOT(openFileLoader()));
		connect( &fileLoader, SIGNAL(imageFileChanged()), this, SLOT(imageFileChanged()));
	}
	connect(ui.actionColor, SIGNAL(triggered()), this, SLOT(openColorPreferences()));
	connect( preferencesColor, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(openHistogramSaver()));
	connect( &histogramSaver, SIGNAL(savePing(const QString&,int,int)), plotWidget, SLOT(toPing(const QString&,int,int)));
	connect( &histogramSaver, SIGNAL(saveAscii(const QString&)), plotWidget, SLOT(toAscii(const QString&)));
	connect( plotWidget, SIGNAL(postStatusMessage(const QString&)), this, SLOT(postStatusMessage(const QString&)));
	connect( plotWidget, SIGNAL(rangeChanged()), this, SIGNAL(rangeChanged()));


	plotWidget->addZoomActions( rangeControls, ui.menuZoom );
	plotWidget->addDisplayActions( ui.menuDisplay, NULL );
	if ( plotModeControls ){
		plotWidget->addPlotModeActions( ui.menuConfigure, NULL, NULL);
	}
	else {
		preferencesColor->setMultipleHistogramColorsVisible( false );
	}
	plotWidget->setDisplayPlotTitle( true );
	plotWidget->setDisplayAxisTitles( true );
}

bool HistogramMain::setImage(shared_ptr<const ImageInterface<Float> > img ){
	bool imageSet = plotWidget->setImage( img );
	return imageSet;
}

bool HistogramMain::setImageRegion( ImageRegion* imageRegion, int id ){
	bool regionSet = plotWidget->setImageRegion( imageRegion, id );
	return regionSet;
}

void HistogramMain::deleteImageRegion( int id ){
	plotWidget->deleteImageRegion( id );
}

void HistogramMain::imageRegionSelected( int id ){
	plotWidget->imageRegionSelected( id );
}



pair<double,double> HistogramMain::getRange() const {
	return plotWidget->getMinMaxValues();
}

void HistogramMain::setDisplayPlotTitle( bool display ){
	plotWidget->setDisplayPlotTitle( display );
}

void HistogramMain::setDisplayAxisTitles( bool display ){
	plotWidget->setDisplayAxisTitles( display );
}

void HistogramMain::setPlotMode( int mode ){
	plotWidget->setPlotMode( mode );
}

void HistogramMain::setChannelCount( int count ){
	plotWidget->setChannelCount( count );
}

void HistogramMain::setChannelValue( int value ){
	plotWidget->setChannelValue( value );
}

void HistogramMain::openFileLoader(){
	fileLoader.exec();
}

void HistogramMain::openHistogramSaver(){
	if ( ! plotWidget->isEmpty() ){
		histogramSaver.exec();
	}
	else {
		QMessageBox::warning(this, "Histogram Empty", "There is no data in the histogram to save.");
	}
}

void HistogramMain::openColorPreferences(){
	preferencesColor->exec();
}

void HistogramMain::colorsChanged(){
	QColor histogramColor = preferencesColor->getHistogramColor();
	QColor fitCurveColor = preferencesColor->getFitCurveColor();
	QColor fitEstimateColor = preferencesColor->getFitEstimateColor();
	QList<QColor> multipleHistogramColors = preferencesColor->getMultipleHistogramColors();
	plotWidget->setHistogramColor( histogramColor );
	plotWidget->setFitCurveColor( fitCurveColor );
	plotWidget->setFitEstimateColor( fitEstimateColor );
	plotWidget->setMultipleHistogramColors( multipleHistogramColors );
}

void HistogramMain::imageFileChanged(){
	QString imageFile = fileLoader.getFilePath();
    shared_ptr<const ImageInterface<Float> > image;
	bool success = generateImage( imageFile, image );
	if ( success ){
		/*bool histogramSet =*/ plotWidget->setImage( image );
	}
	else {
		QString msg( "Please check that the file "+imageFile+" represents a valid image.");
		QMessageBox::warning( this, "Problem Loading Image", msg );
	}
}

bool HistogramMain::generateImage( const QString& imagePath,
		shared_ptr<const ImageInterface<Float> > image ) {
	bool success = true;
	try {
		String filePath = imagePath.toStdString();
		logger << LogIO::NORMAL
				<< "\nLoading Image: "<<filePath<<"\n"
				<< LogIO::POST;
		image.reset(new PagedImage<Float>(filePath));
	}
	catch (AipsError x) {
		String msg = x.getMesg();
		logger << LogIO::SEVERE << "Caught exception: " << msg<< LogIO::EXCEPTION;
		success = false;;
	}
	return success;
}

void HistogramMain::postStatusMessage( const QString& statusMsg ){
	ui.statusbar->showMessage( statusMsg, 60000 );
}

void HistogramMain::closeEvent( QCloseEvent* event){
	plotWidget->clearAll();
	emit closing();
	event->accept();
}


HistogramMain::~HistogramMain(){
}
}
