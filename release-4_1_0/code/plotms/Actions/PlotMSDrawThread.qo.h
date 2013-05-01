//# PlotMSDrawThread.qo.h: Subclass of PlotMSThread for drawing.
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
#ifndef PLOTMSDRAWTHREAD_QO_H_
#define PLOTMSDRAWTHREAD_QO_H_

#include <plotms/Actions/PlotMSThread.qo.h>

#include <vector>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMSPlotter;

// Subclass of PlotMSThread that handles following the progress of canvas
// drawing.
class PlotMSDrawThread : public PlotMSThread, public PlotOperationWatcher {
    Q_OBJECT
    
public:
    // Constructor which takes the plotter, and optional post-thread method
    // parameters.
    PlotMSDrawThread(PlotMSPlotter* plotter,
            PMSPTMethod postThreadMethod = NULL,
            PMSPTObject postThreadObject = NULL);
    
    // Destructor.
    ~PlotMSDrawThread();
    
    
    // Updates the internal list of canvases based upon the parent
    // PlotMSPlotter.
    void updatePlotterCanvases();
    
    // Implements PlotMSThread::startOperation().
    void startOperation();
    
    bool isDrawing() const;

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
    // Parent plotter.
    PlotMSPlotter* itsPlotter_;
    
    // Operations.
    vector<PlotOperationPtr> itsOperations_;
    
    // Mutex for accessing operations.
    QMutex itsOperationsMutex_;
    
    // Flag for if thread is currently running or not.
    bool isRunning_;
    
    // Flag indicating that one or more operations tried to call
    // operationChanged() while the operations mutex was locked.
    bool itsOperationFlag_;
    
    // Flag for whether thread was canceled or not.
    bool wasCanceled_;
};

}

#endif /* PLOTMSDRAWTHREAD_QO_H_ */
