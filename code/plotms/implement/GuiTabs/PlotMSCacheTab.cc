//# PlotMSCacheTab.cc: Plot tab for managing the cache.
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
#include <plotms/GuiTabs/PlotMSCacheTab.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>

#include <QHeaderView>

namespace casa {

////////////////////////////////
// PLOTMSCACHETAB DEFINITIONS //
////////////////////////////////

PlotMSCacheTab::PlotMSCacheTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent) :
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    // Setup widgets.
    metaTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    loadedTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    availableTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    
    // Connect widgets.
    parent->synchronizeAction(PlotMSAction::CACHE_RELEASE, releaseButton);
    parent->synchronizeAction(PlotMSAction::CACHE_LOAD, loadButton);
}

PlotMSCacheTab::~PlotMSCacheTab() { }


void PlotMSCacheTab::update(const PlotMSPlot& plot) {
    metaTable->setRowCount(0);
    loadedTable->setRowCount(0);
    
    static const vector<PMS::Axis>& axes = PMS::axes();
    availableTable->setRowCount(axes.size());
    for(unsigned int i = 0; i < axes.size(); i++) {
        availableTable->setItem(i, 0, new QTableWidgetItem(PMS::axis(
                                axes[i]).c_str()));
        availableTable->setItem(i, 1, new QTableWidgetItem(PMS::axisType(
                                PMS::axisType(axes[i])).c_str()));
    }
    availableTable->sortItems(0);
    availableTable->resizeRowsToContents();
    
    vector<pair<PMS::Axis, unsigned int> > laxes = plot.data().loadedAxes();
    unsigned int nmeta = 0, nloaded = 0;
    for(unsigned int i = 0; i < laxes.size(); i++) {
        if(PlotMSCache::axisIsMetaData(laxes[i].first)) nmeta++;
        else                                            nloaded++;
    }
    metaTable->setRowCount(nmeta);
    loadedTable->setRowCount(nloaded);
    
    QTableWidget* tw; unsigned int ti; QTableWidgetItem* twi;
    QString qaxis;
    unsigned int imeta = 0, iloaded = 0;
    for(unsigned int i = 0; i < laxes.size(); i++) {
        if(PlotMSCache::axisIsMetaData(laxes[i].first)) {
            tw = metaTable;   ti = imeta;   imeta++;
        } else {
            tw = loadedTable; ti = iloaded; iloaded++;
        }
        
        qaxis = PMS::axis(laxes[i].first).c_str();
        
        // move from available table
        for(int r = 0; r < availableTable->rowCount(); r++) {
            twi = availableTable->item(r, 0);
            if(twi->text() == qaxis) {
                availableTable->takeItem(r, 0);
                tw->setItem(ti, 0, twi);
                
                twi = new QTableWidgetItem(QString("%1").arg(laxes[i].second));
                tw->setItem(ti, 1, twi);
                
                twi = availableTable->takeItem(r, 1);
                tw->setItem(ti, 2, twi);
                
                availableTable->removeRow(r);
                break;
            }
        }
    }
    
    if(nmeta > 0) {
        metaTable->sortItems(0);
        metaTable->resizeRowsToContents();
    }
    if(nloaded > 0) {
        loadedTable->sortItems(0);
        loadedTable->resizeRowsToContents();
    }
}

vector<PMS::Axis> PlotMSCacheTab::selectedLoadOrReleaseAxes(bool load) const {
    QTableWidget* tw = load ? availableTable : loadedTable;
    QList<QTableWidgetSelectionRange> sel = tw->selectedRanges();
    vector<PMS::Axis> axes;
    vector<int> removeRows;
    
    PMS::Axis axis;
    bool ok;
    for(int i = 0; i < sel.size(); i++) {
        for(int r = sel[i].bottomRow(); r <= sel[i].topRow(); r++) {
            axis = PMS::axis(tw->item(r, 0)->text().toStdString(), &ok);
            if(ok && !PlotMSCache::axisIsMetaData(axis)) axes.push_back(axis);
        }
    }
    return axes;
}

}
