//# PlotMSThread.cc: Threading classes for PlotMS.
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
#include <plotms/Actions/PlotMSThread.qo.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

//////////////////////////////
// PLOTMSTHREAD DEFINITIONS //
//////////////////////////////

PlotMSThread::PlotMSThread(QtProgressWidget* progress,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        itsProgressWidget_(progress), itsPostThreadMethod_(postThreadMethod),
        itsPostThreadObject_(postThreadObject) {
    connect(progress, SIGNAL(backgroundRequested()), SLOT(background()));
    connect(progress, SIGNAL(pauseRequested()), SLOT(pause()));
    connect(progress, SIGNAL(resumeRequested()), SLOT(resume()));
    connect(progress, SIGNAL(cancelRequested()), SLOT(cancel()));
    
    connect(this, SIGNAL(initializeProgress(const QString&)),
            progress, SLOT(initialize(const QString&)));
    connect(this, SIGNAL(updateProgress(unsigned int, const QString&)),
            progress, SLOT(setProgress(unsigned int, const QString&)));
    connect(this, SIGNAL(finalizeProgress()), progress, SLOT(finalize()));
}

PlotMSThread::~PlotMSThread() { }

void PlotMSThread::postThreadMethod() {
    if(itsPostThreadMethod_ != NULL && itsPostThreadObject_ != NULL)
        (*itsPostThreadMethod_)(itsPostThreadObject_, wasCanceled());
}

void PlotMSThread::initializeProgressWidget(const String& operationName) {
    emit initializeProgress(operationName.c_str()); }

void PlotMSThread::updateProgressWidget(unsigned int progress,
        const String& status) { emit updateProgress(progress, status.c_str());}

void PlotMSThread::finalizeProgressWidget() { emit finalizeProgress(); }

void PlotMSThread::setAllowedOperations(bool background, bool pauseResume,
        bool cancel) {
    itsProgressWidget_->setAllowedOperations(background, pauseResume,cancel); }


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
    vector<PlotCanvasPtr> canvases = itsPlotter_->getPlotter()
                                     ->canvasLayout()->allCanvases();
    vector<PlotOperationPtr> ops;
    PlotOperationPtr op;
    bool found;
    
    // Make list of PlotOperations not already in itsOperations_.  Make sure
    // list is unique.
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        op = canvases[i]->operationDraw();
        if(op.null()) continue;
        
        found = false;
        for(unsigned int j = 0; !found && j < itsOperations_.size(); j++)
            if(itsOperations_[j] == op) found = true;
        
        for(unsigned int j = 0; !found && j < ops.size(); j++)
            if(ops[j] == op) found = true;
        
        if(!found) ops.push_back(op);
    }
    
    // Remove members of itsOperations_ not in list, unregistering this thread
    // as a watcher in the process.
    int n = (int)itsOperations_.size();
    for(int i = 0; i < n; i++) {
        op = itsOperations_[i];
        found = false;
        for(unsigned int j = 0; !found && j < ops.size(); j++)
            if(ops[j] == op) found = true;
        
        if(!found) {
            op->removeWatcher(this);
            itsOperations_.erase(itsOperations_.begin() + i);
            i--;
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


///////////////////////////////////
// PLOTMSCACHETHREAD DEFINITIONS //
///////////////////////////////////

PlotMSCacheThread::PlotMSCacheThread(PlotMSPlot* plot,
        const vector<PMS::Axis>& axes, const vector<PMS::DataColumn>& data,
        const PlotMSAveraging& averaging, bool setupPlot,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget(),
        postThreadMethod, postThreadObject), itsData_(&plot->data()),
        itsVisSet_(plot->visSet()), itsAxes_(axes), itsAxesData_(data),
        itsAveraging_(averaging), itsSetupPlot_(setupPlot && axes.size() >= 2),
        wasCanceled_(false) {
    // Make sure axes data vector is same length as axes vector.
    if(itsAxesData_.size() != itsAxes_.size())
        itsAxesData_.resize(itsAxes_.size(), PMS::DEFAULT_DATACOLUMN);
}

PlotMSCacheThread::~PlotMSCacheThread() { }


void PlotMSCacheThread::startOperation() {
    if(itsVisSet_ == NULL) {
        emit finishedOperation(this);
        return;
    }

    itsLastProgress_ = 0;
    itsLastStatus_ = "";
    
    initializeProgressWidget("load_axes");
    setAllowedOperations(false, false, true);
    
    PlotMSCacheThreadHelper* th = new PlotMSCacheThreadHelper(*this);
    
    // Connect QThread signals.
    connect(th, SIGNAL(finished()), SLOT(threadFinished()));
    connect(th, SIGNAL(terminated()), SLOT(threadFinished()));
    connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));
    connect(th, SIGNAL(terminated()), th, SLOT(deleteLater()));
    
    th->start();
}

bool PlotMSCacheThread::wasCanceled() const { return wasCanceled_; }

void PlotMSCacheThread::setProgressAndStatus(unsigned int progress,
        const String& status) {
    if(progress != itsLastProgress_ || status != itsLastStatus_) {
        itsLastProgress_ = progress;
        itsLastStatus_ = status;
        updateProgressWidget(itsLastProgress_, itsLastStatus_);
    }
}


void PlotMSCacheThread::background() {
    cout << "PlotMSCacheThread::background() not yet implemented." << endl; }

void PlotMSCacheThread::pause() {
    cout << "PlotMSCacheThread::pause() not yet implemented." << endl; }

void PlotMSCacheThread::resume() {
    cout << "PlotMSCacheThread::resume() not yet implemented." << endl; }

void PlotMSCacheThread::cancel() { wasCanceled_ = true; }


void PlotMSCacheThread::threadFinished() {
    finalizeProgressWidget();
    emit finishedOperation(this);
}


/////////////////////////////////////////
// PLOTMSCACHETHREADHELPER DEFINITIONS //
/////////////////////////////////////////

PlotMSCacheThreadHelper::PlotMSCacheThreadHelper(PlotMSCacheThread& parent) :
        itsParent_(parent) { }

PlotMSCacheThreadHelper::~PlotMSCacheThreadHelper() { }    

void PlotMSCacheThreadHelper::run() {
    itsParent_.itsData_->loadCache(*itsParent_.itsVisSet_, itsParent_.itsAxes_,
            itsParent_.itsAxesData_, itsParent_.itsAveraging_, &itsParent_);
    if(itsParent_.itsSetupPlot_)
        itsParent_.itsData_->setupCache(itsParent_.itsAxes_[0],
                                        itsParent_.itsAxes_[1]);
}


////////////////////////////////////
// PLOTMSEXPORTTHREAD DEFINITIONS //
////////////////////////////////////

PlotMSExportThread::PlotMSExportThread(PlotMSPlot* plot,
        const PlotExportFormat& format, PMSPTMethod postThreadMethod,
        PMSPTObject postThreadObject) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget(),
        postThreadMethod, postThreadObject), itsPlot_(plot),
        itsFormat_(format), itsHelper_(new PlotMSExportThreadHelper(*this)) {
    if(plot != NULL) {
        vector<PlotCanvasPtr> canvases = plot->canvases();
        itsOperations_.resize(canvases.size());
        for(unsigned int i = 0; i < canvases.size(); i++) {
            itsOperations_[i] = canvases[i]->operationExport();
            itsOperations_[i]->addWatcher(this);
        }
    }
    
    // Connect QThread signals.
    connect(itsHelper_, SIGNAL(finished()), SLOT(threadFinished()));
    connect(itsHelper_, SIGNAL(terminated()), SLOT(threadFinished()));
}

PlotMSExportThread::~PlotMSExportThread() {
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        itsOperations_[i]->removeWatcher(this);
    delete itsHelper_;
}


void PlotMSExportThread::startOperation() {
    if(itsPlot_ == NULL) {
        emit finishedOperation(this);
        return;
    }
    
    initializeProgressWidget(PlotCanvas::OPERATION_EXPORT);
    setAllowedOperations(false, false, true);
    
    itsHelper_->start();
}

void PlotMSExportThread::operationChanged(const PlotOperation& operation) {
    if(!itsMutex_.tryLock()) return; // Only happens on a cancel.
        
    String status = operation.currentStatus();
    double progress = 0;
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        progress += itsOperations_[i]->inProgress() ?
                    itsOperations_[i]->currentProgress() : 100;
                    
    updateProgressWidget((unsigned int)(progress / itsOperations_.size()),
                         status);    
    
    itsMutex_.unlock();
}


bool PlotMSExportThread::wasCanceled() const { return false; }


void PlotMSExportThread::background() {
    cout << "PlotMSExportThread::background() not yet implemented." << endl; }

void PlotMSExportThread::pause() {
    cout << "PlotMSExportThread::pause() not yet implemented." << endl; }

void PlotMSExportThread::resume() {
    cout << "PlotMSExportThread::resume() not yet implemented." << endl; }

void PlotMSExportThread::cancel() {
    itsMutex_.lock();
    
    vector<PlotCanvasPtr> canvases = itsPlot_->canvases();
    PlotOperationPtr op;
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        op = canvases[i]->operationExport();
        if(op.null()) continue;
        op->setCancelRequested(true);
    }
    
    itsMutex_.unlock();
}


void PlotMSExportThread::threadFinished() {
    itsMutex_.lock();
    
    finalizeProgressWidget();
    
    if(itsHelper_->itsExportResult_)
        itsPlot_->parent()->getPlotter()->showMessage(
                "Successfully exported plot to " + itsFormat_.location + "!",
                "Export Success");
    else
        itsPlot_->parent()->getPlotter()->showError(
                "There was a problem exporting to file " + itsFormat_.location+
                "!", "Export Error", false);
    
    itsMutex_.unlock();
    
    emit finishedOperation(this);
}


//////////////////////////////////////////
// PLOTMSEXPORTTHREADHELPER DEFINITIONS //
//////////////////////////////////////////

PlotMSExportThreadHelper::PlotMSExportThreadHelper(PlotMSExportThread& parent):
        itsParent_(parent) { }

PlotMSExportThreadHelper::~PlotMSExportThreadHelper() { }

void PlotMSExportThreadHelper::run() {
    itsExportResult_ = itsParent_.itsPlot_->exportToFormat(
                       itsParent_.itsFormat_);
}

}
