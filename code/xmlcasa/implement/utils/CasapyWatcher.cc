//# CasapyWatcher.cc: Class to notify children about casapy events.
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
#include <xmlcasa/utils/CasapyWatcher.h>

namespace casac {

///////////////////////////////
// CASAPYWATCHER DEFINITIONS //
///////////////////////////////

// Static //

vector<CasapyWatcher*> *CasapyWatcher::WATCHERS = 0;

CasapyWatcher::Singleton CasapyWatcher::SINGLETON = Singleton();


void CasapyWatcher::registerWatcher(CasapyWatcher* watcher) {

    if ( WATCHERS == 0 ) {
	WATCHERS = new vector<CasapyWatcher*>();
    }

    if ( watcher == NULL ) return;

    for ( unsigned int i = 0; i < WATCHERS->size(); i++ )
	if ( watcher == (*WATCHERS)[i] ) return;

    WATCHERS->push_back(watcher);
}

void CasapyWatcher::unregisterWatcher(CasapyWatcher* watcher) {
    if ( WATCHERS == 0 ) return;
    if ( watcher == NULL ) return;
    for ( unsigned int i = 0; i < WATCHERS->size(); i++ ) {
	if ( (*WATCHERS)[i] == watcher ) {
	    WATCHERS->erase(WATCHERS->begin() + i);
	    break;
        }
    }
}


void CasapyWatcher::logChanged_(const String& sinkLocation) {
    if ( WATCHERS == 0 ) return;
    for ( unsigned int i = 0; i < WATCHERS->size(); i++ )
	(*WATCHERS)[i]->logChanged(sinkLocation);
}

void CasapyWatcher::logChanged_(LogMessage::Priority filterPriority) {
    if ( WATCHERS == 0 ) return;
    for ( unsigned int i = 0; i < WATCHERS->size(); i++ )
	(*WATCHERS)[i]->logChanged(filterPriority);
}

void CasapyWatcher::casapyClosing_() {
    if ( WATCHERS == 0 ) return;
    for ( unsigned int i = 0; i < WATCHERS->size(); i++ )
	(*WATCHERS)[i]->casapyClosing();
}


// Non-Static //

CasapyWatcher::CasapyWatcher() { }
CasapyWatcher::~CasapyWatcher() { unregisterWatcher(this); }
void CasapyWatcher::logChanged(const String& sinkLocation) { }
void CasapyWatcher::logChanged(LogMessage::Priority filterPriority) { }
void CasapyWatcher::casapyClosing() { }

}
