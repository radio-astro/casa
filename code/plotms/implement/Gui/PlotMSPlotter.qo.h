//# PlotMSPlotter.qo.h: GUI for plotms.
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
#ifndef PLOTMSPLOTTER_QO_H_
#define PLOTMSPLOTTER_QO_H_

#include <plotms/Gui/PlotMSPlotter.ui.h>

#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Actions/PlotMSThread.qo.h>
#include <plotms/Data/PlotMSData.h>
#include <plotms/GuiTabs/PlotMSTabs.qo.h>
#include <plotms/PlotMS/PlotMSLogger.h>

#include <casaqt/QtUtilities/QtProgressWidget.qo.h>
#include <graphics/GenericPlotter/PlotFactory.h>

#include <QtGui>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMS;


// High(ish)-level plotter class that manages the GUI (semi-) transparently to
// the rest of PlotMS.
class PlotMSPlotter : public QMainWindow, Ui::PlotterWindow,
                      public PlotMSActionParameters, public PlotDrawWatcher {
    Q_OBJECT
    
public:
    // Static //
    
    // Returns "about" text for the given implementation, using HTML or not.
    static String aboutText(Plotter::Implementation implementation,
                            bool useHTML = true);
    
    
    // Non-Static //
    
    // Constructor that creates a plotter with the given parent using the given
    // implementation.
    PlotMSPlotter(PlotMS* parent,
                  Plotter::Implementation impl = Plotter::DEFAULT);
    
    // Destructor.
    ~PlotMSPlotter();
    
    
    // Accessor methods.
    // <group>
    PlotFactoryPtr getFactory() { return itsFactory_; }
    PlotterPtr getPlotter() { return itsPlotter_; }
    QtProgressWidget* getProgressWidget() { return itsThreadProgress_; }
    PlotMSPlotTab* getPlotTab() { return itsPlotTab_; }
    PlotMSToolsTab* getToolsTab() { return itsToolsTab_; }
    PlotMSOptionsTab* getOptionsTab() { return itsOptionsTab_; }
    // </group>
    
    
    // Execution Methods //
    
    // Shows/hides the GUI.
    void showGUI(bool show = true);
    
    // Enters the plotter's execution loop, and returns its return value.
    // Only during this execution loops will GUI windows be shown and
    // GUI events be handled.  The execution loop ends when the user clicks
    // the "close" or "quit" buttons.  This method can be called multiple
    // times.
    int execLoop();
    
    // See showGUI() and execLoop().
    int showAndExec(bool show = true) {
        showGUI(show);
        return execLoop();
    }
    
    // Runs the given operation thread, keeping GUI and progress information
    // synchronized as necessary.  The given thread will be deleted upon
    // completion.
    void doThreadedOperation(PlotMSThread* thread);
    
    // Implements PlotDrawWatcher::canvasDrawBeginning().
    bool canvasDrawBeginning(PlotOperationPtr drawOperation,
                bool drawingIsThreaded, int drawnLayersFlag);
    
    
    // GUI Methods //
    
    // Shows/Hides the "iteration" widgets and actions on the GUI.
    void showIterationButtons(bool show);
    
    // Pops up a dialog to ask the user the given question with the given
    // window title.  Returns true if the user says "Yes"; false for "No" or
    // otherwise rejecting the dialog.
    bool showQuestion(const String& message, const String& title);
    
    // Holds/Releases drawing on all plot canvases.
    // <group>
    void holdDrawing();
    void releaseDrawing();
    // </group>
    
    
    // Plotter Customization Methods //
    
    // Sets the window title to the given.
    void setWindowTitle(const String& windowTitle);
    
    // Sets the status bar text to the given.
    void setStatusText(const String& statusText);
    
    // Clears the status bar text.
    void clearStatusText() { setStatusText(""); }
    
    // Overrides QMainWindow::setToolButtonStyle().
    void setToolButtonStyle(Qt::ToolButtonStyle style);
    
    
    // Action Methods //    
    
    // Returns a map between PlotMS actions and the QActions associated with
    // them in the GUI.  Triggering the QActions will trigger the proper PlotMS
    // action, and the QAction will be kept properly checked as needed.
    const QMap<PlotMSAction::Type, QAction*>& plotActionMap() const;
    
    // Gets/Sets the text for the QAction associated with the given PlotMS
    // action.
    // <group>
    String actionText(PlotMSAction::Type type);
    void setActionText(PlotMSAction::Type type, const String& text);
    // </group>
    
    // Gets/Sets whether the QAction associated with the given PlotMS action
    // is checked.  Has no effect on actions that are not checkable.
    // <group>
    bool actionIsChecked(PlotMSAction::Type type) const;
    void setActionIsChecked(PlotMSAction::Type type, bool checked,
            bool alsoTriggerAction = false);
    // </group>
    
    // Implements PlotMSActionParameters::actionPlot().  Returns the plot tab's
    // currently selected plot.
    PlotMSPlot* actionPlot(PlotMSAction::Type type) const;
    
    // Implements PlotMSActionParameters::actionBool().  See actionIsChecked().
    bool actionBool(PlotMSAction::Type type) const;
    
    // Implements PlotMSActionParameters::actionAxes().  Returns the plot tab's
    // selected axes for the appropriate action type.
    vector<PMS::Axis> actionAxes(PlotMSAction::Type type) const;

    
public slots:
    // Shows the given error/warning message in a GUI window.
    void showError(const String& message, const String& title, bool isWarning);
    
    // Shows the given informational message in a GUI window.
    void showMessage(const String& message, const String& title);
    
    // Slot for showing an "about" dialog.
    void showAbout();
    
protected:
    // Overrides QWidget::closeEvent(), in case we're dealing with a plotter
    // that isn't Qt and thus is in its own window (and possibly its own
    // execution loop).
    void closeEvent(QCloseEvent* event);
    
private:
    // PlotMS parent.
    PlotMS* itsParent_;
    
    // Flag for whether the underlying Plotter is Qt-based or not.
    bool isQt_;
    
    // Plot factory.
    PlotFactoryPtr itsFactory_;
    
    // Plotter.
    PlotterPtr itsPlotter_;
    
    // Plot tab.
    PlotMSPlotTab* itsPlotTab_;
    
    // Tools tab.
    PlotMSToolsTab* itsToolsTab_;
    
    // Options tab.
    PlotMSOptionsTab* itsOptionsTab_;
    
    // Tool buttons on the tabs.
    QList<QToolButton*> itsToolButtons_;
    
    // Widget for displaying thread progress.
    QtProgressWidget* itsThreadProgress_;
    
    // Current thread (or NULL for none).
    PlotMSThread* itsCurrentThread_;
    
    // Waiting threads.
    vector<PlotMSThread*> itsWaitingThreads_;
    
    // Flag for whether triggered QActions should trigger the associated PlotMS
    // action or not.  Should be false when the QAction's checked state is
    // being updated.
    bool itsActionFlag_;
    
    // Map between PlotMS actions and QActions.
    QMap<PlotMSAction::Type, QAction*> itsActionMap_;
    
    // "About" string.
    QString itsAboutString_;
    
    
    // Initializes the plotter with the given implementation.  MUST be called
    // from constructors.
    void initialize(Plotter::Implementation impl);
    
    // Method for when the given action has been triggered.
    void action(QAction* which);
    
private slots:
    // Action-specific slots to map to general action() method.
    // <group>
    void actionFlag_() { action(actionFlag); }
    void actionUnflag_() { action(actionUnflag); }
    void actionLocate_() { action(actionLocate); }
    void actionClearRegions_() { action(actionClearRegions); }
    void actionIterFirst_() { action(actionIterFirst); }
    void actionIterPrev_() { action(actionIterPrev); }
    void actionIterNext_() { action(actionIterNext); }
    void actionIterLast_() { action(actionIterLast); }
    void actionMarkRegions_() { action(actionMarkRegions); }
    void actionZoom_() { action(actionZoom); }
    void actionPan_() { action(actionPan); }
    void actionTrackerHover_() { action(actionTrackerHover); }
    void actionTrackerDisplay_() { action(actionTrackerDisplay); }
    void actionStackBack_() { action(actionStackBack); }
    void actionStackBase_() { action(actionStackBase); }
    void actionStackForward_() { action(actionStackForward); }
    void actionCacheLoad_() { action(actionCacheLoad); }
    void actionCacheRelease_() { action(actionCacheRelease); }
    void actionHoldRelease_() { action(actionHoldRelease); }
    void actionClearPlots_() { action(actionClearPlots); }
    void actionQuit_() { action(actionQuit); }
    // </group>    
    
    // Slot for when the currently running thread is finished.  Performs
    // cleanup and starts next waiting thread if applicable.
    void currentThreadFinished();
};
typedef CountedPtr<PlotMSPlotter> PlotMSPlotterPtr;

}

#endif /* PLOTMSPLOTTER_QO_H_ */
