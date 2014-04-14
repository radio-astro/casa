//# Copyright (C) 2009
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
//# $Id: $
#include "PlotMSDataCollapsible.qo.h"
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <QLineEdit>
#include <QCheckBox>
#include <QDebug>

namespace casa {

PlotMSDataCollapsible::PlotMSDataCollapsible(PlotMSPlotter* plotter,
		QWidget *parent )
    : QWidget(parent),
      minimizeAction( "Minimize", this ),
      maximizeAction( "Maximize", this ),
      closeAction( "Close", this ),
      SIZE_COLLAPSED( 50 ), SIZE_EXPANDED( 525 ){
	ui.setupUi(this);
	nameLabel = new QLineEdit( this );
	nameLabel->setEnabled( false );
	nameLabel->setAlignment( Qt::AlignRight );


	setAutoFillBackground( true );
	QPalette pal = palette();
	QColor bgColor( "#F0F0F0" );
	pal.setColor( QPalette::Background, bgColor );
	setPalette( pal );
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	ui.widgetLayout->setContentsMargins(2,2,2,2);
	plotTab = new PlotMSPlotTab( plotter );
	connect( plotTab, SIGNAL(plottableChanged()), this, SIGNAL(plottableChanged()));

	maximizeDisplay();

	//Context Menu
	setContextMenuPolicy( Qt::CustomContextMenu );
	connect( this, SIGNAL( customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu( const QPoint&)) );

}


PlotMSDataTab* PlotMSDataCollapsible::getSingleData(){
	PlotMSDataTab* dataTab = NULL;
	if ( plotTab != NULL ){
		dataTab = plotTab->getData();
	}
	return dataTab;
}


QSize PlotMSDataCollapsible::minimumSizeHint() const {
	QSize hint ( 200, minimumSize );
	return hint;
}


QSize PlotMSDataCollapsible::sizeHint() const {
	QSize hint ( 200, minimumSize );
	return hint;
}

bool PlotMSDataCollapsible::isMinimized() const {
	bool minimized = false;
	if ( minimumSize == SIZE_COLLAPSED ){
		minimized = true;
	}
	return minimized;
}

void PlotMSDataCollapsible::openCloseDisplay() {
	bool minimized = isMinimized();
	if ( minimized ) {
		maximizeDisplay();
	} else {
		minimizeDisplay();
	}

	updateGeometry();
}

void PlotMSDataCollapsible::minimizeDisplay() {
	String identifier;
	if ( plotTab != NULL ){
		ui.widgetLayout->removeWidget( plotTab );
		plotTab->setParent( NULL );
		identifier = plotTab->getFileName();
	}

	nameLabel->setText( identifier.c_str() );

	ui.widgetLayout->addWidget( nameLabel );

	minimumSize = SIZE_COLLAPSED;
}

void PlotMSDataCollapsible::maximizeDisplay() {

	ui.widgetLayout->removeWidget( nameLabel );
	nameLabel->setParent( NULL );

	if ( plotTab != NULL ){
		ui.widgetLayout->addWidget( plotTab );
	}


	minimumSize = SIZE_EXPANDED;
}

void PlotMSDataCollapsible::parametersHaveChanged(const PlotMSWatchedParameters& params,
                  int updateFlag){
	if ( plotTab != NULL ){
		plotTab->parametersHaveChanged( params, updateFlag );
	}
}

void PlotMSDataCollapsible::plotsChanged(const PlotMSPlotManager& manager,
		int index){
	if ( plotTab != NULL ){
		plotTab->plotsChanged( manager, index );
	}
}

bool PlotMSDataCollapsible::isPlottable() const {
	bool plottable = false;
	if ( plotTab != NULL ){
		plottable = plotTab->isPlottable();
	}
	return plottable;
}



vector<PMS::Axis> PlotMSDataCollapsible::getSelectedLoadAxes() const {
	vector<PMS::Axis> axes;
	if ( plotTab != NULL ){
		axes = plotTab->selectedLoadAxes();
	}
	return axes;
}

vector<PMS::Axis> PlotMSDataCollapsible::getSelectedReleaseAxes() const {
	vector<PMS::Axis> axes;
	if ( plotTab != NULL ){
		axes = plotTab->selectedReleaseAxes();
	}
	return axes;
}

PlotMSPlot* PlotMSDataCollapsible::getPlot(){
	PlotMSPlot* plot = NULL;
	if ( plotTab != NULL ){
		plot = plotTab->currentPlot();
	}
	return plot;
}


void PlotMSDataCollapsible::plot( bool forceReload){
	if ( plotTab != NULL ){
		plotTab->plot( forceReload );
	}
}


void PlotMSDataCollapsible::setGridSize( int rowCount, int colCount ){
	if ( plotTab != NULL ){
		plotTab->setGridSize( rowCount, colCount );

	}
}

void PlotMSDataCollapsible::getLocation( Int& rowIndex, Int& colIndex ){
	if ( plotTab != NULL ){
		plotTab->getLocation( rowIndex, colIndex );
	}
}

void PlotMSDataCollapsible::showContextMenu( const QPoint& location ) {

	//Set-up the context
	QPoint showLocation = mapToGlobal( location );
	QMenu contextMenu;
	bool displayMinimized = isMinimized();
	if ( displayMinimized ){
		contextMenu.addAction( &maximizeAction );
	}
	else {
		contextMenu.addAction( &minimizeAction );
	}
	contextMenu.addAction( &closeAction );


	//Act on the user's selection.
	QAction* selectedAction = contextMenu.exec( showLocation );
	if ( selectedAction == &closeAction ){
		plotTab->removePlot();
		emit close( this );
	}
	else if ( selectedAction == &minimizeAction ){
		openCloseDisplay();
	}
	else if ( selectedAction == &maximizeAction ){
		openCloseDisplay();
	}
	else {
		qDebug() << "Unrecognized action Collapsible::contextMenu";
	}

}



PlotMSDataCollapsible::~PlotMSDataCollapsible(){
	if ( nameLabel->parent() == NULL ){
		delete nameLabel;
	}
}

}
