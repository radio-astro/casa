//# PlotMSAction.h: Actions on plotms that can be triggered.
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

#ifndef PROGRESSMONITOR_H_
#define PROGRESSMONITOR_H_

#include <casa/BasicSL/String.h>

namespace casa {

/**
 * Abstraction for a display widget that can show the progress of an
 * operation.
 */

class ProgressMonitor {
public:
	//Initialize the monitor with the identifying operation.
	virtual void initialize(const String&) = 0;
	//Set the progress to a specific amount and status.
	virtual void setProgress(unsigned int, const String&) = 0;
	//Stop the progress
	virtual void finalize() = 0;
	//Set whether the user is allowed to cancel, put the operation in the background,
	//or pause/resume the operation.
	virtual void setAllowedOperations(bool background, bool pauseResume, bool cancel) = 0;
protected:
	ProgressMonitor(){}
	virtual ~ProgressMonitor(){}
};

}
#endif /* PROGRESSMONITOR_H_ */
