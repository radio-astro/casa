/*
 *  This file was originally part of the "D-Bus++ - C++ bindings for D-Bus":
 *
 *          src/eventloop-integration.cpp
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


#include <casadbus/session/Dispatcher.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <dbus/dbus.h>


namespace casa {
    namespace dbus {

	inline double millis(timeval tv) {
	    return (tv.tv_sec *1000.0 + tv.tv_usec/1000.0);
	}

	Timeout::Timeout(DBus::Timeout::Internal *ti, Dispatcher *bd) : DBus::Timeout(ti),_enabled(true),
									_interval(Timeout::interval()), _repeat(true),
									_expiration(0), _disp(bd) {
	    timeval now;
	    gettimeofday(&now, NULL);

	    _expiration = millis(now) + _interval;

	    _disp->_mutex_t.lock();
	    _disp->_timeouts.push_back(this);
	    _disp->_mutex_t.unlock();
	}

	Timeout::~Timeout( ) {
	    _disp->_mutex_t.lock();
	    _disp->_timeouts.remove(this);
	    _disp->_mutex_t.unlock();
	}

	void Timeout::toggle() {
	    _enabled = DBus::Timeout::enabled();
	}

	Watch::Watch(DBus::Watch::Internal *wi, Dispatcher *bd) : DBus::Watch(wi), _fd(Watch::descriptor()), _flags(0),
									_disp(bd), _enabled(true), _state(0) {

	    _disp->_mutex_w.lock();
	    _disp->_watches.push_back(this);
	    _disp->_mutex_w.unlock();

	    int flg = POLLHUP | POLLERR;

	    if (DBus::Watch::flags() & DBUS_WATCH_READABLE)
		flg |= POLLIN;
	    if (DBus::Watch::flags() & DBUS_WATCH_WRITABLE)
		flg |= POLLOUT;

	    _flags = flg;
	    _enabled = DBus::Watch::enabled();
	}

	Watch::~Watch( ) {
	    _disp->_mutex_w.lock();
	    _disp->_watches.remove(this);
	    _disp->_mutex_w.unlock();
	}

	void Watch::toggle() {
	    _enabled = DBus::Watch::enabled();
	}

	void Dispatcher::enter() {
	    _running = true;

	    while (_running) {
		do_iteration();
	    }
	}

	void Dispatcher::leave() {
	    _running = false;
	}

	void Dispatcher::do_iteration() {
	    dispatch_pending();
	    dispatch();
	}

	DBus::Timeout *Dispatcher::add_timeout(DBus::Timeout::Internal *ti) {
	    Timeout *bt = new Timeout(ti, this);
	    bt->expired = new DBus::Callback<Dispatcher, void, Timeout &>(this, &Dispatcher::timeout_expired);
// 	    bt->_data = bt;
	    return bt;
	}

	void Dispatcher::rem_timeout(DBus::Timeout *t) {
	    delete t;
	}

	DBus::Watch *Dispatcher::add_watch(DBus::Watch::Internal *wi) {
	    Watch *bw = new Watch(wi, this);
	    bw->ready = new DBus::Callback<Dispatcher, void, Watch &>(this, &Dispatcher::watch_ready);
// 	    bw->_data = bw;
	    return bw;
	}

	void Dispatcher::rem_watch(DBus::Watch *w) {
	    delete w;
	}

	void Dispatcher::timeout_expired(Timeout &et) {
	    et.handle( );
// 	    Timeout *to = reinterpret_cast<Timeout *>(et._data);
// 	    to->handle();
	}

	void Dispatcher::watch_ready(Watch &ew) {
// 	    Watch *wt = reinterpret_cast<Watch *>(ew._data);

	    int flags = 0;

	    if (ew._state & POLLIN)
		flags |= DBUS_WATCH_READABLE;
	    if (ew._state & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;
	    if (ew._state & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;
	    if (ew._state & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	    ew.handle(flags);
	}


	Dispatcher::~Dispatcher( ) {

	    _mutex_w.lock();
	    std::list<Watch*>::iterator wi = _watches.begin();
	    while (wi != _watches.end()) {
		std::list<Watch*>::iterator wmp = wi;
		++wmp;
		_mutex_w.unlock();
		delete (*wi);
		_mutex_w.lock();
		wi = wmp;
	    }
	    _mutex_w.unlock();

	    _mutex_t.lock();
	    std::list<Timeout*>::iterator ti = _timeouts.begin();
	    while (ti != _timeouts.end()) {
		std::list<Timeout*>::iterator tmp = ti;
		++tmp;
		_mutex_t.unlock();
		delete (*ti);
		_mutex_t.lock();
		ti = tmp;
	    }
	    _mutex_t.unlock();
	}


	void Dispatcher::dispatch() {

	    if ( ! _running ) { return; }

	    _mutex_w.lock();
	    int nfd = _watches.size();
	    pollfd fds[nfd];
	    std::list<Watch*>::iterator wi = _watches.begin();

	    for (nfd = 0; wi != _watches.end(); ++wi) {
		if ((*wi)->enabled()) {
		    fds[nfd].fd = (*wi)->descriptor();
		    fds[nfd].events = (*wi)->flags();
		    fds[nfd].revents = 0;

		    ++nfd;
		}
	    }
	    _mutex_w.unlock();

	    int wait_min = 10000;
	    std::list<Timeout*>::iterator ti;

	    _mutex_t.lock();
	    for (ti = _timeouts.begin(); ti != _timeouts.end(); ++ti) {
		if ((*ti)->enabled() && (*ti)->interval() < wait_min)
		    wait_min = (*ti)->interval();
	    }
	    _mutex_t.unlock();

	    poll(fds, nfd, wait_min);

	    timeval now;
	    gettimeofday(&now, NULL);
	    double now_millis = millis(now);

	    _mutex_t.lock();
	    ti = _timeouts.begin();

	    while (ti != _timeouts.end()) {
		std::list<Timeout*>::iterator tmp = ti;
		++tmp;

		if ((*ti)->enabled() && now_millis >= (*ti)->_expiration) {
		    (*ti)->expired(*(*ti));

		    if ((*ti)->_repeat) {
			(*ti)->_expiration = now_millis + (*ti)->_interval;
		    }
		}
		ti = tmp;
	    }
	    _mutex_t.unlock();

	    _mutex_w.lock();
	    for (int j = 0; j < nfd; ++j) {
		std::list<Watch*>::iterator wi;

		for (wi = _watches.begin(); wi != _watches.end();) {
		    std::list<Watch*>::iterator tmp = wi;
		    ++tmp;

		    if ((*wi)->enabled() && (*wi)->_fd == fds[j].fd) {
			if (fds[j].revents) {
			    (*wi)->_state = fds[j].revents;
			    (*wi)->ready(*(*wi));
			    fds[j].revents = 0;
			}
		    }
		    wi = tmp;
		}
	    }
	    _mutex_w.unlock();
	}
    }
}
