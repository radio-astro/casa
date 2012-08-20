//# ImageProperties.qo.h: an object that collects data-range and other info about a casa image
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

#ifndef DISPLAY_IMAGEINFO_H_
#define DISPLAY_IMAGEINFO_H_
#include <string>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

namespace casa {
    namespace viewer {

	// This class provides a priori image information derived from the image itself. It was
	// created to standardize the access to image properties, since this information is needed
	// in a variety of places and is currently found in a variety of manners. The idea was that
	// it would provide easy, standardized (e.g. velocity provided in km/s) access. There are still
	// likely issues to iron out with expericence from new images.
	//
	// It may be desirable to have 2nd order image information, e.g. which axes map to the x, y
	// and z viewer display axes (or perhaps not), but if so, this information should be provided
	// by a derived class.
	//
	class ImageProperties {

	    public:
		ImageProperties( );
		ImageProperties( const std::string &/*path*/ );
		const ImageProperties &operator=( const std::string & );

		bool hasDirectionAxis( ) const { return has_direction_axis; }
		const std::string &directionType( ) const { return direction_type; }
		bool hasSpectralAxis( ) const { return has_spectral_axis; }
		const Vector<Int> &shape( ) const { return shape_; }
		Vector<double> raRange( ) const { return ra_range; }
		std::vector<std::string> raRangeAsStr( ) const { return ra_range_str; }
		Vector<double> decRange( ) const { return dec_range; }
		std::vector<std::string> decRangeAsStr( ) const { return dec_range_str; }
		Vector<Quantum<Double> > restoringBeam( ) const { return beam_vec; }
		std::vector<std::string> restoringBeamAsStr( ) const { return beam_string_vec; }
		Vector<double> freqRange( const std::string &units="" ) const;
		const std::string &freqUnits( ) const { return freq_units; }
		Vector<double> veloRange( const std::string &units="" ) const;
		const std::string &veloUnits( ) const { return velo_units; }
		const std::string &path( ) const { return path_; }
		bool ok( ) const { return status_ok; }

		// export required CoordinateSystem functions instead of returning a CoordinateSystem reference...
		int spectralAxisNumber() const { return cs_.spectralAxisNumber( ); }

	    private:
		void reset( const std::string &path="" );
		bool status_ok;
		std::string path_;
		Vector<Int> shape_;
		bool has_direction_axis;
		std::string direction_type;
		bool has_spectral_axis;
		Vector<double> freq_range;
		std::string freq_units;
		Vector<double> velo_range;
		std::string velo_units;
		Vector<double> ra_range;
		std::vector<std::string> ra_range_str;
		Vector<double> dec_range;
		std::vector<std::string> dec_range_str;
		Vector<Quantum<Double> > beam_vec;
		std::vector<std::string> beam_string_vec;
		CoordinateSystem cs_;
	};
    }
}


#endif
