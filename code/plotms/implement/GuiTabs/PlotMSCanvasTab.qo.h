//# PlotMSCanvasTab.qo.h: Plot tab to manage canvas parameters.
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
#ifndef PLOTMSCANVASTAB_QO_H_
#define PLOTMSCANVASTAB_QO_H_

#include <plotms/GuiTabs/PlotMSCanvasTab.ui.h>

#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSPlotSubtab to manage canvas parameters.
class PlotMSCanvasTab : public PlotMSPlotSubtab, Ui::CanvasTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSCanvasTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSCanvasTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Canvas"; }
    
    // Implements PlotMSPlotSubtab::getValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.  Must be called after
    // PlotMSDisplayTab::getValue() for the "same as plot title" option to
    // work.
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void update(const PlotMSPlot& plot);
    
private:
    // Widgets for titles/labels for the canvas, x axis, and y axis,
    // respectively.
    QtLabelWidget* itsTitleWidget_, *itsXLabelWidget_, *itsYLabelWidget_;
    
    // Widgets for lines for grid major and minor.
    PlotLineWidget* itsGridMajorLineWidget_, *itsGridMinorLineWidget_;
};

}

#endif /* PLOTMSCANVASTAB_QO_H_ */
