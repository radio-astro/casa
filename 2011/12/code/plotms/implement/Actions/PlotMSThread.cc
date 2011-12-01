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

#include <casaqt/QtUtilities/QtProgressWidget.qo.h>

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

}
