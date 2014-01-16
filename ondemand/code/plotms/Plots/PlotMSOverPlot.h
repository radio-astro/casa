//# PlotMSOverPlot.h: Subclass of PlotMSPlot for a single plot/canvas.
//# Copyright (C) 2012
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
#ifndef PLOTMSOVERPLOT_H_
#define PLOTMSOVERPLOT_H_

#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Data/PlotMSIndexer.h>


#include <casa/namespace.h>

namespace casa {

class PMS_PP_Canvas;
class PMS_PP_Axes;
class PMS_PP_Iteration;

class PlotMSOverPlot : public PlotMSPlot {
public:
    // Static

    // See PlotMSPlot::makeParameters()
    // <group>
    static PlotMSPlotParameters makeParameters(PlotMSApp *plotms);
    static void makeParameters(PlotMSPlotParameters &params, PlotMSApp *plotms);
    // </group>

    PlotMSOverPlot(PlotMSApp *parent);
    virtual ~PlotMSOverPlot();

    // Simple class to hold parameter to resume updating after a threaded
    // cache loading
    class TCLParams {
    public:
        // Parameters
        // <group>
        bool releaseWhenDone;
        bool updateCanvas;
        bool updateDisplay;
        bool endCacheLog;
        bool updateIteration;
        // </group>

        // Constructor
        TCLParams()
            :
            releaseWhenDone(false),
            updateCanvas(false),
            updateDisplay(false),
            endCacheLog(false)
        {}
    };

    // virtual methods
    String name() const;
    String spectype() const { return "Over"; }
    vector<MaskedScatterPlotPtr> plots() const;
    vector<PlotCanvasPtr> canvases() const;
    //void setupPlotSubtabs(PlotMSPlotTab &tab) const;
    virtual void setupPlotSubtabs(PlotInformationManager& tab) const;
    void attachToCanvases();
    void detachFromCanvases();
    //void plotTabHasChanged(PlotMSPlotTab&) {}
    Int iter() { return iter_; }

protected:
    // Template pattern methods
    bool assignCanvases(PlotMSPages &pages);
    bool initializePlot();
    bool parametersHaveChanged_(const PlotMSWatchedParameters &params,
                                int updateFlag, bool releaseWhenDone);
    void updateCanvasesAndPlotsForAxes();
    PlotMSRegions selectedRegions(const vector<PlotCanvasPtr> &canvases) const;

    void resize(PlotMSPages&, uInt rows, uInt cols);

    void constructorSetup();
    void updatePages();
    bool updateCache();
    bool updateCanvas();
    bool updateDisplay();
    void setColors();

    bool firstIter();
    bool prevIter();
    bool nextIter();
    bool lastIter();
    bool resetIter();
    bool setIter( int index );
    void recalculateIteration();

    void updatePlots();
    bool updateIndexing();

    void logPoints();
    void logIter(Int iter, Int nIter);

private:
    vector<vector<MaskedScatterPlotPtr> > itsPlots_;
    vector<vector<PlotCanvasPtr> > itsCanvases_;
    vector<vector</*QPScatterPlot**/ColoredPlotPtr> > itsColoredPlots_;
    TCLParams itsTCLParams_;

    Int iter_;
    Int iterStep_;

    // Disable copy constructor and assignment operator
    // <group>
    PlotMSOverPlot(const PlotMSOverPlot&);
    PlotMSOverPlot& operator=(const PlotMSOverPlot&);
    // </group>

    void clearCanvasProperties( int row, int col);
    void setCanvasProperties (int row, int col, PlotAxis cx, PlotAxis cy,
    		PMS::Axis x, PMS::Axis y, bool set, PMS_PP_Canvas *canv,
    		uInt rows, uInt cols, PMS_PP_Axes *axes, PMS_PP_Iteration *iter,
    		uInt iteration );

public:
    static void cacheLoaded(void *obj, bool wasCanceled)
    {
        PlotMSOverPlot *cobj = static_cast<PlotMSOverPlot*>(obj);
        if(cobj != NULL)
            cobj->cacheLoaded_(wasCanceled);
    }
private:
    void cacheLoaded_(bool wasCanceled);

};

} // namespace casa

#endif//PLOTMSOVERPLOT_H_
