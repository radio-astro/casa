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

#include "ActionInformation.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/PlotMSIndexer.h>
#include <plotms/Client/Client.h>
namespace casa {

ActionInformation::ActionInformation( Client* client )
	: PlotMSAction( client ){
	itsType_= SEL_INFO;
}

bool ActionInformation::doActionSpecific(PlotMSApp* plotms) {
	 Record retval;
	 doActionWithResponse(plotms, retval);
	 return true;
}

bool ActionInformation::doActionWithResponse(PlotMSApp* plotms, Record &retval) {
	// Locate/Flag/Unflag on all visible canvases.
	const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	vector<PlotCanvasPtr> visibleCanv = client->currentCanvases();

	// Get flagging parameters.
	//PlotMSFlagging flagging = plotms->getPlotter()->getFlaggingTab()->getValue();

	// Keep list of plots that have to be redrawn.
	vector<PlotMSPlot*> redrawPlots;

	PlotMSPlot* plot;
	for(unsigned int i = 0; i < plots.size(); i++) {
		plot = plots[i];
		if(plot == NULL) continue;

		// Get parameters.
		PlotMSPlotParameters& params = plot->parameters();

		// Detect if we are showing flagged/unflagged points (for locate)
		PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
		Bool showUnflagged=(d->unflaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);
		Bool showFlagged=(d->flaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);
		bool selectAll = true;
		vector<PlotCanvasPtr> canv = plot->canvases();
		for(uInt j = 0; j < canv.size(); ++j) {
			if(canv[j]->standardMouseTools()->selectTool()->
					getSelectedRects().size() > 0) {
				selectAll = false;
				break;
			}
		}
		for(unsigned int j = 0; j < canv.size(); j++) {
			// Only apply to visible canvases.
			bool visible = false;
			for(unsigned int k= 0; !visible && k < visibleCanv.size(); k++)
				if(canv[j] == visibleCanv[k]) visible = true;
			if(!visible) continue;

			// Get selected regions on that canvas.
			vector<PlotRegion> regions = canv[j]->standardMouseTools()
	                    		->selectTool()->getSelectedRects();

			// Actually do locate/flag/unflag...
			try {
				int iterCount = plot->cache().nIter();
				int plotIterCount = plot->iter()+j;
				if(plotIterCount >= iterCount ) break;
				Record d;
				d = plot->cache().indexer(plot->iter()+j).locateInfo(
						Vector<PlotRegion>(regions), showUnflagged,
						showFlagged, selectAll);

				//int n = retval.nfields();
				//for(uInt r = 0; r < d.nfields(); ++r) {
					//    retval.defineRecord(n+r, d.subRecord(r));
				//}
				retval.defineRecord(i*j + j, d);
				// ...and catch any reported errors.
			} catch(AipsError& err) {
				itsDoActionResult_ = "Error during info";
				itsDoActionResult_ += ": " + err.getMesg();
				return false;
			} catch(...) {
				itsDoActionResult_ = "Unknown error during info!";
				return false;
			}
		}
	}
	return true;
}


ActionInformation::~ActionInformation() {
}

} /* namespace casa */
