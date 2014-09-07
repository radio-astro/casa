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

#ifndef THREADCONTROLLER_H_
#define THREADCONTROLLER_H_

#include <plotms/Threads/ThreadCommunication.h>

namespace casa {

//# Forward declarations.
class ProgressMonitor;
class BackgroundThread;
class PlotMSPlot;

// Typedefs for using post-thread methods.
// <group>
typedef void* PMSPTObject;
typedef void PMSPTMethod(PMSPTObject, bool);
// </group>

class ThreadController : public ThreadCommunication {
public:
	// Constructor which takes the progress widget to use, and an optional
	// post-thread method that should be called when the thread is finished.
	ThreadController(ProgressMonitor* progress,
			PMSPTMethod postThreadMethod = NULL,
			PMSPTObject postThreadObject = NULL);

	virtual ~ThreadController();

	//***********************************************************************
	//            Thread Communication Interface
	//***********************************************************************

	//Worker classes can store and error message.
	virtual void setError( const String& errorMessage );
	// Allows the cache to set the status.
	virtual void setStatus(const String& status);
	virtual void setProgress(unsigned int progress);
	virtual void finished( );
	// These signals are used to update the QtProgressWidget across different
	// threads.  They shouldn't need to be used by other classes, even
	// children.
	// <group>
	virtual void initializeProgress(const String& operationName);
	virtual void setProgress(unsigned int progress, const String& status);
	virtual void finishProgress();

	// </group>

	// Work classes can specify whether they support background operation, pause/resume,
	//and cancelling.
	virtual void setAllowedOperations(bool background, bool pauseResume, bool cancel);

	//Callback for classes doing the work to check whether they should
	//cancel what they are doing or not.
	virtual bool wasCanceled() const;

	//********************************************************************
	//                 Thread Control
	//********************************************************************

	//Store the thread that will be doing the work
	void setWorkThread( BackgroundThread* workThread );

	// For when the user requests "background" for the thread.
	virtual void background();

	// For when the user requests "pause" for the thread.
	virtual void pause();

	// For when the user requests "resume" for the thread.
	virtual void resume();

	// For when the user requests "cancel" for the thread.
	virtual void cancel();

	// Executes the post-thread method as needed.  Does nothing if a
	// post-thread method was not set in the constructor.
	virtual void postThreadMethod();

	//Returns the result of the background operation.
	bool getResult() const;

	//Returns any error from the background operation.
	String getError() const;
	bool isErrorWarning() const;
	String getErrorTitle() const;
	PlotMSPlot* getPlot();
protected:

	//Starts the thread
	void startThread();

	// Flag for whether thread was canceled or not.
	volatile bool wasCanceled_;

	//Holds error messages
	String error;
	String errorTitle;
	bool errorWarning;



	// Progress widget.
	ProgressMonitor* itsProgressWidget_;

	// Method/Object to run when thread is finished.
	// <group>
	PMSPTMethod* itsPostThreadMethod_;
	PMSPTObject itsPostThreadObject_;
	// </group>


private:
	ThreadController( const ThreadController& controller );
	ThreadController operator=( const ThreadController& other );

	BackgroundThread* bgThread;

};

} /* namespace casa */
#endif /* THREADCONTROLLER_H_ */
