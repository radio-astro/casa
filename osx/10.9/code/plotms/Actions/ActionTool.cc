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

#include "ActionTool.h"
#include <plotms/Client/Client.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
namespace casa {

ActionTool::ActionTool( Client* client )
: PlotMSAction( client ){
	toolEnabled = false;
}

bool ActionTool::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		toolEnabled = client->isActionEnabled( itsType_ );
		parametersLoaded = true;
	}
	return parametersLoaded;
}

ToolCode ActionTool::getToolCode() const {
	return NONE_TOOL;
}

bool ActionTool::doActionSpecific(PlotMSApp* plotms){

	ToolCode toolCode=NONE_TOOL;
	bool useAnnotator = false;
	// Set tool to enum value corresponding to desired tool,
	// based on this Action's type (also an enum).
	// Note that the SelectTool, as the entity holding the rectangles, doubles
	// for selection and for subtracting selections, distinguished by
	// having separate enum values.
	//
	if ( toolEnabled ){
		toolCode = getToolCode();
		useAnnotator = isAnnotator();
	}


	const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	vector<PlotCanvasPtr> canv;
	for(unsigned int i = 0; i < plots.size(); i++) {
		if(plots[i] == NULL) continue;
		canv = plots[i]->canvases();
		for(unsigned int j = 0; j < canv.size(); j++) {
			if(canv[j].null()) continue;

			// Update standard mouse tools.
			canv[j]->standardMouseTools()->setActive(!useAnnotator);
			if(!useAnnotator)   {
				PlotStandardMouseToolGroupPtr x;
				x=canv[j]->standardMouseTools();
				x->setActiveTool(toolCode);
			}
		}
	}

	bool result = doTool( plotms );
	return result;
}

bool ActionTool::isAnnotator() const {
	return false;
}



bool ActionTool::doTool(PlotMSApp* /*plotms*/) {
	return true;
}

ActionTool::~ActionTool() {
}

} /* namespace casa */
