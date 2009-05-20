//# PlotMSLogger.cc: Extension of PlotLogger for use with plotms.
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
#include <plotms/PlotMS/PlotMSLogger.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

//////////////////////////////
// PLOTMSLOGGER DEFINITIONS //
//////////////////////////////

// Static //

int PlotMSLogger::levelToEventFlag(Level level, bool debug) {
    int events = PlotLogger::NO_EVENTS;
    
    // Use fall-through behavior of switch to cumulatively add flags.
    switch(level) {
    case HIGH: events |= PlotLogger::DRAW_INDIVIDUAL | INITIALIZE_GUI;
    
    case MED:  events |= PlotLogger::EXPORT_TOTAL | PlotLogger::MSG_WARN;
    
    case LOW:  events |= PlotLogger::DRAW_TOTAL | PlotLogger::MSG_ERROR |
                         LOAD_CACHE | NUM_POINTS;

    default: break;
    }
    
    if(debug) events |= PlotLogger::METHODS_MAJOR | PlotLogger::OBJECTS_MAJOR |
                        PlotLogger::MSG_INFO;
    
    return events;
}


// Non-Static //

PlotMSLogger::PlotMSLogger() : itsParent_(NULL) { }

PlotMSLogger::~PlotMSLogger() { }


void PlotMSLogger::setParent(PlotMS* p) {
    itsParent_ = p;
    if(p != NULL) itsLogger_ = p->getPlotter()->getPlotter()->logger();
    else          itsLogger_ = PlotLoggerPtr();
}

void PlotMSLogger::setLevel(Level level, bool debug) {
    if(itsParent_ != NULL)
        itsParent_->getParameters().setLogLevel(level, debug);
}


void PlotMSLogger::postMessage(const PlotLogMessage& message) {
    if(!itsLogger_.null()) itsLogger_->postMessage(message); }

void PlotMSLogger::postMessage(const String& origin1, const String& origin2,
        const String& message, LogMessage::Priority priority) {
    if(!itsLogger_.null())
        itsLogger_->postMessage(origin1, origin2, message, priority);
}

PlotLogGeneric PlotMSLogger::markMeasurement(const String& o1,
        const String& o2, bool post) {
    if(!itsLogger_.null()) return itsLogger_->markMeasurement(o1, o2, post);
    else return PlotLogGeneric("PlotMSLogger", "markMeasurement", "");
}

PlotLogMeasurement PlotMSLogger::releaseMeasurement(bool post) {
    if(!itsLogger_.null()) return itsLogger_->releaseMeasurement(post);
    else return PlotLogMeasurement("PlotMSLogger", "releaseMeasurement");
}


void PlotMSLogger::setEventFlags_(int flags) {
    if(!itsLogger_.null()) itsLogger_->setEventFlags(flags);
}

}
