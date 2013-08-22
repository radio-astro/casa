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

#ifndef THREADCOMMUNICATION_H_
#define THREADCOMMUNICATION_H_

#include <casa/BasicSL/String.h>

namespace casa {

/**
 * Defines communication mechanisms that the model and data can use to communicate
 * with the UI concerning progress and status.
 */

class ThreadCommunication {

public:

	// These signals are used to update the QtProgressWidget across different
	// threads.  They shouldn't need to be used by other classes, even
	// children.
	// <group>
	virtual void initializeProgress(const String& operationName) = 0;
	virtual void setProgress(unsigned int progress, const String& status) = 0;
	virtual void finishProgress() = 0;
	// </group>

	// Allows the cache to set the progress.
	virtual void setProgress(unsigned int progress) =0;


	// Allows the cache to set the status.
	virtual void setStatus(const String& status) = 0;
	virtual void setError( const String& errorMessage ) = 0;
	virtual void setAllowedOperations(bool background, bool pauseResume, bool cancel) = 0;

	//Thread signals it is done.
	virtual void finished( ) = 0;

	//Enquire whether the thread was cancelled by the user.
	virtual bool wasCanceled() const = 0;

protected:
	ThreadCommunication(){}
	virtual ~ThreadCommunication(){}
};

}
#endif /* THREADCOMMUNICATION_H_ */
