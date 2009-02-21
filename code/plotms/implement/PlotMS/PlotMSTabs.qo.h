//# PlotMSTabs.qo.h: Tab GUI widgets.
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
#ifndef PLOTMSTABS_QO_H_
#define PLOTMSTABS_QO_H_

#include <plotms/PlotMS/PlotMSOptionsTab.ui.h>
#include <plotms/PlotMS/PlotMSPlotTab.ui.h>
#include <plotms/PlotMS/PlotMSToolsTab.ui.h>

#include <plotms/PlotMS/PlotMSParameters.h>
#include <plotms/PlotMS/PlotMSPlotManager.h>
#include <plotms/PlotMS/PlotWidgets.qo.h>

#include <QLineEdit>
#include <QMap>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMS;
class PlotMSPlotter;


// Abstract parent for any widget that goes in the tabbed side panel of the
// PlotMSPlotter.
class PlotMSTab : public QWidget, public PlotMSParametersWatcher {
    Q_OBJECT
    
public:
    // Constructor that takes the parent and plotter for this tab.
    PlotMSTab(PlotMS* parent, PlotMSPlotter* plotter);
    
    // Destructor.
    virtual ~PlotMSTab();
    
    
    // Returns all tool buttons on this tab.
    virtual QList<QToolButton*> toolButtons() const = 0;
    
protected:
    // Parent.
    PlotMS* itsParent_;
    
    // Plotter.
    PlotMSPlotter* itsPlotter_;
    
    // Default text for labels, so that they can be easily switched to red or
    // normal depending on whether the item has changed or not.
    QMap<QLabel*, QString> itsLabelDefaults_;
    
    
    // Updates the text for the given label using the given changed flag.
    // Uses the static changedText method along with the label default text in
    // itsLabelDefaults_.
    void changedText(QLabel* label, bool changed);
    
    
    // Returns the given text altered as required by the changed flag.  If
    // changed is false, the text is not changed (except to replace spaces with
    // &nbsp;); if changed is true, the text is made red.
    // <group>
    static QString changedText(const QString& text, bool changed);
    static QString changedText(const String& text, bool changed) {
        QString str(text.c_str());
        return changedText(str, changed); }
    static QString changedText(const char* text, bool changed) {
        QString str(text);
        return changedText(str, changed); }
    // </group>
    
    // Convenience methods for setting the current value of the given QComboBox
    // to the given.
    // <group>
    static bool setChooser(QComboBox* chooser, const QString& value);
    static bool setChooser(QComboBox* chooser, const String& value) {
        QString str(value.c_str());
        return setChooser(chooser, str); }
    static bool setChooser(QComboBox* chooser, const char* value) {
        QString str(value);
        return setChooser(chooser, str); }
    // </group>
};


// Subclass of PlotMSTab that handles options for PlotMSPlotter.  Currently:
// * tool button style, and
// * log level.
// Watches PlotMS's PlotMSParameters for changes to update the GUI as needed.
class PlotMSOptionsTab : public PlotMSTab, Ui::OptionsTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent and plotter.
    PlotMSOptionsTab(PlotMS* parent, PlotMSPlotter* plotter);
    
    // Destructor.
    ~PlotMSOptionsTab();
    
    
    // Implements PlotMSTab::toolButtons().
    QList<QToolButton*> toolButtons() const;
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the GUI as needed if the given parameters are the PlotMS parent's
    // parameters.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired);
    
private:
    // Watched parameters for PlotMS.
    PlotMSParameters& itsParameters_;
    
private slots:
    // When the user changes the tool button style on the GUI.
    void toolButtonStyleChanged(int newIndex);
    
    // When the user changes the log level on the GUI.
    void logLevelChanged(const QString& newLevel);
};


// Subclass of PlotMSTab that manages PlotMSPlots in the GUI.  WARNING:
// currently can only handle PlotMSSinglePlots.  Watches the current
// PlotMSPlot's parameters for changes to update the GUI as needed and watches
// the PlotMSPlotManager for changes to the plots.
class PlotMSPlotTab : public PlotMSTab, Ui::PlotTab,
                      public PlotMSPlotManagerWatcher {
    Q_OBJECT
    
public:
    // Constructor which takes the parent and plotter.
    PlotMSPlotTab(PlotMS* parent, PlotMSPlotter* plotter);
    
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


// Subclass of PlotMSTab that handles the tools for the current plot.  Watches
// no parameters.
class PlotMSToolsTab : public PlotMSTab, Ui::ToolsTab,
                       public PlotTrackerToolNotifier {
    Q_OBJECT
    
public:
    // Constructor which takes the parent and the plotter.
    PlotMSToolsTab(PlotMS* parent, PlotMSPlotter* plotter);
    
    // Destructor.
    ~PlotMSToolsTab();
    
    
    // Implements PlotMSTab::toolButtons().
    QList<QToolButton*> toolButtons() const;
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged.  Currently
    // does nothing.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired);
    
    
    // Show/hide the iteration buttons on this tab.
    void showIterationButtons(bool show);
    
protected:
    // Implements PlotTrackerToolNotifier::notifyTrackerChanged().  Updates the
    // tracker information in the line edit, if the proper checkbox is toggled.
    void notifyTrackerChanged(PlotTrackerTool& tool);
    
private slots:
    // Slot to update the text of the hold/release drawing button to reflect
    // the current state the of the global hold/release action.
    void holdReleaseActionChanged();
    
    // Slot to update the currently select tool based on the state of the
    // global tool actions.
    void toolChanged();
};

}

#endif /* PLOTMSTABS_QO_H_ */
