#include "FeatherPlotWidget.qo.h"

#include <QDebug>
#include <QContextMenuEvent>

namespace casa {
FeatherPlotWidget::FeatherPlotWidget(const QString& title, bool origData, QWidget *parent)
    : QWidget(parent), plot( NULL ),
      singleDishWeightColor(Qt::cyan), singleDishDataColor(Qt::green),
      interferometerWeightColor( Qt::magenta), interferometerDataColor(Qt::blue),
      plotTypeAction( "Scatter Plot", this ), zoom90Action( "Zoom 90% of Amplitude", this),
      zoomNeutralAction( "Zoom Neutral", this ),contextMenu( this ),
      singleDishFunction( "Single Dish"), interferometerFunction( "Interferometer"),
      singleDishWeightFunction("Single Dish Weight"), interferometerWeightFunction( "Interferometer Weight"),
      singleDishDataFunction("Single Dish Slice Cut"), interferometerDataFunction( "Interferometer Slice Cut"),

      mouseMove( false ), legendVisible( true ), originalData( origData ), lineThickness(1),
      plotTitle(title){
	ui.setupUi(this);

	if ( !originalData ){
		resetPlot( FeatherPlot::SLICE_CUT );
	}
	else {
		resetPlot( FeatherPlot::ORIGINAL );
	}
	initializeActions();
}

void FeatherPlotWidget::initializeActions(){

	plotTypeAction.setStatusTip( "Toggle between a single-dish/interferometer amplitude scatter plot and function slice cuts.");
	plotTypeAction.setCheckable( true );
	connect( &plotTypeAction, SIGNAL(triggered()), this, SLOT(changePlotType()));

	zoom90Action.setStatusTip( "Selecting this will zoom to include 90% of amplitude.");
	zoom90Action.setCheckable( true );
	connect( &zoom90Action, SIGNAL(triggered()), this, SLOT(changeZoom90()));

	zoomNeutralAction.setStatusTip( "Restore the graph back to its original non-zoomed state");
	connect( &zoomNeutralAction, SIGNAL(triggered()), this, SLOT(zoomNeutral()));

	if ( !originalData ){
		contextMenu.addAction( &plotTypeAction );
		contextMenu.addAction( &zoom90Action );
	}
	contextMenu.addAction( &zoomNeutralAction );
}

bool FeatherPlotWidget::getPracticalLegendVisibility() const {
	//Whether or not we actual show a legend depends on the plot type (scatter plots
	//don't need a legend) and on whether the user wants one.
	bool scatterPlot = plotTypeAction.isChecked();
	bool actualVisibility = legendVisible && !scatterPlot;
	return actualVisibility;
}

void FeatherPlotWidget::changePlotType(){
	bool scatterPlot = plotTypeAction.isChecked();
	zoom90Action.setEnabled( !scatterPlot );
	if ( scatterPlot ){
		resetPlot( FeatherPlot::SCATTER_PLOT );
		addZoomNeutralScatterPlot();
		zoom90Action.setChecked( false );
		qDebug() << "Scatter plot";
	}
	else {
		resetPlot( FeatherPlot::SLICE_CUT );
		addZoomNeutralCurves();
	}
}

void FeatherPlotWidget::setLineThickness( int thickness ){
	lineThickness = thickness;
	plot->setLineThickness( thickness );
	plot->replot();
}

void FeatherPlotWidget::setLegendVisibility( bool visible ){
	legendVisible = visible;
	//Override if we are displaying a scatter plot.  Scatter plots
	//should never have a legend.
	bool actualVisibility = this->getPracticalLegendVisibility();
	plot->setLegendVisibility( actualVisibility );
	plot->replot();
}

void FeatherPlotWidget::setPlotColors( QColor dataSDColor, QColor dataINTColor,
		QColor weightSDColor, QColor weightINTColor, QColor scatterPlotColor ){
	singleDishDataColor = dataSDColor;
	interferometerDataColor = dataINTColor;
	singleDishWeightColor = weightSDColor;
	interferometerWeightColor = weightINTColor;
	this->scatterPlotColor = scatterPlotColor;
	resetColors();
}

void FeatherPlotWidget::resetColors(){
	FeatherPlot::PlotType plotType = plot->getPlotType();
	if ( plotType == FeatherPlot::SLICE_CUT ){
		plot->setFunctionColor( singleDishDataFunction, singleDishDataColor );
		plot->setFunctionColor( interferometerDataFunction, interferometerDataColor );
		plot->setFunctionColor( singleDishWeightFunction, singleDishWeightColor );
		plot->setFunctionColor( interferometerWeightFunction, interferometerWeightColor );
	}
	else if ( plotType == FeatherPlot::SCATTER_PLOT ){
		plot->setFunctionColor( "", scatterPlotColor );
	}
	else {
		plot->setFunctionColor( this->singleDishFunction, singleDishDataColor );
		plot->setFunctionColor( this->interferometerFunction, interferometerDataColor );
	}
	plot->replot();
}


pair<double,double> FeatherPlotWidget::getMaxMin( QVector<double> values ) const {
	double minValue = numeric_limits<double>::max();
	double maxValue = numeric_limits<double>::min();
	for ( int i = 0; i < values.size(); i++ ){
		if ( values[i]< minValue ){
			minValue = values[i];
		}
		else if ( values[i] > maxValue ){
			maxValue = values[i];
		}
	}
	pair<double,double> minMaxPair( minValue, maxValue );
	return minMaxPair;
}

void FeatherPlotWidget::changeZoom90(){
	qDebug() << "Changing 90% zoom level";
	if ( zoom90Action.isChecked()){
		//plot->zoomNeutral();
		pair<double,double> singleDishMinMax = getMaxMin( singleDishWeightYValues );
		pair<double,double> interferometerMinMax = getMaxMin( interferometerWeightYValues );
		double minValue = qMin( singleDishMinMax.first, interferometerMinMax.first );
		double maxValue = qMax( singleDishMinMax.second, interferometerMinMax.second );
		double increase = (maxValue - minValue) * .25;
		minValue = minValue + increase;
		maxValue = maxValue - increase;
		qDebug() << "New minValue="<<minValue<<" new maxValue="<<maxValue;

		Double minX = numeric_limits<double>::max();
		Double maxX = numeric_limits<double>::min();
		QVector<double> singleDishXZoom;
		QVector<double> singleDishYZoom;
		initializeRangeLimitedData( minValue, maxValue, singleDishXZoom, singleDishYZoom,
				singleDishWeightXValues, singleDishWeightYValues, &minX, &maxX );
		plot->addCurve( singleDishXZoom, singleDishYZoom, singleDishWeightColor, singleDishWeightFunction, QwtPlot::yLeft );

		QVector<double> interferometerXZoom;
		QVector<double> interferometerYZoom;
		initializeRangeLimitedData( minValue, maxValue, interferometerXZoom, interferometerYZoom,
					interferometerWeightXValues, interferometerWeightYValues, &minX, &maxX );
		plot->addCurve( interferometerXZoom, interferometerYZoom, interferometerWeightColor, interferometerWeightFunction, QwtPlot::yLeft );

		qDebug() << "Minx="<<minX<<" maxx="<<maxX;
		QVector<double> singleDishX;
		QVector<double> singleDishY;
		initializeDomainLimitedData( minX, maxX, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
		plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishDataFunction, QwtPlot::yRight );

		QVector<double> interferometerX;
		QVector<double> interferometerY;
		initializeDomainLimitedData( minX, maxX, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
		plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerDataFunction, QwtPlot::yRight );
		plot->replot();
	}
	else {
		zoomNeutral();
	}
}


void FeatherPlotWidget::contextMenuEvent( QContextMenuEvent* event ){
	contextMenu.exec( event->globalPos());
}

void FeatherPlotWidget::initializeRangeLimitedData( double minValue, double maxValue,
		QVector<double>& xValues, QVector<double>& yValues,
		const QVector<double>& originalXValues, const QVector<double>& originalYValues,
		Double* minX, Double* maxX) const{
	int count = qMin( originalXValues.size(), originalYValues.size());
	int rangeCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalYValues[i] && originalYValues[i] <= maxValue ){
			rangeCount++;
		}
	}
	qDebug() << "oldCount="<<count<<" rangeCount="<<rangeCount;
	xValues.resize( rangeCount );
	yValues.resize( rangeCount );
	qDebug() << "Finished resize";
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalYValues[i] && originalYValues[i] <= maxValue ){
			xValues[j] = originalXValues[i];
			yValues[j] = originalYValues[i];
			qDebug() << "Added yValue="<<yValues[j]<<" xValue="<<xValues[j];
			if ( xValues[j] < *minX ){
				*minX = xValues[j];
			}
			else if (xValues[j] > *maxX ){
				*maxX = xValues[j];
			}
			j++;
		}
	}
}

void FeatherPlotWidget::initializeDomainLimitedData( double minValue, double maxValue,
		QVector<double>& xValues, QVector<double>& yValues,
		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const{
	int count = qMin( originalXValues.size(), originalYValues.size());
	int domainCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalXValues[i] && originalXValues[i] <= maxValue ){
			domainCount++;
		}
	}
	qDebug() << "oldCount="<<count<<" rangeCount="<<domainCount;
	xValues.resize( domainCount );
	yValues.resize( domainCount );
	qDebug() << "Finished resize";
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalXValues[i] && originalXValues[i] <= maxValue ){
			xValues[j] = originalXValues[i];
			yValues[j] = originalYValues[i];
			qDebug() << "Added yValue="<<yValues[j]<<" xValue="<<xValues[j];
			j++;
		}
	}
}

void FeatherPlotWidget::zoomNeutral(){
	FeatherPlot::PlotType originalPlotType = plot->getPlotType();
	resetPlot( originalPlotType );
	if ( originalPlotType == FeatherPlot::SLICE_CUT ||
			originalPlotType == FeatherPlot::ORIGINAL ){
		addZoomNeutralCurves();
	}
	else {
		addZoomNeutralScatterPlot();
	}
	zoom90Action.setChecked( false );
}

void FeatherPlotWidget::addZoomNeutralScatterPlot(){
	plot->addCurve( singleDishDataYValues, interferometerDataYValues, scatterPlotColor, "", QwtPlot::yLeft );
	plot->replot();
}

void FeatherPlotWidget::addZoomNeutralCurves(){
	plot->addCurve( singleDishWeightXValues, singleDishWeightYValues, singleDishWeightColor, singleDishWeightFunction, QwtPlot::yLeft );
	plot->addCurve( interferometerWeightXValues, interferometerWeightYValues, interferometerWeightColor, interferometerWeightFunction, QwtPlot::yLeft );
	plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishDataFunction, QwtPlot::yRight );
	plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerDataFunction, QwtPlot::yRight );
	plot->replot();
}


void FeatherPlotWidget::resetPlot( FeatherPlot::PlotType plotType ){
	QLayout* layout = ui.frame->layout();
	if ( layout == NULL ){
		layout = new QHBoxLayout( ui.frame );
	}
	else {
		layout->removeWidget( plot );
		delete plot;
	}
	plot = new FeatherPlot( this );
	layout->addWidget( plot );
	plot->initializePlot( plotTitle, plotType );
	plot->setLegendVisibility( getPracticalLegendVisibility() );
	plot->setLineThickness( lineThickness );
	ui.frame->setLayout( layout );
}

void FeatherPlotWidget::setSingleDishWeight( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	singleDishWeightXValues.resize( count );
	singleDishWeightYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		singleDishWeightXValues[i] = xValues[i];
		singleDishWeightYValues[i] = yValues[i];
	}
	plot->addCurve( singleDishWeightXValues, singleDishWeightYValues, singleDishWeightColor, singleDishWeightFunction, QwtPlot::yLeft );
	plot->replot();
}

void FeatherPlotWidget::setSingleDishData( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	singleDishDataXValues.resize( count );
	singleDishDataYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		singleDishDataXValues[i] = xValues[i];
		singleDishDataYValues[i] = yValues[i];
	}
	if ( !originalData ){
		plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishDataFunction, QwtPlot::yRight );
	}
	else {
		plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishFunction, QwtPlot::yLeft );
	}
	plot->replot();
}

void FeatherPlotWidget::setInterferometerWeight( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	interferometerWeightXValues.resize( count );
	interferometerWeightYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		interferometerWeightXValues[i] = xValues[i];
		interferometerWeightYValues[i] = yValues[i];
	}
	plot->addCurve( interferometerWeightXValues, interferometerWeightYValues, interferometerWeightColor, interferometerWeightFunction, QwtPlot::yLeft );
	plot->replot();
}

void FeatherPlotWidget::setInterferometerData( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	interferometerDataXValues.resize( count );
	interferometerDataYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		interferometerDataXValues[i] = xValues[i];
		interferometerDataYValues[i] = yValues[i];
	}
	if ( !originalData ){
		plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerDataFunction, QwtPlot::yRight );
	}
	else {
		plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerFunction, QwtPlot::yLeft );
	}
	plot->replot();
}


FeatherPlotWidget::~FeatherPlotWidget(){

}
}
