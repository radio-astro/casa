//# Functional.h: tools for functional programming (in C++) or STL iterator tools
//# Copyright (C) 2012
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

#if ! defined(_GRAPHPAD_FUNCTIONAL_H_)
#define _GRAPHPAD_FUNCTIONAL_H_

namespace graphpad {
    namespace functional_internal_ {
	template<typename F, typename G>
	struct zero_arg_ {
		zero_arg_(F &fref, G &gref) : f(fref), g(gref) { }
		typename F::return_type operator( )( ) { return f(g( )); }
	    private:
		F &f;
		G &g;
	};

	template<typename F>
	struct one_arg_ {
		one_arg_(F &fref) : f(fref) { }
		template<typename G>
		zero_arg_<F,G> operator( )( G &g ) { return zero_arg_<F,G>(f,g); }
	    private:
		F &f;
	};
    }

    template<typename F>
    functional_internal_::one_arg_<F> compose( F &f ) { return functional_internal_::one_arg_<F>(f); }

}

#endif
