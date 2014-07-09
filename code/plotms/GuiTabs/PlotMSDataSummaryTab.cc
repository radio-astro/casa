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
#include <plotms/GuiTabs/PlotMSDataSummaryTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/GuiTabs/PlotMSDataCollapsible.qo.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>

#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <QDebug>

namespace casa {


// Constructors/Destructors//

PlotMSDataSummaryTab::PlotMSDataSummaryTab(PlotMSPlotter* parent) :
		PlotMSTab(parent){

	ui.setupUi(this);

	its_force_reload = false;
	rowLimit = 1;
	colLimit = 1;

	 // Add as watcher.
	parent->getParent()->getPlotManager().addWatcher(this);
    
    scrollWidget = new QWidget( ui.dataScroll );
    ui.dataScroll->setWidget( scrollWidget );
    ui.dataScroll->setWidgetResizable( true );
    QVBoxLayout* scrollLayout = new QVBoxLayout();
    scrollLayout->setSpacing( 2 );
    scrollLayout->setContentsMargins(2,2,2,2);
    scrollWidget->setLayout( scrollLayout );

    scrollWidget->setAutoFillBackground( true );
    QPalette pal = scrollWidget->palette();
    QColor bgColor( Qt::blue );
    pal.setColor( QPalette::Background, bgColor );
    scrollWidget->setPalette( pal );
    bottomSpacer = new QSpacerItem( 200,20, QSizePolicy::Preferred, QSizePolicy::Expanding );

    connect( ui.addSingleButton, SIGNAL(clicked()), this, SLOT(addSinglePlot()));


    //So plot parameters are up and available for the user.
    addSinglePlot();


    // Additional slot for Plot button for shift+plot forced redraw feature.
     // All this does is note if shift was down during the click.
     // This slot should be called before the main one in synchronize action.
     connect( ui.plotButton, SIGNAL(clicked()),  SLOT(observeModKeys()) );

     // To fix zoom stack first-element bug, must be sure Zoom, Pan, etc
     // in toolbar are all unclicked.   Leave it to the Plotter aka
     // "parent" know how to do this, offering a slot
     // we can plug the Plot button into.
     // The prepareForPlotting() slot is available for other things to do
     // at this point in time (like evil diagnostic experiements).
     connect( ui.plotButton, SIGNAL(clicked()),  parent, SLOT(prepareForPlotting()) );

     // Synchronize plot button.  This makes the reload/replot happen.
     itsPlotter_->synchronizeAction(PlotMSAction::PLOT, ui.plotButton);

     plotIndex = 0;
}

PlotMSDataSummaryTab::~PlotMSDataSummaryTab() {

}

void PlotMSDataSummaryTab::emptyLayout(){
	QLayout* scrollLayout = scrollWidget->layout();
	for ( int i = 0; i < dataList.size(); i++ ){
		scrollLayout->removeWidget( dataList[i]);
	}
}

void PlotMSDataSummaryTab::setGridSize( int rowCount, int colCount ){

	//Store the maximum number of plots we can support.
	rowLimit = rowCount;
	colLimit = colCount;


	//Tell everyone to update their grid size, disabling any whose location
	//exceeds the current limits.
	int dataCount = dataList.size();
	for ( int i = 0; i < dataCount; i++ ){
		dataList[i]->setGridSize( rowCount, colCount );
	}

	//Generate new plots.
	//plot( true );
}

void PlotMSDataSummaryTab::fillLayout(){
	QLayout* scrollLayout = scrollWidget->layout();
	for ( int i = 0; i < dataList.size(); i++ ){
		scrollLayout->addWidget( dataList[i]);
	}
}


void PlotMSDataSummaryTab::addSinglePlot(){
	//Minimize any open plots so the new one will be seen.

	int dataCount = dataList.size();
	for ( int i = 0; i < dataCount; i++ ){
		dataList[i]->minimizeDisplay();
	}
	insertData( -1 );
}


void PlotMSDataSummaryTab::insertData( int index ){
	/*****
	 * NOTE::  When we support overlays, we will have
	 * to change this method so it doesn't just insert
	 * at index, but it figures out what the index should
	 * be with overlays thrown in.
	 */
	PlotMSDataCollapsible* plotTab = NULL;
	if ( 0 <= index && index < dataList.size() ){
		plotTab = dataList[index];
	}
	else {
		QLayout* scrollLayout = scrollWidget->layout();
		scrollLayout->removeItem( bottomSpacer );
		plotTab = new PlotMSDataCollapsible( itsPlotter_, scrollWidget );
		connect(  plotTab, SIGNAL( close(PlotMSDataCollapsible*)),
				this, SLOT( close(PlotMSDataCollapsible*)));
		plotTab->setGridSize( rowLimit, colLimit );
		dataList.append( plotTab );
		scrollLayout->addWidget( plotTab );
		scrollLayout->addItem( bottomSpacer );
	}
}

void PlotMSDataSummaryTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag) {
	for ( int i = 0; i < dataList.size(); i++ ){
		dataList[i]->parametersHaveChanged( p, updateFlag );
	}
}

// Implements PlotMSPlotManagerWatcher::plotsChanged().
void PlotMSDataSummaryTab::plotsChanged(const PlotMSPlotManager& manager){
	for ( int i = 0; i < dataList.size(); i++ ){
		dataList[i]->plotsChanged( manager, i );
	}
}

void PlotMSDataSummaryTab::singleDataChanged(PlotMSDataCollapsible* collapsible){
	int singleDataIndex = dataList.indexOf( collapsible );
	emit changed( singleDataIndex );
}

void PlotMSDataSummaryTab::close( PlotMSDataCollapsible* collapsible ){
	QLayout* scrollLayout = scrollWidget->layout();
	scrollLayout->removeWidget( collapsible );
	int collapseIndex = dataList.indexOf( collapsible );
	if ( collapseIndex >= 0 ){
		dataList.removeAt( collapseIndex );
	}
	delete collapsible;
}

void PlotMSDataSummaryTab::plot(){
	plotIndex = 0;
	completePlots.clear();
	doPlotting();
}

void PlotMSDataSummaryTab::doPlotting(){
	for ( int i = plotIndex; i < dataList.size(); i++ ){
		bool plotCompleted = dataList[i]->plot( its_force_reload );
		if ( !plotCompleted ){
			plotIndex = i;
			break;
		}
	}
}

void PlotMSDataSummaryTab::completePlotting( bool success ){
	int dataCount = dataList.size();
	if ( plotIndex < dataCount ){
		completePlots[dataList[plotIndex]] = success;
	}
	plotIndex = plotIndex + 1;
	if ( plotIndex < dataCount){
		//We haven't finished telling all the threads to update their data.
		doPlotting();
	}
	if ( plotIndex == dataCount){
		//All the threads have finished updating their data.
		QList<PlotMSDataCollapsible*> plotKeys = completePlots.keys();
		int plotKeyCount = plotKeys.size();

		//Clear out any old data in case one canvas has two sets of data (overplot)
		//and a redraw of the first plot has old data from the second plot.
		for ( int i = 0; i < plotKeyCount; i++ ){
			plotKeys[i]->clearData();
		}
		//Trigger redraws
		for ( int i = 0; i < plotKeyCount; i++ ){
			plotKeys[i]->completePlotting( completePlots[ plotKeys[i] ]);
		}
	}
}

vector<vector<PMS::Axis> > PlotMSDataSummaryTab::selectedLoadAxes() const {
	vector<vector<PMS::Axis> > loadAxes;
	for ( int i = 0; i < dataList.size(); i++ ){
		loadAxes.push_back( dataList[i]->getSelectedLoadAxes() );
	}
	return loadAxes;
}

vector<vector<PMS::Axis> > PlotMSDataSummaryTab::selectedReleaseAxes() const {
	vector<vector<PMS::Axis> > releaseAxes;
	for ( int i = 0; i < dataList.size(); i++ ){
		releaseAxes.push_back( dataList[i]->getSelectedReleaseAxes() );
	}
	return releaseAxes;
}

vector<PlotMSPlot*> PlotMSDataSummaryTab::getCurrentPlots(){
	vector<PlotMSPlot*> currentPlots;
	for ( int i = 0; i < dataList.size(); i++ ){
		PlotMSPlot* plot = dataList[i]->getPlot();
		if ( plot != NULL ){
			currentPlots.push_back( plot );
		}
	}
	return currentPlots;
}

void PlotMSDataSummaryTab::observeModKeys()   {
	// Bitflags report if shift, etc were down during click of Plot button
	Qt::KeyboardModifiers itsModKeys = QApplication::keyboardModifiers();
	bool using_shift_key = (itsModKeys & Qt::ShiftModifier) !=0;
	bool always_replot_checked = ui.forceReplotChk->isChecked();
	its_force_reload = using_shift_key  ||  always_replot_checked;
}

void PlotMSDataSummaryTab::resizeEvent( QResizeEvent* /*event*/ ){
	QSize currentSize = size();
	int usedHeight = 0;
	for ( int i = 0; i < dataList.size(); i++ ){
		QSize widgetSize = dataList[i]->sizeHint();
		usedHeight = usedHeight + widgetSize.height();
	}

	int openIndex = -1;
	for ( int i = 0; i < dataList.size(); i++ ){
		if ( !dataList[i]->isMinimized() ){
			openIndex = i;
		}
	}
	//Pass the height increase/descrease to the open one.
	if ( openIndex >= 0 ){
		int heightDiff = currentSize.height() - usedHeight;
		dataList[openIndex]->resetHeight( heightDiff );
	}
}

}
