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

#include "ThreadController.h"
#include <plotms/Threads/BackgroundThread.h>
#include <graphics/GenericPlotter/ProgressMonitor.h>

namespace casa {

ThreadController::ThreadController(ProgressMonitor* progress,
		PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        itsProgressWidget_(progress),
        itsPostThreadMethod_(postThreadMethod),
        itsPostThreadObject_(postThreadObject) {
	bgThread = NULL;
	wasCanceled_ = false;
}

void ThreadController::cancel(){
	if ( bgThread != NULL ){
		bgThread->terminate();
		bgThread->cancel();
	}
	wasCanceled_ = true;
}

bool ThreadController::wasCanceled() const {
	return wasCanceled_;
}

void ThreadController::background(){
	 cout << "Thread::background() not yet implemented." << endl;
}

void ThreadController::pause(){
	cout << "Thread::pause() not yet implemented." << endl;
}

void ThreadController::resume(){
	cout << "Thread::resume() not yet implemented." << endl;
}


bool ThreadController::getResult() const {
	bool success = false;
	if ( bgThread != NULL ){
		success = bgThread->getResult();
	}
	return success;
}

String ThreadController::getError() const {
	return error;
}

void ThreadController::startThread(){
	bgThread->run();
}

void ThreadController::setWorkThread( BackgroundThread* workThread ){
	bgThread = workThread;
}

void ThreadController::postThreadMethod() {
    if(itsPostThreadMethod_ != NULL && itsPostThreadObject_ != NULL)
        (*itsPostThreadMethod_)(itsPostThreadObject_, wasCanceled());
}

//---------------------------------------------------------------------------
//                 ThreadCommunication Interface
//---------------------------------------------------------------------------

void ThreadController::initializeProgress(const String& operationName) {
	if ( itsProgressWidget_ != NULL ){
		itsProgressWidget_->initialize( operationName );
	}
}

void ThreadController::setProgress(unsigned int progress, const String& status) {
	if ( itsProgressWidget_ != NULL ){
		itsProgressWidget_->setProgress( progress, status );
	}
}

void ThreadController::finishProgress() {
	if ( itsProgressWidget_ != NULL ){
		itsProgressWidget_->finalize();
	}
}

void ThreadController::finished(){
	if ( getResult() ){
		postThreadMethod();
	}
}

void ThreadController::setStatus(const String& /*status*/){

}

void ThreadController::setProgress(unsigned int progress){
	setProgress( progress, "");
}

void ThreadController::setAllowedOperations(bool background, bool pauseResume,
        bool cancel) {
    itsProgressWidget_->setAllowedOperations(background, pauseResume,cancel);
}

void ThreadController::setError( const String& errorMessage ){
	error = errorMessage;
}

ThreadController::~ThreadController() {
	delete bgThread;
}

} /* namespace casa */
