//# Image.h: generic 2D image handling for graphpad.
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

#if ! defined(_GRAPHPAD_IMAGE_H_)
#define _GRAPHPAD_IMAGE_H_

namespace graphpad {

    // axis dimensions
    enum PlotAxes { RA, DEC, FREQ, VELO, STOKES, NOAXIS };

    // Draw images onto a "renderer". This uses a "generator" to load data from domain-specific
    // (e.g. casa) storage. This draws a 2D image which cooresponds to user might see when a
    // single channel of an image cube is displayed in the casa viewer (without tick marks, axis
    // labels, plot labels or other decorations. The selection of which channel, slice, etc. to
    // load (in the casa context) would be achieved by configuration of the generator. The
    // generator is just a producer of a series of numbers which this function uses along with
    // a color map to lay pixels down via the "renderer". It is assumed that the generator will
    // provide enough pixels for the renderer's width & height.
    template<class Renderer, typename Generator>
    void lay_pixels( Renderer r, Generator gen ) {
	for(size_t i = 0; i < r.height( ); ++i) {
	    for(size_t j = 0; j < r.width( ); ++j) {
		r.copy_pixel(j , i, gen( ));
	    }
	}
    }

}
#endif
