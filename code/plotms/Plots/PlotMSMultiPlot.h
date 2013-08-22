//# PlotMSMultiPlot.h: plot with one x (or y) vs. many y's (or x's).
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
#ifndef PLOTMSMULTIPLOT_H_
#define PLOTMSMULTIPLOT_H_

#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSSinglePlot.h>

//#include <casa/namespace.h>

namespace casa {

// PlotMSMultiPlot is a subclass of PlotMSPlot that handles plots that have one
// x (or y) versus many y's (or x's).
class PlotMSMultiPlot : public PlotMSPlot {
public:
    // Static //
    
    // See PlotMSPlot::makeParameters().
    // <group>
    static PlotMSPlotParameters makeParameters(PlotMSApp* plotms);
    static void makeParameters(PlotMSPlotParameters& params, PlotMSApp* plotms);
    // </group>
    
    
    // Non-Static //
    
    // Constructor which takes PlotMS parent.
    PlotMSMultiPlot(PlotMSApp* parent);
    
    // Destructor.
    ~PlotMSMultiPlot();
    
    
    // Implements PlotMSPlot::name().
    String name() const;
    
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
    void plotTabHasChanged(PlotMSPlotTab& tab);
    
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

    // A temporary override until cache iterated slicing works
    virtual void releaseDrawing();    
private:
    // Grid of plots.
    vector<vector<MaskedScatterPlotPtr> > itsPlots_;
    
    // Grid of canvases.
    vector<vector<PlotCanvasPtr> > itsCanvases_;
    
    vector<vector<PlotMSData*> > itsIndexers_;
    
    // See PlotMSSinglePlot::TCLParams class documentation
    PlotMSSinglePlot::TCLParams itsTCLParams_;
    
    
    // Updates the plot subtabs for the given number of axes.
    void updateSubtabs(unsigned int nRows, unsigned int nCols) const;
    
    // Updates the canvases and plots for a new number of axes, using the plot
    // parameters.
    void updateCanvasesAndPlotsForAxes();
    
    // Updates helper methods.
    // <group>
    bool updateCache();
    bool updateCanvas();
    bool updateDisplay();
    // </group>
    
    // Post-thread methods for loading the cache.
public:
	static void	cacheLoaded (void *obj, bool wasCanceled)
	{
		PlotMSMultiPlot *cobj = static_cast < PlotMSMultiPlot * >(obj);
		if (cobj != NULL)
			cobj->cacheLoaded_ (wasCanceled);
	}
	
private:
	void cacheLoaded_ (bool wasCanceled);


};

}

#endif /* PLOTMSMULTIPLOT_H_ */
