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

#include <casaqt/QtUtilities/QtActionGroup.qo.h>
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
    
    // Set up synchronizer
    QtActionSynchronizer* sync = new QtActionSynchronizer(parent);
    
    // Synchronize hand tool buttons
    sync->synchronize(actionMap.value(PlotMSAction::TOOL_MARK_REGIONS),
            handMarkRegions);
    sync->synchronize(actionMap.value(PlotMSAction::TOOL_ZOOM),
            handZoom);
    sync->synchronize(actionMap.value(PlotMSAction::TOOL_PAN),
            handPan);
    sync->synchronize(actionMap.value(PlotMSAction::TOOL_ANNOTATE_TEXT),
            handAnnotateText);
    sync->synchronize(actionMap.value(PlotMSAction::TOOL_ANNOTATE_RECTANGLE),
            handAnnotateRectangle);
    
    // Synchronize selected region buttons
    sync->synchronize(actionMap.value(PlotMSAction::SEL_CLEAR_REGIONS),
            regionsClear);
    sync->synchronize(actionMap.value(PlotMSAction::SEL_FLAG), regionsFlag);
    sync->synchronize(actionMap.value(PlotMSAction::SEL_UNFLAG),regionsUnflag);
    sync->synchronize(actionMap.value(PlotMSAction::SEL_LOCATE),regionsLocate);
    
    // Synchronize stack buttons
    sync->synchronize(actionMap.value(PlotMSAction::STACK_BACK), stackBack);
    sync->synchronize(actionMap.value(PlotMSAction::STACK_BASE), stackBase);
    sync->synchronize(actionMap.value(PlotMSAction::STACK_FORWARD),
            stackForward);
    
    // Synchronize tracker buttons
    sync->synchronize(actionMap.value(PlotMSAction::TRACKER_HOVER),
            trackerHover);
    sync->synchronize(actionMap.value(PlotMSAction::TRACKER_DISPLAY),
            trackerDisplay);
    
    // Synchronize iteration buttons
    sync->synchronize(actionMap.value(PlotMSAction::ITER_FIRST),
            iterationFirst);
    sync->synchronize(actionMap.value(PlotMSAction::ITER_PREV), iterationPrev);
    sync->synchronize(actionMap.value(PlotMSAction::ITER_NEXT), iterationNext);
    sync->synchronize(actionMap.value(PlotMSAction::ITER_LAST), iterationLast);
    
    // Synchronize hold/release button
    sync->synchronize(actionMap.value(PlotMSAction::HOLD_RELEASE_DRAWING),
            holdReleaseDrawing);
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
    iterationBox->setVisible(show); }


void PlotMSToolsTab::toolsUnchecked() { handNone->setChecked(true); }


void PlotMSToolsTab::notifyTrackerChanged(PlotTrackerTool& tool) {
    if(trackerDisplay->isChecked())
        trackerEdit->setText(tool.getAnnotation()->text().c_str());
}

}
