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
  $Id: tprj1.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* tproj1 tests spherical projections for closure.
*
*---------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <wcstrig.h>
#include <prj.h>


int main()

{
  void projex();
  int  status;
  const double tol = 1.0e-9;
  struct prjprm prj;


  printf(
    "Testing closure of WCSLIB spherical projection routines (tprj1.c)\n"
    "-----------------------------------------------------------------\n");

  /* List status return messages. */
  printf("\nList of prj status return values:\n");
  for (status = 1; status <= 4; status++) {
    printf("%4d: %s.\n", status, prj_errmsg[status]);
  }

  printf("\n");


  prjini(&prj);

  /* AZP: zenithal/azimuthal perspective. */
  prj.pv[1] =   0.5;
  prj.pv[2] =  30.0;
  projex("AZP", &prj, 90, 5, tol);

  /* SZP: slant zenithal perspective. */
  prj.pv[1] =   0.5;
  prj.pv[2] = 210.0;
  prj.pv[3] =  60.0;
  projex("SZP", &prj, 90, -90, tol);

  /* TAN: gnomonic. */
  projex("TAN", &prj, 90, 5, tol);

  /* STG: stereographic. */
  projex("STG", &prj, 90, -85, tol);

  /* SIN: orthographic/synthesis. */
  prj.pv[1] = -0.3;
  prj.pv[2] =  0.5;
  projex("SIN", &prj, 90, 45, tol);

  /* ARC: zenithal/azimuthal equidistant. */
  projex("ARC", &prj, 90, -90, tol);

  /* ZPN: zenithal/azimuthal polynomial. */
  prj.pv[0] =  0.00000;
  prj.pv[1] =  0.95000;
  prj.pv[2] = -0.02500;
  prj.pv[3] = -0.15833;
  prj.pv[4] =  0.00208;
  prj.pv[5] =  0.00792;
  prj.pv[6] = -0.00007;
  prj.pv[7] = -0.00019;
  prj.pv[8] =  0.00000;
  prj.pv[9] =  0.00000;
  projex("ZPN", &prj, 90, 10, tol);

  /* ZEA: zenithal/azimuthal equal area. */
  projex("ZEA", &prj, 90, -85, tol);

  /* AIR: Airy's zenithal projection. */
  prj.pv[1] = 45.0;
  projex("AIR", &prj, 90, -85, tol);

  /* CYP: cylindrical perspective. */
  prj.pv[1] = 3.0;
  prj.pv[2] = 0.8;
  projex("CYP", &prj, 90, -90, tol);

  /* CEA: cylindrical equal area. */
  prj.pv[1] = 0.75;
  projex("CEA", &prj, 90, -90, tol);

  /* CAR: plate carree. */
  projex("CAR", &prj, 90, -90, tol);

  /* MER: Mercator's. */
  projex("MER", &prj, 85, -85, tol);

  /* SFL: Sanson-Flamsteed. */
  projex("SFL", &prj, 90, -90, tol);

  /* PAR: parabolic. */
  projex("PAR", &prj, 90, -90, tol);

  /* MOL: Mollweide's projection. */
  projex("MOL", &prj, 90, -90, tol);

  /* AIT: Hammer-Aitoff. */
  projex("AIT", &prj, 90, -90, tol);

  /* COP: conic perspective. */
  prj.pv[1] =  60.0;
  prj.pv[2] =  15.0;
  projex("COP", &prj, 90, -25, tol);

  /* COE: conic equal area. */
  prj.pv[1] =  60.0;
  prj.pv[2] = -15.0;
  projex("COE", &prj, 90, -90, tol);

  /* COD: conic equidistant. */
  prj.pv[1] = -60.0;
  prj.pv[2] =  15.0;
  projex("COD", &prj, 90, -90, tol);

  /* COO: conic orthomorphic. */
  prj.pv[1] = -60.0;
  prj.pv[2] = -15.0;
  projex("COO", &prj, 85, -90, tol);

  /* BON: Bonne's projection. */
  prj.pv[1] = 30.0;
  projex("BON", &prj, 90, -90, tol);

  /* PCO: polyconic. */
  projex("PCO", &prj, 90, -90, tol);

  /* TSC: tangential spherical cube. */
  projex("TSC", &prj, 90, -90, tol);

  /* CSC: COBE quadrilateralized spherical cube. */
  projex("CSC", &prj, 90, -90, 4.0e-2);

  /* QSC: quadrilateralized spherical cube. */
  projex("QSC", &prj, 90, -90, tol);

  /* HPX: HEALPix projection. */
  prj.pv[1] = 4.0;
  prj.pv[2] = 3.0;
  projex("HPX", &prj, 90, -90, tol);

  return 0;
}


/*----------------------------------------------------------------------------
*   PROJEX exercises the spherical projection routines.
*
*   Given:
*      pcode[4]  char     Projection code.
*      north     int      Northern cutoff latitude, degrees.
*      south     int      Southern cutoff latitude, degrees.
*      tol       double   Reporting tolerance, degrees.
*
*   Given and returned:
*      prj       prjprm*  Projection parameters.
*---------------------------------------------------------------------------*/

void projex(pcode, prj, north, south, tol)

char   pcode[4];
int    north, south;
double tol;
struct prjprm *prj;

{
  int    lat, lng, stat1[361], stat2[361], status;
  register int j;
  double dlat, dlatmx, dlng, dlngmx, dr, drmax;
  double lat1, lat2[361], lng1[361], lng2[361];
  double r, theta;
  double x[361], x1[361], x2[361], y[361], y1[361], y2[361];


  strcpy(prj->code, pcode);

  /* Uncomment the next line to test alternative initializations of */
  /* projection parameters.                                         */
  /* prj->r0 = R2D; */

  printf("Testing %s; latitudes%3d to%4d, reporting tolerance%8.1E deg.\n",
    prj->code, north, south, tol);

  dlngmx = 0.0;
  dlatmx = 0.0;

  prjset(prj);
  for (lat = north; lat >= south; lat--) {
    lat1 = (double)lat;

    for (j = 0, lng = -180; lng <= 180; lng++, j++) {
      lng1[j] = (double)lng;
    }

    if (prj->prjs2x(prj, 361, 1, 1, 1, lng1, &lat1, x, y, stat1) == 1) {
      printf("   %3s(S2X) ERROR 1: %s\n", pcode, prj_errmsg[1]);
      continue;
    }

    if (prj->prjx2s(prj, 361, 0, 1, 1, x, y, lng2, lat2, stat2) == 1) {
      printf("   %3s(X2S) ERROR 1: %s\n", pcode, prj_errmsg[1]);
      continue;
    }

    for (j = 0; j < 361; j++) {
      if (stat1[j]) continue;

      if (stat2[j]) {
        printf("   %3s(X2S): lng1 =%20.15f  lat1 =%20.15f\n",
          pcode, lng1[j], lat1);
        printf("                x =%20.15f     y =%20.15f  ERROR%3d\n",
          x[j], y[j], stat2[j]);
        continue;
      }

      dlng = fabs(lng2[j] - lng1[j]);
      if (dlng > 180.0) dlng = fabs(dlng-360.0);
      if (abs(lat) != 90 && dlng > dlngmx) dlngmx = dlng;
      dlat = fabs(lat2[j] - lat1);
      if (dlat > dlatmx) dlatmx = dlat;

      if (dlat > tol) {
        printf("        %3s: lng1 =%20.15f  lat1 =%20.15f\n",
          pcode, lng1[j], lat1);
        printf("                x =%20.15f     y =%20.15f\n",
          x[j], y[j]);
        printf("             lng2 =%20.15f  lat2 =%20.15f\n",
          lng2[j], lat2[j]);
      } else if (abs(lat) != 90) {
        if (dlng > tol) {
          printf("        %3s: lng1 =%20.15f  lat1 =%20.15f\n",
            pcode, lng1[j], lat1);
          printf("                x =%20.15f     y =%20.15f\n",
            x[j], y[j]);
          printf("             lng2 =%20.15f  lat2 =%20.15f\n",
            lng2[j], lat2[j]);
        }
      }
    }
  }

  printf("             Maximum residual (sky): lng%10.3E   lat%10.3E\n",
    dlngmx, dlatmx);


  /* Test closure at a point close to the reference point. */
  r = 1.0;
  theta = -180.0;

  drmax = 0.0;

  for (j = 1; j <= 12; j++) {
    x1[0] = r*cosd(theta);
    y1[0] = r*sind(theta);

    if ((status = prj->prjx2s(prj, 1, 1, 1, 1, x1, y1, lng1, &lat1, stat2))) {
      printf("   %3s(X2S):   x1 =%20.15f    y1 =%20.15f  ERROR%3d\n",
        pcode, x1[0], y1[0], status);

    } else if ((status = prj->prjs2x(prj, 1, 1, 1, 1, lng1, &lat1, x2, y2,
                                     stat1))) {
      printf("   %3s(S2X):   x1 =%20.15f    y1 =%20.15f\n",
        pcode, x1[0], y1[0]);
      printf("              lng =%20.15f   lat =%20.15f  ERROR%3d\n",
        lng1[0], lat1, status);

    } else {
      dr = sqrt((x2[0]-x1[0])*(x2[0]-x1[0]) + (y2[0]-y1[0])*(y2[0]-y1[0]));
      if (dr > drmax) drmax = dr;
      if (dr > tol) {
        printf("        %3s:   x1 =%20.15f    y1 =%20.15f\n",
          pcode, x1[0], y1[0]);
        printf("              lng =%20.15f   lat =%20.15f\n",
          lng1[0], lat1);
        printf("               x2 =%20.15f    y2 =%20.15f\n",
          x2[0], y2[0]);
      }
    }

    r /= 10.0;
    theta += 15.0;
  }

  printf("             Maximum residual (ref):  dR%10.3E\n", drmax);

  prjini(prj);

  return;
}
