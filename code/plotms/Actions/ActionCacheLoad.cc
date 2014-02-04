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

#include "ActionCacheLoad.h"
#include <plotms/Client/Client.h>
#include <plotms/Threads/CacheThread.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <QDebug>

namespace casa {

ActionCacheLoad::ActionCacheLoad( Client* client )
	: ActionCache( client ){
	initialize();

}

ActionCacheLoad::ActionCacheLoad( Client* client, vector<PlotMSPlot*> plots,
		PMSPTMethod postThreadMethod )
	: ActionCache( client, plots, postThreadMethod){
	initialize();
	int plotCount = plots.size();
	for ( int i = 0; i < plotCount; i++ ){
		vector<PMS::Axis> plotAxes = plots[i]->getCachedAxes();
		axes.push_back( plots[i]->getCachedAxes() );
		cachedData.push_back(plots[i]->getCachedData() );
	}
}

void ActionCacheLoad::initialize(){
	itsType_ = CACHE_LOAD;
	setupPlot = false;
}

void ActionCacheLoad::setSetupPlot( bool setUp ){
	setupPlot = setUp;
}


bool ActionCacheLoad::isAxesValid( vector<pair<PMS::Axis,unsigned int > > /*cacheAxes*/, int /*plotIndex*/, int /*axisIndex*/ ) const {
	bool valid = true;
	/*if ( cacheAxes.size() > 0 ){
		for(unsigned int j = 0; j < cacheAxes.size(); j++){
			if( cacheAxes[j].first == axes[axisIndex] ){
				valid = false;
				break;
			}
		}
	}*/
	return valid;
}

void ActionCacheLoad::setUpWorkParameters(CacheThread* cacheThread, int plotIndex, vector<PMS::Axis>& axes){

	PlotMSPlotParameters& params = plots[plotIndex]->parameters();
	PMS_PP_MSData* paramsData = params.typedGroup<PMS_PP_MSData>();
	if ( cacheThread != NULL ){
		cacheThread->setLoad(true);
		cacheThread->setCacheBase(&plots[plotIndex]->cache());
		if ( cachedData.size() == 0 ){
			cacheThread->setAxesData( axes.size() );
		}
		else {
			cacheThread->setAxesData( cachedData[plotIndex] );
		}

		cacheThread->setName( paramsData->filename() );
		cacheThread->setSelection(  paramsData->selection() );
		cacheThread->setAveraging( paramsData->averaging() );
		cacheThread->setTransformations(paramsData->transformations());
		cacheThread->setSetupPlot( setupPlot );
	}
}

bool ActionCacheLoad::loadAxes() {
	bool axesLoaded = false;
	if ( client != NULL ){
		//Only get the axes from the client if they
		//have not already been specified.
		if ( axes.size() == 0 ){
			axes = client->getSelectedLoadAxes();
		}
		if ( axes.size() > 0 ){
			axesLoaded = true;
		}
		//We also need to get the data-column,model,corrected,
		if ( this->cachedData.size() == 0 ){
			int plotCount = plots.size();
			for ( int i = 0; i < plotCount; i++ ){
				cachedData.push_back(plots[i]->getCachedData());
			}
		}
	}
	return axesLoaded;
}

ActionCacheLoad::~ActionCacheLoad() {
}

} /* namespace casa */
