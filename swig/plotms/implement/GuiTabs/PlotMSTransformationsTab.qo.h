//# PlotMSTransformationsTab.qo.h: Plot tab for transformations parameters.
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
#ifndef PLOTMSTRANSFORMATIONSTAB_QO_H_
#define PLOTMSTRANSFORMATIONSTAB_QO_H_

#include <plotms/GuiTabs/PlotMSTransformationsTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMSTransformationsWidget;


// Subclass of PlotMSTab that handles picking MS filename, selection, and
// averaging parameters.
class PlotMSTransformationsTab : public PlotMSPlotSubtab, Ui::TransTab {
    Q_OBJECT
    
public:    
    // Constructor which takes the parent tab and plotter.
    PlotMSTransformationsTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSTransformationsTab();
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Trans"; }

    // Implements PlotMSPlotSubtab::getValue().
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);
    
private:

    // Widget for MS selection.
    PlotMSTransformationsWidget* itsTransformationsWidget_;
};

}

#endif /* PLOTMSTRANSFORMATIONSTAB_QO_H_ */
