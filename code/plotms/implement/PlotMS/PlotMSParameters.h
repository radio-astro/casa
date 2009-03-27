//# PlotMSParameters.h: Parameter classes for plotms.
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
#ifndef PLOTMSPARAMETERS_H_
#define PLOTMSPARAMETERS_H_

#include <plotms/PlotMS/PlotMSLogger.h>

#include <vector>

namespace casa {

//# Forward Declarations
class PlotMSPlot;
class PlotMSPlotParameters;
class PlotMSSinglePlot;
class PlotMSSinglePlotParameters;
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
    // The updateFlag and redrawRequired parameters let the watcher know which
    // categories the changes were in and whether a redraw is required,
    // respectively.
    virtual void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired) = 0;
};


// Abstract class for parameters that may be watched by one or more interested
// classes.  Any subclass is assumed to have different properties in one or
// more of the update categories defined by the UpdateFlag enum.  Using this
// system, any classes watching the parameters for changes can be notified
// which categories the changes occurred in.
class PlotMSWatchedParameters {
public:
    // Static //
    
    // Enum holding the different update "categories."  Each category can be
    // updated separately from the others, which can be used to avoid
    // unnecessary work.  The enum values can be or-ed together to indicate
    // that multiple categories have been changed.
    enum UpdateFlag {
        MS     = 1,  // The MS (filename, selection, ...).  Both the MS and the
                     //  cache need to be updated.
        CACHE  = 2,  // The plot axes.  The cache needs to be updated.
        CANVAS = 4,  // The canvas display options (grid, legend, title, ...).
                     //  The canvas needs to be updated.
        PLOT   = 8,  // The plot options (symbol, title, ...).  The plot needs
                     //  to be updated.
        LOG    = 16, // The log (level, debug messages, ...).
        
        NONE = 0,                    // Nothing changed.
        ALL  = MS | CACHE | CANVAS | // Everything changed.
               PLOT | LOG
    };
    
    
    // Non-Static //
    
    // Constructor.
    PlotMSWatchedParameters();
    
    // Copy constructor.  See operator=().
    PlotMSWatchedParameters(const PlotMSWatchedParameters& copy);
    
    // Destructor.
    virtual ~PlotMSWatchedParameters();
    
    
    // Adds/Removes the given watcher for this PlotMSParameters.
    // <group>
    void addWatcher(PlotMSParametersWatcher* watcher);
    void removeWatcher(PlotMSParametersWatcher* watcher);
    // </group>
    
    // Holds update notification because the given watcher will be updating
    // multiple fields.
    void holdNotification(PlotMSParametersWatcher* updater);
    
    // Releases update notification; notifies all watchers of an update except
    // for the one that called holdNotification.
    void releaseNotification();
    
    // Notifies any watchers that the parameters have been updated with the
    // given flags.  If an updater is given, it is NOT notified.
    void notifyWatchers(int updateFlags = ALL, bool redrawRequired = true,
            PlotMSParametersWatcher* updater = NULL);
    
    // Returns true if this PlotMSParameters equals the other, in the different
    // update categories.
    // <group>
    bool equalsMS(PlotMSWatchedParameters& other) const {
        return equals(other, MS); }
    bool equalsCache(PlotMSWatchedParameters& other) const {
        return equals(other, CACHE); }
    bool equalsCanvas(PlotMSWatchedParameters& other) const {
        return equals(other, CANVAS); }
    bool equalsPlot(PlotMSWatchedParameters& other) const {
        return equals(other, PLOT); }
    bool equalsLog(PlotMSWatchedParameters& other) const {
        return equals(other, LOG); }
    // </group>
    
    // Equality operators.
    // <group>
    virtual bool operator==(const PlotMSWatchedParameters& other) const {
        return equals(other, ALL); }
    virtual bool operator!=(const PlotMSWatchedParameters& other) const {
        return !(operator==(other)); }
    // </group>
    
    // Copy operator.  Does NOT copy the current update state or watchers as
    // these are assumed to be unique to each parameters object.  Should be
    // overridden by children.
    virtual PlotMSWatchedParameters& operator=(
            const PlotMSWatchedParameters& copy);
    
    
    // ABSTRACT METHODS //
    
    // Returns true if this PlotMSParameters equals the other, in the given
    // update categories (bit-wise or of UpdateFlags).
    virtual bool equals(const PlotMSWatchedParameters& other,
                        int updateFlags) const = 0;    

protected:
    // Returns the current update flag.
    int currentUpdateFlag() const;
    
    // Returns the current redraw required flag.
    bool currentRedrawRequired() const;
    
    // Provides access to children to indicate whether the given update flag
    // should be turned on or off, and the redraw required flag should be
    // updated as given.  This should be used by setter functions to classify
    // the changes appropriately.  If update notifications are NOT being held,
    // any watchers will immediately be notified of the change.
    void updateFlag(UpdateFlag update, bool on = true,
            bool redrawRequired = true);
    
    // Provides access to children to indicate which update flags are on.
    // The given value should be a bit-wise or of one or more UpdateFlag
    // enum values.  The redraw required flag is also updated to the given.
    void updateFlags(int updateFlags, bool redrawRequired = true);
    
private:
    // Update flags.
    // <group>
    int itsUpdateFlags_;
    bool itsRedrawFlag_;
    // </group>
    
    // Watchers.
    vector<PlotMSParametersWatcher*> itsWatchers_;
    
    // Watcher that is currently holding notifications, or NULL for none.
    PlotMSParametersWatcher* itsUpdater_;
};


// Subclass of PlotMSWatchedParameters that hold parameters for the whole
// plotter.  These parameters include:
// * log level.
class PlotMSParameters : public PlotMSWatchedParameters {
public:
    // Constructor.
    PlotMSParameters(PlotMSLogger::Level logLevel = PlotMSLogger::OFF,
            bool debug = false);
    
    // Copy constructor.  See operator=().
    PlotMSParameters(const PlotMSParameters& copy);
    
    // Destructor.
    ~PlotMSParameters();

    
    // Include overloaded methods.
    using PlotMSWatchedParameters::operator=;

    
    // Returns the current log level.
    PlotMSLogger::Level logLevel() const;
    
    // Returns the current log debug flag.
    bool logDebug() const;
    
    // Sets the current log level.  Note: this will notify any watchers unless
    // notifications are being held.
    void setLogLevel(PlotMSLogger::Level level, bool debug);
    
    // Implements PlotMSWatchedParameters::equals().  Will return false if the
    // other parameters are not of type PlotMSParameters.
    bool equals(const PlotMSWatchedParameters& other,
                            int updateFlags) const;
    
    // Copy operator.  See PlotMSWatchedParameters::operator=().
    PlotMSParameters& operator=(const PlotMSParameters& copy);
    
private:
    // Log level.
    PlotMSLogger::Level itsLogLevel_;
    
    // Log debug flag.
    bool itsLogDebug_;
};

}

#endif /* PLOTMSPARAMETERS_H_ */
