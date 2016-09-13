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
#ifndef PLOTMSDATACOLLAPSIBLE_QO_H
#define PLOTMSDATACOLLAPSIBLE_QO_H

#include <QtGui/QWidget>
#include <casa/BasicSL/String.h>
#include <plotms/GuiTabs/PlotMSDataCollapsible.ui.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/PlotMS/PlotMS.h>

class QLineEdit;

namespace casa {

class PlotMSPlotTab;
class PlotMSDataTab;
class PlotMSPlotter;

/**
 * Represents a single plot's properties that can be shown maximized or minimized;
 * handles whether a plot will be shown on the display.
 */

class PlotMSDataCollapsible : public QWidget{
    Q_OBJECT

public:
    PlotMSDataCollapsible(PlotMSPlotter* plotter, QWidget* parent, int plotIndex = -1);
    ~PlotMSDataCollapsible();

    //Return the plot tab.
    PlotMSDataTab* getSingleData();

    //Set the numbers of rows and cols that the page can currently
    //support.
    void setGridSize( int rowCount, int colCount );

    //Return the row and column location for this plot.
    void getLocation( Int& rowIndex, Int& colIndex );

    //Plot parameters have changed.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
                   int updateFlag);

    //Return the minimum size for the plot properties.
    virtual QSize minimumSizeHint() const;

    //Return the size.
    virtual QSize sizeHint() const;

    //Reset the height based on the positive or negative amount of new space that
    //is available.
    void resetHeight( int diff );

    //The plots being displayed have changed.
    void plotsChanged(const PlotMSPlotManager& manager, int index);

    //Returns the plot
    PlotMSPlot* getPlot();

    //Force (or not) the plots to update their displays.
    bool plot( bool forceReload );

    //Returns whether or not the plot is displayed by this Collapsible.
    bool managesPlot(PlotMSPlot* plot ) const;

    vector<PMS::Axis> getSelectedLoadAxes() const;
    vector<PMS::Axis> getSelectedReleaseAxes() const;

    //Returns whether this plot will be plotted, i.e.,
    //its gridRow and gridCol are nonnegative AND there is an empty location
    //for the plot, either at its preferred gridRow and gridCol or at an empty
    //location somewhere else.
    bool isPlottable() const;

    //Change to a compact display
    void minimizeDisplay();

    //Returns whether or not the display is compact (minimized) or expanded.
    bool isMinimized() const;

    //This was put in to support overplotting.  When two plots are sharing the
    //same canvas, we don't want to trigger a redraw until all the plots sharing
    //the same canvas are done updating their data in background threads.
    void completePlotting( bool success );
    void clearData();

    //Return the ms that was loaded or an empty string.
    String getFile() const;

signals:
	void close( PlotMSDataCollapsible*);

private slots:
	void closePlot();
	void openCloseDisplay();
	void showContextMenu( const QPoint& location );

private:

	void maximizeDisplay();

    QAction minimizeAction;
    QAction maximizeAction;
    QAction closeAction;
    const int SIZE_COLLAPSED;
    const int SIZE_EXPANDED;
    const int SIZE_WIDTH;
    int minimumSize;

    QLineEdit* nameLabel;
    PlotMSPlotTab* plotTab;
    Ui::PlotMSDataCollapsibleClass ui;
};
}

#endif // PLOTMSDATACOLLAPSIBLE_QO_H
