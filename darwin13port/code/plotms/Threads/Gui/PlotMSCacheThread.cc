//# PlotMSCacheThread.cc: Subclass of PlotMSThread for cache loading.
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
#include <plotms/Threads/Gui/PlotMSCacheThread.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
//#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Data/PlotMSCacheBase.h>
#include <QDebug>

namespace casa {

///////////////////////////////////
// PLOTMSCACHETHREAD DEFINITIONS //
///////////////////////////////////

PlotMSCacheThread::PlotMSCacheThread(QtProgressWidget* progress, PlotMSPlotter* plotter,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        PlotMSThread( progress, postThreadMethod, postThreadObject ),
        itsPlotter_( plotter ){
}



PlotMSCacheThread::~PlotMSCacheThread() { }


void PlotMSCacheThread::startOperation() {

    itsLastProgress_ = 0;
    itsLastStatus_ = "";
    
    initializeProgress(itsLoad_ ? PMS::LOG_ORIGIN_LOAD_CACHE :
                                        PMS::LOG_ORIGIN_RELEASE_CACHE);
    setAllowedOperations(false, false, true);
    
    /*PlotMSCacheThreadHelper* th = new PlotMSCacheThreadHelper(*this);
    
    // Connect QThread signals.
    connect(th, SIGNAL(finished()), SLOT(threadFinished()));
    connect(th, SIGNAL(terminated()), SLOT(threadFinished()));
    connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));
    connect(th, SIGNAL(terminated()), th, SLOT(deleteLater()));
    
    th->start();*/
    startThread();
}

void PlotMSCacheThread::cancelOperation(){
	cancelThread();
}

void PlotMSCacheThread::setProgressAndStatus(unsigned int progress,
        const String& status) {
    if(progress != itsLastProgress_ || status != itsLastStatus_) {
        itsLastProgress_ = progress;
        itsLastStatus_ = status;
        PlotMSThread::setProgress(itsLastProgress_, itsLastStatus_);
    }
}




void PlotMSCacheThread::threadFinished() {
    finalizeProgress();
    // Show error message if one occurred, and say the thread was canceled so
    // that the plot won't update the display.
    bool success = getResult();
    if( !success ) {
    	String errorOperation( "Load Error ");
    	if ( !itsLoad_ ){
    		errorOperation = "ReleaseError";
    	}
        //itsPlot_->parent()->showError(error,
        //        itsLoad_ ? "Load Error" : "Release Error");
    	if ( itsPlotter_ != NULL ){
    		itsPlotter_->showError( error, errorOperation, false);
    	}
        wasCanceled_ = true;
    }
    
    //emit finishedOperation(this);
    signalFinishedOperation( this );
}




}
