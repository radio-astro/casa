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
  $Id: twcsfix.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* twcsfix tests the translation routines for non-standard WCS keyvalues, the
* wcsfix() suite, and the spectral coordinate translation routine wcssptr().
*
*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include <wcs.h>
#include <wcsunits.h>
#include <wcsfix.h>


void parser(struct wcsprm *);

const int NAXIS = 3;
const double CRPIX[3] =  {90.0, 90.0, 1.0};
const double PC[3][3] = {{ 1.0,  0.0, 0.0},
                         { 0.0,  1.0, 0.0},
                         { 0.0,  0.0, 1.0}};
const double CDELT[3] =  {-1.0, 1.0, 19.68717093222};

char CUNIT[3][9] = {"ARCSEC", "ARCSEC", "KM/SEC"};

/* N.B. non-standard. */
char CTYPE[3][9] = {"RA---NCP", "DEC--NCP", "FELO-HEL"};

const double CRVAL[3] = {265.6220947090*3600.0, -28.98849996030*3600.0,
                         5569.27104};
const double RESTFRQ = 1.42040575e9;
const double RESTWAV = 0.0;

/* N.B. non-standard, corresponding to MJD 35884.04861111 */
const char DATEOBS[] = "1957/02/15 01:10:00";

int main()

{
  char ctypeS[9];
  int i, stat[NWCSFIX], status;
  struct wcsprm wcs;

  printf("Testing WCSLIB translator for non-standard usage (twcsfix.c)\n"
         "------------------------------------------------------------\n\n");

  wcs.flag = -1;
  parser(&wcs);

  /* Print the unmodified struct. */
  wcsprt(&wcs);
  printf("\n------------------------------------"
         "------------------------------------\n");

  /* Fix non-standard WCS keyvalues. */
  if ((status = wcsfix(7, 0, &wcs, stat))) {
    printf("wcsfix error, status returns: (");
    for (i = 0; i < NWCSFIX; i++) {
      printf(i ? ", %d" : "%d", stat[i]);
    }
    printf(")\n");
    return 1;
  }

  wcsprt(&wcs);
  printf("\n------------------------------------"
         "------------------------------------\n");

  /* Should now have a 'VOPT-F2W' axis, translate it to frequency. */
  strcpy(ctypeS, "FREQ-???");
  i = -1;
  if ((status = wcssptr(&wcs, &i, ctypeS))) {
    printf("wcssptr ERROR %d: %s.n", status, wcs_errmsg[status]);
    return 1;
  }

  if ((status = wcsset(&wcs))) {
    printf("wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
    return 1;
  }

  wcsprt(&wcs);

  wcsfree(&wcs);

  return 0;
}

/*--------------------------------------------------------------------------*/

void parser(wcs)

struct wcsprm *wcs;

{
  int i, j, status;
  double *pcij;

  /* In practice a parser would read the FITS header until it encountered  */
  /* the NAXIS keyword which must occur near the start, before any of the  */
  /* WCS keywords.  It would then use wcsini() to allocate memory for      */
  /* arrays in the wcsprm struct and set default values.  In this          */
  /* simulation the header keyvalues are set as global variables.          */
  wcsnpv(2);
  wcsini(1, NAXIS, wcs);


  /* Now the parser scans the FITS header, identifying WCS keywords and    */
  /* loading their values into the appropriate elements of the wcsprm      */
  /* struct.                                                               */

  for (j = 0; j < NAXIS; j++) {
    wcs->crpix[j] = CRPIX[j];
  }

  pcij = wcs->pc;
  for (i = 0; i < NAXIS; i++) {
    for (j = 0; j < NAXIS; j++) {
      *(pcij++) = PC[i][j];
    }
  }

  for (i = 0; i < NAXIS; i++) {
    wcs->cdelt[i] = CDELT[i];
  }

  for (i = 0; i < NAXIS; i++) {
    strcpy(wcs->cunit[i], &CUNIT[i][0]);
  }

  for (i = 0; i < NAXIS; i++) {
    strcpy(wcs->ctype[i], &CTYPE[i][0]);
  }

  for (i = 0; i < NAXIS; i++) {
    wcs->crval[i] = CRVAL[i];
  }

  wcs->restfrq = RESTFRQ;
  wcs->restwav = RESTWAV;

  wcs->pv[0].i = -1;
  wcs->pv[0].m = -1;
  wcs->pv[0].value = -1.0;
  wcs->npv = 1;

  strcpy(wcs->dateobs, DATEOBS);


  /* Translate non-standard units specifications before wcsset(). */
  for (i = 0; i < NAXIS; i++) {
    wcsutrn(7, wcs->cunit[i]);
  }

  /* Extract information from the FITS header. */
  if ((status = wcsset(wcs))) {
    printf("wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
  }

  return;
}
