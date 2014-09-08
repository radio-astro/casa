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
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/PlotMS/PlotMSRegions.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPage.h>
#include <plotms/Data/PlotMSIndexer.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMSApp;
class PlotMSPages;
class PMS_PP_Cache;
class PMS_PP_Canvas;
class PMS_PP_Axes;
class PMS_PP_Iteration;
class PMS_PP_Display;

// Class for a single "plot" concept.  Generally speaking this one
// plot handles one data source across potentially many scatter plots and
// canvases, separated by whatever criteria the subclasses decide on.  The 
// class PlotMSPlot handles interfacing with the rest of PlotMS and MS file and
// selection, and provides some useful members and methods for subclasses.
class PlotMSPlot : public PlotMSParametersWatcher {
public:
    // Static //    
    
    // Returns a new instance of PlotMSPlotParameters suitable for use with
    // this class, using the given PlotMS parent.
    static PlotMSPlotParameters makeParameters(PlotMSApp* plotms);
    
    // Updates the given PlotMSPlotParameters to be suitable for use with this
    // class, using the given PlotMS parent.
    static void makeParameters(PlotMSPlotParameters& params, PlotMSApp* plotms);
    
    void customizeAutoSymbol( const PlotSymbolPtr& baseSymbol, uInt dataSize  );
    // Non-Static //
    
    // Constructor which takes the parent PlotMS object.  Starts out with
    // default data parameters.
    PlotMSPlot(PlotMSApp* parent);
    
    // Destructor.
    ~PlotMSPlot();

    void resize(PlotMSPages&, uInt rows, uInt cols);

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
    
    // Returns a human-readable name for this plot.  Does not have to be
    // unique.
    String name() const;

    // Returns specialization Id for this plot
    String spectype() const { return "Unknown";};
    
    // Returns the plots assigned to this plot.
    vector<MaskedScatterPlotPtr> plots() const;
    
    // Returns the canvases that have been assigned to this plot.
    vector<PlotCanvasPtr> canvases() const;
    
    // Attaches/Detaches internal plot objects to their assigned canvases.
    // <group>
    void attachToCanvases();
    void detachFromCanvases();
    Int iter() { return iter_; }
    // </group>
    
    //The cache load did not succeed so clear the plot and the cache.
    void dataMissing();
    
    //This method was written because in scripting mode, there was a segfault when
    //the grid size was changed (for example 2 x 3 to 1x1).  It was in a draw thread
    //that had stale data it was touching after its associated canvas was deleted.
    //The purpose of the method is to wait for the draw threads to finish and then
    //keep them from redrawing while the deletion is in progress.
    void waitForDrawing( bool holdDrawing );

    // Returns a reference to the plot's parameters.
    // <group>
    const PlotMSPlotParameters& parameters() const;
    PlotMSPlotParameters& parameters();
    // </group>
    
    // Returns the visible canvases (accessible via
    // PlotMSPlotter::currentCanvases()) associated with this plot.
    vector<PlotCanvasPtr> visibleCanvases() const;
   
    // Returns all selected regions on all canvases associated with this plot.
    virtual PlotMSRegions selectedRegions() const;
  
    // PlotMSPlotter::currentCanvases()) associated with this plot.
    PlotMSRegions visibleSelectedRegions() const;

    // Initializes the plot with the given canvases.  Initializes any internal
    // plot objects via the protected initializePlot() method, then assigns
    // the plot objects to the canvases via the protected assignCanvases()
    // method, then calls parametersUpdated() to properly set the plot.
    // Drawing is held before these operations, and then released afterwards.
    // Returns true if all operations succeeded; false if at least one failed.
    bool initializePlot(PlotMSPages& pages);
    
    // Gets the plot's data source.
    // <group>
    PlotMSCacheBase& cache() { return *itsCache_; };
    const PlotMSCacheBase& cache() const { return *itsCache_; };
    // </group>
    
    // Gets the plot's parent.
    PlotMSApp* parent() { return itsParent_; };
    
    // Steps the iteration
    bool firstIter();
    bool prevIter();
    bool nextIter(); 
    bool lastIter();
    bool setIter( int index );
    bool resetIter();
    void recalculateIteration();
    Int nIter();

    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the data parameters and then calls parametersHaveChanged_().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag);
    
    // Calls the dataChanged() method on the MaskedScatterPlots.  This WILL
    // cause a redraw of the affected canvases.
    void plotDataChanged();
    
    //Returns true if the plot is an iteration plot.
    bool isIteration() const;

    // Exports canvases associated with this plot to the given format.  Exports
    // to multiple files if the plot has more than one canvas.
    bool exportToFormat(const PlotExportFormat& format);
    void exportToFormatCancel();
    
    void cacheLoaded_(bool wasCanceled);

    // This method should be called when the given canvas (which was owned by
    // this plot) was disowned.
    void canvasWasDisowned(PlotCanvasPtr canvas);
    vector<PMS::Axis> getCachedAxes();
    vector<PMS::DataColumn> getCachedData();

    Record locateInfo(int plotIterIndex, const Vector<PlotRegion>& regions,
    		bool showUnflagged, bool showFlagged, bool selectAll ) const ;

    PlotLogMessage* locateRange( int plotIterIndex, const Vector<PlotRegion> & regions,
    		bool showUnflagged, bool showFlagged);

    PlotLogMessage* flagRange( int canvasIndex, casa::PlotMSFlagging& flagging,
    		const Vector<PlotRegion>& regions, bool showFlagged);

    // Generates and assigns canvases that this plot will be using, with the
    // given PlotMSPages object.  This is called when the plot is first
    // created, and can be called by the plot itself if its canvas layout
    // has changed (in which case this method should check for the canvases
    // that have already been assigned to it in the page).
    bool assignCanvases(PlotMSPages& pages);
    void updateLocation();

    //Clear the title and axes from all this plots canvases.
    void clearCanvases();

    //Whether a thread is currently updating the cache.
    bool isCacheUpdating() const;
    void setCacheUpdating( bool updating );

    void updatePlots();
    bool updateIndexing();

    void logPoints();
    void logIter(Int iter, Int nIter);

    static void cacheLoaded(void *obj, bool wasCanceled)
    {
        PlotMSPlot *cobj = static_cast<PlotMSPlot*>(obj);
        if(cobj != NULL){
            cobj->cacheLoaded_(wasCanceled);
        }
    }

protected:
    
    
    // Initializes any internal plot objects, but does NOT set parameters or
    // attach to canvases.  Will only be called ONCE, before assignCanvases and
    // parametersUpdated, as long as the public initializePlot method is not
    // overridden.  Returns true for success, false for failure.
    bool initializePlot();
    
    // Updates plot members for parameters specific to the child plot type.
    // Returns true if the drawing should be released right away; if false is
    // returned, the child class is expect to release drawing when finished.
    bool parametersHaveChanged_(const PlotMSWatchedParameters& params,
                                int updateFlag, bool releaseWhenDone );
    
    // Helper method for selectedRegions() and visibleSelectedRegions() that
    // returns the selected regions for plots in the given canvases.
    PlotMSRegions selectedRegions(
            const vector<PlotCanvasPtr>& canvases) const;
    
    void constructorSetup();
    void updatePages();
    bool updateCache( );
    bool updateCanvas();
    bool updateDisplay();
    void setColors();
    
    // Force data update by clearing the cache
    bool updateData();
    
    // Returns true if drawing is currently being held on all plot canvases,
    // false otherwise.
    bool allDrawingHeld();
    
    // Holds drawing on all plot canvases.
    void holdDrawing();
    
    // Releases drawing on all plot canvases, which will also cause a redraw.
    void releaseDrawing();
    
    int getPageIterationCount( const PlotMSPage& page );

    void waitOnCanvases();

    // MEMBERS //
    
    // PlotMS parent.
    PlotMSApp* itsParent_;
    
    // Plot factory for generating plot objects as needed.
    PlotFactoryPtr itsFactory_;
    
    // Parameters.
    PlotMSPlotParameters itsParams_;
    
    // Cache.
    PlotMSCacheBase* itsCache_;
    
    //Used to determine if a thread is running to update the cache.
    volatile bool cacheUpdating;

private:
    void waitOnCanvas( const PlotCanvasPtr& canvas );

    // Disable copy constructor and operator for now.
    // <group>
    PlotMSPlot(const PlotMSPlot& copy);
    PlotMSPlot& operator=(const PlotMSPlot& copy);
    // </group>

    //Adjust the amount of plot data that this plot is holding.
    void resizePlots( int rows, int cols );

    //Return the dimensions of the plot data that this plot should hold.
    void getPlotSize( Int& rows, Int& cols );

    //Returns the iteration for the canvas located at row, r, and column, c.
    int getIterationIndex( int r, int c, const PlotMSPage& page );

    //Post a plot message to the logger.
    void logMessage( const QString& msg ) const;

    void clearCanvasProperties( int row, int col);
    void setCanvasProperties (int row, int col, PMS_PP_Cache*,
    		PMS_PP_Axes* axes, bool set, PMS_PP_Canvas *canv,
    		uInt rows, uInt cols, PMS_PP_Iteration *iter,
    		uInt iteration );

    //Note:  First index for a plot is the dataCount,
    //second index is the number of iteration.
    vector<vector<MaskedScatterPlotPtr> > itsPlots_;

    //Note:  First index for a canvas is the number of rows,
    //second index is the column withen a grid.
    vector<vector<PlotCanvasPtr> > itsCanvases_;

    vector<vector</*QPScatterPlot**/ColoredPlotPtr> > itsColoredPlots_;
    TCLParams itsTCLParams_;
    int gridRow;
    int gridCol;

    Int iter_;
    Int iterStep_;

    static const uInt PIXEL_THRESHOLD;
    static const uInt MEDIUM_THRESHOLD;
    static const uInt LARGE_THRESHOLD;
};

}

#endif /* PLOTMSPLOT_H_ */
