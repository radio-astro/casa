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
  $Id: tlog.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
*   tlog tests the logarithmic coordinate transformation routines for closure.
*
*---------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>

#include <log.h>


#define NCRD 10000

const double tol = 1.0e-13;


int main()

{
  register int j, k;
  int    stat1[NCRD], stat2[NCRD], status;
  double logc[NCRD], resid, residmax, step, x0[NCRD], x1[NCRD];
  double crval = 3.3;


  printf(
    "Testing closure of WCSLIB logarithmic coordinate routines (tlog.c)\n"
    "------------------------------------------------------------------\n");

  /* List status return messages. */
  printf("\nList of log status return values:\n");
  for (status = 2; status <= 3; status++) {
    printf("%4d: %s.\n", status, log_errmsg[status]);
  }


  /* Construct a logarithmic axis and test closure. */
  step = (40.0/NCRD) / 2.0;
  for (j = 0, k = -NCRD; j < NCRD; j++, k += 2) {
    x0[j] = k*step;
  }
  printf("\nLogarithmic range: %.1f to %.1f, step: %.4f\n", x0[0], x0[NCRD-1],
    x0[1] - x0[0]);

  /* Convert the first to the second. */
  if ((status = logx2s(crval, NCRD, 1, 1, x0, logc, stat1))) {
    printf("logx2s ERROR %d: %s.\n", status, log_errmsg[status]);
  }

  /* Convert the second back to the first. */
  if ((status = logs2x(crval, NCRD, 1, 1, logc, x1, stat2))) {
    printf("logs2x ERROR %d: %s.\n", status, log_errmsg[status]);
  }

  residmax = 0.0;

  /* Test closure. */
  for (j = 0; j < NCRD; j++) {
    if (stat1[j]) {
      printf("logx2s: x =%20.12E -> log = ???, stat = %d\n", x0[j], stat1[j]);
      continue;
    }

    if (stat2[j]) {
      printf("logs2x: x =%20.12E -> log =%20.12E -> x = ???, stat = %d\n",
             x0[j], logc[j], stat2[j]);
      continue;
    }

    if (x0[j] == 0.0) {
      resid = fabs(x1[j] - x0[j]);
    } else {
      resid = fabs((x1[j] - x0[j]) / x0[j]);
      if (resid > residmax) residmax = resid;
    }

    if (resid > tol) {
      printf("logx2s: x =%20.12E -> log =%20.12E ->\n        x =%20.12E, "
        "resid =%20.12E\n", x0[j], logc[j], x1[j], resid);
    }
  }

  if (residmax > tol) printf("\n");
  printf("logx2s: Maximum residual =%19.12E\n", residmax);

  return 0;
}
