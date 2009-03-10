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
#include <plotms/PlotMS/PlotMSThread.qo.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

//////////////////////////////
// PLOTMSTHREAD DEFINITIONS //
//////////////////////////////

PlotMSThread::PlotMSThread(PlotProgressWidget* progress,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        itsProgressWidget_(progress), itsPostThreadMethod_(postThreadMethod),
        itsPostThreadObject_(postThreadObject) {
    connect(progress, SIGNAL(backgroundRequested()), SLOT(background()));
    connect(progress, SIGNAL(pauseRequested()), SLOT(pause()));
    connect(progress, SIGNAL(resumeRequested()), SLOT(resume()));
    connect(progress, SIGNAL(cancelRequested()), SLOT(cancel()));
    
    connect(this, SIGNAL(initializeProgress(const String&)),
            progress, SLOT(initialize(const String&)));
    connect(this, SIGNAL(updateProgress(unsigned int, const String&)),
            progress, SLOT(setProgress(unsigned int, const String&)));
    connect(this, SIGNAL(finalizeProgress()), progress, SLOT(finalize()));
}

PlotMSThread::~PlotMSThread() {
    /*if(itsDeletePostThreadMethod_) delete itsPostThreadMethod_;*/ }

/*
PlotMSPostThreadMethod PlotMSThread::getPostThreadMethod() {
    return itsPostThreadMethod_; }
PlotMSPostThreadMethodObject PlotMSThread::getPostThreadMethodObject() {
    return itsPostThreadMethodObject_; }
*/

void PlotMSThread::postThreadMethod() {
    if(itsPostThreadMethod_ != NULL && itsPostThreadObject_ != NULL)
        (*itsPostThreadMethod_)(itsPostThreadObject_);
}

void PlotMSThread::initializeProgressWidget(const String& operationName) {
    emit initializeProgress(operationName); }

void PlotMSThread::updateProgressWidget(unsigned int progress,
        const String& status) { emit updateProgress(progress, status); }

void PlotMSThread::finalizeProgressWidget() { emit finalizeProgress(); }


//////////////////////////////////
// PLOTMSDRAWTHREAD DEFINITIONS //
//////////////////////////////////

PlotMSDrawThread::PlotMSDrawThread(PlotMSPlotter* plotter,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        PlotMSThread(plotter->getProgressWidget(), postThreadMethod,
        postThreadObject) {
    initialize(plotter->getPlotter()->canvasLayout()->allCanvases());
    connect(this, SIGNAL(updateOperations()), SLOT(operationsUpdated()));
}

PlotMSDrawThread::PlotMSDrawThread(PlotMSPlot* plot) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget()) {
    initialize(plot->canvases());
    connect(this, SIGNAL(updateOperations()), SLOT(operationsUpdated()));
}

PlotMSDrawThread::~PlotMSDrawThread() { }


void PlotMSDrawThread::startOperation() {
    if(itsCanvases_.size() == 0) {
        emit finishedOperation(this);
        return;
    }
    
    initializeProgressWidget(itsOperations_[0]->name());
    
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        itsOperations_[i]->setIsFinished(false);
    
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        if(itsCanvases_[i]->drawingIsHeld()) itsCanvases_[i]->releaseDrawing();
        else                                 itsCanvases_[i]->refresh();
    }
}

void PlotMSDrawThread::operationChanged(const PlotOperation& operation) {
    itsOpMutex_->lock();
    
    itsLastStatus_ = operation.currentStatus();
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        if(&*itsOperations_[i] == &operation)
            itsLastProgresses_[i] = operation.currentProgress();
    
    itsOpMutex_->unlock();
    
    if(itsUpdateMutex_->tryLock()) {
        itsUpdateMutex_->unlock();
        emit updateOperations();
    } else {
        itsOpMutex_->lock();
        itsLastUpdateWaiting_ = true;
        itsOpMutex_->unlock();
    }
}


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
    cout << "PlotMSDrawThread::cancel() not yet implemented." << endl;
}


void PlotMSDrawThread::initialize(const vector<PlotCanvasPtr>& canvases) {
    itsCanvases_.clear();
    itsOperations_.clear();
    
    itsLastProgresses_.clear();
    itsLastStatus_ = "";
    itsLastUpdateWaiting_ = false;
    
    itsOpMutex_ = new PlotMSMutex();
    itsUpdateMutex_ = new PlotMSMutex();
    
    // Remove nulls and duplicates (shouldn't be necessary).
    PlotCanvasPtr canvas;
    bool found;
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        canvas = canvases[i];
        found = false;
        for(unsigned int j = 0; !found && j < itsCanvases_.size(); j++)
            if(itsCanvases_[j] == canvas) found = true;
        if(!found) itsCanvases_.push_back(canvas);
    }
    
    // Get draw operations.
    itsOperations_.resize(itsCanvases_.size());
    for(unsigned int i = 0; i < itsOperations_.size(); i++) {
        itsOperations_[i] = itsCanvases_[i]->operationDraw();
        itsOperations_[i]->addWatcher(this);
    }
    
    // Initialize last progresses.
    itsLastProgresses_.resize(itsCanvases_.size(), 0);
}


void PlotMSDrawThread::operationsUpdated() {
    itsUpdateMutex_->lock();
    
    itsOpMutex_->lock();
    
    unsigned int n = itsLastProgresses_.size();
    double progress = 0;
    bool done = true;
    
    for(unsigned int i = 0; i < itsLastProgresses_.size(); i++) {
        progress += itsLastProgresses_[i];
        done &= itsOperations_[i]->isFinished();
    }
    String status = itsLastStatus_;
    
    itsOpMutex_->unlock();
    
    updateProgressWidget((unsigned int)(progress / n), status);
    
    itsOpMutex_->lock();
    if(done) {
        itsLastUpdateWaiting_ = false;
        for(unsigned int i = 0; i < itsOperations_.size(); i++)
            itsOperations_[i]->removeWatcher(this);
        finalizeProgressWidget();
        itsCanvases_.clear();
        itsOperations_.clear();
        itsOpMutex_->unlock();        
        itsUpdateMutex_->unlock();
        emit finishedOperation(this);
        return;
    }
    itsOpMutex_->unlock();
    
    itsUpdateMutex_->unlock();
    
    itsOpMutex_->lock();
    bool b = itsLastUpdateWaiting_;
    itsOpMutex_->unlock();
    
    if(b) {
        itsOpMutex_->lock();
        itsLastUpdateWaiting_ = false;
        itsOpMutex_->unlock();
        emit updateOperations();
    }
}


///////////////////////////////////
// PLOTMSCACHETHREAD DEFINITIONS //
///////////////////////////////////

PlotMSCacheThread::PlotMSCacheThread(PlotMSPlot* plot,
        const vector<PMS::Axis>& axes, const vector<PMS::DataColumn>& data,
        bool loadAxes) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget()),
        itsData_(&plot->data()), itsVisSet_(plot->visSet()), itsAxes_(axes),
        itsAxesData_(data), itsLoadAxes_(loadAxes),
        itsAveraging_(plot->parameters().averaging()) {
    // Make sure axes data vector is same length as axes vector.
    if(itsAxesData_.size() != itsAxes_.size())
        itsAxesData_.resize(itsAxes_.size(), PMS::DEFAULT_DATACOLUMN);
    
    // Connect QThread signals.
    QThread::connect((QThread*)this, SIGNAL(finished()),
                     SLOT(threadFinished()));
    QThread::connect((QThread*)this, SIGNAL(terminated()),
                     SLOT(threadFinished()));
}

PlotMSCacheThread::~PlotMSCacheThread() { }


void PlotMSCacheThread::run() {
    if(itsVisSet_ == NULL) return;

    if(itsLoadAxes_) {
        itsData_->loadCache(*itsVisSet_, itsAxes_, itsAxesData_, itsAveraging_,
                            this);
    } else {
        cout << "PlotMSCacheThread::release_axes not yet implemented." << endl;
    }
}

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

void PlotMSCacheThread::cancel() {
    cout << "PlotMSCacheThread::cancel() not yet implemented." << endl; }


void PlotMSCacheThread::threadFinished() {
    finalizeProgressWidget();
    emit finishedOperation(this);
}

}
