/*
 *  This file was originally part of the "D-Bus++ - C++ bindings for D-Bus":
 *
 *          include/dbus-c++/eventloop-integration.h
 *
 *  I found the BusDispatcher to be... not so great. I was eventually able to
 *  get it to work. All I wanted to do was be able to wait until a signal to
 *  arrive. To do that, I had to use a "dispatcher". I did not want to hitch
 *  my wagon to glib or qt or ..., but only wanted a simple minimal dispatcher.
 *  Thus, despite admonitions to the contrary, I adopted the "BusDispatcher".
 *  the signal I was waiting for was delivered to my callback promptly, but
 *  I found that there was a pregnant pause between when I told the dispatcher
 *  to "leave( )" undoubtedly because of timeouts in polling. I tried to
 *  create a subclass of BusDispatcher, but because all of the _mutex_*
 *  were private to the DefaultMainLoop (what a mess). At that point,
 *  decided to implement my own, minimal, dispatcher. It is hard to believe
 *  this should be necessary, but apparently so.
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
 *  Copyright (C) 2009       Associated Universities Inc.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef DBUS_DISPATCHER_H_
#define DBUS_DISPATCHER_H_

#include <dbus-c++/dbus.h>
#include <dbus-c++/connection.h>

namespace casa {
    namespace dbus {

	class Dispatcher;

	class Timeout : public DBus::Timeout {
	    public:
		Timeout( DBus::Timeout::Internal *, Dispatcher *);
		~Timeout( );
		void toggle();
		DBus::Slot<void, Timeout &> expired;
	
	    private:
		bool _enabled;
		int _interval;
		bool _repeat;

		double _expiration;
/* 		void *_data; */
	
		Dispatcher *_disp;
		friend class Dispatcher;
	};

	class Watch : public DBus::Watch {

	    public:
		Watch( DBus::Watch::Internal *, Dispatcher *);
		~Watch( );
		void toggle();

		DBus::Slot<void, Watch &> ready;

	    private:
		bool _enabled;

		int _fd;
		int _flags;
		int _state;

/* 		void *_data; */

		Dispatcher *_disp;
		friend class Dispatcher;
	};

	class Dispatcher : public DBus::Dispatcher {

	    public:
		Dispatcher() : _running(false) { }
		~Dispatcher();

		// pure virtual functions from Dispatcher
		void enter();
		void leave();

		DBus::Timeout *add_timeout(DBus::Timeout::Internal *);
		void rem_timeout(DBus::Timeout *);

		DBus::Watch *add_watch(DBus::Watch::Internal *);
		void rem_watch(DBus::Watch *);

		// helper function
		void do_iteration();
		void watch_ready(Watch &);
		void timeout_expired(Timeout &);

		void dispatch( );

	    private:
		DBus::DefaultMutex _mutex_t;
		std::list<Timeout*> _timeouts;

		DBus::DefaultMutex _mutex_w;
		std::list<Watch*> _watches;

		bool _running;
		friend class Timeout;
		friend class Watch;
	};
    }
}

#endif
