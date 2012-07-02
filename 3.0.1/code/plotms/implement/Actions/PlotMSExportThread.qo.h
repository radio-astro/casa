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

#include <plotms/Actions/PlotMSThread.qo.h>

#include <graphics/GenericPlotter/PlotOptions.h>

#include <QThread>

#include <vector>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMSExportThreadHelper;
class PlotMSPlot;


// Subclass of PlotMSThread for exporting a plot.
class PlotMSExportThread : public PlotMSThread, public PlotOperationWatcher {
    Q_OBJECT
    
    //# Friend class declarations.
    friend class PlotMSExportThreadHelper;
    
public:
    // Constructor which takes the plot to export, the export format
    // parameters, and optional post-thread method parameters.
    PlotMSExportThread(PlotMSPlot* plot, const PlotExportFormat& format,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);
    
    // Destructor.
    ~PlotMSExportThread();
    
    
    // Implements PlotMSThread::startOperation().
    void startOperation();
    
    // Implements PlotOperationWatcher::operationChanged().
    void operationChanged(const PlotOperation& operation);
    
protected:
    // Implements PlotMSThread::wasCanceled().
    bool wasCanceled() const;
    
protected slots:
    // Implements PlotMSThread::background().  Currently is unimplemented.
    void background();
    
    // Implements PlotMSThread::pause().  Currently is unimplemented.
    void pause();
    
    // Implements PlotMSThread::resume().  Currently is unimplemented.
    void resume();
    
    // Implements PlotMSThread::cancel().  Currently is unimplemented.
    void cancel();
    
private:
    // Plot.
    PlotMSPlot* itsPlot_;
    
    // Format.
    PlotExportFormat itsFormat_;
    
    // Export operations.
    vector<PlotOperationPtr> itsOperations_;
    
    // Mutex.
    QMutex itsMutex_;
    
    // Helper.
    PlotMSExportThreadHelper* itsHelper_;
    
private slots:
    // Slot for when the QThread finishes.
    void threadFinished();
};


// Helper class for PlotMSExportThread that actually does the work in a new
// QThread.
class PlotMSExportThreadHelper : public QThread {
    Q_OBJECT
    
    //# Friend class declarations.
    friend class PlotMSExportThread;
    
public:
    // Constructor that takes parent.
    PlotMSExportThreadHelper(PlotMSExportThread& parent);
    
    // Destructor.
    ~PlotMSExportThreadHelper();
    
protected:
    // Implements QThread::run().
    void run();
    
private:
    // Parent.
    PlotMSExportThread& itsParent_;
    
    // Stores the result of the export.
    bool itsExportResult_;
};

}

#endif /* PLOTMSEXPORTTHREAD_QO_H_ */
