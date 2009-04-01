//# PlotMSPlotTab.qo.h: Subclass of PlotMSTab for controlling plot parameters.
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
#ifndef PLOTMSPLOTTAB_QO_H_
#define PLOTMSPLOTTAB_QO_H_

#include <plotms/GuiTabs/PlotMSPlotTab.ui.h>

#include <plotms/Gui/PlotWidgets.qo.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>
#include <plotms/Plots/PlotMSPlotManager.h>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSTab that manages PlotMSPlots in the GUI.  WARNING:
// currently can only handle PlotMSSinglePlots.  Watches the current
// PlotMSPlot's parameters for changes to update the GUI as needed and watches
// the PlotMSPlotManager for changes to the plots.
class PlotMSPlotTab : public PlotMSTab, Ui::PlotTab,
                      public PlotMSPlotManagerWatcher {
    Q_OBJECT
    
public:
    // Constructor which takes the parent and plotter.
    PlotMSPlotTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSPlotTab();
    
    
    // Implements PlotMSTab::toolButtons().
    QList<QToolButton*> toolButtons() const;
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the GUI as needed if the given parameters are the current PlotMSPlot's
    // parameters.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired);
    
    // Implements PlotMSPlotManagerWatcher::plotsChanged().
    void plotsChanged(const PlotMSPlotManager& manager);
    
    
    // Returns the currently selected plot.  WARNING: currently can only handle
    // PlotMSSinglePlots.
    PlotMSPlot* currentPlot() const;
    
    // Returns the parameters currently set by the user on the GUI (but NOT
    // necessarily set on the underlying plot parameters).
    PlotMSSinglePlotParameters currentlySetParameters() const;
    
    // Returns the axes that the user has selected to load into the cache.
    vector<PMS::Axis> selectedLoadAxes() const {
        return selectedLoadOrReleaseAxes(true); }
    
    // Returns the axes that the user has selected to release from the cache.
    vector<PMS::Axis> selectedReleaseAxes() const {
        return selectedLoadOrReleaseAxes(false); }
    
private:
    // Widgets for file selection for the MS and export, respectively.
    PlotFileWidget* itsMSFileWidget_, *itsExportFileWidget_;
    
    // Widgets for titles/labels for the plot, canvas, x axis, and y axis,
    // respectively.
    PlotLabelWidget* itsPlotTitleWidget_, *itsCanvasTitleWidget_,
                     *itsXLabelWidget_, *itsYLabelWidget_;
    
    // Widgets for symbols for unflagged and flagged points, respectively.
    PlotSymbolWidget* itsSymbolWidget_, *itsMaskedSymbolWidget_;
    
    // Widgets for the range for the x axis and y axis, respectively.
    PlotRangeWidget* itsXRangeWidget_, *itsYRangeWidget_;
    
    // Widgets for MS selection.
    QMap<PlotMSSelection::Field, QLineEdit*> itsSelectionValues_;
    
    // Reference to plot manager.
    PlotMSPlotManager& itsPlotManager_;
    
    // Currently selected plot.
    PlotMSSinglePlot* itsCurrentPlot_;
    
    // Parameters for the currently selected plot.
    PlotMSSinglePlotParameters* itsCurrentParameters_;
    
    // Whether or not to check for changed parameters and update the GUI
    // accordingly.
    bool itsUpdateFlag_;
    
    
    // Sets up the GUI to display the parameters for the given plot.
    void setupForPlot(PlotMSPlot* plot);
    
    // Returns the axes the user has selected to load or release, depending on
    // the load flag.
    vector<PMS::Axis> selectedLoadOrReleaseAxes(bool load) const;
    
    // Updates the cache axes tables to reflect the current plot.
    void updateCacheTables();
    
private slots:
    // Slot for when the user changes the "go" chooser on the top.
    void goChanged(int index);

    // Slot for when the user clicks "go" on the top of the tab.
    void goClicked();

    // Slot for when the user changes the value for any parameters.  Updates
    // the GUI to show which parameters have been changed (if any).
    void tabChanged();
    
    // Slot for clearing the MS parameters (but doesn't apply the changes).
    void msClear();
    
    // Slot for plotting after setting parameters.
    void plot();
    
    // Slot for exporting the current plot.
    void exportClicked();
};

}

#endif /* PLOTMSPLOTTAB_QO_H_ */
