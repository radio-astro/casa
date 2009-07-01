//# PlotMSAxesTab.qo.h: Plot tab for axes parameters.
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
#ifndef PLOTMSAXESTAB_QO_H_
#define PLOTMSAXESTAB_QO_H_

#include <plotms/GuiTabs/PlotMSAxesTab.ui.h>

#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <plotms/Gui/PlotRangeWidget.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSPlotSubtab that manages plot axes parameters.
class PlotMSAxesTab : public PlotMSPlotSubtab, Ui::AxesTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSAxesTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSAxesTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Axes"; }
    
    // Implements PlotMSPlotSubtab::getValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void update(const PlotMSPlot& plot);
    
private:
    // Widgets for the range for the x axis and y axis, respectively.
    PlotRangeWidget* itsXRangeWidget_, *itsYRangeWidget_;
};

}

#endif /* PLOTMSAXESTAB_QO_H_ */
