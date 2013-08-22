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
#include <plotms/Threads/Gui/PlotMSExportThread.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

#include <QDebug>

namespace casa {

////////////////////////////////////
// PLOTMSEXPORTTHREAD DEFINITIONS //
////////////////////////////////////

PlotMSExportThread::PlotMSExportThread( QtProgressWidget* progressWidget, PlotMSPlotter* plotter,
		PMSPTMethod postThreadMethod, PMSPTObject postThreadObject
	)
	: PlotMSThread(progressWidget,postThreadMethod, postThreadObject),itsPlotter_( plotter ){
	PlotExportFormat format = itsPlotter_->getPlotExportFormat();
	exportLocation = format.location;
	itsPlot_ = itsPlotter_->getCurrentPlot();
    if(itsPlot_ != NULL) {
        vector<PlotCanvasPtr> canvases = itsPlot_->canvases();
        itsOperations_.resize(canvases.size());
        for(unsigned int i = 0; i < canvases.size(); i++) {
            itsOperations_[i] = canvases[i]->operationExport();
            itsOperations_[i]->addWatcher(this);
        }
    }


}

PlotMSExportThread::~PlotMSExportThread() {
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        itsOperations_[i]->removeWatcher(this);

}


void PlotMSExportThread::startOperation() {
	if(itsPlot_ == NULL) {
    	signalFinishedOperation( this );
        return;
    }

    initializeProgress(PlotCanvas::OPERATION_EXPORT);
    setAllowedOperations(false, false, true);
    startThread();
    //threadFinished();
}

void PlotMSExportThread::operationChanged(const PlotOperation& operation) {
    if(!itsMutex_.tryLock()) return; // Only happens on a cancel.

    String status = operation.currentStatus();
    double progress = 0;
    for(unsigned int i = 0; i < itsOperations_.size(); i++)
        progress += itsOperations_[i]->inProgress() ?
                    itsOperations_[i]->currentProgress() : 100;

    setProgress((unsigned int)(progress / itsOperations_.size()), status);

    itsMutex_.unlock();
}

void PlotMSExportThread::cancel() {
    itsMutex_.lock();
    cancel();
    wasCanceled_ = true;
    itsMutex_.unlock();
}


void PlotMSExportThread::threadFinished() {
    itsMutex_.lock();
    finalizeProgress();
    if( itsPlotter_ ->isInteractive() ) {
    	bool success = getResult();
    	if( success ) {
    		//itsPlot_->parent()->getPlotter()->showMessage(
    		itsPlotter_->showMessage(
                "Successfully exported plot to " + exportLocation + "!",
                "Export Success");
    	}
    	else {
    		//itsPlot_->parent()->getPlotter()->showError(
    		itsPlotter_->showError(
                "There was a problem exporting to file " + exportLocation +
                "!", "Export Error", false);
    	}
    }
    itsMutex_.unlock();

    //emit finishedOperation(this);
    signalFinishedOperation( this );
}
}



