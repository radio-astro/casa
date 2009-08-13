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
#include <plotms/Actions/PlotMSCacheThread.qo.h>

#include <msvis/MSVis/VisSet.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

namespace casa {

///////////////////////////////////
// PLOTMSCACHETHREAD DEFINITIONS //
///////////////////////////////////

PlotMSCacheThread::PlotMSCacheThread(PlotMSPlot* plot, PlotMSData* data,
        const vector<PMS::Axis>& axes, const vector<PMS::DataColumn>& dataCols,
        const PlotMSAveraging& averaging, bool setupPlot,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget(),
        postThreadMethod, postThreadObject), itsPlot_(plot),
        itsData_(data), itsVisSet_(plot->visSet()), itsLoad_(true),
        itsAxes_(axes), itsAxesData_(dataCols), itsAveraging_(averaging),
        itsSetupPlot_(setupPlot && axes.size() >= 2), wasCanceled_(false) {
    // Make sure axes data vector is same length as axes vector.
    if(itsAxesData_.size() != itsAxes_.size())
        itsAxesData_.resize(itsAxes_.size(), PMS::DEFAULT_DATACOLUMN);
}

PlotMSCacheThread::PlotMSCacheThread(PlotMSPlot* plot,
        const vector<PMS::Axis>& axes, PMSPTMethod postThreadMethod,
        PMSPTObject postThreadObject) :
        PlotMSThread(plot->parent()->getPlotter()->getProgressWidget(),
        postThreadMethod, postThreadObject), itsPlot_(plot),
        itsData_(&plot->data()), itsVisSet_(plot->visSet()), itsLoad_(false),
        itsAxes_(axes), itsSetupPlot_(false), wasCanceled_(false) { }

PlotMSCacheThread::~PlotMSCacheThread() { }


void PlotMSCacheThread::startOperation() {
    if(itsVisSet_ == NULL) {
        emit finishedOperation(this);
        return;
    }

    itsLastProgress_ = 0;
    itsLastStatus_ = "";
    
    initializeProgressWidget(itsLoad_ ? PMS::LOG_ORIGIN_LOAD_CACHE :
                                        PMS::LOG_ORIGIN_RELEASE_CACHE);
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
    
    // Show error message if one occurred, and say the thread was canceled so
    // that the plot won't update the display.
    if(!itsCacheError_.empty()) {
        itsPlot_->parent()->showError(itsCacheError_,
                itsLoad_ ? "Load Error" : "Release Error");
        wasCanceled_ = true;
    }
    
    emit finishedOperation(this);
}


/////////////////////////////////////////
// PLOTMSCACHETHREADHELPER DEFINITIONS //
/////////////////////////////////////////

PlotMSCacheThreadHelper::PlotMSCacheThreadHelper(PlotMSCacheThread& parent) :
        itsParent_(parent) { }

PlotMSCacheThreadHelper::~PlotMSCacheThreadHelper() { }    

void PlotMSCacheThreadHelper::run() {
    try {
        // Load
        if(itsParent_.itsLoad_) {
            itsParent_.itsData_->loadCache(*itsParent_.itsVisSet_,
                    itsParent_.itsAxes_, itsParent_.itsAxesData_,
                    itsParent_.itsAveraging_, &itsParent_);
            if(itsParent_.itsSetupPlot_)
                itsParent_.itsData_->setupCache(itsParent_.itsAxes_[0],
                                                itsParent_.itsAxes_[1]);
        
        // Release
        } else {
            itsParent_.itsData_->releaseCache(itsParent_.itsAxes_);
        }
        
    } catch(AipsError& err) {
        itsParent_.itsCacheError_ = "Error during cache ";
        itsParent_.itsCacheError_+=itsParent_.itsLoad_? "loading": "releasing";
        itsParent_.itsCacheError_ += ": " + err.getMesg();
    } catch(...) {
        itsParent_.itsCacheError_ = "Unknown error during cache ";
        itsParent_.itsCacheError_+=itsParent_.itsLoad_? "loading": "releasing";
        itsParent_.itsCacheError_+="!";
    }
}

}
