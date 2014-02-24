//# PlotMSPlotTab.cc: Subclass of PlotMSTab for controlling plot parameters.
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
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/GuiTabs/PlotMSAxesTab.qo.h>
#include <plotms/GuiTabs/PlotMSCacheTab.qo.h>
#include <plotms/GuiTabs/PlotMSCanvasTab.qo.h>
#include <plotms/GuiTabs/PlotMSDataTab.qo.h>
#include <plotms/GuiTabs/PlotMSDataSummaryTab.qo.h>
#include <plotms/GuiTabs/PlotMSDisplayTab.qo.h>
#include <plotms/GuiTabs/PlotMSIterateTab.qo.h>
#include <plotms/GuiTabs/PlotMSExportTab.qo.h>
#include <plotms/GuiTabs/PlotMSTransformationsTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <QDebug>
namespace casa {

//////////////////////////////////
// PLOTMSPLOTSUBTAB DEFINITIONS //
//////////////////////////////////

PlotMSPlotSubtab::PlotMSPlotSubtab(PlotMSPlotTab* plotTab,
        PlotMSPlotter* parent) : PlotMSTab(parent), itsPlotTab_(plotTab) { }

PlotMSPlotSubtab::~PlotMSPlotSubtab() { }

PlotMSPlotParameters PlotMSPlotSubtab::currentlySetParameters() const {
    return itsPlotTab_->currentlySetParameters();
}


///////////////////////////////
// PLOTMSPLOTTAB DEFINITIONS //
///////////////////////////////

PlotMSPlotTab::PlotMSPlotTab(PlotMSPlotter* parent) :  PlotMSTab(parent),
        itsPlotManager_(parent->getParent()->getPlotManager()),
        itsCurrentPlot_(NULL), itsCurrentParameters_(NULL),
        itsUpdateFlag_(true),forceReloadCounter_(0){
    setupUi(this);
    closing = false;

    // Setup tab widget.
    tabWidget->removeTab(0);
    
    // Create the plot for this tab.
    plotIndex = itsPlotManager_.numPlots();

    itsPlotManager_.addOverPlot();
    plotsChanged(itsPlotManager_, plotIndex);
}

void PlotMSPlotTab::removePlot(){
	closing = true;
	itsPlotManager_.removePlot( itsCurrentPlot_ );
}

bool PlotMSPlotTab::setGridSize( int rowCount, int colCount ){
	bool limitsValid = false;
	PlotMSIterateTab* iterateTab = findIterateTab();
	if ( iterateTab != NULL ){
		limitsValid = iterateTab->setGridSize( rowCount, colCount );
	}
	return limitsValid;
}

void PlotMSPlotTab::getLocation( Int& rowIndex, Int& colIndex ){
	if ( itsCurrentParameters_ != NULL ){
		PMS_PP_Iteration* iterParams = itsCurrentParameters_->typedGroup<PMS_PP_Iteration>();
		if ( iterParams != NULL ){
			rowIndex = iterParams->getGridRow();
			colIndex = iterParams->getGridCol();
		}
	}
}

bool PlotMSPlotTab::isPlottable() const {
	bool plottable = false;
	if ( !closing && itsCurrentPlot_ != NULL ){
		plottable = itsPlotManager_.isPlottable( itsCurrentPlot_ );
	}
	return plottable;
}



PlotMSPlotTab::~PlotMSPlotTab() { }


QList<QToolButton*> PlotMSPlotTab::toolButtons() const {
    QList<QToolButton*> list;
    foreach(PlotMSPlotSubtab* tab, itsSubtabs_) list << tab->toolButtons();
    return list;
}


void PlotMSPlotTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag) {
	(void)updateFlag;
    if(&p == itsCurrentParameters_ && itsCurrentPlot_ != NULL){
        setupForPlot(/*itsCurrentPlot_*/);
    }
}




void PlotMSPlotTab::plotsChanged(const PlotMSPlotManager& manager,
		int index, bool show) {

	int plotCount = manager.numPlots();
    if( plotCount == 0 ) {
        itsCurrentPlot_ = NULL;
        itsCurrentParameters_ = NULL;
    }
    else {
    	if ( itsCurrentPlot_ == NULL ){
    		if ( index < 0 ){
    			index = plotIndex;
    		}

    		if ( 0 <= index && index < plotCount ){
    			itsCurrentPlot_ = const_cast<PlotMSPlot*>(manager.plot(index));
    		}

    	}
    	if ( show && !closing ){
    		setupForPlot();
        }
    }
}


PlotMSPlot* PlotMSPlotTab::currentPlot() const {
	return itsCurrentPlot_;
}


PlotMSPlotParameters PlotMSPlotTab::currentlySetParameters() const {
    PlotMSPlotParameters params(itsPlotter_->getPlotFactory());
    if(itsCurrentParameters_ != NULL){
    	params = *itsCurrentParameters_;
    }

    foreach(PlotMSPlotSubtab* tab, itsSubtabs_){
    	tab->getValue(params);
    }
    return params;
}

PlotMSDataTab* PlotMSPlotTab::getData() {
	PlotMSDataTab* dataTab = this->findOrCreateDataTab();
	return dataTab;
}
String PlotMSPlotTab::getFileName() const {
	PlotMSPlotParameters params = currentlySetParameters();
    PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    String fileName;
    if ( d != NULL ){
    	fileName = d->filename();
    }
    return fileName;
}



String PlotMSPlotTab::getAveragingSummary() const {
	PlotMSPlotParameters params = currentlySetParameters();
	PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
	String avgStr;
	if ( d != NULL ){
		const PlotMSAveraging& average = d->averaging();
		avgStr = average.toStringShort();
	}
	return avgStr;
}


// Public Slots //

void PlotMSPlotTab::plot( bool forceReload ) {

  //  cout << "PlotMSPlotTab::plot()" << endl;

    if(itsCurrentParameters_ != NULL) {
        PlotMSPlotParameters params = currentlySetParameters();
        PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>(),
                     *cd = itsCurrentParameters_->typedGroup<PMS_PP_MSData>();
        PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>(),
                    *cc = itsCurrentParameters_->typedGroup<PMS_PP_Cache>();

        // Redo the plot if any of:
        //   1) Parameters have changed, 
        //   2) Cache loading was canceled,
        //   3) User was holding down the shift key
        //       Case #3 works by changing dummyChangeCount to 
        //       imitate case #1.
        //	 4) User has changed check box indicating we have changed from non-plotting to plotting.
		//
		// note as of Aug 2010: .casheReady() seems to return false even if cache was cancelled.
        bool paramsChanged = params != *itsCurrentParameters_;
        bool cancelledCache = !itsCurrentPlot_->cache().cacheReady();
        if (forceReload)    {
			forceReloadCounter_++;   
			paramsChanged=true;   // just to make sure we're noticed
		}
		
		// whether forced reload or not, must make sure PlotMSSelection in params
		// has some value set.   Otherwise, we might always get a "no match" 
		// and reload and therefore a bored user waiting.
		// Must remove constness of the reference returned by d->selection()
		PlotMSSelection &sel = (PlotMSSelection &)d->selection();
		sel.setForceNew(forceReloadCounter_);

        if (paramsChanged || cancelledCache ) {
            if (paramsChanged) {
                // check for "clear selections on axes change" setting
                if(itsParent_->getParameters().clearSelectionsOnAxesChange() &&
                       ((c != NULL && cc != NULL && (c->xAxis() != cc->xAxis() ||
                         c->yAxis() != cc->yAxis())) || (d != NULL && cd != NULL &&
                         d->filename() != cd->filename())))    {
                    vector<PlotCanvasPtr> canv = itsCurrentPlot_->canvases();
                    for(unsigned int i = 0; i < canv.size(); i++){
                    	if ( !canv[i].null() ){
                    		canv[i]->clearSelectedRects();
                    	}
                    }
                    itsPlotter_->getAnnotator().clearAll();
                }
                itsCurrentParameters_->holdNotification(this);
                *itsCurrentParameters_ = params;
                itsCurrentParameters_->releaseNotification();
            } else if (cancelledCache) {
                // Tell the plot to redraw itself because of the cache.
                itsCurrentPlot_->parametersHaveChanged(*itsCurrentParameters_,
                        PMS_PP::UPDATE_REDRAW & PMS_PP::UPDATE_CACHE);
            }
            plotsChanged(itsPlotManager_);
        }
    }
}


// Protected //

void PlotMSPlotTab::clearSubtabs() {
    itsSubtabs_.clear();
    tabWidget->clear();
}

void PlotMSPlotTab::clearAfter(int index) {
    while(index < itsSubtabs_.size()) {
        itsSubtabs_.removeAt(itsSubtabs_.size() - 1);
        tabWidget->removeTab(tabWidget->count() - 1);
    }
}



void PlotMSPlotTab::addSubtab(PlotMSPlotSubtab* tab) {
    insertSubtab(itsSubtabs_.size(), tab); }

void PlotMSPlotTab::insertSubtab(int index, PlotMSPlotSubtab* tab) {
    if(tab == NULL) return;
    
    if(itsSubtabs_.contains(tab)) {
        if(index == itsSubtabs_.indexOf(tab)) return;
        itsSubtabs_.removeAll(tab);
        tabWidget->removeTab(tabWidget->indexOf(tab));
    } else {
        connect(tab, SIGNAL(changed()), SLOT(tabChanged()));
    }
    
    itsSubtabs_.insert(index, tab);
    tabWidget->insertTab(index, tab, tab->tabName());
}




PlotMSAxesTab*  PlotMSPlotTab::addAxesSubtab ()
{
     return insertAxesSubtab ( itsSubtabs_.size() );
}


PlotMSAxesTab* PlotMSPlotTab::insertAxesSubtab (int index)
{
     PlotMSAxesTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSAxesTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL){
          tab = new PlotMSAxesTab (this, itsPlotter_);
          connect( tab, SIGNAL(yAxisIdentifierChanged(int,QString)),
        		  this, SLOT(changeAxisIdentifier(int,QString)));
          connect( tab, SIGNAL(yAxisIdentifierRemoved(int)),
        		  this, SLOT(removeAxisIdentifier(int)));
     }
     insertSubtab (index, tab);
     return tab;
}

void PlotMSPlotTab::changeAxisIdentifier( int index, QString id ){
	PlotMSDisplayTab* displayTab = findDisplayTab();
	if ( displayTab != NULL ){
		displayTab->setAxisIdentifier( index, id );
	}
}

void PlotMSPlotTab::removeAxisIdentifier( int index ){
	PlotMSDisplayTab* displayTab = findDisplayTab();
	if ( displayTab != NULL ){
		displayTab->removeAxisIdentifier( index );
	}
}



void PlotMSPlotTab::insertAxes (int index)
{
	insertAxesSubtab( index );
}


PlotMSCacheTab* PlotMSPlotTab::addCacheSubtab ()
{
     return insertCacheSubtab (itsSubtabs_.size ());
}

void PlotMSPlotTab::insertCache (int index){
	insertCacheSubtab( index );
}

PlotMSCacheTab* PlotMSPlotTab::insertCacheSubtab (int index)
{
     PlotMSCacheTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSCacheTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL)
          tab = new PlotMSCacheTab (this, itsPlotter_);
     insertSubtab (index, tab);
     return tab;
}




PlotMSCanvasTab*  PlotMSPlotTab::addCanvasSubtab (){
     return insertCanvasSubtab (itsSubtabs_.size ());
}


void PlotMSPlotTab::insertCanvas (int index){
	insertCanvasSubtab( index );
}

PlotMSCanvasTab*  PlotMSPlotTab::insertCanvasSubtab (int index){
     PlotMSCanvasTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSCanvasTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL)
          tab = new PlotMSCanvasTab (this, itsPlotter_);
     insertSubtab (index, tab);
     return tab;
}



PlotMSDataTab*  PlotMSPlotTab::addDataSubtab (){
     return insertDataSubtab (itsSubtabs_.size ());
}

PlotMSIterateTab* PlotMSPlotTab::findIterateTab(){
	PlotMSIterateTab* tab = NULL;
	foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
		tab = dynamic_cast < PlotMSIterateTab * >(t);
		if (tab != NULL){
			break;
		}
	}
	return tab;
}

PlotMSDataTab* PlotMSPlotTab::findOrCreateDataTab(){
	 PlotMSDataTab *tab = NULL;
	 foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
		 tab = dynamic_cast < PlotMSDataTab * >(t);
	     if (tab != NULL){
	    	 break;
	     }
	 }
	 if (tab == NULL){
		 tab = new PlotMSDataTab (this, itsPlotter_);
	 }
	 return tab;
}

PlotMSDataTab*  PlotMSPlotTab::insertDataSubtab (int index){
     PlotMSDataTab* tab = findOrCreateDataTab();
     insertSubtab (index, tab);
     return tab;
}

void  PlotMSPlotTab::insertData(int index){
	insertDataSubtab( index );
}

PlotMSDisplayTab*  PlotMSPlotTab::addDisplaySubtab ()
{
     return insertDisplaySubtab (itsSubtabs_.size ());
}


PlotMSDisplayTab* PlotMSPlotTab::insertDisplaySubtab (int index){
     PlotMSDisplayTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSDisplayTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL){
          tab = new PlotMSDisplayTab (this, itsPlotter_);
     }
     insertSubtab (index, tab);
     return tab;
}

PlotMSDisplayTab* PlotMSPlotTab::findDisplayTab(){
	PlotMSDisplayTab* tab = NULL;
	foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
		tab = dynamic_cast < PlotMSDisplayTab * >(t);
		if (tab != NULL){
			break;
		}
	}
	return tab;
}

void PlotMSPlotTab::insertDisplay(int index){
	insertDisplaySubtab( index );
}

PlotMSIterateTab*  PlotMSPlotTab::addIterateSubtab (){
     return insertIterateSubtab (itsSubtabs_.size ());
}

void PlotMSPlotTab::insertIterate(int index){
	insertIterateSubtab( index );
}

PlotMSIterateTab* PlotMSPlotTab::insertIterateSubtab (int index){
     PlotMSIterateTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSIterateTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL){
         tab = new PlotMSIterateTab (this, itsPlotter_);
         Int rows = 1;
         Int cols = 1;
         itsPlotManager_.getGridSize( rows, cols );
         tab->setGridSize( rows, cols );
     }
     insertSubtab (index, tab);
     return tab;
}



PlotMSTransformationsTab*  PlotMSPlotTab::addTransformationsSubtab (){
     return insertTransformationsSubtab (itsSubtabs_.size ());
}


PlotMSTransformationsTab*  PlotMSPlotTab::insertTransformationsSubtab (int index){
     PlotMSTransformationsTab *tab = NULL;
     foreach (PlotMSPlotSubtab * t, itsSubtabs_) {
          tab = dynamic_cast < PlotMSTransformationsTab * >(t);
          if (tab != NULL)
               break;
     }
     if (tab == NULL)
          tab = new PlotMSTransformationsTab (this, itsPlotter_);
     insertSubtab (index, tab);
     return tab;
}


void  PlotMSPlotTab::insertTransformations (int index){
	insertTransformationsSubtab( index );
}


// Private //

void PlotMSPlotTab::setupForPlot() {
    tabWidget->setEnabled(itsCurrentPlot_ != NULL);
    
    if(itsCurrentParameters_ != NULL){
        itsCurrentParameters_->removeWatcher(this);
    }
    itsCurrentParameters_ = NULL;
    
    if(itsCurrentPlot_ == NULL) return;
    bool oldupdate = itsUpdateFlag_;
    itsUpdateFlag_ = false;
    
    PlotMSPlotParameters& params = itsCurrentPlot_->parameters();
    params.addWatcher(this);
    itsCurrentParameters_ = &params;
    
    insertData(0);
    insertAxes(1);
    insertIterate(2);
    insertTransformations(3);
    insertDisplay(4);
    insertCanvas(5);
    clearAfter(6);

    // TODO update tool buttons
    
    foreach(PlotMSPlotSubtab* tab, itsSubtabs_){
    	tab->setValue(params);
    }
    
    itsUpdateFlag_ = oldupdate;
    tabChanged();
}



vector<PMS::Axis> PlotMSPlotTab::selectedLoadOrReleaseAxes(bool load) const {
    const PlotMSCacheTab* tab;
    foreach(const PlotMSPlotSubtab* t, itsSubtabs_) {
        if((tab = dynamic_cast<const PlotMSCacheTab*>(t)) != NULL)
            return tab->selectedLoadOrReleaseAxes(load);
    }
    return vector<PMS::Axis>();
}


void PlotMSPlotTab::tabChanged() {
    if(itsUpdateFlag_ && itsCurrentPlot_ != NULL) {
        itsUpdateFlag_ = false;
        foreach(PlotMSPlotSubtab* tab, itsSubtabs_){
            tab->update(*itsCurrentPlot_);
        }
        itsUpdateFlag_ = true;
    }
}

}
