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

#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

/////////////////////////////////
// PLOTMSCANVASTAB DEFINITIONS //
/////////////////////////////////

PlotMSCanvasTab::PlotMSCanvasTab(PlotMSPlotTab* plotTab,PlotMSPlotter* parent):
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    PlotFactoryPtr factory = parent->getPlotFactory();
    
    // Setup widgets.
    itsTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    titleFrame->layout()->addWidget(itsTitleWidget_);
    QButtonGroup* group = new QButtonGroup(titleFrame);
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
    itsLabelDefaults_.insert(xAxisSection, xAxisLabel->text());
    itsLabelDefaults_.insert(xLabelLabel, xLabelLabel->text());
    itsLabelDefaults_.insert(yAxisSection, yAxisLabel->text());
    itsLabelDefaults_.insert(yLabelLabel, yLabelLabel->text());
    itsLabelDefaults_.insert(gridLabel, gridLabel->text());
    
    // Connect widgets.
    connect(itsTitleWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(titleFont, SIGNAL(toggled(bool)), SLOT(tfontChanged()));
    connect(titleFontSpinner, SIGNAL(valueChanged(int)), this, SLOT(tfontChanged()));
    connect(legend, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(legendChooser,SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(xAxis, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsXLabelWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(xAxisFont, SIGNAL(toggled(bool)), SLOT(xfontChanged()));
    connect(xAxisFontSpinner, SIGNAL(valueChanged(int)), this, SLOT(xfontChanged()));
    connect(yAxis, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsYLabelWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(yAxisFont, SIGNAL(toggled(bool)), SLOT(yfontChanged()));
    connect(yAxisFontSpinner, SIGNAL(valueChanged(int)), this, SLOT(yfontChanged()));
    connect(gridMajor, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsGridMajorLineWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(gridMinor, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsGridMinorLineWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSCanvasTab::~PlotMSCanvasTab() { }


void PlotMSCanvasTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_Canvas* c = params.typedGroup<PMS_PP_Canvas>();
    PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(c == NULL) {
        params.setGroup<PMS_PP_Canvas>();
        c = params.typedGroup<PMS_PP_Canvas>();
    }
    if(d == NULL) {
        params.setGroup<PMS_PP_Display>();
        d = params.typedGroup<PMS_PP_Display>();
    }
    
    c->setTitleFormat(itsTitleWidget_->getValue());
    c->setTitleFontSet(titleFont->isChecked());
    c->setTitleFont(titleFontSpinner->value());

    c->showLegend(legend->isChecked());
    c->setLegendPosition( PlotCanvas::legendPosition(
                  legendChooser->currentText().toStdString()));

    c->showXAxis(xAxis->isChecked());
    c->setXFontSet(xAxisFont->isChecked());
    c->setXAxisFont(xAxisFontSpinner->value());
    c->setXLabelFormat(itsXLabelWidget_->getValue());

    c->showYAxis(yAxis->isChecked());
    c->setYFontSet(yAxisFont->isChecked());
    c->setYAxisFont(yAxisFontSpinner->value());
    c->setYLabelFormat(itsYLabelWidget_->getValue());

    c->showGrid(gridMajor->isChecked(), gridMinor->isChecked(),
            itsGridMajorLineWidget_->getLine(),
            itsGridMinorLineWidget_->getLine());
}

void PlotMSCanvasTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Canvas* c = params.typedGroup<PMS_PP_Canvas>();
    const PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(c == NULL || d == NULL) return; // shouldn't happen
    
    itsTitleWidget_->setValue(c->titleFormat().format);
    titleFont->setChecked(c->titleFontSet());
    titleFontSpinner->setValue(c->titleFont());
    
    legend->setChecked(c->legendShown());
    setChooser(legendChooser, PlotCanvas::legendPosition(c->legendPosition()));
    
    xAxis->setChecked(c->xAxisShown());
    itsXLabelWidget_->setValue(c->xLabelFormat().format);
    xAxisFont->setChecked(c->xFontSet());
    xAxisFontSpinner->setValue(c->xAxisFont());
    yAxis->setChecked(c->yAxisShown());
    itsYLabelWidget_->setValue(c->yLabelFormat().format);
    yAxisFont->setChecked(c->yFontSet());
    yAxisFontSpinner->setValue(c->yAxisFont());

    gridMajor->setChecked(c->gridMajorShown());
    itsGridMajorLineWidget_->setLine(c->gridMajorLine());
    gridMinor->setChecked(c->gridMinorShown());
    itsGridMinorLineWidget_->setLine(c->gridMinorLine());
}

void PlotMSCanvasTab::update(const PlotMSPlot& plot) {
    const PlotMSPlotParameters& params = plot.parameters();
    PlotMSPlotParameters newParams(params);
    getValue(newParams);
    
    const PMS_PP_Canvas* c = params.typedGroup<PMS_PP_Canvas>(),
                       *c2 = newParams.typedGroup<PMS_PP_Canvas>();
    
    highlightWidgetText(titleLabel, c->titleFormat() != c2->titleFormat() ||
                                    c->titleFontSet() != c2->titleFontSet() ||
                                    c->titleFont() != c2->titleFont());
    highlightWidgetText(legendLabel, c->legendShown() != c2->legendShown() ||
                (c->legendShown()&&c->legendPosition()!=c2->legendPosition()));
    
    highlightWidgetText(xAxisLabel, c->xAxisShown() != c2->xAxisShown());
    highlightWidgetText(xAxisSection, c->xFontSet() != c2->xFontSet() ||
                                      c->xAxisFont() != c2->xAxisFont());
    highlightWidgetText(xLabelLabel, c->xLabelFormat() != c2->xLabelFormat());
    highlightWidgetText(yAxisLabel, c->yAxisShown() != c2->yAxisShown());
    highlightWidgetText(yAxisSection, c->yFontSet() != c2->yFontSet() ||
                                      c->yAxisFont() != c2->yAxisFont());
    highlightWidgetText(yLabelLabel, c->yLabelFormat() != c2->yLabelFormat());
    
    highlightWidgetText(gridLabel, c->gridMajorShown() != c2->gridMajorShown() ||
                c->gridMinorShown() != c2->gridMinorShown() ||
                (c->gridMajorShown() &&
                 *c->gridMajorLine() != *c2->gridMajorLine()) ||
                (c->gridMinorShown() &&
                 *c->gridMinorLine() != *c2->gridMinorLine()));
}

void PlotMSCanvasTab::tfontChanged() {
    titleFontSpinner->setEnabled(titleFont->isChecked());
    changed();
}

void PlotMSCanvasTab::xfontChanged() {
    xAxisFontSpinner->setEnabled(xAxisFont->isChecked());
    changed();
}

void PlotMSCanvasTab::yfontChanged() {
    yAxisFontSpinner->setEnabled(yAxisFont->isChecked());
    changed();
}

}
