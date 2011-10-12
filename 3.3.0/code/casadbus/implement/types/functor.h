//# functor.h: declaration & definition of functors + args
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

#ifndef __casadbus_functor_h__
#define __casadbus_functor_h__
#include <casadbus/types/ptr.h>
#include <string>
#include <stdexcept>

namespace casa {
    namespace functor {

	class args { 
	    public:
		virtual ~args( ) { }
	};

	template<class T1> class args01 : public args {
	    public:
		args01( T1 v1 ) : a1(v1) { }
		T1 one( ) { return a1; }
	    private:
		T1 a1;
	};

	template<class T1, class T2> class args02 : public args {
	    public:
		args02( T1 v1, T2 v2 ) : a1(v1), a2(v2) { }
		T1 one( ) { return a1; }
		T2 two( ) { return a2; }
	    private:
		T1 a1;
		T2 a2;
	};

	template<class T1, class T2, class T3> class args03 : public args {
	    public:
		args03( T1 v1, T2 v2, T3 v3 ) : a1(v1), a2(v2), a3(v3) { }
		T1 one( ) { return a1; }
		T2 two( ) { return a2; }
		T3 three( ) { return a3; }
	    private:
		T1 a1;
		T2 a2;
		T3 a3;
	};

	template<class T1, class T2, class T3, class T4> class args04 : public args {
	    public:
		args04( T1 v1, T2 v2, T3 v3, T4 v4 ) : a1(v1), a2(v2), a3(v3), a4(v4) { }
		T1 one( ) { return a1; }
		T2 two( ) { return a2; }
		T3 three( ) { return a3; }
		T4 four( ) { return a4; }
	    private:
		T1 a1;
		T2 a2;
		T3 a3;
		T4 a4;
	};


	class invocation_exception : public std::runtime_error {
	    public:
		invocation_exception( ) : std::runtime_error( "functor/argument mismatch" ) { }
	};

	class f_ {
	    public:
		f_( ) { }
		virtual void invoke( args *a ) = 0;
		virtual ~f_( ) { }
	};

	class f {
	    public:
		f( const f &other ) : ptr(other.ptr) { }
		void operator()( args *a ) { ptr->invoke( a ); }
		void operator()( args &a ) { ptr->invoke( &a ); }
		std::string state( ) const { return ptr.state( ); }
		virtual ~f( ) { }
	    private:
		f( );
		f( f_ *p ) : ptr(p) { }
		void operator=(const f &);
		memory::cptr<f_> ptr;

		template <class C> friend f make( C *o, void (C::*i)( ) );
		template <class C, class P1> friend f make( C *o, void (C::*i)(P1) );
		template <class C, class P1, class P2> friend f make( C *o, void (C::*i)(P1,P2) );
		template <class C, class P1, class P2, class P3> friend f make( C *o, void (C::*i)(P1,P2,P3) );
		template <class C, class P1, class P2, class P3, class P4> friend f make( C *o, void (C::*i)(P1,P2,P3,P4) );
	};

	
	template <class C> class f00 : public f_ {
	    public:
		f00( C *o, void (C::*i)( ) ) : obj(o), impl(i) { }
		void invoke( ) { (*obj.*impl)( ); }
		void invoke( args *ga ) { (*this)( ); }
	    private:
		C *obj;
		void (C::*impl)( );
	};

	template <class C, class P1> class f01 : public f_ {
	    public:
		f01( C *o, void (C::*i)(P1) ) : obj(o), impl(i) { }
		void invoke( P1 a1 ) { (*obj.*impl)(a1); }
		void invoke( args *ga ) {
		    args01<P1> *a = dynamic_cast<args01<P1>*>(ga);
		    if ( a ) { (*this)(a->one()); }
		    else { throw invocation_exception( ); }
		}
	    private:
		C *obj;
		void (C::*impl)(P1);
	};

	template <class C, class P1, class P2> class f02 : public f_ {
	    public:
		f02( C *o, void (C::*i)(P1,P2) ) : obj(o), impl(i) { }
		void invoke( P1 a1, P2 a2 ) { (*obj.*impl)(a1,a2); }
		void invoke( args *ga ) {
		    args02<P1,P2> *a = dynamic_cast<args02<P1,P2>*>(ga);
		    if ( a ) { invoke(a->one(),a->two()); }
		    else { throw invocation_exception( ); }
		}
	    private:
		C *obj;
		void (C::*impl)(P1,P2);
	};

	template <class C, class P1, class P2, class P3> class f03 : public f_ {
	    public:
		f03( C *o, void (C::*i)(P1,P2,P3) ) : obj(o), impl(i) { }
		void invoke( P1 a1, P2 a2, P3 a3 ) { (*obj.*impl)(a1,a2,a3); }
		void invoke( args *ga ) {
		    args03<P1,P2,P3> *a = dynamic_cast<args03<P1,P2,P3>*>(ga);
		    if ( a ) { (*this)(a->one(),a->two(),a->three()); }
		    else { throw invocation_exception( ); }
		}
	    private:
		C *obj;
		void (C::*impl)(P1,P2,P3);
	};

	template <class C, class P1, class P2, class P3, class P4> class f04 : public f_ {
	    public:
		f04( C *o, void (C::*i)(P1,P2,P3,P4) ) : obj(o), impl(i) { }
		void invoke( P1 a1, P2 a2, P3 a3, P4 a4 ) { (*obj.*impl)(a1,a2,a3,a4); }
		void invoke( args *ga ) {
		    args04<P1,P2,P3,P4> *a = dynamic_cast<args04<P1,P2,P3,P4>*>(ga);
		    if ( a ) { (*this)(a->one(),a->two(),a->three(),a->four()); }
		    else { throw invocation_exception( ); }
		}
	    private:
		C *obj;
		void (C::*impl)(P1,P2,P3,P4);
	};

	template <class C> f make( C *o, void (C::*i)( ) )
		{ return f(new functor::f00<C>( o, i )); }
	template <class C, class P1> f make( C *o, void (C::*i)(P1) )
		{ return f(new functor::f01<C,P1>( o, i )); }
	template <class C, class P1, class P2> f make( C *o, void (C::*i)(P1,P2) )
		{ return f(new functor::f02<C,P1,P2>( o, i )); }
	template <class C, class P1, class P2, class P3> f make( C *o, void (C::*i)(P1,P2,P3) )
		{ return f(new functor::f03<C,P1,P2,P3>( o, i )); }
	template <class C, class P1, class P2, class P3, class P4> f make( C *o, void (C::*i)(P1,P2,P3,P4) )
		{ return f(new functor::f04<C,P1,P2,P3,P4>( o, i )); }

    }
}

#endif
