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

#include <QApplication>
#include <QDesktopWidget>

namespace casa {

//////////////////////////////////
// PLOTMSPARAMETERS DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

PlotMSParameters::PlotMSParameters(PlotMSLogger::Level logLevel, bool debug,
        bool clearSelection, int cachedImageWidth, int cachedImageHeight) :
        itsLogLevel_(logLevel), itsLogDebug_(debug),
        itsClearSelectionsOnAxesChange_(clearSelection),
        itsCachedImageWidth_(cachedImageWidth),
        itsCachedImageHeight_(cachedImageHeight) { }

PlotMSParameters::PlotMSParameters(const PlotMSParameters& copy) :
        PlotMSWatchedParameters(copy) {
    operator=(copy);
}

PlotMSParameters::~PlotMSParameters() { }


// Public Methods //

PlotMSLogger::Level PlotMSParameters::logLevel() const { return itsLogLevel_; }
bool PlotMSParameters::logDebug() const { return itsLogDebug_; }

void PlotMSParameters::setLogLevel(PlotMSLogger::Level level, bool debug) {
    if(level != itsLogLevel_ || debug != itsLogDebug_) {
        itsLogLevel_ = level;
        itsLogDebug_ = debug;
        updateFlag(LOG, true, false);
    }
}

bool PlotMSParameters::clearSelectionsOnAxesChange() const {
    return itsClearSelectionsOnAxesChange_; }
void PlotMSParameters::setClearSelectionsOnAxesChange(bool flag) {
    if(flag != itsClearSelectionsOnAxesChange_) {
        itsClearSelectionsOnAxesChange_ = flag;
        // it's not actually a change to LOG, but use that for now..
        updateFlag(LOG, true, false);
    }
}

pair<int, int> PlotMSParameters::cachedImageSize() const {
    return pair<int, int>(itsCachedImageWidth_, itsCachedImageHeight_); }
void PlotMSParameters::setCachedImageSize(int width, int height) {
    if(width != itsCachedImageWidth_ || height != itsCachedImageHeight_) {
        itsCachedImageWidth_ = width;
        itsCachedImageHeight_ = height;
        // it's not actually a change to LOG, but use that for now..
        updateFlag(LOG, true, false);
    }
}

void PlotMSParameters::setCachedImageSizeToResolution() {
    QRect res = QApplication::desktop()->screenGeometry();
    setCachedImageSize(res.width(), res.height());
}


bool PlotMSParameters::equals(const PlotMSWatchedParameters& other,
        int updateFlags) const {
    const PlotMSParameters* o = dynamic_cast<const PlotMSParameters*>(&other);
    if(o == NULL) return false;
    
    if(updateFlags & LOG) return itsLogLevel_ == o->itsLogLevel_ &&
                                 itsLogDebug_ == o->itsLogDebug_ &&
                                 itsClearSelectionsOnAxesChange_ ==
                                     o->itsClearSelectionsOnAxesChange_ &&
                                 itsCachedImageWidth_ ==
                                     o->itsCachedImageWidth_ &&
                                 itsCachedImageHeight_ ==
                                     o->itsCachedImageHeight_;
    else                  return false;
}

PlotMSParameters& PlotMSParameters::operator=(const PlotMSParameters& copy) {
    PlotMSWatchedParameters::operator=(copy);
    setLogLevel(copy.logLevel(), copy.logDebug());
    setClearSelectionsOnAxesChange(copy.clearSelectionsOnAxesChange());
    pair<int, int> size = copy.cachedImageSize();
    setCachedImageSize(size.first, size.second);
    return *this;
}

}

