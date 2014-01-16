//# DisplayCoordinateSystem.cc: hold a collection of coordinates
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
//#
//# $Id: CoordinateSystem.cc 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#include <display/Display/DisplayCoordinateSystem.h>

namespace casa {

    Vector<String> DisplayCoordinateSystem::worldAxisUnits() const {
        return cs_.worldAxisUnits( );
    }
    Vector<String> DisplayCoordinateSystem::worldAxisNames() const {
        return cs_.worldAxisNames( );
    }
    bool DisplayCoordinateSystem::setWorldAxisNames( const Vector<String> &names ) {
        return cs_.setWorldAxisNames(names);
    }
    bool DisplayCoordinateSystem::setWorldAxisUnits( const Vector<String> &names ) {
        return cs_.setWorldAxisUnits(names);
    }

    Vector<int> DisplayCoordinateSystem::worldAxes(unsigned int whichCoord) const {
        return cs_.worldAxes(whichCoord);
    }
    Vector<int> DisplayCoordinateSystem::pixelAxes(unsigned int whichCoord) const {
        return cs_.pixelAxes(whichCoord);
    }

    Vector<int> DisplayCoordinateSystem::directionAxesNumbers() const {
        return cs_.directionAxesNumbers( );
    }

    String DisplayCoordinateSystem::format( String& units, Coordinate::formatType format, double worldValue, unsigned int worldAxis,
                                                 bool isAbsolute, bool showAsAbsolute, int precision, bool usePrecForMixed ) const {
        return cs_.format( units, format, worldValue, worldAxis, isAbsolute, showAsAbsolute, precision, usePrecForMixed );
    }
#if 0
     bool DisplayCoordinateSystem::toWorld( Vector<double> &world, const Vector<double> &pixel ) const {
        return cs_.toWorld(world,pixel);
     }
     bool DisplayCoordinateSystem::toPixel( Vector<double> &pixel, const Vector<double> &world ) const {
        return cs_.toPixel(pixel,world);
     }
#endif

    void DisplayCoordinateSystem::makePixelRelative( Vector<double>& pixel ) const {
		cs_.makePixelRelative(pixel);
    }
    void DisplayCoordinateSystem::makePixelAbsolute( Vector<double>& pixel ) const {
        cs_.makePixelAbsolute(pixel);
    }
    void DisplayCoordinateSystem::makeWorldAbsolute (Vector<double>& world) const {
		cs_.makeWorldAbsolute(world);
    }
    void DisplayCoordinateSystem::makeWorldRelative (Vector<double>& world) const {
        cs_.makeWorldRelative(world);
    }

    bool DisplayCoordinateSystem::toWorld( Vector<double> &world, const Vector<double> &pixel ) const {
		return cs_.toWorld(world,pixel);
    }
    bool DisplayCoordinateSystem::toPixel(Vector<double> &pixel, const Vector<double> &world) const {
		return cs_.toPixel(pixel,world);
    }

    bool DisplayCoordinateSystem::toMix( Vector<double>& worldOut,       Vector<double>& pixelOut,
                                         const Vector<double>& worldIn,  const Vector<double>& pixelIn,
                                         const Vector<bool>& worldAxes,  const Vector<bool>& pixelAxes,
                                         const Vector<double>& worldMin, const Vector<double>& worldMax) const {
        return cs_.toMix( worldOut, pixelOut, worldIn, pixelIn, worldAxes, pixelAxes, worldMin, worldMax );
    }

	DisplayCoordinateSystem DisplayCoordinateSystem::restore( const RecordInterface &container,const String &fieldName ) {
        CoordinateSystem *result = CoordinateSystem::restore(container,fieldName);
		if ( result == 0 ) throw AipsError("failed to read coordinate system...");
		DisplayCoordinateSystem cs(*result);
		delete result;
		return cs;
	}
		
    DisplayCoordinateSystem::~DisplayCoordinateSystem( ) { }

	void DisplayCoordinateSystem::transpose(const Vector<int> &newWorldOrder, const Vector<int> &newPixelOrder) {
		transposition_log.push_back( transposition_log_t::value_type(newWorldOrder,newPixelOrder) );
        cs_.transpose(newWorldOrder,newPixelOrder);
	}

	void DisplayCoordinateSystem::match( const DisplayCoordinateSystem &other ) {
		for ( transposition_log_t::const_iterator it=other.transposition_log.begin( );
			  it != other.transposition_log.end( ); ++it ) {
			transpose(it->first,it->second);
		}
	}

    Vector<int> DisplayCoordinateSystem::transposeShape( const Vector<int> &shape, bool world ) {
        Vector<int> result(shape.size());
        std::copy(shape.begin(), shape.end(), result.begin( ));
		for ( transposition_log_t::const_iterator it=transposition_log.begin( );
			  it != transposition_log.end( ); ++it ) {
            const Vector<int> &index = world ? it->first : it->second;
            Vector<int> orig(result.size( ));
            std::copy( result.begin(), result.end(), orig.begin( ));
            for ( int i=0; i < (int) result.size( ); ++i ) {
                if ( i != index[i] ) {
                    result[i] = orig[index[i]];
                }
            }
        }
        return result;
    }
}
