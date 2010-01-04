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

#include <graphics/GenericPlotter/PlotOperation.h>

#include <QMutex>
#include <QObject>
#include <QString>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class QtProgressWidget;


// Useful macro for using post-thread methods.
#define PMS_POST_THREAD_METHOD(CLASS, METHOD)                                 \
public:                                                                       \
    static void METHOD (void* obj, bool wasCanceled) {                        \
        CLASS * cobj = static_cast< CLASS *>(obj);                            \
        if(cobj != NULL) cobj -> METHOD##_(wasCanceled);                      \
    }                                                                         \
                                                                              \
private:                                                                      \
    void METHOD##_(bool wasCanceled);

// Typedefs for using post-thread methods.
// <group>
typedef void* PMSPTObject;
typedef void PMSPTMethod(PMSPTObject, bool);
// </group>


// Abstract class for a threaded operation for plotms.  PlotMSThread does not
// directly inherit from QThread in case the threading is happening elsewhere
// in the code (such as a plotter implementation that threads its own drawing).
// Classes that want to run PlotMSThreads should:
// 1) Provide a QtProgressWidget in the main (GUI) thread.
// 2) Connect the finishedOperation() signal to a slot as needed.
// 3) Call startOperation() to start the thread.
// 4) Call the post-thread method after completion, if there is one.
class PlotMSThread : public QObject {
    Q_OBJECT
    
public:
    // Constructor which takes the progress widget to use, and an optional
    // post-thread method that should be called when the thread is finished.
    PlotMSThread(QtProgressWidget* progress,
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
    
    // These signals are used to update the QtProgressWidget across different
    // threads.  They shouldn't need to be used by other classes, even
    // children.
    // <group>
    void initializeProgress(const QString& operationName);
    void updateProgress(unsigned int progress, const QString& status);
    void finalizeProgress();
    // </group>
    
protected:
    // ABSTRACT METHODS //
    
    // Returns true if the threaded finished due to be canceled, false
    // otherwise.
    virtual bool wasCanceled() const = 0;
    
    
    // IMPLEMENTED METHODS //
    
    // Access for subclasses to initialize the progress widget with the given
    // operation name.
    void initializeProgressWidget(const String& operationName);
    
    // Access for subclasses to update the progress widget.
    void updateProgressWidget(unsigned int progress, const String& status);
    
    // Access for subclasses to finalize the progress widget.
    void finalizeProgressWidget();
    
    // Access for subclasses to set allowed operations on the progress widget.
    void setAllowedOperations(bool background, bool pauseResume, bool cancel);
    
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
    QtProgressWidget* itsProgressWidget_;
    
    // Method/Object to run when thread is finished.
    // <group>
    PMSPTMethod* itsPostThreadMethod_;
    PMSPTObject itsPostThreadObject_;
    // </group>
};


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
