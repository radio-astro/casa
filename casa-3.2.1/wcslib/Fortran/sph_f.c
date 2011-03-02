/*============================================================================

    WCSLIB 4.3 - an implementation of the FITS WCS standard.
    Copyright (C) 1995-2007, Mark Calabretta

    This file is part of WCSLIB.

    WCSLIB is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    WCSLIB is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
    more details.

    You should have received a copy of the GNU Lesser General Public License
    along with WCSLIB.  If not, see <http://www.gnu.org/licenses/>.

    Correspondence concerning WCSLIB may be directed to:
       Internet email: mcalabre@atnf.csiro.au
       Postal address: Dr. Mark Calabretta
                       Australia Telescope National Facility, CSIRO
                       PO Box 76
                       Epping NSW 1710
                       AUSTRALIA

    Author: Mark Calabretta, Australia Telescope National Facility
    http://www.atnf.csiro.au/~mcalabre/index.html
    $Id: sph_f.c,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*===========================================================================*/

#include <sph.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define sphx2s_ F77_FUNC(sphx2s, SPHX2S)
#define sphs2x_ F77_FUNC(sphs2x, SPHS2X)

/*--------------------------------------------------------------------------*/

int sphx2s_(
   const double eul[5],
   const int *nphi,
   const int *ntheta,
   const int *spt,
   const int *sll,
   const double phi[],
   const double theta[],
   double lng[],
   double lat[])

{
   return sphx2s(eul, *nphi, *ntheta, *spt, *sll, phi, theta, lng, lat);
}

/*--------------------------------------------------------------------------*/

int sphs2x_(
   const double eul[5],
   const int *nlng,
   const int *nlat,
   const int *sll,
   const int *spt,
   const double lng[],
   const double lat[],
   double phi[],
   double theta[])

{
   return sphs2x(eul, *nlng, *nlat, *sll, *spt, lng, lat, phi, theta);
}
