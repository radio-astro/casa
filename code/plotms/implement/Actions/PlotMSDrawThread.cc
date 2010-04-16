//# PlotMSDrawThread.cc: Subclass of PlotMSThread for drawing.
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
#include <plotms/Actions/PlotMSDrawThread.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

//////////////////////////////////
// PLOTMSDRAWTHREAD DEFINITIONS //
//////////////////////////////////

PlotMSDrawThread::PlotMSDrawThread(PlotMSPlotter* plotter,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        PlotMSThread(plotter->getProgressWidget(), postThreadMethod,
        postThreadObject), itsPlotter_(plotter), isRunning_(false),
        itsOperationFlag_(false), wasCanceled_(false) {
    updatePlotterCanvases();
}

PlotMSDrawThread::~PlotMSDrawThread() {
    // unregister self from operations
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        itsOperations_[i]->removeWatcher(this);
}


void PlotMSDrawThread::updatePlotterCanvases() {
    if(itsPlotter_ == NULL) return;
    
    itsOperationsMutex_.lock();
    vector<PlotCanvasPtr> canvases = itsPlotter_->currentCanvases();
    vector<PlotOperationPtr> ops;
    PlotOperationPtr op;
    bool found;
    
    // Make unique list of PlotOperations from current canvases.
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        op = canvases[i]->operationDraw();
        if(op.null()) continue;
        
        found = false;        
        for(unsigned int j = 0; !found && j < ops.size(); j++)
            if(ops[j] == op) found = true;
        
        if(!found) ops.push_back(op);
    }

    // Remove members of itsOperations_ not in list, unregistering this thread
    // as a watcher in the process.  If the operator is already in
    // itsOperations_, remove it from the list so it doesn't get added twice.
    int n = (int)itsOperations_.size();
    for(int i = 0; i < n; i++) {
        op = itsOperations_[i];
        found = false;
        for(unsigned int j = 0; !found && j < ops.size(); j++) {
            if(ops[j] == op) {
                found = true;
                ops.erase(ops.begin() + j);
            }
        }
        
        if(!found) {
            op->removeWatcher(this);
            itsOperations_.erase(itsOperations_.begin() + i);
            i--;
            n--;
        }
    }
    
    // Add list to itsOperations_, registering this thread as a watcher in the
    // process.
    for(unsigned int i = 0; i < ops.size(); i++) {
        op = ops[i];
        op->addWatcher(this);
        itsOperations_.push_back(op);
    }
    
    itsOperationsMutex_.unlock();
}

void PlotMSDrawThread::startOperation() {
    itsOperationsMutex_.lock();
    
    itsOperationFlag_ = false;
    
    // Make sure we have work to do..
    bool done = itsOperations_.size() == 0;
    if(!done) {
        done = true;
        for(unsigned int i = 0; i < itsOperations_.size(); i++)
            done &= itsOperations_[i]->isFinished();
    }
    
    if(done) {
        itsOperationsMutex_.unlock();
        emit finishedOperation(this);
        return;
    }
    
    initializeProgressWidget(itsOperations_[0]->name());
    setAllowedOperations(false, false, true);
    
    isRunning_ = true;
    itsOperationsMutex_.unlock();
}

void PlotMSDrawThread::operationChanged(const PlotOperation& operation) {
    if(!itsOperationsMutex_.tryLock()) {
        itsOperationFlag_ = true;
        return;
    }

    if(!isRunning_) {
        itsOperationsMutex_.unlock();
        return;
    }
    
    String status = operation.currentStatus();
    double progress = 0;
    bool done = true;
    unsigned int n = itsOperations_.size();
    for(unsigned int i = 0; i < n; i++) {
        progress += itsOperations_[i]->inProgress() ?
                    itsOperations_[i]->currentProgress() : 100;
        done &= itsOperations_[i]->isFinished();
    }
    
    updateProgressWidget((unsigned int)(progress / n), status);
    
    if(done) {
        finalizeProgressWidget();
        isRunning_ = false;
        itsOperationsMutex_.unlock();
        emit finishedOperation(this);
        return;
    }
    
    itsOperationsMutex_.unlock();
}


bool PlotMSDrawThread::wasCanceled() const { return wasCanceled_; }


void PlotMSDrawThread::background() {
    cout << "PlotMSDrawThread::background() not yet implemented." << endl;
}

void PlotMSDrawThread::pause() {
    cout << "PlotMSDrawThread::pause() not yet implemented." << endl;
}

void PlotMSDrawThread::resume() {
    cout << "PlotMSDrawThread::resume() not yet implemented." << endl;
}

bool PlotMSDrawThread::isDrawing() const {
	return isRunning_;
}

void PlotMSDrawThread::cancel() {
    itsOperationsMutex_.lock();
    if(!isRunning_) {
        itsOperationsMutex_.unlock();
        return;
    }
    
    for(unsigned int i = 0; i < itsOperations_.size(); i++) {
        if(itsOperations_[i]->inProgress()) 
            itsOperations_[i]->setCancelRequested(true);
    }
    
    wasCanceled_ = true;
    
    itsOperationsMutex_.unlock();
    
    if(itsOperationFlag_) {
        operationChanged(*itsOperations_[itsOperations_.size() - 1]);
        itsOperationFlag_ = false;
    }
}

}
