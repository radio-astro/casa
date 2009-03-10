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

#include <graphics/GenericPlotter/PlotCanvas.h>
#include <msvis/MSVis/VisSet.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotWidgets.qo.h>

#include <QThread>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMSData;
class PlotMSPlot;
class PlotMSPlotter;


// Useful macro for using post-thread methods.
#define PMS_POST_THREAD_METHOD(CLASS, METHOD)                                 \
public:                                                                       \
    static void METHOD (void* obj) {                                          \
        CLASS * cobj = static_cast< CLASS *>(obj);                            \
        if(cobj != NULL) cobj -> METHOD##_();                                 \
    }                                                                         \
                                                                              \
private:                                                                      \
    void METHOD##_();

// Typedefs for using post-thread methods.
// <group>
typedef void* PMSPTObject;
typedef void PMSPTMethod(PMSPTObject);
// </grooup>


// Abstract class for a threaded operation for plotms.  PlotMSThread does not
// directly inherit from QThread in case the threading is happening elsewhere
// in the code (such as a plotter implementation that threads its own drawing).
// Classes that want to run PlotMSThreads should:
// 1) Provide a PlotProgressWidget in the main (GUI) thread.
// 2) Connect the finishedOperation() signal to a slot as needed.
// 3) Call startOperation() to start the thread.
// 4) Call the post-thread method after completion, if there is one.
class PlotMSThread : public virtual QObject {
    Q_OBJECT
    
public:
    // Constructor which takes the progress widget to use, and an optional
    // post-thread method that should be called when the thread is finished.
    PlotMSThread(PlotProgressWidget* progress,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);
    
    // Destructor.
    virtual ~PlotMSThread();
    
    
    // ABSTRACT METHODS //
    
    // Abstract method which does the operation.  IMPORTANT: subclasses MUST
    // emit the finished() signal when finished.
    virtual void startOperation() = 0;
    
    
    // IMPLEMENTED METHODS //
    
    // Method which terminates the operation regardless of whether it has
    // finished or not.
    virtual void terminateOperation() { cancel(); }
      
    // Executes the post-thread method as needed.  Does nothing if a
    // post-thread method was not set in the constructor.
    virtual void postThreadMethod();
    
signals:
    // This signal MUST be emitted after start() has been called, and when the
    // operation has finished.  The thread parameter points to the thread that
    // has just completed.
    void finishedOperation(PlotMSThread* thread);
    
    // These signals are used to update the PlotProgressWidget across different
    // threads.  They shouldn't need to be used by other classes, even
    // children.
    // <group>
    void initializeProgress(const String& operationName);
    void updateProgress(unsigned int progress, const String& status);
    void finalizeProgress();
    // </group>
    
protected:
    // Access for subclasses to initialize the progress widget with the given
    // operation name.
    void initializeProgressWidget(const String& operationName);
    
    // Access for subclasses to update the progress widget.
    void updateProgressWidget(unsigned int progress, const String& status);
    
    // Access for subclasses to finalize the progress widget.
    void finalizeProgressWidget();
    
protected slots:
    // For when the user requests "background" for the thread.
    virtual void background() = 0;
    
    // For when the user requests "pause" for the thread.
    virtual void pause() = 0;
    
    // For when the user requests "resume" for the thread.
    virtual void resume() = 0;
    
    // For when the user requests "cancel" for the thread.
    virtual void cancel() = 0;
    
private:
    // Progress widget.
    PlotProgressWidget* itsProgressWidget_;
    
    // Method/Object to run when thread is finished.
    // <group>
    PMSPTMethod* itsPostThreadMethod_;
    PMSPTObject itsPostThreadObject_;
    // </group>
};


// SUBCLASSES OF PLOTMSTHREAD //

// Subclass of PlotMSThread for releasing/redrawing a list of canvases for
// plotting implementations that have their own threaded drawing.
class PlotMSDrawThread : public PlotMSThread, public PlotOperationWatcher {
    Q_OBJECT
    
public:
    // Constructor which releases/redraws all canvases on the given plotter.
    // A post-thread method can also be given.
    PlotMSDrawThread(PlotMSPlotter* plotter,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);
    
    // Constructor which releases/redraws all canvases associated with the
    // given plot.  A post-thread method can also be given.
    PlotMSDrawThread(PlotMSPlot* plot);
    
    // Destructor.
    ~PlotMSDrawThread();
    
    
    // Implements PlotMSThread::startOperation().
    void startOperation();
    
    // Implements PlotOperationWatcher::operationChanged().
    void operationChanged(const PlotOperation& operation);
    
signals:
    // Cross-thread signal, for internal use.
    void updateOperations();
    
protected slots:
    // Implements PlotMSThread::background().
    void background();
    
    // Implements PlotMSThread::pause().
    void pause();
    
    // Implements PlotMSThread::resume().
    void resume();
    
    // Implements PlotMSThread::cancel().
    void cancel();
    
private:
    // Canvases.
    vector<PlotCanvasPtr> itsCanvases_;
    
    // Operations.
    vector<PlotOperationPtr> itsOperations_;
    
    // Last set progresses.
    vector<unsigned int> itsLastProgresses_;
    
    // Last set status.
    String itsLastStatus_;
    
    // Flag for whether an update has been posted and needs to update the GUI.
    bool itsLastUpdateWaiting_;
    
    // Mutex.
    PlotMutexPtr itsOpMutex_, itsUpdateMutex_;
    
    
    // Should be called from the constructor.
    void initialize(const vector<PlotCanvasPtr>& canvases);
    
private slots:
    // Cross-thread slot, for internal use.
    void operationsUpdated();
};


// Subclass of PlotMSThread for loading/releases axes in a PlotMSCache.
class PlotMSCacheThread : public PlotMSThread, public virtual QThread {
    Q_OBJECT
    
    friend class PlotMSCache;
    
public:
    // Constructor which takes the PlotMSData, the axes, and whether the axes
    // are being loaded or released.
    PlotMSCacheThread(PlotMSPlot* plot, const vector<PMS::Axis>& axes,
            const vector<PMS::DataColumn>& data, bool loadAxes);
    
    // Destructor.
    ~PlotMSCacheThread();
    
    
    // Implements PlotMSThread::startOperation().
    void startOperation() { start(); }
    
protected:
    // Implements QThread::run().
    void run();
    
    // Allows the cache to set the progress.
    void setProgress(unsigned int progress) {
        setProgressAndStatus(progress, itsLastStatus_); }
    
    // Allows the cache to set the status.
    void setStatus(const String& status) {
        setProgressAndStatus(itsLastProgress_, status); }
    
    // Allows the cache to set the progress and the status.
    void setProgressAndStatus(unsigned int progress, const String& status);
    
protected slots:
    // Implements PlotMSThread::background().
    void background();
    
    // Implements PlotMSThread::pause().
    void pause();
    
    // Implements PlotMSThread::resume().
    void resume();
    
    // Implements PlotMSThread::cancel().
    void cancel();
    
private:
    // Data.
    PlotMSData* itsData_;
    
    // VisSet.
    VisSet* itsVisSet_;
    
    // Axes.
    vector<PMS::Axis> itsAxes_;
    
    // Axes data columns.
    vector<PMS::DataColumn> itsAxesData_;
    
    // Whether the axes should be loaded or released.
    bool itsLoadAxes_;
    
    // MS averaging.
    PlotMSAveraging itsAveraging_;
    
    // Last set progress.
    unsigned int itsLastProgress_;
    
    // Last set status.
    String itsLastStatus_;
    
private slots:
    // Slot for when the QThread finishes.
    void threadFinished();
};


// UTILITY CLASSES //

// Implementation of PlotMutex for QThreads using a QMutex.  Note: this is a
// duplication of QPMutex in the QwtPlotter, but this is a small and simple
// class and its re-implementation avoids a dependency on the QwtPlotter.
class PlotMSMutex : public PlotMutex {
public:
    // Constructor.
    PlotMSMutex() { }
    
    // Destructor.
    ~PlotMSMutex() { }
    
    
    // Implements PlotMutex::lock().
    void lock() { itsMutex_.lock(); }
    
    // Implements PlotMutex::unlock().
    void unlock() { itsMutex_.unlock(); }
    
    // Implements PlotMutex::tryLock().
    bool tryLock() { return itsMutex_.tryLock(); }
    
private:
    // Mutex.
    QMutex itsMutex_;
};

}

#endif /* PLOTMSTHREAD_QO_H_ */
