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

QString PlotMSCacheThread::getName() const {
	return "Cache Thread";
}

void PlotMSCacheThread::startOperation() {

    itsLastProgress_ = 0;
    itsLastStatus_ = "";
    
    initializeProgress(itsLoad_ ? PMS::LOG_ORIGIN_LOAD_CACHE :
                                        PMS::LOG_ORIGIN_RELEASE_CACHE);
    setAllowedOperations(false, false, true);
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

bool PlotMSCacheThread::isCacheThread() const {
	return true;
}


void PlotMSCacheThread::threadFinished() {
    finalizeProgress();
    // Show error message if one occurred, and say the thread was canceled so
    // that the plot won't update the display.
    bool success = getResult();
    if( !success ) {
    	errorTitle = "Load Error ";
    	if ( !itsLoad_ ){
    		errorTitle = "ReleaseError";
    	}
        errorWarning = true;
        wasCanceled_ = true;
    }
    
    signalFinishedOperation( this );
}




}
