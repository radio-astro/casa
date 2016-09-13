//# Position.h: world coordinates of a point on a display panel
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

#ifndef CASA_DISPLAY_POSITION_H
#define CASA_DISPLAY_POSITION_H
#include <display/Display/DisplayCoordinateSystem.h>

namespace casa {
	namespace viewer {
		// Why not use MDirection? It's limited by valid astronomical coordinates on the sky Hz vs arcsec does not fly.
		// The idea here is to represent a point, line, plane, etc. (depending on the dimensions of the coordinate
		// system and the coordinate contained here) and eventually have positions which can convert themselves to
		// match different coordinate systems used in other images; removing some of the tangle of linToWorld(...)
		// et al. that snakes through canvases and display datas currently. It may well be that some of this may
		// need to remain in the current canvases/datas, but at least the viewer (and regions) will be able to talk
		// about positions independent of the rest of the viewer hierarchy.
		class Position {
		public:
			Position( ) { }
			// Later this could check for consistency between the coordinate and the system...
			Position( const DisplayCoordinateSystem &cs, const Quantity &x, const Quantity &y ) : csys_(cs), coord_(2) {
				coord_(0) = x;
				coord_(1) = y;
			}
			// Later this could check for consistency between the coordinate and the system...
			Position( const DisplayCoordinateSystem &cs, const Quantity &x, const Quantity &y, const Quantity &z ) : csys_(cs), coord_(3) {
				coord_(0) = x;
				coord_(1) = y;
				coord_(2) = z;
			}
			Position( const Position &that ) : csys_(that.csys_), coord_(that.coord_) { }

			const DisplayCoordinateSystem &csys( ) const {
				return csys_;
			}
			const Vector<Quantity> &coord( ) const {
				return coord_;
			}

			void show( std::ostream &out ) const;

		protected:
			DisplayCoordinateSystem csys_;
			Vector<Quantity> coord_;
		};

		inline std::ostream &operator<<( std::ostream &out, const casa::viewer::Position &pos ) {
			pos.show(out);
			return out;
		}

	}

	inline std::ostream &operator<<( std::ostream &out, const casa::viewer::Position &pos ) {
		pos.show(out);
		return out;
	}
}

inline std::ostream &operator<<( std::ostream &out, const casa::viewer::Position &pos ) {
	pos.show(out);
	return out;
}

#endif

