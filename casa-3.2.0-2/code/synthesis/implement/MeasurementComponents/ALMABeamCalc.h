//# ALMABeamCalc.h: Definition for BeamCalc
//# Copyright (C) 2010
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_ALMABEAMCALC_H
#define SYNTHESIS_ALMABEAMCALC_H

#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>

namespace casa
{
  enum ALMABeamCalcBandCode
    {
      BeamCalc_ALMA_1 = 0,
      BeamCalc_ALMA_2,
      BeamCalc_ALMA_3,
      BeamCalc_ALMA_4,
      BeamCalc_ALMA_5,
      BeamCalc_ALMA_6,
      BeamCalc_ALMA_7,
      BeamCalc_ALMA_8,
      BeamCalc_ALMA_9,
      BeamCalc_ALMA_10,
      
      ALMABeamCalc_NumBandCodes	/* this line last */
    };
    
  struct ALMAApertureCalcParams
  {
    Int oversamp;			/* average this many points per cell */
    TempImage<Complex> *aperture;	/* Jones planes [Nx,Ny,NStokes,NFreq]*/
    Double x0, y0;			/* center of cell 0, 0, meters */
    Double dx, dy;			/* increment in meters */
    Int nx, ny;			/* calculation plane size in cells */
    /* last cell is at coordinates:
       X = x0 + (nx-1)*dx
       Y = y0 + (ny-1)*dy
    */
    Double pa;			/* Parallactic angle, radians */
    Double freq;			/* GHz */
    enum ALMABeamCalcBandCode band;
  };
  
  extern struct BeamCalcGeometry ALMABeamCalcGeometryDefaults[ALMABeamCalc_NumBandCodes];
  
  Int ALMACalculateAperture(struct ALMAApertureCalcParams *ap);
};

#endif
