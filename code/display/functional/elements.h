//# elements.h: templates, classes and functions for "functional" programming, e.g. with STL iterators
//# with surrounding Gui functionality
//# Copyright (C) 2005,2009
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef DISPLAY_FUNCTIONAL_ELEMENTS_H_
#define DISPLAY_FUNCTIONAL_ELEMENTS_H_
#include <math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	namespace viewer {

		// class for applying a range limit to values...
		template<typename T> class RangeLimiter {
		public:
			// No-Op range limiter...
			RangeLimiter( T (*mod)(T) = 0 ) : noop_(true), mod_(mod) { }
			RangeLimiter( T min, T max, T (*mod)(T) = 0 ) : noop_(false), min_(min), max_(max), mod_(mod) { }
			RangeLimiter( const RangeLimiter &other ) : noop_(other.noop_), min_(other.min_), max_(other.max_), mod_(other.mod_) { }
			const RangeLimiter &operator=( const RangeLimiter &other ) {
				noop_ = other.noop_;
				min_ = other.min_;
				max_ = other.max_;
				mod_ = other.mod_;
				return *this;
			}
			virtual T operator( )( T value ) {
				return noop_ ? (mod_ ? (*mod_)(value) : value) : value < min_ ? min_ : value > max_ ? max_ : (mod_ ? (*mod_)(value) : value);
			}
			virtual ~RangeLimiter( ) { }
		private:
			bool noop_;
			T min_;
			T max_;
			T (*mod_)(T);
		};

		template<typename T,typename CT=std::vector<T> > class filter {
		public:
			enum comparisons { EQUAL, UNEQUAL };
			filter( T compare_element, comparisons c=UNEQUAL ) {
				comparitor = compare_element;
				if ( c == EQUAL ) op = &filter<T,CT>::equality;
				else op = &filter<T,CT>::inequality;
			}
			void operator( )( T ele ) {
				if ( (this->*op)(ele) ) cache.push_back(ele);
			}
			operator CT( ) { return cache; }
			void clear( ) { cache.clear( ); }
		private:
			T comparitor;
			bool (filter<T,CT>::*op)(T);
			bool equality( T ele ) { return ele == comparitor; }
			bool inequality( T ele ) { return ele != comparitor; }
			CT cache;
		};
			

	}
}


#endif
