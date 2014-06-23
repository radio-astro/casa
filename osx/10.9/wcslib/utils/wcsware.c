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
  $Id: wcsware.c,v 4.7 2011/02/07 07:03:43 cal103 Exp $
*=============================================================================
* wcsware extracts the WCS keywords for an image from the specified FITS file,
* constructs wcsprm structs for each coordinate representation found and
* performs a variety of operations using them.
*---------------------------------------------------------------------------*/

char usage[] =
"Usage: wcsware [-a<alt>] [-f] [-h<hdu>] [-p] [-x] [<fitsfile>]\n"
"\n"
"wcsware extracts the WCS keywords for an image from the specified FITS\n"
"file, constructs wcsprm structs for each coordinate representation found\n"
"and performs a variety of operations using them.\n\n"
"The FITS file may be specified according to the syntax understood by\n"
"cfitsio, for example \"file.fits.gz+1\" refers to the first extension of\n"
"a gzip'd FITS file.  Use \"-\" or omit the file name for input from stdin.\n"
"\n"
"Options:\n"
"  -a<alt>      Specify an alternate coordinate representation to be used\n"
"               (ignored if there is only one).\n"
"  -f           Apply wcsfix() to the header.\n"
"  -h<hdu>      Move to HDU number (1-relative) which is expected to\n"
"               contain an image array.  (Useful for input from stdin.)\n"
"  -p           Print the struct(s) using wcsprt() (default operation).\n"
"  -x           Convert pixel coordinates, obtained from stdin, to world\n"
"               coordinates using wcsp2s().\n"
"  -w           Convert world coordinates, obtained from stdin, to pixel\n"
"               coordinates using wcss2p().\n";

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fitsio.h>

#include <wcshdr.h>
#include <wcsfix.h>
#include <wcs.h>
#include <getwcstab.h>

int main(int argc, char **argv)

{
  char alt = ' ', *header, idents[3][80], *infile;
  int  alts[27], c, dofix = 0, doprt = 0, dopix = 0, doworld = 0, hdunum = 1,
       hdutype, i, j, nelem, nkeyrec, nreject, nwcs, *stat = 0x0, status;
  double *imgcrd = 0x0, phi, *pixcrd = 0x0, theta, *world = 0x0;
  struct wcsprm *wcs;
  fitsfile *fptr;


  /* Parse options. */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (!argv[i][1]) break;

    switch (argv[i][1]) {
    case 'a':
      alt = toupper(argv[i][2]);
      break;

    case 'f':
      dofix = 1;
      break;

    case 'h':
      hdunum = atoi(argv[i]+2);
      break;

    case 'p':
      doprt = 1;
      break;

    case 'x':
      dopix = 1;
      break;

    case 'w':
      doworld = 1;
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
    printf("wcsware: Cannot access %s.\n", infile);
    return 1;
  }

  if (!dopix && !doworld) doprt = 1;


  /* Open the FITS file and move to the required HDU. */
  status = 0;
  if (fits_open_file(&fptr, infile, READONLY, &status)) goto fitserr;
  if (fits_movabs_hdu(fptr, hdunum, &hdutype, &status)) goto fitserr;
  if (hdutype != IMAGE_HDU) {
    fprintf(stderr, "ERROR, HDU number %d does not contain an image array.\n",
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

  if (wcs == 0x0) {
    fprintf(stderr, "No world coordinate systems found.\n");
    return 1;
  }

  /* Read -TAB arrays from the binary table extension (if necessary). */
  if (fits_read_wcstab(fptr, wcs->nwtb, (wtbarr *)wcs->wtb, &status)) {
    goto fitserr;
  }

  fits_close_file(fptr, &status);


  /* Translate non-standard WCS keyvalues? */
  if (dofix) {
    stat = malloc(NWCSFIX * sizeof(int));
    if ((status = wcsfix(7, 0, wcs, stat))) {
      for (i = 0; i < NWCSFIX; i++) {
        if (stat[i] > 0) {
           fprintf(stderr, "wcsfix ERROR %d: %s.\n", status,
                   wcsfix_errmsg[stat[i]]);
        }
      }

      return 1;
    }
  }

  /* Sort out alternates. */
  if (alt) {
    wcsidx(nwcs, &wcs, alts);

    if (alt == ' ') {
      if (alts[0] == -1) {
        fprintf(stderr, "WARNING, no primary coordinate representation.\n");
        alt = '\0';
      }

    } else if (alt < 'A' || alt > 'Z') {
      fprintf(stderr, "WARNING, alternate specifier \"%c\" is invalid.\n",
        alt);
      alt = '\0';

    } else {
      if (alts[alt - 'A' + 1] == -1) {
        fprintf(stderr, "WARNING, no alternate coordinate representation "
                        "\"%c\".\n", alt);
        alt = '\0';
      }
    }
  }


  /* Initialize and possibly print the structs. */
  for (i = 0; i < nwcs; i++) {
    if (alt && (wcs+i)->alt[0] != alt) {
      continue;
    } else if (i) {
      printf("\nType <CR> for next: ");
      fgetc(stdin);
    }

    if ((status = wcsset(wcs+i))) {
      fprintf(stderr, "wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
      continue;
    }

    /* Get WCSNAME out of the wcsprm struct. */
    strcpy(idents[2], (wcs+i)->wcsname);
    if (strlen(idents[2])) {
      printf("\n%s\n", idents[2]);
    }

    /* Print the struct. */
    if (doprt) {
      wcsprt(wcs+i);
    }

    /* Transform coordinates? */
    if (dopix || doworld) {
      nelem = (wcs+i)->naxis;
      world  = realloc(world,  nelem * sizeof(double));
      imgcrd = realloc(imgcrd, nelem * sizeof(double));
      pixcrd = realloc(pixcrd, nelem * sizeof(double));
      stat   = realloc(stat,   nelem * sizeof(int));

      if (dopix) {
        /* Transform pixel coordinates. */
        while (1) {
          printf("\nEnter %d pixel coordinate element%s: ", nelem,
            (nelem==1)?"":"s");
          c = fgetc(stdin);
          if (c == EOF || c == '\n') {
            if (c == EOF) printf("\n");
            break;
          }
          ungetc(c, stdin);

          scanf("%lf", pixcrd);
          for (j = 1; j < nelem; j++) {
            scanf("%*[ ,]%lf", pixcrd+j);
          }
          while (fgetc(stdin) != '\n');

          printf("Pixel: ");
          for (j = 0; j < nelem; j++) {
            printf("%s%14.9g", j?", ":"", pixcrd[j]);
          }

          if ((status = wcsp2s(wcs+i, 1, nelem, pixcrd, imgcrd, &phi, &theta,
                               world, stat))) {
            fprintf(stderr, "wcsp2s ERROR %d: %s.\n", status,
              wcs_errmsg[status]);

          } else {
            printf("\nImage: ");
            for (j = 0; j < nelem; j++) {
              if (j == (wcs+i)->lng || j == (wcs+i)->lat) {
                /* Print angles in fixed format. */
                printf("%s%14.6f", j?", ":"", imgcrd[j]);
              } else {
                printf("%s%14.9g", j?", ":"", imgcrd[j]);
              }
            }

            printf("\nWorld: ");
            for (j = 0; j < nelem; j++) {
              if (j == (wcs+i)->lng || j == (wcs+i)->lat) {
                /* Print angles in fixed format. */
                printf("%s%14.6f", j?", ":"", world[j]);
              } else {
                printf("%s%14.9g", j?", ":"", world[j]);
              }
            }
            printf("\n");
          }
        }
      }


      if (doworld) {
        /* Transform world coordinates. */
        while (1) {
          printf("\nEnter %d world coordinate element%s: ", nelem,
            (nelem==1)?"":"s");
          c = fgetc(stdin);
          if (c == EOF || c == '\n') {
            if (c == EOF) printf("\n");
            break;
          }
          ungetc(c, stdin);

          scanf("%lf", world);
          for (j = 1; j < nelem; j++) {
            scanf("%*[ ,]%lf", world+j);
          }
          while (fgetc(stdin) != '\n');

          printf("World: ");
          for (j = 0; j < nelem; j++) {
            if (j == (wcs+i)->lng || j == (wcs+i)->lat) {
              /* Print angles in fixed format. */
              printf("%s%14.6f", j?", ":"", world[j]);
            } else {
              printf("%s%14.9g", j?", ":"", world[j]);
            }
          }

          if ((status = wcss2p(wcs+i, 1, nelem, world, &phi, &theta, imgcrd,
                               pixcrd, stat))) {
            fprintf(stderr, "wcss2p ERROR %d: %s.\n", status,
              wcs_errmsg[status]);

          } else {
            printf("\nImage: ");
            for (j = 0; j < nelem; j++) {
              if (j == (wcs+i)->lng || j == (wcs+i)->lat) {
                /* Print angles in fixed format. */
                printf("%s%14.6f", j?", ":"", imgcrd[j]);
              } else {
                printf("%s%14.9g", j?", ":"", imgcrd[j]);
              }
            }

            printf("\nPixel: ");
            for (j = 0; j < nelem; j++) {
              printf("%s%14.9g", j?", ":"", pixcrd[j]);
            }
            printf("\n");
          }
        }
      }
    }
  }

  status = wcsvfree(&nwcs, &wcs);

  return 0;

fitserr:
  fits_report_error(stderr, status);
  fits_close_file(fptr, &status);
  return 1;
}
