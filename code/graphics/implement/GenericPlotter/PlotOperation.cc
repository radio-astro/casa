//# PlotOperation.cc: Classes for managing large, threaded plot operations.
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
#include <graphics/GenericPlotter/PlotOperation.h>

namespace casa {

///////////////////////////////
// PLOTOPERATION DEFINITIONS //
///////////////////////////////

PlotOperation::PlotOperation(const String& name, PlotMutexPtr mutex) :
        m_name(name), m_mutex(mutex) {
    reset();
}

PlotOperation::~PlotOperation() { }


#define PO_GETTER(TYPE, NAME)                                                 \
    m_mutex->lock();                                                          \
    TYPE temp = NAME;                                                         \
    m_mutex->unlock();                                                        \
    return temp;

String PlotOperation::name() const { PO_GETTER(String, m_name) } 
bool PlotOperation::inProgress() const { PO_GETTER(bool, m_inProgress) }
bool PlotOperation::isFinished() const { PO_GETTER(bool, m_isFinished) }
unsigned int PlotOperation::currentProgress() const {
    PO_GETTER(unsigned int, m_currentProgress) }
String PlotOperation::currentStatus() const {
    PO_GETTER(String, m_currentStatus) }


#define PO_SETTER(NAME, VAL)                                                  \
    m_mutex->lock();                                                          \
    NAME = VAL;                                                               \
    m_mutex->unlock();

void PlotOperation::setInProgress(bool inProgress) {
    PO_SETTER(m_inProgress, inProgress) }
void PlotOperation::setIsFinished(bool isFinished) {
    PO_SETTER(m_isFinished, isFinished) }
void PlotOperation::setCurrentProgress(unsigned int currentProgress) {
    PO_SETTER(m_currentProgress, currentProgress)
    if(m_currentProgress > 100) m_currentProgress = 100;
}
void PlotOperation::setCurrentStatus(const String& currentStatus) {
    PO_SETTER(m_currentStatus, currentStatus) }

void PlotOperation::setMutex(PlotMutexPtr mutex) { m_mutex = mutex; }

void PlotOperation::reset() {
    m_inProgress = false;
    m_isFinished = false;
    m_currentProgress = 0;
}

}
