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
#include <algorithm>

namespace casa {
    namespace viewer {

	struct strip_chars {
	    // should generalize to strip 'chars'...
	    strip_chars( const char */*chars*/ ) { }
	    strip_chars(const strip_chars &other) : str(other.str) { }
	    operator std::string( ) const { return str; }
	    void operator( )( const char &c ) { if ( c != '[' && c != ']' ) str += c; }
	private:
	    std::string str;
	};

	ImageProperties::ImageProperties( )  : status_ok(false), has_direction_axis(false),
					       has_spectral_axis(false) { }

	ImageProperties::ImageProperties( const std::string &path ) { reset(path); }

	const ImageProperties &ImageProperties::operator=( const std::string &path ) { reset(path); return *this; }

	Vector<double> ImageProperties::freqRange( const std::string &units ) const {
	    if ( units.size( ) == 0 )
		return freq_range;
	    Quantum<Vector<double> > range(freq_range,freq_units.c_str( ));
	    range.convert(units.c_str( ));
	    return range.getValue( );
	}

	Vector<double> ImageProperties::veloRange( const std::string &units ) const {
	    if ( units.size( ) == 0 )
		return velo_range;
	    Quantum<Vector<double> > range(velo_range,velo_units.c_str( ));
	    range.convert(units.c_str( ));
	    return range.getValue( );
	}

	void ImageProperties::reset( const std::string &path ) {

	    // clear settings...
	    status_ok = false;
	    path_ = path;
	    has_direction_axis = false;
	    has_spectral_axis = false;
	    direction_type = "";
	    shape_.resize(0);
	    freq_range.resize(0);
	    freq_units = "";
	    velo_range.resize(0);
	    velo_units = "";
	    ra_range.resize(0);
	    ra_range_str.resize(0);
	    dec_range.resize(0);
	    dec_range_str.resize(0);
	    beam_vec.resize(0);
	    beam_string_vec.resize(0);
	    // check for validity...
	    if ( path_ == "" ) return;
	  
	    if ( ImageOpener::imageType(path_) != ImageOpener::AIPSPP ||
		 imagePixelType(path_) != TpFloat )
		return;

	    PagedImage<Float> image(path_, TableLock::AutoNoReadLocking);
	    if ( image.ok( ) == false )
		return;

	    cs_ = image.coordinates( );
	    ImageAnalysis ia(&image);
	    shape_ = image.shape( ).asVector( );
	    if ( shape_.size( ) <= 0 )
		return;

	    // initialize...
	    status_ok = true;

	    if ( cs_.hasDirectionCoordinate( ) ) {
		has_direction_axis = true;
		const DirectionCoordinate &direction = cs_.directionCoordinate( );
		direction_type = MDirection::showType(direction.directionType( ));
		Vector<double> refval = direction.referenceValue( );
		if ( refval.size( ) == 2 ) {
		    Vector<int> direction_axes = cs_.directionAxesNumbers( );
		    Vector<double> pix(direction_axes.size( ));
		    for ( unsigned int x=0; x < pix.size( ); ++x ) pix[x] = 0;
		    Vector<double> world (pix.size( ));
		    direction.toWorld(world,pix);

		    ra_range.resize(2);
		    dec_range.resize(2);
		    casa::String units;
		    ra_range[0] = world[0];
		    ra_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[0], 0, true, true ));
		    dec_range[0] = world[1];
		    dec_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[1], 1, true, true ));

		    for ( unsigned int x=0; x < pix.size( ); ++x ) pix[x] = shape_[direction_axes[x]];
		    direction.toWorld(world,pix);
		    ra_range[1] = world[0];
		    ra_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[0], 0, true, true ));
		    dec_range[1] = world[1];
		    dec_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[1], 1, true, true ));
		}
	    }

	    if ( cs_.hasSpectralAxis( ) && shape_[cs_.spectralAxisNumber( )] > 1 ) {
		has_spectral_axis = true;
		SpectralCoordinate spec = cs_.spectralCoordinate( );
		Vector<String> spec_unit_vec = spec.worldAxisUnits( );
		if ( spec_unit_vec(0) == "Hz" ) spec_unit_vec(0) = "GHz";
		spec.setWorldAxisUnits(spec_unit_vec);
		freq_range.resize(2);
		double last_channel = shape_[cs_.spectralAxisNumber( )]-1;
		if ( spec.toWorld(freq_range(0),0) == false ||
		     spec.toWorld(freq_range(1),last_channel) == false ) {
		    has_spectral_axis = false;
		    freq_range.resize(0);
		} else {
		    freq_units = spec_unit_vec(0);
		    spec.setVelocity( "km/s" );
		    velo_range.resize(2);
		    if ( spec.pixelToVelocity(velo_range(0),0) == false ||
			 spec.pixelToVelocity(velo_range(1),last_channel) == false ) {
			velo_range.resize(0);
		    } else {
			velo_units = "km/s";
		    }
		}
	    }

	    std::ostringstream buf;
	    ImageInfo ii = image.imageInfo();
	    beam_vec = ii.restoringBeam();
	    if ( beam_vec.size( ) == 3 &&
		 beam_vec[0].isConform("arcsec") &&
		 beam_vec[1].isConform("arcsec") &&
		 beam_vec[2].isConform("deg") ) {
		char buf[512];
		sprintf( buf,"%.2f\"", beam_vec[0].getValue("arcsec") );
		beam_string_vec.push_back(buf);
		sprintf( buf,"%.2f\"", beam_vec[1].getValue("arcsec") );
		beam_string_vec.push_back(buf);
		sprintf( buf,"%.2f%c", beam_vec[2].getValue("deg"), 0x00B0 );
		beam_string_vec.push_back(buf);
	    }

	}

    }
}

