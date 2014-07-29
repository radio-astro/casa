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

PlotMSExportThread::PlotMSExportThread( QtProgressWidget* progressWidget,
		PlotMSPlotter* plotter, const String& location,
		PMSPTMethod postThreadMethod, PMSPTObject postThreadObject)
	: PlotMSThread(progressWidget,postThreadMethod, postThreadObject),
	  itsPlotter_( plotter ){

	exportLocation = location;
	itsPlots_ = itsPlotter_->getCurrentPlots();

	int plotCount = itsPlots_.size();
	itsOperations_.resize( plotCount );
	for ( int j = 0; j < plotCount; j++ ){
		vector<PlotCanvasPtr> canvases = itsPlots_[j]->canvases();
		itsOperations_[j].resize(canvases.size());
		for(unsigned int i = 0; i < canvases.size(); i++) {
			itsOperations_[j][i] = canvases[i]->operationExport();
			itsOperations_[j][i]->addWatcher(this);
		}
	}
}

PlotMSExportThread::~PlotMSExportThread() {
	int opCount = itsOperations_.size();
	for ( int j = 0; j < opCount; j++ ){
		int opCountJ = itsOperations_[j].size();
		for(int i = 0; i < opCountJ; i++){
			itsOperations_[j][i]->removeWatcher(this);
		}
	}
}


void PlotMSExportThread::startOperation() {
	if(itsPlots_.size() == 0) {
    	signalFinishedOperation( this );
        return;
    }

    initializeProgress(PlotCanvas::OPERATION_EXPORT);
    setAllowedOperations(false, false, true);
    startThread();
    //threadFinished();
}

QString PlotMSExportThread::getName() const {
	return "Export Thread";
}
void PlotMSExportThread::operationChanged(const PlotOperation& operation) {
    if(!itsMutex_.tryLock()) return; // Only happens on a cancel.

    String status = operation.currentStatus();
    double progress = 0;
    for ( unsigned int j = 0; j < itsOperations_.size(); j++ ){
    for(unsigned int i = 0; i < itsOperations_[j].size(); i++){
        progress += itsOperations_[j][i]->inProgress() ?
                    itsOperations_[j][i]->currentProgress() : 100;
    }
    }

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
    		itsPlotter_->showMessage(
                "Successfully exported plot to " + exportLocation + "!",
                "Export Success");
    	}
    	else {
    		itsPlotter_->showError(
                "There was a problem exporting to file " + exportLocation +
                ".", "Export Error", false);
    	}
    }
    itsMutex_.unlock();

    //emit finishedOperation(this);
    signalFinishedOperation( this );
}
}



