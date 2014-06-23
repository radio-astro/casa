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

#include "ActionClearRegions.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Client/Client.h>

namespace casa {

ActionClearRegions::ActionClearRegions( Client* client )
	: PlotMSAction( client ){
	itsType_= SEL_CLEAR_REGIONS;
}

bool ActionClearRegions::doActionSpecific(PlotMSApp* plotms){
	const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	vector<PlotCanvasPtr> visibleCanv = client->currentCanvases();
	vector<PlotCanvasPtr> canv;
	for(unsigned int i = 0; i < plots.size(); i++) {
		if(plots[i] == NULL) continue;
		canv = plots[i]->canvases();
		for(unsigned int j = 0; j < canv.size(); j++) {
			if(canv[j].null()) continue;

			// Only apply to visible canvases.
			bool found = false;
			for(unsigned int k = 0; !found && k < visibleCanv.size(); k++){
				if(canv[j] == visibleCanv[k]){
					found = true;
					break;
				}
			}
			if(!found) continue;

			canv[j]->standardMouseTools()->selectTool()->clearSelectedRects();
		}
	}
	return true;
}

ActionClearRegions::~ActionClearRegions() {
}

} /* namespace casa */
