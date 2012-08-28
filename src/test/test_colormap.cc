//# test_colormap.cc: test calculated colormaps and casa static colormaps
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
#include <map>
#include <iostream>
#include <graphpad/Colormap.h>
#include <agg2/agg_color_rgba.h>
#include <agg2/agg_dda_line.h>
#include <agg2/agg_rendering_buffer.h>
#include <agg2/agg_pixfmt_rgb.h>
#include <agg2/agg_renderer_base.h>
#include <graphpad/couplers/CasaColormapGen.h>

enum { square_size  = 200 };

// calc example ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
class color_interpolator_calc {
    public:
	color_interpolator_calc(agg::rgba8 c1, agg::rgba8 c2, unsigned len) : off(0), cb(0, len,c1, c2) { }

	void operator ++( ) { ++off; }

	void operator+=(agg::rgba8::value_type delta ) { off += delta; }

	agg::rgba8 color() const { return cb[off]; }

    private:
	double off;
	graphpad::CalcColormap<agg::rgba8> cb;
};

// AGG example ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
class color_interpolator_rgba8 {
    public:
        color_interpolator_rgba8(agg::rgba8 c1, agg::rgba8 c2, unsigned len) :
            m_r(c1.r, c2.r, len), m_g(c1.g, c2.g, len), m_b(c1.b, c2.b, len), m_a(c1.a, c2.a, len) { }

        void operator ++( ) {
            ++m_r; ++m_g; ++m_b; ++m_a;
        }

	void operator+=(agg::rgba8::value_type delta ) {
	    m_r += delta; m_g += delta; m_b += delta; m_a += delta;
	}

        agg::rgba8 color() const {
            return agg::rgba8(m_r.y(), m_g.y(), m_b.y(), m_a.y());
        }

    private:
        agg::dda_line_interpolator<16> m_r;
        agg::dda_line_interpolator<16> m_g;
        agg::dda_line_interpolator<16> m_b;
        agg::dda_line_interpolator<16> m_a;
};


// Rendering a square with color interpolation between its corners
// The colors of the corners are ordered CCW started from bottom-left,
// assuming that the Y axis goes up.
//------------------------------------------------------------------

template<class Renderer,class Interpolator>
void color_square_rgba8( Renderer& r, int x, int y, int size,
			 agg::rgba8 c1, agg::rgba8 c2, agg::rgba8 c3, agg::rgba8 c4) {
    int i, j;
    Interpolator cy1(c1, c4, size);
    Interpolator cy2(c2, c3, size);
    for(i = 0; i < size; ++i) {
        Interpolator cx(cy1.color(), cy2.color(), size);
        for(j = 0; j < size; ++j) {
            r.copy_pixel(x + j, y + i, cx.color());
            ++cx;
        }
        ++cy1;
        ++cy2;
    }
}


template<class Renderer,class COLORMAP>
void color_bar_rgba8( Renderer& r, COLORMAP cmap ) {
    cmap.setRange(0,1);
    for(size_t i = 0; i < r.height( ); ++i) {
	agg::rgba8 pix = cmap[(double)i/(double)r.height( )];

        for(size_t j = 0; j < r.width( ); ++j) {
	    r.copy_pixel(j , i, pix);
        }
    }
}

// Writing the buffer to a .PPM file, assuming it has
// RGB-structure, one byte per color component
//--------------------------------------------------
bool write_ppm( const unsigned char* buf, unsigned width, unsigned height, const std::string &file_name ) {
    FILE* fd = fopen(file_name.c_str( ), "wb");
    if(fd) {
        fprintf(fd, "P6 %d %d 255 ", width, height);
        fwrite(buf, 1, width * height * 3, fd);
        fclose(fd);
        return true;
    }
    return false;
}

int main( int argc, char *argv[] ) {
    using namespace graphpad;


    CalcColormap<agg::rgba8> cmap(0,square_size,agg::rgba8(0xc6,0,0),agg::rgba8(0xc6,0xff,0xff));
    std::cout << "[1]: sampling pixels ";
    for ( double i = 0; i < square_size; i+=20 ) {
	agg::rgba8 pix = cmap[i];
	printf( "(0x%x,0x%x,0x%x)/%d ", pix.r, pix.g, pix.b, pix.a );
    }
    std::cout << std::endl;

    color_interpolator_rgba8 dda_cmap( agg::rgba8(0xc6,0,0), agg::rgba8(0xc6,0xff,0xff), square_size );
    std::cout << "[2]:                 ";
    for ( agg::rgba8::value_type i = 0; i < square_size; i+=10, dda_cmap+=20 ) {
	agg::rgba8 pix = dda_cmap.color( );
	printf( "(0x%x,0x%x,0x%x)/%d ", pix.r, pix.g, pix.b, pix.a );
    }
    std::cout << std::endl;

    // AGG color square ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
{   unsigned char* buffer = new unsigned char[square_size * square_size * 3];

    agg::rendering_buffer                 rbuf( buffer,
						square_size,
						square_size,
						-square_size * 3); // Flip Y to go up

    agg::pixfmt_rgb24                     pf(rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> rbase(pf);

    color_square_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,color_interpolator_rgba8>( rbase, 0, 0, square_size,
											agg::rgba8(0xc6, 0,    0),     // Bottom-left
											agg::rgba8(0xc6, 0,    0xff),  // Bottom-right
											agg::rgba8(0xc6, 0xff, 0xff),  // Top-right
											agg::rgba8(0xc6, 0xfe, 0));    // Top-left


    write_ppm(buffer, square_size, square_size, "agg_test.ppm");
    delete [] buffer;
}
    // calc color square ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
{   unsigned char* buffer = new unsigned char[square_size * square_size * 3];

    agg::rendering_buffer                 rbuf( buffer,
						square_size,
						square_size,
						-square_size * 3); // Flip Y to go up

    agg::pixfmt_rgb24                     pf(rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> rbase(pf);

    color_square_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,color_interpolator_calc>( rbase, 0, 0, square_size,
										       agg::rgba8(0xc6, 0,    0),     // Bottom-left
										       agg::rgba8(0xc6, 0,    0xff),  // Bottom-right
										       agg::rgba8(0xc6, 0xff, 0xff),  // Top-right
										       agg::rgba8(0xc6, 0xfe, 0));    // Top-left


    write_ppm(buffer, square_size, square_size, "agg_test2.ppm");
    delete [] buffer;
}

    std::cout << "[3]: agg_test.ppm and agg_test2.ppm should be nearly (roundoff) identical..." << std::endl;

    // casa colormaps    ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    typedef std::map<std::string, LookupColormap<agg::rgba8> > casa_colormaps_type;
    casa_colormaps_type casa_colormaps;
    CasaColormapGen<agg::rgba8> cmapgen("./casa_colormaps/default.tbl");
    cmapgen.begin( );
    size_t loaded_colormaps = 0;
    for ( size_t i=0; i < cmapgen.numColormaps( ); ++i, cmapgen.nextColormap( ) ) {
	std::cout << "[" << (4+i) << "]: loading " << cmapgen.name( ) << "[" << cmapgen.size( ) << "]" << std::endl;
	LookupColormap<agg::rgba8> casacmap( 0, 200 );
	casacmap.fill( cmapgen, cmapgen.size( ) );
	casa_colormaps.insert(casa_colormaps_type::value_type(cmapgen.name( ),casacmap));
	++loaded_colormaps;
    }


{   unsigned char* buffer = new unsigned char[256 * 30 * 3];

    agg::rendering_buffer                 rbuf( buffer,
						30,
						256,
						-30 * 3); // Flip Y to go up

    agg::pixfmt_rgb24                     pf(rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> rbase(pf);

    std::string cur_colormap_name = "Rainbow 2";
    std::string cur_dump_file = "rainbow2.ppm";
    std::cout << "[" << (4+loaded_colormaps) << "]: saving " << cur_colormap_name << " as " << cur_dump_file << std::endl;
    color_bar_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,LookupColormap<agg::rgba8> >( rbase, casa_colormaps[cur_colormap_name] );
    write_ppm(buffer, 30, 256, cur_dump_file);

    cur_colormap_name = "Smooth 2";
    cur_dump_file = "smooth2.ppm";
    std::cout << "[" << (5+loaded_colormaps) << "]: saving " << cur_colormap_name << " as " << cur_dump_file << std::endl;
    color_bar_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,LookupColormap<agg::rgba8> >( rbase, casa_colormaps[cur_colormap_name] );
    write_ppm(buffer, 30, 256, cur_dump_file);

    cur_colormap_name = "Cube Helix";
    cur_dump_file = "cubehelix.ppm";
    std::cout << "[" << (6+loaded_colormaps) << "]: saving " << cur_colormap_name << " as " << cur_dump_file << std::endl;
    color_bar_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,LookupColormap<agg::rgba8> >( rbase, casa_colormaps[cur_colormap_name] );
    write_ppm(buffer, 30, 256, cur_dump_file);

    cur_colormap_name = "Hot Metal 2";
    cur_dump_file = "hotmetal2.ppm";
    std::cout << "[" << (7+loaded_colormaps) << "]: saving " << cur_colormap_name << " as " << cur_dump_file << std::endl;
    color_bar_rgba8<agg::renderer_base<agg::pixfmt_rgb24>,LookupColormap<agg::rgba8> >( rbase, casa_colormaps[cur_colormap_name] );
    write_ppm(buffer, 30, 256, cur_dump_file);

    delete [] buffer;
}

}
