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
#include <plotms/Gui/PlotRangeWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

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
    PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    PMS_PP_Canvas* cv = params.typedGroup<PMS_PP_Canvas>();
    if(c == NULL) {
        params.setGroup<PMS_PP_Cache>();
        c = params.typedGroup<PMS_PP_Cache>();
    }
    if(cv == NULL) {
        params.setGroup<PMS_PP_Canvas>();
        cv = params.typedGroup<PMS_PP_Canvas>();
    }
    
    c->setXAxis(PMS::axis(xChooser->currentText().toStdString()),
                PMS::dataColumn(xDataChooser->currentText().toStdString()));
    cv->setXAxis(xAttachBottom->isChecked() ? X_BOTTOM : X_TOP);
    cv->setXRange(itsXRangeWidget_->isCustom(), itsXRangeWidget_->getRange());
    
    c->setYAxis(PMS::axis(yChooser->currentText().toStdString()),
                PMS::dataColumn(yDataChooser->currentText().toStdString()));
    cv->setYAxis(yAttachLeft->isChecked() ? Y_LEFT : Y_RIGHT);
    cv->setYRange(itsYRangeWidget_->isCustom(), itsYRangeWidget_->getRange());
}

void PlotMSAxesTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Canvas* cv = params.typedGroup<PMS_PP_Canvas>();
    if(c == NULL || cv == NULL) return; // shouldn't happen
    
    PMS::Axis xAxis = c->xAxis(), yAxis = c->yAxis();
    setChooser(xChooser, PMS::axis(xAxis));
    setChooser(xDataChooser, PMS::dataColumn(c->xDataColumn()));
    if(cv->xAxis() == X_BOTTOM) xAttachBottom->setChecked(true);
    else                        xAttachTop->setChecked(true);
    itsXRangeWidget_->setRange(PMS::axisType(xAxis) == PMS::TTIME,
            cv->xRangeSet(), cv->xRange().first, cv->xRange().second);
    
    setChooser(yChooser, PMS::axis(yAxis));
    setChooser(yDataChooser, PMS::dataColumn(c->yDataColumn()));
    if(cv->yAxis() == Y_LEFT) yAttachLeft->setChecked(true);
    else                      yAttachRight->setChecked(true);
    itsYRangeWidget_->setRange(PMS::axisType(yAxis) == PMS::TTIME,
            cv->yRangeSet(), cv->yRange().first, cv->yRange().second);
}

void PlotMSAxesTab::update(const PlotMSPlot& plot) {
    const PlotMSPlotParameters& params = plot.parameters();
    PlotMSPlotParameters newParams(params);
    getValue(newParams);
    
    const PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>(),
                      *c2 = newParams.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Canvas* cv = params.typedGroup<PMS_PP_Canvas>(),
                       *cv2 = newParams.typedGroup<PMS_PP_Canvas>();
    
    // shouldn't happen
    if(c == NULL || c2 == NULL || cv == NULL || cv2 == NULL) return;
    
    // Update widgets.
    vector<pair<PMS::Axis,unsigned int> > laxes = plot.data().loadedAxes();
    bool found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == c2->xAxis()) found = true;
    xInCache->setChecked(found);
    xDataFrame->setVisible(PMS::axisIsData(c2->xAxis()));
    
    found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == c2->yAxis()) found = true;
    yInCache->setChecked(found);
    yDataFrame->setVisible(PMS::axisIsData(c2->yAxis()));
    
    // Update labels.
    changedText(xLabel, c->xAxis() != c2->xAxis() || (xDataFrame->isVisible()&&
                c->xDataColumn() != c2->xDataColumn()));
    changedText(xDataLabel, c->xDataColumn() != c2->xDataColumn());
    changedText(xAttachLabel, cv->xAxis() != cv2->xAxis());
    changedText(xRangeLabel, cv->xRangeSet() != cv2->xRangeSet() ||
                (cv->xRangeSet() && cv->xRange() != cv2->xRange()));
    
    changedText(yLabel, c->yAxis() != c2->yAxis() || (yDataFrame->isVisible()&&
                c->yDataColumn() != c2->yDataColumn()));
    changedText(yDataLabel, c->yDataColumn() != c2->yDataColumn());
    changedText(yAttachLabel, cv->yAxis() != cv2->yAxis());
    changedText(yRangeLabel, cv->yRangeSet() != cv2->yRangeSet() ||
                (cv->yRangeSet() && cv->yRange() != cv2->yRange()));
}

}
