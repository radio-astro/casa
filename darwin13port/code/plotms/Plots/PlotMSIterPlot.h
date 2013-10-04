//# PlotMSIterPlot.h: Subclass of PlotMSPlot for a single plot/canvas.
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
#ifndef PLOTMSITERPLOT_H_
#define PLOTMSITERPLOT_H_

#include <plotms/Plots/PlotMSPlot.h>

//#include <casa/namespace.h>
#include <plotms/Data/PlotMSIndexer.h>
#include <plotms/Data/MSCache.h>

namespace casa {

// Implementation of PlotMSPlot for a single plot on a single canvas.  Uses
// PlotMSSinglePlotParameters in addition to PlotMSPlotParameters.
class PlotMSIterPlot : public PlotMSPlot {
public:
    // Static //    
    
    // See PlotMSPlot::makeParameters().
    // <group>
    static PlotMSPlotParameters makeParameters(PlotMSApp* plotms);
    static void makeParameters(PlotMSPlotParameters& params, PlotMSApp* plotms);
    // </group>
    
    
    // Simple class to hold parameter to resume updating after a threaded
    // cache loading.
    class TCLParams {
    public:
        // Constructor.
        TCLParams() : releaseWhenDone(false), updateCanvas(false),
                updateDisplay(false), endCacheLog(false) { }
        
        // Destructor.
        ~TCLParams() { }
        
        // Parameters.
        // <group>
        bool releaseWhenDone;
        bool updateCanvas;
        bool updateDisplay;
        bool endCacheLog;
        // </group>
    };
    
    
    // Non-Static //
    
    // Constructor which takes the PlotMS parent.  Starts out with default
    // parameters.
    PlotMSIterPlot(PlotMSApp* parent);
    
    // Destructor.
    ~PlotMSIterPlot();
    
    
    // Include overloaded methods.
    using PlotMSPlot::initializePlot;
    
    
    // Implements PlotMSPlot::name().
    String name() const;
    
    // implement PlotMSPlot::spectype().
    String spectype() const { return "Iter";};

    // Implements PlotMSPlot::plots().
    vector<MaskedScatterPlotPtr> plots() const;
    
    // Implements PlotMSPlot::canvases().
    vector<PlotCanvasPtr> canvases() const;
    
    // Implements PlotMSPlot::setupPlotSubtabs().
    void setupPlotSubtabs(/*PlotMSPlotTab*/PlotInformationManager& tab) const;
    
    // Implements PlotMSPlot::attachToCanvases().
    void attachToCanvases();
    
    // Implements PlotMSPlot::detachFromCanvases().
    void detachFromCanvases();
    
    // Implements PlotMSPlot::plotTabHasChanged().
    //void plotTabHasChanged(PlotMSPlotTab& tab) { (void)tab; }

    // Step the iteration
    bool firstIter();
    bool prevIter();
    bool nextIter();
    bool lastIter();

    virtual Int iter() { return iter_; };

protected:
    // Implements PlotMSPlot::assignCanvases().
    bool assignCanvases(PlotMSPages& pages);
    
    // Implements PlotMSPlot::initializePlot().
    bool initializePlot();
    
    // Implements PlotMSPlot::parametersHaveChanged_().
    bool parametersHaveChanged_(const PlotMSWatchedParameters& params,
                int updateFlag, bool releaseWhenDone);
    
    // Implements PlotMSPlot::selectedRegions().
    PlotMSRegions selectedRegions(
                const vector<PlotCanvasPtr>& canvases) const;
    
    // Overrides PlotMSPlot::constructorSetup().
    void constructorSetup();

    // Clear the underlying data cache
    virtual bool updateData() { itsCache_->clear();  return True; };   


private:    
    // Convenient access to single plot.
    MaskedScatterPlotPtr itsPlot_;
    
    // Convenient access to single plot as a colored plot.
    ColoredPlotPtr itsColoredPlot_;
    
    // Convenient access to single canvas.
    PlotCanvasPtr itsCanvas_;
    
    // See TCLParams class documentation
    TCLParams itsTCLParams_;
    
  // The indexer
  PlotMSIndexerPtr itsIndexer_;


  // Iteration counter
  Int iter_;

    // Disable copy constructor and operator for now.
    // <group>
    PlotMSIterPlot(const PlotMSIterPlot& copy);
    PlotMSIterPlot& operator=(const PlotMSIterPlot& copy);
    // </group>
    
    // Updates helper methods.
    // <group>
    bool updateCache();
    bool updateIndexing();
    bool resetIter();
    bool updateCanvas();
    bool updateDisplay();
    // </group>

    // Set default colors
    void setColors();

    // Log the number of points plotted; 
    void logPoints();
    
    // Log iteration info
    void logIter(Int iter,Int nIter);

    // Post-thread methods for loading the cache.
    public:
	static void cacheLoaded (void *obj, bool wasCanceled)
	{
	     PlotMSIterPlot *cobj = static_cast < PlotMSIterPlot * >(obj);
	     if (cobj != NULL)
	          cobj->cacheLoaded_ (wasCanceled);
	}
	
	private:
	void cacheLoaded_ (bool wasCanceled);
	

};

}

#endif /* PLOTMSSINGLEPLOT_H_ */
