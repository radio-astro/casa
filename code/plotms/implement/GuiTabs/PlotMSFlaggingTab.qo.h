//# PlotMSFlaggingTab.qo.h: Subclass of PlotMSTab for flagging.
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
#ifndef PLOTMSFLAGGINGTAB_QO_H_
#define PLOTMSFLAGGINGTAB_QO_H_

#include <plotms/GuiTabs/PlotMSFlaggingTab.ui.h>

#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations.
class PlotMSPlot;


// Subclass of PlotMSTab that handles flagging.  WARNING: currently can only a
// single PlotMSPlot.
class PlotMSFlaggingTab : public PlotMSTab, Ui::FlaggingTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSFlaggingTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSFlaggingTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Flagging"; }
    
    // Implements PlotMSTab::toolButtons().
    QList<QToolButton*> toolButtons() const;
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Currently
    // does nothing.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired) { }
    
    
    // See PlotMSFlaggingWidget::getValue().
    PlotMSFlagging getValue(PlotMSPlot* plot = NULL) const;
};

}

#endif /* PLOTMSFLAGGINGTAB_QO_H_ */
