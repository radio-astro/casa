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
  $Id: twcssub.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* twcssub tests wcssub() which extracts the coordinate description for a
* subimage from a wcsprm struct.
*
*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include <wcs.h>


/* In real life these would be encoded as FITS header keyrecords. */
const int NAXIS = 4;
const double CRPIX[4] =  { 1025.0,  64.0, 512.0, 513.0};
const double PC[4][4] = {{    1.1,   0.0,   0.0,   0.0},
                         {    0.0,   1.0,   0.0,   0.0},
                         {    0.0,   0.0,   1.0,   0.1},
                         {    0.0,   0.0,   0.2,   1.0}};
const double CDELT[4] =  {-9.2e-6,  10.0,   1.0,  -1.0};
char CUNIT[4][16] = {"m", "s", "deg", "deg"};
char CTYPE[4][16] = {"WAVE-F2W",  "TIME", "XLAT-SZP",  "XLON-SZP"};

const double CRVAL[4] =  {0.214982042, -2e3, -30.0, 150.0};
const double LONPOLE  = 150.0;
const double LATPOLE  = 999.0;
const double RESTFRQ  =   1.42040575e9;
const double RESTWAV  =   0.0;

char CNAME[4][16] = {"Wavelength", "Time", "Latitude", "Longitude"};

int NPS, NPV;
struct pvcard PV[10];
struct pscard PS[10];


int main()

{
  int axes[4], i, j, nsub, status;
  struct wcsprm wcs, wcsext;
  double *pcij;

  PV[0].i = 1;			/* Frequency on axis 1. */
  PV[0].m = 1;			/* Parameter number 1.  */
  PV[0].value = -1.0;		/* PV1_1.               */

  PV[1].i = 3;			/* Latitude on axis 3.  */
  PV[1].m = 1;			/* Parameter number 1.  */
  PV[1].value = 2.0;		/* PV3_1.               */

  PV[2].i = 3;			/* Latitude on axis 3.  */
  PV[2].m = 2;			/* Parameter number 2.  */
  PV[2].value = 210.0;		/* PV3_2.               */

  PV[3].i = 3;			/* Latitude on axis 3.  */
  PV[3].m = 3;			/* Parameter number 3.  */
  PV[3].value = 60.0;		/* PV3_3.               */

  NPV = 4;

  PS[0].i = 2;			/* Time on axis 2.      */
  PS[0].m = 1;			/* Parameter number 1.  */
  strcpy(PS[0].value, "UTC");	/* PS2_1.               */

  NPS = 1;

  wcs.flag = -1;
  wcsini(1, NAXIS, &wcs);

  for (j = 0; j < NAXIS; j++) {
    wcs.crpix[j] = CRPIX[j];
  }

  pcij = wcs.pc;
  for (i = 0; i < NAXIS; i++) {
    for (j = 0; j < NAXIS; j++) {
      *(pcij++) = PC[i][j];
    }
  }

  for (i = 0; i < NAXIS; i++) {
    wcs.cdelt[i] = CDELT[i];
  }

  for (i = 0; i < NAXIS; i++) {
    strcpy(wcs.cunit[i], &CUNIT[i][0]);
    strcpy(wcs.ctype[i], &CTYPE[i][0]);
    strcpy(wcs.cname[i], &CNAME[i][0]);
  }

  for (i = 0; i < NAXIS; i++) {
    wcs.crval[i] = CRVAL[i];
  }

  wcs.lonpole = LONPOLE;
  wcs.latpole = LATPOLE;

  wcs.restfrq = RESTFRQ;
  wcs.restwav = RESTWAV;

  wcs.npv = NPV;
  for (i = 0; i < NPV; i++) {
    wcs.pv[i] = PV[i];
  }

  wcs.nps = NPS;
  for (i = 0; i < NPS; i++) {
    wcs.ps[i] = PS[i];
  }


  /* Initialize the wcsprm struct. */
  (void) wcsset(&wcs);

  printf("Testing WCSLIB subimage extraction routine (twcssub.c)\n"
         "------------------------------------------------------\n");
  printf("\nInitial contents of wcsprm struct:\n");
  wcsprt(&wcs);


  /* Extract the coordinate description for a subimage. */
  nsub = 3;
  wcsext.flag = -1;
  axes[0] = WCSSUB_LONGITUDE;
  axes[1] = WCSSUB_LATITUDE;
  axes[2] = -(WCSSUB_SPECTRAL | WCSSUB_STOKES);
  printf("\n\nExtracted contents of wcsprm struct:\n");
  if ((status = wcssub(1, &wcs, &nsub, axes, &wcsext))) {
    printf("wcssub ERROR %d: %s.\n", status, wcs_errmsg[status]);
  } else if (nsub == 0) {
    printf("None of the requested subimage axes were found.\n");
  } else if ((status = wcsset(&wcsext))) {
    printf("wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
  } else {
    wcsprt(&wcsext);
  }


  /* Set it up for failure by setting PC1_3 non-zero. */
  *(wcs.pc+2) = 1.0;
  nsub = 2;
  axes[0] = 4;
  axes[1] = 3;
  if ((status = wcssub(1, &wcs, &nsub, axes, &wcsext)) == 13) {
    printf("\n\nReceived wcssub status 13 as expected for a non-separable "
           "subimage\ncoordinate system.\n");
  } else {
    printf("\n\nERROR: expected wcssub status 13 for a non-separable "
           "subimage coordinate\nsystem, but received status %d instead.\n",
           status);
  }

  wcsfree(&wcs);
  wcsfree(&wcsext);

  return 0;
}
