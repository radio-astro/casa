//# PlotLogger.cc: Classes to perform various logging actions for the plotter.
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
#include <graphics/GenericPlotter/PlotLogger.h>

#include <graphics/GenericPlotter/Plotter.h>

#include <casa/OS/Memory.h>

namespace casa {

////////////////////////////////
// PLOTLOGGENERIC DEFINITIONS //
////////////////////////////////

PlotLogGeneric::PlotLogGeneric(const String& origin1, const String& origin2,
        const String& message, LogMessage::Priority priority) :
        m_origin1(origin1), m_origin2(origin2), m_message(message),
        m_priority(priority) { }

PlotLogGeneric::~PlotLogGeneric() { }

const String& PlotLogGeneric::origin1() const { return m_origin1; }
const String& PlotLogGeneric::origin2() const { return m_origin2; }
void PlotLogGeneric::message(ostream& os) const { os << m_message; }
LogMessage::Priority PlotLogGeneric::priority() const { return m_priority; }


////////////////////////////////////
// PLOTLOGMEASUREMENT DEFINITIONS //
////////////////////////////////////

// Static //

String PlotLogMeasurement::timeUnits(TimeUnit t) {
    switch(t) {
    case SECOND: return "seconds";
    
    default: return "";
    }
}

String PlotLogMeasurement::memoryUnits(MemoryUnit m) {
    switch(m) {
    case BYTE:     return "bytes";
    case KILOBYTE: return "kilobytes";
    case MEGABYTE: return "megabytes";
    
    default: return "";
    }
}


// Non-Static //

PlotLogMeasurement::PlotLogMeasurement(const String& origin1,
        const String& origin2, TimeUnit timeUnit, MemoryUnit memoryUnit,
        LogMessage::Priority priority): m_origin1(origin1), m_origin2(origin2),
        m_time(-1), m_memory(0), m_timeUnit(timeUnit), m_memoryUnit(memoryUnit),
        m_priority(priority) {
    startMeasurement();
}

PlotLogMeasurement::~PlotLogMeasurement() { }

const String& PlotLogMeasurement::origin1() const { return m_origin1; }
const String& PlotLogMeasurement::origin2() const { return m_origin2; }

void PlotLogMeasurement::message(ostream& outstream) const {
    if(m_time < 0) const_cast<PlotLogMeasurement*>(this)->stopMeasurement();
    
    outstream << "END\tTime: ";
    if(m_time >= 0)
        outstream << "+" << m_time << " " << timeUnits(m_timeUnit);
    else outstream << "unreported";
    outstream << ".  Memory: ";
    if(m_memory >= 0) outstream << "+";
    outstream << m_memory << " " << memoryUnits(m_memoryUnit);
    outstream << '.';
}

LogMessage::Priority PlotLogMeasurement::priority() const { return m_priority;}


time_t PlotLogMeasurement::startTime() const { return m_startTime; }
unsigned int PlotLogMeasurement::startMemory() const { return m_startMemory; }
double PlotLogMeasurement::time() const { return m_time; }
double PlotLogMeasurement::memory() const { return m_memory; }
PlotLogMeasurement::TimeUnit PlotLogMeasurement::timeUnit() const {
    return m_timeUnit; }
PlotLogMeasurement::MemoryUnit PlotLogMeasurement::memoryUnit() const {
    return m_memoryUnit; }

void PlotLogMeasurement::startMeasurement() {
    m_startTime = std::time(NULL);
    m_startMemory = Memory::allocatedMemoryInBytes();
}

void PlotLogMeasurement::stopMeasurement() {
    time_t t = std::time(NULL);
    m_time = t - m_startTime;
    m_memory = Memory::allocatedMemoryInBytes() - m_startMemory;
    if(m_memoryUnit == KILOBYTE)      m_memory /= 1024;
    else if(m_memoryUnit == MEGABYTE) m_memory /= 1024 * 1024;

    startMeasurement();
}


///////////////////////////////
// PLOTLOGLOCATE DEFINITIONS //
///////////////////////////////

PlotLogLocate PlotLogLocate::canvasLocate(PlotCanvas* canv,
        const PlotRegion& reg) {
    if(canv == NULL)
        return PlotLogLocate("PlotLogLocate", "canvasLocate", reg, NULL);
    
    return PlotLogLocate("PlotLogLocate","canvasLocate",reg,canv->locate(reg));
}


PlotLogLocate::PlotLogLocate(const String& origin1, const String& origin2,
        const PlotRegion& locateRegion,
        vector<vector<pair<unsigned int,unsigned int> > >* locatedIndices,
        LogMessage::Priority priority, bool deleteIndicesOnDestruction) :
        m_origin1(origin2), m_origin2(origin2), m_priority(priority),
        m_region(locateRegion), m_indices(locatedIndices),
        m_shouldDelete(deleteIndicesOnDestruction) { }

PlotLogLocate::PlotLogLocate(const PlotLogLocate& copy) :
    m_origin1(copy.origin1()), m_origin2(copy.origin2()),
    m_priority(copy.priority()), m_region(copy.locateRegion()),
    m_indices(copy.indices()), m_shouldDelete(copy.willDeleteIndices()) {
    const_cast<PlotLogLocate&>(copy).m_shouldDelete = false;
}

PlotLogLocate::~PlotLogLocate() {
    if(m_shouldDelete && m_indices != NULL) delete m_indices;
}


const String& PlotLogLocate::origin1() const { return m_origin1; }
const String& PlotLogLocate::origin2() const { return m_origin2; }

void PlotLogLocate::message(ostream& os) const {
    unsigned int np = numSearchedPlots(), ni = numLocatedIndices();
    os << "Locating indices with x in [" << m_region.left() << ", "
       << m_region.right() << "] and y in [" << m_region.bottom() << ", "
       << m_region.top() << ".  Searched " << np << " plots and found " << ni
       << " indices.";
    if(np > 0 && ni > 0) os << '\n';
    
    bool first = true;
    unsigned int n, from, to;
    for(unsigned int i = 0; i < np; i++) {
        n = m_indices->at(i).size();
        if(n > 0) {
            if(!first) os << '\n';
            first = false;
        }
        if(np > 0 && n > 0) os << "Plot " << i << ": ";
        if(n > 0) os << "[";
        
        for(unsigned int j = 0; j < n; i++) {
            from = (*m_indices)[i][j].first;
            to = (*m_indices)[i][j].second;
            os << from;
            if(to != from) os << "-" << to;
            if(j < n - 1) os << ", ";
        }
        if(n > 0) os << "]";
    }
}

LogMessage::Priority PlotLogLocate::priority() const { return m_priority; }


const PlotRegion& PlotLogLocate::locateRegion() const { return m_region; }
unsigned int PlotLogLocate::numLocatedIndices() const {
    if(m_indices == NULL) return 0;
    unsigned int n = 0;
    for(unsigned int i = 0; i < m_indices->size(); i++)
        for(unsigned int j = 0; j < m_indices->at(i).size(); j++)
            n += (*m_indices)[i][j].second - (*m_indices)[i][j].first + 1;

    return n;
}

unsigned int PlotLogLocate::numSearchedPlots() const {
    return (m_indices != NULL) ? m_indices->size() : 0; }

vector<vector<pair<unsigned int, unsigned int> > >*
PlotLogLocate::indices() const { return m_indices; }

vector<pair<unsigned int, unsigned int> >*
PlotLogLocate::plotIndices(unsigned int index) const {
    if(m_indices == NULL || index >= m_indices->size()) return NULL;
    else return &(*m_indices)[index];
}

bool PlotLogLocate::willDeleteIndices() const { return m_shouldDelete; }


////////////////////////////
// PLOTLOGGER DEFINITIONS //
////////////////////////////

PlotLogger::PlotLogger(Plotter* plotter) : m_plotter(plotter) { }

PlotLogger::~PlotLogger() { }

void PlotLogger::setMeasurementEvents(int flags) {
    if(m_plotter != NULL) m_plotter->setLogMeasurementEvents(flags); }

void PlotLogger::postMessage(const PlotLogMessage& message) {
    m_logger.origin(LogOrigin(message.origin1(), message.origin2()));
    m_logger.priority(message.priority());
    message.message(m_logger.output());
    m_logger << LogIO::POST;
}

void PlotLogger::postMessage(const String& origin1, const String& origin2,
        const String& message, LogMessage::Priority priority) {
    postMessage(PlotLogGeneric(origin1, origin2, message, priority)); }

PlotLogGeneric PlotLogger::markMeasurement(const String& origin1,
        const String& origin2, bool postStartMessage) {
    PlotLogGeneric startMessage(origin1, origin2, "START\tCurrent memory usage"
            ": "+String::toString(Memory::allocatedMemoryInBytes()/1024.0)+" "+
            PlotLogMeasurement::memoryUnits(PlotLogMeasurement::KILOBYTE)+".");
    if(postStartMessage) postMessage(startMessage);
    m_measurements.push_back(PlotLogMeasurement(origin1, origin2));  
    return startMessage;
}

PlotLogMeasurement PlotLogger::releaseMeasurement(bool postReleaseMessage) {
    if(m_measurements.size() == 0) // invalid
        return PlotLogMeasurement("PlotLogger", "releaseMeasurement");
    
    // Go through and update previous times.    
    PlotLogMeasurement m = m_measurements[m_measurements.size() - 1];
    m_measurements.erase(m_measurements.begin() + m_measurements.size() - 1);
    m.stopMeasurement();
    if(postReleaseMessage) postMessage(m);
    
    return m;
}

PlotLogLocate PlotLogger::locate(PlotCanvas* canvas, const PlotRegion& region,
            bool postLocateMessage) {
    if(canvas == NULL) return PlotLogLocate("", "", region, NULL);
    
    vector<vector<pair<unsigned int, unsigned int> > >* res = canvas->locate(
            region);
    PlotLogLocate msg("PlotCanvas", "locate", region, res, LogMessage::NORMAL,
            false);
    if(postLocateMessage) postMessage(msg);
    return msg;
}

}
