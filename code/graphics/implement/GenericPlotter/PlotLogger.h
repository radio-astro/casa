//# PlotLogger.h: Classes to perform various logging actions for the plotter.
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
#ifndef PLOTLOGGER_H_
#define PLOTLOGGER_H_

#include <graphics/GenericPlotter/PlotOptions.h>

#include <casa/Logging/LogIO.h>
#include <casa/Utilities/CountedPtr.h>

#include <time.h>
#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class PlotCanvas;
class Plotter;


// Abstract superclass for all messages going through the PlotLogger.
class PlotLogMessage {
public:
    // Constructor.
    PlotLogMessage() { }
    
    // Destructor.
    virtual ~PlotLogMessage() { }    
    
    
    // Returns the first (main) origin for this message.  Usually a class name.
    // See LogIO::origin().
    virtual const String& origin1() const = 0;
    
    // Returns the second origin for this message.  Usually a method name.  See
    // LogIO::origin().
    virtual const String& origin2() const = 0;
    
    // Reports the message to the given output stream.
    virtual void message(ostream& outstream) const = 0;
    
    // Returns the message's priority.  See LogIO::priority().
    virtual LogMessage::Priority priority() const = 0;
};


// A generic log message that could be anything.
class PlotLogGeneric : public virtual PlotLogMessage {
public:    
    // Constructor which takes the origin(s), the message, and an optional
    // priority.
    PlotLogGeneric(const String& origin1, const String& origin2,
            const String& message,
            LogMessage::Priority priority = LogMessage::NORMAL);
    
    // Destructor.
    virtual ~PlotLogGeneric();
    
    
    // Implements PlotLogMessage::origin1().
    virtual const String& origin1() const;
    
    // Implements PlotLogMessage::origin2().
    virtual const String& origin2() const;
    
    // Implements PlotLogMessage::message().
    virtual void message(ostream& outstream) const;
    
    // Implements PlotLogMessage::priority().
    virtual LogMessage::Priority priority() const;
    
private:
    String m_origin1, m_origin2, m_message; // Origins and message.
    LogMessage::Priority m_priority;        // Priority.
};


// Used to report time and memory measurements.  This functionality can be
// accessed either directly with a PlotLogMeasurement object or indirectly
// through the PlotLogger class.
class PlotLogMeasurement : public virtual PlotLogMessage {
public:
    // Static //
    
    // Available time units.  Currently only seconds because that's all that
    // C++ natively supports.
    enum TimeUnit {
        SECOND
    };
    
    // Available memory units.
    enum MemoryUnit {
        BYTE, KILOBYTE, MEGABYTE
    };
    
    // Get a string representation of the given time/memory unit.
    // <group>
    static String timeUnits(TimeUnit t);   
    static String memoryUnits(MemoryUnit m);
    // </group>
    
    
    // Non-Static //
    
    // Constructor which takes the origin(s), optional time and memory
    // units, and an optional priority.  Also calls startMeasurement().
    PlotLogMeasurement(const String& origin1, const String& origin2,
                       TimeUnit timeUnit = SECOND,
                       MemoryUnit memoryUnit = KILOBYTE,
                       LogMessage::Priority priority = LogMessage::NORMAL);
    
    // Destructor.
    ~PlotLogMeasurement();
    
    
    // Implements PlotLogMessage::origin1().
    const String& origin1() const;
    
    // Implements PlotLogMessage::origin2().
    const String& origin2() const;
    
    // Implements PlotLogMessage::message().  Message is:
    // END.\tTime: [time] [timeUnits].  Memory: [memory] [memoryUnits].
    // If the measurement has not been ended, calls stopMeasurement() first.
    void message(ostream& outstream) const;
    
    // Implements PlotLogMessage::priority().
    LogMessage::Priority priority() const;
    
    
    // Returns the time/memory when the measurement started.
    // <group>
    time_t startTime() const;
    unsigned int startMemory() const;
    // </group>
    
    // Returns the time/memory difference between when the measurement started
    // and when the measurement ended.  Invalid if the measurement was never
    // started and ended.
    // <group>
    double time() const;    
    double memory() const;
    // </group>
    
    // Returns the time/memory units for this measurement.
    // <group>
    TimeUnit timeUnit() const;
    MemoryUnit memoryUnit() const;
    // </group>
   
    // Starts the measurement by setting the start time and memory.
    // Measurement automatically begins when the object is constructed, but
    // can be restarted as desired.
    void startMeasurement();
    
    // Calculates the measurements from the last starting point to this point,
    // and generates the log message.
    void stopMeasurement();
    
private:
    String m_origin1, m_origin2;     // Origins
    time_t m_startTime;              // Start time
    unsigned int m_startMemory;      // Start memory
    double m_time, m_memory;         // Time and memory
    TimeUnit m_timeUnit;             // Time unit
    MemoryUnit m_memoryUnit;         // Memory unit
    LogMessage::Priority m_priority; // Message priority
};


// Used to report located indices.  Basically just a container for the results
// of a PlotCanvas::locate.
class PlotLogLocate : public virtual PlotLogMessage {
public:
    // Static //
    
    // Convenience access to PlotCanvas::locate() to return a PlotLogLocate
    // message.
    static PlotLogLocate canvasLocate(PlotCanvas* canvas,
            const PlotRegion& region);
    
    
    // Non-Static //
    
    // Constructor which takes the two origins, the region that was located,
    // and the located indices (see PlotCanvas::locate()).  If
    // deleteIndicesOnDestruction is true, the given indices vector will be
    // deleted when this message is destroyed.  This should be used with care
    // if multiple PlotLogLocates using the same located indices are being
    // used.
    PlotLogLocate(const String& origin1, const String& origin2,
            const PlotRegion& locateRegion,
            vector<vector<pair<unsigned int,unsigned int> > >* locatedIndices,
            LogMessage::Priority priority = LogMessage::NORMAL,
            bool deleteIndicesOnDestruction = true);
    
    // Copy constructor.  NOTE: will set the deleteIndicesOnDestruction flag on
    // the original to false so that the indices won't be deleted from out
    // under the copy.
    PlotLogLocate(const PlotLogLocate& copy);
    
    // Destructor.
    ~PlotLogLocate();
    
    
    // Implements PlotLogMessage::origin1().
    const String& origin1() const;
    
    // Implements PlotLogMessage::origin2().
    const String& origin2() const;
    
    // Implements PlotLogMessage::message().
    void message(ostream& outstream) const;
    
    // Implements PlotLogMessage::priority().
    LogMessage::Priority priority() const;
    
    
    // Returns the region that was located.
    const PlotRegion& locateRegion() const;
    
    // Returns the total number of points that were located.
    unsigned int numLocatedIndices() const;
    
    // Returns the number of plots that were searched.
    unsigned int numSearchedPlots() const;
    
    // Returns the located indices.
    vector<vector<pair<unsigned int, unsigned int> > >* indices() const;
    
    // Returns the located indices for the plot at the given index, or NULL for
    // invalid.
    vector<pair<unsigned int, unsigned int> >* plotIndices(
            unsigned int index) const;
    
    // Returns whether or not this message will delete the indices vector on
    // destruction or not.
    bool willDeleteIndices() const;
    
private:
    String m_origin1, m_origin2;     // Origins
    LogMessage::Priority m_priority; // Priority
    PlotRegion m_region;             // Region
    vector<vector<pair<unsigned int, unsigned int> > >* m_indices; // Indices
    bool m_shouldDelete;             // Should delete indices
};


// Subclass of PlotLogMessage to unify messages for method entering/exiting.
class PlotLogMethod : public virtual PlotLogMessage {
public:
    // Constructor which takes the class and method names, a flag for whether
    // the method is entering or exiting, and an optional additional message
    // and priority.
    PlotLogMethod(const String& className, const String& methodName,
            bool entering, const String& message = String(),
            LogMessage::Priority priority = LogMessage::NORMAL);
    
    // Destructor.
    ~PlotLogMethod();
    
    
    // Implements PlotLogMessage::origin1().
    const String& origin1() const;
    
    // Implements PlotLogMessage::origin2().
    const String& origin2() const;
    
    // Implements PlotLogMessage::message().
    void message(ostream& outstream) const;
    
    // Implements PlotLogMessage::priority().
    LogMessage::Priority priority() const;
    
private:
    String m_class, m_method, m_message; // Class, method, and message.
    LogMessage::Priority m_priority;     // Priority.
};


// Subclass of PlotLogMessage to unify messages for object creation/deletion.
class PlotLogObject : public virtual PlotLogMessage {
public:
    // Constructor which takes the class name and object address, a flag for
    // whether the object is being created or destroyed, and an optional
    // additional message and priority.
    PlotLogObject(const String& className, void* address, bool creation,
            const String& message = String(),
            LogMessage::Priority priority = LogMessage::NORMAL);
    
    // Destructor.
    ~PlotLogObject();
    
    
    // Implements PlotLogMessage::origin1().
    const String& origin1() const;
    
    // Implements PlotLogMessage::origin2().
    const String& origin2() const;
    
    // Implements PlotLogMessage::message().
    void message(ostream& outstream) const;
    
    // Implements PlotLogMessage::priority().
    LogMessage::Priority priority() const;
    
private:
    String m_class, m_method, m_message; // Class, method, and message.
    LogMessage::Priority m_priority;     // Priority.
};


// A PlotLogger is used to log messages to an underlying CASA log object, as
// well as provide access to different logging functionality like measurements.
// PlotLogger is associated with a single Plotter object and should be used by
// all children of that Plotter (canvases, plot items, etc.) to report their
// behavior if the proper flag is turned on.
class PlotLogger {
public:
    // Static //
    
    // Events that someone may want to log.  Specifying more than one event can
    // be used by doing a bitwise or of one or more enum values into a flag.
    enum Event {
        DRAW_TOTAL      = 1,   // Replotting/redrawing the whole GUI.
        DRAW_INDIVIDUAL = 2,   // Replotting/redrawing each plot item.
        METHODS_MAJOR   = 4,   // Entering/exiting major methods.
        OBJECTS_MAJOR   = 8,   // Creation/deletion of major objects.
        EXPORT_TOTAL    = 16,  // Exporting canvases to file.
        
        MSG_INFO        = 32,  // Messages with information.
        MSG_WARN        = 64,  // Messages with warnings.
        MSG_ERROR       = 128, // Messages with errors.
        
        NO_EVENTS       = 0    // No measurement events.
    };
    
    
    // Non-Static //
    
    // Constructor which takes the Plotter this logger is associated with.
    PlotLogger(Plotter* plotter);
    
    // Destructor.
    virtual ~PlotLogger();
    
    
    // Event Methods //
    
    // Calls Plotter::logEventFlags().
    int eventFlags() const;
    
    // Calls Plotter::setLogEventFlags().
    void setEventFlags(int flags);
    
    
    // Message Methods //
    
    // Posts the given message to the underlying CASA logger.
    void postMessage(const PlotLogMessage& message);
    
    // Plots a PlotLogGeneric message with the given parameters to the
    // underlying CASA logger.
    void postMessage(const String& origin1, const String& origin2,
                     const String& message,
                     LogMessage::Priority priority = LogMessage::NORMAL);
    
    
    // Measurement Methods //
    
    // Marks the logger to begin a time/memory measurement.  Measurement marks
    // can be recursive.  Returns a generic message saying that measurement has
    // begun, which will be also posted to the log if postStartMessage is true.
    PlotLogGeneric markMeasurement(const String& origin1,const String& origin2,
            bool postStartMessage = true);
    
    // Gets the measurement since the last mark.  The message will also be
    // posted to the log if postReleaseMessage is true.
    PlotLogMeasurement releaseMeasurement(bool postReleaseMessage = true);
    
    
    // Locate Methods //
    
    // Calls locate on the given canvas and returns the result as a message
    // that will also be posted if postLocateMessage is true.
    PlotLogLocate locate(PlotCanvas* canvas, const PlotRegion& region,
            bool postLocateMessage = true);
    
private:
    Plotter* m_plotter; // Plotter.    
    LogIO m_logger;     // For reporting log information.
    
    // Current measurement marks.
    vector<PlotLogMeasurement> m_measurements;
};
typedef CountedPtr<PlotLogger> PlotLoggerPtr;

}

#endif /* PLOTLOGGER_H_ */
