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
#include <QLabel>
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
	nameLabel = new QLabel( "");
	plotCheck = new QCheckBox();
	plotCheck->setChecked( true );

	setAutoFillBackground( true );
	QPalette pal = palette();
	QColor bgColor( "#F0F0F0" );
	pal.setColor( QPalette::Background, bgColor );
	setPalette( pal );
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	ui.widgetLayout->setContentsMargins(2,2,2,2);
	plotTab = new PlotMSPlotTab( plotter );

	maximizeDisplay();
	prevPlotShown = false;

	//Context Menu
	setContextMenuPolicy( Qt::CustomContextMenu );
	connect( this, SIGNAL( customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu( const QPoint&)) );
	connect( plotCheck, SIGNAL(stateChanged(int)), this, SIGNAL( plottableChanged()));
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
	/*if ( minimumSize != SIZE_COLLAPSED ){
		if ( plotTab != NULL ){
			QSize pSize = plotTab->sizeHint();
			hint.setHeight( pSize.height());
		}
	}*/
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
	ui.widgetLayout->addWidget( plotCheck );
	ui.widgetLayout->addWidget( nameLabel );

	minimumSize = SIZE_COLLAPSED;
}

void PlotMSDataCollapsible::maximizeDisplay() {
	ui.widgetLayout->removeWidget( plotCheck );
	ui.widgetLayout->removeWidget( nameLabel );
	nameLabel->setParent( NULL );
	plotCheck->setParent( NULL );
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
		bool plottable = plotTab->isPlottable();
		plotCheck->setChecked( plottable );
	}
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

bool PlotMSDataCollapsible::isVisibilityChange() const {
	bool visibilityChange = false;
	if ( prevPlotShown != isPlottable()){
		visibilityChange = true;
	}
	return visibilityChange;
}


void PlotMSDataCollapsible::plot( bool forceReload){
	if ( plotTab != NULL ){
		prevPlotShown = isPlottable();
		if ( prevPlotShown ){
			plotTab->plot( forceReload );
		}
	}
}



bool PlotMSDataCollapsible::isPlottable() const {
	return plotCheck->isChecked();
}

void PlotMSDataCollapsible::setPlottable( bool plottable ){
	plotCheck->setChecked( plottable );
}

void PlotMSDataCollapsible::resetPlottable( bool enable) {
	plotCheck->setEnabled( enable );
	if ( !enable ){
		plotCheck->setChecked( false );
	}
}





void PlotMSDataCollapsible::setGridSize( int rowCount, int colCount ){
	if ( plotTab != NULL ){
		bool limitsValid = plotTab->setGridSize( rowCount, colCount );
		if ( !limitsValid ){
			resetPlottable( false );
		}
		else {
			plotCheck->setEnabled( true );
		}
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
