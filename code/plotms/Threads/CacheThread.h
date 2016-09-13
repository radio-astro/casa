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

#ifndef CACHETHREAD_H_
#define CACHETHREAD_H_

#include <plotms/Threads/BackgroundThread.h>
#include <plotms/PlotMS/PlotMSSelection.h>
#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSTransformations.h>
#include <plotms/PlotMS/PlotMSCalibration.h>

namespace casa {

class PlotMSCacheBase;
class PlotMSPlot;

class CacheThread : public BackgroundThread {
public:
	CacheThread();
	void setLoad( bool load );
	void setSetupPlot( bool usePlot );
	void setPlot( PlotMSPlot* plot );
	virtual PlotMSPlot* getPlot();
	void setCacheBase( PlotMSCacheBase* cacheBase );
	void setAxes( vector<PMS::Axis> axes );
	void setAxesData( int size  );
	void setAxesData( vector<PMS::DataColumn> cachedData );
	void setName( String msName );
	void setSelection( PlotMSSelection selection );
	void setAveraging( PlotMSAveraging averaging );
	void setTransformations( PlotMSTransformations transforms );
	void setCalibration( PlotMSCalibration calibration );
	virtual ~CacheThread();
protected:
	virtual bool doWork();
	virtual void finished(){}
private:
	CacheThread( const CacheThread& other );
	CacheThread operator=( const CacheThread& other );

	//Log error, set status, and do clean up when an
	//exception is thrown.
	void handleError(String message );

	bool itsLoad;
	bool itsSetupPlot;
	PlotMSCacheBase* itsCache;
	vector<PMS::Axis> workAxes;
	vector<PMS::DataColumn> itsAxesData;
	String itsMSName;
	PlotMSSelection itsSelection;
	PlotMSAveraging itsAveraging;
	PlotMSTransformations itsTransformations;
	PlotMSCalibration itsCalibration;
	PlotMSPlot* itsPlot;
};

} /* namespace casa */
#endif /* CACHETHREAD_H_ */
