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

#ifndef ACTIONCACHE_H_
#define ACTIONCACHE_H_

#include <plotms/Actions/PlotMSAction.h>


namespace casa {

class PlotMSPlot;
class CacheThread;

class ActionCache  : public PlotMSAction {
public:
	ActionCache( Client* client );
	ActionCache( Client* client, vector<PlotMSPlot*> plots, PMSPTMethod postThreadMethod = NULL);
	virtual ~ActionCache();
protected:
	bool doWork();
	virtual bool isAxesValid( vector<pair<PMS::Axis,unsigned int > > cacheAxes, int plotIndex, int axisIndex ) const = 0;
	virtual bool loadParameters();
	virtual bool loadAxes() = 0;
	virtual void checkFeasibility(PlotMSApp* plotms, int plotIndex, vector<PMS::Axis>& a ) const;
	virtual void setUpWorkParameters(CacheThread* cacheThread, int plotIndex, vector<PMS::Axis>& axes) = 0;
	virtual bool doActionSpecific(PlotMSApp* plotms);
	vector<PlotMSPlot*> plots;
	vector<vector<PMS::Axis> > axes;

};

} /* namespace casa */
#endif /* PLOTMSACTIONCACHELOAD_H_ */
