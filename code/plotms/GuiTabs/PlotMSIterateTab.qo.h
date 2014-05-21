//# PlotMSIterationTab.qo.h: Plot tab for iterated plot settings 
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
#ifndef PLOTMSITERATETAB_QO_H_
#define PLOTMSITERATETAB_QO_H_

#include <plotms/GuiTabs/PlotMSIterateTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class QtLabelWidget;


// Subclass of PlotMSPlotSubtab to manage plot display parameters.
class PlotMSIterateTab : public PlotMSPlotSubtab, Ui::IterateTab {
    Q_OBJECT

    
public:

    // Constructor which takes the parent tab and plotter.
    PlotMSIterateTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSIterateTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Page"; }
    
    // Implements PlotMSPlotSubtab::getValue().
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);

    // Uses the index chooser at the top, with the given number of rows and
    // columns, to manage multi-plot display parameters.
    bool setGridSize(unsigned int nRows,unsigned int nCols);

    //Returns true if a reasonable row and column location has been
    //set (nonzero); false otherwise.
    bool isPlottable() const;

signals:
	void plottableChanged();

private slots:
	//Whether to use a single global axis has changed.
	void globalChanged();
	void locationChanged();

private:
	void hideGridLocation( bool hide );
	void setGridIndices( int rowIndex, int colIndex );


	//Location of the plot
	int gridRow;
	int gridCol;
};
    
    

    




}

#endif /* PLOTMSDISPLAYTAB_QO_H_ */
