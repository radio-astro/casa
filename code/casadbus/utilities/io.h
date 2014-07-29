//# IO.h: IO utilities for dbus values
//# Copyright (C) 2013
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

#ifndef CASADBUS_IO_H_
#define CASADBUS_IO_H_

#include <sstream>
#include <iterator>

#include <casadbus/types/variant.h>

/********************************************************************************************
*********************************************************************************************
**** The first attempt at generalized std-c++ was based upon "copying" elements of data  ****
**** structures to an output stream, as with:                                            ****
**** --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ****
****	std::ostream_iterator<std::pair<std::string,dbus::variant> > out( cout, ", " );  ****
****	std::copy( details.begin( ), details.end( ), out );                              ****
****	std::cout << std::endl;                                                          ****
**** --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ****
**** This proved to be unsuccessful (and infact this example does not work) because      ****
**** there is no way to provide a shift operator for non-standard types which will allow ****
**** std::pair<K,V> to be output. For example, defining a shift operator for this        ****
**** example like:                                                                       ****
**** --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ****
**** ostream &operator<<( ostream &out, const std::pair<std::string,                     ****
****                      casa::dbus::variant> &p );                                     ****
**** --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ****
**** ONLY works if this operator is added to the "std" namespace. Instead, these         ****
**** functions for converting arbitrary types to strings are inteded to be used with the ****
**** std::transform(...) function, like:                                                 ****
**** --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ****
****	std::string toStdString( const std::pair<size_t,size_t> & data) {                ****
****		std::ostringstream str;                                                      ****
****		str << data.first << ", " << data.second;                                    ****
****		return str.str();                                                            ****
**** 	}                                                                                ****
****                                                                                     ****
****	std::transform( a_map.begin(), a_map.end(),                                      ****
****	                std::ostream_iterator<std::string>( std::cout, "\n" ),           ****
****	                toString );                                                      ****
*********************************************************************************************
********************************************************************************************/
namespace casa {
	namespace dbus {

		std::string asString( const variant &v );

		inline std::string asString( const std::string &s ) { return s; }

		template<typename T>
		std::string asString( const T &t ) {
			std::ostringstream s;
			s << asString(t);
			return s.str( );
		}

		template<typename K, typename V>
		std::string asString( const std::pair<K,V> &p ) {
			std::ostringstream s;
			s << asString(p.first) << ": " << asString(p.second);
			return s.str();
		}
	}
}

#endif
