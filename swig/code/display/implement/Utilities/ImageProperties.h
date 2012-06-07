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

namespace casa {
    namespace viewer {
	class ImageProperties {
	    public:
		ImageProperties( );
		ImageProperties( const std::string &/*path*/ );
		const ImageProperties &operator=( const std::string & );
		bool hasDirectionAxis( ) const { return has_direction_axis; }
		const std::string &directionType( ) const { return direction_type; }
		bool hasSpectralAxis( ) const { return has_spectral_axis; }
		const Vector<Int> &shape( ) const { return shape_; }
		const Vector<double> &freqRange( ) const { return freq_range; }
		const std::string &freqUnits( ) const { return freq_units; }
		const Vector<double> &veloRange( ) const { return velo_range; }
		const std::string &veloUnits( ) const { return velo_units; }
		double beamArea( ) const { return beam_area; }
		const std::string &path( ) const { return path_; }
		bool ok( ) const { return status_ok; }
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
		double beam_area;
	};
    }
}


#endif
