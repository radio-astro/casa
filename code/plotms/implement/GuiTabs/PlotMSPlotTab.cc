//# PlotMSPlotTab.cc: Subclass of PlotMSTab for controlling plot parameters.
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
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSExportThread.qo.h>
#include <plotms/PlotMS/PlotMS.h>

namespace casa {

///////////////////////////////
// PLOTMSPLOTTAB DEFINITIONS //
///////////////////////////////

PlotMSPlotTab::PlotMSPlotTab(PlotMSPlotter* parent) :  PlotMSTab(parent),
        itsPlotManager_(parent->getParent()->getPlotManager()),
        itsCurrentPlot_(NULL), itsCurrentParameters_(NULL),
        itsUpdateFlag_(true) {
    setupUi(this);
    
    itsPlotManager_.addWatcher(this);
    
    // GUI Setup //
    
    // Setup go
    plotsChanged(itsPlotManager_);
    
    // Setup MS tab
    itsMSFileWidget_ = new QtFileWidget(true, false);
    QtUtilities::putInFrame(msLocationFrame, itsMSFileWidget_);
    
    itsMSSelectionWidget_ = new PlotMSSelectionWidget();
    QtUtilities::putInFrame(msSelectionFrame, itsMSSelectionWidget_);
    QtUtilities::putInScrollArea(msSelectionFrame);
    
    itsMSAveragingWidget_ = new PlotMSAveragingWidget();
    QtUtilities::putInFrame(msAveragingFrame, itsMSAveragingWidget_);
    
    itsLabelDefaults_.insert(msLocationLabel, msLocationLabel->text());
    itsLabelDefaults_.insert(msSelectionLabel, msSelectionLabel->text());
    itsLabelDefaults_.insert(msAveragingLabel, msAveragingLabel->text());
    
    // Setup axes tab
    const vector<String>& axes = PMS::axesStrings();
    String xs = PMS::axis(PMS::DEFAULT_XAXIS),
           ys = PMS::axis(PMS::DEFAULT_YAXIS);
    unsigned int xdef = axes.size(), ydef = axes.size();
    for(unsigned int i = 0; i < axes.size(); i++) {
        axesXChooser->addItem(axes[i].c_str());
        axesYChooser->addItem(axes[i].c_str());
        if(axes[i] == xs) xdef = i;
        if(axes[i] == ys) ydef = i;
    }
    if(xdef < axes.size()) axesXChooser->setCurrentIndex(xdef);
    if(ydef < axes.size()) axesYChooser->setCurrentIndex(ydef);
    
    const vector<String>& data = PMS::dataColumnStrings();
    xs = PMS::dataColumn(PMS::DEFAULT_DATACOLUMN);
    xdef = data.size();
    for(unsigned int i = 0; i < data.size(); i++) {
        axesXDataChooser->addItem(data[i].c_str());
        axesYDataChooser->addItem(data[i].c_str());
        if(data[i] == xs) xdef = i;
    }
    if(xdef < data.size()) {
        axesXDataChooser->setCurrentIndex(xdef);
        axesYDataChooser->setCurrentIndex(ydef);
    }
    
    itsXRangeWidget_ = new PlotRangeWidget(true);
    itsYRangeWidget_ = new PlotRangeWidget(true);
    QtUtilities::putInFrame(axesXRangeFrame, itsXRangeWidget_);
    QtUtilities::putInFrame(axesYRangeFrame, itsYRangeWidget_);
    
    itsLabelDefaults_.insert(axesXLabel, axesXLabel->text());
    itsLabelDefaults_.insert(axesXDataLabel, axesXDataLabel->text());
    itsLabelDefaults_.insert(axesXAttachLabel, axesXAttachLabel->text());
    itsLabelDefaults_.insert(axesXRangeLabel, axesXRangeLabel->text());
    itsLabelDefaults_.insert(axesYLabel, axesYLabel->text());
    itsLabelDefaults_.insert(axesYDataLabel, axesYDataLabel->text());
    itsLabelDefaults_.insert(axesYAttachLabel, axesYAttachLabel->text());
    itsLabelDefaults_.insert(axesYRangeLabel, axesYRangeLabel->text());
    
    // Setup cache tab
    cacheMetaTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    cacheLoadedTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    cacheAvailableTable->horizontalHeader()->setResizeMode(
            QHeaderView::Stretch);
    
    // Setup plot tab
    itsPlotTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    PlotFactoryPtr factory = itsPlotter_->getFactory();
    itsSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_SYMBOL(factory), false, false, false, false);
    itsMaskedSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_MASKED_SYMBOL(factory), false, false, false, false);
    QtUtilities::putInFrame(plotTitleFrame, itsPlotTitleWidget_);
    QtUtilities::putInFrame(plotUFFrame, itsSymbolWidget_);
    QtUtilities::putInFrame(plotFFrame, itsMaskedSymbolWidget_);
    
    itsLabelDefaults_.insert(plotTitleLabel, plotTitleLabel->text());
    itsLabelDefaults_.insert(plotUFlabel, plotUFlabel->text());
    itsLabelDefaults_.insert(plotFLabel, plotFLabel->text());
    
    // Setup canvas tab
    itsCanvasTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    canvasTitleFrame->layout()->addWidget(itsCanvasTitleWidget_);
    QButtonGroup* group = new QButtonGroup(canvasTitleFrame);
    group->addButton(canvasTitleSameAsPlot);
    itsCanvasTitleWidget_->addRadioButtonsToGroup(group);

    vector<String> leg = PlotCanvas::allLegendPositionStrings();
    for(unsigned int i = 0; i < leg.size(); i++)
        canvasLegendChooser->addItem(leg[i].c_str());
    
    itsXLabelWidget_= new QtLabelWidget(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    itsYLabelWidget_= new QtLabelWidget(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    QtUtilities::putInFrame(canvasXLabelFrame, itsXLabelWidget_);
    QtUtilities::putInFrame(canvasYLabelFrame, itsYLabelWidget_);
    
    itsGridMajorLineWidget_ = new PlotLineWidget(factory);
    itsGridMinorLineWidget_ = new PlotLineWidget(factory);
    QtUtilities::putInFrame(canvasGridMajorFrame, itsGridMajorLineWidget_);
    QtUtilities::putInFrame(canvasGridMinorFrame, itsGridMinorLineWidget_);
    canvasGridMajorFrame->setEnabled(false);
    canvasGridMinorFrame->setEnabled(false);
    
    itsLabelDefaults_.insert(canvasTitleLabel, canvasTitleLabel->text());
    itsLabelDefaults_.insert(canvasLegendLabel, canvasLegendLabel->text());
    itsLabelDefaults_.insert(canvasXAxisLabel, canvasXAxisLabel->text());
    itsLabelDefaults_.insert(canvasXLabelLabel, canvasXLabelLabel->text());
    itsLabelDefaults_.insert(canvasYAxisLabel, canvasYAxisLabel->text());
    itsLabelDefaults_.insert(canvasYLabelLabel, canvasYLabelLabel->text());
    itsLabelDefaults_.insert(canvasGridLabel, canvasGridLabel->text());
    
    // Setup export tab
    itsExportFileWidget_ = new QtFileWidget(false, true);
    QtUtilities::putInFrame(exportFileFrame, itsExportFileWidget_);
    
    exportFormat->addItem("[by file extension]");
    vector<String> formats = PlotExportFormat::supportedFormatStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        exportFormat->addItem(formats[i].c_str());
        
    
    // Initialize to no plot (empty).
    setupForPlot(NULL);
    
    
    // GUI Connect //
    
    // Get the actions that buttons will be connected to.
    const QMap<PlotMSAction::Type, QAction*>& actionMap =
        itsPlotter_->plotActionMap();
    
    // Connect top
    connect(goChooser, SIGNAL(currentIndexChanged(int)), SLOT(goChanged(int)));
    connect(goButton, SIGNAL(clicked()), SLOT(goClicked()));
    
    // Connect ms
    connect(itsMSFileWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(itsMSSelectionWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(itsMSAveragingWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    
    connect(msClearButton, SIGNAL(clicked()), SLOT(msClear()));
    connect(msPlotButton, SIGNAL(clicked()), SLOT(plot()));
    
    // Connect axes
    connect(axesXChooser,SIGNAL(currentIndexChanged(int)), SLOT(tabChanged()));
    connect(axesXDataChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(tabChanged()));
    connect(axesXAttachBottom, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsXRangeWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(axesYChooser,SIGNAL(currentIndexChanged(int)), SLOT(tabChanged()));
    connect(axesYDataChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(tabChanged()));
    connect(axesYAttachLeft, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsYRangeWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    
    connect(axesPlotButton, SIGNAL(clicked()), SLOT(plot()));
    
    // Connect cache
    connect(cacheReleaseButton, SIGNAL(clicked()),
            actionMap[PlotMSAction::CACHE_RELEASE], SLOT(trigger()));
    connect(cacheLoadButton, SIGNAL(clicked()),
            actionMap[PlotMSAction::CACHE_LOAD], SLOT(trigger()));
    
    // Connect plot
    connect(itsPlotTitleWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(itsSymbolWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(itsMaskedSymbolWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    
    connect(plotPlotButton, SIGNAL(clicked()), SLOT(plot()));
    
    // Connect canvas
    connect(canvasTitleSameAsPlot, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsCanvasTitleWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasLegend, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(canvasLegendChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(tabChanged()));
    connect(canvasXAxis, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsXLabelWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasYAxis, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsYLabelWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasGridMajor, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsGridMajorLineWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasGridMinor, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsGridMinorLineWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasPlotButton, SIGNAL(clicked()), SLOT(plot()));
    
    // Connect export
    connect(exportButton, SIGNAL(clicked()),
    		actionMap[PlotMSAction::PLOT_EXPORT], SLOT(trigger()));
}

PlotMSPlotTab::~PlotMSPlotTab() { }

QList<QToolButton*> PlotMSPlotTab::toolButtons() const {
    QList<QToolButton*> list;
    foreach(PlotMSTab* tab, itsTabs_) list << tab->toolButtons();
    return list;
}

void PlotMSPlotTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) {
    if(&p == itsCurrentParameters_) plotsChanged(itsPlotManager_);
}

void PlotMSPlotTab::plotsChanged(const PlotMSPlotManager& manager) {
    goChooser->clear();
    
    // Add plot names to go chooser.
    const PlotMSPlot* plot;
    int setIndex = -1;
    for(unsigned int i = 0; i < manager.numPlots(); i++) {
        plot = manager.plot(i);
        goChooser->addItem(plot->name().c_str());
        
        // Keep the chooser on the same plot
        if(itsCurrentPlot_ != NULL && itsCurrentPlot_ == plot)
            setIndex = (int)i;
    }
    
    // Add "new" action(s) to go chooser.
    goChooser->addItem("New Single Plot");
    
    // Add "clear" action to go chooser.
    goChooser->addItem("Clear Plotter");
    
    // If not showing a current plot, pick the latest plot if it exists.
    if(itsCurrentPlot_ == NULL && goChooser->count() > 2)
        setIndex = goChooser->count() - 3;
    
    // Set to current plot, or latest plot if no current plot, and set tab.
    if(setIndex >= 0 && setIndex < goChooser->count() - 2) {
        goChooser->setCurrentIndex(setIndex);
        goClicked();
    }
}

PlotMSPlot* PlotMSPlotTab::currentPlot() const { return itsCurrentPlot_; }

PlotMSSinglePlotParameters PlotMSPlotTab::currentlySetParameters() const {
    // MS tab
    PlotMSSinglePlotParameters params(itsParent_, itsMSFileWidget_->getFile());
    params.setSelection(itsMSSelectionWidget_->getValue());
    params.setAveraging(itsMSAveragingWidget_->getValue());
    
    // Axes tab
    params.setXAxis(PMS::axis(axesXChooser->currentText().toStdString()));
    params.setXDataColumn(PMS::dataColumn(
                          axesXDataChooser->currentText().toStdString()));
    params.setCanvasXAxis(axesXAttachBottom->isChecked() ? X_BOTTOM : X_TOP);
    params.setXRange(itsXRangeWidget_->isCustom(),
                     itsXRangeWidget_->getRange());
    
    params.setYAxis(PMS::axis(axesYChooser->currentText().toStdString()));
    params.setYDataColumn(PMS::dataColumn(
                          axesYDataChooser->currentText().toStdString()));
    params.setCanvasYAxis(axesYAttachLeft->isChecked() ? Y_LEFT : Y_RIGHT);
    params.setYRange(itsYRangeWidget_->isCustom(),
                     itsYRangeWidget_->getRange());
    
    // Plot tab
    params.setPlotTitleFormat(itsPlotTitleWidget_->getValue());
    params.setSymbol(itsSymbolWidget_->getSymbol());
    params.setMaskedSymbol(itsMaskedSymbolWidget_->getSymbol());
    
    // Canvas tab
    if(canvasTitleSameAsPlot->isChecked())
        params.setCanvasTitleFormat(itsPlotTitleWidget_->getValue());
    else
        params.setCanvasTitleFormat(itsCanvasTitleWidget_->getValue());
    params.setLegend(canvasLegend->isChecked(), PlotCanvas::legendPosition(
                     canvasLegendChooser->currentText().toStdString()));
    params.setShowAxes(canvasXAxis->isChecked(), canvasYAxis->isChecked());
    params.setCanvasXAxisLabelFormat(itsXLabelWidget_->getValue());
    params.setCanvasYAxisLabelFormat(itsYLabelWidget_->getValue());
    params.setShowGrid(canvasGridMajor->isChecked(),
            canvasGridMinor->isChecked());
    PlotLinePtr major = params.showGridMajor() ?
            itsGridMajorLineWidget_->getLine() : params.gridMajorLine();
    PlotLinePtr minor = params.showGridMinor() ?
            itsGridMinorLineWidget_->getLine() : params.gridMinorLine();
    params.setGridLines(major, minor);
    
    return params;
}

PlotExportFormat PlotMSPlotTab::currentlySetExportFormat() const {
    String file = itsExportFileWidget_->getFile();
    PlotExportFormat::Type t = (exportFormat->currentIndex() == 0) ?
    		PlotExportFormat::typeForExtension(file) :
    	    PlotExportFormat::exportFormat(
    	        exportFormat->currentText().toStdString());

    PlotExportFormat format(t, file);
    format.resolution = exportHighRes->isChecked()? PlotExportFormat::HIGH :
                                                    PlotExportFormat::SCREEN;
    format.dpi = (exportDPI->isVisible() && exportDPI->isChecked()) ?
                 exportDPIspinner->value() : -1;
    if(exportSize->isVisible() && exportSize->isChecked()) {
        format.width  = exportSizeSpinner1->value();
        format.height = exportSizeSpinner2->value();
    } else {
        format.width  = -1;
        format.height = -1;
    }
    
    return format;
}


// Protected //

void PlotMSPlotTab::addTab(PlotMSTab* tab) {
    if(tab != NULL && !itsTabs_.contains(tab)) {
        itsTabs_.append(tab);
        tabWidget->addTab(tab, tab->tabName());
    }
}


// Private //

void PlotMSPlotTab::setupForPlot(PlotMSPlot* p) {
    // for now, only deal with single plots
    PlotMSSinglePlot* plot = dynamic_cast<PlotMSSinglePlot*>(p);
    itsCurrentPlot_ = plot;
    tabWidget->setEnabled(plot != NULL);
    
    if(itsCurrentParameters_ != NULL)
        itsCurrentParameters_->removeWatcher(this);
    itsCurrentParameters_ = NULL;
    
    if(plot == NULL) return;
    
    bool oldupdate = itsUpdateFlag_;
    itsUpdateFlag_ = false;
    
    PlotMSSinglePlotParameters& params = plot->singleParameters();
    params.addWatcher(this);
    itsCurrentParameters_ = &params;
    
    // MS tab
    itsMSFileWidget_->setFile(params.isSet() ? params.filename() : "");\
    itsMSSelectionWidget_->setValue(params.selection());
    itsMSAveragingWidget_->setValue(params.averaging());
    
    // Axes tab
    PMS::Axis xAxis = params.xAxis(), yAxis = params.yAxis();
    setChooser(axesXChooser, PMS::axis(xAxis));
    setChooser(axesXDataChooser, PMS::dataColumn(params.xDataColumn()));
    if(params.canvasXAxis() == X_BOTTOM) axesXAttachBottom->setChecked(true);
    else                                 axesXAttachTop->setChecked(true);
    
    setChooser(axesYChooser, PMS::axis(yAxis));
    setChooser(axesYDataChooser, PMS::dataColumn(params.yDataColumn()));
    if(params.canvasYAxis() == Y_LEFT) axesYAttachLeft->setChecked(true);
    else                               axesYAttachRight->setChecked(true);
    
    // Cache tab
    updateCacheTables();
    
    // Plot tab
    itsPlotTitleWidget_->setValue(params.plotTitleFormat().format);
    itsSymbolWidget_->setSymbol(params.symbol());
    itsMaskedSymbolWidget_->setSymbol(params.maskedSymbol());
    
    // Canvas tab
    if(params.plotTitleFormat().format == params.canvasTitleFormat().format)
        canvasTitleSameAsPlot->setChecked(true);
    else
        itsCanvasTitleWidget_->setValue(params.canvasTitleFormat().format);
    
    canvasLegend->setChecked(params.showLegend());
    setChooser(canvasLegendChooser,
               PlotCanvas::legendPosition(params.legendPosition()));
    
    canvasXAxis->setChecked(params.showXAxis());
    itsXLabelWidget_->setValue(params.canvasXAxisLabelFormat().format);
    canvasYAxis->setChecked(params.showYAxis());
    itsYLabelWidget_->setValue(params.canvasYAxisLabelFormat().format);

    canvasGridMajor->setChecked(params.showGridMajor());
    itsGridMajorLineWidget_->setLine(params.gridMajorLine());
    canvasGridMinor->setChecked(params.showGridMinor());
    itsGridMinorLineWidget_->setLine(params.gridMinorLine());
    
    itsUpdateFlag_ = oldupdate;
    
    tabChanged();
}

vector<PMS::Axis> PlotMSPlotTab::selectedLoadOrReleaseAxes(bool load) const {
    QTableWidget* tw = load ? cacheAvailableTable : cacheLoadedTable;
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

void PlotMSPlotTab::updateCacheTables() {
    cacheMetaTable->setRowCount(0);
    cacheLoadedTable->setRowCount(0);
    
    static const vector<PMS::Axis>& axes = PMS::axes();
    cacheAvailableTable->setRowCount(axes.size());
    for(unsigned int i = 0; i < axes.size(); i++) {
        cacheAvailableTable->setItem(i, 0, new QTableWidgetItem(PMS::axis(
                                     axes[i]).c_str()));
        cacheAvailableTable->setItem(i, 1, new QTableWidgetItem(PMS::axisType(
                                     PMS::axisType(axes[i])).c_str()));
    }
    cacheAvailableTable->sortItems(0);
    cacheAvailableTable->resizeRowsToContents();
    
    if(itsCurrentPlot_ == NULL) return;
    
    vector<pair<PMS::Axis, unsigned int> > laxes =
        itsCurrentPlot_->data().loadedAxes();
    unsigned int nmeta = 0, nloaded = 0;
    for(unsigned int i = 0; i < laxes.size(); i++) {
        if(PlotMSCache::axisIsMetaData(laxes[i].first)) nmeta++;
        else                                            nloaded++;
    }
    cacheMetaTable->setRowCount(nmeta);
    cacheLoadedTable->setRowCount(nloaded);
    
    QTableWidget* tw; unsigned int ti; QTableWidgetItem* twi;
    QString qaxis;
    unsigned int imeta = 0, iloaded = 0;
    for(unsigned int i = 0; i < laxes.size(); i++) {
        if(PlotMSCache::axisIsMetaData(laxes[i].first)) {
            tw = cacheMetaTable;   ti = imeta;   imeta++;
        } else {
            tw = cacheLoadedTable; ti = iloaded; iloaded++;
        }
        
        qaxis = PMS::axis(laxes[i].first).c_str();
        
        // move from available table
        for(int r = 0; r < cacheAvailableTable->rowCount(); r++) {
            twi = cacheAvailableTable->item(r, 0);
            if(twi->text() == qaxis) {
                cacheAvailableTable->takeItem(r, 0);
                tw->setItem(ti, 0, twi);
                
                twi = new QTableWidgetItem(QString("%1").arg(laxes[i].second));
                tw->setItem(ti, 1, twi);
                
                twi = cacheAvailableTable->takeItem(r, 1);
                tw->setItem(ti, 2, twi);
                
                cacheAvailableTable->removeRow(r);
                break;
            }
        }
    }
    
    if(nmeta > 0) {
        cacheMetaTable->sortItems(0);
        cacheMetaTable->resizeRowsToContents();
    }
    if(nloaded > 0) {
        cacheLoadedTable->sortItems(0);
        cacheLoadedTable->resizeRowsToContents();
    }
}


// Private Slots //

void PlotMSPlotTab::goChanged(int index) {
    if(index < (int)itsPlotManager_.numPlots()) {
        // show "edit" button if not current plot
        goButton->setText("Edit");
        goButton->setVisible(itsPlotManager_.plot(index) != itsCurrentPlot_);
    } else {
        // show "go" button
        goButton->setText("Go");
        goButton->setVisible(true);
    }
}

void PlotMSPlotTab::goClicked() {
    int index = goChooser->currentIndex();
    
    if(index < (int)itsPlotManager_.numPlots()) {
        setupForPlot(itsPlotManager_.plot(index));
        goChanged(index);
        
    } else {
        int newSinglePlot = goChooser->count() - 2,
            clearPlotter  = goChooser->count() - 1;
        
        if(index == newSinglePlot) {
            // this will update the go chooser as necessary
            PlotMSPlot* plot = itsPlotManager_.addSinglePlot(itsParent_);
            
            // switch to new plot if needed
            if(itsCurrentPlot_ != NULL) {
                for(unsigned int i = 0; i < itsPlotManager_.numPlots(); i++) {
                    if(itsPlotManager_.plot(i) == plot) {
                        goChooser->setCurrentIndex(i);
                        goClicked();
                        break;
                    }
                }
            }
            
        } else if(index == clearPlotter) {
            // this will update the go chooser as necessary
            itsPlotter_->plotActionMap().value(
                    PlotMSAction::CLEAR_PLOTTER)->trigger();
            setupForPlot(NULL);
        }
    }
}

void PlotMSPlotTab::tabChanged() {
    if(itsUpdateFlag_ && itsCurrentPlot_ != NULL) {
        itsUpdateFlag_ = false;
        
        // for now, only deal with single plots
        PlotMSSinglePlot* plot = dynamic_cast<PlotMSSinglePlot*>(
                                 itsCurrentPlot_);
        PlotMSSinglePlotParameters& params = plot->singleParameters();
        PlotMSSinglePlotParameters newParams = currentlySetParameters();
        
        // MS tab, labels
        changedText(msLocationLabel,
                    newParams.filename() != params.filename());
        changedText(msSelectionLabel,
                    newParams.selection() != params.selection());
        changedText(msAveragingLabel,
                    newParams.averaging() != params.averaging());
        
        // Axes tab, widgets
        vector<pair<PMS::Axis,unsigned int> > laxes= plot->data().loadedAxes();
        bool found = false;
        for(unsigned int i = 0; !found && i < laxes.size(); i++)
            if(laxes[i].first == newParams.xAxis()) found = true;
        axesXInCache->setChecked(found);
        axesXDataFrame->setVisible(PMS::axisIsData(newParams.xAxis()));
        itsXRangeWidget_->setIsDate(
                PMS::axisType(newParams.xAxis()) == PMS::TTIME);
        
        found = false;
        for(unsigned int i = 0; !found && i < laxes.size(); i++)
            if(laxes[i].first == newParams.yAxis()) found = true;
        axesYInCache->setChecked(found);
        axesYDataFrame->setVisible(PMS::axisIsData(newParams.yAxis()));
        itsYRangeWidget_->setIsDate(
                PMS::axisType(newParams.yAxis()) == PMS::TTIME);
        
        // Axes tab, labels
        changedText(axesXLabel, params.xAxis() != newParams.xAxis() ||
                    (axesXDataFrame->isVisible() &&
                     params.xDataColumn() != newParams.xDataColumn()));
        changedText(axesXDataLabel,
                    params.xDataColumn() != newParams.xDataColumn());
        changedText(axesXAttachLabel,
                    params.canvasXAxis() != newParams.canvasXAxis());
        changedText(axesXRangeLabel,
                    params.xRangeSet() != newParams.xRangeSet() ||
                    (params.xRangeSet() &&
                     params.xRange() != newParams.xRange()));
        
        changedText(axesYLabel, params.yAxis() != newParams.yAxis() ||
                    (axesYDataFrame->isVisible() &&
                     params.yDataColumn() != newParams.yDataColumn()));
        changedText(axesYDataLabel,
                    params.yDataColumn() != newParams.yDataColumn());
        changedText(axesYAttachLabel,
                    params.canvasYAxis() != newParams.canvasYAxis());
        changedText(axesYRangeLabel,
                    params.yRangeSet() != newParams.yRangeSet() ||
                    (params.yRangeSet() &&
                     params.yRange() != newParams.yRange()));
        
        // Plot tab, labels
        changedText(plotTitleLabel,
                    params.plotTitleFormat() != newParams.plotTitleFormat());
        changedText(plotUFlabel, *params.symbol() != *newParams.symbol());
        changedText(plotFLabel,
                    *params.maskedSymbol() != *newParams.maskedSymbol());
        
        // Canvas tab, labels
        changedText(canvasTitleLabel, params.canvasTitleFormat() !=
                    newParams.canvasTitleFormat());
        changedText(canvasLegendLabel,
                    params.showLegend() != newParams.showLegend() ||
                    (params.showLegend() &&
                     params.legendPosition() != newParams.legendPosition()));
        
        changedText(canvasXAxisLabel,
                    params.showXAxis() != newParams.showXAxis());
        changedText(canvasXLabelLabel, params.canvasXAxisLabelFormat() !=
                    newParams.canvasXAxisLabelFormat());
        changedText(canvasYAxisLabel,
                    params.showYAxis() != newParams.showYAxis());
        changedText(canvasYLabelLabel, params.canvasYAxisLabelFormat() !=
                    newParams.canvasYAxisLabelFormat());
        
        changedText(canvasGridLabel, params.showGridMajor() !=
                    newParams.showGridMajor() || params.showGridMinor() !=
                    newParams.showGridMinor() || *params.gridMajorLine() !=
                    *newParams.gridMajorLine() || *params.gridMinorLine() !=
                    *newParams.gridMinorLine());
        
        itsUpdateFlag_ = true;
    }
}

void PlotMSPlotTab::msClear() {
    itsMSFileWidget_->setFile("");
    itsMSSelectionWidget_->setValue(PlotMSSelection());
    itsMSAveragingWidget_->setValue(PlotMSAveraging());
}

void PlotMSPlotTab::plot() {
    if(itsCurrentParameters_ != NULL) {
        PlotMSSinglePlotParameters params = currentlySetParameters();
        
        // Plot if 1) parameters have changed, 2) cache loading was canceled
        // previously.
        bool pchanged = params != *itsCurrentParameters_,
             cload = !itsCurrentPlot_->data().cacheReady();
        if(pchanged || cload) {
            if(pchanged) {
                if(itsParent_->getParameters().clearSelectionsOnAxesChange() &&
                   (params.xAxis() != itsCurrentParameters_->xAxis() ||
                    params.yAxis() != itsCurrentParameters_->yAxis())) {
                    vector<PlotCanvasPtr> canv = itsCurrentPlot_->canvases();
                    for(unsigned int i = 0; i < canv.size(); i++)
                        canv[i]->standardMouseTools()->selectTool()
                               ->clearSelectedRects();
                }
                
                itsCurrentParameters_->holdNotification(this);
                *itsCurrentParameters_ = params;
                itsCurrentParameters_->releaseNotification();
            } else if(cload) {
                itsCurrentPlot_->parametersHaveChanged(*itsCurrentParameters_,
                        PlotMSWatchedParameters::CACHE, true);
            }
            plotsChanged(itsPlotManager_);
        }
    }
}

}
