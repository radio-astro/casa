//# PlotMSOptionsTab.cc: Subclass of PlotMSTab for plotms options.
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
#include <plotms/GuiTabs/PlotMSOptionsTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSLoggerWidget.qo.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <QDebug>

namespace casa {

//////////////////////////////////
// PLOTMSOPTIONSTAB DEFINITIONS //
//////////////////////////////////

PlotMSOptionsTab::PlotMSOptionsTab(PlotMSPlotter* parent) : PlotMSTab(parent),
        itsParameters_(parent->getParent()->getParameters()),
        itsChangeFlag_(true), itsLoggerWidget_(NULL) {
    setupUi(this);
    
    // Add self as watcher to plotms parameters.
    itsParameters_.addWatcher(this);
    
    // Update GUI.
    parametersHaveChanged(itsParameters_, PlotMSWatchedParameters::NO_UPDATES);
    
    // Connect widgets
    connect(buttonStyle, SIGNAL(currentIndexChanged(int)),
            SLOT(toolButtonStyleChanged(int)));
    connect(clearSelection, SIGNAL(toggled(bool)),
            SLOT(clearSelectionChanged(bool)));
    connect(cacheSize, SIGNAL(toggled(bool)), SLOT(cachedImageSizeChanged()));
    connect(cacheSizeWidth, SIGNAL(valueChanged(int)),
            SLOT(cachedImageSizeChanged()));
    connect(cacheSizeHeight, SIGNAL(valueChanged(int)),
            SLOT(cachedImageSizeChanged()));
    connect(cacheSizeResolution, SIGNAL(clicked()),
            SLOT(cachedImageSizeScreenResolution()));
    connect(histLimitSpinner, SIGNAL(valueChanged(int)),
            SLOT(historyLimitChanged()));
    connect(rowSpin, SIGNAL(valueChanged(int)), SLOT(gridChanged()));
    connect(colSpin, SIGNAL(valueChanged(int)), SLOT(gridChanged()));
}

PlotMSOptionsTab::~PlotMSOptionsTab() { }

void PlotMSOptionsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag) {
    (void)updateFlag;
    if(&p == &itsParameters_) {
        bool oldChange = itsChangeFlag_;
        itsChangeFlag_ = false;
        
        if(itsLoggerWidget_ != NULL) {
            itsLoggerWidget_->setFilename(itsParameters_.logFilename());
            itsLoggerWidget_->setEvents(itsParameters_.logEvents());
            itsLoggerWidget_->setPriority(itsParameters_.logPriority());
        }

        clearSelection->setChecked(
                itsParameters_.clearSelectionsOnAxesChange());
        
        pair<int, int> size = itsParameters_.cachedImageSize();
        cacheSize->setChecked(size.first > 0 && size.second > 0);
        cacheSizeWidth->setValue(size.first);
        cacheSizeHeight->setValue(size.second);
        
        histLimitSpinner->setValue(itsParameters_.chooserHistoryLimit());
        rowSpin->setValue( itsParameters_.getRowCount());
        colSpin->setValue( itsParameters_.getColCount());

        itsChangeFlag_ = oldChange;
    }
}

void PlotMSOptionsTab::setupForMaxWidth(int maxWidth) {
    if(itsLoggerWidget_ != NULL) return;
    
    itsLoggerWidget_ = new PlotMSLoggerWidget(itsParameters_.logFilename(),
            itsParameters_.logEvents(), itsParameters_.logPriority(), maxWidth,
            false, false);
    QtUtilities::putInFrame(logFrame, itsLoggerWidget_);
    connect(itsLoggerWidget_, SIGNAL(changed()), SLOT(logChanged()));
    
    // Update GUI.
    parametersHaveChanged(itsParameters_, PlotMSWatchedParameters::NO_UPDATES);
}


void PlotMSOptionsTab::toolButtonStyleChanged(int newIndex) {
    Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
    if(newIndex == 1)      style = Qt::ToolButtonTextOnly;
    else if(newIndex == 2) style = Qt::ToolButtonTextBesideIcon;
    else if(newIndex == 3) style = Qt::ToolButtonTextUnderIcon;
    itsPlotter_->setToolButtonStyle(style);
}



void PlotMSOptionsTab::logChanged() {
    if(!itsChangeFlag_ || itsLoggerWidget_ == NULL) return;
    
    itsParameters_.holdNotification(this);
    itsParameters_.setLogFilename(itsLoggerWidget_->filename());
    itsParameters_.setLogFilter(itsLoggerWidget_->events(),
                                itsLoggerWidget_->priority());
    itsParameters_.releaseNotification();
}

void PlotMSOptionsTab::clearSelectionChanged(bool value) {
    if(!itsChangeFlag_) return;
    
    itsParameters_.holdNotification(this);
    itsParameters_.setClearSelectionsOnAxesChange(value);
    itsParameters_.releaseNotification();
}

void PlotMSOptionsTab::cachedImageSizeChanged() {
    if(!itsChangeFlag_) return;
    
    itsParameters_.holdNotification(this);
    if(cacheSize->isChecked())
        itsParameters_.setCachedImageSize(cacheSizeWidth->value(),
                cacheSizeHeight->value());
    else
        itsParameters_.setCachedImageSize(-1, -1);
    itsParameters_.releaseNotification();
}

void PlotMSOptionsTab::cachedImageSizeScreenResolution() {
    if(!itsChangeFlag_) return;
    
    itsChangeFlag_ = false;
    
    itsParameters_.holdNotification(this);
    itsParameters_.setCachedImageSizeToResolution();
    pair<int, int> size = itsParameters_.cachedImageSize();
    cacheSizeWidth->setValue(size.first);
    cacheSizeHeight->setValue(size.second);
    itsParameters_.releaseNotification();
    
    itsChangeFlag_ = true;
}

void PlotMSOptionsTab::historyLimitChanged() {
    if(!itsChangeFlag_) return;
    
    itsChangeFlag_ = false;
    itsParameters_.holdNotification(this);
    
    itsParameters_.setChooserListoryLimit(histLimitSpinner->value());
    
    itsParameters_.releaseNotification();
    itsChangeFlag_ = true;
}

void PlotMSOptionsTab::gridChanged(){
	if(!itsChangeFlag_){
		return;
	}

	int oldRowCount = itsParameters_.getRowCount();
	int oldColCount = itsParameters_.getColCount();
	int rowCount = rowSpin->value();
	int colCount = colSpin->value();

	bool changed = false;
	if ( oldRowCount != rowCount || oldColCount != colCount ){
		changed = true;
	}

	if ( changed ){
		itsChangeFlag_ = false;
		itsPlotter_->setShowProgress( false );
		itsParameters_.holdNotification(this);


		itsParameters_.setRowCount(rowCount);
		itsParameters_.setColCount(colCount);
		//Required so that if the user has unplotted changes, for example, selection
		//the new grid will pick up the changes.
		bool plotted = itsPlotter_->plot();
		if ( !plotted ){
			//There was no data change so trigger the grid change through
			// a release notification.
			itsParameters_.releaseNotification();
		}
		itsChangeFlag_ = true;
	}
}

}
