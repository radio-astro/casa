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
#include <plotms/Gui/PlotMSPlotter.qo.h>

#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <casaqt/QtUtilities/QtActionGroup.qo.h>
#include <casaqt/QtUtilities/QtProgressWidget.qo.h>
#include <plotms/Actions/ActionFactory.h>
#include <plotms/Actions/ActionExport.h>
#include <plotms/Threads/Gui/PlotMSCacheThread.qo.h>
#include <plotms/Threads/Gui/PlotMSDrawThread.qo.h>
#include <plotms/Threads/Gui/PlotMSExportThread.qo.h>
#include <plotms/GuiTabs/PlotMSAnnotatorTab.qo.h>
#include <plotms/GuiTabs/PlotMSFlaggingTab.qo.h>
#include <plotms/GuiTabs/PlotMSOptionsTab.qo.h>
#include <plotms/GuiTabs/PlotMSExportTab.qo.h>
#include <plotms/GuiTabs/PlotMSDataSummaryTab.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/GuiTabs/PlotMSToolsTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <QCloseEvent>
#include <QDockWidget>
#include <QMessageBox>
#include <QProcess>
#include <QSet>
#include <QSplitter>
#include <QThread>
#include <QDebug>


namespace casa {

///////////////////////////////
// PLOTMSPLOTTER DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

PlotMSPlotter::PlotMSPlotter(PlotMSApp* parent, Plotter::Implementation impl) :
        itsParent_(parent), itsAnnotator_(this)
        {
	parent->getPlotManager().addWatcher(&itsAnnotator_);
    initialize(impl);
}

PlotMSPlotter::~PlotMSPlotter() {
    if(!isQt_) {
        // have to manually delete generic plotter stuff
        delete &*itsPlotter_;
    }
}


// Public Methods //



void PlotMSPlotter::canvasAdded( PlotCanvasPtr& canvas ){

	// connect new canvases' tracker to tool tab
	PlotStandardMouseToolGroupPtr tools = canvas->standardMouseTools();
	tools->trackerTool()->addNotifier(itsToolsTab_);
	tools->selectTool()->setDrawRects(true);

	// register annotator tool
	canvas->registerMouseTool(
			PlotMouseToolPtr(&itsAnnotator_, false),
			false, true);

	// add plotmsplotter as draw watcher
	canvas->registerDrawWatcher(PlotDrawWatcherPtr(this,false));

	// watch for keystrokes related to using tracker feature
	canvas->registerKeyHandler(
			PlotKeyEventHandlerPtr(itsToolsTab_->tracker_key_handler,
					false));


}

void PlotMSPlotter::showGUI(bool show) {
    isClosed_ = false;
    setVisible(show);
    itsPlotter_->showGUI(show);
}

bool PlotMSPlotter::guiShown() const {
	return QMainWindow::isVisible();
}

int PlotMSPlotter::execLoop() {
    QApplication::processEvents();
    
    if(isQt_) return itsFactory_->execLoop();
    else {
        itsFactory_->execLoop(); // HOPEFULLY this runs asynchronously
                                 // but shouldn't be a problem as long as we
                                 // keep using Qt
        return qApp->exec();
    }
}

ThreadController* PlotMSPlotter::getThreadController( PlotMSAction::Type type,
		PMSPTMethod postThreadMethod, PlotMSPlot* postThreadObject,
		int index ){
	PlotMSThread* controller = NULL;

	if ( type == PlotMSAction::PLOT_EXPORT ){
		PlotExportFormat format = itsParent_->getExportFormat();
		controller = new PlotMSExportThread( itsThreadProgress_, this, format.location );
	}
	else if ( type == PlotMSAction::CACHE_LOAD || type == PlotMSAction::CACHE_RELEASE ){
		if ( postThreadObject == NULL ){
			vector<PlotMSPlot*> plots = getCurrentPlots();
			int plotCount = plots.size();
			if ( 0 <= index && index < plotCount ){
				PlotMSPlotParameters params = plots[index]->parameters();
				PMS_PP_Cache* paramsCache = params.typedGroup<PMS_PP_Cache>();
				controller = new PlotMSCacheThread(itsThreadProgress_,this,
						&PMS_PP_Cache::notifyWatchers, paramsCache);
			}
		}
		else {
			controller = new PlotMSCacheThread( itsThreadProgress_, this, postThreadMethod,
					postThreadObject );
		}
	}
	else if ( type == PlotMSAction::HOLD_RELEASE_DRAWING ){
		controller = new PlotMSDrawThread( this, itsThreadProgress_);
	}
	else {
		qDebug() << "PlotMSPlotter::getThreadController Unsupported threaded operation: "<< type;
	}
	return controller;
}

void PlotMSPlotter::setAnnotationModeActive( PlotMSAction::Type type, bool active ){
	if ( active ){
		PlotMSAnnotator::Mode annotateMode = PlotMSAnnotator::TEXT;
		if ( type == PlotMSAction::TOOL_ANNOTATE_RECTANGLE ){
			annotateMode = PlotMSAnnotator::RECTANGLE;
		}
		getAnnotator().setDrawingMode(annotateMode);
	}
	getAnnotator().setActive(active);
}

void PlotMSPlotter::doThreadedOperation( ThreadController* controller) {
    if(controller == NULL){
    	return;
    }
    PlotMSThread* t = dynamic_cast<PlotMSThread*>(controller );
    if(itsCurrentThread_ == NULL) {
        // no currently running thread, so start this one
        itsCurrentThread_ = t;
        connect(itsCurrentThread_, SIGNAL(finishedOperation(PlotMSThread*)),
                SLOT(currentThreadFinished()));
        
        // disable and show progress GUI
        foreach(QWidget* widget, itsEnableWidgets_) widget->setEnabled(false);
        QRect rect = itsThreadProgress_->geometry();
        rect.moveCenter(geometry().center());
        itsThreadProgress_->move(rect.topLeft());
        itsThreadProgress_->setVisible(true);
        itsThreadProgress_->setEnabled(true);
        itsCurrentThread_->startOperation();
    } else {
        // there is a currently running thread, so add to waiting list
        itsWaitingThreads_.push_back( t );
    }
}

bool PlotMSPlotter::isDrawing() const {
	return itsCurrentThread_ != NULL && itsCurrentThread_->thread()->isRunning();
}

bool PlotMSPlotter::isClosed() const {
	return isClosed_;
}

bool PlotMSPlotter::canvasDrawBeginning(
	PlotOperationPtr drawOperation,
    bool drawingIsThreaded, int drawnLayersFlag
) {

	(void)drawOperation,(void)drawnLayersFlag;
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
            dt = new PlotMSDrawThread(this, itsThreadProgress_);
            itsWaitingThreads_.push_back(dt);
        }
        return true;
    }

    dt = new PlotMSDrawThread(this, itsThreadProgress_ );
    doThreadedOperation(dt);
    return true;
}


void PlotMSPlotter::showIterationButtons(bool show) {
    if(!show && actionIterationToolbar->isChecked()) {
        actionIterationToolbar->setChecked(false);
    }
    actionIterationToolbar->setEnabled(show);
    iterationToolBar->setEnabled(show);
    itsToolsTab_->showIterationButtons(show);
}

bool PlotMSPlotter::showQuestion(const String& message, const String& title) {
    return QMessageBox::question(
    	this, title.c_str(), message.c_str(),
           QMessageBox::Yes | QMessageBox::No
        ) == QMessageBox::Yes;
}

void PlotMSPlotter::setWindowTitle(const String& windowTitle) {
    QMainWindow::setWindowTitle(QString(windowTitle.c_str()));
}

void PlotMSPlotter::setStatusText(const String& statusText) {
    if(statusText.empty()) {
    	statusBar()->clearMessage();
    }
    else {
    	statusBar()->showMessage(statusText.c_str());
    }
}

void PlotMSPlotter::setToolButtonStyle(Qt::ToolButtonStyle style) {
    QMainWindow::setToolButtonStyle(style);
    for(int i = 0; i < itsToolButtons_.size(); i++) {
        itsToolButtons_[i]->setToolButtonStyle(style);
    }
}


const QMap<PlotMSAction::Type, QAction*>& PlotMSPlotter::plotActionMap() const {
    return itsActionMap_;
}

void PlotMSPlotter::synchronizeAction(
	PlotMSAction::Type action, QAbstractButton* button
) {
    itsActionSynchronizer_.synchronize(itsActionMap_.value(action), button);
}

String PlotMSPlotter::actionText(PlotMSAction::Type type) {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) {
    	return action->text().toStdString();
    }
    else {
    	return "";
    }
}

void PlotMSPlotter::setActionText(PlotMSAction::Type type, const String& text) {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) {
    	action->setText(text.c_str());
    }
}

bool PlotMSPlotter::actionIsChecked(PlotMSAction::Type type) const {
    QAction* action = itsActionMap_.value(type);
    return (action != NULL && action->isChecked());
}

void PlotMSPlotter::setActionIsChecked(
	PlotMSAction::Type type, bool checked, bool alsoTriggerAction
) {
    QAction* action = itsActionMap_.value(type);
    if(action != NULL) {
    	if(action->isCheckable()) {
    		action->setChecked(checked);
    	}
    	if(alsoTriggerAction) {
    		action->trigger();
    	}
    }
}


// Public Slots //

void PlotMSPlotter::showError(
	const String& message, const String& title, bool isWarning
)  {
	if (itsParent_->its_want_avoid_popups)   {
		// We are likely running from an automated python script, or otherwise
		// don't want to create any blocking popups.
		// Call upon showMessage to do the right thing.
		showMessage( String( (isWarning)? "WARNING: " : "ERROR: ")  +  message , title);

	 } else {
	
	    if(isWarning) {
	    	QMessageBox::warning( this, title.c_str(), message.c_str());
	    }
	    else {
	    	QMessageBox::critical(this, title.c_str(), message.c_str());
	    }
	 }
}



void PlotMSPlotter::showMessage(const String& message, const String& title) {
	if (itsParent_->its_want_avoid_popups)  {

		// Update status bar 
		// (Ignore the title)
		statusbar->showMessage( QString::fromStdString( message ));
		
		// Also write msg to logger
		// (not entirely sure how this is supposed to be done)
		getParent()->getLogger()->postMessage(PMS::LOG_ORIGIN, String("showError()"), 
			message);
		
		#if (0)   // (DSW) stolen from some .h for reference - do not use this code (i think, maybe)
		void postMessage(const PlotLogMessage& message);
		void postMessage(const String& origin1, const String& origin2,
                     const String& message,
                     int eventType = PlotLogMessage::DEFAULT_EVENT_TYPE);
		#endif
 
	
	} else {
		
		QMessageBox::information(this, title.c_str(), message.c_str());
 
	}
}



void PlotMSPlotter::showAbout() {
    QMessageBox::about(this, "About PlotMS", itsAboutString_);
}





void PlotMSPlotter::prepareForPlotting()   {
	
	//PlotMSToolsTab *ttab =  getToolsTab();
	
	// This is a fix for the zoom stack bug, JIRA CAS-1770.
	// The fix is to force all tool buttons unpressed
	// before doing a reload/redraw of the plot.
	// toolsUnchecked() has a slightly misleading name.  It causes
	// all the tool buttons - Zoom, Pan etc to become unchecked by
	// checking the "None" radio button in the tools tab.
	// These radio buttons are wired to the tool bar, causing them
	// to also become unchecked.
	itsToolsTab_->toolsUnchecked();
}




// Protected Methods //

void PlotMSPlotter::closeEvent(QCloseEvent* event) {
	
    if(itsCurrentThread_ != NULL) {

		// Normally in interactive mode, verify quitting with user.
		// If avoiding gui popups, typically due to casaplotms being run
		// by automated python scripts,  assume answer of "yes, i want to quit"
        if ( ! itsParent_->its_want_avoid_popups)   {
		    bool reply = showQuestion("One or more threaded operations are not yet "
                "complete!  Do you still want to quit?", "PlotMS Quit");
                
			if (reply==false)   {
				event->ignore();
				return;
			}
		}
        
        for(unsigned int i = 0; i < itsWaitingThreads_.size(); i++)
            delete itsWaitingThreads_[i];
        itsWaitingThreads_.clear();
        itsCurrentThread_->cancel();
    }
    isClosed_ = true;
    // Close plotter separately if not Qt-based.
    if(!isQt_) itsPlotter_->close();
}



// Private Methods //

void PlotMSPlotter::initialize(Plotter::Implementation imp) {
    // GUI initialize.
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
    
    // Set various properties/members.
    setAnimated(false);
    itsAboutString_ = aboutText(itsFactory_->implementation(), true).c_str();
    
    // Set up main splitter for tabs/plotter.
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setOpaqueResize(false);
    setCentralWidget(splitter);
    
    QTabWidget* tabWidget = new QTabWidget();
    tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    splitter->addWidget(tabWidget);
    
    //Initialize menu actions.
    connect(menuSummary, SIGNAL(triggered()), this, SLOT(summarize()));
    connect(menuExport, SIGNAL(triggered()), this, SLOT(exportPlots()));

    // Get list of widgets to enable/disable.
    if(menuWidget() != NULL) itsEnableWidgets_ << menuWidget();
    QList<QToolBar*> toolbars = findChildren<QToolBar*>();
    for(int i = 0; i < toolbars.size(); i++) itsEnableWidgets_ << toolbars[i];
    QList<QDockWidget*> docks = findChildren<QDockWidget*>();
    for(int i = 0; i < docks.size(); i++) itsEnableWidgets_ << docks[i];
    itsEnableWidgets_ << splitter;
    
    // Set up action map.
    itsActionMap_.insert(PlotMSAction::SEL_FLAG, actionFlag);
    itsActionMap_.insert(PlotMSAction::SEL_UNFLAG, actionUnflag);
    itsActionMap_.insert(PlotMSAction::SEL_LOCATE, actionLocate);
    itsActionMap_.insert(PlotMSAction::SEL_CLEAR_REGIONS, actionClearRegions);
    
    itsActionMap_.insert(PlotMSAction::ITER_FIRST, actionIterFirst);
    itsActionMap_.insert(PlotMSAction::ITER_PREV, actionIterPrev);
    itsActionMap_.insert(PlotMSAction::ITER_NEXT, actionIterNext);
    itsActionMap_.insert(PlotMSAction::ITER_LAST, actionIterLast);
    
    itsActionMap_.insert(PlotMSAction::TOOL_MARK_REGIONS, actionMarkRegions);
    itsActionMap_.insert(PlotMSAction::TOOL_SUBTRACT_REGIONS, actionMinusRegions);
    itsActionMap_.insert(PlotMSAction::TOOL_ZOOM, actionZoom);
    itsActionMap_.insert(PlotMSAction::TOOL_PAN, actionPan);
    itsActionMap_.insert(PlotMSAction::TOOL_ANNOTATE_TEXT, actionAnnotateText);
    itsActionMap_.insert(PlotMSAction::TOOL_ANNOTATE_RECTANGLE,
                         actionAnnotateRectangle);
    
    itsActionMap_.insert(PlotMSAction::TRACKER_ENABLE_HOVER, actionTrackerHover);
    itsActionMap_.insert(PlotMSAction::TRACKER_ENABLE_DISPLAY, actionTrackerDisplay);
    
    itsActionMap_.insert(PlotMSAction::STACK_BACK, actionStackBack);
    itsActionMap_.insert(PlotMSAction::STACK_BASE, actionStackBase);
    itsActionMap_.insert(PlotMSAction::STACK_FORWARD, actionStackForward);
    
    itsActionMap_.insert(PlotMSAction::CACHE_LOAD, actionCacheLoad);
    itsActionMap_.insert(PlotMSAction::CACHE_RELEASE, actionCacheRelease);
    
    itsActionMap_.insert(PlotMSAction::MS_SUMMARY, actionMSSummary);
    itsActionMap_.insert(PlotMSAction::PLOT, actionPlot);
    itsActionMap_.insert(PlotMSAction::PLOT_EXPORT, actionPlotExport);
    
    itsActionMap_.insert(PlotMSAction::HOLD_RELEASE_DRAWING, actionHoldRelease);
    itsActionMap_.insert(PlotMSAction::CLEAR_PLOTTER, actionClearPlots);
    itsActionMap_.insert(PlotMSAction::QUIT, actionQuit);

    
    // Set up annotator.
    QMenu* annotatorMenu = new QMenu();
    annotatorMenu->addAction(actionAnnotateText);
    annotatorMenu->addAction(actionAnnotateRectangle);
    actionAnnotate->setMenu(annotatorMenu);
    itsAnnotator_.setActions(actionAnnotate, itsActionMap_, itsFactory_);
    
    // Set up tabs.
    //itsPlotTab_ = new PlotMSPlotTab(this);
    itsPlotTab_ = new PlotMSDataSummaryTab( this );
    itsFlaggingTab_ = new PlotMSFlaggingTab(this);
    itsToolsTab_ = new PlotMSToolsTab(this);
    itsAnnotatorTab_ = new PlotMSAnnotatorTab(this);
    itsOptionsTab_ = new PlotMSOptionsTab(this);
   
    itsToolButtons_ << itsPlotTab_->toolButtons();
    itsToolButtons_ << itsFlaggingTab_->toolButtons();
    itsToolButtons_ << itsToolsTab_->toolButtons();
    itsToolButtons_ << itsAnnotatorTab_->toolButtons();
    itsToolButtons_ << itsOptionsTab_->toolButtons();
    
    int maxWidth = itsPlotTab_->maximumWidth();
    if(itsToolsTab_->maximumWidth() < maxWidth)
        maxWidth = itsToolsTab_->maximumWidth();
    if(itsAnnotatorTab_->maximumWidth() < maxWidth)
        maxWidth = itsAnnotatorTab_->maximumWidth();
    if(itsOptionsTab_->maximumWidth() < maxWidth)
        maxWidth = itsOptionsTab_->maximumWidth();

    //
    // Put in to avoid display issues on the mack
    //
    tabWidget->setMaximumWidth(325);
    itsPlotTab_->setupForMaxWidth(maxWidth);
    itsFlaggingTab_->setupForMaxWidth(maxWidth);
    itsToolsTab_->setupForMaxWidth(maxWidth);
    itsAnnotatorTab_->setupForMaxWidth(maxWidth);
    itsOptionsTab_->setupForMaxWidth(maxWidth);
    

    tabWidget->addTab(itsPlotTab_, itsPlotTab_->tabName());
    tabWidget->addTab(itsFlaggingTab_, itsFlaggingTab_->tabName());
    tabWidget->addTab(itsToolsTab_, itsToolsTab_->tabName());
    tabWidget->addTab(itsAnnotatorTab_, itsAnnotatorTab_->tabName());
    tabWidget->addTab(itsOptionsTab_, itsOptionsTab_->tabName());
    
    // Set up threads.
    itsCurrentThread_ = NULL;
    itsThreadProgress_ = new QtProgressWidget(false, false, false, true, false,
                                              this);
    itsThreadProgress_->setVisible(false);
    itsThreadProgress_->setWindowIcon(windowIcon());
    
    // Insert annotator tool button after pan.
    QList<QAction*> tools = toolsToolBar->actions();
    for(int i = 0; i < tools.size(); i++) {
        if(tools[i] == actionPan) {
            if(i < tools.size() - 1)
                toolsToolBar->insertAction(tools[i + 1], actionAnnotate);
            else toolsToolBar->addAction(actionAnnotate);
            break;
        }
    }
    
    // Set up exclusive tool actions.
    QtActionGroup* toolGroup = new QtActionGroup(this);
    toolGroup->addAction(actionMarkRegions);
    toolGroup->addAction(actionMinusRegions);
    toolGroup->addAction(actionZoom);
    toolGroup->addAction(actionPan);
    toolGroup->addAction(actionAnnotateText);
    toolGroup->addAction(actionAnnotateRectangle);
    connect(toolGroup, SIGNAL(triggered(QAction*)), SLOT(action(QAction*)));
    connect(toolGroup, SIGNAL(unchecked()),
            itsToolsTab_, SLOT(toolsUnchecked()));
    
    // Set up non-exclusive actions.
    QSet<QAction*> actions;
    actions << actionFlag << actionUnflag << actionLocate << actionClearRegions
            << actionIterFirst << actionIterPrev << actionIterNext
            << actionIterLast << actionAnnotate << actionTrackerHover
            << actionTrackerDisplay << actionStackBack << actionStackBase
            << actionStackForward << actionCacheLoad << actionCacheRelease
            << actionMSSummary << actionPlot << actionPlotExport
            << actionHoldRelease << actionClearPlots <<
            actionQuit;
    foreach(QAction* a, actions)
        connect(a, SIGNAL(triggered()), SLOT(action_()));
    
    // Connect remaining actions.
    connect(actionAbout, SIGNAL(triggered()), SLOT(showAbout()));
    connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    // Set up plotter.
    itsPlotter_ = itsFactory_->plotter("PlotMS", false, false,
            itsParent_->getParameters().logEvents(), false);
    
    // If Qt, put in window.  Otherwise, just hope that it does something
    // sensible.
    isQt_ = imp == Plotter::QWT && itsPlotter_->isQWidget();
    if(isQt_) {
        QWidget* w = dynamic_cast<QWidget*>(itsPlotter_.operator->());
        w->setContentsMargins(0, 0, 0, 0);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter->addWidget(w);
        itsEnableWidgets_ << w;
    }

    isClosed_ = false;
    
    // Force window to set size before drawing to avoid unnecessary redraws.
    QApplication::processEvents();
    
    setMinimumHeight(600);
    resize(width(), 600);
    setVisible(false);
    showIterationButtons( true );
}


// Private Slots //

void PlotMSPlotter::action(QAction* act) {
    if(act == NULL) return;
    setStatusText(" " );//
    clearStatusText();
    
    // If it's the annotate button, turn on/off the action corresponding to the
    // current mode.
    if(act == actionAnnotate) {
        PlotMSAnnotator::Mode m = itsAnnotator_.drawingMode();
        if(m == PlotMSAnnotator::TEXT)
            actionAnnotateText->setChecked(act->isChecked());
        else if(m == PlotMSAnnotator::RECTANGLE)
            actionAnnotateRectangle->setChecked(act->isChecked());
        return;
        
    // If it's one of the specific annotate mode actions, update the general
    // annotate button.
    } else if(act == actionAnnotateText || act == actionAnnotateRectangle) {
        actionAnnotate->setChecked(actionAnnotateText->isChecked() ||
                actionAnnotateRectangle->isChecked());
    }
    
    
    // Set up the generic PlotMS action mapped to the QAction.
    PlotMSAction::Type type = itsActionMap_.key(act);

    // Trigger the action.
    _triggerAction(type);

}

bool PlotMSPlotter::_triggerAction(PlotMSAction::Type type) {
	CountedPtr<PlotMSAction> action = ActionFactory::getAction( type, this);
	bool result = action->doAction(itsParent_);
	if ( !result ){
		showError( action->doActionResult(), "Action Failed!", false );
	}
	return result;
}


bool PlotMSPlotter::exportPlot( const PlotExportFormat& format, const bool async) {
	(void)async;
	ActionExport action( this );
	action.setExportFormat( format );
	action.setInteractive( true );
	bool result = action.doAction(itsParent_);
	if ( !result ){
		showError( action.doActionResult(), "Export Failed!", false );
	}
	return result;
}

void PlotMSPlotter::currentThreadFinished() {
    // Run post-thread method as needed.
	if ( itsCurrentThread_!= NULL && ! itsCurrentThread_->wasCanceled() ){
		itsCurrentThread_->postThreadMethod();
	}

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
        foreach(QWidget* widget, itsEnableWidgets_) widget->setEnabled(true);
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

bool PlotMSPlotter::isActionEnabled( PlotMSAction::Type type ) const {
	QAction* guiAction = itsActionMap_[type];
	return guiAction->isChecked();
}

vector<PlotMSPlot*> PlotMSPlotter::getCurrentPlots() const {
	return itsPlotTab_->getCurrentPlots();
}

vector<vector<PMS::Axis> > PlotMSPlotter::getSelectedLoadAxes() const {
	return itsPlotTab_->selectedLoadAxes();
}

vector<vector<PMS::Axis> >PlotMSPlotter::getSelectedReleaseAxes() const {
	return itsPlotTab_->selectedReleaseAxes();
}



PlotMSFlagging PlotMSPlotter::getFlagging() const {
	return itsFlaggingTab_->getValue();
}

void PlotMSPlotter::setFlagging(PlotMSFlagging flag){
	itsFlaggingTab_->setValue(flag);
}

bool PlotMSPlotter::isInteractive() const {
	return true;
}

void PlotMSPlotter::summarize(){
	CountedPtr<PlotMSAction> summaryAction = ActionFactory::getAction(PlotMSAction::SUMMARY_DIALOG, this);
	summaryAction->doAction( itsParent_ );
}

void PlotMSPlotter::exportPlots(){
	CountedPtr<PlotMSAction> exportAction = ActionFactory::getAction(PlotMSAction::EXPORT_DIALOG, this);
	exportAction->doAction( itsParent_ );
}

void PlotMSPlotter::plot(){
	itsPlotTab_->plot();
}

void PlotMSPlotter::gridSizeChanged( int rowCount, int colCount ){
	itsPlotTab_->setGridSize(rowCount, colCount);
}


}
