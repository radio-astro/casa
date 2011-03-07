//# VLAIlluminationConvFunc.h: Definition for VLAIlluminationConvFunc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#ifndef SYNTHESIS_BEAMCALC_H
#define SYNTHESIS_BEAMCALC_H

//#include <casa/complex.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>

namespace casa
{
  enum BeamCalcBandCode
    {
      BeamCalc_VLA_L = 0,
      BeamCalc_VLA_C,
      BeamCalc_VLA_X,
      BeamCalc_VLA_U,
      BeamCalc_VLA_K,
      BeamCalc_VLA_Q,
      BeamCalc_VLA_4,

      BeamCalc_EVLA_L,
      BeamCalc_EVLA_S,
      BeamCalc_EVLA_C,
      BeamCalc_EVLA_X,
      BeamCalc_EVLA_U,
      BeamCalc_EVLA_K,
      BeamCalc_EVLA_A,
      BeamCalc_EVLA_Q,
      BeamCalc_EVLA_4,
      
      BeamCalc_NumBandCodes	/* this line last */
    };
  
  struct BeamCalcGeometry	/* all dimensions in meters, GHz */
  {
    char name[16];	/* name of antenna, e.g., VLA */
    Double sub_h;	/* subreflector vertex height above primary vertex */
    Double feedpos[3];	/* position of feed */
    Double subangle;	/* angle subtended by the subreflector */
    Double legwidth;	/* strut width */
    Double legfoot;		/* distance from optic axis of leg foot */
    Double legapex;		/* hight of leg intersection */
    Double Rhole;		/* radius of central hole */
    Double Rant;		/* antenna radius */
    Double reffreq;		/* a reference frequency */
    Double taperpoly[5];	/* polynomial expanded about reffreq */
    Int ntaperpoly;		/* number of terms in polynomial */
    
    /* to be added later
       Double focus;
       Double dfeedpos[3];
       Double dsub_z;
    */
  };
  
  struct ApertureCalcParams
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
    enum BeamCalcBandCode band;
  };
  
  extern struct BeamCalcGeometry BeamCalcGeometryDefaults[BeamCalc_NumBandCodes];
  
  Int calculateAperture(struct ApertureCalcParams *ap);
};

#endif
