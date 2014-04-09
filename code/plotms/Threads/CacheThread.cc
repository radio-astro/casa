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

#include "CacheThread.h"
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/Threads/ThreadCommunication.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QDebug>

namespace casa {

CacheThread::CacheThread() {
	itsPlot = NULL;

}

void CacheThread::setLoad( bool load ){
	itsLoad = load;
}

void CacheThread::setSetupPlot( bool usePlot ){
	itsSetupPlot = usePlot;
}

void CacheThread::setCacheBase( PlotMSCacheBase* cacheBase ){
	itsCache = cacheBase;
}

void CacheThread::setAxes( vector<PMS::Axis> axes ){
	workAxes = axes;
}

void CacheThread::setAxesData( int size ){
	Assert( size >= 0 );
	itsAxesData.resize(size, PMS::DEFAULT_DATACOLUMN);
}

void CacheThread::setAxesData( vector<PMS::DataColumn> cachedData ){
	itsAxesData = cachedData;
}

void CacheThread::setName( String msName ){
	itsMSName = msName;
}

void CacheThread::setSelection( PlotMSSelection selection ){
	itsSelection = selection;
}

void CacheThread::setAveraging( PlotMSAveraging averaging ){
	itsAveraging = averaging;
}

void CacheThread::setTransformations( PlotMSTransformations transforms ){
	itsTransformations = transforms;
}

void CacheThread::setPlot( PlotMSPlot* plot ){
	itsPlot = plot;
}

bool CacheThread::doWork(){
	bool success = true;

	 // Make sure axes data vector is same length as axes vector.

	if(itsAxesData.size() != workAxes.size()){
		 itsAxesData.resize(workAxes.size(), PMS::DEFAULT_DATACOLUMN);
	}
	try {
		// Load
		if( itsLoad ) {
			if ( itsCache ){
				itsCache->load(workAxes, itsAxesData, itsMSName, itsSelection,
						itsAveraging, itsTransformations, threadController );
			}
			else {
				throw(AipsError("Problem in Cache Thread::run A"));
			}

			if( itsSetupPlot ) {
				//There will be an indexer for each data set.
				int dataCount = itsAxesData.size()/2;
				if ( itsCache ){
					itsCache->resizeIndexer( dataCount );
				}
				else {
					throw(AipsError("Problem in CacheThread::run B"));
				}
				if ( itsCache ){
					itsCache->clearRanges();
					bool globalRanges = False;
					for ( int i = 0; i < dataCount; i++ ){
						itsCache->setUpIndexer(PMS::NONE, globalRanges, globalRanges, i);
					}
				}
			}
			// Release
		}
		else {
			if (itsCache ){
				itsCache->release( workAxes );
			}
			else {
				throw(AipsError("Problem in CacheThread::run C"));
			}
		}

	}
	catch(AipsError& err) {
		handleError(err.getMesg());
		success = false;

	} catch(...) {
		handleError( "");
		success = false;
	}
	return success;
}

void CacheThread::handleError(String message ){
	String error = "Error during cache ";
	error += itsLoad ? "loading": "releasing";
	if ( message.length() > 0 ){
		error += ": " + message;
	}
	//Cleanup the plot so the data is not left in an
	//inconsistent state.
	if ( itsPlot != NULL ){
		itsPlot->dataMissing();
	}
	//Notify that an error has ocurred.
	if ( threadController != NULL ){
		threadController->setError( error );
	}
}

CacheThread::~CacheThread() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
