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

#include "ActionSelect.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Client/Client.h>

namespace casa {

ActionSelect::ActionSelect( Client* client )
:PlotMSAction( client ){
}

bool ActionSelect::doActionSpecific(PlotMSApp* plotms){
	// Locate/Flag/Unflag on all visible canvases.
	const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	vector<PlotCanvasPtr> visibleCanv = client->currentCanvases();

	// Get flagging parameters.
	//PlotMSFlagging flagging = client->getFlagging();

	PlotMSPlot* plot;
	for(unsigned int i = 0; i < plots.size(); i++) {
		plot = plots[i];
		if(plot == NULL) continue;

		// Get parameters.
		PlotMSPlotParameters& params = plot->parameters();
		PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();

		// Detect if we are showing flagged/unflagged points (for locate)
		PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
		Bool showUnflagged=(d->unflaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);
		Bool showFlagged=(d->flaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);

		vector<PlotCanvasPtr> canv = plot->canvases();
		for(unsigned int j = 0; j < canv.size(); j++) {
			// Only apply to visible canvases.
			bool visible = false;
			for(unsigned int k= 0; !visible && k < visibleCanv.size(); k++)
				if(canv[j] == visibleCanv[k]) visible = true;
			if(!visible) continue;

			// Get selected regions on that canvas.
			vector<PlotRegion> regions= canv[j]->getSelectedRects();
			if(regions.size() == 0) continue;

			// Actually do locate/flag/unflag...
			PlotLogMessage* m = NULL;
			try {
				m = doFlagOperation( plot, j, regions, showUnflagged, showFlagged );

				// ...and catch any reported errors.
			} catch(AipsError& err) {
				itsDoActionResult_ = "Error during ";
				itsDoActionResult_ += getOperationLabel();
				itsDoActionResult_ += ": " + err.getMesg();
				return false;
			} catch(...) {
				itsDoActionResult_ = "Unknown error during ";
				itsDoActionResult_ += getOperationLabel();
				itsDoActionResult_ += "!";
				return false;
			}
			// Log results.
			if(m != NULL) {
				stringstream msg;

				// For multiple plots or canvases, add a note at the
				// beginning to indicate which one this is.
				if(plots.size() > 1 || canv.size() > 1) {
					msg << "[";

					if(plots.size() > 1) msg << "Plot #" << i;
					if(plots.size() > 1 && canv.size() > 1) msg << ", ";
					if(canv.size() > 1) msg << "Canvas #" << j;

					msg << "]: ";
				}

				// Append region values for x-axis.
				msg << PMS::axis(c->xAxis()) << " in ";
				for(unsigned int k = 0; k < regions.size(); k++) {
					if(k > 0) msg << " or ";
					msg << "[" << regions[k].left() << " "
							<< regions[k].right() << "]";
				}

				// Append region values for y-axis.
				msg << ", " << PMS::axis(c->yAxis()) << " in ";
				for(unsigned int k = 0; k < regions.size(); k++) {
					if(k > 0) msg << " or ";
					msg << "[" << regions[k].bottom() << " "
							<< regions[k].top() << "]";
				}

				// Append result as returned by cache.
				msg << ":\n" << m->message();
				m->message(msg.str(), true);

				// Log message.
				plotms->getLogger()->postMessage(*m);

				delete m;
				m = NULL;
			}


			// If this plot was flagged/unflagged, add it to the redraw
			// list.
			addRedrawPlot( plot );

		}

		redrawPlots( plot, visibleCanv);
	}
	return true;
}

void ActionSelect::addRedrawPlot( PlotMSPlot* plot ){
	flaggedPlots.push_back( plot );
}

void ActionSelect::redrawPlots(PlotMSPlot* plot, vector<PlotCanvasPtr>& visibleCanv  ){
	// For a flag/unflag, need to tell the plots to redraw themselves,
	// and clear selected regions.
	bool hold = client->allDrawingHeld();
	if(!hold) client->holdDrawing();

	for(unsigned int i = 0; i < flaggedPlots.size(); i++) {
		flaggedPlots[i]->plotDataChanged();

		vector<PlotCanvasPtr> canv = plot->canvases();
		for(unsigned int j = 0; j < canv.size(); j++) {
			// Only apply to visible canvases.
			bool visible = false;
			for(unsigned int k = 0;
					!visible && k < visibleCanv.size(); k++)
				if(canv[j] == visibleCanv[k]) visible = true;
			if(!visible) continue;

			canv[j]->clearSelectedRects();
		}
	}

	if(!hold) client->releaseDrawing();
	flaggedPlots.clear();
}

ActionSelect::~ActionSelect() {
}

} /* namespace casa */
