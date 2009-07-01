//# PlotMSMSTab.qo.h: Plot tab for MS parameters.
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
#ifndef PLOTMSMSTAB_QO_H_
#define PLOTMSMSTAB_QO_H_

#include <plotms/GuiTabs/PlotMSMSTab.ui.h>

#include <plotms/Gui/PlotMSAveragingWidget.qo.h>
#include <plotms/Gui/PlotMSSelectionWidget.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSTab that handles picking MS filename, selection, and
// averaging parameters.
class PlotMSMSTab : public PlotMSPlotSubtab, Ui::MSTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSMSTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSMSTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "MS"; }
    
    // Implements PlotMSPlotSubtab::getValue().
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);
    
private:
    // Widget for MS filename.
    QtFileWidget* itsFileWidget_;
    
    // Widget for MS selection.
    PlotMSSelectionWidget* itsSelectionWidget_;
    
    // Widget for MS averaging.
    PlotMSAveragingWidget* itsAveragingWidget_;
};

}

#endif /* PLOTMSMSTAB_QO_H_ */
