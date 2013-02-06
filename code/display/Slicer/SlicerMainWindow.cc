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
#include <display/Slicer/SliceZoomer.h>
#include <display/Slicer/SliceColorPreferences.qo.h>

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QCursor>
#include <QFile>
#include <qwt_plot_curve.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>

namespace casa {

SlicerMainWindow::SlicerMainWindow(QWidget *parent)
    : QMainWindow( parent ), image( NULL ), imageAnalysis( NULL ), colorPreferences( NULL ),
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

	//Color preferences
	colorPreferences = new SliceColorPreferences( this );
	connect( colorPreferences, SIGNAL( colorsChanged()), this, SLOT(resetColors()));
	resetColors();

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
	initAxisFont( QwtPlot::yLeft, "Intensity");

	//QwtPlotPanner* plotPanner = new QwtPlotPanner( slicePlot.canvas() );
	//plotPanner->setMouseButton( Qt::RightButton, Qt::NoButton);
	ui.plotFrame->setLayout( layout );
	ui.plotFrame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	//X-Axis units
	ui.xAxisComboBox->addItem( DISTANCE_AXIS );
	ui.xAxisComboBox->addItem( POSITION_X_AXIS );
	ui.xAxisComboBox->addItem( POSITION_Y_AXIS );

	//Clear
	connect( ui.clearButton, SIGNAL(clicked()), this, SLOT(clearCurves()));

	//Actions
	connect(ui.actionExport, SIGNAL(triggered()), this, SLOT(exportSlice()));
	connect(ui.actionColor, SIGNAL(triggered()), this, SLOT(showColorDialog()));

	initializeZooming();
}

void SlicerMainWindow::initializeZooming(){

	plotZoomer = new SliceZoomer( slicePlot.canvas() );
	connect( plotZoomer, SIGNAL(zoomed(const QwtDoubleRect&)), this, SLOT(checkZoom()));

	connect(ui.actionZoomIn, SIGNAL(triggered()), this, SLOT( zoomIn()));
	connect(ui.actionZoomOut, SIGNAL(triggered()), this, SLOT( zoomOut()));
	connect(ui.actionZoomNeutral, SIGNAL(triggered()), this, SLOT( zoomNeutral()));
}

bool SlicerMainWindow::checkZoom(){
	//We we revert a zoom back to the original, we won't see
	//the plot unless auto-scaling is turned back on.
	bool neutralState = false;
	if ( plotZoomer->zoomRectIndex() == 0 ){
		bool scaleChanged = false;
		if ( ! slicePlot.axisAutoScale( QwtPlot::xBottom )){
			slicePlot.setAxisAutoScale( QwtPlot::xBottom);
			scaleChanged = true;
		}
		if ( !slicePlot.axisAutoScale( QwtPlot::yLeft)){
			slicePlot.setAxisAutoScale( QwtPlot::yLeft);
			scaleChanged = true;
		}
		if ( scaleChanged ){
			slicePlot.replot();
		}
		neutralState = true;
	}
	return neutralState;
}

void SlicerMainWindow::zoomIn(){
	plotZoomer->zoomIn( );
}

void SlicerMainWindow::zoomOut(){
	plotZoomer->zoomOut();
}

void SlicerMainWindow::zoomNeutral(){
	if ( !checkZoom()){
		plotZoomer->zoomNeutral();
	}
}

void SlicerMainWindow::resetColors(){
	sliceCurveColor = colorPreferences->getSliceColor();
	QList<int> keys = curveMap.keys();
	int keyCount = keys.size();
	for ( int i = 0; i < keyCount; i++ ){
		QPen curvePen = curveMap[keys[i]]->pen();
		curvePen.setColor( sliceCurveColor );
		curveMap[keys[i]]->setPen(curvePen);
	}
	slicePlot.replot();
}

void SlicerMainWindow::showColorDialog(){
	colorPreferences->show();
}


void SlicerMainWindow::exportSlice(){
	QString fileName = QFileDialog::getSaveFileName(this,tr("Export 1D Slice"),
			QString(), tr( "Images (*.png *.jpg);; Text files (*.txt)"));
	if ( fileName.length() > 0 ){
		QString ext = fileName.section('.', -1);
		bool successfulExport = true;
		if (ext =="png" || ext == "jpg"){
			successfulExport = toImageFormat(fileName, ext );
		}
		else {
			if (ext != "txt"){
				fileName.append(".txt");
			}
			successfulExport = toASCII(fileName);
		}
	}
}

bool SlicerMainWindow::toASCII( const QString& fileName ){
	QFile file( fileName );
	bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
	const QString LINE_END( "\n");
	if ( success ){
		QTextStream out( &file );
		out.setFieldWidth( 20 );
		QList<int> keys = curveMap.keys();
		int keyCount = keys.size();
		String imageName = image->name();
		QString title = "1-D Slice(s) for "+QString(imageName.c_str());
		out << title << LINE_END << LINE_END;
		for ( int i = 0; i < keyCount; i++ ){
			if ( slicerMap.contains( keys[i])){
				slicerMap[keys[i]]->toAscii( out );
			}
			out << LINE_END;
			out.flush();
		}
		file.close();
	}
	return success;
}


bool SlicerMainWindow::toImageFormat( const QString& fileName, const QString& format ){
	QSize plotSize = ui.plotFrame->size();
	QPixmap pixmap(plotSize.width(), plotSize.height());
	pixmap.fill(Qt::white );
	QwtPlotPrintFilter filter;
	int options = QwtPlotPrintFilter::PrintFrameWithScales | QwtPlotPrintFilter::PrintBackground;
	filter.setOptions( options );
	slicePlot.print( pixmap, filter );
	bool imageSaved = pixmap.save( fileName, format.toStdString().c_str());
	if ( !imageSaved ){
		QString msg("There was a problem saving the histogram.\nPlease check the file path.");
		QMessageBox::warning( this, "Save Problem", msg);
	}
	return imageSaved;
}

void SlicerMainWindow::initAxisFont( int axisId, const QString& axisTitle ){
	const int AXIS_FONT_SIZE = 8;
	QwtText axisLabel( axisTitle );
	QFont axisFont = slicePlot.axisFont( axisId );
	axisFont.setPointSize( AXIS_FONT_SIZE );
	axisFont.setBold( true );
	axisLabel.setFont( axisFont );
	slicePlot.setAxisTitle( axisId, axisLabel );
}



void SlicerMainWindow::autoCountChanged( bool selected ){
	ui.pointCountLineEdit->setEnabled( !selected );
	if ( !selected ){
		ui.pointCountLineEdit->setText( "0");
	}
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
	initAxisFont( QwtPlot::xBottom, xAxisChoice );
	QVector<double> pixels = slicer->getPixels();
	addSliceToPlot( xValues, pixels, regionId );
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
	QList<int> keys = curveMap.keys();
	int keyCount = keys.size();
	for( int i = 0; i < keyCount; i++ ){
		QwtPlotCurve* curve = curveMap.take(keys[i]);
		curve->detach();
		delete curve;
	}
}


void SlicerMainWindow::addSliceToPlot( QVector<double>& xValues,
			QVector<double>& yValues, int regionId ){
	if ( !ui.accumulateCheckBox->isChecked()){
		clearCurves();
	}
	QwtPlotCurve* curve  = new QwtPlotCurve();
	curve->setData( xValues, yValues );
	QPen curvePen( sliceCurveColor );
	curvePen.setWidth( 1 );
	curve->setPen(curvePen);

	//We are plotting multiple curves and we now
	//have an update of one of them
	if ( curveMap.contains(regionId) ){
		curveMap[regionId]->detach();
		delete curveMap[regionId];
	}
	curveMap[regionId] = curve;

	curve->attach(&slicePlot);
	slicePlot.replot();
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
	delete plotZoomer;
}
}
