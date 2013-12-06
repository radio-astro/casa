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

#ifndef PLOTMSACTIONCACHELOAD_H_
#define PLOTMSACTIONCACHELOAD_H_

#include <plotms/Actions/ActionCache.h>


namespace casa {

class PlotMSPlot;

class ActionCacheLoad  : public ActionCache {
public:
	ActionCacheLoad( Client* client );
	ActionCacheLoad( Client* client, PlotMSPlot* plot,
			PMSPTMethod postThreadMethod);
	//ActionCacheLoad( Client* client, PlotMSPlot* plot, bool interactive );
	//ActionCacheLoad( Client* client, PlotMSPlot* plot );, itsCache_,
						   /* axes, data,
						    d->filename(),
						    d->selection(),
						    d->averaging(),
						    d->transformations(),
						    false,
						    &PlotMSOverPlot::cacheLoaded, this);)*/
	void setSetupPlot( bool setUp );

	virtual ~ActionCacheLoad();
protected:
	virtual bool loadAxes();
	virtual bool isAxesValid( vector<pair<PMS::Axis,unsigned int > > cacheAxes, int axisIndex ) const;
	virtual void setUpWorkParameters(CacheThread* cacheThread, vector<PMS::Axis>& axes );
private:
	void initialize();
	vector<PMS::DataColumn> cachedData;
	bool setupPlot;
};

} /* namespace casa */
#endif /* PLOTMSACTIONCACHELOAD_H_ */
