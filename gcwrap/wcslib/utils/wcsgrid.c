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
  $Id: wcsgrid.c,v 4.7 2011/02/07 07:03:43 cal103 Exp $
*=============================================================================
*
* wcsgrid extracts the WCS keywords for an image from the specified FITS file
* and uses pgsbox() to plot a 2-D coordinate graticule for each representation
* found.  Refer to the usage notes below.
*
* TODO
*   Subimaging option.
*---------------------------------------------------------------------------*/

char usage[] =
"Usage: wcsgrid [-a<alt>] [-d<pgdev>] [-h<hdu>] [<fitsfile>]\n"
"\n"
"wcsgrid extracts the WCS keywords for an image from the specified FITS\n"
"file and uses pgsbox() to plot a 2-D coordinate graticule for each\n"
"alternate representation found.\n\n"
"The FITS file may be specified according to the syntax understood by\n"
"cfitsio, for example \"file.fits.gz+1\" refers to the first extension of\n"
"a gzip'd FITS file.  Use \"-\" or omit the file name for input from stdin.\n"
"\n"
"Options:\n"
"  -a<alt>      Plot a graticule only for the alternate representation\n"
"               specified (ignored if there is only one).\n"
"  -d<pgdev>    PGPLOT device type (default XWINDOW, use \"?\" for list).\n"
"  -h<hdu>      Move to HDU number (1-relative) which is expected to\n"
"               contain an image array.  (Useful for input from stdin.)\n";

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
  char alt = '\0', *header, idents[3][80], *infile, keyword[16], nlcprm[1],
       opt[2], pgdev[16];
  int  c0[] = {-1, -1, -1, -1, -1, -1, -1};
  int  alts[27], gcode[2], hdunum = 1, hdutype, i, ic, naxes, naxis[2],
       nkeyrec, nreject, nwcs, stat[NWCSFIX], status;
  float  blc[2], trc[2];
  double cache[257][4], grid1[3], grid2[3], nldprm[1];
  struct wcsprm *wcs;
  nlfunc_t pgwcsl_;
  fitsfile *fptr;


  /* Parse options. */
  strcpy(pgdev, "/XWINDOW");
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (!argv[i][1]) break;

    switch (argv[i][1]) {
    case 'a':
      alt = toupper(argv[i][2]);
      break;

    case 'd':
      if (argv[i][2] == '?') {
        cpgldev();
        return 0;
      }

      if (argv[i][2] == '/') {
        strncpy(pgdev+1, argv[i]+3, 15);
      } else {
        strncpy(pgdev+1, argv[i]+2, 15);
      }
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
    printf("HDU number %d contains a %d-D image array.\n", hdunum, naxes);
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

  /* Translate non-standard WCS keyvalues. */
  if ((status = wcsfix(7, 0, wcs, stat))) {
    status = 0;
    for (i = 0; i < NWCSFIX; i++) {
      if (stat[i] > 0) {
         fprintf(stderr, "wcsfix ERROR %d: %s.\n", stat[i],
                 wcsfix_errmsg[stat[i]]);

        /* Ignore problems with CDi_ja and DATE-OBS. */
        if (!(i == CDFIX || i == DATFIX)) status = 1;
      }
    }

    if (status) return 1;
  }

  /* Sort out alternates. */
  if (alt) {
    wcsidx(nwcs, &wcs, alts);

    if (alt == ' ') {
      if (alts[0] == -1) {
        fprintf(stderr, "WARNING, no primary coordinate representation, "
                        "doing all.\n");
        alt = '\0';
      }

    } else if (alt < 'A' || alt > 'Z') {
      fprintf(stderr, "WARNING, alternate specifier \"%c\" is invalid, "
                      "doing all.\n", alt);
      alt = '\0';

    } else {
      if (alts[alt - 'A' + 1] == -1) {
        fprintf(stderr, "WARNING, no alternate coordinate representation "
                        "\"%c\", doing all.\n", alt);
        alt = '\0';
      }
    }
  }

  /* Get image dimensions from the header. */
  sprintf(keyword, "NAXIS%d", wcs->lng + 1);
  fits_read_key(fptr, TINT, "NAXIS1", naxis,   NULL, &status);
  sprintf(keyword, "NAXIS%d", wcs->lat + 1);
  fits_read_key(fptr, TINT, "NAXIS2", naxis+1, NULL, &status);

  if ((naxis[0] < 2) || (naxis[1] < 2)) {
    fprintf(stderr, "ERROR, HDU number %d contains degenerate image axes.\n",
      hdunum);
    return 1;
  }

  fits_close_file(fptr, &status);


  /* Plot setup. */
  blc[0] = 0.5f;
  blc[1] = 0.5f;
  trc[0] = naxis[0] + 0.5f;
  trc[1] = naxis[1] + 0.5f;

  if (cpgbeg(0, pgdev, 1, 1) != 1) {
    fprintf(stderr, "ERROR, failed to open PGPLOT device %s.\n", pgdev);
    return 1;
  }
  cpgvstd();

  cpgwnad(blc[0], trc[0], blc[0], trc[1]);
  cpgask(1);
  cpgpage();

  /* Compact lettering. */
  cpgsch(0.8f);

  /* Draw full grid lines. */
  gcode[0] = 2;
  gcode[1] = 2;
  grid1[0] =    0.0;
  grid2[0] =    0.0;

  /* These are for the projection boundary. */
  grid1[1] = -180.0;
  grid1[2] =  180.0;
  grid2[1] =  -90.0;
  grid2[2] =   90.0;

  cpgsci(1);

  for (i = 0; i < nwcs; i++) {
    if (alt && (wcs+i)->alt[0] != alt) {
      continue;
    }

    if ((status = wcsset(wcs+i))) {
      fprintf(stderr, "wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
      continue;
    }

    /* Draw the frame. */
    cpgbox("BC", 0.0f, 0, "BC", 0.0f, 0);

    /* Axis labels; use CNAMEia in preference to CTYPEia. */
    if ((wcs+i)->cname[0][0]) {
      strcpy(idents[0], (wcs+i)->cname[0]);
    } else {
      strcpy(idents[0], (wcs+i)->ctype[0]);
    }

    if ((wcs+i)->cname[1][0]) {
      strcpy(idents[1], (wcs+i)->cname[1]);
    } else {
      strcpy(idents[1], (wcs+i)->ctype[1]);
    }

    /* Title; use WCSNAME. */
    strcpy(idents[2], (wcs+i)->wcsname);
    if (strlen(idents[2])) {
      printf("\n%s\n", idents[2]);
    }

    /* Formatting control for celestial coordinates. */
    if (strncmp((wcs+i)->ctype[0], "RA", 2) == 0) {
      /* Right ascension in HMS, declination in DMS. */
      opt[0] = 'G';
      opt[1] = 'E';
    } else {
      /* Other angles in decimal degrees. */
      opt[0] = 'A';
      opt[1] = 'B';
    }

    /* Draw the celestial grid.  The grid density is set for each world */
    /* coordinate by specifying LABDEN = 1224. */
    ic = -1;
    cpgsbox(blc, trc, idents, opt, 0, 1224, c0, gcode, 0.0, 0, grid1, 0,
      grid2, 0, pgwcsl_, 1, WCSLEN, 1, nlcprm, (int *)(wcs+i), nldprm, 256,
      &ic, cache, &status);

    /* Delimit the projection boundary. */
    if ((wcs+i)->cel.prj.category != ZENITHAL) {
      /* Reset to the native coordinate graticule. */
      (wcs+i)->crval[0] = (wcs+i)->cel.prj.phi0;
      (wcs+i)->crval[1] = (wcs+i)->cel.prj.theta0;
      (wcs+i)->lonpole  = 999.0;
      (wcs+i)->latpole  = 999.0;
      status = wcsset(wcs+i);

      ic = -1;
      cpgsbox(blc, trc, idents, opt, -1, 0, c0, gcode, 0.0, 2, grid1, 2,
        grid2, 0, pgwcsl_, 1, WCSLEN, 1, nlcprm, (int *)(wcs+i), nldprm, 256,
        &ic, cache, &status);
    }

    cpgpage();
  }

  status = wcsvfree(&nwcs, &wcs);

  return 0;

fitserr:
  fits_report_error(stderr, status);
  fits_close_file(fptr, &status);
  return 1;
}
