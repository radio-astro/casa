//# PlotMSCacheThread.qo.h: Subclass of PlotMSThread for cache loading.
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
//# $Id: $
#ifndef PLOTMSCACHETHREAD_QO_H_
#define PLOTMSCACHETHREAD_QO_H_

#include <plotms/Threads/Gui/PlotMSThread.qo.h>
#include <plotms/PlotMS/PlotMSSelection.h>
#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSTransformations.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <QThread>

#include <vector>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMSCacheBase;
class PlotMSPlot;

// Subclass of PlotMSThread for loading axes in a PlotMSCache.
class PlotMSCacheThread : public PlotMSThread {
    Q_OBJECT
    
    //# Friend class declarations.
    friend class PlotMSCacheBase;
    friend class MSCache;
    friend class CalCache;
    friend class PlotMSCacheThreadHelper;
    
public:
    // LOADING constructor which takes the PlotMSPlot, the PlotMSData, the axes
    // and data columns, the averaging, a flag for whether to call setupPlot
    // after the loading, and optional post-thread method parameters.
   /* PlotMSCacheThread(PlotMSPlot* plot, PlotMSCacheBase* cache,
		      const vector<PMS::Axis>& axes, 
		      const vector<PMS::DataColumn>& data2,
		      const String& msname, 
		      const PlotMSSelection& selection, 
		      const PlotMSAveraging& averaging, 
		      const PlotMSTransformations& transformations, 
		      bool setupPlot = false,
		      PMSPTMethod postThreadMethod = NULL,
		      PMSPTObject postThreadObject = NULL);*/
    
    // RELEASING constructor which takes the PlotMSPlot, the axes, and optional
    // post-thread method parameters.
    /*PlotMSCacheThread(PlotMSPlot* plot, const vector<PMS::Axis>& axes,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);*/
    PlotMSCacheThread(QtProgressWidget* progress,
                PMSPTMethod postThreadMethod = NULL,
                PMSPTObject postThreadObject = NULL);
    
    // Destructor.
    ~PlotMSCacheThread();
    
    
    // Implements PlotMSThread::startOperation().
    virtual void startOperation();
    
    virtual void cancelOperation();

protected:

    
    // Allows the cache to set the progress.
    void setProgress(unsigned int progress) {
        setProgressAndStatus(progress, itsLastStatus_); }
    
    // Allows the cache to set the status.
    void setStatus(const String& status) {
        setProgressAndStatus(itsLastProgress_, status); }
    
    // Allows the cache to set the progress and the status.
    void setProgressAndStatus(unsigned int progress, const String& status);
    
    // Slot for when the QThread finishes.
    virtual void threadFinished();

private:
    PlotMSCacheThread( const PlotMSCacheThread& other );
    PlotMSCacheThread operator=( const PlotMSCacheThread& other );

    // Plot.
    PlotMSPlot* itsPlot_;
    
    // Data.
    PlotMSCacheBase* itsCache_;
    
    // Load (true) or release (false) axes.
    bool itsLoad_;
    
    // Axes.
    vector<PMS::Axis> itsAxes_;
    
    // Axes data columns.
    vector<PMS::DataColumn> itsAxesData_;
    
    // MS selection/averaging info
    String itsMSName_;
    PlotMSSelection itsSelection_;
    PlotMSAveraging itsAveraging_;
    PlotMSTransformations itsTransformations_;
    
    // Whether to set up the cache afterwards.
    bool itsSetupPlot_;
    
    // Last set progress.
    unsigned int itsLastProgress_;
    
    // Last set status.
    String itsLastStatus_;
    

    
    // Error message, if there was one (otherwise empty).
    String itsCacheError_;
    

};




}

#endif /* PLOTMSCACHETHREAD_QO_H_ */
