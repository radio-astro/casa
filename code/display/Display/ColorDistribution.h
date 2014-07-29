//# ColorDistribution.h: global functions for generating color cube dimensions
//# Copyright (C) 1994,1995,1996,1997,1998,1999
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

#ifndef TRIALDISPLAY_COLORDISTRIBUTION_H
#define TRIALDISPLAY_COLORDISTRIBUTION_H

#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Global functions for generating color cube dimensions
// </summary>

// This table is used to map X number of color cells
// into an even cube for RGB distribution.  It is unclear
// in my mind what a good HSV distribution ought to be.
// Maybe read table as H,V,S.  Probably build different
// one for HSV.

// <group name="Color Distribution interface">

	typedef struct {
		unsigned int nCells;
		unsigned int nRed, nGreen, nBlue;
	} colorDistItem;


// Given nCells cells, return the dimensions of an RGB or HSV color cube
// that maximally uses those cells by finding the best fit dimensions.
// If pow2 is True, then returned dimensions are limited to powers of two.
// <group>
	Bool getRGBDistribution(uInt nCells, Bool pow2,
	                        uInt & nRed, uInt & nGreen, uInt & nBlue);
	Bool getHSVDistribution(uInt nCells, Bool pow2,
	                        uInt & nHue, uInt & nSat, uInt & nVal);
// </group>
// </group>


} //# NAMESPACE CASA - END

#endif
