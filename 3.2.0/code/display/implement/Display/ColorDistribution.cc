//# ColorDistribution.cc: utilities for generating optimally-sized color cubes
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001
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

#include <casa/iostream.h>
#include <display/Display/ColorDistribution.h>

namespace casa { //# NAMESPACE CASA - BEGIN

static colorDistItem Pow2RGBTable[13] =
{
  { 1, 1, 1, 1},
  { 2, 2, 1, 1},
  { 4, 2, 2, 1},
  { 8, 2, 2, 2},
  { 16, 4, 2, 2},
  { 32, 4, 4, 2},
  { 64, 4, 4, 4},
  { 128, 8, 4, 4},
  { 256, 8, 8, 4},
  { 512, 8, 8, 8},
  { 1024, 16, 8, 8},
  { 2048, 16, 16, 8},
  { 4096, 16, 16, 16}
};

static colorDistItem RGBTable[126] =
{
 { 1, 1, 1, 1 },
 { 2, 2, 1, 1 },
 { 3, 3, 1, 1 },
 { 4, 2, 2, 1 },
 { 6, 3, 2, 1 },
 { 8, 2, 2, 2 },
 { 9, 3, 3, 1 },
 { 12, 3, 2, 2 },
 { 16, 4, 2, 2 },
 { 18, 3, 3, 2 },
 { 24, 4, 3, 2 },
 { 27, 3, 3, 3 },
 { 30, 5, 3, 2 },
 { 32, 4, 4, 2 },
 { 36, 4, 3, 3 },
 { 40, 5, 4, 2 },
 { 45, 5, 3, 3 },
 { 48, 4, 4, 3 },
 { 50, 5, 5, 2 },
 { 60, 5, 4, 3 },
 { 64, 4, 4, 4 },
 { 72, 6, 4, 3 },
 { 75, 5, 5, 3 },
 { 80, 5, 4, 4 },
 { 90, 6, 5, 3 },
 { 96, 6, 4, 4 },
 { 100, 5, 5, 4 },
 { 108, 6, 6, 3 },
 { 120, 6, 5, 4 },
 { 125, 5, 5, 5 },
 { 140, 7, 5, 4 },
 { 144, 6, 6, 4 },
 { 150, 6, 5, 5 },
 { 168, 7, 6, 4 },
 { 175, 7, 5, 5 },
 { 180, 6, 6, 5 },
 { 196, 7, 7, 4 },
 { 210, 7, 6, 5 },
 { 216, 6, 6, 6 },
 { 240, 8, 6, 5 },
 { 245, 7, 7, 5 },
 { 252, 7, 6, 6 },
 { 256, 8, 8, 4 },
 { 280, 8, 7, 5 },
 { 288, 8, 6, 6 },
 { 294, 7, 7, 6 },
 { 320, 8, 8, 5 },
 { 336, 8, 7, 6 },
 { 343, 7, 7, 7 },
 { 378, 9, 7, 6 },
 { 384, 8, 8, 6 },
 { 392, 8, 7, 7 },
 { 432, 9, 8, 6 },
 { 441, 9, 7, 7 },
 { 448, 8, 8, 7 },
 { 486, 9, 9, 6 },
 { 504, 9, 8, 7 },
 { 512, 8, 8, 8 },
 { 560, 10, 8, 7 },
 { 567, 9, 9, 7 },
 { 576, 9, 8, 8 },
 { 630, 10, 9, 7 },
 { 640, 10, 8, 8 },
 { 648, 9, 9, 8 },
 { 700, 10, 10, 7 },
 { 720, 10, 9, 8 },
 { 729, 9, 9, 9 },
 { 792, 11, 9, 8 },
 { 800, 10, 10, 8 },
 { 810, 10, 9, 9 },
 { 880, 11, 10, 8 },
 { 891, 11, 9, 9 },
 { 900, 10, 10, 9 },
 { 968, 11, 11, 8 },
 { 990, 11, 10, 9 },
 { 1000, 10, 10, 10 },
 { 1024, 16, 8, 8},
 { 1080, 12, 10, 9 },
 { 1089, 11, 11, 9 },
 { 1100, 11, 10, 10 },
 { 1188, 12, 11, 9 },
 { 1200, 12, 10, 10 },
 { 1210, 11, 11, 10 },
 { 1296, 12, 12, 9 },
 { 1320, 12, 11, 10 },
 { 1331, 11, 11, 11 },
 { 1430, 13, 11, 10 },
 { 1440, 12, 12, 10 },
 { 1452, 12, 11, 11 },
 { 1560, 13, 12, 10 },
 { 1573, 13, 11, 11 },
 { 1584, 12, 12, 11 },
 { 1690, 13, 13, 10 },
 { 1716, 13, 12, 11 },
 { 1728, 12, 12, 12 },
 { 1848, 14, 12, 11 },
 { 1859, 13, 13, 11 },
 { 1872, 13, 12, 12 },
 { 2002, 14, 13, 11 },
 { 2016, 14, 12, 12 },
 { 2028, 13, 13, 12 },
 { 2156, 14, 14, 11 },
 { 2184, 14, 13, 12 },
 { 2197, 13, 13, 13 },
 { 2340, 15, 13, 12 },
 { 2352, 14, 14, 12 },
 { 2366, 14, 13, 13 },
 { 2520, 15, 14, 12 },
 { 2535, 15, 13, 13 },
 { 2548, 14, 14, 13 },
 { 2700, 15, 15, 12 },
 { 2730, 15, 14, 13 },
 { 2744, 14, 14, 14 },
 { 2912, 16, 14, 13 },
 { 2925, 15, 15, 13 },
 { 2940, 15, 14, 14 },
 { 3120, 16, 15, 13 },
 { 3136, 16, 14, 14 },
 { 3150, 15, 15, 14 },
 { 3328, 16, 16, 13 },
 { 3360, 16, 15, 14 },
 { 3375, 15, 15, 15 },
 { 3584, 16, 16, 14 },
 { 3600, 16, 15, 15 },
 { 3840, 16, 16, 15 },
 { 4096, 16, 16, 16 }
};

Bool getHSVDistribution(uInt nCells, Bool pow2,
			uInt & nHue, uInt & nSat, uInt & nVal)
{
  // map <H,S,V> to <R,B,G>
  // maybe do something more intelligent later
  return getRGBDistribution(nCells, pow2, nHue, nVal, nSat);
}

Bool getRGBDistribution(uInt nCells, Bool pow2,
			uInt & nRed, uInt & nGreen, uInt & nBlue)
{
  // Do a binary search on the table and fill in nRed, nGreen, nBlue
  if (nCells == 0) return False;

  // Goal is to find cell such that t[i].nCells >= nCells > t[i-1]
  uInt a = 0;
  uInt b = pow2 ? 12 : 125;
  colorDistItem * t = pow2 ? Pow2RGBTable : RGBTable;

  if (nCells > 4096)
    {
      a = b;
    }
  else
    {
      while (a < b-1)
	{
	  uInt m = (a+b)/2;
	  
	  if (t[m].nCells <= nCells)
	    a = m;
	  else
	    b = m;
	}
      
    }
  
  nRed = t[a].nRed;
  nGreen = t[a].nGreen;
  nBlue = t[a].nBlue;

  return True;
}




} //# NAMESPACE CASA - END

