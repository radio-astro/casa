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

#include "ActionCache.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Client/Client.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/MSCache.h>
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/Threads/CacheThread.h>
#include <QDebug>
#include <fstream>

namespace casa {

ActionCache::ActionCache( Client* client )
: PlotMSAction( client ){
}

ActionCache::ActionCache( Client* client, vector<PlotMSPlot*> cachePlots,
		PMSPTMethod postThreadMethod )
:PlotMSAction( client, postThreadMethod, cachePlots ){
	int plotCount = cachePlots.size();
	for ( int i = 0; i < plotCount; i++ ){
		plots.push_back( cachePlots[i] );
	}
}

bool ActionCache::doActionSpecific( PlotMSApp* plotms ){
	bool ok = true;
	int plotCount = plots.size();
	for ( int k = 0; k < plotCount; k++ ){
		PlotMSPlotParameters& params = plots[k]->parameters();
		PMS_PP_MSData* paramsData = params.typedGroup<PMS_PP_MSData>();
		PMS_PP_Cache* paramsCache = params.typedGroup<PMS_PP_Cache>();
		if(paramsData == NULL || paramsData->filename().empty()) {
			itsDoActionResult_ = "MS has not been loaded into the cache!";
			ok = false;
		}
		else {
			if(paramsCache == NULL) {
				itsDoActionResult_ = "Cache parameters not available!  (Shouldn't "
						"happen.)";
				ok = false;
			}
			else {
				PlotMSCacheBase& cache = plots[k]->cache();
				vector<PMS::Axis> a;

				// Remove any duplicates or axes.  If loading, also make sure that the
				// given axes are not already loaded.  If releasing, make sure that the
				// axes are loaded.
				vector<pair<PMS::Axis, unsigned int> > loaded = cache.loadedAxes();
				for(unsigned int i = 0; i < axes[k].size(); i++) {
					bool valid = true;
					for(unsigned int j = 0;  j < a.size(); j++){
						if(a[j] == axes[k][i]){
							valid = false;
							break;
						}
					}

					if(valid) {
						valid = isAxesValid( loaded, k, i );
					}
					if(valid){
						a.push_back(axes[k][i]);
					}
				}

				// Make sure that currently used axes and/or meta-data isn't being
				// released.
				checkFeasibility( plotms, k,  a );
				if(a.size() > 0) {
					CacheThread* cacheThread = new CacheThread();
					cacheThread->setAxes( a );
					cacheThread->setSetupPlot( false );
					setUpWorkParameters( cacheThread, k, a );
					setUpClientCommunication( cacheThread, k );
					ok = initiateWork( cacheThread );
				}
			}
		}
	}
	return ok;
}

void ActionCache::checkFeasibility(PlotMSApp* /*plotms*/, int /*plotIndex*/, vector<PMS::Axis>& /*a*/ ) const {
}



bool ActionCache::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		//Only load the plot from the client
		//if one has not already been specified.
		if ( plots.size() == 0 ){
			plots = client->getCurrentPlots();
		}
		if ( plots.size() > 0 ){
			parametersLoaded = loadAxes();
		}
	}
	return parametersLoaded;
}

ActionCache::~ActionCache() {
}

} /* namespace casa */
