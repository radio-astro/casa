//# PlotMSWatchedParameters.cc: Classes for watched/synchronized parameters.
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
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

namespace casa {

/////////////////////////////////////////
// PLOTMSWATCHEDPARAMETERS DEFINITIONS //
/////////////////////////////////////////

// Static //

const int PlotMSWatchedParameters::NO_UPDATES = 0;
vector<int> PlotMSWatchedParameters::FLAGS = vector<int>();
vector<String> PlotMSWatchedParameters::NAMES = vector<String>();


int PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(const String& name) {
    // Check if it's already registered.
    for(unsigned int i = 0; i < NAMES.size(); i++)
        if(NAMES[i] == name) return FLAGS[i];
    
    // Get the next flag value (next sequential bit value after the last).
    int NEXT_FLAG = 1;
    if(FLAGS.size() > 0) NEXT_FLAG = FLAGS[FLAGS.size() - 1] * 2;
    
    // Add new flag.
    FLAGS.push_back(NEXT_FLAG);
    NAMES.push_back(name);
    return NEXT_FLAG;
    
    /* Old version.
    static int NEXT_FLAG = 1;
    for(unsigned int i = 0; i < NAMES.size(); i++)
        if(NAMES[i] == name) return FLAGS[i];
        
    FLAGS.push_back(NEXT_FLAG);
    NAMES.push_back(name);
    NEXT_FLAG *= 2;
    return FLAGS[FLAGS.size() - 1];
    */
    
    /* Wes's magic version.
    int NEXT_FLAG = 1;
    if(!NAMES.size() && name != "REDRAW") {
        NAMES.push_back("REDRAW");
        FLAGS.push_back(NEXT_FLAG);
    }
    for(unsigned int i = 0; i < NAMES.size(); i++) {
        NEXT_FLAG *= 2;
        if(NAMES[i] == name) return FLAGS[i];
    }
    FLAGS.push_back(NEXT_FLAG);
    NAMES.push_back(name);
    return FLAGS[FLAGS.size() - 1];
    */
}

void PlotMSWatchedParameters::UNREGISTER_UPDATE_FLAG(const String& name) {
	// cerr << "UNREGISTER_UPDATE_FLAG " << name << endl;
    for(unsigned int i = 0; i < NAMES.size(); i++) {
        if(NAMES[i] == name) {
            NAMES.erase(NAMES.begin() + i);
            FLAGS.erase(FLAGS.begin() + i);
            break;
        }
    }
}
void PlotMSWatchedParameters::UNREGISTER_UPDATE_FLAG(int flag) {
	// cerr << "UNREGISTER_UPDATE_FLAG " << flag << endl;
    for(unsigned int i = 0; i < FLAGS.size(); i++) {
        if(FLAGS[i] == flag) {
            FLAGS.erase(FLAGS.begin() + i);
            NAMES.erase(NAMES.begin() + i);
            break;
        }
    }
}


int PlotMSWatchedParameters::UPDATE_FLAG(const String& name) {
    for(unsigned int i = 0; i < NAMES.size(); i++){
	// cerr << "UPDATE_FLAG *" << NAMES[i] << "*" << endl;
        if(NAMES[i] == name) return FLAGS[i];
    }
    // cerr << "UPDATE_FLAG *" << name << "*" << endl;
    return NO_UPDATES;
}

String PlotMSWatchedParameters::UPDATE_FLAG(int flag) {
    for(unsigned int i = 0; i < FLAGS.size(); i++)
        if(FLAGS[i] == flag) return NAMES[i];
    return "";
}

vector<int> PlotMSWatchedParameters::UPDATE_FLAGS() { return FLAGS; }
vector<String> PlotMSWatchedParameters::UPDATE_FLAG_NAMES() { return NAMES; }

int PlotMSWatchedParameters::ALL_UPDATE_FLAGS() {
    int val = NO_UPDATES;
    //cerr << "ALL_UPDATE_FLAGS ";
    for(unsigned int i = 0; i < FLAGS.size(); i++){
	    val |= FLAGS[i];
	    // cerr << i << " ";
    }
    //cerr << val << endl;
    return val;
}

vector<int> PlotMSWatchedParameters::UPDATE_FLAGS(int value) {
    vector<int> v;
    for(unsigned int i = 0; i < FLAGS.size(); i++)
        if(value & FLAGS[i]) v.push_back(FLAGS[i]);
    return v;
}
vector<String> PlotMSWatchedParameters::UPDATE_FLAG_NAMES(int value) {
    vector<String> v;
    for(unsigned int i = 0; i < FLAGS.size(); i++)
        if(value & FLAGS[i]) v.push_back(NAMES[i]);
    return v;
}


// Constructors/Destructors //

PlotMSWatchedParameters::PlotMSWatchedParameters():itsUpdateFlags_(NO_UPDATES),
        isHolding_(false), itsUpdater_(NULL) { }

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
    isHolding_ = true;
    itsUpdater_ = updater; }

void PlotMSWatchedParameters::releaseNotification() {
    isHolding_ = false;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        if(itsUpdater_ == NULL || itsUpdater_ != itsWatchers_[i])
            itsWatchers_[i]->parametersHaveChanged(*this, itsUpdateFlags_);

    itsUpdateFlags_ = NO_UPDATES;
    itsUpdater_ = NULL;
}


// Protected Methods //

int PlotMSWatchedParameters::currentUpdateFlag() const {
    return itsUpdateFlags_; }

void PlotMSWatchedParameters::updateFlag(int updateFlag, bool on) {
    // Make sure given flag is registered.
    bool found = false;
    for(unsigned int i = 0; !found && i < FLAGS.size(); i++)
        if(FLAGS[i] == updateFlag) found = true;
    if(!found) return;
    
    bool changed = false;
    
    //cerr << "before itsUpdateFlags_ " << itsUpdateFlags_ << endl;
    if(on) {
        changed = !(itsUpdateFlags_ & updateFlag);
        if(changed) itsUpdateFlags_ |= updateFlag;
    } else {
        changed = itsUpdateFlags_ & updateFlag;
        if(changed) itsUpdateFlags_ &= ~updateFlag;
    }
    //cerr << "after itsUpdateFlags_ " << itsUpdateFlags_ << endl;
    
    // Only notify watchers if something changed and notifications aren't
    // currently being held.
    if(changed && !isHolding_) releaseNotification();
}

void PlotMSWatchedParameters::updateFlag(const String& updateFlagName,bool on){
    updateFlag(UPDATE_FLAG(updateFlagName), on); }

void PlotMSWatchedParameters::updateFlags(int updateFlags) {
    if(itsUpdateFlags_ != updateFlags) {
        //cerr << "before updateFlags " << itsUpdateFlags_ << endl;
        itsUpdateFlags_ = updateFlags;
        //cerr << "after updateFlags " << itsUpdateFlags_ << endl;
        
        // Only notify watchers if notifications aren't currently being held.
        if(!isHolding_) releaseNotification();
    }
}

}
