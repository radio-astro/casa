//# PlotMSFlaggingTab.cc: Subclass of PlotMSTab for flagging.
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
#include <plotms/GuiTabs/PlotMSFlaggingTab.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

///////////////////////////////////
// PLOTMSFLAGGINGTAB DEFINITIONS //
///////////////////////////////////

PlotMSFlaggingTab::PlotMSFlaggingTab(PlotMSPlotter* parent) :
        PlotMSTab(parent) {
    setupUi(this);
    
    // Synchronize go button with flag action.
    itsPlotter_->synchronizeAction(PlotMSAction::SEL_FLAG, goButton);
}

PlotMSFlaggingTab::~PlotMSFlaggingTab() { }


PlotMSFlagging PlotMSFlaggingTab::getValue(PlotMSPlot* plot) const {
    return flaggingWidget->getValue(plot); }
void PlotMSFlaggingTab::setValue(const PlotMSFlagging& value) {
    flaggingWidget->setValue(value); }

}
