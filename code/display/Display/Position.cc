//# Position.cc: represent a point (or line or plane) with respect to an image coordinate system
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
#include <display/Display/Position.h>

namespace casa {
	namespace viewer {

		/*static std::ostream &operator<<( std::ostream &out, const casa::CoordinateSystem &csys ) {
			size_t size = csys.nCoordinates( );
			out << "[";
			for ( size_t i=0; i < size; ++i ) {
				out << csys.showType(i);
				if ( i != size-1 ) out << ",";
			}
			out << "]";
			return out;
		}*/

		void Position::show( std::ostream &out ) const {

			out << "(";
			for ( size_t i = 0; i < coord_.size( ); ++i ) {
				out << coord_(i);
				if ( i != coord_.size( )-1 ) out << ",";
			}
			out << "):" << coord_;
		}
	}
}
