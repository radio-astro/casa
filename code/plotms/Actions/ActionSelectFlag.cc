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

#include "ActionSelectFlag.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Client/Client.h>
namespace casa {

ActionSelectFlag::ActionSelectFlag( Client* client )
: ActionSelect( client ){
	itsType_=SEL_FLAG;
}

string ActionSelectFlag::getOperationLabel() const {
	return "flagging";
}

PlotLogMessage* ActionSelectFlag::doFlagOperation( PlotMSPlot* plot,
		int canvasIndex, vector<PlotRegion>& regions,
		bool /*showUnflagged*/, bool /*showFlagged*/){
	// Get flagging parameters.
	PlotMSFlagging flagging = client->getFlagging();
	PlotLogMessage* m = plot->flagRange(canvasIndex,
			flagging, Vector<PlotRegion>(regions), true);
	return m;

}

ActionSelectFlag::~ActionSelectFlag() {
}

} /* namespace casa */
