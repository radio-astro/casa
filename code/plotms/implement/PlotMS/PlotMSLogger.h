//# PlotMSLogger.h: Extension of PlotLogger for use with plotms.
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
#ifndef PLOTMSLOGGER_H_
#define PLOTMSLOGGER_H_

#include <plotms/PlotMS/PlotMSConstants.h>

#include <graphics/GenericPlotter/PlotLogger.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotMS;


// PlotMSLogger wraps around a PlotLogger to provide useful access to PlotMS
// classes and additonal logging events.
class PlotMSLogger {
    friend class PlotMS;
    
public:
    // Static //
    
    // Enum meant to add values to PlotLogger::Event for PlotMS-specific
    // events.  The idea is to bitwise-or these values with PlotLogger::Event
    // values and let the PlotLogger deal with the flags it knows about and
    // PlotMSLogger to deal with the flags it added.
    enum Event {
        INITIALIZE_GUI = 1024,
        LOAD_CACHE     = 2048
    };
    
    // Enum for the log level, which controls the number and type of log
    // messages that PlotMS produces.  The levels correspond to the different
    // PlotLogger event flags as follows:
    // * OFF = NO_EVENTS,
    // * LOW = DRAW_TOTAL | MSG_ERROR | LOAD_CACHE,
    // * MED = [LOW] | EXPORT_TOTAL | MSG_WARN,
    // * HIGH = [MED] | DRAW_INDIVIDUAL | INITIALIZE_GUI
    // If the additional debug flag is true, the following events are added:
    // * METHODS_MAJOR | OBJECTS_MAJOR, MSG_INFO
    // <group>
    PMS_ENUM1(Level, levels, levelStrings, level,
              OFF, LOW, MED, HIGH)
    PMS_ENUM2(Level, levels, levelStrings, level,
              "off", "low", "medium", "high")
    // </group>
    
    // Converts the given level to a log measurement event flag (see
    // documentation for Level enum) and debug flag.
    static int levelToEventFlag(Level level, bool debug);
    
    
    // Non-Static //
    
    // Constructor.  Parent must be set using setParent() before logger can be
    // used.
    PlotMSLogger();
    
    // Destructor.
    ~PlotMSLogger();
    
    
    // Sets the parent to the given.  Uses the PlotLogger from the parent's
    // plotter.
    void setParent(PlotMS* parent);
    
    // Sets the log level and debug flag on the parent PlotMS.
    void setLevel(Level level, bool debug);
    
    
    // See PlotLogger::postMessage().
    // <group>
    void postMessage(const PlotLogMessage& message);
    void postMessage(const String& origin1, const String& origin2,
                     const String& message,
                     LogMessage::Priority priority = LogMessage::NORMAL);
    // </group>
    
    // See PlotLogger::markMeasurement().
    PlotLogGeneric markMeasurement(const String& origin1, const String& origin2,
                                   bool postStartMessage = true);
    
    // See PlotLogger::releaseMeasurement().
    PlotLogMeasurement releaseMeasurement(bool postReleaseMessage = true);
    
protected:
    // Method to allow PlotMS to set the measurement event flags on the hidden
    // PlotLogger.
    void setEventFlags_(int flags);
    
private:
    // PlotMS parent.
    PlotMS* itsParent_;
    
    // PlotLogger.
    PlotLoggerPtr itsLogger_;
};

}

#endif /* PLOTMSLOGGER_H_ */
