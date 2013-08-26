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

#include <plotms/Threads/ThreadController.h>
#include <QObject>

//#include <casa/namespace.h>

namespace casa {

class QtProgressWidget;

// Abstract class to control a threaded operation from the GUI.
// Classes that want to run PlotMSThreads should:
// 1) Provide a QtProgressWidget in the main (GUI) thread.
// 2) Connect the finishedOperation() signal to a slot as needed.
// 3) Call startOperation() to start the thread.
// 4) Call the post-thread method after completion, if there is one.

class PlotMSThread : public QObject, public ThreadController {
   Q_OBJECT
    
public:
    // Constructor which takes the progress widget to use, and an optional
    // post-thread method that should be called when the thread is finished.
    PlotMSThread( QtProgressWidget* progress,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);
    
    // Destructor.
    virtual ~PlotMSThread();
    
    
    // ABSTRACT METHODS //
    
    // Abstract method which does the operation.  IMPORTANT: subclasses MUST
    // emit the finished() signal when finished.
    virtual void startOperation() = 0;
    
    
    // IMPLEMENTED METHODS //
    virtual void finished();

    //Overridden from BackgroundThread in order to update the progress
    //widget on the GUI thread.
    virtual void initializeProgress(const String& operationName);
    virtual void setProgress(unsigned int progress, const String& status);
    virtual void finishProgress();

signals:
    // This signal MUST be emitted after start() has been called, and when the
    // operation has finished.  The thread parameter points to the thread that
    // has just completed.
    void finishedOperation(PlotMSThread* thread);
    void updateProgress(unsigned int value, const QString& statusStr );
    void initProgress(const QString& operationName );
    void finalizeProgress();

protected:
    //Allows subclasses to override and do any clean-up they need to
    //once the background thread has completed.
    virtual void threadFinished(){}
    //Signal that clean-up and thread work is done.
    void signalFinishedOperation( PlotMSThread* thread );

protected slots:
	// For when the user requests "cancel" for the thread.
	void cancelThread();

private slots:
	// For when the user requests "background" for the thread.
	void backgroundThread();

	// For when the user requests "pause" for the thread.
	void pauseThread();

	// For when the user requests "resume" for the thread.
	void resumeThread();



};
}

#endif /* PLOTMSTHREAD_QO_H_ */
