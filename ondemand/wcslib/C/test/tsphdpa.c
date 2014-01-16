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
  $Id: tsphdpa.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* tsphdpa tests sphdpa().
*
*---------------------------------------------------------------------------*/
#include <stdio.h>

#include "sph.h"

int main()

{
  double dist, lat, lat0, lng, lng0, pa;

  printf("\nEnter reference (lng,lat): ");
  scanf("%lf%*[ ,	]%lf", &lng0, &lat0);

  while (1) {
    printf("\nEnter   field   (lng,lat): ");
    scanf("%lf%*[ ,	]%lf", &lng, &lat);

    sphdpa(1, lng0, lat0, &lng, &lat, &dist, &pa);

    printf("(%.4f,%.4f) - (%.4f,%.4f) -> (%.4f,%.4f) (dist,pa)\n",
      lng0, lat0, lng, lat, dist, pa);

    sphpad(1, lng0, lat0, &dist, &pa, &lng, &lat);

    printf("(%.4f,%.4f) + (%.4f,%.4f) -> (%.4f,%.4f) (lng,lat)\n",
      lng0, lat0, dist, pa, lng, lat);
  }

  return 0;
}
