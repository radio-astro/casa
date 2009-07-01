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
#include <plotms/Plots/PlotMSSinglePlotParameters.h>

#include <casa/namespace.h>

namespace casa {

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
    
    // Implements PlotMSPlot::layoutNumCanvases().
    unsigned int layoutNumCanvases() const;
    
    // Implements PlotMSPlot::layoutNumPages().
    unsigned int layoutNumPages() const;
    
    // Implements PlotMSPlot::generateCanvases().
    vector<PlotCanvasPtr> generateCanvases(PlotMSPages& pages);
    
    // Implements PlotMSPlot::setupPlotSubtabs().
    void setupPlotSubtabs(PlotMSPlotTab& tab) const;
    
    // Implements PlotMSPlot::parameters().
    // <group>
    const PlotMSPlotParameters& parameters() const;
    PlotMSPlotParameters& parameters();
    // </group>
    
    // Implements PlotMSPlot::selectedRegions().
    PlotMSRegions selectedRegions() const;
    
    // Implements PlotMSPlot::visibleSelectedRegions().
    PlotMSRegions visibleSelectedRegions() const;
    
    
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
    
    // Implements PlotMSPlot::hasThreadedCaching().
    bool hasThreadedCaching() const { return true; }
    
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

#endif /* PLOTMSSINGLEPLOT_H_ */
