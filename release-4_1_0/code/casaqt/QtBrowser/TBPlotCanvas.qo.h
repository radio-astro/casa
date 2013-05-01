//# TBPlotCanvas.qo.h: Canvas for data plotting using qwt.
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
#ifndef TBPLOTCANVAS_H_
#define TBPLOTCANVAS_H_

#include <casaqt/QtBrowser/TBPlotCanvas.ui.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <graphics/GenericPlotter/PlotOptions.h>
#include <graphics/GenericPlotter/PlotEventHandler.h>

#include <graphics/GenericPlotter/PlotFactory.h>

#include <casa/BasicSL/String.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBPlotData;
class TBLocateParams;

// <summary>
// Format for the curves on the plot canvas.
// </summary>
//
// <synopsis>
// A TBPlotFormat specifies how to display a plot curve.  The format includes
// a PlotLine and a PlotSymbol.
// </synopsis>

class TBPlotFormat {
public:
    // Constructor that takes a factory to build the line and symbol.
    TBPlotFormat(PlotFactoryPtr factory);

    ~TBPlotFormat();

    
    // Sets the curve style using the given QString name.
    void setCurveStyle(QString str);

    // Sets the point style using the given QString name.
    void setPointStyle(QString str);

    // Line to use with the plot
    PlotLinePtr line;
    
    // Symbol to use with the plot
    PlotSymbolPtr symbol;
    
    /*
    // Returns a QwtSymbol using the format's point style, size, and color.
    QwtSymbol getSymbol();

    // Curve style.
    QwtPlotCurve::CurveStyle curveStyle;

    // Point style.
    QwtSymbol::Style pointStyle;

    // Point color.
    QColor color;

    // Point size.
    // <group>
    int size1;
    int size2;
    // </group>
    */
};


// <summary>
// Canvas for data plotting using a given plotting implementation.
// </summary>
//
// <synopsis>
// TBPlotCanvas uses and controls a plotting implementation canvas.  If the
// implementation is Qt, it is embedded in the TBPlotCanvas; otherwise it is
// assumed to be in a separate window.
// </synopsis>

class TBPlotCanvas : public QWidget, Ui::PlotCanvas,
                     public PlotSelectEventHandler {
    Q_OBJECT
    
public:
    // Constructor with a factory.
    TBPlotCanvas(PlotFactoryPtr factory);

    ~TBPlotCanvas();

    
    // Sets the X-axis title to the given String.
    void setXAxisTitle(String title);

    // Sets the Y-axis title to the given String.
    void setYAxisTitle(String title);
    
    // Sets which grids are shown on the canvas.
    void setShownGrids(bool xMaj, bool xMin, bool yMaj, bool yMin);
    
    // Sets the x axis to display values as dates.
    void setXAxisDate(bool date = true, bool mjsec = true);
    
    // Sets the y axis to display values as dates.
    void setYAxisDate(bool date = true, bool mjsec = true);

    // Returns the name of the current table being plotted.
    String getCurrentTable();

    // Sets the name of the current table being plotted.
    void setTable(String table);
    
    // Returns the current number of plots currently on the canvas.
    int getNumPlots();
    
    
    // Returns the data corresponding to the current plots.
    vector<TBPlotData*> allData();
    
    // Implements PlotSelectEventHandler::handleSelect().
    void handleSelect(const PlotSelectEvent& event);    
    
    // Plots the given data.  If overplot is true all old plots
    // are kept as well, otherwise the old plots are cleared first.
    // See PlotCanvas::plot().
    void plot(TBPlotData* data, const TBPlotFormat& format,
              bool overplot = false);
    
    // Exports the PlotCanvas to an image in the given format at the given
    // location and returns whether the operation succeeded or not.
    Result exportToImage(String format, String location);
    
    // If there is a rectangle shape from a user-selected region, clear it
    // from the canvas.
    void clearSelectedRectangle();
    
    // If there is a current user-selected region, return it.  Otherwise
    // behavior is undefined.
    PlotRegion currentSelection();

public slots:
    // Clears all current plots and hides the two axes.
    void clearAndHideAxes();
    
signals:
    // Signal that is emitted whenever the user selects a region, or the
    // currently selected region is cleared.  If selected is true, then the
    // user selected a region and a rectangle is now on the canvas.  If
    // selected is false, then there is no longer a current selected region.
    void regionSelected(bool selected);

private:
    // Plotter factory
    PlotFactoryPtr factory;
    
    // Plotter
    PlotterPtr plotter;
    
    // Plot canvas
    PlotCanvasPtr canvas;
    
    // Holds the name of the current table being plotted.
    String currentTable;
    
    // Currently selected region, or NULL if there is none.
    PlotShapeRectanglePtr selectedRegion;
    
    // Current data.
    vector<TBPlotData*> data;
    
private slots:    
    // Slot for clearing all the internal tracking of current plots.
    void clearPlots();
};

}

#endif /* TBPLOTCANVAS_H_ */
