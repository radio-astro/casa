//# Colormap.h: generic colormap handling for graphpad.
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

#if ! defined(_GRAPHPAD_COLORMAP_H_)
#define _GRAPHPAD_COLORMAP_H_
#include <agg2/agg_basics.h>
#include <vector>

namespace graphpad {

    // must support exp or log scaling of values prior to colormap application either
    // by parameterizing colormap (ctor) or parameterizing access function (parameter or template)
    template<typename Pixel>
    class Colormap {
	public:
	    typedef Pixel return_type;
	    Colormap( double minv, double maxv ) : min_val(minv), val_range(maxv-minv) { }
	    virtual ~Colormap( ) { }
	    virtual Pixel operator[]( double val ) const = 0;
	    Pixel operator( )(double val) const { return (*this)[val]; }
	    virtual void setRange( double min, double max ) {
		min_val = min;
		val_range = max-min;
	    }
	protected:
	    double min_val;
	    double val_range;
    };

    // default contructors are required for standard containers...
    template<typename Pixel>
    class CalcColormap : public Colormap<Pixel> {
	public:
	    CalcColormap( ) : Colormap<Pixel>(0,1), r_diff((typename Pixel::value_type)0xffff),
				r_diff((typename Pixel::value_type)0xffff), b_diff((typename Pixel::value_type)0xffff) { }
	    CalcColormap( double minv, double maxv, Pixel minc, Pixel maxc ) :
				Colormap<Pixel>(minv,maxv), min_color(minc),
				r_diff(maxc.r-minc.r), g_diff(maxc.g-minc.g), b_diff(maxc.b-minc.b) { }
	    Pixel operator[]( double val ) const {
		double scale = ((val-Colormap<Pixel>::min_val)/Colormap<Pixel>::val_range);
		return Pixel( min_color.r+(typename Pixel::value_type)(scale*r_diff),
			      min_color.g+(typename Pixel::value_type)(scale*g_diff),
			      min_color.b+(typename Pixel::value_type)(scale*b_diff) );
	    }
	private:
	    Pixel min_color;
	    double r_diff;
	    double g_diff;
	    double b_diff;
    };


    template<typename Pixel>
    class LookupColormap : public Colormap<Pixel> {
	public:
	    LookupColormap( ) : Colormap<Pixel>(0,1) { }
	    LookupColormap( double minv, double maxv ) : Colormap<Pixel>(minv,maxv) { }

	    template<typename Generator>
	    void fill( Generator generator, size_t size ) {
		colors.resize(size);
		std::generate(colors.begin(), colors.end(), generator);
	    }

	    Pixel operator[]( double val ) const {
		// could interpolate between colors...
		// some casa colormaps have just two colors...
		size_t off = (size_t)((double)colors.size()*(val-super::min_val)/super::val_range);
		return colors[off];
	    }
	private:
	    typedef Colormap<Pixel> super;
	    std::vector<Pixel> colors;
    };
}
#endif
