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
#include <plotms/Threads/Gui/PlotMSThread.qo.h>

#include <casaqt/QtUtilities/QtProgressWidget.qo.h>
#include <graphics/GenericPlotter/ProgressMonitor.h>

#include <QTime>
#include <QDebug>
namespace casa {

//////////////////////////////
// PLOTMSTHREAD DEFINITIONS //
//////////////////////////////

PlotMSThread::PlotMSThread(QtProgressWidget* progress,
        PMSPTMethod postThreadMethod, PMSPTObject postThreadObject)
        : ThreadController( progress, postThreadMethod, postThreadObject ) {
    connect(progress, SIGNAL(backgroundRequested()), SLOT(backgroundThread()));
    connect(progress, SIGNAL(pauseRequested()), SLOT(pauseThread()));
    connect(progress, SIGNAL(resumeRequested()), SLOT(resumeThread()));
    connect(progress, SIGNAL(cancelRequested()), SLOT(cancelThread()));
    
    //Signal/slot calls rather than direct calls so that the updates go on
    //the GUI thread
    connect(this, SIGNAL(initProgress(const QString&)),
            progress, SLOT(initialize(const QString&)));
    connect(this, SIGNAL(updateProgress(unsigned int, const QString&)),
            progress, SLOT(setProgress(unsigned int, const QString&)));
    connect(this, SIGNAL(finalizeProgress()), progress, SLOT(finalize()));

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    id = qrand() % 100;
}


void PlotMSThread::initializeProgress(const String& operationName){
	QString opName( operationName.c_str() );
	emit initProgress( opName );
}

void PlotMSThread::setProgress(unsigned int progress, const String& status){
	QString statusStr( status.c_str() );
	emit updateProgress( progress, statusStr );
}

void PlotMSThread::finishProgress(){
	emit finalizeProgress();
}

void PlotMSThread::finished( ){
	threadFinished();
}

void PlotMSThread::signalFinishedOperation(PlotMSThread* thread ){
	emit finishedOperation( thread );
}

// For when the user requests "background" for the thread.
void PlotMSThread::backgroundThread(){
	background();
}

	// For when the user requests "pause" for the thread.
void PlotMSThread::pauseThread(){
	pause();
}

// For when the user requests "resume" for the thread.
void PlotMSThread::resumeThread(){
	resume();
}

// For when the user requests "cancel" for the thread.
void PlotMSThread::cancelThread(){
	cancel();
}

int PlotMSThread::getId(){
	return id;
}

bool PlotMSThread::isCacheThread() const {
	return false;
}


PlotMSThread::~PlotMSThread() { }

}


