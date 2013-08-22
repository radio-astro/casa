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

#ifndef ACTIONSELECT_H_
#define ACTIONSELECT_H_

#include <plotms/Actions/PlotMSAction.h>

namespace casa {

class ActionSelect  : public PlotMSAction {
public:
	ActionSelect( Client* client );


	virtual ~ActionSelect();
protected:
	virtual bool doActionSpecific(PlotMSApp* plotms);
	virtual PlotLogMessage* doFlagOperation( PlotMSPlot* plot,
			int canvasIndex, vector<PlotRegion>& regions, bool showUnflagged, bool showFlagged ) = 0;
	virtual string getOperationLabel() const = 0;
	virtual void redrawPlots(PlotMSPlot* plot, vector<PlotCanvasPtr>& visibleCanv  );

private:
	// Keep list of plots that have to be redrawn.
	vector<PlotMSPlot*> flaggedPlots;
};

} /* namespace casa */
#endif /* ACTIONSELECT_H_ */
