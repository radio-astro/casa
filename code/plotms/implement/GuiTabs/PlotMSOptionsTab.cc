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

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

//////////////////////////////////
// PLOTMSOPTIONSTAB DEFINITIONS //
//////////////////////////////////

PlotMSOptionsTab::PlotMSOptionsTab(PlotMSPlotter* parent) : PlotMSTab(parent),
        itsParameters_(parent->getParent()->getParameters()),
        itsChangeFlag_(true) {
    setupUi(this);
    
    // Log levels
    const vector<String>& levels = PlotMSLogger::levelStrings();
    for(unsigned int i = 0; i < levels.size(); i++)
        logLevel->addItem(levels[i].c_str());
    setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));
    
    // Add self as watcher to plotms parameters.
    itsParameters_.addWatcher(this);
    
    // Update GUI.
    parametersHaveChanged(itsParameters_, 0, false);
    
    // Connect widgets
    connect(buttonStyle, SIGNAL(currentIndexChanged(int)),
            SLOT(toolButtonStyleChanged(int)));
    connect(logLevel, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(logLevelChanged(const QString&)));
    connect(logDebug, SIGNAL(toggled(bool)), SLOT(logDebugChanged(bool)));
    connect(clearSelection, SIGNAL(toggled(bool)),
            SLOT(clearSelectionChanged(bool)));
    connect(cacheSize, SIGNAL(toggled(bool)), SLOT(cachedImageSizeChanged()));
    connect(cacheSizeWidth, SIGNAL(valueChanged(int)),
            SLOT(cachedImageSizeChanged()));
    connect(cacheSizeHeight, SIGNAL(valueChanged(int)),
            SLOT(cachedImageSizeChanged()));
    connect(cacheSizeResolution, SIGNAL(clicked()),
            SLOT(cachedImageSizeScreenResolution()));
}

PlotMSOptionsTab::~PlotMSOptionsTab() { }

QList<QToolButton*> PlotMSOptionsTab::toolButtons() const {
    return QList<QToolButton*>();
}

void PlotMSOptionsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) {
    if(&p == &itsParameters_) {
        bool oldChange = itsChangeFlag_;
        itsChangeFlag_ = false;
        
        setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));        
        logDebug->setChecked(itsParameters_.logDebug());

        clearSelection->setChecked(
                itsParameters_.clearSelectionsOnAxesChange());
        
        pair<int, int> size = itsParameters_.cachedImageSize();
        cacheSize->setChecked(size.first > 0 && size.second > 0);
        cacheSizeWidth->setValue(size.first);
        cacheSizeHeight->setValue(size.second);

        itsChangeFlag_ = oldChange;
    }
}


void PlotMSOptionsTab::toolButtonStyleChanged(int newIndex) {
    Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
    if(newIndex == 1)      style = Qt::ToolButtonTextOnly;
    else if(newIndex == 2) style = Qt::ToolButtonTextBesideIcon;
    else if(newIndex == 3) style = Qt::ToolButtonTextUnderIcon;
    itsPlotter_->setToolButtonStyle(style);
}

void PlotMSOptionsTab::logLevelChanged(const QString& newLevel) {
    if(!itsChangeFlag_) return;
    
    bool ok;
    PlotMSLogger::Level l = PlotMSLogger::level(newLevel.toStdString(), &ok);
    if(ok) {
        itsParameters_.holdNotification(this);
        itsParameters_.setLogLevel(l, itsParameters_.logDebug());
        itsParameters_.releaseNotification();
    }
}

void PlotMSOptionsTab::logDebugChanged(bool value) {
    if(!itsChangeFlag_) return;
    
    itsParameters_.holdNotification(this);
    itsParameters_.setLogLevel(itsParameters_.logLevel(), value);
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

}
