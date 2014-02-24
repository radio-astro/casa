//# PlotMSDisplayTab.qo.h: Plot tab to manage plot display parameters.
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
#ifndef PLOTMSDISPLAYTAB_QO_H_
#define PLOTMSDISPLAYTAB_QO_H_

#include <plotms/GuiTabs/PlotMSDisplayTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class PlotMSDataSymbolWidget;


// Subclass of PlotMSPlotSubtab to manage plot display parameters.
class PlotMSDisplayTab : public PlotMSPlotSubtab, Ui::DisplayTab {
    Q_OBJECT
    
    
public:

    // Constructor which takes the parent tab and plotter.
    PlotMSDisplayTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSDisplayTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Display"; }
    
    // Implements PlotMSPlotSubtab::getValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().  WARNING: for now, only works
    // with PlotMSSinglePlotParameters.
    void update(const PlotMSPlot& plot);

    //Support for overplotting with multiple y-axes.
    void setAxisIdentifier( int index, QString id );
    void removeAxisIdentifier( int index );
 
private slots:
	void updateText( QLabel* source, bool highlight );
	void changeSymbolViewed();

private:
	void makeDataSymbol();
	void updateMultipleAxisSupport();
    
    QList<PlotMSDataSymbolWidget*> dataSymbols;
    
    // Display parameters.
    PMS_PP_Display itsPDisplay_;
    

};

}

#endif /* PLOTMSDISPLAYTAB_QO_H_ */
