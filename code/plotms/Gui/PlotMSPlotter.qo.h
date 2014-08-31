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

#include <casaqt/QtUtilities/QtActionGroup.qo.h>
#include <graphics/GenericPlotter/PlotFactory.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSAnnotator.h>
#include <plotms/Client/Client.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <QMainWindow>
#include <QToolButton>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QtProgressWidget;
class PlotMSApp;
class PlotMSAnnotatorTab;
class PlotMSFlaggingTab;
class PlotMSOptionsTab;
class PlotMSPlotTab;
class PlotMSExportTab;
class PlotMSThread;
class PlotMSToolsTab;
class PlotMSDataSummaryTab;


// High(ish)-level plotter class that manages the GUI (semi-) transparently to
// the rest of PlotMS.
class PlotMSPlotter : public QMainWindow, Ui::PlotterWindow,
                      public PlotDrawWatcher, public Client {
    Q_OBJECT
    
public:

    //Methods from the client interface
    virtual bool isActionEnabled( PlotMSAction::Type type ) const;

    virtual vector<PlotMSPlot*> getCurrentPlots() const;
    virtual void plot();
    virtual vector<vector<PMS::Axis> > getSelectedLoadAxes() const;
    virtual vector<vector<PMS::Axis> > getSelectedReleaseAxes() const;
    virtual PlotMSFlagging getFlagging() const;
    virtual bool isInteractive() const;
	virtual void canvasAdded( PlotCanvasPtr& canvas );
	virtual void setAnnotationModeActive( PlotMSAction::Type type, bool active );
	virtual vector<String> getFiles() const;


	// Static //
    
    // Returns "about" text for the given implementation, using HTML or not.
    static String aboutText(Plotter::Implementation implementation,
                            bool useHTML = true);
    
    
    // Non-Static //
    
    // Constructor that creates a plotter with the given parent using the given
    // implementation.
    PlotMSPlotter(PlotMSApp* parent,
                  Plotter::Implementation impl = Plotter::DEFAULT);
    
    // Destructor.
    ~PlotMSPlotter();
    
    
    // Accessor methods.
    // <group>
    PlotMSApp* getParent() { return itsParent_; }

    QtProgressWidget* getProgressWidget() { return itsThreadProgress_; }

    PlotMSAnnotator& getAnnotator() { return itsAnnotator_; }
    // </group>

    virtual bool isDrawing() const;
    virtual bool isClosed() const;

    // Execution Methods //
    void setShowProgress( bool showProgressDialog);
    // Shows/hides the GUI.
    virtual void showGUI(bool show = true);
    
    // Returns true if the GUI is currently visible, false otherwise.
    virtual bool guiShown() const;
    
    // Enters the plotter's execution loop, and returns its return value.
    // Only during this execution loops will GUI windows be shown and
    // GUI events be handled.  The execution loop ends when the user clicks
    // the "close" or "quit" buttons.  This method can be called multiple
    // times.
    virtual int execLoop();
    

    
    // Runs the given operation thread, keeping GUI and progress information
    // synchronized as necessary.  The given thread will be deleted upon
    // completion.
    virtual void doThreadedOperation(/*PlotMSThread**/ThreadController* thread);
    
    // Implements PlotDrawWatcher::canvasDrawBeginning().
    bool canvasDrawBeginning(PlotOperationPtr drawOperation,
                bool drawingIsThreaded, int drawnLayersFlag);
    virtual ThreadController* getThreadController( PlotMSAction::Type type,
    		PMSPTMethod postThreadMethod = NULL, PlotMSPlot* plot = NULL,
    		int index = -1 );
    
    // GUI Methods //
    
    // Shows/Hides the "iteration" widgets and actions on the GUI.
    void showIterationButtons(bool show);
    
    // Pops up a dialog to ask the user the given question with the given
    // window title.  Returns true if the user says "Yes"; false for "No" or
    // otherwise rejecting the dialog.
    bool showQuestion(const String& message, const String& title);
    


    
    
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
    
    // Returns a map between PlotMSApp actions and the QActions associated with
    // them in the GUI.  Triggering the QActions will trigger the proper PlotMSApp
    // action, and the QAction will be kept properly checked as needed.
    const QMap<PlotMSAction::Type, QAction*>& plotActionMap() const;
    
    // Synchronizes the given button with the given action type.  (See
    // QtActionSynchronizer class.)
    void synchronizeAction(PlotMSAction::Type action, QAbstractButton* button);
    
    // Gets/Sets the text for the QAction associated with the given PlotMSApp
    // action.
    // <group>
    String actionText(PlotMSAction::Type type);
    void setActionText(PlotMSAction::Type type, const String& text);
    // </group>
    
    // Gets/Sets whether the QAction associated with the given PlotMSApp action
    // is checked.  Has no effect on actions that are not checkable.
    // <group>
    bool actionIsChecked(PlotMSAction::Type type) const;
    void setActionIsChecked(PlotMSAction::Type type, bool checked,
            bool alsoTriggerAction = false);
    // </group>
    
    // export a plot to a file
    virtual bool exportPlot(const PlotExportFormat& format, const bool async);
    virtual void setFlagging(PlotMSFlagging flag);
    virtual void gridSizeChanged( int rowCount, int colCount );

public slots:
    // Shows the given error/warning message in a GUI window.
    virtual void showError(const String& message, const String& title, bool isWarning);
    
    // Shows the given informational message in a GUI window.
    virtual void showMessage(const String& message, const String& title, bool warning = false);
    
    // Slot for showing an "about" dialog.
    void showAbout();
    
    // Take any clean-up actions before a Plot is made
    // 1) unclick any zoom, pan, etc buttons to avoid bug in zoom stack
    // 2) ... anything else needed in the future ...
    void prepareForPlotting();
    
    virtual bool close(){
    	return QMainWindow::close();
    }

    
protected:
    // Overrides QWidget::closeEvent(), in case we're dealing with a plotter
    // that isn't Qt and thus is in its own window (and possibly its own
    // execution loop).
    void closeEvent(QCloseEvent* event);
    


private:
    // PlotMSApp parent.
    PlotMSApp* itsParent_;
    
    // Flag for whether the underlying Plotter is Qt-based or not.
    bool isQt_;

    // Flag for whether the widget is open or not
    bool isClosed_;
    

    

    // Widgets to be enabled/disabled during threading.
    QList<QWidget*> itsEnableWidgets_;
    
    //Plot tab
    PlotMSDataSummaryTab* itsPlotTab_;

    // Flagging tab (on the plot tab).
    PlotMSFlaggingTab* itsFlaggingTab_;
    
    // Tools tab.
    PlotMSToolsTab* itsToolsTab_;
    
    // Annotator tab.
    PlotMSAnnotatorTab* itsAnnotatorTab_;
    
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
    
    // Map between PlotMSApp actions and QActions.
    QMap<PlotMSAction::Type, QAction*> itsActionMap_;
    
    // Action synchronizer.
    QtActionSynchronizer itsActionSynchronizer_;
    
    // Annotator tool.
    PlotMSAnnotator itsAnnotator_;
    
    // "About" string.
    QString itsAboutString_;
    
    bool showProgressWidget;

    QTabWidget* tabWidget;

    
    bool _triggerAction(/*PlotMSAction& action*/PlotMSAction::Type type);
    
    // Initializes the plotter with the given implementation.  MUST be called
    // from constructors.
    void initialize(Plotter::Implementation impl);
    
private slots:    
    // Method for when an action has been triggered.
    void action_() { action(dynamic_cast<QAction*>(sender())); }
    
    // Method for when the given action has been triggered.
    void action(QAction* which);

    // Slot for when the currently running thread is finished.  Performs
    // cleanup and starts next waiting thread if applicable.
    void currentThreadFinished();

    //Summarize an MS
	void summarize();

	//Export the current plots
	void exportPlots();

	void tabChanged( );
};
typedef CountedPtr<PlotMSPlotter> PlotMSPlotterPtr;

}

#endif /* PLOTMSPLOTTER_QO_H_ */
