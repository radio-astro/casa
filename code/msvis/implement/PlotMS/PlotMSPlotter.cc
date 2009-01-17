//# PlotMSPlotter.cc: GUI for plotms.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#include <msvis/PlotMS/PlotMSPlotter.qo.h>

#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <msvis/PlotMS/PlotMS.h>
//#include <xmlcasa/version.h>

#include <fstream>

#include <casa/iomanip.h>

// TODO PlotMSPlotter: grid spacing?, casa version, hold/release canvas drawing
// TODO PlotMS: ms selection

namespace casa {

///////////////////////////////
// PLOTMSPLOTTER DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

PlotMSPlotter::PlotMSPlotter(PlotMS* parent, Plotter::Implementation impl) :
        itsLogger_(parent->getLogger()), itsLogLevel_(parent->getLogLevel()) {
    init(1, 1, impl);
    registerHandler(parent);
}

PlotMSPlotter::PlotMSPlotter(unsigned int nrows, unsigned int ncols,
        PlotMS* parent, Plotter::Implementation impl) :
        itsLogger_(parent->getLogger()), itsLogLevel_(parent->getLogLevel()) {
    init(nrows, ncols, impl);
    registerHandler(parent);
}

PlotMSPlotter::~PlotMSPlotter() { }


// Public Methods //

PlotCanvasPtr PlotMSPlotter::getCanvas(unsigned int row, unsigned int col) {
    if(row >= nRows_ || col >= nCols_) return itsCanvases_[0];
    else return itsCanvases_[(row * nCols_) + col];
}


int PlotMSPlotter::execLoop() {
    if(isQwt_) {
        setVisible(true);
        return itsFactory_->execLoop();
    } else {
        itsFactory_->execLoop(); // HOPEFULLY this runs asynchronously
                                 // but shouldn't be a problem as long as we
                                 // keep using Qt
        return qApp->exec();
    }
}


void PlotMSPlotter::registerHandler(PlotMSHandler* handler) {
    for(unsigned int i = 0; i < itsHandlers_.size(); i++)
        if(itsHandlers_[i] == handler) return;
    
    itsHandlers_.push_back(handler);
}

void PlotMSPlotter::showIterationButtons(bool show) {
    actionNextIter->setVisible(show);
    actionPrevIter->setVisible(show);
    actionApplyIter->setVisible(show);
}


unsigned int PlotMSPlotter::getNRows() const { return nRows_; }
unsigned int PlotMSPlotter::getNCols() const { return nCols_; }

void PlotMSPlotter::setGridDimensions(unsigned int nrows, unsigned int ncols) {
    if(nrows < 1) nrows = 1;
    if(ncols < 1) ncols = 1;
    if(nrows == nRows_ && ncols == nCols_) return;
    
    PlotLayoutGridPtr layout = new PlotLayoutGrid(nrows, ncols);
    unsigned int index = 0;
    PlotCanvasPtr canvas;
    for(unsigned int i = 0; i < nrows; i++) {
        for(unsigned int j = 0; j < ncols; j++) {
            canvas = (index < itsCanvases_.size()) ?
                     itsCanvases_[index++] : itsFactory_->canvas();
            layout->setCanvasAt(PlotGridCoordinate(i,j), canvas);
        }
    }

    itsPlotter_->setCanvasLayout(layout);
    itsCanvases_ = layout->allCanvases();
    
    PlotStandardMouseToolGroupPtr group;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        group = itsCanvases_[i]->standardMouseTools();
        group->trackerTool()->addNotifier(itsToolsDock_);
        group->selectTool()->setDrawRects(true);
    }
    nRows_ = nrows; nCols_ = ncols;
}

void PlotMSPlotter::updateOptions(const PlotMSOptions& opts) {
    itsOptionsDock_->setOptions(opts);
    itsToolsDock_->setOptions(opts);
}

void PlotMSPlotter::setGridSpacing(unsigned int spacing) {
    itsPlotter_->canvasLayout()->setSpacing(spacing);
}

void PlotMSPlotter::setWindowTitle(const String& windowTitle) {
    QMainWindow::setWindowTitle(QString(windowTitle.c_str()));
}

void PlotMSPlotter::setStatusText(const String& statusText) {
    if(statusText.empty()) statusBar()->clearMessage();
    else                   statusBar()->showMessage(statusText.c_str());
}

void PlotMSPlotter::setCanvasTitle(String canvasTitle, unsigned int row,
                                   unsigned int col) {
    getCanvas(row, col)->setTitle(canvasTitle);
}

void PlotMSPlotter::setAxesLabels(String xLabel, String yLabel,
                                  unsigned int row, unsigned int col) {
    getCanvas(row, col)->setAxisLabel(X_BOTTOM, xLabel);
    getCanvas(row, col)->setAxisLabel(Y_LEFT, yLabel);
}

PlotAxisScale PlotMSPlotter::getXAxisType(unsigned int row, unsigned int col) {
    return getCanvas(row, col)->axisScale(X_BOTTOM);
}

PlotAxisScale PlotMSPlotter::getYAxisType(unsigned int row, unsigned int col) {
    return getCanvas(row, col)->axisScale(Y_LEFT);
}

void PlotMSPlotter::setXAxisType(PlotAxisScale type, unsigned int row,
                                 unsigned int col) {
    getCanvas(row, col)->setAxisScale(X_BOTTOM, type);
}

void PlotMSPlotter::setYAxisType(PlotAxisScale type, unsigned int row,
                                 unsigned int col) {
    getCanvas(row, col)->setAxisScale(Y_LEFT, type);
}

void PlotMSPlotter::setGrid(bool xMaj, bool xMin, bool yMaj, bool yMin,
        unsigned int row, unsigned int col) {
    getCanvas(row, col)->setGridShown(xMaj, xMin, yMaj, yMin);
}

void PlotMSPlotter::showAxes(bool showX, bool showY, unsigned int row,
        unsigned int col) {
    PlotCanvasPtr canvas = getCanvas(row, col);
    canvas->showAxis(X_BOTTOM, showX);
    canvas->showAxis(Y_LEFT, showY);
}

void PlotMSPlotter::showLegend(bool show, PlotCanvas::LegendPosition position,
                               unsigned int row, unsigned int col) {
    getCanvas(row, col)->showLegend(show, position);
}


void PlotMSPlotter::showLines(bool showLines) {
    itsLine_->setStyle(showLines ? PlotLine::SOLID : PlotLine::NOLINE);
}

void PlotMSPlotter::showSymbols(bool showSymbols) {
    itsSymbol_->setSymbol(showSymbols ? PlotSymbol::DIAMOND :
                                      PlotSymbol::NOSYMBOL);
}

PlotLinePtr PlotMSPlotter::setLine(String color, PlotLine::Style style,
                                   double width) {
    setLine(itsFactory_->line(color, style, width));
    return itsLine_;
}

void PlotMSPlotter::setLine(PlotLinePtr line) { itsLine_ = line; }

PlotSymbolPtr PlotMSPlotter::setSymbol(PlotSymbol::Symbol symbol, String color,
                                       double size, bool outline) {
    PlotSymbolPtr sym = itsFactory_->symbol(symbol);
    sym->setAreaFill(color);
    sym->setSize(size, size);
    sym->setLine("000000", outline ? PlotLine::SOLID : PlotLine::NOLINE, 1);
    setSymbol(sym);
    return itsSymbol_;
}

void PlotMSPlotter::setSymbol(PlotSymbolPtr symbol) { itsSymbol_ = symbol; }

PlotAreaFillPtr PlotMSPlotter::setAreaFill(String color,
                                           PlotAreaFill::Pattern pattern) {
    setAreaFill(itsFactory_->areaFill(color, pattern));
    return itsAreaFill_;
}

void PlotMSPlotter::setAreaFill(PlotAreaFillPtr a) { itsAreaFill_ = a; }


ScatterPlotPtr PlotMSPlotter::plotxy(PlotPointDataPtr data, String color,
        String title, bool overplot, unsigned int cRow, unsigned int cCol) {
    ScatterPlotPtr plot = itsFactory_->scatterPlot(data);
    plot->setTitle(title);
    
    PlotLinePtr line = itsFactory_->line(*itsLine_);
    if(!color.empty()) line->setColor(color);
    plot->setLine(line);
    PlotSymbolPtr symbol = itsFactory_->symbol(*itsSymbol_);
    if(!color.empty()) {
        symbol->setLine(color);
        symbol->setAreaFill(color);
    }
    plot->setSymbol(symbol);
    
    if(!overplot) getCanvas(cRow, cCol)->clearPlots();
    getCanvas(cRow, cCol)->plotItem(plot);
    return plot;
}


PlotAnnotationPtr PlotMSPlotter::annotation(double x, double y, String text,
        bool mainLayer, unsigned int cRow, unsigned int cCol) {
    PlotAnnotationPtr annotation =  itsFactory_->annotation(text, x, y);
    getCanvas(cRow, cCol)->plotItem(annotation, mainLayer ? MAIN : ANNOTATION);
    return annotation;
}

PlotShapeRectanglePtr PlotMSPlotter::rectangle(double left, double top,
        double right, double bottom, bool mainLayer, unsigned int cRow,
        unsigned int cCol) {
    PlotShapeRectanglePtr rect = itsFactory_->shapeRectangle(left, top,
                                                           right, bottom);
    rect->setLine(itsLine_);
    rect->setAreaFill(itsAreaFill_);
    getCanvas(cRow, cCol)->plotItem(rect, mainLayer ? MAIN : ANNOTATION);
    return rect;
}


void PlotMSPlotter::clear(unsigned int canvasRow, unsigned int canvasCol) {
    getCanvas(canvasRow, canvasCol)->clearItems();
    getCanvas(canvasRow, canvasCol)->standardMouseTools()->selectTool()->
                                     clearSelectedRects();
}

void PlotMSPlotter::clearAll() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        itsCanvases_[i]->clearItems();
        itsCanvases_[i]->standardMouseTools()->selectTool()->
                         clearSelectedRects();
    }
}


// Public Slots //

void PlotMSPlotter::showError(const String& message, const String& title,
        bool isWarning) {
    if(isWarning) QMessageBox::warning( this, title.c_str(), message.c_str());
    else          QMessageBox::critical(this, title.c_str(), message.c_str());
}

void PlotMSPlotter::showMessage(const String& message, const String& title) {
    QMessageBox::information(this, title.c_str(), message.c_str());
}


// Protected Methods //

void PlotMSPlotter::closeEvent(QCloseEvent* event) {
    if(!isQwt_) {
        itsPlotter_->close();
    }
}


// Private Methods //

void PlotMSPlotter::init(unsigned int nrows, unsigned int ncols,
                         Plotter::Implementation imp) {
    setupUi(this);
    itsFactory_ = plotterImplementation(imp);
    if(itsFactory_.null()) {
        String error = "Invalid plotter implementation.";
        if(imp == Plotter::QWT)
            error += "  Make sure that you have qwt installed and the "
                     "AIPS_HAS_QWT compiler flag turned on!";
        throw error;
    }
    
    // Qt
    
    // Set up main window
    itsOptionsDock_ = new PlotMSOptionsDock();
    QDockWidget* d = itsOptionsDock_->getDockWidget();
    addDockWidget(Qt::LeftDockWidgetArea, d);
    actionOptionsDock->setChecked(true);
    
    itsToolsDock_ = new PlotMSToolsDock();
    itsToolsDock_->setLogLevel(itsLogLevel_);
    d = itsToolsDock_->getDockWidget();
    addDockWidget(Qt::LeftDockWidgetArea, d);
    actionToolsDock->setChecked(true);
    
    // Set up actions
    holdReleaseDrawing(actionHoldRelease->isChecked());
    connect(actionOptionsDock, SIGNAL(triggered()),
            SLOT(showHideOptionsDock()));
    connect(itsOptionsDock_->getDockWidget(), SIGNAL(visibilityChanged(bool)),
            SLOT(optionsDockShownOrHidden(bool)));
    connect(itsOptionsDock_, SIGNAL(optionsSet(const PlotMSOptions&)),
            SLOT(optionsChanged(const PlotMSOptions&)));
    connect(actionToolsDock, SIGNAL(triggered()), SLOT(showHideToolsDock()));
    connect(itsToolsDock_->getDockWidget(), SIGNAL(visibilityChanged(bool)),
            SLOT(toolsDockShownOrHidden(bool)));
    connect(itsToolsDock_, SIGNAL(showError(const String&,const String&,bool)),
            SLOT(showError(const String&, const String&, bool)));
    connect(itsToolsDock_,
            SIGNAL(mouseToolChanged(PlotStandardMouseToolGroup::Tool)),
            SLOT(mouseToolChanged(PlotStandardMouseToolGroup::Tool)));
    connect(itsToolsDock_, SIGNAL(trackerTurned(bool, bool)),
            SLOT(trackerTurned(bool, bool)));
    connect(itsToolsDock_, SIGNAL(exportRequested(const PlotExportFormat&, int,
            int)), SLOT(exportRequested(const PlotExportFormat&, int, int)));
    connect(itsToolsDock_, SIGNAL(logLevelChanged(PlotMSLogger::Level)),
            SLOT(logLevelChanged(PlotMSLogger::Level)));;
    connect(actionMarkRegions, SIGNAL(triggered()), SLOT(actionMark_()));
    connect(actionClearRegions, SIGNAL(triggered()), SLOT(actionClear_()));
    connect(actionFlag, SIGNAL(triggered()), SLOT(actionFlag_()));
    connect(actionUnflag, SIGNAL(triggered()), SLOT(actionUnflag_()));
    connect(actionLocate, SIGNAL(triggered()), SLOT(actionLocate_()));
    connect(actionNextIter, SIGNAL(triggered()), SLOT(actionNext_()));
    connect(actionPrevIter, SIGNAL(triggered()), SLOT(actionPrev_()));
    connect(actionHoldRelease, SIGNAL(toggled(bool)),
            SLOT(holdReleaseDrawing(bool)));
    connect(actionAbout, SIGNAL(triggered()), SLOT(showAbout()));
    connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    // Non-Qt
    
    // Set up plotter
    itsPlotter_ = itsFactory_->plotter(nrows, ncols, "PlotMS", true,
            PlotMSLogger::levelToMeasurementFlag(itsLogLevel_));
    nRows_ = nrows; nCols_ = ncols;
    itsCanvases_ = itsPlotter_->canvasLayout()->allCanvases();
    PlotStandardMouseToolGroupPtr group;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        group = itsCanvases_[i]->standardMouseTools();
        group->trackerTool()->addNotifier(itsToolsDock_);
        group->selectTool()->setDrawRects(true);
    }
    
    isQwt_ = imp == Plotter::QWT && itsPlotter_->isQWidget();
    if(isQwt_) {
        QHBoxLayout* l = new QHBoxLayout(frame);
        l->setContentsMargins(3, 3, 3, 3);
        l->addWidget(dynamic_cast<QWidget*>(itsPlotter_.operator->()));
    }
    
    // Set up defaults
    itsLine_ = itsFactory_->line("black", PlotLine::NOLINE, 1.0);
    itsSymbol_ = itsFactory_->symbol(PlotSymbol::CIRCLE);
    itsSymbol_->setLine(itsLine_);
    itsSymbol_->setAreaFill("blue");
    itsSymbol_->setSize(3, 3);
    
    // Force window to set size before drawing to avoid unecessary redraws.
    QApplication::processEvents();
    itsOptionsDock_->resize(itsOptionsDock_->minimumSize());
    itsToolsDock_->resize(itsToolsDock_->minimumSize());
    setVisible(false);
}

void PlotMSPlotter::action(QAction* act) {
    if(act == actionMarkRegions) {
        //markSelected_ = actionMarkRegions->isChecked();
        updateMouseSelectTools();
    } else if(act == actionClearRegions) {
        for(unsigned int i = 0; i < itsCanvases_.size(); i++)
            itsCanvases_[i]->standardMouseTools()->selectTool()->
                             clearSelectedRects();

    } else if(act == actionFlag || act == actionUnflag || act == actionLocate){
        vector<PlotMSRect> v;
        
        unsigned int crow(0), ccol(0);
        vector<double> ulX, ulY, lrX, lrY;
        for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
            crow = i / nCols_;
            ccol = i % nCols_;
            itsCanvases_[i]->standardMouseTools()->selectTool()->
                             getSelectedRects(ulX, ulY, lrX, lrY);
            for(unsigned int j = 0; j < ulX.size(); j++)
                v.push_back(PlotMSRect(ulX[j], ulY[j], lrX[j], lrY[j],
                                       crow, ccol));
        }
        if(v.size() == 0) return;
        
        bool all = actionApplyIter->isVisible()&& actionApplyIter->isChecked();
        for(unsigned int i = 0; i < itsHandlers_.size(); i++) {
            if(act == actionFlag) itsHandlers_[i]->handleFlag(v, all);
            else if(act == actionUnflag) itsHandlers_[i]->handleUnflag(v, all);
            else if(act == actionLocate) itsHandlers_[i]->handleLocate(v, all);
        }
        
    } else if(act == actionPrevIter) {
        for(unsigned int i = 0; i < itsHandlers_.size(); i++)
            itsHandlers_[i]->handlePrev();
    } else if(act == actionNextIter) {
        for(unsigned int i = 0; i < itsHandlers_.size(); i++)
            itsHandlers_[i]->handleNext();
    } else if(act == actionQuit) close();
}

void PlotMSPlotter::updateMouseSelectTools() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        if(itsCanvases_[i]->standardMouseTools()->activeToolType() ==
            PlotStandardMouseToolGroup::SELECT) {
            itsCanvases_[i]->standardMouseTools()->selectTool()->setActive(
                             actionMarkRegions->isChecked());
        }
    }
}


// Private Slots //

void PlotMSPlotter::showHideOptionsDock() {
    QDockWidget* d = itsOptionsDock_->getDockWidget();
    if(d->isVisible()) d->hide();
    else               d->show();
}

void PlotMSPlotter::optionsDockShownOrHidden(bool visible) {
    actionOptionsDock->blockSignals(true);
    actionOptionsDock->setChecked(visible);
    actionOptionsDock->blockSignals(false);
}

void PlotMSPlotter::optionsChanged(const PlotMSOptions& newOptions) {
    for(unsigned int i = 0; i < itsHandlers_.size(); i++)
        itsHandlers_[i]->handleOptionsChanged(newOptions);
}

void PlotMSPlotter::showHideToolsDock() {
    QDockWidget* d = itsToolsDock_->getDockWidget();
    if(d->isVisible()) d->hide();
    else               d->show();
}

void PlotMSPlotter::toolsDockShownOrHidden(bool visible) {
    actionToolsDock->blockSignals(true);
    actionToolsDock->setChecked(visible);
    actionToolsDock->blockSignals(false);
}

void PlotMSPlotter::mouseToolChanged(PlotStandardMouseToolGroup::Tool tool) {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i]->standardMouseTools()->setActiveTool(tool);
    if(tool == PlotStandardMouseToolGroup::SELECT)
        updateMouseSelectTools();
}

void PlotMSPlotter::trackerTurned(bool hover, bool display) {
    PlotStandardMouseToolGroupPtr group;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        group = itsCanvases_[i]->standardMouseTools();
        group->trackerTool()->setDrawText(hover);
        group->turnTracker(hover || display);
    }
}

void PlotMSPlotter::exportRequested(const PlotExportFormat& format, int row,
        int col) {
    bool error = false,
         wholePlotter = row < 0 || col < 0 || row >= (int)nRows_ ||
                        col >= (int)nCols_;
    if(wholePlotter) error = !itsPlotter_->exportToFile(format);
    else             error = !getCanvas(row, col)->exportToFile(format);

    if(error)
        showError("There was a problem exporting to file " + format.location +
                  "!", "Export Error", false);
    else showMessage(String("Successfully exported ") +
                     (wholePlotter ? "plotter" : "canvas") + " to " +
                     PlotExportFormat::exportFormat(format.type) + " file!",
                     "Export Success");
}

void PlotMSPlotter::logLevelChanged(PlotMSLogger::Level level) {
    for(unsigned int i = 0; i < itsHandlers_.size(); i++)
        itsHandlers_[i]->handleLogLevelChanged(level);
}

void PlotMSPlotter::holdReleaseDrawing(bool toggled) {
    if(toggled) {
        for(unsigned int i = 0; i < itsCanvases_.size(); i++)
            itsCanvases_[i]->holdDrawing();
        actionHoldRelease->setText("Release Drawing");
    } else {
        for(unsigned int i = 0; i < itsCanvases_.size(); i++)
            itsCanvases_[i]->releaseDrawing();
        actionHoldRelease->setText("Hold Drawing");
    }
}

void PlotMSPlotter::showAbout() {
    QMessageBox::about(this, "About PlotMS",
                       aboutText(itsFactory_->implementation(), true).c_str());
}


// Static //

String PlotMSPlotter::aboutText(Plotter::Implementation impl, bool useHTML) {
    stringstream ss;
    
    // Intro
    ss << "PlotMS is an interactive plotting application which deals with "
          "visibility data.  PlotMS is part of the CASA (";
    if(useHTML) ss << "<b>";
    ss << "C";
    if(useHTML) ss << "</b>";
    ss << "ommon ";
    if(useHTML) ss << "<b>";
    ss << "A";
    if(useHTML) ss << "</b>";
    ss << "stronomy ";
    if(useHTML) ss << "<b>";
    ss << "S";
    if(useHTML) ss << "</b>";
    ss << "oftware ";
    if(useHTML) ss << "<b>";
    ss << "A";
    if(useHTML) ss << "</b>";
    ss << "pplications) package and uses the Qt library for GUI work.";
    
    // Instance info
    ss << (useHTML ? "<hr />" : "\n\n");
    if(useHTML) ss << "<table><tr><td>";
    ss << "PlotMS Version:";
    ss << (useHTML ? "</td><td>" : "\t");
    ss << PLOTMS_VERSION_STR;
    ss << (useHTML ? "</td></tr><tr><td>" : "\n");
    ss << "Plotter Impl.:";
    ss << (useHTML ? "</td><td>" : "\t");
    ss << PlotterImplementations::implementationName(impl) << ", version "
       << PlotterImplementations::implementationVersion(impl);
    ss << (useHTML ? "<br />" : "\n              \t");
    ss << PlotterImplementations::implementationAbout(impl, useHTML);
    ss << (useHTML ? "</td></tr><tr><td>" : "\n");
    
    /*
    // Basically copies code from VersionInfo::report(), but avoids the need
    // to link against the xmlcasa library.
    ss << "CASA Version:";
    ss << (useHTML ? "</td><td>" : "\t");
    int mver = VersionInfo::majorVersion();
    ss << (mver/10) << "." << (mver%10) << "." << VersionInfo::minorVersion()
       << " (build #" << setfill('0') << setw(2) << VersionInfo::patch()
       << ")." << setfill(' ');
    const char* info = VersionInfo::info();
    if(info && *info) ss << " - " << info;
    ss << (useHTML ? "<br />" : "\n             \t");
    ss << "Built on " << VersionInfo::date() << ".";
    ss << (useHTML ? "</td></tr><tr><td>" : "\n");
    */
    
    ss << "Qt Version:";
    ss << (useHTML ? "</td><td>" : "\t");
    ss << qVersion();
    ss << (useHTML ? "</td></tr></table>" : "\n");
    
    // Links
    ss << (useHTML ? "<hr />" : "\n");
    if(useHTML) ss << "<b>";
    ss << "Links:";
    ss << (useHTML ? "</b><ul style=\"margin-top: 0px\"><li>" : "\n\t* ");
    ss << "CASA Homepage: ";
    if(useHTML) ss << "<a href=\"http://casa.nrao.edu/\">";
    ss << "http://casa.nrao.edu/";
    ss << (useHTML ? "</a></li><li>" : "\n\t* ");
    ss << PlotterImplementations::implementationLink(impl, useHTML);
    ss << (useHTML ? "</li><li>" : "\n\t* ");
    ss << "Trolltech (Qt) Homepage: ";
    if(useHTML) ss << "<a href=\"http://trolltech.com/qt/\">";
    ss << "http://trolltech.com/qt/";
    if(useHTML) ss << "</li></ul>";
    
    return ss.str();
}

}
