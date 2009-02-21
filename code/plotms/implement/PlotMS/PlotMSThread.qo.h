//# PlotMSThread.qo.h: Threading classes for PlotMS.
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
#ifndef PLOTMSTHREAD_QO_H_
#define PLOTMSTHREAD_QO_H_

#include <plotms/PlotMS/PlotMSProgress.ui.h>

#include <graphics/GenericPlotter/Plotter.h>

#include <QMutex>
#include <QThread>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMS;
class PlotMSOperationThread;
class PlotMSPlot;
class PlotMSProgress;


// A thread for handling large operations.  These threaded operations use
// three threads total: the main (GUI) thread in PlotMSPlotter, this thread,
// and the actual operation thread.  This thread is used to periodically check
// the progress of the operation thread to report to the GUI thread to update
// the progress GUI accordingly.
class PlotMSThread : public QThread {
    Q_OBJECT
    
public:
    // Static //
    
    // Returns an operation thread for redrawing the canvases associated with
    // the given plotter.
    static PlotMSOperationThread* threadForRedrawingPlotter(PlotterPtr p);
    
    // Returns an operation thread for redrawing the canvases associated with
    // the given plot.
    static PlotMSOperationThread* threadForRedrawingPlot(PlotMSPlot* plot);
    
    
    // Default check time to update the progress GUI in milliseconds.
    static const unsigned int DEFAULT_CHECK_TIME_MSEC;
    
    
    // Non-Static //
    
    // Constructor which takes the thread that runs the actual operation, a
    // progress GUI to keep updated (which should be in the main/ thread), and
    // an optional check time parameter in milliseconds.
    PlotMSThread(PlotMSOperationThread* thread, PlotMSProgress* progress,
            unsigned int checkTimeMSec = DEFAULT_CHECK_TIME_MSEC);
    
    // Destructor.
    ~PlotMSThread();
    
signals:
    // This signal is emitted when the operation progress percentage and/or
    // status changes.  It is connected to the PlotMSProgress object's
    // setProgress slot.
    void progressAndStatusChanged(unsigned int progress,
                                  const String& newStatus);
    
protected:
    // Overrides QThread::run().  Runs the operation thread and keeps the
    // progress widget updated appropriately.
    void run();
    
private:
    // Operation thread.
    PlotMSOperationThread* itsThread_;
    
    // Progress GUI.
    PlotMSProgress* itsProgress_;
    
    // Check time, in milliseconds.
    unsigned int itsCheckTime_;
};


// Operation Thread Classes //

// Abstract class for doing a single threaded operation.
class PlotMSOperationThread : public QThread {
    Q_OBJECT
    
public:
    // Constructor.
    PlotMSOperationThread() { }
    
    // Destructor.
    virtual ~PlotMSOperationThread() { }
    
    // Returns the operation name for this thread.  Does not need to be
    // synchronized.
    virtual String operationName() const = 0;
    
    // Returns the current progress of the thread in the [0 100] range.  Needs
    // to be synchronized.
    virtual unsigned int currentProgress() const = 0;
    
    // Returns the current status of the thread.  Needs to be syncrhonized.
    virtual String currentStatus() const = 0;
    
protected:
    // Forces children to override QThread::run().
    virtual void run() = 0;
};


// Concrete subclass of PlotMSOperationThread used for releasing drawing on
// canvases.
class PlotMSDrawThread : public PlotMSOperationThread {
    Q_OBJECT
    
public:
    // Constructor which takes the canvases on which to release drawing.
    PlotMSDrawThread(const vector<PlotCanvasPtr>& canvases);
    
    // Destructor.
    ~PlotMSDrawThread();
    
    
    // Implements PlotMSOperationThread::operationName().
    String operationName() const { return PlotCanvas::OPERATION_DRAW; }
    
    // Implements PlotMSOperationThread::currentProgress().
    unsigned int currentProgress() const;
    
    // Implements PlotMSOperationThread::currentProgress().
    String currentStatus() const;
    
protected:
    // Implements PlotMSThread::run().  Releases drawing on the canvases.
    void run();
    
private:
    // Canvases.
    vector<PlotCanvasPtr> itsCanvases_;
    
    // Mutex (implemented by PlotMSMutex).
    PlotMutexPtr itsMutex_;
    
    // Current canvas index.
    unsigned int itsIndex_;
    
    // Current operation.
    PlotOperationPtr itsOperation_;
};


// Helper Classes //

// GUI for displaying progress information.  In the future, it will also
// support the "background", "pause", and "cancel" features.
class PlotMSProgress : public QWidget, Ui::ProgressWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the PlotMS parent an optional parent widget.
    PlotMSProgress(PlotMS* plotms, QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSProgress();
    
    
    // Initializes the GUI with the given operation name.  Should be called
    // before the operation starts.
    void initialize(const String& operationName);
    
    // Finalizes the GUI.  Should be called after the operation ends.
    void finalize();
    
public slots:
    // Sets the status to the given.
    void setStatus(const String& status);
    
    // Sets the progress percentage (0 - 100) to the given.
    void setProgress(unsigned int progress);
    
    // Sets the progress percentage (0 - 100) and the status to the given.
    void setProgress(unsigned int progress, const String& status) {
        setProgress(progress);
        setStatus(status);
    }
    
private:
    // Parent.
    PlotMS* itsParent_;
    
private slots:
    // For the "background" button.
    void background();
    
    // For the "pause"/"resume" button.
    void pauseResume();
    
    // For the "cancel" button.
    void cancel();
};


// Implementation of PlotMutex for QThreads using a QMutex.  Note: this is a
// duplication of QPMutex in the QwtPlotter, but this is a small and simple
// class and its re-implementation avoids a dependency on the QwtPlotter.
class PlotMSMutex : public PlotMutex {
public:
    // Constructor.
    PlotMSMutex();
    
    // Destructor.
    ~PlotMSMutex();
    
    
    // Implements PlotMutex::lock().
    void lock();
    
    // Implements PlotMutex::unlock().
    void unlock();
    
private:
    // Mutex.
    QMutex itsMutex_;
};

}

#endif /* PLOTMSTHREAD_QO_H_ */
