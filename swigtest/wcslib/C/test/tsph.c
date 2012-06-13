/*============================================================================

  WCSLIB 4.7 - an implementation of the FITS WCS standard.
  Copyright (C) 1995-2011, Mark Calabretta

  This file is part of WCSLIB.

  WCSLIB is free software: you can redistribute it and/or modify it under the
  terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option)
  any later version.

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
  $Id: tsph.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* tsph tests the spherical coordinate transformation routines for closure.
*
*---------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <sph.h>
#include <wcstrig.h>


int main()

{
  int   j, lat, lng;
  double coslat, dlat, dlatmx, dlng, dlngmx, lng1[361], lng2[361], eul[5],
         lat1, lat2[361], phi[361], theta[361], zeta;
  const double tol = 1.0e-12;


  printf(
  "Testing closure of WCSLIB coordinate transformation routines (tsph.c)\n"
  "---------------------------------------------------------------------\n");


  /* Set reference angles. */
  eul[0] =  90.0;
  eul[1] =  30.0;
  eul[2] = -90.0;
  printf("\n%s\n%s%10.4f%10.4f%10.4f\n",
    "Celestial longitude and latitude of the native pole, and native",
    "longitude of the celestial pole (degrees):", eul[0], eul[1], eul[2]);

  eul[3] = cosd(eul[1]);
  eul[4] = sind(eul[1]);

  printf ("Reporting tolerance:%8.1E degrees of arc.\n", tol);

  dlngmx = 0.0;
  dlatmx = 0.0;

  for (lat = 90; lat >= -90; lat--) {
    lat1 = (double)lat;
    coslat = cosd(lat1);

    for (j = 0, lng = -180; lng <= 180; lng++, j++) {
      lng1[j] = (double)lng;
    }

    sphs2x(eul, 361, 1, 1, 1, lng1, &lat1, phi, theta);
    sphx2s(eul, 361, 0, 1, 1, phi, theta, lng2, lat2);

    for (j = 0; j <= 360; j++) {
      dlng = fabs(lng2[j] - lng1[j]);
      if (dlng > 180.0) dlng = fabs(dlng-360.0);
      dlng *= coslat;
      dlat = fabs(lat2[j]-lat1);

      if (dlng > dlngmx) dlngmx = dlng;
      if (dlat > dlatmx) dlatmx = dlat;

      if (dlng > tol || dlat > tol) {
        printf("Unclosed: lng1 =%20.15f  lat1 =%20.15f\n", lng1[j], lat1);
        printf("           phi =%20.15f theta =%20.15f\n", phi[j], theta[j]);
        printf("          lng2 =%20.15f  lat2 =%20.15f\n", lng2[j], lat2[j]);
       }
    }
  }


  /* Test closure at points close to the pole. */
  for (j = -1; j <= 1; j += 2) {
    zeta = 1.0;
    lng1[0] = -180.0;

    for (lat = 0; lat < 12; lat++) {
      lat1 = (double)j*(90.0 - zeta);

      sphs2x(eul, 1, 1, 1, 1, lng1, &lat1, phi, theta);
      sphx2s(eul, 1, 1, 1, 1, phi, theta, lng2, lat2);

      dlng = fabs(lng2[0] - lng1[0]);
      if (dlng > 180.0) dlng = fabs(dlng-360.0);
      dlng *= coslat;
      dlat = fabs(lat2[0]-lat1);

      if (dlng > dlngmx) dlngmx = dlng;
      if (dlat > dlatmx) dlatmx = dlat;


      if (dlng > tol || dlat > tol) {
        printf("Unclosed: lng1 =%20.15f  lat1 =%20.15f\n", lng1[0], lat1);
        printf("           phi =%20.15f theta =%20.15f\n", phi[0], theta[0]);
        printf("          lng2 =%20.15f  lat2 =%20.15f\n", lng2[0], lat2[0]);
      }

      zeta /= 10.0;
      lng1[0] += 30.0;
    }
  }

  printf("\nMaximum residual: lng%10.3E   lat%10.3E\n", dlngmx, dlatmx);

  return 0;
}
