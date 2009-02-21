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

#include <plotms/PlotMS/PlotMSPlot.h>

namespace casa {

/////////////////////////////////////////
// PLOTMSWATCHEDPARAMETERS DEFINITIONS //
/////////////////////////////////////////

// Constructors/Destructors //

PlotMSWatchedParameters::PlotMSWatchedParameters() : itsUpdateFlags_(NONE),
        itsRedrawFlag_(false), itsUpdater_(NULL) { }

PlotMSWatchedParameters::PlotMSWatchedParameters(
        const PlotMSWatchedParameters& copy) : itsUpdateFlags_(NONE),
        itsRedrawFlag_(false), itsUpdater_(NULL) {
    operator=(copy);
}

PlotMSWatchedParameters::~PlotMSWatchedParameters() { }


// Public Methods //

void PlotMSWatchedParameters::addWatcher(PlotMSParametersWatcher* watcher) {
    if(watcher == NULL) return;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        if(itsWatchers_[i] == watcher) return;
    itsWatchers_.push_back(watcher);
}

void PlotMSWatchedParameters::removeWatcher(PlotMSParametersWatcher* watcher) {
    if(watcher == NULL) return;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++) {
        if(itsWatchers_[i] == watcher) {
            itsWatchers_.erase(itsWatchers_.begin() + i);
            return;
        }
    }
}

void PlotMSWatchedParameters::holdNotification(PlotMSParametersWatcher* updater) {
    itsUpdater_ = updater;
}

void PlotMSWatchedParameters::releaseNotification() {
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        if(itsUpdater_ == NULL || itsUpdater_ != itsWatchers_[i])
            itsWatchers_[i]->parametersHaveChanged(*this, itsUpdateFlags_,
                                                   itsRedrawFlag_);

    itsUpdateFlags_ = NONE;
    itsRedrawFlag_ = false;
    itsUpdater_ = NULL;
}

void PlotMSWatchedParameters::notifyWatchers(int updateFlags,
        bool redrawRequired, PlotMSParametersWatcher* updater) {
    holdNotification(updater);
    itsUpdateFlags_ = updateFlags;
    itsRedrawFlag_ = redrawRequired;
    releaseNotification();
}

PlotMSWatchedParameters& PlotMSWatchedParameters::operator=(
            const PlotMSWatchedParameters& copy) {
    //itsUpdateFlags_ = copy.itsUpdateFlags_;
    //itsWatchers_ = copy.itsWatchers_;
    //itsUpdater_ = NULL;
    return *this;
}


// Protected Methods //

int PlotMSWatchedParameters::currentUpdateFlag() const {
    return itsUpdateFlags_; }
bool PlotMSWatchedParameters::currentRedrawRequired() const {
    return itsRedrawFlag_; }

void PlotMSWatchedParameters::updateFlag(UpdateFlag update, bool on,
        bool redrawRequired) {
    bool changed = false;
    
    if(on) {
        changed = !(itsUpdateFlags_ & update);
        if(changed) itsUpdateFlags_ |= update;
    } else {
        changed = itsUpdateFlags_ & update;
        if(changed) itsUpdateFlags_ &= ~update;
    }

    if(itsRedrawFlag_ != (itsRedrawFlag_ || redrawRequired)) {
        itsRedrawFlag_ |= redrawRequired;
        changed = true;
    }
    
    // Only notify watchers if something changed and notifications aren't
    // currently being held.
    if(changed && itsUpdater_ == NULL) releaseNotification();
}

void PlotMSWatchedParameters::updateFlags(int updateFlags, bool redrawRequired) {
    if(itsUpdateFlags_ != updateFlags ||
       itsRedrawFlag_ != (itsRedrawFlag_ || redrawRequired)) {
        itsUpdateFlags_ = updateFlags;
        itsRedrawFlag_ |= redrawRequired;
        
        // Only notify watchers if notifications aren't currently being held.
        if(itsUpdater_ == NULL) releaseNotification();
    }
}


//////////////////////////////////
// PLOTMSPARAMETERS DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

PlotMSParameters::PlotMSParameters(PlotMSLogger::Level logLevel) :
        itsLogLevel_(logLevel) { }

PlotMSParameters::PlotMSParameters(const PlotMSParameters& copy) :
        PlotMSWatchedParameters(copy) {
    operator=(copy);
}

PlotMSParameters::~PlotMSParameters() { }


// Public Methods //

PlotMSLogger::Level PlotMSParameters::logLevel() const { return itsLogLevel_; }
void PlotMSParameters::setLogLevel(PlotMSLogger::Level level) {
    if(level != itsLogLevel_) {
        itsLogLevel_ = level;
        updateFlag(LOG, true, false);
    }
}

bool PlotMSParameters::equals(const PlotMSWatchedParameters& other,
        int updateFlags) const {
    const PlotMSParameters* o = dynamic_cast<const PlotMSParameters*>(&other);
    if(o == NULL) return false;
    
    if(updateFlags & LOG) return itsLogLevel_ == o->itsLogLevel_;
    else                  return false;
}

PlotMSParameters& PlotMSParameters::operator=(const PlotMSParameters& copy) {
    PlotMSWatchedParameters::operator=(copy);
    setLogLevel(copy.logLevel());
    return *this;
}

}

