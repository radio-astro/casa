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
//#include <plotms/Threads/ThreadController.h>
#include <plotms/Threads/BackgroundThread.h>
#include <plotms/Client/Client.h>
namespace casa {

//////////////////////////////
// PLOTMSACTION DEFINITIONS //
//////////////////////////////

// Static //

/*const String PlotMSAction::P_PLOT = "plot";
const String PlotMSAction::P_ON_OFF = "on_off";
const String PlotMSAction::P_AXES = "axes";
const String PlotMSAction::P_FILE = "file";
const String PlotMSAction::P_FORMAT = "format";
const String PlotMSAction::P_HIGHRES = "highres";
const String PlotMSAction::P_DPI = "dpi";
const String PlotMSAction::P_WIDTH = "width";
const String PlotMSAction::P_HEIGHT = "height";
const String PlotMSAction::P_INTERACTIVE = "interactive";
*/



// Constructors/Destructors //

PlotMSAction::PlotMSAction( Client* requestor,
		PMSPTMethod postThreadMethod,
		PMSPTObject postThreadObject )
	:client( requestor ), threadController( NULL ),
	 postThreadObject( postThreadObject ),
	 postThreadMethod( postThreadMethod ) {
	useThreading = true;

}

PlotMSAction::~PlotMSAction() { }

void PlotMSAction::setUseThreading( bool useThread ){
	useThreading = useThread;
}




// Public Methods //
bool PlotMSAction::isValid() {
	return loadParameters();
}

void PlotMSAction::setUpClientCommunication( BackgroundThread* thread ){
	//Client gives us the object it wants to use for communication.
	threadController = client->getThreadController( itsType_, postThreadMethod, postThreadObject );
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
		thread->run();
		if( postThreadMethod != NULL && postThreadObject != NULL){
		    (*postThreadMethod )(postThreadObject, false);
		}
	}
	return true;
}

bool PlotMSAction::loadParameters(){
	return true;
}

bool PlotMSAction::doAction(PlotMSApp* plotms) {
	itsDoActionResult_ = "";

	if(!isValid() || plotms == NULL) {
		itsDoActionResult_ = "Set parameters were not valid!";
		return false;
	}

	bool result = doActionSpecific( plotms );
	return result;
}

bool PlotMSAction::doActionWithResponse(PlotMSApp* /*plotms*/, Record & /*retval*/) {
    return true;
}

const String& PlotMSAction::doActionResult() const{
	return itsDoActionResult_;
}

}
