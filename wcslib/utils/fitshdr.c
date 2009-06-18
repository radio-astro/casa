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
    $Id: fitshdr.c,v 4.3 2007/12/27 05:50:31 cal103 Exp $
*=============================================================================
* Usage: fitshdr [infile]
*-----------------------------------------------------------------------------
* List headers from a FITS file specified on the command line, or else on
* stdin, printing them as 80-character keyrecords without trailing blanks.
*
* If invoked as 'rpfhdr' rather than 'fitshdr' it also handles RPFITS format
* which has a block size of 2560 (rather than 2880) but otherwise looks like
* FITS.
*
* Handles large files (>2GiB) via macros in wcsconfig_utils.h.
*---------------------------------------------------------------------------*/

/* Get LFS definitions for stdio.h. */
#include <wcsconfig_utils.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char **argv)

{
  char cbuff[2880], *cptr;
  char dashes[84] = "----------------------------------------"
                    "----------------------------------------";
  char equals[84] = "========================================"
                    "========================================";
  char spaces[84] = "                                        "
                    "                                        ";
  char *format, rpfits[8] = "RPFITS";
  int  len;
  unsigned int blksiz = 2880, inhdr = 0, nhdr = 0, seekable = 1;
  unsigned long long int iblock = 0, nblock = 0, nbyte;

  /* If an input file name was specified then reopen it as stdin */
  /* (doesn't affect seekability).                               */
  if (argc > 1) {
    if (access(argv[1], R_OK) == -1) {
      perror(argv[1]);
      return 2;
    }

    if (freopen(argv[1], "r", stdin) == NULL) {
      perror(argv[1]);
      return 2;
    }
  }

  /* Check for standard FITS or RPFITS. */
  if (!fread(cbuff, (size_t)80, (size_t)1, stdin)) {
    perror(argv[1]);
    return 2;
  }

  if (strncmp(cbuff, "SIMPLE  = ", 10) == 0) {
    if (!fread(cbuff+80, (size_t)80, (size_t)1, stdin)) {
      perror(argv[1]);
      return 2;
    }

    /* Assume FITS by default. */
    format = rpfits + 2;
    blksiz = 2880;

    /* Check for RPFITS. */
    if (strncmp(cbuff+80, "FORMAT  =               RPFITS", 30) == 0 ||
        strncmp(cbuff+80, "FORMAT  =             'RPFITS'", 30) == 0) {
      if (strcmp(*argv, "rpfhdr") == 0) {
        /* If invoked as 'rpfhdr' then allow RPFITS. */
        format = rpfits;
        blksiz = 2560;
      } else {
        /* Otherwise disallow RPFITS but issue a warning. */
        printf("WARNING: Input appears to be RPFITS, continuing anyway using "
          "2880-byte blocks.\n" );
      }
    }

    /* Read the rest of the first block. */
    if (!fread(cbuff+160, (size_t)(blksiz-160), (size_t)1, stdin)) {
      perror(argv[1]);
      return 2;
    }

    inhdr = 1;

  } else {
    /* If we have not been invoked as 'rpfhdr' then bail out now. */
    if (strcmp(*argv, "rpfhdr") != 0) {
      fprintf(stderr, "Input file does not appear to be standard FITS.\n" );
      return 1;
    }

    /* RPFITS may have a block or two of rubbish before the first header. */
    format = rpfits;
    blksiz = 2560;

    if (!fread(cbuff+80, (size_t)(blksiz-80), (size_t)1, stdin)) {
      perror(argv[1]);
      return 2;
    }

    while (iblock < 4) {
      if (!fread(cbuff, (size_t)blksiz, (size_t)1, stdin)) {
        perror(argv[1]);
        return 2;
      }

      iblock++;
      if (strncmp(cbuff, "SIMPLE  = ", 10) == 0) {
        inhdr = 1;
        break;
      }
    }

    if (!inhdr) {
      fprintf(stderr, "Input does not appear to be FITS or RPFITS.\n" );
      return 1;
    }

    if (iblock) {
      nbyte = blksiz * iblock;
      printf("Skipped %lld block%s of rubbish of size %d bytes (%lld "
        "bytes).\n", iblock, (iblock > 1)?"s":"", blksiz, nbyte);
    }
  }

  printf("%s\n%s header number %d at block number %lld.\n%s\n", equals,
    format, ++nhdr, ++iblock, dashes);


  /* Scan through the file. */
  while (1) {
    if (!inhdr) {
      /* Searching for a header. */
      if (!fread(cbuff, (size_t)10, (size_t)1, stdin)) break;

      if (strncmp(cbuff, "SIMPLE  = ", 10) == 0 ||
          strncmp(cbuff, "XTENSION= ", 10) == 0) {
        /* Found a header. */
        if (iblock) {
          nbyte = blksiz * nblock;
          printf("Skipped %lld block%s of data of size %d bytes (%lld "
            "bytes).\n", nblock, (nblock == 1)?"":"s", blksiz, nbyte);
        }

        if (!fread(cbuff+10, (size_t)(blksiz-10), (size_t)1, stdin)) break;

        printf("%s\n%s header number %d at block number %lld.\n%s\n",
          equals, format, ++nhdr, ++iblock, dashes);
        inhdr = 1;
        nblock = 0;

      } else {
        /* Seek past it if possible. */
        if (seekable) {
#ifdef HAVE_FSEEKO
          if (fseeko(stdin, (off_t)(blksiz-10), SEEK_CUR)) {
#else
          if (fseek(stdin, (long)(blksiz-10), SEEK_CUR)) {
#endif
            if (errno == ESPIPE || errno == EBADF) {
              seekable = 0;
            } else {
              break;
            }
          }
        }

        if (!seekable) {
          if (!fread(cbuff+10, (size_t)(blksiz-10), (size_t)1, stdin)) break;
        }

        iblock++;
      }
    }

    if (inhdr) {
      for (cptr = cbuff; cptr < cbuff + blksiz; cptr += 80) {
        /* Write out a keyrecord without trailing blanks. */
        for (len = 80; len > 0; len--) {
          if (cptr[len-1] != ' ') break;
        }
        printf("%.*s\n", len, cptr);

        /* Check for end-of-header. */
        if (strncmp(cptr, "END     ", 8) == 0) {
          inhdr = 0;
          printf("%s\n", dashes);
          fflush(stdout);
          break;
        }
      }

      /* Get the next header block. */
      if (inhdr) {
        if (!fread(cbuff, (size_t)blksiz, (size_t)1, stdin)) break;
        iblock++;
      }

    } else {
      nblock++;

      if (nblock%1000 == 0) {
        /* Report progress on stderr in case it's saved to file. */
        nbyte = blksiz * nblock;
        fprintf(stderr, "Skipping %lld blocks of data of size %d bytes "
          "(%lld bytes).   \r", nblock, blksiz, nbyte);
        fflush(stderr);
      }
    }
  }

  if (feof(stdin)) {
    nbyte = blksiz * nblock;
    printf("Skipped %lld block%s of data of size %d bytes (%lld bytes). \n",
      nblock, (nblock == 1)?"":"s", blksiz, nbyte);

    nbyte = blksiz * iblock;
    printf("%s\nEnd-of-file after %d HDU%s in %lld x %d-byte blocks (%lld "
      "bytes).\n%s\n", equals, nhdr, (nhdr == 1)?"":"s", iblock, blksiz,
      nbyte, dashes);

    fprintf(stderr, "%s\r", spaces);

  } else {
    perror(argv[1]);
    return 2;
  }

  return 0;
}
