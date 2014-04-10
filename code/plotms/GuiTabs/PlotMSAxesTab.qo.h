//# PlotMSAxesTab.qo.h: Plot tab for axes parameters.
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
#ifndef PLOTMSAXESTAB_QO_H_
#define PLOTMSAXESTAB_QO_H_

#include <plotms/GuiTabs/PlotMSAxesTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMSAxisWidget;


// Subclass of PlotMSPlotSubtab that manages plot axes parameters.
class PlotMSAxesTab : public PlotMSPlotSubtab, Ui::AxesTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent tab and plotter.
    PlotMSAxesTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSAxesTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Axes"; }
    
    // Implements PlotMSPlotSubtab::getValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void update(const PlotMSPlot& plot);
    
signals:
	//Identifier for a y-axis has changed.
	void yAxisIdentifierChanged( int index, QString id );

	//A y-axis has been removed.
	void yAxisIdentifierRemoved( int index );

private slots:
	//Add a new y-axis.
	void addYWidget();

	//Remove a y-axis.
	void removeYWidget();

	//Selected data for a y-axis has changed.
	void axisIdentifierChanged(PlotMSAxisWidget* axisWidget);

	//Display the settings for a new y-axis.
	void yAxisSelected( int index );

private:

	//Hide/show features associated with multipleYAxes.
	void setMultipleAxesYEnabled();

	//Relabel based on the new y-axis index.
	void setYAxisLabel( PlotMSAxisWidget* yWidget, int index );

    // Widgets for the x axis and y axis, respectively.
    PlotMSAxisWidget* itsXWidget_;
    QList<PlotMSAxisWidget*> itsYWidgets_;
};

}

#endif /* PLOTMSAXESTAB_QO_H_ */
