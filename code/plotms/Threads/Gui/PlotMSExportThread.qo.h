//# PlotMSExportThread.qo.h: Subclass of PlotMSThread for exporting plots.
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
#ifndef PLOTMSEXPORTTHREAD_QO_H_
#define PLOTMSEXPORTTHREAD_QO_H_

#include <plotms/Threads/Gui/PlotMSThread.qo.h>
#include <graphics/GenericPlotter/PlotOperation.h>
#include <graphics/GenericPlotter/PlotOptions.h>
//#include <QThread>
#include <QMutex>
#include <vector>

//#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMSPlot;
class PlotMSPlotter;

// Subclass of PlotMSThread for exporting a plot.
class PlotMSExportThread : public PlotMSThread, public PlotOperationWatcher {
    Q_OBJECT
    

    
public:
    // Constructor which takes the plot to export, the export format
    // parameters, and optional post-thread method parameters.
    PlotMSExportThread( QtProgressWidget* widget, PlotMSPlotter* plotter, const String& location,
    	/*PlotMSPlot* plot, const PlotExportFormat& format, bool interactive,*/
        PMSPTMethod postThreadMethod = NULL, PMSPTObject postThreadObject = NULL
    );
    
    // Destructor.
    ~PlotMSExportThread();
    virtual void cancel();

    // Implements PlotMSThread::startOperation().
    virtual void startOperation();
    
    // Implements PlotOperationWatcher::operationChanged().
    void operationChanged(const PlotOperation& operation);



protected:
    // Slot for when the QThread finishes.
    virtual void threadFinished();

private:
    PlotMSExportThread( const PlotMSExportThread& other );
    PlotMSExportThread operator=( const PlotMSExportThread& other );

    // Plot.
    vector<PlotMSPlot*> itsPlots_;
    
    // Format.
    //PlotExportFormat itsFormat_;
    String exportLocation;
    
    // Export operations.
    vector<vector<PlotOperationPtr> >itsOperations_;
    
    // Mutex.
    QMutex itsMutex_;
    
    PlotMSPlotter* itsPlotter_;

};



}

#endif /* PLOTMSEXPORTTHREAD_QO_H_ */
