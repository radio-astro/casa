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

#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSData.h>
#include <plotms/PlotMS/PlotMSPlotParameters.h>

#include <graphics/GenericPlotter/PlotCanvas.h>
#include <graphics/GenericPlotter/PlotFactory.h>
#include <msvis/MSVis/VisSet.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMS;


// Abstract class for a single "plot" concept.  Generally speaking this one
// plot handles one data source across potentially many scatter plots and
// canvases, separated by whatever criteria the subclasses decide on.  The base
// class PlotMSPlot handles interfacing with the rest of PlotMS and MS file and
// selection, and provides some useful members and methods for subclasses.
class PlotMSPlot: public PlotMSParametersWatcher {
public:    
    // Constructor which takes the parent PlotMS object.  Starts out with
    // default parameters.
    PlotMSPlot(PlotMS* parent);
    
    // Destructor.
    virtual ~PlotMSPlot();
    
    
    // ABSTRACT METHODS //
    
    // Returns a human-readable name for this plot.  Does not have to be
    // unique.
    virtual String name() const = 0;
    
    // Returns the number of rows/columns that this plot requires.
    // <group>
    virtual unsigned int layoutNumRows() const = 0;
    virtual unsigned int layoutNumCols() const = 0;
    // </group>
    
    // Returns a reference to the plot's parameters.
    // <group>
    virtual const PlotMSPlotParameters& parameters() const = 0;
    virtual PlotMSPlotParameters& parameters() = 0;
    // </group>
    
    
    // IMPLEMENTED METHODS //
    
    // Returns the number of canvases that this plot requires.  Default value
    // is layoutNumRows() * layoutNumCols().
    virtual unsigned int layoutNumCanvases() const;
    
    // Initializes the plot with the given canvases.  Initializes any internal
    // plot objects via the protected initializePlot() method, then assigns
    // the plot objects to the canvases via the protected assignCanvases()
    // method, then calls parametersUpdated() to properly set the plot.
    // Drawing is held before these operations, and then released afterwards.
    // Returns true if all operations succeeded; false if at least one failed.
    virtual bool initializePlot(const vector<PlotCanvasPtr>& canvases);
    
    // Returns the canvases that have been assigned to this plot.
    virtual vector<PlotCanvasPtr> canvases() const;
    
    // Attaches/Detaches internal plot objects to their assigned canvases.
    // <group>
    virtual void attachToCanvases();
    virtual void detachFromCanvases();
    // </group>
    
    // Gets the plot's data source.
    // <group>
    virtual PlotMSData& data();
    virtual const PlotMSData& data() const;
    // </group>
    
    // Gets the plot's VisSet source.  WARNING: could be null if MS hasn't been
    // set/opened yet.
    // <group>
    virtual VisSet* visSet();
    virtual const VisSet* visSet() const;
    // </group>
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Calls the
    // protected updated methods as needed.
    virtual void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired);
    
    // Exports canvases associated with this plot to the given format.  Exports
    // to multiple files if the plot has more than one canvas.
    virtual bool exportToFormat(const PlotExportFormat& format);
    
protected:
    // ABSTRACT METHODS //
    
    // Initializes any internal plot objects, but does NOT set parameters or
    // attach to canvases.  Will only be called ONCE, before assignCanvases and
    // parametersUpdated, as long as the public initializePlot method is not
    // overridden.  Returns true for success, false for failure.
    virtual bool initializePlot() = 0;
    
    // Assigns internal plot objects to the canvases held in the itsCanvases_
    // member, but does NOT set parameters.  Will only be called ONCE, after
    // initializePlot and before parametersUpdated, as long as the public
    // initializePlot method is not overridden.  Returns true for success,
    // false for failure.
    virtual bool assignCanvases() = 0;
    
    // Helper method that updates the cache axes with the current parameters.
    // Should update the cache's loaded axes, and return true for success or
    // false for failure.
    virtual bool updateCache() = 0;
    
    // Helper method that updates the cache axes with the current parameters.
    // Should update the canvas display.
    virtual bool updateCanvas() = 0;
    
    // Helper method that updates the cache axes with the current parameters.
    // Should update the plot settings.
    virtual bool updatePlot() = 0;
    
    
    // IMPLEMENTED METHODS //
    
    // Note: this method MUST be called in the constructor of any children
    // classes.
    virtual void constructorSetup();
    
    // Helper method that updates the MS with the current parameters.  Updates
    // the MS, selected MS, and VisSet with the current parameters.
    virtual bool updateMS();    
    
    // Returns true if drawing is currently being held on at least one plot
    // canvas, false otherwise.
    virtual bool drawingHeld();
    
    // Holds drawing on all plot canvases.
    virtual void holdDrawing();
    
    // Releases drawing on all plot canvases, which will also cause a redraw.
    virtual void releaseDrawing();
    
    // Convenience methods for starting/stopping a log measurement event for
    // loading the cache, if the event flag has been set in PlotMS.
    // <group>
    virtual void startLogCache();    
    virtual void endLogCache();
    // </group>
    

    // MEMBERS //
    
    // PlotMS parent.
    PlotMS* itsParent_;
    
    // Plot factory for generating plot objects as needed.
    PlotFactoryPtr itsFactory_;
    
    // MS objects.
    // <group>
    MeasurementSet itsMS_;
    MeasurementSet itsSelectedMS_;
    VisSet* itsVisSet_;
    // </group>
    
    // Data.
    PlotMSData itsData_;
    
    // Plot objects.
    vector<MaskedScatterPlotPtr> itsPlots_;
    
    // Plot canvases.
    vector<PlotCanvasPtr> itsCanvases_;
    
    // Mapping of which plot objects are on which canvas.
    map<MaskedScatterPlot*, PlotCanvasPtr> itsCanvasMap_;
    
private:
    // Disable copy constructor and operator for now.
    // <group>
    PlotMSPlot(const PlotMSPlot& copy);
    PlotMSPlot& operator=(const PlotMSPlot& copy);
    // </group>
};


// Implementation of PlotMSPlot for a single plot on a single canvas.  Uses
// PlotMSSinglePlotParameters in addition to PlotMSPlotParameters.
class PlotMSSinglePlot : public PlotMSPlot {
public:    
    // Constructor which takes the PlotMS parent.  Starts out with default
    // parameters.
    PlotMSSinglePlot(PlotMS* parent);
    
    // Destructor.
    ~PlotMSSinglePlot();
    
    
    // Include overloaded methods.
    using PlotMSPlot::initializePlot;
    
    
    // Implements PlotMSPlot::name().
    String name() const;
    
    // Implements PlotMSPlot::layoutNumRows().
    unsigned int layoutNumRows() const;
    
    // Implements PlotMSPlot::layoutNumCols().
    unsigned int layoutNumCols() const;
    
    // Implements PlotMSPlot::parameters().
    // <group>
    const PlotMSPlotParameters& parameters() const;
    PlotMSPlotParameters& parameters();
    // </group>
    
    
    // Returns a reference to the plot's single parameters.
    // <group>
    const PlotMSSinglePlotParameters& singleParameters() const;
    PlotMSSinglePlotParameters& singleParameters();
    // </group>
    
protected:
    // Implements PlotMSPlot::initializePlot().
    bool initializePlot();
    
    // Implements PlotMSPlot::assignCanvases().
    bool assignCanvases();
    
    // Implements PlotMSPlot::updateCache().
    bool updateCache();
    
    // Implements PlotMSPlot::updateCanvas().
    bool updateCanvas();
    
    // Implements PlotMSPlot::updatePlot().
    bool updatePlot();
    
private:
    // Convenient access to single plot.
    MaskedScatterPlotPtr itsPlot_;
    
    // Convenient access to single canvas.
    PlotCanvasPtr itsCanvas_;
    
    // Plot parameters.
    PlotMSSinglePlotParameters itsParameters_;
    
    
    // Disable copy constructor and operator for now.
    // <group>
    PlotMSSinglePlot(const PlotMSSinglePlot& copy);
    PlotMSSinglePlot& operator=(const PlotMSSinglePlot& copy);
    // </group>
};

}

#endif /* PLOTMSPLOT_H_ */
