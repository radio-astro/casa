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

#include "ActionCacheRelease.h"
#include <plotms/Client/Client.h>
#include <plotms/Threads/CacheThread.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/MSCache.h>

namespace casa {

ActionCacheRelease::ActionCacheRelease( Client* client )
	: ActionCache( client ){
	itsType_ = CACHE_RELEASE;
}

void ActionCacheRelease::setUpWorkParameters(CacheThread* cacheThread, vector<PMS::Axis>& /*axes*/ ){
	/*ct = new PlotMSCacheThread(plot, a,
						&PMS_PP_Cache::notifyWatchers, paramsCache);*/
	if ( cacheThread != NULL ){
		cacheThread->setLoad( false );
		cacheThread->setCacheBase( NULL );
	}
}

bool ActionCacheRelease::isAxesValid( vector<pair<PMS::Axis,unsigned int > > cacheAxes, int axisIndex ) const {
	bool valid = false;
	for(unsigned int j = 0;  j < cacheAxes.size(); j++){
		if(cacheAxes[j].first == axes[axisIndex]){
			valid = true;
			break;
		}
	}
	return valid;
}

void ActionCacheRelease::checkFeasibility(PlotMSApp* plotms, vector<PMS::Axis>& a ) const {
	stringstream ss;
	ss << "The following axes could not be released because they are "
			"currently in use:";
	bool removed = false;
	PlotMSPlotParameters& params = plot->parameters();
	PMS_PP_Cache* paramsCache = params.typedGroup<PMS_PP_Cache>();
	PMS::Axis x = paramsCache->xAxis(), y = paramsCache->yAxis();
	for(int i = 0; i < (int)a.size(); i++) {
		if(a[i]== x || a[i]== y || MSCache::axisIsMetaData(a[i])) {
			if(removed) ss << ',';
			ss << ' ' << PMS::axis(a[i]);
			a.erase(a.begin() + i);
			i--;
			removed = true;
		}
	}
	if(removed) {
		ss << '.';

		plotms->getLogger()->postMessage(PMS::LOG_ORIGIN,
				PMS::LOG_ORIGIN_RELEASE_CACHE, ss.str(),
				PlotLogger::MSG_WARN);
	}
}

bool ActionCacheRelease::loadAxes() {
	bool axesLoaded = false;
	if ( client != NULL ){
		axes = client->getSelectedReleaseAxes();
		if ( axes.size() > 0 ){
			axesLoaded = true;
		}
	}
	return axesLoaded;
}


ActionCacheRelease::~ActionCacheRelease() {
}

} /* namespace casa */
