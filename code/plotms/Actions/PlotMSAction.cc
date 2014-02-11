//# PlotMSAction.cc: Actions on plotms that can be triggered.
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
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Threads/BackgroundThread.h>
#include <plotms/Client/Client.h>
#include <QDebug>

namespace casa {

//////////////////////////////
// PLOTMSACTION DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

PlotMSAction::PlotMSAction( Client* requestor,
		PMSPTMethod postThreadMethod,
		vector<PlotMSPlot*> postObject )
	:client( requestor ), threadController( NULL ),
	 postThreadMethod( postThreadMethod ) {
	useThreading = true;
	int postCount = postObject.size();
	for ( int i = 0; i < postCount; i++ ){
		postThreadObject.push_back( postObject[i]);
	}


}

PlotMSAction::~PlotMSAction() { }

void PlotMSAction::setUseThreading( bool useThread ){
	useThreading = useThread;
}


// Public Methods //
bool PlotMSAction::isValid() {
	return loadParameters();
}

void PlotMSAction::setUpClientCommunication( BackgroundThread* thread, int index ){
	int threadObjectCount = postThreadObject.size();
	if ( 0 <= index && index < threadObjectCount ){
		//Client gives us the object it wants to use for communication.
		threadController = client->getThreadController( itsType_, postThreadMethod, postThreadObject[index], index );
	}
	else {
		threadController = client->getThreadController( itsType_ );
	}


	if ( threadController != NULL ){
		//We put the thread into the controller so that the client can
		//later delete it.
		threadController->setWorkThread( thread );

		//We tell the cache thread about a limited interface in the
		//controller that it can use to update and get commands from the
		//client.
		thread->setClientCommunication( threadController );
	}
}

bool PlotMSAction::initiateWork( BackgroundThread* thread ){
	//We either tell the client to run it as a thread or we run it
	//directly ourself.
	if ( useThreading ){
		client->doThreadedOperation( threadController );
	}
	else {
		if ( client != NULL ){
			client->setOperationCompleted( true );
		}
		thread->run();
		bool operationCompleted = thread->getResult();
		if ( client != NULL ){
			client->setOperationCompleted( operationCompleted );
		}	
		if ( operationCompleted ){
			int objectSize = postThreadObject.size();
			if( postThreadMethod != NULL && objectSize > 0){
				for ( int i = 0; i < objectSize; i++ ){
					(*postThreadMethod )(postThreadObject[i], false);
				}
			}

		}

	}
	return true;
}

bool PlotMSAction::loadParameters(){
	return true;
}

bool PlotMSAction::doAction(PlotMSApp* plotms) {
	itsDoActionResult_ = "";
	bool result = false;
	if(!isValid() || plotms == NULL) {
		itsDoActionResult_ = "Set parameters were not valid!";
	}
	else {
		result = doActionSpecific( plotms );
	}
	return result;
}

bool PlotMSAction::doActionWithResponse(PlotMSApp* /*plotms*/, Record & /*retval*/) {
    return true;
}

const String& PlotMSAction::doActionResult() const{
	return itsDoActionResult_;
}

}
