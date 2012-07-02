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


	class f {
	    public:
		f( ) { }
		virtual void operator()( args *a ) = 0;
		virtual void operator()( args &a ) = 0;
		virtual ~f( ) { }
	};

	template <class C> class f00 : public f {
	    public:
		f00( C *o, void (C::*i)( ) ) : obj(o), impl(i) { }
		void operator( )( ) { (*obj.*impl)( ); }
		void operator( )( args *ga ) { (*this)( ); }
		void operator( )( args &a ) { (*this)(&a); }
	    private:
		C *obj;
		void (C::*impl)( );
	};

	template <class C, class P1> class f01 : public f {
	    public:
		f01( C *o, void (C::*i)(P1) ) : obj(o), impl(i) { }
		void operator( )( P1 a1 ) { (*obj.*impl)(a1); }
		void operator( )( args *ga ) {
		    args01<P1> *a = dynamic_cast<args01<P1>*>(ga);
		    if ( a ) { (*this)(a->one()); }
		    else { throw invocation_exception( ); }
		}
		void operator()( args &a ) { (*this)(&a); }
	    private:
		C *obj;
		void (C::*impl)(P1);
	};

	template <class C, class P1, class P2> class f02 : public f {
	    public:
		f02( C *o, void (C::*i)(P1,P2) ) : obj(o), impl(i) { }
		void operator( )( P1 a1, P2 a2 ) { (*obj.*impl)(a1,a2); }
		void operator( )( args *ga ) {
		    args02<P1,P2> *a = dynamic_cast<args02<P1,P2>*>(ga);
		    if ( a ) { (*this)(a->one(),a->two()); }
		    else { throw invocation_exception( ); }
		}
		void operator()( args &a ) { (*this)(&a); }
	    private:
		C *obj;
		void (C::*impl)(P1,P2);
	};

	template <class C, class P1, class P2, class P3> class f03 : public f {
	    public:
		f03( C *o, void (C::*i)(P1,P2,P3) ) : obj(o), impl(i) { }
		void operator( )( P1 a1, P2 a2, P3 a3 ) { (*obj.*impl)(a1,a2,a3); }
		void operator( )( args *ga ) {
		    args03<P1,P2,P3> *a = dynamic_cast<args03<P1,P2,P3>*>(ga);
		    if ( a ) { (*this)(a->one(),a->two(),a->three()); }
		    else { throw invocation_exception( ); }
		}
		void operator()( args &a ) { (*this)(&a); }
	    private:
		C *obj;
		void (C::*impl)(P1,P2,P3);
	};

	template <class C, class P1, class P2, class P3, class P4> class f04 : public f {
	    public:
		f04( C *o, void (C::*i)(P1,P2,P3,P4) ) : obj(o), impl(i) { }
		void operator( )( P1 a1, P2 a2, P3 a3, P4 a4 ) { (*obj.*impl)(a1,a2,a3,a4); }
		void operator( )( args *ga ) {
		    args04<P1,P2,P3,P4> *a = dynamic_cast<args04<P1,P2,P3,P4>*>(ga);
		    if ( a ) { (*this)(a->one(),a->two(),a->three(),a->four()); }
		    else { throw invocation_exception( ); }
		}
		void operator()( args &a ) { (*this)(&a); }
	    private:
		C *obj;
		void (C::*impl)(P1,P2,P3,P4);
	};

	template <class C> functor::f *make( C *o, void (C::*i)( ) )
		{ return new functor::f00<C>( o, i ); }
	template <class C, class P1> functor::f *make( C *o, void (C::*i)(P1) )
		{ return new functor::f01<C,P1>( o, i ); }
	template <class C, class P1, class P2> functor::f *make( C *o, void (C::*i)(P1,P2) )
		{ return new functor::f02<C,P1,P2>( o, i ); }
	template <class C, class P1, class P2, class P3> functor::f *make( C *o, void (C::*i)(P1,P2,P3) )
		{ return new functor::f03<C,P1,P2,P3>( o, i ); }
	template <class C, class P1, class P2, class P3, class P4> functor::f *make( C *o, void (C::*i)(P1,P2,P3,P4) )
		{ return new functor::f04<C,P1,P2,P3,P4>( o, i ); }

    }
}

#endif
