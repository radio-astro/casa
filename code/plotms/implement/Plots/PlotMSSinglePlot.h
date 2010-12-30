//# PlotMSSinglePlot.h: Subclass of PlotMSPlot for a single plot/canvas.
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
#ifndef PLOTMSSINGLEPLOT_H_
#define PLOTMSSINGLEPLOT_H_

#include <plotms/Plots/PlotMSPlot.h>

#include <casa/namespace.h>

namespace casa {

// Implementation of PlotMSPlot for a single plot on a single canvas.  Uses
// PlotMSSinglePlotParameters in addition to PlotMSPlotParameters.
class PlotMSSinglePlot : public PlotMSPlot {
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
    PlotMSSinglePlot(PlotMSApp* parent);
    
    // Destructor.
    ~PlotMSSinglePlot();
    
    
    // Include overloaded methods.
    using PlotMSPlot::initializePlot;
    
    
    // Implements PlotMSPlot::name().
    String name() const;
    
    // Implements PlotMSPlot::plots().
    vector<MaskedScatterPlotPtr> plots() const;
    
    // Implements PlotMSPlot::canvases().
    vector<PlotCanvasPtr> canvases() const;
    
    // Implements PlotMSPlot::setupPlotSubtabs().
    void setupPlotSubtabs(PlotMSPlotTab& tab) const;
    
    // Implements PlotMSPlot::attachToCanvases().
    void attachToCanvases();
    
    // Implements PlotMSPlot::detachFromCanvases().
    void detachFromCanvases();
    
    // Implements PlotMSPlot::plotTabHasChanged().
    void plotTabHasChanged(PlotMSPlotTab& tab) { (void)tab; }
    
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
    
private:    
    // Convenient access to single plot.
    MaskedScatterPlotPtr itsPlot_;
    
    // Convenient access to single plot as a colored plot.
    ColoredPlotPtr itsColoredPlot_;
    
    // Convenient access to single canvas.
    PlotCanvasPtr itsCanvas_;
    
    // See TCLParams class documentation
    TCLParams itsTCLParams_;
    
    
    // Disable copy constructor and operator for now.
    // <group>
    PlotMSSinglePlot(const PlotMSSinglePlot& copy);
    PlotMSSinglePlot& operator=(const PlotMSSinglePlot& copy);
    // </group>
    
    // Updates helper methods.
    // <group>
    bool updateCache();
    bool updateCanvas();
    bool updateDisplay();
    // </group>
    
    // Post-thread methods for loading the cache.
    public:
	static void cacheLoaded (void *obj, bool wasCanceled)
	{
	     PlotMSSinglePlot *cobj = static_cast < PlotMSSinglePlot * >(obj);
	     if (cobj != NULL)
	          cobj->cacheLoaded_ (wasCanceled);
	}
	
	private:
	void cacheLoaded_ (bool wasCanceled);
	

};

}

#endif /* PLOTMSSINGLEPLOT_H_ */
