//# TBPlotter.qo.h: Widget to collect plot parameters and plot on the canvas.
//# Copyright (C) 2005
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
#ifndef TBPLOTTER_H_
#define TBPLOTTER_H_

#include <casaqt/QtBrowser/TBPlotter.ui.h>
#include <casaqt/QtBrowser/TBPlotCanvas.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <graphics/GenericPlotter/PlotFactory.h>

#include <QtGui>

#include <map>
#include <vector>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBBrowser;
class TBPlotCanvas;
class QProgressPanel;
class TBTableTabs;

// <summary>
// Parameters for a single field for collecting plotting data.
// </summary>
//
// <synopsis>
// A PlotParams accompanies each axis when plot data is being collected.
// A PlotParams indicates which field is being plotted and other important
// parameters.
// </synopsis>

class PlotParams {
public:
    // Default Constructor
    PlotParams(): rowNumbers(false), complex(false), complexAmp(false),
                  colIndex(0) { }

    
    // Indicates whether this plot axis is only the row numbers (true) or an
    // actual field (false).
    bool rowNumbers;

    // Indicates whether this field is complex or not.
    bool complex;

    // If the field is complex, indicates whether the amplitude (true) or the
    // phase (false) is used.
    bool complexAmp;

    // Indicates which field index is to be used.
    unsigned int colIndex;

    // If the field is an array, indicates the array slice to be used.
    vector<int> slice;
};

// <summary>
// Data for plotting.
// </summary>
//
// <synopsis>
// A TBPlotData holds two double arrays and the number of points.
// </synopsis>

class TBPlotData {
public:
    // Default Constructor.
    TBPlotData() { }
    
    // Constructor which takes the number of points and the two arrays.
    TBPlotData(PlotPointDataPtr d) : data(d) { }
    
    ~TBPlotData() { }
    
    // Actual data.
    PlotPointDataPtr data;
    
    // Table this data is from.
    TBTableTabs* table;
    
    // Row numbers that correspond to the given data.  In the future may want
    // to replace with a less memory-heavy implementation.
    vector<int> rows;
    
    // Title for the plot; usually xAxisName vs. yAxisName
    String title;
};

// <summary>
// Small widget to provide a variable number of spinners to create a slice.
// </summary>
//
// <synopsis>
// A PlotSlicer provides a variable number of QSpinBoxes to allow the user
// to enter an array slice.  The number of spinners depends on the
// dimensionality (i.e., a two-dimensional array will have two spinners).  The
// range of the spinners depends on the array's shape (i.e., a 4x2 array will
// have a [0, 3] range on the first spinner and a [0, 1] range on the second
// spinner).  A PlotSlicer can also display a QComboBox to allow the user to
// choose between phase and amplitude for complex numbers.
// </synopsis>

class PlotSlicer : public QHBoxLayout {
    Q_OBJECT
    
public:
    // Default Constructor.
    PlotSlicer();

    ~PlotSlicer();

    
    // Sets the dimensions of the slicer to the given vector.  If complex is
    // true, a combobox to choose between phase and amplitude is also shown.
    bool setDimension(vector<int>* d, bool complex = false);

    // Retrieves the array slice into the given vector.  complex is set to true
    // if the slice is for complex numbers; if complex is true, amp indicates
    // whether the slice is for the amplitude (true) or the phase (false).
    void getDimension(vector<int>& d, bool& complex, bool& amp);

private:
    // All current spinners.
    vector<QSpinBox*> spinners;

    // Complex chooser.
    QComboBox* complexChooser;

    // Whether the current slice is for a complex or not.
    bool complex;
};

// <summary>
// Widget to collect plot parameters and plot on the canvas.
// </summary>
//
// <synopsis>
// A TBPlotter consists of a TBPlotCanvas and other widgets to control plot
// parameters and other options.  The TBPlotter has four sections in a vertical
// layout.  The first section is the TBPlotCanvas.  The second is the data
// parameters: which table and rows to use, which fields to plot, etc.  The
// third is the graph format: scatter vs. line, point formatting, etc.  The
// fourth are plotting tools such as saving as an image.
// </synopsis>

class TBPlotter : public QMainWindow, public Ui::Plotter {
    Q_OBJECT
    
public:
    // Constructor that takes the parent browser.
    TBPlotter(TBBrowser* browser, PlotFactoryPtr factory);

    ~TBPlotter();

    
    // Adds a QProgressPanel to the plotter with the given parameters and
    // returns it.
    QProgressPanel* addProgressPanel(String label, bool hideable,
                                     bool cancelable);

    // Removes the given QProgressPanel from the plotter.
    void removeProgressPanel(QProgressPanel* panel);
    
protected:
    // Capture when the window closes.  If the parameters dock widget is
    // floating, close it manually.
    void closeEvent(QCloseEvent* event);

private:
    // Parent browser.
    TBBrowser* browser;
    
    // Plotter factory
    PlotFactoryPtr factory;

    // For each table the plotter knows about, the dimensions of the fields are
    // kept for fast access.  So, for example, the dimensions of a field can be
    // found with dimensions[tableName][columnIndex].
    map<String, vector<vector<int>*> > dimensions;

    // Since only certain field types are plottable, adjustedIndices allows for
    // translation between the index of the combobox (which contains only
    // plottable fields) and the index in the table (which contains all
    // fields).  So, for example, adjustedIndices[plottableIndex] = tableIndex.
    vector<int> adjustedIndices;

    // PlotSlicer for the x-axis.
    PlotSlicer xSlice;

    // PlotSlicer for the y-axis.
    PlotSlicer ySlice;

    // Types for the displayed plottable fields.
    vector<String> types;

    // Is true if the current selection for the x-axis is valid, false
    // otherwise.  If the axis is invalid, it cannot be used for plotting.
    bool xValid;

    // Is true if the current selection for the x-axis is valid, false
    // otherwise.  If the axis is invalid, it cannot be used for plotting.
    bool yValid;

    // Flag to indicate whether GUI-generated events are "genuine."
    bool update;

    // Plot canvas.
    TBPlotCanvas* plotCanvas;

    
    // Collects the parameters and plots as indicated.
    void doPlot(bool overplot = true);
    
private slots:
    // Slot for when the user chooses a different table from the combobox.
    // Updates the displayed options for the table.
    void tableChosen(QString name);

    // Slot for when the user chooses a field for the x-axis.
    void xChosen(int x);

    // Slot for when the user chooses a field for the y-axis.
    void yChosen(int y);
    
    // Slot for code common to xChosen and yChosen.
    void chosen(bool x, int i);

    // Slot for the "Clear and Plot" button.  See doPlot(false);
    void plot();

    // Slot for the "Overplot" button.  See doPlot(true);
    void overplot();

    // Opens a new TBPlotter window.
    void openNewPlotter();

    // Slot for when a table is opened in the browser.  Adds the table name
    // to the list of opened tables.
    void tableOpened(String table);

    // Slot for when a table is closed in the browser.  Removes the table
    // name from the list of opened tables.
    void tableClosed(String table);

    // Slot for the "Clear" button.  See TBPlotCanvas::clearAndHideAxes().
    void clear();

    // Slot for the "All Rows" button.  Sets the row selection to be all the
    // rows in the currently selected table.
    void allRows();

    // Slot to open a QColorDialog for the given line edit.
    void setColor(QLineEdit* lineEdit);

    // Convenience slot
    void setLineColor() { setColor(lineColorEdit); }
    
    // Convenience slot
    void setSymbolColor() { setColor(symbolColorEdit); }
    
    // Slot to export the current plot canvas to the image format specified
    // by the QComboBox.
    void exportImage();
    
    // Slot for when the user enters a filter rule for the table at the given
    // index.
    void filterRuleEntered(int i);
    
    // Slot for when the user enters a filter rule for the table at the given
    // index.
    void filterRuleCleared(int i);
    
    // Slot for when the user changes the grid options.
    void gridChanged();
    
    void regionSelected(bool selected);
    
    // Slot for when the user clicks "locate".
    void selectLocate();
    
    // Slot for when the user clears the currently selected region.
    void clearSelection();
};

}

#endif /* TBPLOTTER_H_ */
