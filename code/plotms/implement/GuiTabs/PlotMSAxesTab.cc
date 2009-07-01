//# PlotMSAxesTab.cc: Plot tab for axes parameters.
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
#include <plotms/GuiTabs/PlotMSAxesTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>

namespace casa {

///////////////////////////////
// PLOTMSAXESTAB DEFINITIONS //
///////////////////////////////

PlotMSAxesTab::PlotMSAxesTab(PlotMSPlotter* parent): PlotMSPlotSubtab(parent) {
    setupUi(this);
    
    // Setup axes choices.
    const vector<String>& axes = PMS::axesStrings();
    String xs = PMS::axis(PMS::DEFAULT_XAXIS),
           ys = PMS::axis(PMS::DEFAULT_YAXIS);
    unsigned int xdef = axes.size(), ydef = axes.size();
    for(unsigned int i = 0; i < axes.size(); i++) {
        xChooser->addItem(axes[i].c_str());
        yChooser->addItem(axes[i].c_str());
        if(axes[i] == xs) xdef = i;
        if(axes[i] == ys) ydef = i;
    }
    if(xdef < axes.size()) xChooser->setCurrentIndex(xdef);
    if(ydef < axes.size()) yChooser->setCurrentIndex(ydef);
    
    // Setup data column choices.
    const vector<String>& data = PMS::dataColumnStrings();
    xs = PMS::dataColumn(PMS::DEFAULT_DATACOLUMN);
    xdef = data.size();
    for(unsigned int i = 0; i < data.size(); i++) {
        xDataChooser->addItem(data[i].c_str());
        yDataChooser->addItem(data[i].c_str());
        if(data[i] == xs) xdef = i;
    }
    if(xdef < data.size()) {
        xDataChooser->setCurrentIndex(xdef);
        yDataChooser->setCurrentIndex(ydef);
    }
    
    // Setup range widgets.
    itsXRangeWidget_ = new PlotRangeWidget(true);
    itsYRangeWidget_ = new PlotRangeWidget(true);
    QtUtilities::putInFrame(xRangeFrame, itsXRangeWidget_);
    QtUtilities::putInFrame(yRangeFrame, itsYRangeWidget_);
    
    // Insert label defaults.
    itsLabelDefaults_.insert(xLabel, xLabel->text());
    itsLabelDefaults_.insert(xDataLabel, xDataLabel->text());
    itsLabelDefaults_.insert(xAttachLabel, xAttachLabel->text());
    itsLabelDefaults_.insert(xRangeLabel, xRangeLabel->text());
    itsLabelDefaults_.insert(yLabel, yLabel->text());
    itsLabelDefaults_.insert(yDataLabel, yDataLabel->text());
    itsLabelDefaults_.insert(yAttachLabel, yAttachLabel->text());
    itsLabelDefaults_.insert(yRangeLabel, yRangeLabel->text());
    
    // Connect widgets.
    connect(xChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(xDataChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(xAttachBottom, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsXRangeWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(yChooser,SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(yDataChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(yAttachLeft, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsYRangeWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSAxesTab::~PlotMSAxesTab() { }


void PlotMSAxesTab::getValue(PlotMSPlotParameters& params) const {
    PlotMSSinglePlotParameters* sp =
        dynamic_cast<PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    sp->setXAxis(PMS::axis(xChooser->currentText().toStdString()));
    sp->setXDataColumn(PMS::dataColumn(
                       xDataChooser->currentText().toStdString()));
    sp->setCanvasXAxis(xAttachBottom->isChecked() ? X_BOTTOM : X_TOP);
    sp->setXRange(itsXRangeWidget_->isCustom(), itsXRangeWidget_->getRange());
    
    sp->setYAxis(PMS::axis(yChooser->currentText().toStdString()));
    sp->setYDataColumn(PMS::dataColumn(
                       yDataChooser->currentText().toStdString()));
    sp->setCanvasYAxis(yAttachLeft->isChecked() ? Y_LEFT : Y_RIGHT);
    sp->setYRange(itsYRangeWidget_->isCustom(), itsYRangeWidget_->getRange());
}

void PlotMSAxesTab::setValue(const PlotMSPlotParameters& params) {
    const PlotMSSinglePlotParameters* sp =
        dynamic_cast<const PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    PMS::Axis xAxis = sp->xAxis(), yAxis = sp->yAxis();
    setChooser(xChooser, PMS::axis(xAxis));
    setChooser(xDataChooser, PMS::dataColumn(sp->xDataColumn()));
    if(sp->canvasXAxis() == X_BOTTOM) xAttachBottom->setChecked(true);
    else                              xAttachTop->setChecked(true);
    
    setChooser(yChooser, PMS::axis(yAxis));
    setChooser(yDataChooser, PMS::dataColumn(sp->yDataColumn()));
    if(sp->canvasYAxis() == Y_LEFT) yAttachLeft->setChecked(true);
    else                            yAttachRight->setChecked(true);
}

void PlotMSAxesTab::update(const PlotMSPlot& plot) {
    const PlotMSSinglePlot* p = dynamic_cast<const PlotMSSinglePlot*>(&plot);
    if(p == NULL) return;
    const PlotMSSinglePlotParameters& sp = p->singleParameters();
    
    PlotMSSinglePlotParameters newParams(sp);
    getValue(newParams);
    
    // Update widgets.
    vector<pair<PMS::Axis,unsigned int> > laxes = plot.data().loadedAxes();
    bool found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == newParams.xAxis()) found = true;
    xInCache->setChecked(found);
    xDataFrame->setVisible(PMS::axisIsData(newParams.xAxis()));
    itsXRangeWidget_->setIsDate(
            PMS::axisType(newParams.xAxis()) == PMS::TTIME);
    
    found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == newParams.yAxis()) found = true;
    yInCache->setChecked(found);
    yDataFrame->setVisible(PMS::axisIsData(newParams.yAxis()));
    itsYRangeWidget_->setIsDate(
            PMS::axisType(newParams.yAxis()) == PMS::TTIME);
    
    // Update labels.
    changedText(xLabel, sp.xAxis() != newParams.xAxis() ||
                (xDataFrame->isVisible() &&
                 sp.xDataColumn() != newParams.xDataColumn()));
    changedText(xDataLabel, sp.xDataColumn() != newParams.xDataColumn());
    changedText(xAttachLabel, sp.canvasXAxis() != newParams.canvasXAxis());
    changedText(xRangeLabel, sp.xRangeSet() != newParams.xRangeSet() ||
                (sp.xRangeSet() && sp.xRange() != newParams.xRange()));
    
    changedText(yLabel, sp.yAxis() != newParams.yAxis() ||
                (yDataFrame->isVisible() &&
                 sp.yDataColumn() != newParams.yDataColumn()));
    changedText(yDataLabel, sp.yDataColumn() != newParams.yDataColumn());
    changedText(yAttachLabel, sp.canvasYAxis() != newParams.canvasYAxis());
    changedText(yRangeLabel, sp.yRangeSet() != newParams.yRangeSet() ||
                (sp.yRangeSet() && sp.yRange() != newParams.yRange()));
}

}
