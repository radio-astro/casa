//# PlotMSCanvasTab.cc: Plot tab to manage canvas parameters.
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
#include <plotms/GuiTabs/PlotMSCanvasTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

/////////////////////////////////
// PLOTMSCANVASTAB DEFINITIONS //
/////////////////////////////////

PlotMSCanvasTab::PlotMSCanvasTab(PlotMSPlotter* parent) :
        PlotMSPlotSubtab(parent) {
    setupUi(this);
    
    PlotFactoryPtr factory = parent->getFactory();
    
    // Setup widgets.
    itsTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    titleFrame->layout()->addWidget(itsTitleWidget_);
    QButtonGroup* group = new QButtonGroup(titleFrame);
    group->addButton(titleSameAsPlot);
    itsTitleWidget_->addRadioButtonsToGroup(group);

    vector<String> leg = PlotCanvas::allLegendPositionStrings();
    for(unsigned int i = 0; i < leg.size(); i++)
        legendChooser->addItem(leg[i].c_str());
    
    itsXLabelWidget_= new QtLabelWidget(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    itsYLabelWidget_= new QtLabelWidget(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    QtUtilities::putInFrame(xLabelFrame, itsXLabelWidget_);
    QtUtilities::putInFrame(yLabelFrame, itsYLabelWidget_);
    
    itsGridMajorLineWidget_ = new PlotLineWidget(factory);
    itsGridMinorLineWidget_ = new PlotLineWidget(factory);
    QtUtilities::putInFrame(gridMajorFrame, itsGridMajorLineWidget_);
    QtUtilities::putInFrame(gridMinorFrame, itsGridMinorLineWidget_);
    gridMajorFrame->setEnabled(false);
    gridMinorFrame->setEnabled(false);
    
    itsLabelDefaults_.insert(titleLabel, titleLabel->text());
    itsLabelDefaults_.insert(legendLabel, legendLabel->text());
    itsLabelDefaults_.insert(xAxisLabel, xAxisLabel->text());
    itsLabelDefaults_.insert(xLabelLabel, xLabelLabel->text());
    itsLabelDefaults_.insert(yAxisLabel, yAxisLabel->text());
    itsLabelDefaults_.insert(yLabelLabel, yLabelLabel->text());
    itsLabelDefaults_.insert(gridLabel, gridLabel->text());
    
    // Connect widgets.
    connect(titleSameAsPlot, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsTitleWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(legend, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(legendChooser,SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(xAxis, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsXLabelWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(yAxis, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsYLabelWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(gridMajor, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsGridMajorLineWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(gridMinor, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsGridMinorLineWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSCanvasTab::~PlotMSCanvasTab() { }


void PlotMSCanvasTab::getValue(PlotMSPlotParameters& params) const {
    PlotMSSinglePlotParameters* sp =
        dynamic_cast<PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    if(titleSameAsPlot->isChecked())
        sp->setCanvasTitleFormat(sp->plotTitleFormat());
    else sp->setCanvasTitleFormat(itsTitleWidget_->getValue());
    sp->setLegend(legend->isChecked(), PlotCanvas::legendPosition(
                  legendChooser->currentText().toStdString()));
    sp->setShowAxes(xAxis->isChecked(), yAxis->isChecked());
    sp->setCanvasXAxisLabelFormat(itsXLabelWidget_->getValue());
    sp->setCanvasYAxisLabelFormat(itsYLabelWidget_->getValue());
    sp->setShowGrid(gridMajor->isChecked(), gridMinor->isChecked());
    PlotLinePtr major = sp->showGridMajor() ?
            itsGridMajorLineWidget_->getLine() : sp->gridMajorLine();
    PlotLinePtr minor = sp->showGridMinor() ?
            itsGridMinorLineWidget_->getLine() : sp->gridMinorLine();
    sp->setGridLines(major, minor);
}

void PlotMSCanvasTab::setValue(const PlotMSPlotParameters& params) {
    const PlotMSSinglePlotParameters* sp =
        dynamic_cast<const PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    if(sp->plotTitleFormat().format == sp->canvasTitleFormat().format)
        titleSameAsPlot->setChecked(true);
    else
        itsTitleWidget_->setValue(sp->canvasTitleFormat().format);
    
    legend->setChecked(sp->showLegend());
    setChooser(legendChooser,PlotCanvas::legendPosition(sp->legendPosition()));
    
    xAxis->setChecked(sp->showXAxis());
    itsXLabelWidget_->setValue(sp->canvasXAxisLabelFormat().format);
    yAxis->setChecked(sp->showYAxis());
    itsYLabelWidget_->setValue(sp->canvasYAxisLabelFormat().format);

    gridMajor->setChecked(sp->showGridMajor());
    itsGridMajorLineWidget_->setLine(sp->gridMajorLine());
    gridMinor->setChecked(sp->showGridMinor());
    itsGridMinorLineWidget_->setLine(sp->gridMinorLine());
}

void PlotMSCanvasTab::update(const PlotMSPlot& plot) {
    const PlotMSSinglePlot* p = dynamic_cast<const PlotMSSinglePlot*>(&plot);
    if(p == NULL) return;
    
    const PlotMSSinglePlotParameters& params = p->singleParameters();
    PlotMSSinglePlotParameters newParams(params);
    getValue(newParams);
    
    changedText(titleLabel, params.canvasTitleFormat() !=
                newParams.canvasTitleFormat());
    changedText(legendLabel,
                params.showLegend() != newParams.showLegend() ||
                (params.showLegend() &&
                 params.legendPosition() != newParams.legendPosition()));
    
    changedText(xAxisLabel,
                params.showXAxis() != newParams.showXAxis());
    changedText(xLabelLabel, params.canvasXAxisLabelFormat() !=
                newParams.canvasXAxisLabelFormat());
    changedText(yAxisLabel,
                params.showYAxis() != newParams.showYAxis());
    changedText(yLabelLabel, params.canvasYAxisLabelFormat() !=
                newParams.canvasYAxisLabelFormat());
    
    changedText(gridLabel, params.showGridMajor() !=
                newParams.showGridMajor() || params.showGridMinor() !=
                newParams.showGridMinor() || *params.gridMajorLine() !=
                *newParams.gridMajorLine() || *params.gridMinorLine() !=
                *newParams.gridMinorLine());
}

}
