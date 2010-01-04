//# PlotMSPlot.h: High level plot concept across potentially multiple objects.
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
#ifndef PLOTMSPLOT_H_
#define PLOTMSPLOT_H_

#include <graphics/GenericPlotter/PlotFactory.h>
#include <plotms/Data/PlotMSData.h>
#include <plotms/PlotMS/PlotMSRegions.h>
#include <plotms/Plots/PlotMSPlotParameters.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMS;
class PlotMSPages;
class PlotMSPlotTab;

// Abstract class for a single "plot" concept.  Generally speaking this one
// plot handles one data source across potentially many scatter plots and
// canvases, separated by whatever criteria the subclasses decide on.  The base
// class PlotMSPlot handles interfacing with the rest of PlotMS and MS file and
// selection, and provides some useful members and methods for subclasses.
class PlotMSPlot : public PlotMSParametersWatcher {
public:
    // Static //    
    
    // Returns a new instance of PlotMSPlotParameters suitable for use with
    // this class, using the given PlotMS parent.
    static PlotMSPlotParameters makeParameters(PlotMS* plotms);
    
    // Updates the given PlotMSPlotParameters to be suitable for use with this
    // class, using the given PlotMS parent.
    static void makeParameters(PlotMSPlotParameters& params, PlotMS* plotms);
    
    
    // Non-Static //
    
    // Constructor which takes the parent PlotMS object.  Starts out with
    // default data parameters.
    PlotMSPlot(PlotMS* parent);
    
    // Destructor.
    virtual ~PlotMSPlot();
    
    
    // ABSTRACT METHODS //
    
    // Returns a human-readable name for this plot.  Does not have to be
    // unique.
    virtual String name() const = 0;
    
    // Returns the plots assigned to this plot.
    virtual vector<MaskedScatterPlotPtr> plots() const = 0;
    
    // Returns the canvases that have been assigned to this plot.
    virtual vector<PlotCanvasPtr> canvases() const = 0;
    
    // Sets up subtabs for the given plot tab, using the add/insert subtab
    // methods.
    virtual void setupPlotSubtabs(PlotMSPlotTab& tab) const = 0;
    
    // Attaches/Detaches internal plot objects to their assigned canvases.
    // <group>
    virtual void attachToCanvases() = 0;
    virtual void detachFromCanvases() = 0;
    // </group>
    
    // Called when the GUI settings in the plot tab have changed.
    virtual void plotTabHasChanged(PlotMSPlotTab& tab) = 0;
    
    
    // IMPLEMENTED METHODS //
    
    // Returns a reference to the plot's parameters.
    // <group>
    virtual const PlotMSPlotParameters& parameters() const;
    virtual PlotMSPlotParameters& parameters();
    // </group>
    
    // Returns the visible canvases (accessible via
    // PlotMSPlotter::currentCanvases()) associated with this plot.
    virtual vector<PlotCanvasPtr> visibleCanvases() const;
    
    // Returns all selected regions on all canvases associated with this plot.
    virtual PlotMSRegions selectedRegions() const;
    
    // Returns selected regions on all visible canvases (accessible via
    // PlotMSPlotter::currentCanvases()) associated with this plot.
    virtual PlotMSRegions visibleSelectedRegions() const;
    
    // Initializes the plot with the given canvases.  Initializes any internal
    // plot objects via the protected initializePlot() method, then assigns
    // the plot objects to the canvases via the protected assignCanvases()
    // method, then calls parametersUpdated() to properly set the plot.
    // Drawing is held before these operations, and then released afterwards.
    // Returns true if all operations succeeded; false if at least one failed.
    virtual bool initializePlot(PlotMSPages& pages);
    
    // Gets the plot's data source.
    // <group>
    virtual PlotMSData& data();
    virtual const PlotMSData& data() const;
    // </group>
    
    // Gets the plot's parent.
    virtual PlotMS* parent();
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the data parameters and then calls parametersHaveChanged_().
    virtual void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag);
    
    // Calls the dataChanged() method on the MaskedScatterPlots.  This WILL
    // cause a redraw of the affected canvases.
    virtual void plotDataChanged();
    
    // Exports canvases associated with this plot to the given format.  Exports
    // to multiple files if the plot has more than one canvas.
    virtual bool exportToFormat(const PlotExportFormat& format);
    
    // This method should be called when the given canvas (which was owned by
    // this plot) was disowned.
    virtual void canvasWasDisowned(PlotCanvasPtr canvas);
    
protected:
    // ABSTRACT METHODS //
    
    // Generates and assigns canvases that this plot will be using, with the
    // given PlotMSPages object.  This is called when the plot is first
    // created, and can be called by the plot itself if its canvas layout
    // has changed (in which case this method should check for the canvases
    // that have already been assigned to it in the page).
    virtual bool assignCanvases(PlotMSPages& pages) = 0;
    
    // Initializes any internal plot objects, but does NOT set parameters or
    // attach to canvases.  Will only be called ONCE, before assignCanvases and
    // parametersUpdated, as long as the public initializePlot method is not
    // overridden.  Returns true for success, false for failure.
    virtual bool initializePlot() = 0;
    
    // Updates plot members for parameters specific to the child plot type.
    // Returns true if the drawing should be released right away; if false is
    // returned, the child class is expect to release drawing when finished.
    virtual bool parametersHaveChanged_(const PlotMSWatchedParameters& params,
            int updateFlag, bool releaseWhenDone) = 0;
    
    // Helper method for selectedRegions() and visibleSelectedRegions() that
    // returns the selected regions for plots in the given canvases.
    virtual PlotMSRegions selectedRegions(
            const vector<PlotCanvasPtr>& canvases) const = 0;
    
    
    // IMPLEMENTED METHODS //
    
    // Note: this method MUST be called in the constructor of any children
    // classes.
    virtual void constructorSetup();
    
    // Force data update by clearing the cache
    virtual bool updateData() { itsData_.clearCache(); return True; };   
    
    // Returns true if drawing is currently being held on all plot canvases,
    // false otherwise.
    virtual bool allDrawingHeld();
    
    // Holds drawing on all plot canvases.
    virtual void holdDrawing();
    
    // Releases drawing on all plot canvases, which will also cause a redraw.
    virtual void releaseDrawing();
    

    // MEMBERS //
    
    // PlotMS parent.
    PlotMS* itsParent_;
    
    // Plot factory for generating plot objects as needed.
    PlotFactoryPtr itsFactory_;
    
    // Parameters.
    PlotMSPlotParameters itsParams_;
    
    // Data.
    PlotMSData itsData_;
    
private:
    // Disable copy constructor and operator for now.
    // <group>
    PlotMSPlot(const PlotMSPlot& copy);
    PlotMSPlot& operator=(const PlotMSPlot& copy);
    // </group>
};

}

#endif /* PLOTMSPLOT_H_ */
