//# test_image.cc: test generating a ppm file from a casa image
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
#include <graphpad/couplers/CasaImageGen.h>
#include <graphpad/Functional.h>

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



    // ----------------------------------------------------------------------------------------------------
    // ----       load casa colormaps
    // ----------------------------------------------------------------------------------------------------
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

    // ----------------------------------------------------------------------------------------------------
    // ----       stream a channel from the image cube
    // ----------------------------------------------------------------------------------------------------
    CasaImageGen<float> im( "g35.03_II_nh3_11.hline.image" );
    im.begin( RA, DEC, FREQ );

    std::vector<int> shape = im.shape( );
    if ( shape.size( ) != 2 ) {
	fprintf( stderr, "bad shape for image channel...\n" );
	return(1);
    }

    std::pair<float,float> minmax = im.getMinMax( );

{   unsigned char* buffer = new unsigned char[shape[0] * shape[1] * 3];

    agg::rendering_buffer                 rbuf( buffer,
						shape[0],
						shape[1],
						-shape[0] * 3); // Flip Y to go up


    agg::pixfmt_rgb24                     pf(rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> rbase(pf);

    LookupColormap<agg::rgba8> colormap = casa_colormaps["Rainbow 2"];
    colormap.setRange( minmax.first, minmax.second );

    char filename[512];
    for ( size_t x = 84; x <= 90; ++x ) {
	im.select( x );
	// compose the image generator (im) with the colormap (colormap)
	// to map image values to raster pixels...
	lay_pixels( rbase, compose(colormap)(im) );
	sprintf( filename, "g35.03_II_nh3_11.hline.%lu.ppm", x );
	write_ppm(buffer, shape[0], shape[1], filename);
    }

    delete [] buffer;
}
    
}

