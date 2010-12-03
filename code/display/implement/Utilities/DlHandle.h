//# DlHandle.h: smart pointer for display library.
//# Copyright (C) 2010
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

#ifndef DLHANDLE_H_
#define DLHANDLE_H_
#include <cstddef>

namespace casa { //# NAMESPACE CASA - BEGIN

    class DlTarget;

    class DlHandleBase {

	protected:
	    DlHandleBase( ) { }
	    DlHandleBase( const DlHandleBase & ) { }
	    virtual void target_gone( ) const = 0;
	    virtual bool null( ) const = 0;

	    void throw_exception( const char * ) const;

	    virtual ~DlHandleBase( ) { }
	private:
	    friend class DlTarget;
	
	    // Prevent heap allocation
	    void * operator new   (size_t);
	    void * operator new[] (size_t);
    };


    template <class T> class DlHandle : public DlHandleBase {
	public:
	    DlHandle( ) : target_(0) { }
	    DlHandle( T *tgt ) : target_(tgt) { if ( target_ ) target_->reg(this); }
	    DlHandle( const DlHandle<T> &other ) : DlHandleBase(other), target_(other.target_) { if ( target_ ) target_->reg(this); }
	    T *operator->( ) { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return target_; }
	    const T *operator->( ) const { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return target_; }
	    operator T*( ) { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return target_; }
	    T &operator*( ) { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return *target_; }
	    operator const T*( ) const { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return target_; }
	    const T &operator*( ) const { if ( ! target_ ) throw_exception( "null pointer in DlHandle" ); return *target_; }
	    DlHandle<T> &operator=( const DlHandle<T> &other ) { if ( target_ ) target_->unreg(this); target_ = other.target_; if ( target_ ) target_->reg(this); return *this; }
	    T *operator=( T *tgt ) { if ( target_ ) target_->unreg(this); target_ = tgt; if ( target_ ) target_->reg(this); return target_; }

	    ~DlHandle( ) { if ( target_ ) target_->unreg(this); }

	    bool null( ) const { return target_ == 0; }

	protected:
	    void target_gone( ) const { ((DlHandle<T>*)this)->target_ = 0; }
	    T *target_;
    };
}
#endif
