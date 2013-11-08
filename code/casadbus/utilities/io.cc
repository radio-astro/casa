//# io.cc: implementation of I/O functions for dbus types
//#
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
#include <casadbus/utilities/io.h>
#include <casadbus/types/record.h>
#include <algorithm>

namespace casa {
	namespace dbus {

		std::string asString( const variant &v ) {
			std::ostringstream out;
			switch( v.type( ) ) {
				case variant::BOOL:
					out << v.getBool( );
					break;
				case variant::INT:
					out << v.getInt( );
					break;
				case variant::DOUBLE:
					out << v.getDouble( );
					break;
				case variant::COMPLEX:
					out << v.getComplex( );
					break;
				case variant::BOOLVEC:
					{	const std::vector<bool> &vec = v.getBoolVec( );
						std::ostream_iterator<bool> o( out, "," );
						std::copy( vec.begin( ), vec.end( ), o );
					}
					break;
				case variant::INTVEC:
					{	const std::vector<int> &vec = v.getIntVec( );
						std::ostream_iterator<int> o( out, "," );
						std::copy( vec.begin( ), vec.end( ), o );
					}
					break;
				case variant::DOUBLEVEC:
					{	const std::vector<double> &vec = v.getDoubleVec( );
						std::ostream_iterator<double> o( out, "," );
						std::copy( vec.begin( ), vec.end( ), o );
					}
					break;
				case variant::COMPLEXVEC:
					{	const std::vector<std::complex<double> > &vec = v.getComplexVec( );
						std::ostream_iterator<std::complex<double> > o( out, "," );
						std::copy( vec.begin( ), vec.end( ), o );
					}
					break;
				case variant::STRING:
					out << v.getString( );
					break;
				case variant::STRINGVEC:
					{	const std::vector<std::string> &vec = v.getStringVec( );
						std::ostream_iterator<std::string> o( out, "," );
						std::copy( vec.begin( ), vec.end( ), o );
					}
					break;
				case variant::RECORD:
					{	const casa::dbus::record &rec = v.getRecord( );
						std::ostream_iterator<std::string> o( out, "," );
						std::transform( rec.begin( ), rec.end( ), o,
										(std::string (*)(const std::pair<std::string,variant>&)) asString );
					}
					break;
				default:
					out << "*unrecognized*variant*";
					break;
			}
			return out.str( );
		}
	}
}
