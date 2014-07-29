//# PlotMSDataTab.qo.h: Plot tab for data parameters.
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
#ifndef PLOTMSDATATAB_QO_H_
#define PLOTMSDATATAB_QO_H_

#include <plotms/GuiTabs/PlotMSDataTab.ui.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMSAveragingWidget;
class PlotMSSelectionWidget;
class QtFileWidget;


// Subclass of PlotMSTab that handles picking MS filename, selection, and
// averaging parameters.
class PlotMSDataTab : public PlotMSPlotSubtab {
    Q_OBJECT
    
public:    
    // Constructor which takes the parent tab and plotter.
    PlotMSDataTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    virtual ~PlotMSDataTab();

    // Implements PlotMSPlotSubtab::getValue().
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);
    
    QString tabName() const { return "Data"; }

    //Returns the name of the file that will be plotted.
    String getFileName() const;

    //Returns any selection criteria for the data.
    String getSelection() const;

    //Returns the type of averaging for the data.
    String getAveraging() const;

signals:
	void changed();

private:
    
    // Widget for MS selection.
    PlotMSSelectionWidget* itsSelectionWidget_;
    
    // Widget for MS averaging.
    PlotMSAveragingWidget* itsAveragingWidget_;

    QtFileWidget* itsFileWidget_;

    Ui::PlotMSDataTabClass ui;
};

}

#endif /* PLOTMSDATATAB_QO_H_ */
