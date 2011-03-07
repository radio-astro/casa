//# PlotMSWatchedParameters.h: Classes for watched/synchronized parameters.
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
#ifndef PLOTMSWATCHEDPARAMETERS_H_
#define PLOTMSWATCHEDPARAMETERS_H_

#include <casa/BasicSL/String.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class PlotMSWatchedParameters;


// Interface for classes that wish to be notified when PlotMSWatchedParameters
// have changed.  This watching system is used to keep the different aspects of
// PlotMS synchronized with parameters that could potentially come from many
// different sources.
class PlotMSParametersWatcher {
public:
    // Constructor.
    PlotMSParametersWatcher() { }
    
    // Destructor.
    virtual ~PlotMSParametersWatcher() { }
    
    
    // This method is called whenever the watched parameters have been changed.
    // This can either happen immediately after any change or, if notification
    // has been held via PlotMSWatchedParameters::holdNotification, when
    // notification is released via
    // PlotMSWatchedParameters::releaseNotification.  If this watcher is the
    // watcher that was holding notifications, this method is NOT called.
    // The updateFlag parameter lets the watcher know which categories the
    // changes were in.
    virtual void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag) = 0;
};


// Abstract class for parameters that may be watched by one or more interested
// classes.  Any subclass is assumed to have different properties in one or
// more of the update categories defined different update flag values which
// must be registered with the public static methods in
// PlotMSWatchedParameters.  Using this system, any classes watching the
// parameters for changes can be notified which categories the changes occurred
// in.
class PlotMSWatchedParameters {
public:
    // Static //
    
    // "Base", or no updates, flag.
    static const int NO_UPDATES;
    
    
    // Registers an update flag with the given name (if it is not already
    // registered) and returns its flag value.
    static int REGISTER_UPDATE_FLAG(const String& name);
    
    // Unregisters the given update flag, if it is registered.
    // <group>
    static void UNREGISTER_UPDATE_FLAG(const String& name);
    static void UNREGISTER_UPDATE_FLAG(int flag);
    // </group>
    
    // Converts between an update flag's name and value, if valid.
    // <group>
    static int UPDATE_FLAG(const String& name);
    static String UPDATE_FLAG(int flag);
    // </group>
    
    // Returns all registered update flags.
    // <group>
    static vector<int> UPDATE_FLAGS();
    static vector<String> UPDATE_FLAG_NAMES();
    // </group>
    
    // Returns all registered update flags as one or-ed value.
    static int ALL_UPDATE_FLAGS();
    
    // Returns all registered update flags that were turned on in the given
    // flags value.
    // <group>
    static vector<int> UPDATE_FLAGS(int value);
    static vector<String> UPDATE_FLAG_NAMES(int value);
    // </group>
    
    
    // Non-Static //
    
    // Constructor.
    PlotMSWatchedParameters();
    
    // Destructor.
    virtual ~PlotMSWatchedParameters();
    
    
    // Adds/Removes the given watcher for this PlotMSParameters.
    // <group>
    void addWatcher(PlotMSParametersWatcher* watcher);
    void removeWatcher(PlotMSParametersWatcher* watcher);
    // </group>
    
    // Holds update notifications for any registered watchers.  Notifications
    // will not be sent out until releaseNotification() is called.  If a
    // non-NULL watcher is given, it will be excluded from notifications when
    // releaseNotification() is called.
    void holdNotification(PlotMSParametersWatcher* updater = NULL);
    
    // Releases update notification; notifies all watchers of an update except
    // for the one (if any) that called holdNotification.
    void releaseNotification();
    
    // Equality operators.
    // <group>
    virtual bool operator==(const PlotMSWatchedParameters& other) const {
        return equals(other, ALL_UPDATE_FLAGS()); }
    virtual bool operator!=(const PlotMSWatchedParameters& other) const {
        return !(operator==(other)); }
    // </group>
    
    
    // ABSTRACT METHODS //
    
    // Returns true if this PlotMSParameters equals the other, in the given
    // update categories flag.
    virtual bool equals(const PlotMSWatchedParameters& other,
                        int updateFlags) const = 0;    

protected:
    // Returns the current update flag.
    int currentUpdateFlag() const;
    
    // Provides access to children to indicate whether the given update flag
    // should be turned on or off.  This should be used by setter functions to
    // classify the changes appropriately.  If update notifications are NOT
    // being held, any watchers will immediately be notified of the change.
    // <group>
    void updateFlag(int updateFlag, bool on = true);
    void updateFlag(const String& updateFlagName, bool on = true);
    // </group>
    
    // Provides access to children to indicate which update flags are on.
    // The given value should be a bit-wise or of one or more UpdateFlag
    // enum values.
    void updateFlags(int updateFlags);
    
private:
    // Current update flags.
    int itsUpdateFlags_;
    
    // Watchers.
    vector<PlotMSParametersWatcher*> itsWatchers_;
    
    // Flag for whether notifications are currently being held or not.
    bool isHolding_;
    
    // Watcher that is currently holding notifications, or NULL for none.
    PlotMSParametersWatcher* itsUpdater_;
    
    
    // Static //
    
    // Registered flags.
    static vector<int> FLAGS;
    
    // Registered flag names.
    static vector<String> NAMES;
};

}

#endif /* PLOTMSWATCHEDPARAMETERS_H_ */
