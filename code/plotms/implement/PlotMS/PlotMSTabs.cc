//# PlotMSTabs.cc: Tab GUI widgets.
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
#include <plotms/PlotMS/PlotMSTabs.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/PlotMS/PlotMS.h>

namespace casa {

///////////////////////////
// PLOTMSTAB DEFINITIONS //
///////////////////////////

PlotMSTab::PlotMSTab(PlotMS* parent, PlotMSPlotter* plotter) :
        itsParent_(parent), itsPlotter_(plotter) { }

PlotMSTab::~PlotMSTab() { }


void PlotMSTab::changedText(QLabel* label, bool changed) {
    if(itsLabelDefaults_.contains(label))
        label->setText(changedText(itsLabelDefaults_.value(label),
                       changed));
}

QString PlotMSTab::changedText(const QString& t, bool changed) {
    QString str(t);
    str.replace(' ', "&nbsp;");
    if(changed) str = "<font color=\"#FF0000\">"+ t + "</font>";
    return str;
}

bool PlotMSTab::setChooser(QComboBox* chooser, const QString& value) {
    for(int i = 0; i < chooser->count(); i++) {
        if(chooser->itemText(i) == value) {
            chooser->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}


//////////////////////////////////
// PLOTMSOPTIONSTAB DEFINITIONS //
//////////////////////////////////

PlotMSOptionsTab::PlotMSOptionsTab(PlotMS* parent, PlotMSPlotter* plotter) :
        PlotMSTab(parent, plotter), itsParameters_(parent->getParameters()) {
    setupUi(this);
    
    // Log levels
    const vector<String>& levels = PlotMSLogger::levelStrings();
    for(unsigned int i = 0; i < levels.size(); i++)
        logLevel->addItem(levels[i].c_str());
    setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));
    
    // Add self as watcher to plotms parameters.
    itsParameters_.addWatcher(this);
    
    // Connect widgets
    connect(buttonStyle, SIGNAL(currentIndexChanged(int)),
            SLOT(toolButtonStyleChanged(int)));
    connect(logLevel, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(logLevelChanged(const QString&)));
}

PlotMSOptionsTab::~PlotMSOptionsTab() { }

QList<QToolButton*> PlotMSOptionsTab::toolButtons() const {
    return QList<QToolButton*>();
}

void PlotMSOptionsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) {
    if(&p == &itsParameters_) {
        logLevel->blockSignals(true);
        setChooser(logLevel, PlotMSLogger::level(itsParameters_.logLevel()));
        logLevel->blockSignals(false);
    }
}


void PlotMSOptionsTab::toolButtonStyleChanged(int newIndex) {
    Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
    if(newIndex == 1)      style = Qt::ToolButtonTextOnly;
    else if(newIndex == 2) style = Qt::ToolButtonTextBesideIcon;
    else if(newIndex == 3) style = Qt::ToolButtonTextUnderIcon;
    itsPlotter_->setToolButtonStyle(style);
}

void PlotMSOptionsTab::logLevelChanged(const QString& newLevel) {
    bool ok;
    PlotMSLogger::Level l = PlotMSLogger::level(newLevel.toStdString(), &ok);
    if(ok) {
        itsParameters_.holdNotification(this);
        itsParameters_.setLogLevel(l);
        itsParameters_.releaseNotification();
    }
}


///////////////////////////////
// PLOTMSPLOTTAB DEFINITIONS //
///////////////////////////////

PlotMSPlotTab::PlotMSPlotTab(PlotMS* parent, PlotMSPlotter* plotter) :
        PlotMSTab(parent, plotter), itsPlotManager_(parent->getPlotManager()),
        itsCurrentPlot_(NULL), itsCurrentParameters_(NULL),
        itsUpdateFlag_(true) {
    setupUi(this);
    
    itsPlotManager_.addWatcher(this);
    
    // GUI Setup //
    
    // Setup go
    plotsChanged(itsPlotManager_);
    
    // Setup MS tab
    itsMSFileWidget_ = new PlotFileWidget(true, false);
    QtUtilities::putInFrame(msLocationFrame, itsMSFileWidget_);
    
    vector<PlotMSSelection::Field> fields = PlotMSSelection::fields();
    QGridLayout* l = new QGridLayout(msSelectionFrame);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(3);    
    QLabel* label; QLineEdit* val;
    for(unsigned int i = 0; i < fields.size(); i++) {
        label = new QLabel(QString(PlotMSSelection::field(fields[i]).c_str()));
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        l->addWidget(label, i, 0);        
        val = new QLineEdit();       
        l->addWidget(val, i, 1);
        itsSelectionValues_.insert(fields[i], val);
    }
    
    QtUtilities::putInScrollArea(msSelectionFrame);
    //QtUtilities::putInScrollArea(msAveragingFrame);
    
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
    
    itsXRangeWidget_ = new PlotRangeWidget();
    itsYRangeWidget_ = new PlotRangeWidget();
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
    itsPlotTitleWidget_ = new PlotLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    PlotFactoryPtr factory = itsPlotter_->getFactory();
    itsSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_SYMBOL(factory), false, false);
    itsMaskedSymbolWidget_ = new PlotSymbolWidget(factory, 
            PMS::DEFAULT_MASKED_SYMBOL(factory), false, false);
    QtUtilities::putInFrame(plotTitleFrame, itsPlotTitleWidget_);
    QtUtilities::putInFrame(plotUFFrame, itsSymbolWidget_);
    QtUtilities::putInFrame(plotFFrame, itsMaskedSymbolWidget_);
    
    itsLabelDefaults_.insert(plotTitleLabel, plotTitleLabel->text());
    itsLabelDefaults_.insert(plotUFlabel, plotUFlabel->text());
    itsLabelDefaults_.insert(plotFLabel, plotFLabel->text());
    
    // Setup canvas tab
    itsCanvasTitleWidget_ = new PlotLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    canvasTitleFrame->layout()->addWidget(itsCanvasTitleWidget_);
    QButtonGroup* group = new QButtonGroup(canvasTitleFrame);
    group->addButton(canvasTitleSameAsPlot);
    itsCanvasTitleWidget_->addRadioButtonsToGroup(group);

    vector<String> leg = PlotCanvas::allLegendPositionStrings();
    for(unsigned int i = 0; i < leg.size(); i++)
        canvasLegendChooser->addItem(leg[i].c_str());
    
    itsXLabelWidget_ = new PlotLabelWidget(
                       PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    itsYLabelWidget_ = new PlotLabelWidget(
                       PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT);
    QtUtilities::putInFrame(canvasXLabelFrame, itsXLabelWidget_);
    QtUtilities::putInFrame(canvasYLabelFrame, itsYLabelWidget_);
    
    itsLabelDefaults_.insert(canvasTitleLabel, canvasTitleLabel->text());
    itsLabelDefaults_.insert(canvasLegendLabel, canvasLegendLabel->text());
    itsLabelDefaults_.insert(canvasXAxisLabel, canvasXAxisLabel->text());
    itsLabelDefaults_.insert(canvasXLabelLabel, canvasXLabelLabel->text());
    itsLabelDefaults_.insert(canvasYAxisLabel, canvasYAxisLabel->text());
    itsLabelDefaults_.insert(canvasYLabelLabel, canvasYLabelLabel->text());
    
    // Setup export tab
    itsExportFileWidget_ = new PlotFileWidget(false, true);
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
    QMapIterator<PlotMSSelection::Field,QLineEdit*> selit(itsSelectionValues_);
    while (selit.hasNext()) {
        selit.next();
        connect(selit.value(), SIGNAL(editingFinished()), SLOT(tabChanged()));
    }
    connect(msAveragingChannel, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(msAveragingChannelValue, SIGNAL(valueChanged(int)),
            SLOT(tabChanged()));
    
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
    connect(canvasLegendNone, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(canvasLegendChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(tabChanged()));
    connect(canvasXAxis, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsXLabelWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasYAxis, SIGNAL(toggled(bool)), SLOT(tabChanged()));
    connect(itsYLabelWidget_, SIGNAL(changed()), SLOT(tabChanged()));
    connect(canvasPlotButton, SIGNAL(clicked()), SLOT(plot()));
    
    // Connect export
    connect(exportButton, SIGNAL(clicked()), SLOT(exportClicked()));
}

PlotMSPlotTab::~PlotMSPlotTab() { }

QList<QToolButton*> PlotMSPlotTab::toolButtons() const {
    return QList<QToolButton*>();
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
    PlotMSSelection sel;
    QMapIterator<PlotMSSelection::Field, QLineEdit*> i(itsSelectionValues_);
    while(i.hasNext()) {
         i.next();
         sel.setValue(i.key(), i.value()->text().toStdString());
     }
    params.setSelection(sel);
    PlotMSAveraging avg;
    avg.setChannel(msAveragingChannel->isChecked());
    if(avg.channel())
        avg.setChannelValue(msAveragingChannelValue->value() / 100.0);
    params.setAveraging(avg);
    
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
    params.setLegend(!canvasLegendNone->isChecked(),PlotCanvas::legendPosition(
                     canvasLegendChooser->currentText().toStdString()));
    params.setShowAxes(canvasXAxis->isChecked(), canvasYAxis->isChecked());
    params.setCanvasXAxisLabelFormat(itsXLabelWidget_->getValue());
    params.setCanvasYAxisLabelFormat(itsYLabelWidget_->getValue());
    
    return params;
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
    itsMSFileWidget_->setFile(params.isSet() ? params.filename() : "");
    const PlotMSSelection& sel = params.selection();
    QMapIterator<PlotMSSelection::Field, QLineEdit*> i(itsSelectionValues_);
    while(i.hasNext()) {
         i.next();
         i.value()->setText(sel.getValue(i.key()).c_str());
    }
    const PlotMSAveraging& avg = params.averaging();
    msAveragingChannel->setChecked(avg.channel());
    msAveragingChannelValue->setValue((int)(avg.channelValue() * 100));
    
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
    
    canvasLegendNone->setChecked(!params.showLegend());
    setChooser(canvasLegendChooser,
               PlotCanvas::legendPosition(params.legendPosition()));
    
    canvasXAxis->setChecked(params.showXAxis());
    itsXLabelWidget_->setValue(params.canvasXAxisLabelFormat().format);
    canvasYAxis->setChecked(params.showYAxis());
    itsYLabelWidget_->setValue(params.canvasYAxisLabelFormat().format);
    
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
        if(newParams.averaging().channel())
            msAveragingChannelValue->setValue(100);
        else msAveragingChannelValue->setValue(0);
        
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
        
        itsUpdateFlag_ = true;
    }
}

void PlotMSPlotTab::msClear() {
    itsMSFileWidget_->setFile("");
    QMapIterator<PlotMSSelection::Field, QLineEdit*> i(itsSelectionValues_);
    while(i.hasNext()) {
         i.next();
         i.value()->setText("");
    }
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

void PlotMSPlotTab::exportClicked() {
    String file = itsExportFileWidget_->getFile();
    bool ok;
    PlotExportFormat::Type t = PlotExportFormat::exportFormat(
                               exportFormat->currentText().toStdString(), &ok);
    if(exportFormat->currentIndex() != 0 && !ok) {
        itsPlotter_->showError("Could not parse export format!  (Shouldn't "
                               "happen.)", "Export Error", false);
        return;
    }
    if(exportFormat->currentIndex() == 0) {
        t = PlotExportFormat::typeForExtension(file, &ok);
        if(!ok) {
            itsPlotter_->showError("Invalid format extension for filename '" +
                                   file + "'!", "Export Error", false);
            return;
        }
    }

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
    
    itsPlotter_->doThreadedOperation(
            new PlotMSExportThread(itsCurrentPlot_, format));
}


////////////////////////////////
// PLOTMSTOOLSTAB DEFINITIONS //
////////////////////////////////

PlotMSToolsTab::PlotMSToolsTab(PlotMS* parent, PlotMSPlotter* plotter) :
        PlotMSTab(parent, plotter) {
    setupUi(this);
    
    // Get the actions that buttons will be connected to.
    const QMap<PlotMSAction::Type, QAction*>& actionMap =
        itsPlotter_->plotActionMap();
    
    // Set up hand tools
    QAction* act = actionMap.value(PlotMSAction::TOOL_MARK_REGIONS);
    connect(handMarkRegions, SIGNAL(toggled(bool)), act,
            SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    act = actionMap.value(PlotMSAction::TOOL_ZOOM);
    connect(handZoom, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    act = actionMap.value(PlotMSAction::TOOL_PAN);
    connect(handPan, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), SLOT(toolChanged()));
    
    // Set up selected regions
    connect(regionsClear, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::CLEAR_REGIONS), SLOT(trigger()));
    connect(regionsFlag, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::FLAG), SLOT(trigger()));
    connect(regionsUnflag, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::UNFLAG), SLOT(trigger()));
    connect(regionsLocate, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::LOCATE), SLOT(trigger()));
    
    // Set up stack
    connect(stackBack, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_BACK), SLOT(trigger()));
    connect(stackBase, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_BASE), SLOT(trigger()));
    connect(stackForward, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::STACK_FORWARD), SLOT(trigger()));
    
    // Set up tracker
    act = actionMap.value(PlotMSAction::TRACKER_HOVER);
    connect(trackerHover, SIGNAL(toggled(bool)), act, SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), trackerHover, SLOT(setChecked(bool)));
    act = actionMap.value(PlotMSAction::TRACKER_DISPLAY);
    connect(trackerDisplay, SIGNAL(toggled(bool)), act,SLOT(setChecked(bool)));
    connect(act, SIGNAL(toggled(bool)), trackerDisplay,SLOT(setChecked(bool)));
    
    // Set up iteration
    connect(iterationFirst, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_FIRST), SLOT(trigger()));
    connect(iterationPrev, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_PREV), SLOT(trigger()));
    connect(iterationNext, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_NEXT), SLOT(trigger()));
    connect(iterationLast, SIGNAL(clicked()),
            actionMap.value(PlotMSAction::ITER_LAST), SLOT(trigger()));
    
    // Set up hold/release
    act = actionMap.value(PlotMSAction::HOLD_RELEASE_DRAWING);
    connect(holdReleaseDrawing, SIGNAL(toggled(bool)), act,
            SLOT(setChecked(bool)));
    connect(act, SIGNAL(changed()), SLOT(holdReleaseActionChanged()));
}

PlotMSToolsTab::~PlotMSToolsTab() { }

QList<QToolButton*> PlotMSToolsTab::toolButtons() const {
    return QList<QToolButton*>() << regionsClear << regionsFlag
           << regionsUnflag << regionsLocate << stackBack << stackBase
           << stackForward << iterationFirst << iterationPrev << iterationNext
           << iterationLast;
}

void PlotMSToolsTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) { }

void PlotMSToolsTab::showIterationButtons(bool show) {
    iterationBox->setVisible(show);
}


void PlotMSToolsTab::notifyTrackerChanged(PlotTrackerTool& tool) {
    if(trackerDisplay->isChecked())
        trackerEdit->setText(tool.getAnnotation()->text().c_str());
}


void PlotMSToolsTab::holdReleaseActionChanged() {
    QAction* act = itsPlotter_->plotActionMap().value(
                   PlotMSAction::HOLD_RELEASE_DRAWING);
    holdReleaseDrawing->blockSignals(true);
    holdReleaseDrawing->setChecked(act->isChecked());
    holdReleaseDrawing->setText(act->text());
    holdReleaseDrawing->blockSignals(false);
}

void PlotMSToolsTab::toolChanged() {
    const QMap<PlotMSAction::Type, QAction*>& actionMap =
        itsPlotter_->plotActionMap();
    bool mark = actionMap[PlotMSAction::TOOL_MARK_REGIONS]->isChecked(),
         zoom = actionMap[PlotMSAction::TOOL_ZOOM]->isChecked(),
         pan  = actionMap[PlotMSAction::TOOL_PAN]->isChecked();
    
    handMarkRegions->blockSignals(true);
    handZoom->blockSignals(true);
    handPan->blockSignals(true);
    
    if(!mark && !zoom && !pan) handNone->setChecked(true);
    else if(mark) handMarkRegions->setChecked(true);
    else if(zoom) handZoom->setChecked(true);
    else if(pan) handPan->setChecked(true);
    
    handMarkRegions->blockSignals(false);
    handZoom->blockSignals(false);
    handPan->blockSignals(false);
}

}
