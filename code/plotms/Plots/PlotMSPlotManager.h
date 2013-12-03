//# PlotMSPlotManager.h: Manages PlotMSPlots for plotms.
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
#ifndef PLOTMSPLOTMANAGER_H_
#define PLOTMSPLOTMANAGER_H_

#include <graphics/GenericPlotter/PlotFactory.h>
#include <plotms/Plots/PlotMSPages.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declararations
class PlotMSApp;
class PlotMSPlot;
class PlotMSPlotManagerWatcher;
class PlotMSPlotParameters;
class PlotMSOverPlot;


// Class which manages PlotMSPlots for plotms.  Mainly handles adding new plots
// and managing their PlotCanvases and the arrangement of PlotCanvases in the
// Plotter.  Any PlotMSPlots should be owned by the manager, which will handle
// deletion as necessary.
class PlotMSPlotManager {
    
    //# Friend class declarations.
    friend class PlotMSMultiPlot;
    friend class PlotMSOverPlot;
    
public:
    // Constructor.  Parent must be set using setParent() before manager can be
    // used.
    PlotMSPlotManager();
    
    // Destructor.
    ~PlotMSPlotManager();

    
    // Gets/Sets the parent PlotMS.
    // <group>
    PlotMSApp* parent();
    void setParent(PlotMSApp* parent);
    // </group>
    
    // Returns the plotter used by the parent.
    PlotterPtr plotter();
    
    // Adds the given watcher to this manager.
    void addWatcher(PlotMSPlotManagerWatcher* watcher);
    
    // Removes the given watcher from this manager.
    void removeWatcher(PlotMSPlotManagerWatcher* watcher);
    
    
    // Returns the number of plots.
    unsigned int numPlots() const;
    
    // Returns all or one of the plots.
    // <group>
    const vector<PlotMSPlot*>& plots() const;
    PlotMSPlot* plot(unsigned int index);
    const PlotMSPlot* plot(unsigned int index) const;
    // </group>
    
    // Returns all or one of the plot parameters.
    // <group>
    const vector<PlotMSPlotParameters*>& plotParameters() const;
    PlotMSPlotParameters* plotParameters(unsigned int index);
    // </group>
    
    // Creates a new PlotMSOverPlot, initializes it properly, adds it to the
    // plotter, and returns a pointer to it.  If parameters are given, they are
    // used; otherwise the defaults are used.
    PlotMSOverPlot* addOverPlot(const PlotMSPlotParameters* p = NULL);

    // Clears out all plots and canvases.
    void clearPlotsAndCanvases();

private:
    // Parent.
    PlotMSApp* itsParent_;
    
    // Plotter.
    PlotterPtr itsPlotter_;
    
    // Plot factory.
    PlotFactoryPtr itsFactory_;
    
    // Plots.  Will be deleted on destruction.
    vector<PlotMSPlot*> itsPlots_;
    
    // Plot parameters.  Will not be deleted on destruction.
    vector<PlotMSPlotParameters*> itsPlotParameters_;
    
    // Watchers.
    vector<PlotMSPlotManagerWatcher*> itsWatchers_;
    
    // Plot pages.
    PlotMSPages itsPages_;
    
    
    // Helper method for add*Plot methods.
    void addPlot(PlotMSPlot* plot, const PlotMSPlotParameters* p);
    
    // Notifies any watchers that the managed plots have changed.
    void notifyWatchers() const;
};


// Interface for any class that wants to be notified when the plots in the
// manager have changed.
class PlotMSPlotManagerWatcher {
public:
    // Constructor.
    PlotMSPlotManagerWatcher() { }
    
    // Destructor.
    virtual ~PlotMSPlotManagerWatcher() { }
    
    
    // Will be called whenever the plots in the manager change.
    virtual void plotsChanged(const PlotMSPlotManager& manager) = 0;
};

}

#endif /* PLOTMSPLOTMANAGER_H_ */
