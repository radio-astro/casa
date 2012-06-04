//# ImageProperties.cc: an object that collects data-range and other info about a casa image
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

#include <display/Utilities/ImageProperties.h>
#include <images/Images/ImageOpener.h>

#include <float.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageAnalysis.h>
#include <sstream>
#include <casa/Arrays/VectorSTLIterator.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

namespace casa {
    namespace viewer {

	struct max_ftor {
	    max_ftor( ) : max(-FLT_MAX) { }
	    max_ftor( const max_ftor &other ) : max(other.max) { }
	    operator float( ) const { return max; }
	    void operator( )( float f ) { if ( f > max ) max = f; }
	private:
	    float max;
	};

	struct min_ftor {
	    min_ftor( ) : min(FLT_MAX) { }
	    min_ftor( const min_ftor &other ) : min(other.min) { }
	    operator float( ) const { return min; }
	    void operator( )( float f ) { if ( f < min ) min = f; }
	private:
	    float min;
	};

	ImageProperties::ImageProperties( const std::string &path ) : status_ok(false), path_(path), has_direction_axis(false),
								      has_spectral_axis(false),
								      velo_units("km/s"), beam_area(0) {

	    if ( ImageOpener::imageType(path) != ImageOpener::AIPSPP ||
		 imagePixelType(path) != TpFloat )
		return;

	    PagedImage<Float> image(path, TableLock::AutoNoReadLocking);
	    if ( image.ok( ) == false )
		return;

	    CoordinateSystem cs = image.coordinates( );
	    ImageAnalysis ia(&image);
	    shape_ = image.shape( ).asVector( );
	    if ( shape_.size( ) <= 0 )
		return;

	    status_ok = true;

	    if ( cs.hasDirectionCoordinate( ) ) {
		has_direction_axis = true;
		const DirectionCoordinate &direction = cs.directionCoordinate( );
		direction_type = MDirection::showType(direction.directionType( ));
	    }

	    if ( cs.hasSpectralAxis( ) && shape_[cs.spectralAxisNumber( )] > 1 ) {
		has_spectral_axis = true;
		Vector<Double> refval(cs.referenceValue( ));
		const SpectralCoordinate &spec = cs.spectralCoordinate( );
		Vector<String> spec_unit_vec = spec.worldAxisUnits( );
		if ( spec_unit_vec(0) == "Hz" ) spec_unit_vec(0) = "GHz";
		Vector<Double> xy(2);
		xy(0) = refval(0);
		xy(1) = refval(1);
		Vector<Float> zx,zy;
		if ( ia.getFreqProfile( xy, zx, zy, "world", "freq", 0, 0, 0, spec_unit_vec(0) ) ) {
		    freq_range.resize(2);
		    Float min = std::for_each(zx.begin(),zx.end(),min_ftor( ));
		    Float max = std::for_each(zx.begin(),zx.end(),max_ftor( ));
		    freq_range(0) = min;
		    freq_range(1) = max;
		    freq_units = spec_unit_vec(0);
		}

		if ( ia.getFreqProfile( xy, zx, zy, "world", "radio velocity", 0, 0, 0, spec_unit_vec(0) ) ) {
		    velo_range.resize(2);
		    Float min = std::for_each(zx.begin(),zx.end(),min_ftor( ));
		    Float max = std::for_each(zx.begin(),zx.end(),max_ftor( ));
		    velo_range(0) = min;
		    velo_range(1) = max;
		}
	    }

	    ImageInfo ii = image.imageInfo();
	    Vector<Quantum<Double> > beam = ii.restoringBeam();
	    std::string imageUnits = image.units().getName();
	    std::transform( imageUnits.begin(), imageUnits.end(), imageUnits.begin(), ::toupper );
	    Int afterCoord = -1;
	    Int dC = cs.findCoordinate(Coordinate::DIRECTION, afterCoord);
	    // use contains() not == so moment maps are dealt with nicely
	    if ( beam.nelements()==3 && dC!=-1 && imageUnits.find("JY/BEAM") != std::string::npos ) {
		DirectionCoordinate dCoord = cs.directionCoordinate(dC);
		Vector<String> units(2);
		units(0) = units(1) = "rad";
		dCoord.setWorldAxisUnits(units);
		Vector<Double> deltas = dCoord.increment();

		Double major = beam(0).getValue(Unit("rad"));
		Double minor = beam(1).getValue(Unit("rad"));
		beam_area = C::pi/(4*log(2)) * major * minor / abs(deltas(0) * deltas(1));
	    }
	}
    }
}

