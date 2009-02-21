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

// Static //

PlotMSOperationThread* PlotMSThread::threadForRedrawingPlotter(PlotterPtr p) {
    vector<PlotCanvasPtr> canvases;
    if(!p.null()) canvases = p->canvasLayout()->allCanvases();
    return new PlotMSDrawThread(canvases);
}

PlotMSOperationThread* PlotMSThread::threadForRedrawingPlot(PlotMSPlot* plot) {
    vector<PlotCanvasPtr> canvases;
    if(plot != NULL) canvases = plot->canvases();
    return new PlotMSDrawThread(canvases);
}

const unsigned int PlotMSThread::DEFAULT_CHECK_TIME_MSEC = 250;


// Non-Static //

PlotMSThread::PlotMSThread(PlotMSOperationThread* thread,
        PlotMSProgress* progress, unsigned int checkTime) :
        itsThread_(thread), itsProgress_(progress), itsCheckTime_(checkTime) {
    if(itsProgress_ != NULL)
        connect(this,
                SIGNAL(progressAndStatusChanged(unsigned int, const String&)),
                itsProgress_, SLOT(setProgress(unsigned int, const String&)));
    
    if(itsCheckTime_ == 0) itsCheckTime_ = DEFAULT_CHECK_TIME_MSEC;
}

PlotMSThread::~PlotMSThread() {
    if(itsThread_ != NULL) delete itsThread_;
}


void PlotMSThread::run() {
    if(itsThread_ == NULL) return;
    
    if(itsProgress_ != NULL)
        itsProgress_->initialize(itsThread_->operationName());
    
    itsThread_->start();
    while(itsThread_->isRunning() && !itsThread_->isFinished()) {
        usleep(itsCheckTime_ * 1000);
        emit progressAndStatusChanged(itsThread_->currentProgress(),
                                      itsThread_->currentStatus());
    }
    
    itsProgress_->finalize();
}


//////////////////////////////////
// PLOTMSDRAWTHREAD DEFINITIONS //
//////////////////////////////////

PlotMSDrawThread::PlotMSDrawThread(const vector<PlotCanvasPtr>& canvases) :
        itsCanvases_(canvases), itsMutex_(new PlotMSMutex()), itsIndex_(0) {
    // get rid of null canvases
    for(int i = 0; (unsigned int)i < itsCanvases_.size(); i++) {
        if(itsCanvases_[i].null()) {
            itsCanvases_.erase(itsCanvases_.begin() + i);
            i--;
        }
    }
}

PlotMSDrawThread::~PlotMSDrawThread() { }

unsigned int PlotMSDrawThread::currentProgress() const {
    unsigned int progress = 0;
    itsMutex_->lock();
    if(!itsOperation_.null()) {
        // Progress is a function of the current canvas progress and the total
        // number of canvases.
        double n = 100.0 * itsCanvases_.size();
        double perc = itsOperation_->currentProgress() / n;
        perc += itsIndex_ * 100;
        progress = (unsigned int)(perc / n);
    }
    itsMutex_->unlock();
    return progress;
}

String PlotMSDrawThread::currentStatus() const {
    if(!itsOperation_.null()) return itsOperation_->currentStatus();
    else                      return "";
}


void PlotMSDrawThread::run() {
    PlotCanvasPtr canvas;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        canvas = itsCanvases_[i];
        itsOperation_ = canvas->operationDraw(itsMutex_);
        itsIndex_ = i;
        
        if(canvas->drawingIsHeld()) canvas->releaseDrawing();
        else                        canvas->refresh();
    }
}


////////////////////////////////
// PLOTMSPROGRESS DEFINITIONS //
////////////////////////////////

PlotMSProgress::PlotMSProgress(PlotMS* plotms, QWidget* parent) :
        QWidget(parent), itsParent_(plotms) {
    setupUi(this);
    connect(backgroundButton, SIGNAL(clicked()), SLOT(background()));
    connect(pauseButton, SIGNAL(clicked()), SLOT(pauseResume()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(cancel()));
}

PlotMSProgress::~PlotMSProgress() { }


void PlotMSProgress::initialize(const String& operationName) {
    operationLabel->setText(operationName.c_str());
    progressBar->setValue(0);
    statusLabel->setText("Starting " + QString(operationName.c_str()) + ".");
}

void PlotMSProgress::finalize() {
    progressBar->setValue(100);
    statusLabel->setText("Finished " + operationLabel->text() + ".");
}


void PlotMSProgress::setStatus(const String& status) {
    statusLabel->setText(status.c_str());
}

void PlotMSProgress::setProgress(unsigned int progress) {
    if(progress > 100) progress = 100;
    progressBar->setValue(progress);
}


void PlotMSProgress::background() {
    itsParent_->getLogger().postMessage("PlotMSThread", "background",
            "Method not yet implemented!");
}

void PlotMSProgress::pauseResume() {
    pauseButton->setText(pauseButton->isChecked() ? "Resume" : "Pause");
    itsParent_->getLogger().postMessage("PlotMSThread", "pause/resume",
            "Method not yet implemented!");
}

void PlotMSProgress::cancel() {
    itsParent_->getLogger().postMessage("PlotMSThread", "cancel",
            "Method not yet implemented!");
}


/////////////////////////////
// PLOTMSMUTEX DEFINITIONS //
/////////////////////////////

PlotMSMutex::PlotMSMutex() { }
PlotMSMutex::~PlotMSMutex() { }

void PlotMSMutex::lock() { itsMutex_.lock(); }
void PlotMSMutex::unlock() { itsMutex_.unlock(); }

}
