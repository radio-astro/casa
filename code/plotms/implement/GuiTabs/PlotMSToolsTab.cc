//# PlotMSToolsTab.cc: Subclass of PlotMSTab for tools management.
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
#include <plotms/GuiTabs/PlotMSToolsTab.qo.h>

#include <plotms/Actions/PlotMSAction.h>
#include <plotms/PlotMS/PlotMS.h>

namespace casa {

////////////////////////////////
// PLOTMSTOOLSTAB DEFINITIONS //
////////////////////////////////

PlotMSToolsTab::PlotMSToolsTab(PlotMSPlotter* parent) : PlotMSTab(parent) {
    setupUi(this);
    
    // Get the actions that buttons will be connected to.
    const QMap<PlotMSAction::Type, QAction*>& actionMap =
        itsPlotter_->plotActionMap();
    
    // Set up hand tools
    QAction* act = actionMap.value(PlotMSAction::TOOL_MARK_REGIONS);
    connect(handMarkRegions, SIGNAL(toggled(bool)), act,
            SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    act = actionMap.value(PlotMSAction::TOOL_ZOOM);
    connect(handZoom, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    act = actionMap.value(PlotMSAction::TOOL_PAN);
    connect(handPan, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    
    // Set up selected regions
    connect(regionsClear, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::SEL_CLEAR_REGIONS), SLOT(trigger()));
    connect(regionsFlag, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::SEL_FLAG), SLOT(trigger()));
    connect(regionsUnflag, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::SEL_UNFLAG), SLOT(trigger()));
    connect(regionsLocate, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::SEL_LOCATE), SLOT(trigger()));
    
    // Set up stack
    connect(stackBack, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_BACK), SLOT(trigger()));
    connect(stackBase, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_BASE), SLOT(trigger()));
    connect(stackForward, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_FORWARD), SLOT(trigger()));
    
    // Set up tracker
    act = actionMap.value(PlotMSAction::TRACKER_HOVER);
    connect(trackerHover, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), trackerHover, SLOT(setChecked(bool)));
    act = actionMap.value(PlotMSAction::TRACKER_DISPLAY);
    connect(trackerDisplay, SIGNAL(toggled(bool)), act,SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), trackerDisplay,SLOT(setChecked(bool)));
    
    // Set up iteration
    connect(iterationFirst, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_FIRST), SLOT(trigger()));
    connect(iterationPrev, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_PREV), SLOT(trigger()));
    connect(iterationNext, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_NEXT), SLOT(trigger()));
    connect(iterationLast, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_LAST), SLOT(trigger()));
    
    // Set up hold/release
    act = actionMap.value(PlotMSAction::HOLD_RELEASE_DRAWING);
    connect(holdReleaseDrawing, SIGNAL(toggled(bool)), act,
            SLOT(setChecked(bool)));
    connect(act, SIGNAL(changed()), SLOT(holdReleaseActionChanged()));
}

PlotMSToolsTab::~PlotMSToolsTab() { }

QList<QToolButton*> PlotMSToolsTab::toolButtons() const {
    return QList<QToolButton*>() << regionsClear << regionsFlag
           << regionsUnflag << regionsLocate << stackBack << stackBase
           << stackForward << iterationFirst << iterationPrev << iterationNext
           << iterationLast;
}

void PlotMSToolsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) { }

void PlotMSToolsTab::showIterationButtons(bool show) {
    iterationBox->setVisible(show);
}


void PlotMSToolsTab::notifyTrackerChanged(PlotTrackerTool& tool) {
    if(trackerDisplay->isChecked())
        trackerEdit->setText(tool.getAnnotation()->text().c_str());
}


void PlotMSToolsTab::holdReleaseActionChanged() {
    QAction* act = itsPlotter_->plotActionMap().value(
                   PlotMSAction::HOLD_RELEASE_DRAWING);
    holdReleaseDrawing->blockSignals(true);
    holdReleaseDrawing->setChecked(act->isChecked());
    holdReleaseDrawing->setText(act->text());
    holdReleaseDrawing->blockSignals(false);
}

void PlotMSToolsTab::toolChanged() {
    const QMap<PlotMSAction::Type, QAction*>& actionMap =
        itsPlotter_->plotActionMap();
    bool mark = actionMap[PlotMSAction::TOOL_MARK_REGIONS]->isChecked(),
         zoom = actionMap[PlotMSAction::TOOL_ZOOM]->isChecked(),
         pan  = actionMap[PlotMSAction::TOOL_PAN]->isChecked();
    
    handMarkRegions->blockSignals(true);
    handZoom->blockSignals(true);
    handPan->blockSignals(true);
    
    if(!mark && !zoom && !pan) handNone->setChecked(true);
    else if(mark) handMarkRegions->setChecked(true);
    else if(zoom) handZoom->setChecked(true);
    else if(pan) handPan->setChecked(true);
    
    handMarkRegions->blockSignals(false);
    handZoom->blockSignals(false);
    handPan->blockSignals(false);
}

}
