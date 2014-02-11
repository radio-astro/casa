//# PlotMSParameters.cc: Parameter classes for plotms.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#include <plotms/PlotMS/PlotMSParameters.h>

#include <casaqt/QtUtilities/QtFileDialog.qo.h>

#include <QApplication>
#include <QDesktopWidget>

namespace casa {

//////////////////////////////////
// PLOTMSPARAMETERS DEFINITIONS //
//////////////////////////////////

// Static //
const int DummyClass::dummyDraw = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("REDRAW");
const int DummyClass::dummyData = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("MSDATA");
const int DummyClass::dummyCache = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("CACHE");
const int DummyClass::dummyAxes = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("AXES");
const int DummyClass::dummyCanvas = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("CANVAS");
const int DummyClass::dummyDisplay = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("DISPLAY");
const int DummyClass::dummyIter = PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("ITERATION");


const int PlotMSParameters::UPDATE_LOG =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("LOG");
const int PlotMSParameters::UPDATE_PLOTMS_OPTIONS =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG("PLOTMS_OPTIONS");

int PlotMSParameters::chooserHistoryLimit() {
    return QtFileDialog::historyLimit(); }
void PlotMSParameters::setChooserListoryLimit(int histLimit) {
    QtFileDialog::setHistoryLimit(histLimit); }


// Constructors/Destructors //

PlotMSParameters::PlotMSParameters(const String& logFilename, int logEvents,
        LogMessage::Priority logPriority, bool clearSelections,
        int cachedImageWidth, int cachedImageHeight, int rows, int cols) :
        itsLogFilename_(logFilename), itsLogEvents_(logEvents),
        itsLogPriority_(logPriority),
        itsClearSelectionsOnAxesChange_(clearSelections),
        itsCachedImageWidth_(cachedImageWidth),
        itsCachedImageHeight_(cachedImageHeight) {
	rowCount = rows;
	colCount = cols;
}

PlotMSParameters::PlotMSParameters(const PlotMSParameters& copy)
    : PlotMSWatchedParameters(copy) {
    operator=(copy);
}

PlotMSParameters::~PlotMSParameters() { }


// Public Methods //

String PlotMSParameters::logFilename() const { return itsLogFilename_; }
void PlotMSParameters::setLogFilename(const String& filename) {
    if(filename != itsLogFilename_) {
        itsLogFilename_ = filename;
        updateFlag(UPDATE_LOG);
    }
}

int PlotMSParameters::logEvents() const { return itsLogEvents_; }
LogMessage::Priority PlotMSParameters::logPriority() const {
    return itsLogPriority_; }

void PlotMSParameters::setLogFilter(int e, LogMessage::Priority p) {
    if(e != itsLogEvents_ || p != itsLogPriority_) {
        itsLogEvents_ = e;
        itsLogPriority_ = p;
        updateFlag(UPDATE_LOG);
    }
}

bool PlotMSParameters::clearSelectionsOnAxesChange() const {
    return itsClearSelectionsOnAxesChange_; }
void PlotMSParameters::setClearSelectionsOnAxesChange(bool flag) {
    if(flag != itsClearSelectionsOnAxesChange_) {
        itsClearSelectionsOnAxesChange_ = flag;
        updateFlag(UPDATE_PLOTMS_OPTIONS);
    }
}

pair<int, int> PlotMSParameters::cachedImageSize() const {
    return pair<int, int>(itsCachedImageWidth_, itsCachedImageHeight_); }
void PlotMSParameters::setCachedImageSize(int width, int height) {
    if(width != itsCachedImageWidth_ || height != itsCachedImageHeight_) {
        itsCachedImageWidth_ = width;
        itsCachedImageHeight_ = height;
        updateFlag(UPDATE_PLOTMS_OPTIONS);
    }
}

void PlotMSParameters::setCachedImageSizeToResolution() {
    QRect res = QApplication::desktop()->screenGeometry();
    setCachedImageSize(res.width(), res.height());
}


void PlotMSParameters::setRowCount(int newRowCount ){
	if ( rowCount != newRowCount ){
		rowCount = newRowCount;
		updateFlag(UPDATE_PLOTMS_OPTIONS);
	}
}


int PlotMSParameters::getRowCount() const {
	return rowCount;
}


void PlotMSParameters::setColCount( int newColCount ){
	if ( colCount != newColCount ){
		colCount = newColCount;
		updateFlag(UPDATE_PLOTMS_OPTIONS);
	}
}

bool PlotMSParameters::setGridSize( int newRowCount, int newColCount ){
	bool changedGridSize = false;
	if ( colCount != newColCount || rowCount != newRowCount ){
		colCount = newColCount;
		rowCount = newRowCount;
		changedGridSize = true;
		updateFlag(UPDATE_PLOTMS_OPTIONS );
	}
	return changedGridSize;
}

int PlotMSParameters::getColCount() const {
	return colCount;
}

PlotMSParameters& PlotMSParameters::operator=(const PlotMSParameters& copy) {
    int update = currentUpdateFlag();
    
    if(!equals(copy, UPDATE_LOG)) {
        itsLogFilename_ = copy.itsLogFilename_;
        itsLogEvents_ = copy.itsLogEvents_;
        itsLogPriority_ = copy.itsLogPriority_;
        update |= UPDATE_LOG;
    }
    
    if(!equals(copy, UPDATE_PLOTMS_OPTIONS)) {
        itsClearSelectionsOnAxesChange_ = copy.itsClearSelectionsOnAxesChange_;
        itsCachedImageWidth_ = copy.itsCachedImageWidth_;
        itsCachedImageHeight_ = copy.itsCachedImageHeight_;
        rowCount = copy.rowCount;
        colCount = copy.colCount;
        update |= UPDATE_PLOTMS_OPTIONS;
    }

    updateFlags(update);
    return *this;
}

bool PlotMSParameters::equals(const PlotMSWatchedParameters& other, int updateFlags) const {
	bool equalParams = true;
    const PlotMSParameters* o = dynamic_cast<const PlotMSParameters*>(&other);
    if(o == NULL){
    	equalParams = false;
    }
    
    if(updateFlags & UPDATE_LOG){
        if(itsLogFilename_ != o->itsLogFilename_ ||
           itsLogEvents_ != o->itsLogEvents_ ||
           itsLogPriority_ != o->itsLogPriority_){
        	equalParams = false;
        }
    }
    
    if(updateFlags & UPDATE_PLOTMS_OPTIONS){
        if(itsClearSelectionsOnAxesChange_!= o->itsClearSelectionsOnAxesChange_
           || itsCachedImageWidth_ != o->itsCachedImageWidth_ ||
           itsCachedImageHeight_ != o->itsCachedImageHeight_ ||
           rowCount != o->rowCount || colCount != o->colCount){
        	equalParams = false;
        }
    }


    return equalParams;
}

}

