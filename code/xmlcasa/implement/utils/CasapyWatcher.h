//# CasapyWatcher.h: Class to notify children about casapy events.
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
#ifndef CASAPYWATCHER_H_
#define CASAPYWATCHER_H_

#include <casa/Logging/LogMessage.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casac {

// Parent for any class that wants to be notified about events in casapy.
class CasapyWatcher {
    
    //# Friend class declarations.
    friend class dbus;
    friend class logsink;
    
public:
    // Static //
    
    // Registers/Unregisters the given watcher.
    // <group>
    static void registerWatcher(CasapyWatcher* watcher);
    static void unregisterWatcher(CasapyWatcher* watcher);
    // </group>
    
    
    // Non-Static //
    
    // Constructor.  Does NOT register watcher (call registerWatcher() to do
    // so).
    CasapyWatcher();
    
    // Destructor.  Unregisters watcher.
    virtual ~CasapyWatcher();
    
    
    // Called when the log sink location and/or filter priority changes in
    // casapy.  Should be overridden if the child class wants to know when this
    // happens.
    // <group>
    virtual void logChanged(const String& sinkLocation);
    virtual void logChanged(LogMessage::Priority filterPriority);
    // </group>
    
    // Called when casapy is closing.  Should be overridden if the child class
    // wants to know when this happens.
    virtual void casapyClosing();
    
private:
    // Static //
#if 0
    // Singleton class that is used to detect when casapy is closing.
    class Singleton {
    public:
        // Constructor.
        Singleton() { }
        
        // Destructor.  Indicates that casapy is closing because there's only
        // one static instance of the class.
        ~Singleton() { CasapyWatcher::casapyClosing_(); }
    };
#endif
    // All registered watchers.
    static vector<CasapyWatcher*> *WATCHERS;
    
    // Used to detect when casapy is closing.
/*     static Singleton SINGLETON; */
    
    
    // Called by the logger component when the sink location or filter priority
    // changes.
    // <group>
    static void logChanged_(const String& sinkLocation);
    static void logChanged_(LogMessage::Priority filterPriority);
    // </group>

    // Called by the dbus component when casapy is closing.
    static void casapyClosing_();
};

}

#endif /* CASAPYWATCHER_H_ */
