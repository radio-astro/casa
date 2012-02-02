//# PlotMSCacheTab.qo.h: Plot tab for managing the cache.
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
#ifndef PLOTMSCACHETAB_QO_H_
#define PLOTMSCACHETAB_QO_H_

#include <plotms/GuiTabs/PlotMSCacheTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

class PlotMSCacheTab : public PlotMSPlotSubtab, Ui::CacheTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent tab and plotter.
    PlotMSCacheTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSCacheTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Cache"; }
    
    // Implements PlotMSPlotSubtab::getValue().  Does nothing.
    void getValue(PlotMSPlotParameters& params) const { (void)params; }
    
    // Implements PlotMSPlotSubtab::setValue().  Does nothing.
    void setValue(const PlotMSPlotParameters& params) { (void)params; }
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);
    
    
    // See PlotMSPlotTab::selectedLoadOrReleaseAxes().
    vector<PMS::Axis> selectedLoadOrReleaseAxes(bool load) const;
};

}

#endif /* PLOTMSCACHETAB_QO_H_ */
