//# QPOperation.h: Qwt implementation of plot operation classes.
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
#ifndef QPOPERATION_H_
#define QPOPERATION_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotOperation.h>

#include <QMutex>

#include <casa/namespace.h>

namespace casa {

// Implementation of PlotMutex for the qwt plotter.  Very thin layer on top of
// QMutex.
class QPMutex : public PlotMutex {
public:
    // Constructor.
    QPMutex();
    
    // Destructor.
    ~QPMutex();
    
    
    // Implements PlotMutex::lock().
    void lock();
    
    // Implements PlotMutex::unlock().
    void unlock();
    
private:
    // Mutex.
    QMutex m_mutex;
};

}

#endif

#endif /* QPOPERATION_H_ */
