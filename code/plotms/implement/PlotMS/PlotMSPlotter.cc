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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#include <plotms/PlotMS/PlotMSPlotter.qo.h>

#include <plotms/PlotMS/PlotMS.h>

#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <casaqt/QtUtilities/QtUtilities.h>

#include <fstream>
#include <limits>

#include <casa/iomanip.h>

namespace casa {

///////////////////////////////
// PLOTMSPLOTTER DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

PlotMSPlotter::PlotMSPlotter(PlotMS* parent, Plotter::Implementation impl) :
        itsParent_(parent) {
    initialize(impl);
}

PlotMSPlotter::~PlotMSPlotter() {
    if(!isQt_) {
        // have to manually delete generic plotter stuff
        delete &*itsPlotter_;
    }
}


// Public Methods //

void PlotMSPlotter::showGUI(bool show) {
    setVisible(show);
    itsPlotter_->showGUI(show);
}

int PlotMSPlotter::execLoop() {
    QCoreApplication::processEvents();
    
    if(isQt_) return itsFactory_->execLoop();
    else {
        itsFactory_->execLoop(); // HOPEFULLY this runs asynchronously
                                 // but shouldn't be a problem as long as we
                                 // keep using Qt
        return qApp->exec();
    }
}

void PlotMSPlotter::doThreadedOperation(PlotMSThread* t) {
    if(t == NULL) return;
    
    if(itsCurrentThread_ == NULL) {
        // no currently running thread, so start this one
        itsCurrentThread_ = t;
        connect(itsCurrentThread_, SIGNAL(finishedOperation(PlotMSThread*)),
                SLOT(currentThreadFinished()));        
        
        // disable and show progress GUI
        setEnabled(false);
        QRect rect = itsThreadProgress_->geometry();
        rect.moveCenter(geometry().center());
        itsThreadProgress_->move(rect.topLeft());
        itsThreadProgress_->setVisible(true);
        itsThreadProgress_->setEnabled(true);
        
        itsCurrentThread_->startOperation();
        
    } else {
        // there is a currently running thread, so add to waiting list
        itsWaitingThreads_.push_back(t);
    }
}

bool PlotMSPlotter::canvasDrawBeginning(PlotOperationPtr drawOperation,
        bool drawingIsThreaded, int drawnLayersFlag) {    
    if(!drawingIsThreaded) {
        cout << "PlotMSPlotter does not currently support threading for "
             << "plotter implementations that do not do threaded drawing "
             << "themselves." << endl;
        return true;
    }
    
    PlotMSDrawThread* dt;
    
    if(itsCurrentThread_ != NULL) {
        dt = dynamic_cast<PlotMSDrawThread*>(itsCurrentThread_);
        if(dt != NULL) {
            // We're already redrawing, so just make sure the thread is
            // up-to-date with all canvases.
            dt->updatePlotterCanvases();
        } else {
            // Not a draw thread, so queue the drawing thread itself (although
            // the actual drawing is still being done in the background).  If
            // the drawing finishes before the current thread, when the drawing
            // thread starts it will immediately exit.
            dt = new PlotMSDrawThread(this);
            itsWaitingThreads_.push_back(dt);
        }
        return true;
    }

    dt = new PlotMSDrawThread(this);
    doThreadedOperation(dt);    
    return true;
}


void PlotMSPlotter::showIterationButtons(bool show) {
    if(!show && actionIterationToolbar->isChecked())
        actionIterationToolbar->setChecked(false);
    actionIterationToolbar->setEnabled(show);
    iterationToolBar->setEnabled(show);
    itsToolsTab_->showIterationButtons(show);
}

bool PlotMSPlotter::showQuestion(const String& message, const String& title) {
    return QMessageBox::question(this, title.c_str(), message.c_str()) ==
           QMessageBox::Yes;
}

void PlotMSPlotter::holdDrawing() {
    vector<PlotCanvasPtr> canvases= itsPlotter_->canvasLayout()->allCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->holdDrawing();
}

void PlotMSPlotter::releaseDrawing() {
    vector<PlotCanvasPtr> canvases= itsPlotter_->canvasLayout()->allCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->releaseDrawing();
}


void PlotMSPlotter::setWindowTitle(const String& windowTitle) {
    QMainWindow::setWindowTitle(QString(windowTitle.c_str()));
}

void PlotMSPlotter::setStatusText(const String& statusText) {
    if(statusText.empty()) statusBar()->clearMessage();
    else                   statusBar()->showMessage(statusText.c_str());
}

void PlotMSPlotter::setToolButtonStyle(Qt::ToolButtonStyle style) {
    QMainWindow::setToolButtonStyle(style);
    for(int i = 0; i < itsToolButtons_.size(); i++)
        itsToolButtons_[i]->setToolButtonStyle(style);
}


const QMap<PlotMSAction::Type, QAction*>& PlotMSPlotter::plotActionMap() const{
    return itsActionMap_; }

String PlotMSPlotter::actionText(PlotMSAction::Type type) {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) return action->text().toStdString();
    else               return "";
}

void PlotMSPlotter::setActionText(PlotMSAction::Type type, const String& text){
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) action->setText(text.c_str());
}

bool PlotMSPlotter::actionIsChecked(PlotMSAction::Type type) const {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) return action->isChecked();
    else               return false;
}

void PlotMSPlotter::setActionIsChecked(PlotMSAction::Type type, bool checked,
        bool alsoTriggerAction) {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL && action->isCheckable()) {
        bool oldaction = itsActionFlag_;
        if(!alsoTriggerAction) itsActionFlag_ = false;
        action->setChecked(checked);
        if(!alsoTriggerAction) itsActionFlag_ = oldaction;
    }
}

PlotMSPlot* PlotMSPlotter::actionPlot(PlotMSAction::Type type) const {
    return itsPlotTab_->currentPlot();
}

bool PlotMSPlotter::actionBool(PlotMSAction::Type type) const {
    return actionIsChecked(type); }

vector<PMS::Axis> PlotMSPlotter::actionAxes(PlotMSAction::Type type) const {
    if(type == PlotMSAction::CACHE_LOAD)
        return itsPlotTab_->selectedLoadAxes();
    else if(type == PlotMSAction::CACHE_RELEASE)
        return itsPlotTab_->selectedReleaseAxes();
    else return vector<PMS::Axis>();
}


// Public Slots //

void PlotMSPlotter::showError(const String& message, const String& title,
        bool isWarning) {
    if(isWarning) QMessageBox::warning( this, title.c_str(), message.c_str());
    else          QMessageBox::critical(this, title.c_str(), message.c_str());
}

void PlotMSPlotter::showMessage(const String& message, const String& title) {
    QMessageBox::information(this, title.c_str(), message.c_str()); }

void PlotMSPlotter::showAbout() {
    QMessageBox::about(this, "About PlotMS", itsAboutString_); }


// Protected Methods //

void PlotMSPlotter::closeEvent(QCloseEvent* event) {
    if(itsCurrentThread_ != NULL) {
        if(!showQuestion("One or more threaded operations are not yet "
                "complete!  Do you still want to quit?", "PlotMS Quit")) {
            event->ignore();
            return;
        }
        
        for(unsigned int i = 0; i < itsWaitingThreads_.size(); i++)
            delete itsWaitingThreads_[i];
        itsWaitingThreads_.clear();
        itsCurrentThread_->terminateOperation();
    }
    
    // Close plotter separately if not Qt-based.
    if(!isQt_) itsPlotter_->close();
}


// Private Methods //

void PlotMSPlotter::initialize(Plotter::Implementation imp) {    
    setupUi(this);
    
    // Try to initialize plotter, and throw error on failure.
    itsFactory_ = plotterImplementation(imp);
    if(itsFactory_.null()) {
        String error = "Invalid plotter implementation.";
        if(imp == Plotter::QWT)
            error += "  Make sure that you have qwt installed and the "
                     "AIPS_HAS_QWT compiler flag turned on!";
        throw AipsError(error);
    }
    
    itsAboutString_ = aboutText(itsFactory_->implementation(), true).c_str();
    itsActionFlag_ = true;
    
    // Qt //
    
    setAnimated(false);
    
    // Action map //    
    itsActionMap_.insert(PlotMSAction::FLAG, actionFlag);
    itsActionMap_.insert(PlotMSAction::UNFLAG, actionUnflag);
    itsActionMap_.insert(PlotMSAction::LOCATE, actionLocate);
    itsActionMap_.insert(PlotMSAction::CLEAR_REGIONS, actionClearRegions);
    
    itsActionMap_.insert(PlotMSAction::ITER_FIRST, actionIterFirst);
    itsActionMap_.insert(PlotMSAction::ITER_PREV, actionIterPrev);
    itsActionMap_.insert(PlotMSAction::ITER_NEXT, actionIterNext);
    itsActionMap_.insert(PlotMSAction::ITER_LAST, actionIterLast);
    
    itsActionMap_.insert(PlotMSAction::TOOL_MARK_REGIONS, actionMarkRegions);
    itsActionMap_.insert(PlotMSAction::TOOL_ZOOM, actionZoom);
    itsActionMap_.insert(PlotMSAction::TOOL_PAN, actionPan);
    
    itsActionMap_.insert(PlotMSAction::TRACKER_HOVER, actionTrackerHover);
    itsActionMap_.insert(PlotMSAction::TRACKER_DISPLAY, actionTrackerDisplay);
    
    itsActionMap_.insert(PlotMSAction::STACK_BACK, actionStackBack);
    itsActionMap_.insert(PlotMSAction::STACK_BASE, actionStackBase);
    itsActionMap_.insert(PlotMSAction::STACK_FORWARD, actionStackForward);
    
    itsActionMap_.insert(PlotMSAction::CACHE_LOAD, actionCacheLoad);
    itsActionMap_.insert(PlotMSAction::CACHE_RELEASE, actionCacheRelease);
    
    itsActionMap_.insert(PlotMSAction::HOLD_RELEASE_DRAWING, actionHoldRelease);    
    itsActionMap_.insert(PlotMSAction::CLEAR_PLOTTER, actionClearPlots);
    itsActionMap_.insert(PlotMSAction::QUIT, actionQuit);
    
    // Set up tabs //    
    itsPlotTab_ = new PlotMSPlotTab(itsParent_, this);
    itsToolsTab_ = new PlotMSToolsTab(itsParent_, this);
    itsOptionsTab_ = new PlotMSOptionsTab(itsParent_, this);
    itsToolButtons_ << itsPlotTab_->toolButtons();
    itsToolButtons_ << itsToolsTab_->toolButtons();
    itsToolButtons_ << itsOptionsTab_->toolButtons();
    
    tabWidget->removeTab(0);
    tabWidget->addTab(itsPlotTab_, "Plots");
    tabWidget->addTab(itsToolsTab_, "Tools");
    tabWidget->addTab(itsOptionsTab_, "Options");
    
    // Set up threads //    
    itsCurrentThread_ = NULL;
    itsThreadProgress_ = new QtProgressWidget(false, false, false, true, this);
    itsThreadProgress_->setVisible(false);
    itsThreadProgress_->setWindowIcon(windowIcon());
    
    // Connect actions //
    connect(actionFlag, SIGNAL(triggered()), SLOT(actionFlag_()));
    connect(actionUnflag, SIGNAL(triggered()), SLOT(actionUnflag_()));
    connect(actionLocate, SIGNAL(triggered()), SLOT(actionLocate_()));
    connect(actionClearRegions, SIGNAL(triggered()),
            SLOT(actionClearRegions_()));
    connect(actionIterFirst, SIGNAL(triggered()), SLOT(actionIterFirst_()));
    connect(actionIterPrev, SIGNAL(triggered()), SLOT(actionIterPrev_()));
    connect(actionIterNext, SIGNAL(triggered()), SLOT(actionIterNext_()));
    connect(actionIterLast, SIGNAL(triggered()), SLOT(actionIterLast_()));
    connect(actionMarkRegions, SIGNAL(toggled(bool)),
            SLOT(actionMarkRegions_()));
    connect(actionZoom, SIGNAL(toggled(bool)), SLOT(actionZoom_()));
    connect(actionPan, SIGNAL(toggled(bool)), SLOT(actionPan_()));
    connect(actionTrackerHover, SIGNAL(toggled(bool)),
            SLOT(actionTrackerHover_()));
    connect(actionTrackerDisplay, SIGNAL(toggled(bool)),
            SLOT(actionTrackerDisplay_()));
    connect(actionStackBack, SIGNAL(triggered()), SLOT(actionStackBack_()));
    connect(actionStackBase, SIGNAL(triggered()), SLOT(actionStackBase_()));
    connect(actionStackForward, SIGNAL(triggered()),
            SLOT(actionStackForward_()));
    connect(actionCacheLoad, SIGNAL(triggered()), SLOT(actionCacheLoad_()));
    connect(actionCacheRelease, SIGNAL(triggered()),
            SLOT(actionCacheRelease_()));
    connect(actionHoldRelease, SIGNAL(toggled(bool)),
            SLOT(actionHoldRelease_()));    
    connect(actionClearPlots, SIGNAL(triggered()), SLOT(actionClearPlots_()));
    connect(actionQuit, SIGNAL(triggered()), SLOT(actionClearPlots_()));

    connect(actionAbout, SIGNAL(triggered()), SLOT(showAbout()));
    connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    
    // Non-Qt //
    
    // Set up plotter
    itsPlotter_ = itsFactory_->plotter("PlotMS", false, false,
            PlotMSLogger::levelToEventFlag(
            itsParent_->getParameters().logLevel()), false);
    
    // If Qt, put in window.  Otherwise, just hope that it does something
    // sensible.
    isQt_ = imp == Plotter::QWT && itsPlotter_->isQWidget();
    if(isQt_) {
        QWidget* w = dynamic_cast<QWidget*>(itsPlotter_.operator->());
        w->setContentsMargins(0, 0, 0, 0);
        QtUtilities::putInFrame(frame, w);
    }
    
    // Force window to set size before drawing to avoid unnecessary redraws.
    QApplication::processEvents();
    setMinimumHeight(600);
    resize(width(), 600);
    setVisible(false);
}

void PlotMSPlotter::action(QAction* act) {
    if(!itsActionFlag_) return;

    if(act == actionMarkRegions || act == actionZoom || act == actionPan) {
        bool mark = actionMarkRegions->isChecked(),
             zoom = actionZoom->isChecked(), pan = actionPan->isChecked();        
        if(mark || zoom || pan) {
            // only perform action on an "on"
            if(!act->isChecked()) return;
            
            itsActionFlag_ = false;
            
            // enforce mutual exclusivity
            actionMarkRegions->setChecked(mark && act == actionMarkRegions);
            actionZoom->setChecked(zoom && act == actionZoom);
            actionPan->setChecked(pan && act == actionPan);
            
            itsActionFlag_ = true;
        }
    }
    
    itsParent_->triggerAction(itsActionMap_.key(act));
}


// Private Slots //

void PlotMSPlotter::currentThreadFinished() {
    // Run post-thread method as needed.
    itsCurrentThread_->postThreadMethod();
    
    // Clean up current thread.
    delete itsCurrentThread_;
    itsCurrentThread_ = NULL;
    
    // Start the next waiting thread, if applicable.
    if(itsWaitingThreads_.size() > 0) {
        itsCurrentThread_ = itsWaitingThreads_[0];
        itsWaitingThreads_.erase(itsWaitingThreads_.begin());
        connect(itsCurrentThread_, SIGNAL(finishedOperation(PlotMSThread*)),
                SLOT(currentThreadFinished()));
        itsCurrentThread_->startOperation();
    } else {
        setEnabled(true);
        itsThreadProgress_->setVisible(false);
        
        // Update plot tab.
        itsPlotTab_->plotsChanged(itsParent_->getPlotManager());
    }
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
    
    // CASA info
    ss << "CASA Version:";
    ss << (useHTML ? "</td><td>" : "\t");
    
    // Append the result of running "avers"
    QProcess* avers = new QProcess();
    avers->start("avers");
    avers->waitForFinished();
    ss << QString(avers->readAllStandardOutput()).replace(
          '\n', "").toStdString();
    ss << (useHTML ? "<br />" : "\n             \t");
    avers->start("avers", QStringList() << "-b");
    avers->waitForFinished();
    ss << "(" << QString(avers->readAllStandardOutput()).replace(
          '\n', "").toStdString() << ")";
    ss << (useHTML ? "</td></tr><tr><td>" : "\n");
    
    // Qt info
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
