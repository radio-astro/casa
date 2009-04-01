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
        itsParameters_(parent->getParent()->getParameters()) {
    setupUi(this);
    
    // Log levels
    const vector<String>& levels = PlotMSLogger::levelStrings();
    for(unsigned int i = 0; i < levels.size(); i++)
        logLevel->addItem(levels[i].c_str());
    setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));
    
    // Add self as watcher to plotms parameters.
    itsParameters_.addWatcher(this);
    
    // Connect widgets
    connect(buttonStyle, SIGNAL(currentIndexChanged(int)),
            SLOT(toolButtonStyleChanged(int)));
    connect(logLevel, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(logLevelChanged(const QString&)));
    connect(logDebug, SIGNAL(toggled(bool)), SLOT(logDebugChanged(bool)));
    connect(clearSelection, SIGNAL(toggled(bool)),
            SLOT(clearSelectionChanged(bool)));
}

PlotMSOptionsTab::~PlotMSOptionsTab() { }

QList<QToolButton*> PlotMSOptionsTab::toolButtons() const {
    return QList<QToolButton*>();
}

void PlotMSOptionsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) {
    if(&p == &itsParameters_) {
        logLevel->blockSignals(true);
        setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));
        logLevel->blockSignals(false);
        
        logDebug->blockSignals(true);
        logDebug->setChecked(itsParameters_.logDebug());
        logDebug->blockSignals(false);
        
        clearSelection->blockSignals(true);
        clearSelection->setChecked(
                itsParameters_.clearSelectionsOnAxesChange());
        clearSelection->blockSignals(false);
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
    bool ok;
    PlotMSLogger::Level l = PlotMSLogger::level(newLevel.toStdString(), &ok);
    if(ok) {
        itsParameters_.holdNotification(this);
        itsParameters_.setLogLevel(l, itsParameters_.logDebug());
        itsParameters_.releaseNotification();
    }
}

void PlotMSOptionsTab::logDebugChanged(bool value) {
    itsParameters_.holdNotification(this);
    itsParameters_.setLogLevel(itsParameters_.logLevel(), value);
    itsParameters_.releaseNotification();
}

void PlotMSOptionsTab::clearSelectionChanged(bool value) {
    itsParameters_.holdNotification(this);
    itsParameters_.setClearSelectionsOnAxesChange(value);
    itsParameters_.releaseNotification();
}

}
