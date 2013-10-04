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
  $Id: tofits.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* tofits turns a list of FITS header keyrecords, one per line, into a proper
* FITS header by padding them with blanks to 80 characters and stripping out
* newline characters.  It also pads the header to an integral number of 2880-
* byte blocks if necessary.
*
* It operates as a filter, e.g.:
*
*     tofits < infile > outfile
*
* Input lines beginning with '#' are treated as comments.
*
*===========================================================================*/

#include <stdio.h>

int main()

{
  int c, i = 0, nkeyrec = 0;

  while ((c = getchar()) != EOF) {
    if (c == '\n') {
      /* Blank-fill the keyrecord. */
      while (i++ < 80) {
        putchar(' ');
      }
      i = 0;
      nkeyrec++;

    } else if (c == '#' && i == 0) {
      /* Discard comments. */
      while ((c = getchar()) != EOF) {
        if (c == '\n') break;
      }

    } else {
      putchar(c);
      i++;
    }
  }

  /* Pad to a multiple of 2880-bytes. */
  if (nkeyrec %= 36) {
    while (nkeyrec++ < 36) {
      i = 0;
      while (i++ < 80) {
        putchar(' ');
      }
    }
  }

  return 0;
}
