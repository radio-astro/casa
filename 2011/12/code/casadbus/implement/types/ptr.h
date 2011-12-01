//# ptr.h: pointer classes (currently only counted pointer) used within casadbus
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

#ifndef __casadbus_ptr_h__
#define __casadbus_ptr_h__
#include <string>
#include <stdio.h>

namespace casa {

    namespace memory {
	template <class T> class cptr {
	    public:
		cptr( ) : ptr((T*)0) { }
		cptr(T *p) : ptr(p) { }
		cptr<T>(const cptr<T> &other) : ptr(other.ptr) { }
		cptr<T>(cptr<T> *other) : ptr(other->ptr) { }
		T *operator->( ) { return ptr.val; }
		T &operator*( ) { return *ptr.val; }

		const T *operator->( ) const { return ptr.val; }
		const T &operator*( ) const { return *ptr.val; }

		bool isNull( ) const { return ptr.isNull( ); }
		const cptr<T> &operator=( const cptr<T> &other ) { ptr = other.ptr; return *this; }
		const cptr<T> &operator=( T *&optr ) { ptr = optr; optr = 0; return *this; }
		std::string state( ) const {
		    char buf[128];
		    sprintf( buf, "0x%lx (%d)", (unsigned long) ptr.val, *ptr.count );
		    return std::string(buf);
		}
		void clear( ) { ptr.clear( ); }
		unsigned int count( ) const { return *ptr.count; }

	    private:
		template <class X> friend bool operator==(const cptr<X>&, X*);
		template <class X> friend bool operator==(X*, const cptr<X>&);

		struct kernel {
		    T *val;
		    unsigned int *count;
		    kernel(T *v) : val(v), count(new unsigned int) { *count = 1u; }
		    kernel( const kernel &other ) : val(other.val), count(other.count) { *count += 1u; }
		    ~kernel( ) { release( ); }
		    void operator=( const kernel &other ) {  release( ); val = other.val; count = other.count; *count += 1u; }
		    void operator=( T *oval ) { release( ); val = oval; count = new unsigned int; *count = 1u; }
		    void release( ) {  if ( --*count == 0u ) { delete val; delete count; } }
		    bool isNull( ) const { return val == 0 ? true : false; }
		    void clear( ) { release( ); val = 0; count = new unsigned int; *count = 1u; }
		    bool eq( T *x ) const { return val == x; }
		};
		kernel ptr;
	};

	template<class T> bool operator==( const cptr<T> &l, T *r ) { return l.ptr.eq(r); }
	template<class T> bool operator==( T *l, const cptr<T> &r ) { return r.ptr.eq(l); }

	// considered introducing something like:
	//
	// template <class D, class B> class cptr_ref {
	//     public:
	//         cptr_ref(cptr<D> &);
	//         cptr_ref( const cptr_ref<D,B> &other);
	//         B *operator->( ) { return ref.operator->( ); }
	//         B &operator*( ) { return ref.operator*( ); }
	// };
	//
	// to represent a base class (B) reference to a counted pointer based on
	// a derived class (D). However, the derived class is still intertwined
	// with the type so it falls short for a number of derived classes.
    }

    using memory::operator==;

}

#endif
		  
		    
