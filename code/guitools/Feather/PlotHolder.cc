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
#include <guitools/Feather/FeatherPlotWidget.qo.h>
#include <guitools/Feather/ExternalAxisWidgetLeft.h>

namespace casa {

PlotHolder::PlotHolder(QWidget *parent)
    : QWidget(parent),
      plotTypeAction( "Scatter Plot", this ), zoom90Action( "Auto Zoom", this),
      zoomNeutralAction( "Zoom Neutral", this ),contextMenu( this ), legendHolder( NULL ),
      legendVisible( false ),
      displayOutputSlice(false),displayOriginalSlice( false),
      displayScatter( false ), tempScatterPlot(false), displayYGraphs( false ){
	ui.setupUi(this);

	initializePlots();
	initializeActions();
}

void PlotHolder::initializeActions(){

	plotTypeAction.setStatusTip( "Toggle between a single-dish/interferometer amplitude scatter plot and function slice cuts.");
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
	FeatherPlotWidget* origXWidget = new FeatherPlotWidget( "Original Data Slice U", FeatherPlot::ORIGINAL);
	connect( origXWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedX(double)));
	connect( origXWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( origXWidget );
	FeatherPlotWidget* origYWidget = new FeatherPlotWidget("Original Data Slice V", FeatherPlot::ORIGINAL);
	connect( origYWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedY(double)));
	connect( origYWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( origYWidget );
	FeatherPlotWidget* xWidget = new FeatherPlotWidget( "Average Visibility Slice U", FeatherPlot::SLICE_CUT);
	connect( xWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedX(double)));
	connect( xWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( xWidget );
	FeatherPlotWidget* yWidget = new FeatherPlotWidget( "Average Visibility Slice V", FeatherPlot::SLICE_CUT);
	connect( yWidget, SIGNAL(dishDiameterChanged(double)), this, SIGNAL(dishDiameterChangedY(double)));
	connect( yWidget, SIGNAL(rectangleZoomed(double,double,double,double)), this, SLOT(rectangleZoomed(double,double,double,double)));
	plots.append( yWidget );
	FeatherPlotWidget* xWidgetScatter = new FeatherPlotWidget( "Average Visibility Scatter U", FeatherPlot::SCATTER_PLOT);
	xWidgetScatter->setPermanentScatter( true );
	plots.append( xWidgetScatter );
	FeatherPlotWidget* yWidgetScatter = new FeatherPlotWidget( "Average Visibility Scatter V", FeatherPlot::SCATTER_PLOT);
	yWidgetScatter->setPermanentScatter( true );
	plots.append( yWidgetScatter );
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
	for ( int i = 0; i < SCATTER_X; i++ ){
		plots[i]->zoomRectangle( minX, maxX, minY, maxY );
	}
}

void PlotHolder::changePlotType(){
	bool scatterPlot = plotTypeAction.isChecked();
	zoom90Action.setEnabled( !scatterPlot );
	FeatherPlot::PlotType plotType = FeatherPlot::NO_TYPE;
	adjustLayout( scatterPlot );
	if ( scatterPlot ){
		zoom90Action.setChecked( false );
		plotType = FeatherPlot::SCATTER_PLOT;
	}
	for ( int i = 0; i < plots.size(); i++ ){
		FeatherPlot::PlotType individualType = plotType;
		if ( plotType == FeatherPlot::NO_TYPE ){
			int divisor = FeatherPlot::NO_TYPE - 1;
			individualType = static_cast<FeatherPlot::PlotType>( i / divisor );
		}
		plots[i]->changePlotType( individualType );
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
	plots[SLICE_X_ORIGINAL]->setLegendVisibility( visible );
	plots[SLICE_Y_ORIGINAL]->setLegendVisibility( visible );
	plots[SLICE_X]->setLegendVisibility( visible );
	plots[SLICE_Y]->setLegendVisibility( visible );
}

void PlotHolder::setDisplayScatterPlot( bool visible ){
	displayScatter = visible;
}

void PlotHolder::setDisplayOutputSlice( bool visible ){
	displayOutputSlice = visible;
}

void PlotHolder::setDisplayOriginalSlice( bool visible ){
	displayOriginalSlice = visible;
}

void PlotHolder::setDisplayYGraphs( bool visible ){
	displayYGraphs = visible;
}

void PlotHolder::setColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
		const QColor& scatterPlotColor, const QColor& dishDiameterLineColor,
		const QColor& zoomRectColor ){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->setPlotColors( colorMap, scatterPlotColor, dishDiameterLineColor, zoomRectColor );
	}
}


//------------------------------------------------------------------------
//                          Data
//------------------------------------------------------------------------

void PlotHolder::setSingleDishWeight( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
		const Vector<Float>& sDy, const Vector<Float>& sDyAmp ){
	plots[SLICE_X]->setSingleDishWeight( sDx, sDxAmp );
	plots[SLICE_Y]->setSingleDishWeight( sDy, sDyAmp );
}

void PlotHolder::setInterferometerWeight( const Vector<Float>& intx, const Vector<Float>& intxAmp,
		const Vector<Float>& inty, const Vector<Float>& intyAmp ){
	plots[SLICE_X]->setInterferometerWeight( intx, intxAmp );
	plots[SLICE_Y]->setInterferometerWeight( inty, intyAmp );
}

void PlotHolder::setSingleDishData( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
		const Vector<Float>& sDy, const Vector<Float>& sDyAmp ){
	plots[SLICE_X]->setSingleDishData( sDx, sDxAmp );
	plots[SLICE_Y]->setSingleDishData( sDy, sDyAmp );
	plots[SCATTER_X]->setSingleDishData( sDx, sDxAmp );
	plots[SCATTER_Y]->setSingleDishData( sDy, sDyAmp );
}

void PlotHolder::setInterferometerData( const Vector<Float>& intx, const Vector<Float>& intxAmp,
		const Vector<Float>& inty, const Vector<Float>& intyAmp ){
	plots[SLICE_X]->setInterferometerData( intx, intxAmp );
	plots[SLICE_Y]->setInterferometerData( inty, intyAmp );
	plots[SCATTER_X]->setInterferometerData( intx, intxAmp );
	plots[SCATTER_Y]->setInterferometerData( inty, intyAmp );
}

void PlotHolder::setSingleDishDataOriginal( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
		const Vector<Float>& sDy, const Vector<Float>& sDyAmp ){
	plots[SLICE_X_ORIGINAL]->setSingleDishData( sDx, sDxAmp );
	plots[SLICE_Y_ORIGINAL]->setSingleDishData( sDy, sDyAmp );
}

void PlotHolder::setInterferometerDataOriginal( const Vector<Float>& intx, const Vector<Float>& intxAmp,
		const Vector<Float>& inty, const Vector<Float>& intyAmp ){
	plots[SLICE_X_ORIGINAL]->setInterferometerData( intx, intxAmp );
	plots[SLICE_Y_ORIGINAL]->setInterferometerData( inty, intyAmp );
}

void PlotHolder::updateScatterData( ){
	plots[SCATTER_X]->addScatterData();
	plots[SCATTER_Y]->addScatterData();
}

void PlotHolder::dishDiameterXChanged( double value ){
	plots[SLICE_X]->setDishDiameter( value );
	plots[SLICE_X_ORIGINAL]->setDishDiameter( value );
}

void PlotHolder::dishDiameterYChanged( double value ){
	plots[SLICE_Y]->setDishDiameter( value );
	plots[SLICE_Y_ORIGINAL]->setDishDiameter( value );
}

//----------------------------------------------------------------------
//                        Layout
//----------------------------------------------------------------------

void PlotHolder::addPlots( QGridLayout*& layout, bool displayYGraphs, int rowIndex, int basePlotIndex ){
	layout->addWidget( plots[basePlotIndex], rowIndex, 1 );
	if ( displayYGraphs ){
		layout->addWidget( plots[basePlotIndex+1], rowIndex, 2);
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

	int rowCount = 0;
	int columnCount = 0;
	if ( displayOriginalSlice || displayOutputSlice || displayScatter ){
		if ( displayYGraphs ){
			//Two graphs plus the left axis.
			columnCount = 3;
		}
		else {
			//There is one graph plus the left axis.
			columnCount = 2;
		}
	}

	if ( displayOriginalSlice ){
		addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SLICE_X_ORIGINAL );
		addPlots( gridLayout, displayYGraphs, rowCount, SLICE_X_ORIGINAL );
		if ( tempScatterPlot ){
			addPlotAxis( rowCount, columnCount, gridLayout, QwtPlot::yRight, SLICE_Y_ORIGINAL );
		}
		rowCount++;
	}

	int basePlotIndex = 0;
	if ( displayOutputSlice ){
		addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SLICE_X );
		addPlots( gridLayout, displayYGraphs, rowCount, SLICE_X );
		//if ( !tempScatterPlot ){
			addPlotAxis( rowCount, columnCount, gridLayout, QwtPlot::yRight, SLICE_Y );
		//}

		//Use the original slice axis for the bottom axis if it is present.
		//That way when the original data is loaded (with the image files)
		//the bottom axis will be correct.
		if ( !displayOriginalSlice ){
			basePlotIndex = 1;
		}
		rowCount++;
	}

	//Add the common shared bottom axis. Note the y-axis is in column 0 so we
	//start the loop at 1.
	if ( rowCount > 0 ){
		for ( int i = 1; i < columnCount; i++ ){
			int plotIndex = 2 * basePlotIndex +(i-1);
			addPlotAxis( rowCount, i, gridLayout, QwtPlot::xBottom, plotIndex );
		}
		rowCount++;
	}

	//The scatter plot cannot share the bottom axis of the other plots so
	//we have to add the scatter plot after putting in the common y-axis
	//the other plots share.
	if ( displayScatter && !tempScatterPlot ){
		addPlotAxis( rowCount, 0, gridLayout, QwtPlot::yLeft, SCATTER_X );
		addPlots( gridLayout, displayYGraphs, rowCount, SCATTER_X );
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

	//Add the legend/remove legend.  We don't need a legend if all we
	//are looking at is scatter plots.
	if ( legendVisible && !tempScatterPlot && (displayOutputSlice || displayOriginalSlice) ){
		if ( legendHolder == NULL ){
			legendHolder = new QWidget( this);
			legendHolder->setMinimumSize( 800, 50 );
			legendHolder->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
		}
		gridLayout->addWidget(legendHolder,rowCount, 0, 1, columnCount, Qt::AlignHCenter);
		if ( displayOutputSlice ){
			plots[SLICE_X]->insertLegend( legendHolder );
		}
		else if ( displayOriginalSlice ){
			plots[SLICE_X_ORIGINAL]->insertLegend( legendHolder );
		}
	}

	setLayout( gridLayout );
}

void PlotHolder::clearPlots(){
	for ( int i = 0; i < plots.size(); i++ ){
		plots[i]->clearPlot();
	}
}



}
