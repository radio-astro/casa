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
#ifndef PLOTMSDATASUMMARYTAB_QO_H_
#define PLOTMSDATASUMMARYTAB_QO_H_

#include <plotms/GuiTabs/PlotMSDataSummaryTab.ui.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <casa/namespace.h>

namespace casa {

class PlotMSDataCollapsible;
class PlotMSPlotTab;

/**
 * Handles adding multiple plots to the GUI.
 */
class PlotMSDataSummaryTab : public PlotMSTab, public PlotMSPlotManagerWatcher {
    Q_OBJECT
    
public:    
    // Constructor which takes the parent tab and plotter.
    PlotMSDataSummaryTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSDataSummaryTab();
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Plot"; }
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the GUI as needed if the given parameters are the current PlotMSPlot's
    // parameters.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
               int updateFlag);

    // Implements PlotMSPlotManagerWatcher::plotsChanged().
    void plotsChanged(const PlotMSPlotManager& manager);

    //Size of the grid (rows & cols).
    void setGridSize( int rowCount, int colCount );

    // Returns the axes that the user has selected to load/release into the cache.
     vector<vector<PMS::Axis> > selectedLoadAxes() const;
     vector<vector<PMS::Axis> > selectedReleaseAxes() const;

     //Tell all of the supported plots to update their displays.
    void plot();

    //Add a plot
    void insertData( int index );

    //Return the currently supported plots.
    vector<PlotMSPlot*> getCurrentPlots();



    //Get the files that the user loaded.
    vector<String> getFiles() const;
    void completePlotting( bool success, PlotMSPlot* plot );
signals:
	void changed( int index );

protected:
	void resizeEvent( QResizeEvent* event );

private slots:
	void singleDataChanged(PlotMSDataCollapsible* collapsible);
	void addSinglePlot( int plotIndex = -1);
	void observeModKeys();
	void close( PlotMSDataCollapsible* collapsible );

private:
	//This was put in to support overplotting.  When two plots are sharing the
	//same canvas, we don't want to trigger a redraw until all the plots sharing
	//the same canvas are done updating their data in background threads.
    void completePlotting( bool success, int plotIndex);
	void emptyLayout();
	void fillLayout();	

    QList<PlotMSDataCollapsible*> dataList;
    QWidget* scrollWidget;
    QSpacerItem* bottomSpacer;
    int rowLimit;
    int colLimit;

    // Flag set if user uses shift+plot or otherwise requests reload&replot
    bool its_force_reload;
    bool makingPlot;
    Ui::PlotMSDataSummaryTabClass ui;
};

}

#endif /* PLOTMSDATASUMMARYTAB_QO_H_ */
