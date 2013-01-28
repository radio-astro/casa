//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "SlicerMainWindow.qo.h"
#include <images/Images/ImageInterface.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <synthesis/MSVis/UtilJ.h>
#include <display/Slicer/SliceWorker.h>

#include <QDebug>
#include <qwt_plot_curve.h>

namespace casa {

SlicerMainWindow::SlicerMainWindow(QWidget *parent)
    : QMainWindow(parent), image(NULL), imageAnalysis( NULL),
      slicePlot( this ), DISTANCE_AXIS("Distance"),
      POSITION_X_AXIS("Position X"), POSITION_Y_AXIS("Position Y"){
	ui.setupUi(this);
	sliceCurveColor = Qt::blue;
	setWindowTitle( "1D Slice Tool");

	//Sample Size
	connect( ui.autoCountCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoCountChanged(bool)));
	ui.autoCountCheckBox->setChecked( true );

	axes.resize( 2 );
	axes[0] = 0;
	axes[1] = 1;

	//Interpolation Method
	methodList = QStringList() << "Nearest" << "Linear" << "Cubic";
	for ( int i = 0; i < methodList.size(); i++ ){
		ui.methodComboBox->addItem( methodList[i] );
	}

	//Add the plot to the widget
	QHBoxLayout* layout = new QHBoxLayout(ui.plotFrame);
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( &slicePlot );
	slicePlot.setCanvasBackground( Qt::white );
	slicePlot.setAxisTitle( QwtPlot::yLeft, "Intensity");
	ui.plotFrame->setLayout( layout );
	ui.plotFrame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	//X-Axis units
	ui.xAxisComboBox->addItem( DISTANCE_AXIS );
	ui.xAxisComboBox->addItem( POSITION_X_AXIS );
	ui.xAxisComboBox->addItem( POSITION_Y_AXIS );


	//Clear
	connect( ui.clearButton, SIGNAL(clicked()), this, SLOT(clearCurves()));
}

void SlicerMainWindow::autoCountChanged( bool selected ){
	ui.pointCountLineEdit->setEnabled( !selected );
	if ( !selected ){
		ui.pointCountLineEdit->setText( "0");
	}
}

void SlicerMainWindow::setAxisTitle( int axisId, const QString& title ){

}


SliceWorker* SlicerMainWindow::getSlicerFor( int regionId ){
	SliceWorker* slicer = NULL;
	if ( slicerMap.contains( regionId )){
		slicer = slicerMap[regionId];
	}
	else {
		slicer = new SliceWorker( regionId );
		slicerMap.insert( regionId, slicer );
	}
	return slicer;
}

int SlicerMainWindow::populateSampleCount() const {
	int sampleCount = 0;
	if ( !ui.autoCountCheckBox->isChecked()){
		QString countStr = ui.pointCountLineEdit->text();
		sampleCount = countStr.toInt();
	}
	return sampleCount;
}

String SlicerMainWindow::populateMethod() const {
	QString methodQ = ui.methodComboBox->currentText();
	methodQ = methodQ.toLower();
	return methodQ.toStdString();
}

void SlicerMainWindow::updatePolyLine(  int regionId,
		const QList<double> & /*worldX*/, const QList<double> & /*worldY*/,
		const QList<int> &pixelX, const QList<int> & pixelY ){
	int pixelXCount = pixelX.size();
	int pixelYCount = pixelY.size();
	if ( pixelXCount > 0 && pixelYCount > 0 ){
		Assert( pixelXCount == pixelYCount );
		polyX.resize( pixelXCount );
		polyY.resize( pixelYCount );
		for ( int i = 0; i < pixelXCount; i++ ){
			polyX[i] = pixelX[i];
			polyY[i] = pixelY[i];
		}
	}
	if ( imageAnalysis != NULL ){
		SliceWorker* slicer = getSlicerFor( regionId );
		slicer->setImageAnalysis( imageAnalysis );
		slicer->setSampleCount( populateSampleCount());
		slicer->setVertices( polyX, polyY );
		slicer->setAxes( axes );
		slicer->setCoords( coords );
		slicer->setMethod( populateMethod() );
		slicer->run();
		sliceFinished( regionId );
	}
}

void SlicerMainWindow::sliceFinished( int regionId ){
	SliceWorker* slicer = getSlicerFor( regionId );
	QString xAxisChoice = ui.xAxisComboBox->currentText();
	QVector<double> xValues;
	if ( xAxisChoice == DISTANCE_AXIS ){
		xValues = slicer->getDistances();

	}
	else if ( xAxisChoice == POSITION_X_AXIS ){
		xValues = slicer->getXPositions();
	}
	else {
		xValues = slicer->getYPositions();
	}
	slicePlot.setAxisTitle( QwtPlot::xBottom, xAxisChoice );
	QVector<double> pixels = slicer->getPixels();
	addSliceToPlot( xValues, pixels );
}

void SlicerMainWindow::deletePolyLine( int regionId){
	SliceWorker* slicer = slicerMap.take( regionId );
	delete slicer;
}

void SlicerMainWindow::updateChannel( int channel ){
	CoordinateSystem cSys = image->coordinates();
	int coordCount = image->ndim();
	coords.resize( coordCount );
	for ( int i = 0; i < coordCount; i++ ){
		coords[i] = 0;
	}
	if ( cSys.hasSpectralAxis() ){
		int index = cSys.spectralAxisNumber();
		coords[index] = channel;
	}
}

void SlicerMainWindow::clearCurves(){
	while( curveList.size() > 0 ){
		QwtPlotCurve* curve = curveList.takeLast();
		curve->detach();
		delete curve;
	}
}


void SlicerMainWindow::addSliceToPlot( QVector<double>& xValues,
			QVector<double>& yValues ){
	if ( !ui.accumulateCheckBox->isChecked()){
		clearCurves();
	}
	QwtPlotCurve* curve  = new QwtPlotCurve();
	curve->setData( xValues, yValues );
	QPen curvePen( sliceCurveColor );
	curvePen.setWidth( 1 );
	curve->setPen(curvePen);
	curve->attach(&slicePlot);
	slicePlot.replot();
	curveList.append( curve );
}

void SlicerMainWindow::setImage( ImageInterface<float>* img ){
	if ( img != NULL && image != img ){
		image = img;
		delete imageAnalysis;
		imageAnalysis = new ImageAnalysis( image );
		updateChannel( 0 );
	}
}


void SlicerMainWindow::clearSlicers(){
	QList<int> keys = slicerMap.keys();
	int keyCount = keys.size();
	for ( int i = 0; i < keyCount; i++ ){
		SliceWorker* slicer = slicerMap.take( keys[i]);
		delete slicer;
	}
}

SlicerMainWindow::~SlicerMainWindow(){
	clearSlicers();
}
}
