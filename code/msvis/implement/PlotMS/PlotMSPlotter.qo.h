//# PlotMSPlotter.qo.h: GUI for plotms.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#ifndef PLOTMSPLOTTER_QO_H_
#define PLOTMSPLOTTER_QO_H_

#include <msvis/PlotMS/PlotMSPlotter.ui.h>

#include <graphics/GenericPlotter/PlotFactory.h>
#include <msvis/PlotMS/PlotMSData.h>
#include <msvis/PlotMS/PlotMSLogger.h>
#include <msvis/PlotMS/PlotMSOptionsDock.qo.h>
#include <msvis/PlotMS/PlotMSToolsDock.qo.h>

#include <QtGui>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMS;

// Extremely simple class to hold the four coordinates of a rectangle, along
// with the indices of the canvas on which is resides (unnecessary for
// single-canvas use).
class PlotMSRect {
public:
    PlotMSRect(double xl, double yt, double xr, double yb, unsigned int cr = 0,
            unsigned int cc = 0) : xLeft(xl), yTop(yt), xRight(xr),
            yBottom(yb), canvasRow(cr), canvasColumn(cc) { }
    
    ~PlotMSRect() { }
    
    double xLeft;
    double yTop;
    double xRight;
    double yBottom;
    
    unsigned int canvasRow;
    unsigned int canvasColumn;
};


// Handler interface for the five callback buttons: flag, unflag, locate,
// next, and previous.
class PlotMSHandler {
public:
    PlotMSHandler() { }
    
    virtual ~PlotMSHandler() { }

    // Callback for when the user interactively changes the plot options and
    // pushes "apply".
    virtual void handleOptionsChanged(const PlotMSOptions& newOptions) { }
    
    // Callback for when the user changes the log level.
    virtual void handleLogLevelChanged(PlotMSLogger::Level newLevel) { }
    
    // Callback for when the "Flag" button is clicked.  rects holds all
    // selected regions (see PlotMSRect class), and applyToAll indicates
    // whether the flagging should be applied to all iteration "slices" or
    // just the current one (irrelevant to non-iteration cases).
    virtual void handleFlag(vector<PlotMSRect> rects, bool applyToAll) { }
    
    // Callback for when the "Unglag" button is clicked.  rects holds all
    // selected regions (see PlotMSRect class), and applyToAll indicates
    // whether the unflagging should be applied to all iteration "slices" or
    // just the current one (irrelevant to non-iteration cases).
    virtual void handleUnflag(vector<PlotMSRect> rects, bool applyToAll) { }
    
    // Callback for when the "Locate" button is clicked.  rects holds all
    // selected regions (see PlotMSRect class), and applyToAll indicates
    // whether the locate should be applied to all iteration "slices" or
    // just the current one (irrelevant to non-iteration cases).
    virtual void handleLocate(vector<PlotMSRect> rects, bool applyToAll) { }
    
    // Callback for when the "Next" button is clicked.  Irrelevant to
    // non-iteration cases.
    virtual void handleNext() { }

    // Callback for when the "Prev" button is clicked.  Irrelevant to
    // non-iteration cases.
    virtual void handlePrev() { }
};

// High(ish)-level plotter class that manages the GenericPlotter hierarchy
// transparently to the user.  Designed specifically for use by the new plotms.
class PlotMSPlotter : public QMainWindow, Ui::PlotterWindow {
    Q_OBJECT
    
public:
    // Static //
    
    // Returns "about" text for the given implementation.
    static String aboutText(Plotter::Implementation implementation,
                            bool useHTML = true);
    
    
    // Non-Static //
    
    // Create a plotter with a single canvas using the given implementation
    // with the given parent.
    PlotMSPlotter(PlotMS* parent,
                  Plotter::Implementation impl = Plotter::DEFAULT);
    
    // Create a plotter with a grid of canvases with the given rows and
    // columns using the given implementation with the given parent.
    PlotMSPlotter(unsigned int nrows, unsigned int ncols, PlotMS* parent,
                  Plotter::Implementation impl = Plotter::DEFAULT);
    
    // Destructor.
    ~PlotMSPlotter();
    
    
    // Accessor Methods //
    
    // Returns the factory used for object generation.
    PlotFactoryPtr getFactory() { return itsFactory_; }
    
    // Returns the Plotter.
    PlotterPtr getPlotter() { return itsPlotter_; }
    
    // Returns all canvases in the plotter.
    vector<PlotCanvasPtr> getCanvases() { return itsCanvases_; }
    
    // Returns canvas at the given row and col.
    PlotCanvasPtr getCanvas(unsigned int row = 0, unsigned int col = 0);
    
    // Returns the line used to draw future plots, shapes, etc.
    PlotLinePtr getLine() { return itsLine_; }
    
    // Returns the symbol used to draw future plots, points, etc.
    PlotSymbolPtr getSymbol() { return itsSymbol_; }
    
    
    // Execution Methods //
    
    // Enter the plotter's execution loop, and return the return value.
    // Only during this execution loops will GUI windows be shown and
    // GUI events be handled.  The execution loop ends when the user clicks
    // the "close" or "quit" buttons.  This method can be called multiple
    // times.
    int execLoop();
    
    
    // Button/Handler Methods //
    
    // Register the given handler.
    void registerHandler(PlotMSHandler* handler);
    
    // Show or hide the "iteration" widgets: next (button), previous (button),
    // and "all" (checkbox).
    void showIterationButtons(bool show);
    
    
    // Plotter Customization Methods //
    
    // Gets the number of rows/columns of canvases.
    // <group>
    unsigned int getNRows() const;
    unsigned int getNCols() const;
    // </group>
    
    // Sets the grid of canvases to have the given number of rows and columns.
    // If GREATER than the current number of rows/columns, the existing
    // canvases are preserved.
    void setGridDimensions(unsigned int nrows, unsigned int ncols);
    
    // Updates the docks/tools/actions/whatever to reflect the given options.
    // Does NOT resize the plotter.
    void updateOptions(const PlotMSOptions& opts);
    
    // Sets the spacing of the grid to the given.  Does nothing for a single
    // canvas.
    void setGridSpacing(unsigned int spacing);
    
    // Sets the plotter window title.
    void setWindowTitle(const String& windowTitle);
    
    // Sets the status bar text to the given.
    void setStatusText(const String& statusText);
    
    // Clears the status bar text.
    void clearStatusText() { setStatusText(""); }
    
    // Sets the text of the title for the canvas at the given row and column.
    void setCanvasTitle(String canvasTitle, unsigned int row = 0,
                        unsigned int col = 0);
    
    // Sets the text of the label for the x-bottom and y-left axes on the
    // canvas at the given row and column.
    void setAxesLabels(String xLabel, String yLabel, unsigned int row = 0,
                       unsigned int col = 0);
    
    // Methods for getting and setting the axis scales for the canvas at the
    // given row and column.
    // <group>
    PlotAxisScale getXAxisType(unsigned int row = 0, unsigned int col = 0);
    
    PlotAxisScale getYAxisType(unsigned int row = 0, unsigned int col = 0);
    
    void setXAxisType(PlotAxisScale type, unsigned int row = 0,
                      unsigned int col = 0);
    
    void setYAxisType(PlotAxisScale type, unsigned int row = 0,
                      unsigned int col = 0);
    
    void setAxesTypes(PlotAxisScale xType, PlotAxisScale yType,
                      unsigned int row = 0, unsigned int col = 0) {
        setXAxisType(xType, row, col);
        setXAxisType(yType, row, col);
    }
    
    void setAxesTypes(PlotAxisScale types, unsigned int row = 0,
                      unsigned int col = 0) {
        setXAxisType(types, row, col);
        setYAxisType(types, row, col);
    }
    // </group>
    
    // Turns the major/minor grids on/off for the canvas at the given position.
    void setGrid(bool xMaj, bool xMin, bool yMaj, bool yMin,
                 unsigned int row = 0, unsigned int col = 0);
    
    // Shows/hides the axes at the given canvas.
    void showAxes(bool showX, bool showY, unsigned int row = 0,
                  unsigned int col = 0);
    
    // Shows/hides the legend at the given position for the canvas at the
    // given row and column (or the first canvas if they are -1)
    void showLegend(bool show = true, PlotCanvas::LegendPosition position =
                    PlotCanvas::INT_LRIGHT, unsigned int row = 0,
                    unsigned int col = 0);
    
    
    // Plot Customization Methods //
    
    // Show/hide the lines for future plots, shapes, etc.
    void showLines(bool showLines = true);
    
    // Show/hide the symbols for future plots, points, etc.
    void showSymbols(bool showSymbols = true);
    
    // Set the line for future plots, shapes, etc. to the given properties.
    PlotLinePtr setLine(String color, PlotLine::Style style = PlotLine::SOLID,
                        double width = 1.0);
    
    // Set the line for future plots, shapes, etc. to the given.
    void setLine(PlotLinePtr line);
    
    // Set the symbol for future plots, points, etc. to the given properties.
    PlotSymbolPtr setSymbol(PlotSymbol::Symbol symbol, String color = "blue",
                            double size = 5, bool outline = true);

    // Set the symbol for future plots, points, etc. to the given.
    void setSymbol(PlotSymbolPtr symbol);
    
    // Set the area fill for future shapes, etc. to the given properties.
    PlotAreaFillPtr setAreaFill(String color,
                           PlotAreaFill::Pattern pattern = PlotAreaFill::FILL);
    
    // Set the area fill for future shapes, etc. to the given.
    void setAreaFill(PlotAreaFillPtr areaFill);
    
    
    // Plotting Methods //
    
    // Plot the given data on the canvas at the given row and column.
    // Note: this will eventually replace the other plot methods.
    ScatterPlotPtr plotData(PlotMSDatas& data, String color = "",
                            String title= "Unnamed Plot", bool overplot= false,
                            unsigned int row = 0,unsigned int col = 0) {
        return plotxy(PlotPointDataPtr(&data.getData(row, col), false), color,
                      title, overplot, row, col);
    }
    
    // Plot the given vectors on the canvas at the given row and column.
    // Note: the referenced Vectors must remain valid for the lifetime of
    // the plotter.
    // <group>
    ScatterPlotPtr plotxy(Vector<double>& x, Vector<double>& y,
                          String color = "", String title = "Unnamed Plot",
                          bool overplot = false, unsigned int row = 0,
                          unsigned int col = 0) {
        return plotxy(itsFactory_->data(x, y),color,title,overplot,row,col); }
    ScatterPlotPtr plotxy(Vector<float>& x, Vector<float>& y,
                          String color = "", String title = "Unnamed Plot",
                          bool overplot = false, unsigned int row = 0,
                          unsigned int col = 0) {
        return plotxy(itsFactory_->data(x, y),color,title,overplot,row,col); }
    ScatterPlotPtr plotxy(Vector<int>& x, Vector<int>& y,
                          String color = "", String title = "Unnamed Plot",
                          bool overplot = false, unsigned int row = 0,
                          unsigned int col = 0) {
        return plotxy(itsFactory_->data(x, y),color,title,overplot,row,col); }
    // </group>
    
    // Plot the given data points on the canvas at the given row and column.
    ScatterPlotPtr plotxy(PlotPointDataPtr data, String color="",
                          String title = "Unnamed Plot", bool overplot = false,
                          unsigned int canvRow = 0, unsigned int canvCol = 0);
    
    
    // Shapes/Annotation/etc. Methods //
    
    // Draw an annotation at the given point with the given text on the canvas
    // at the given row and column.
    PlotAnnotationPtr annotation(double x, double y, String text,
                                 bool useMainLayer = false,
                                 unsigned int canvasRow = 0,
                                 unsigned int canvasCol = 0);
    
    // Draw an annotation at the given points on the canvas at the given row
    // and column.
    PlotShapeRectanglePtr rectangle(double left, double top,
                                    double right, double bottom,
                                    bool useMainLayer = false,
                                    unsigned int canvasRow = 0,
                                    unsigned int canvasCol = 0);
    
    
    // Clearing Methods //
    
    // Clear all plot items from the canvas at the given row and column.
    void clear(unsigned int canvasRow = 0, unsigned int canvasCol = 0);
    
    // Clear all plot items from all canvases.
    void clearAll();
    
public slots:
    // Shows the given error/warning message in a GUI window.
    void showError(const String& message, const String& title, bool isWarning);
    
    // Shows the given informational message in a GUI window.
    void showMessage(const String& message, const String& title);
    
protected:
    // Overrides the close event in case we're dealing with a plotter that
    // isn't Qt and thus is in its own window (and possibly its own execution
    // loop).
    void closeEvent(QCloseEvent* event);
    
private:
    bool isQwt_;
    PlotMSOptionsDock* itsOptionsDock_;
    PlotMSToolsDock* itsToolsDock_;
    
    PlotFactoryPtr itsFactory_;
    PlotterPtr itsPlotter_;
    
    unsigned int nRows_, nCols_;
    vector<PlotCanvasPtr> itsCanvases_;
    
    PlotLinePtr itsLine_;
    PlotSymbolPtr itsSymbol_;
    PlotAreaFillPtr itsAreaFill_;
    
    vector<PlotMSHandler*> itsHandlers_;
    
    PlotMSLogger& itsLogger_;
    PlotMSLogger::Level& itsLogLevel_;
    
    // Initialize the plotter with the given rows and columns.  Called from
    // constructors.
    void init(unsigned int rs, unsigned int cs, Plotter::Implementation imp);
    
    void action(QAction* which);
    
    void updateMouseSelectTools();
    
private slots:
    void showHideOptionsDock();
    void optionsDockShownOrHidden(bool visible);    
    void optionsChanged(const PlotMSOptions& newOptions);
    
    void showHideToolsDock();
    void toolsDockShownOrHidden(bool visible);
    void mouseToolChanged(PlotStandardMouseToolGroup::Tool newTool);
    void trackerTurned(bool hover, bool display);
    void exportRequested(const PlotExportFormat& format, int row, int col);
    void logLevelChanged(PlotMSLogger::Level level);
    
    void actionMark_() { action(actionMarkRegions); }
    void actionClear_() { action(actionClearRegions); }
    void actionFlag_() { action(actionFlag); }
    void actionUnflag_() { action(actionUnflag); }
    void actionLocate_() { action(actionLocate); }
    void actionNext_() { action(actionNextIter); }
    void actionPrev_() { action(actionPrevIter); }
    
    void holdReleaseDrawing(bool toggled);
    
    void showAbout();
};

typedef CountedPtr<PlotMSPlotter> PlotMSPlotterPtr;

}

#endif /* PLOTMSPLOTTER_QO_H_ */
