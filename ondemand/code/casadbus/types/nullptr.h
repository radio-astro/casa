//# nullptr.h: pointer classes (currently only counted pointer) used within casadbus
//#
//# Copyright (C) 2011
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

#ifndef __casadbus_nullptr_h__
#define __casadbus_nullptr_h__
#include <tr1/memory.hpp>

namespace casa {

    namespace memory {

	class _nullptr_t_ {
	    public:
		template<typename T> operator std::tr1::shared_ptr<T>( ) const { return std::tr1::shared_ptr<T>( ); }
		template<typename T> bool check( const std::tr1::shared_ptr<T> &o ) const { return o == std::tr1::shared_ptr<T>( ); }
	    private:
		friend class nullptr_init_;
		void do_init( ) { }
	};

	extern _nullptr_t_ anullptr;

	static class nullptr_init_ {
	    public:
		nullptr_init_( ) { if ( count++ == 0 ) do_init( ); }
		~nullptr_init_( ) { if ( --count == 0 ) { /* could destruct nullptr */ } }
	    private:
		static unsigned int count;
		void do_init( );
	} _nullptr_init_object_;

    }
}

#endif
