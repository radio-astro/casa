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
#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {






////////////////////////////////
// PLOTMSTOOLSTAB DEFINITIONS //
////////////////////////////////

PlotMSToolsTab::PlotMSToolsTab(PlotMSPlotter* parent) : PlotMSTab(parent) {
    setupUi(this);
    
    // Synchronize hand tool buttons
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_MARK_REGIONS,
            handMarkRegions);
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_SUBTRACT_REGIONS,
            handSubtractRegions);
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_ZOOM, handZoom);
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_PAN, handPan);
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_ANNOTATE_TEXT,
            handAnnotateText);
    itsPlotter_->synchronizeAction(PlotMSAction::TOOL_ANNOTATE_RECTANGLE,
            handAnnotateRectangle);
    
    // Synchronize selected region buttons
    itsPlotter_->synchronizeAction(PlotMSAction::SEL_CLEAR_REGIONS,
            regionsClear);
    itsPlotter_->synchronizeAction(PlotMSAction::SEL_FLAG, regionsFlag);
    itsPlotter_->synchronizeAction(PlotMSAction::SEL_UNFLAG,regionsUnflag);
    itsPlotter_->synchronizeAction(PlotMSAction::SEL_LOCATE, regionsLocate);
    
    // Synchronize stack buttons
    itsPlotter_->synchronizeAction(PlotMSAction::STACK_BACK, stackBack);
    itsPlotter_->synchronizeAction(PlotMSAction::STACK_BASE, stackBase);
    itsPlotter_->synchronizeAction(PlotMSAction::STACK_FORWARD, stackForward);
    
    // Synchronize tracker buttons
    itsPlotter_->synchronizeAction(PlotMSAction::TRACKER_ENABLE_HOVER, trackerHoverChk);
    itsPlotter_->synchronizeAction(PlotMSAction::TRACKER_ENABLE_DISPLAY,
            trackerDisplayChk);
    
    // Synchronize iteration buttons
    itsPlotter_->synchronizeAction(PlotMSAction::ITER_FIRST, iterationFirst);
    itsPlotter_->synchronizeAction(PlotMSAction::ITER_PREV, iterationPrev);
    itsPlotter_->synchronizeAction(PlotMSAction::ITER_NEXT, iterationNext);
    itsPlotter_->synchronizeAction(PlotMSAction::ITER_LAST, iterationLast);
    
    // Synchronize hold/release button
    itsPlotter_->synchronizeAction(PlotMSAction::HOLD_RELEASE_DRAWING,
            holdReleaseDrawing);

	connect( clearRecordedValuesBut,  SIGNAL(clicked()),  SLOT(clearRecordedValues()) );
    tracker_key_handler = new TrackerKeyHandler(this);
}


PlotMSToolsTab::~PlotMSToolsTab() { 
	delete tracker_key_handler;
}


QList<QToolButton*> PlotMSToolsTab::toolButtons() const {
    return QList<QToolButton*>() << regionsClear << regionsFlag
           << regionsUnflag << regionsLocate << stackBack << stackBase
           << stackForward << iterationFirst << iterationPrev << iterationNext
           << iterationLast;
}

void PlotMSToolsTab::showIterationButtons(bool show) {
    iterationBox->setVisible(show); }



void PlotMSToolsTab::toolsUnchecked() { 
	handNone->setChecked(true); 
}


void PlotMSToolsTab::notifyTrackerChanged(PlotTrackerTool& tool)   {
    if(trackerDisplayChk->isChecked())
        trackerValueDisplay->setText(tool.getAnnotation()->text().c_str());
}


TrackerKeyHandler::TrackerKeyHandler(PlotMSToolsTab *t)    
	: tools_tab(t){
}


void TrackerKeyHandler::handleKey(const PlotKeyEvent& event)   {
	(void)event;
	tools_tab->takeSnapshotOfTrackerValue();	
}

void PlotMSToolsTab::takeSnapshotOfTrackerValue()    {
    if(trackerDisplayChk->isChecked())   {
		QString txt = trackerValueDisplay->text();   // puts it on clipboard; handy for user
		//txt.append("\n"); // appears that .append automatically linebreaks
		trackerRecordedValues->append(txt);
	}
}


void PlotMSToolsTab::clearRecordedValues()  {
	trackerRecordedValues->setText("");
}


}
