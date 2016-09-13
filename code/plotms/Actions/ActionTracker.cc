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


#include "ActionTracker.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Client/Client.h>
#include <plotms/Plots/PlotMSPlot.h>

namespace casa {

ActionTracker::ActionTracker( Client* client )
:PlotMSAction( client  ){
	trackerEnabled = false;
	defaultHover = false;
	defaultDisplay = false;
}



bool ActionTracker::isHover() const {
	return defaultHover;
}

bool ActionTracker::isDisplay() const {
	return defaultDisplay;
}

bool ActionTracker::doActionSpecific(PlotMSApp* plotms){
	defaultHover = client->isActionEnabled(TRACKER_ENABLE_HOVER);
	defaultDisplay = client->isActionEnabled(TRACKER_ENABLE_DISPLAY);
	bool hover = isHover();
	bool display = isDisplay();

	const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	vector<PlotCanvasPtr> canv;
	for(unsigned int i = 0; i < plots.size(); i++) {
		if(plots[i] == NULL) continue;
		canv = plots[i]->canvases();
		for(unsigned int j = 0; j < canv.size(); j++) {
			if(canv[j].null()) continue;
			canv[j]->standardMouseTools()->turnTracker(hover | display);
			canv[j]->standardMouseTools()->turnTrackerDrawText(hover);
		}
	}

	return true;
}

bool ActionTracker::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		trackerEnabled = client->isActionEnabled( itsType_ );
		parametersLoaded = true;
	}
	return parametersLoaded;
}

ActionTracker::~ActionTracker() {
}

} /* namespace casa */
