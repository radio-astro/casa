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
#include <plotms/GuiTabs/PlotMSTab.qo.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/Plots/PlotMSPlotManager.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class PlotMSAxesTab;
class PlotMSCacheTab;
class PlotMSCanvasTab;
class PlotMSDataTab;
class PlotMSDisplayTab;
class PlotMSIterateTab;
class PlotMSExportTab;
class PlotMSTransformationsTab;
class PlotMSPlotTab;


// Subclass of PlotMSTab for tabs that are meant to be used as subtabs in a
// PlotMSPlotTab.
class PlotMSPlotSubtab : public PlotMSTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent tab and plotter.
    PlotMSPlotSubtab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    virtual ~PlotMSPlotSubtab();
    
    
    // Gets/Sets the MS filename, selection, and averaging using a
    // PlotMSPlotParameters.
    // <group>
    virtual void getValue(PlotMSPlotParameters& params) const = 0;
    virtual void setValue(const PlotMSPlotParameters& params) = 0;
    // </group>
    
    // Updates the labels and other widgets in the subtab using the given
    // plot to check changes against.
    virtual void update(const PlotMSPlot& plot) = 0;
    
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged() to do
    // nothing unless overridden in the child class.
    virtual void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag)   {  (void)params,(void)updateFlag; }
    


signals:
    // This signal should be emitted whenever the value of the widget changes
    // because of user interaction.
    void changed();
    
protected:
    // Plot tab.
    PlotMSPlotTab* itsPlotTab_;
    
    
    // See PlotMSPlotTab::currentlySetParameters().
    PlotMSPlotParameters currentlySetParameters() const;
};


// Subclass of PlotMSTab that manages PlotMSPlots in the GUI.  Watches the
// current PlotMSPlot's parameters for changes to update the GUI as needed and
// watches the PlotMSPlotManager for changes to the plots.
class PlotMSPlotTab : public PlotMSTab, Ui::PlotTab /*,
                      public PlotMSPlotManagerWatcher*/{
    Q_OBJECT
    
    //# Friend class declarations.
    friend class PlotMSPlot;
    friend class PlotMSPlotter;
    
public:
    // Constructor which takes the parent plotter.
    PlotMSPlotTab(PlotMSPlotter* parent, int plotIndex = -1);
    
    // Destructor.
    ~PlotMSPlotTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Plot"; }
    
    // Overrides PlotMSTab::toolButtons().  Should be called AFTER any tabs
    // are added using addTab().
    QList<QToolButton*> toolButtons() const;
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the GUI as needed if the given parameters are the current PlotMSPlot's
    // parameters.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag);
    
    // Implements PlotMSPlotManagerWatcher::plotsChanged().
    void plotsChanged(const PlotMSPlotManager& manager, int index=-1, bool show = true);
    
    //Return the data to support overplots.
    PlotMSDataTab* getData();
    String getFileName() const;

    String getAveragingSummary() const;

    // Returns the currently selected plot.
    PlotMSPlot* currentPlot() const;
    
    // Returns the parameters currently set by the user on the GUI (but NOT
    // necessarily set on the underlying plot parameters).
    PlotMSPlotParameters currentlySetParameters() const;
    
    
    // Returns the axes that the user has selected to load into the cache.
    vector<PMS::Axis> selectedLoadAxes() const {
        return selectedLoadOrReleaseAxes(true);
    }
    
    // Returns the axes that the user has selected to release from the cache.
    vector<PMS::Axis> selectedReleaseAxes() const {
        return selectedLoadOrReleaseAxes(false);
    }

    // Returns true if the location of this plot is still valid after the grid
    // size has changed; false otherwise.
    bool setGridSize( int rowCount, int colCount );

    //Return the location on the grid for this plot.
    void getLocation( Int& rowIndex, Int& colIndex );

    // Remove this plot.
    void removePlot();

    //Returns whether this plot has a valid location on the page where
    //it can be displayed.
    bool isPlottable() const;

    //This was put in to support overplotting.  When two plots are sharing the
    //same canvas, we don't want to trigger a redraw until all the plots sharing
    //the same canvas are done updating their data in background threads.
    void completePlotting( bool success );

    //Remove old data from the plot.
    void clearData();

    bool managesPlot( PlotMSPlot* plot ) const;

public slots:
    // Slot for doing the plot, using the parameters set on the GUI for the
    // current plot.
    bool plot( bool forceReload);
    
    
protected:
    // Clears set subtabs.
    void clearSubtabs();
    
    // Clears set subtabs after (and including) the given index.
    void clearAfter(int index);
    
    // Adds the given subtab to the end of the tab widget.
    void addSubtab(PlotMSPlotSubtab* tab);
    
    // Inserts the given subtab in the tab widget.
    void insertSubtab(int index, PlotMSPlotSubtab* tab);
    
    // Inserts one of the known subtab types if it is not already present,
    // and returns a pointer to it.
    // <group>
    PlotMSAxesTab* addAxesSubtab();
    PlotMSAxesTab* insertAxesSubtab(int index);

    void insertAxes(int index);
    PlotMSCacheTab* addCacheSubtab();
    PlotMSCacheTab* insertCacheSubtab(int index);
    void insertCache(int index);
    PlotMSCanvasTab* addCanvasSubtab();
    PlotMSCanvasTab* insertCanvasSubtab(int index);
    void insertCanvas(int index);
    PlotMSDataTab* addDataSubtab();
    PlotMSDataTab* insertDataSubtab(int index);
    void insertData(int index);
    PlotMSDisplayTab* addDisplaySubtab();
    PlotMSDisplayTab* insertDisplaySubtab(int index);
    void insertDisplay(int index);
    PlotMSIterateTab* addIterateSubtab();
    PlotMSIterateTab* insertIterateSubtab(int index);
    void insertIterate(int index);

    PlotMSTransformationsTab* addTransformationsSubtab();
    PlotMSTransformationsTab* insertTransformationsSubtab(int index);
    void insertTransformations(int index);
    // </group>
    
    // Returns the first subtab with the given type, or NULL if there are none
    // of that type.
    template <class T>
    T* subtab() {
        T* t;
        foreach(PlotMSPlotSubtab* tab, itsSubtabs_)
            if((t = dynamic_cast<T*>(tab)) != NULL) return t;
        return NULL;
    }
    
private:    

    PlotMSDataTab* findOrCreateDataTab();
    PlotMSIterateTab* findIterateTab() const;
    PlotMSDisplayTab* findDisplayTab();
    PlotMSAxesTab* findAxesTab();

    // PlotMSPlotSubtab objects in tab widget.
    QList<PlotMSPlotSubtab*> itsSubtabs_;
    

    // Reference to plot manager.
    PlotMSPlotManager& itsPlotManager_;
    
    // Currently selected plot.
    PlotMSPlot* itsCurrentPlot_;
    
    // Parameters for the currently selected plot.
    PlotMSPlotParameters* itsCurrentParameters_;
    
    // Whether or not to check for changed parameters and update the GUI
    // accordingly.
    bool itsUpdateFlag_;
    bool closing;
    int plotIndex;

    int forceReloadCounter_;
    
    // Sets up the GUI to display the parameters for the given plot.
    void setupForPlot();
    
    // Returns the axes the user has selected to load or release, depending on
    // the load flag. 
    vector<PMS::Axis> selectedLoadOrReleaseAxes(bool load) const;
    
private slots:

    // Slot for when the user changes the value for any parameters.  Updates
    // the GUI to show which parameters have been changed (if any).
    void tabChanged();

    //A y-axis has changed its internal data.
    void changeAxisIdentifier( int index, QString id );

    //y-axis data has been removed.
    void removeAxisIdentifier( int index );

    //void plottableChanged();

};

}

#endif /* PLOTMSPLOTTAB_QO_H_ */
