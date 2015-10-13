// -*- C++ -*-
//# ThreadCoordinator.h: Definition of the ThreadCoordinator class
//# Copyright (C) 2015
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
//# $Id$
#ifndef STDCASA_THREAD_BARRIER_H_
#define STDCASA_THREAD_BARRIER_H_
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace casa {
    class Barrier {
    private:
        boost::mutex mutex_;
        boost::condition_variable cvar_;
        std::size_t count_;
    public:
        explicit Barrier(std::size_t count) : count_{count} { }
        void wait( ) {
            boost::unique_lock<boost::mutex> lock(mutex_);
            if (--count_ == 0) {
                cvar_.notify_all();
            } else {
                cvar_.wait(lock, [this] { return count_ == 0; });
            }
        }
    };
}

#endif
