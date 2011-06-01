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
		cptr<T> &operator=( cptr<T> &other ) { ptr = other.ptr; return *this; }
		cptr<T> &operator=( T *&optr ) { ptr = optr; optr = 0; return *this; }
		std::string state( ) const {
		    char buf[128];
		    sprintf( buf, "0x%lx (%d)", (unsigned long) ptr.val, *ptr.count );
		    return std::string(buf);
		}
	    private:
		struct kernel {
		    T *val;
		    unsigned int *count;
		    kernel(T *v) : val(v), count(new unsigned int) { *count = 1u; }
		    kernel( const kernel &other ) : val(other.val), count(other.count) { *count += 1u; }
		    ~kernel( ) { release( ); }
		    void operator=( kernel &other ) { release( ); val = other.val; count = other.count; *count += 1u; }
		    void operator=( T *oval ) { release( ); val = oval; count = new unsigned int; *count = 1u; }
		    void release( ) { if ( --*count == 0u ) { delete val; delete count; } }
		};
		kernel ptr;
	};
   }
}

#endif
		  
		    
