//# test_image_affine.cc: test generating a ppm file from a casa image with affine transformation
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
#include <list>
#include <limits>
#include <iostream>
#include <tr1/tuple>
#include <algorithm>
#include <graphpad/Colormap.h>
#include <agg2/agg_color_rgba.h>

#include <agg2/agg_dda_line.h>
#include <agg2/agg_rendering_buffer.h>
#include <agg2/agg_pixfmt_rgb.h>
#include <agg2/agg_renderer_base.h>
#include <agg2/agg_scanline_u.h>
#include <agg2/agg_span_allocator.h>
#include <agg2/agg_rasterizer_scanline_aa.h>
#include <agg2/agg_renderer_scanline.h>
#include <agg2/agg_span_gouraud_rgba.h>
#include <agg2/agg_span_interpolator_linear.h>
#include <agg2/agg_span_image_filter_rgb.h>
#include <agg2/agg_span_solid.h>
#include <agg2/agg_trans_affine.h>
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

// Calculate layout of channel maps along one dimension...
//--------------------------------------------------
std::list<size_t> calculate_layout( size_t number, size_t pixel_length, size_t margin=0, size_t page_margin=0, size_t pixel_target=1200 ) {
    typedef std::tr1::tuple<size_t,size_t,double,bool> element_t;
    std::list<element_t> initial;
    // first look for an even division...
    for ( size_t x=2; x < number; ++x ) {
	double full_size = x * pixel_length + (x-1) * margin + page_margin * 2;
	double percent_of_target = full_size / (double) pixel_target;
	if ( percent_of_target > 1.1 ) break;		// go up to 10% over...
	if ( percent_of_target < 0.75 ) continue;	// but occupy at least 80% of target...
	initial.push_back(element_t(x, 1 + (number-1)/x, percent_of_target,(number % x == 0)));
    }
    std::list<size_t> result;
    for ( std::list<element_t>::reverse_iterator it=initial.rbegin( ); it != initial.rend( ); ++it ) {
	if ( std::tr1::get<3>(*it) )
	    result.push_back(std::tr1::get<0>(*it));
    }
    if ( result.size( ) == 0 ) {
	int difference = std::numeric_limits<int>::max( );
	size_t pick = 0;
	// next look for a division that has almost the same number of rows & columns...
	for ( std::list<element_t>::iterator it=initial.begin( ); it != initial.end( ); ++it ) {
	    double percent_of_target = std::tr1::get<2>(*it);
	    if ( percent_of_target < 0.75 || percent_of_target > 1.1 ) continue;
	    if ( abs((int)std::tr1::get<0>(*it)-(int)std::tr1::get<1>(*it)) < difference )
		pick = std::tr1::get<0>(*it);
	}
	if ( pick != 0 ) {
	    result.push_back(pick);
	}
    }
    return result;
}


int main( int argc, char *argv[] ) {
    using namespace graphpad;

    const size_t margin = 2;
    const size_t page_margin = 10;

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

    unsigned char* buffer = new unsigned char[shape[0] * shape[1] * 3];

    agg::rendering_buffer                 srcbuf( buffer,
						  shape[0],
						  shape[1],
						  -shape[0] * 3); // Flip Y to go up


    agg::pixfmt_rgb24                     srcpf(srcbuf);
    agg::renderer_base<agg::pixfmt_rgb24> source_image(srcpf);

    LookupColormap<agg::rgba8> colormap = casa_colormaps["Rainbow 2"];
    colormap.setRange( minmax.first, minmax.second );

    // divide and round up: 1 + ((x - 1) / y)
    int rsize[2] = { 1 + (shape[0]-1) / 4, 1 + (shape[1]-1) / 4 };  // quarter scale
    std::list<size_t> layouts = calculate_layout( im.planes( ), rsize[0], margin, page_margin );
    if ( layouts.size( ) == 0 ) {
	fprintf( stderr, "error, failed to find a layout for the channel map...\n" );
	exit(1);
    }

    const size_t cols = layouts.front( );
    size_t rows = 1 + (im.planes( )-1) / cols;
    if ( rows > 6 ) rows = 6;			// its just convienient to limit the
						// number of rows for development...
    const int plane_offset = 55;		// go to the first channel with something interesting...

    const size_t out_size[2] = { cols * rsize[0] + (cols-1) * margin + page_margin * 2,
				 rows * rsize[1] + (rows-1) * margin + page_margin * 2 };

    unsigned char* out_buffer = new unsigned char[out_size[0] * out_size[1] * 3];
    agg::rendering_buffer                 out_rbuf( out_buffer,
						    out_size[0],
						    out_size[1],
						    out_size[0] * 3);

    agg::pixfmt_rgb24                     out_pf(out_rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> out_rbase(out_pf);

    out_rbase.clear(agg::rgba(1.0, 1.0, 1.0));

    agg::scanline_u8 sl;
    agg::span_allocator<agg::rgba8> sa;
    agg::rasterizer_scanline_aa<> ras;
    agg::trans_affine img_mtx;
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);
    typedef agg::span_image_filter_rgb_bilinear_clip<agg::pixfmt_rgb24,interpolator_type> span_gen_type;
    span_gen_type sg(srcpf, agg::rgba_pre(0, 0.4, 0, 0.5), interpolator);

    int plane = plane_offset;
    size_t beginning = plane;
    size_t ending = beginning;
    bool finished = false;
    for ( size_t r = 0; ! finished && r < rows; ++r ) {
	for ( size_t c = 0; c < cols; ++c ) {
	    // don't run past the end of the cube...
	    if ( plane >= im.planes( ) ) {
		finished = true;
		break;
	    }

	    // draw rectangle which will be "filled" with the image...
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    const size_t blc[2] = { page_margin + c * (rsize[0]+margin), 
				    page_margin + r * (rsize[1]+margin) };
	    const size_t trc[2] = { page_margin + c * (rsize[0]+margin) + rsize[0], 
				    page_margin + r * (rsize[1]+margin) + rsize[1] };
	    ras.reset( );
	    ras.move_to_d(blc[0],blc[1]);
	    ras.line_to_d(blc[0],trc[1]);
	    ras.line_to_d(trc[0],trc[1]);
	    ras.line_to_d(trc[0],blc[1]);
	    ras.line_to_d(blc[0],blc[1]);

	    // set up transformation matrix from rendered
	    // rectangle (i.e. parallelogram) back to image...
	    // parallelogram (bottom left, bottom right, top right) to rectangle:
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    double parallelogram[6] = { blc[0], trc[1], trc[0], trc[1], trc[0], blc[1] };
	    img_mtx.parl_to_rect(parallelogram, 0, 0, shape[0], shape[1]);
	    // same as... for our purposes...
	    // rectangle to parallelogram (inverted):
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    //double parallelogram[6] = { page_margin, trc[1], trc[0], trc[1], trc[0], page_margin }; 
	    //agg::trans_affine img_mtx(0, 0, shape[0], shape[1], parallelogram);
	    //img_mtx.invert();

	    // load image buffer...
	    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
	    ending = plane;
	    im.select(plane++);
	    source_image.clear(agg::rgba(1.0, 1.0, 1.0));
	    lay_pixels( source_image, compose(colormap)(im) );
	    // draw a single channel (plane) from the image cube...
	    agg::render_scanlines_aa(ras,sl,out_rbase,sa,sg);
	}
    }

    char filename[512];
    sprintf( filename, "g35.03_II_nh3_11.hline.%lu-%lu.ppm", beginning, ending );
    write_ppm(out_buffer, out_size[0], out_size[1], filename);

    delete [] buffer;
    
}

