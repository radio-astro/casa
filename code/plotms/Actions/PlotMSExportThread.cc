//# PlotMSExportThread.cc: Subclass of PlotMSThread for exporting plots.
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
#include <plotms/Actions/PlotMSExportThread.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

namespace casa {

////////////////////////////////////
// PLOTMSEXPORTTHREAD DEFINITIONS //
////////////////////////////////////

PlotMSExportThread::PlotMSExportThread(
		PlotMSPlot* plot, const PlotExportFormat& format, bool interactive,
		PMSPTMethod postThreadMethod, PMSPTObject postThreadObject
	)
	: PlotMSThread(
			plot->parent()->getPlotter()->getProgressWidget(),
			postThreadMethod, postThreadObject
		), itsPlot_(plot), itsFormat_(format),
		itsHelper_(new PlotMSExportThreadHelper(this)), _interactive(interactive)
{
    if(plot != NULL) {
        vector<PlotCanvasPtr> canvases = plot->canvases();
        itsOperations_.resize(canvases.size());
        for(unsigned int i = 0; i < canvases.size(); i++) {
            itsOperations_[i] = canvases[i]->operationExport();
            itsOperations_[i]->addWatcher(this);
        }
    }
    
    // TODO !export fix
    // Quick hack for screen resolution images.  Taking a screenshot without
    // drawing the items is basically impossible in the non-main (GUI) thread,
    // so for now just turn on high resolution so that it has to draw each
    // items.  This isn't ideal because it is slow, but for now it's better to
    // have something that works and is slow than something that doesn't work.
    if((itsFormat_.type == PlotExportFormat::JPG ||
       itsFormat_.type == PlotExportFormat::PNG) &&
       itsFormat_.resolution == PlotExportFormat::SCREEN) {
    	cout << "NOTICE: Exporting to images in screen resolution is currently"
    	     << " not working.  Switching to high resolution (which is slower,"
    	     << " but works)." << endl;
    	itsFormat_.resolution = PlotExportFormat::HIGH;
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
    
    itsHelper_->run();
    threadFinished();
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
    if(_interactive) {
    	if(itsHelper_->itsExportResult_) {
    		itsPlot_->parent()->getPlotter()->showMessage(
                "Successfully exported plot to " + itsFormat_.location + "!",
                "Export Success");
    	}
    	else {
    		itsPlot_->parent()->getPlotter()->showError(
                "There was a problem exporting to file " + itsFormat_.location+
                "!", "Export Error", false);
    	}
    }
    itsMutex_.unlock();
    
    emit finishedOperation(this);
}


//////////////////////////////////////////
// PLOTMSEXPORTTHREADHELPER DEFINITIONS //
//////////////////////////////////////////

PlotMSExportThreadHelper::PlotMSExportThreadHelper(PlotMSExportThread* parent):
        itsParent_(parent) { }

PlotMSExportThreadHelper::~PlotMSExportThreadHelper() { }

void PlotMSExportThreadHelper::run() {

    itsExportResult_ = itsParent_->itsPlot_->exportToFormat(
                       itsParent_->itsFormat_);

}

}
