/*============================================================================

    WCSLIB 4.3 - an implementation of the FITS WCS standard.
    Copyright (C) 1995-2007, Mark Calabretta

    This file is part of WCSLIB.

    WCSLIB is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your option)
    any later version.

    WCSLIB is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with WCSLIB; if not, write to the Free Software Foundation, Inc.,
    59 Temple Place, Suite 330, Boston, MA  02111-1307, USA

    Correspondence concerning WCSLIB may be directed to:
       Internet email: mcalabre@atnf.csiro.au
       Postal address: Dr. Mark Calabretta
                       Australia Telescope National Facility, CSIRO
                       PO Box 76
                       Epping NSW 1710
                       AUSTRALIA

    Author: Mark Calabretta, Australia Telescope National Facility
    http://www.atnf.csiro.au/~mcalabre/index.html
    $Id: wcsgrid.c,v 4.3 2007/12/27 05:50:31 cal103 Exp $
*=============================================================================
*
*   wcsgrid extracts the WCS keywords for an image from the specified FITS
*   file and uses pgsbox() to plot a 2-D coordinate graticule for it.  Refer
*   to the usage notes below.
*
* TODO
*   Implement alt.
*   Subimaging option.
*---------------------------------------------------------------------------*/

char usage[] =
"Usage: wcsgrid [-a<alt>] [-h<hdu>] [<fitsfile>]\n"
"\n"
"wcsgrid extracts the WCS keywords for an image from the specified FITS\n"
"file and uses pgsbox() to plot a 2-D coordinate graticule for it.\n"
"\n"
"Options:\n"
"  -a<alt>      Specify an alternate coordinate representation to be used\n"
"               (ignored if there is only one).\n"
"  -h<hdu>      Skip to HDU number (1-relative) which is expected to\n"
"               contain an image array.\n";

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fitsio.h>
#include <cpgplot.h>
#include <cpgsbox.h>

#include <wcshdr.h>
#include <wcsfix.h>
#include <wcs.h>
#include <getwcstab.h>

int main(int argc, char **argv)

{
  char alt = ' ', devtyp[16], *header, idents[3][80], *infile, nlcprm[1],
       opt[2];
  int  c0[] = {-1, -1, -1, -1, -1, -1, -1};
  int  gcode[2], hdunum = 1, hdutype, i, ic, naxes, naxis[2], nkeyrec,
       nreject, nwcs, stat[NWCSFIX], status;
  float  blc[2], trc[2];
  double cache[257][4], grid1[1], grid2[1], nldprm[1];
  struct wcsprm *wcs;
  nlfunc_t pgwcsl_;
  fitsfile *fptr;


  /* Parse options. */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (!argv[i][1]) break;

    switch (argv[i][1]) {
    case 'a':
      alt = toupper(argv[i][2]);
      break;
    case 'h':
      hdunum = atoi(argv[i]+2);
      break;
    default:
      fprintf(stderr, "%s", usage);
      return 1;
    }
  }

  if (i < argc) {
    infile = argv[i++];

    if (i < argc) {
      fprintf(stderr, "%s", usage);
      return 1;
    }
  } else {
    infile = "-";
  }

  /* Check accessibility of the input file. */
  if (strcmp(infile, "-") && access(infile, R_OK) == -1) {
    printf("wcsgrid: Cannot access %s.\n", infile);
    return 1;
  }


  /* Open the FITS file and move to the required HDU. */
  status = 0;
  if (fits_open_file(&fptr, infile, READONLY, &status)) goto fitserr;
  if (fits_movabs_hdu(fptr, hdunum, &hdutype, &status)) goto fitserr;
  if (hdutype != IMAGE_HDU) {
    fprintf(stderr, "ERROR, HDU number %d does not contain an image array.\n",
      hdunum);
    return 1;
  }

  /* Check that we have at least two image axes. */
  if (fits_read_key(fptr, TINT, "NAXIS",  &naxes, NULL, &status)) {
    goto fitserr;
  }

  if (naxes < 2) {
    fprintf(stderr, "ERROR, HDU number %d does not contain a 2-D image.\n",
      hdunum);
    return 1;
  } else if (naxes > 2) {
    printf("HDU number %d contains an %d-D image array.\n", hdunum, naxes);
  }

  /* Get NAXIS, NAXIS1 & NAXIS2 from the header. */
  fits_read_key(fptr, TINT, "NAXIS1", naxis,   NULL, &status);
  fits_read_key(fptr, TINT, "NAXIS2", naxis+1, NULL, &status);

  if ((naxis[0] < 2) || (naxis[1] < 2)) {
    fprintf(stderr, "ERROR, HDU number %d contains degenerate image axes.\n",
      hdunum);
    return 1;
  }

  /* Read in the FITS header, excluding COMMENT and HISTORY keyrecords. */
  if (fits_hdr2str(fptr, 1, NULL, 0, &header, &nkeyrec, &status)) {
    goto fitserr;
  }


  /* Interpret the WCS keywords. */
  if ((status = wcspih(header, nkeyrec, WCSHDR_all, -3, &nreject, &nwcs,
                       &wcs))) {
    fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
    return 1;
  }
  free(header);

  /* Read -TAB arrays from the binary table extension (if necessary). */
  if (fits_read_wcstab(fptr, wcs->nwtb, (wtbarr *)wcs->wtb, &status)) {
    goto fitserr;
  }

  fits_close_file(fptr, &status);

  /* Translate non-standard WCS keyvalues. */
  if ((status = wcsfix(7, 0, wcs, stat))) {
    for (i = 0; i < NWCSFIX; i++) {
      if (stat[i] > 0) {
         fprintf(stderr, "wcsfix ERROR %d: %s.\n", status,
                 wcsfix_errmsg[stat[i]]);
      }
    }

     return 1;
  }


  /* Plot setup. */
  blc[0] = 0.5f;
  blc[1] = 0.5f;
  trc[0] = naxis[0] + 0.5f;
  trc[1] = naxis[1] + 0.5f;

  strcpy(devtyp, "/XWINDOW");
  cpgbeg(0, devtyp, 1, 1);
  cpgvstd();

  cpgwnad(0.0f, 1.0f, 0.0f, 1.0f);
  cpgask(1);
  cpgpage();

  /* Annotation. */
  strcpy(idents[0], "Right ascension");
  strcpy(idents[1], "Declination");

  opt[0] = 'G';
  opt[1] = 'E';

  /* Compact lettering. */
  cpgsch(0.8f);

  /* Draw full grid lines. */
  cpgsci(1);
  gcode[0] = 2;
  gcode[1] = 2;
  grid1[0] = 0.0;
  grid2[0] = 0.0;

  for (i = 0; i < nwcs; i++) {
    if ((status = wcsset(wcs+i))) {
      fprintf(stderr, "wcsset ERROR %d: %s.\n", status,
              wcs_errmsg[status]);
      continue;
    }

    /* Get WCSNAME out of the wcsprm struct. */
    strcpy(idents[2], (wcs+i)->wcsname);
    printf("\n%s\n", idents[2]);

    /* Draw the celestial grid.  The grid density is set for each world */
    /* coordinate by specifying LABDEN = 1224. */
    ic = -1;
    cpgsbox(blc, trc, idents, opt, 0, 1224, c0, gcode, 0.0, 0, grid1, 0,
      grid2, 0, pgwcsl_, 1, WCSLEN, 1, nlcprm, (int *)(wcs+i), nldprm, 256,
      &ic, cache, &status);

    /* Draw the frame. */
    cpgbox("BC", 0.0f, 0, "BC", 0.0f, 0);

    cpgpage();
  }

  status = wcsvfree(&nwcs, &wcs);

  return 0;

fitserr:
  fits_report_error(stderr, status);
  fits_close_file(fptr, &status);
  return 1;
}
