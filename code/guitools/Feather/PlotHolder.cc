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
#include "PlotHolder.qo.h"
#include <QGridLayout>
#include <QDebug>
#include <QContextMenuEvent>
#include <guitools/Feather/FeatherPlotWidgetSlice.h>
#include <guitools/Feather/FeatherPlotWidgetScatter.h>
#include <guitools/Feather/ExternalAxisWidgetLeft.h>


namespace casa {

PlotHolder::PlotHolder(QWidget *parent)
    : QWidget(parent),
      plotTypeAction( "Scatter Plot", this ), zoom90Action( "Auto Zoom", this),
      zoomNeutralAction( "Zoom Neutral", this ),contextMenu( this ), legendHolder( NULL ),
      legendVisible( false ),
      displayOutputSlice(false),
      displayScatter( false ), tempScatterPlot(false),
      displayYGraphs( false ), displayXGraphs( true ),
      xAxisUV( true ){
	ui.setupUi(this);

	initializePlots();
	initializeActions();
}

void PlotHolder::initializeActions(){

	plotTypeAction.setStatusTip( "Toggle between a slice plot and a scatter plot.");
	plotTypeAction.setCheckable( true );
	connect( &plotTypeAction, SIGNAL(triggered()), this, SLOT(changePlotType()));

	zoom90Action.setCheckable( true );
	connect( &zoom90Action, SIGNAL(triggered()), this, SLOT(changeZoom90()));

	zoomNeutralAction.setStatusTip( "Restore the graph back to its original non-zoomed state");
	connect( &zoomNeutralAction, SIGNAL(triggered()), this, SLOT(zoomNeutral()));

	contextMenu.addAction( &plotTypeAction );
	contextMenu.addAction( &zoom90Action );
	contextMenu.addAction( &zoomNeutralAction );

	setContextMenuPolicy( Qt::CustomContextMenu);
	connect( this, SIGNAL(customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
}

void PlotHolder::initializePlots(){
	//Add the plots to the array
	FeatherPlotWidget* xWidget = new FeatherPlotWidgetSlice( "Slice U", FeatherPlot::SLICE_CUT);
	connect( xWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedX(double)));
	connect( xWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( xWidget );
	FeatherPlotWidget* yWidget = new FeatherPlotWidgetSlice( "Slice V", FeatherPlot::SLICE_CUT);
	connect( yWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedY(double)));
	connect( yWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( yWidget );
	FeatherPlotWidget* distanceWidget = new FeatherPlotWidgetSlice( "Slice Radial Distance", FeatherPlot::SLICE_CUT);
	connect( distanceWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedY(double)));
	connect( distanceWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( distanceWidget );
	FeatherPlotWidget* xWidgetScatter = new FeatherPlotWidgetScatter( "Scatter U", FeatherPlot::SCATTER_PLOT);
	plots.append( xWidgetScatter );
	FeatherPlotWidget* yWidgetScatter = new FeatherPlotWidgetScatter( "Scatter V", FeatherPlot::SCATTER_PLOT);
	plots.append( yWidgetScatter );
	FeatherPlotWidget* distanceWidgetScatter = new FeatherPlotWidgetScatter( "Scatter Radial Distance", FeatherPlot::SCATTER_PLOT);
	plots.append( distanceWidgetScatter );
}


PlotHolder::~PlotHolder(){
	for ( int i = 0; i < plots.size(); i++ ){
		delete plots[i];
	}
	delete legendHolder;
	legendHolder = NULL;
}
//------------------------------------------------------------------------
//                   Plot events
//------------------------------------------------------------------------

void PlotHolder::rectangleZoomed( double minX, double maxX, double minY, double maxY ){
	plots[SLICE_X]->zoomRectangle( minX, maxX, minY, maxY );
	plots[SLICE_Y]->zoomRectangle( minX, maxX, minY, maxY );
	plots[SLICE_DISTANCE]->zoomRectangle( minX, maxX, minY, maxY );
}


void PlotHolder::changePlotType(){
	bool scatterPlot = plotTypeAction.isChecked();
	zoom90Action.setEnabled( !scatterPlot );
	adjustLayout( scatterPlot );
	if ( scatterPlot ){
		zoom90Action.setChecked( false );
	}
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->refresh();
	}
}


void PlotHolder::changeZoom90(){
	bool zoom90 = zoom90Action.isChecked();
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->changeZoom90( zoom90 );
	}
}


void PlotHolder::zoomNeutral(){
	zoom90Action.setChecked( false );
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->zoomNeutral();
	}
}


void PlotHolder::showContextMenu( const QPoint& pt ){
	QPoint globalPos = mapToGlobal( pt );
	contextMenu.exec( globalPos );
}


void PlotHolder::setRectangleZoomMode(){
	for ( int i = 0; i < SCATTER_X; i++ ){
		plots[i]->setRectangleZoomMode();
	}
}


void PlotHolder::setDiameterSelectorMode(){
	for ( int i = 0; i < SCATTER_X; i++ ){
		plots[i]->setDiameterSelectorMode();
	}
}

//---------------------------------------------------------------------
//                Preferences
//---------------------------------------------------------------------

void PlotHolder::setLineThickness( int lineThickness ){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setLineThickness( lineThickness );
	}
}


void PlotHolder::setDotSize( int dotSize ){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setDotSize( dotSize );
	}
}


void PlotHolder::setLegendVisibility( bool visible ){
	legendVisible = visible;
	for ( int i = 0; i < SCATTER_X; i++ ){
		plots[i]->setLegendVisibility( visible );
	}
}


void PlotHolder::setDisplayScatterPlot( bool visible ){
	displayScatter = visible;
}

void PlotHolder::clearData(){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->clearData();
	}
}
void PlotHolder::refreshPlots(){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->refresh();
	}
}


void PlotHolder::setDisplayOutputSlice( bool visible ){
	displayOutputSlice = visible;
}

void PlotHolder::setDisplayYGraphs( bool visible ){
	displayYGraphs = visible;
}

void PlotHolder::setDisplayXGraphs( bool visible ){
	displayXGraphs = visible;
}

void PlotHolder::setXAxisUV( bool xAxisUV ){
	this->xAxisUV = xAxisUV;
}


void PlotHolder::setColors( const QMap<PreferencesColor::CurveType,CurveDisplay>& colorMap ){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setPlotColors( colorMap);
	}
}

void PlotHolder::setScatterCurves( FeatherCurveType::CurveType xScatter,
		const QList<FeatherCurveType::CurveType>& yScatters ){
	plots[SCATTER_X]->setScatterCurves( xScatter, yScatters );
	plots[SCATTER_Y]->setScatterCurves( xScatter, yScatters );
	plots[SCATTER_DISTANCE]->setScatterCurves( xScatter, yScatters );
}


void PlotHolder::setLogScale( bool uvScale, bool logScale ){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setLogScale( uvScale, logScale );
	}
}
//------------------------------------------------------------------------
//                          Data
//------------------------------------------------------------------------

void PlotHolder::setData( const Vector<Float>& x, const Vector<Float>& xAmp,
   		const Vector<Float>& y, const Vector<Float>& yAmp,
   		DataType dType ){
	plots[SLICE_X]->setData( x, xAmp, dType );
	plots[SLICE_Y]->setData( y, yAmp, dType );
	plots[SLICE_DISTANCE]->setData( x, xAmp, dType );
	plots[SCATTER_X]->setData( x, xAmp, dType );
	plots[SCATTER_Y]->setData( y, yAmp, dType );
	plots[SCATTER_DISTANCE]->setData( x, xAmp, dType );
}


void PlotHolder::addSumData(){
	plots[SLICE_X]->addSumData();
	plots[SLICE_Y]->addSumData();
	plots[SLICE_DISTANCE]->addSumData();
}

void PlotHolder::updateScatterData( ){
	plots[SCATTER_X]->addZoomNeutralCurves();
	plots[SCATTER_Y]->addZoomNeutralCurves();
	plots[SCATTER_DISTANCE]->addZoomNeutralCurves();
}


void PlotHolder::dishDiameterXChanged( double value ){

	plots[SLICE_X]->setDishDiameter( value );
	plots[SLICE_DISTANCE]->setDishDiameter( value );
	plots[SCATTER_X]->setDishDiameter( value );
	plots[SCATTER_DISTANCE]->setDishDiameter( value );
}


void PlotHolder::dishDiameterYChanged( double value ){
	plots[SLICE_Y]->setDishDiameter( value );
	plots[SCATTER_Y]->setDishDiameter( value );
}

//----------------------------------------------------------------------
//                        Layout
//----------------------------------------------------------------------

void PlotHolder::addPlots( QGridLayout*& layout, int rowIndex, int basePlotIndex ){
	int baseColumn = 1;
	if ( displayXGraphs ){
		layout->addWidget( plots[basePlotIndex], rowIndex, baseColumn );
		baseColumn++;
	}
	if ( displayYGraphs ){
		layout->addWidget( plots[basePlotIndex+1], rowIndex, baseColumn );
	}
}


void PlotHolder::addPlotAxis( int rowIndex, int columnIndex, QGridLayout* layout, QwtPlot::Axis axis, int basePlotIndex ){
	QWidget* axisWidget = plots[basePlotIndex]->getExternalAxisWidget( axis );
	axisWidget->setParent( this );
	layout->addWidget( axisWidget, rowIndex, columnIndex );
}


void PlotHolder::emptyLayout(QLayout* layout ){
	//Take the legend external legend out of the plot.
	//Needed because otherwise the external legend is displayed
	//as a separate dialog when all the plots are removed.
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->clearLegend();
	}

	//Empty everything out of the layout and set parents
	//of widgets to NULL.
	QLayoutItem* layoutItem = layout->itemAt( 0 );
	while( layoutItem != NULL ){
		layout->removeItem( layoutItem );
		QWidget* childWidget = layoutItem->widget();
		if ( childWidget != NULL ){
			childWidget->setParent( NULL );
		}
		layoutItem = layout->itemAt( 0 );
	}
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setParent( NULL );
	}

	//Get rid of the widget holding the legend.
	delete legendHolder;
	legendHolder = NULL;
}


void PlotHolder::adjustLayout( bool scatterPlot ){
	if ( scatterPlot != tempScatterPlot ){
		tempScatterPlot = scatterPlot;
		layoutPlotWidgets();
	}
}


void PlotHolder::layoutPlotWidgets(){
	QLayout* plotLayout = layout();
	if ( plotLayout == NULL ){
		plotLayout = new QGridLayout();
	}
	else {
		emptyLayout( plotLayout );
	}
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(plotLayout);
	gridLayout->setSpacing( 5 );
	gridLayout->setContentsMargins( 2, 2, 2, 2 );

	pair<int,int> gridSize(0,0);
	if ( xAxisUV ){
		gridSize = addUVPlots( gridLayout );
	}
	else {
		gridSize = addRadialPlots( gridLayout );
	}
	int rowCount = gridSize.first;
	int columnCount = gridSize.second;
	if ( rowCount > 0 && columnCount > 0 ){
		//Add the legend/remove legend.
		if ( legendVisible ){
			if ( legendHolder == NULL ){
				legendHolder = new QWidget( this);
				legendHolder->setMinimumSize( 800, 50 );
				legendHolder->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
			}
			gridLayout->addWidget(legendHolder,rowCount, 0, 1, columnCount, Qt::AlignHCenter);
			if ( xAxisUV ){
				plots[SLICE_X]->insertLegend( legendHolder );
			}
			else {
				plots[SLICE_DISTANCE]->insertLegend( legendHolder );
			}
		}
	}
	setLayout( gridLayout );
}


pair<int,int> PlotHolder::addRadialPlots(QGridLayout*& gridLayout){

	//First get the column count
	int columnCount = 0;
	if ( displayOutputSlice || displayScatter ){
		//One graph plus the left axis.
		columnCount = 2;
	}

	//Dynamically determine the row count as we add plots and axes.
	int rowCount = 0;
	if ( columnCount > 0 ){
		if ( displayOutputSlice && !tempScatterPlot){
			Plots type = SLICE_DISTANCE;
			addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, type );
			gridLayout->addWidget( plots[type], 0, 1 );
			addPlotAxis( rowCount, columnCount, gridLayout, QwtPlot::yRight, type );
			rowCount++;
		}

		//Add the bottom axis. Note the y-axis is in column 0 so we start at 1.
		if ( rowCount > 0 ){
			addPlotAxis( rowCount, 1, gridLayout, QwtPlot::xBottom, SLICE_DISTANCE );
			rowCount++;
		}

		//The scatter plot cannot share the bottom axis of the other plots so
		//we have to add the scatter plot after putting in the common y-axis
		//the other plots share.
		if ( displayScatter || tempScatterPlot ){
			addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SCATTER_DISTANCE );
			gridLayout->addWidget( plots[SCATTER_DISTANCE], rowCount, 1 );
			rowCount++;

			//Add the bottom axis for the scatter plot
			addPlotAxis( rowCount, 1, gridLayout, QwtPlot::xBottom, SCATTER_DISTANCE );
			rowCount++;
		}
	}
	pair<int,int> distanceGridSize( rowCount, columnCount );
	return distanceGridSize;
}




pair<int,int> PlotHolder::addUVPlots(QGridLayout*& gridLayout){

	//First get the column count
	int columnCount = 0;
	if ( displayOutputSlice || displayScatter ){
		if ( displayYGraphs && displayXGraphs ){
			//Two graphs plus the left axis.
			columnCount = 3;
		}
		else if ( displayYGraphs || displayXGraphs ){
			//There is one graph plus the left axis.
			columnCount = 2;
		}
	}

	//Dynamically determine the row count as we add plots and axes.
	int rowCount = 0;
	if ( columnCount > 0 ){
		if ( displayOutputSlice  && !tempScatterPlot){
			addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SLICE_X );
			addPlots( gridLayout, rowCount, SLICE_X );
			addPlotAxis( rowCount, columnCount, gridLayout, QwtPlot::yRight, SLICE_Y );
			rowCount++;
		}

		//Add the bottom axis. Note the y-axis is in column 0 so we
		//start the loop at 1.
		if ( rowCount > 0 ){
			for ( int i = 1; i < columnCount; i++ ){
				int plotIndex = i - 1;
				addPlotAxis( rowCount, i, gridLayout, QwtPlot::xBottom, plotIndex );
			}
			rowCount++;
		}

		//The scatter plot cannot share the bottom axis of the other plots so
		//we have to add the scatter plot after putting in the common y-axis
		//the other plots share.
		if ( displayScatter || tempScatterPlot ){
			addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SCATTER_X );
			addPlots( gridLayout, rowCount, SCATTER_X );
			if ( displayYGraphs ){
				addPlotAxis( rowCount, columnCount, gridLayout, QwtPlot::yRight, SCATTER_Y);
			}
			rowCount++;

			//Add the bottom axis for the scatter plots
			for ( int i = 1; i < columnCount; i++ ){
				int plotIndex = SCATTER_X +(i-1);
				addPlotAxis( rowCount, i, gridLayout, QwtPlot::xBottom, plotIndex );
			}
			rowCount++;
		}
	}
	pair<int,int> uvGridSize( rowCount, columnCount );
	return uvGridSize;
}

void PlotHolder::clearPlots(){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->clearPlot();
	}
}



}
