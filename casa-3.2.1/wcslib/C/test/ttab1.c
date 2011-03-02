/*============================================================================

    WCSLIB 4.3 - an implementation of the FITS WCS standard.
    Copyright (C) 1995-2007, Mark Calabretta

    This file is part of WCSLIB.

    WCSLIB is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

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
    $Id: ttab1.c,v 4.3 2007/12/27 05:35:51 cal103 Exp $
*=============================================================================
*
*   ttab1 tests the -TAB routines for closure.
*
*---------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>

#include <tab.h>

/* Reporting tolerance. */
const double tol = 1.0e-8;

#define K1 32
#define K2 16

int main()

{
   const int M = 2;
   const int K[] = {K1, K2};
   const int map[] = {0, 1};
   const double crval[] = {1.0, -1.0};

   int i, j, k, k1, k2, m, n, stat0[128], stat1[128], status;
   double crpix4, epsilon, resid, residmax, time[12], world[11][11][2],
          xt0[12], xt1[12], x0[11][11][2], x1[11][11][2], z;
   struct tabprm tab;

   printf(
      "Testing closure of WCSLIB tabular coordinate routines (ttab1.c)\n"
      "---------------------------------------------------------------\n");

   /* List status return messages. */
   printf("\nList of tab status return values:\n");
   for (status = 1; status <= 5; status++) {
      printf("%4d: %s.\n", status, tab_errmsg[status]);
   }

   printf("\nReporting tolerance %5.1G.\n", tol);


   /* First a 1-dimensional table from Sect. 6.2.3 of Paper III. */
   printf("\nOne-dimensional test:\n");
   tab.flag = -1;
   if ((status = tabini(1, M, K, &tab))) {
      printf("tabini ERROR %d: %s.\n", status, tab_errmsg[status]);
      return 1;
   }

   tab.M = 1;
   tab.K[0] = 8;
   tab.map[0] = 0;
   tab.crval[0] = 0.0;

   tab.index[0][0] = 0.0;
   tab.index[0][1] = 1.0;
   tab.index[0][2] = 1.0;
   tab.index[0][3] = 2.0;
   tab.index[0][4] = 2.0;
   tab.index[0][5] = 3.0;
   tab.index[0][6] = 3.0;
   tab.index[0][7] = 4.0;

   tab.coord[0] = 1997.84512;
   tab.coord[1] = 1997.84631;
   tab.coord[2] = 1993.28451;
   tab.coord[3] = 1993.28456;
   tab.coord[4] = 2001.59234;
   tab.coord[5] = 2001.59239;
   tab.coord[6] = 2002.18265;
   tab.coord[7] = 2002.18301;

   epsilon = 1e-3;
   crpix4  = 0.5;
   xt0[0]  = 0.5 + epsilon - crpix4;
   xt0[1]  = 1.0           - crpix4;
   xt0[2]  = 1.5 - epsilon - crpix4;
   xt0[3]  = 1.5 + epsilon - crpix4;
   xt0[4]  = 2.0           - crpix4;
   xt0[5]  = 2.5 - epsilon - crpix4;
   xt0[6]  = 2.5 + epsilon - crpix4;
   xt0[7]  = 3.0           - crpix4;
   xt0[8]  = 3.5 - epsilon - crpix4;
   xt0[9]  = 3.5 + epsilon - crpix4;
   xt0[10] = 4.0           - crpix4;
   xt0[11] = 4.5 - epsilon - crpix4;

   if ((status = tabx2s(&tab, 12, 1, (double *)xt0, (double *)time, stat0))) {
      printf("tabx2s ERROR %d: %s.\n", status, tab_errmsg[status]);
   }

   if ((status = tabs2x(&tab, 12, 1, (double *)time, (double *)xt1, stat1))) {
      printf("tabx2s ERROR %d: %s.\n", status, tab_errmsg[status]);
   }

   printf("    x   ->   time   ->   x\n");
   for (i = 0; i < 12; i++) {
      printf("%8.5f%12.5f%9.5f\n", xt0[i], time[i], xt1[i]);
   }
   printf("\n");

   /* Test closure. */
   residmax = 0.0;
   for (i = 0; i < 12; i++) {
      if (stat0[i]) {
         printf("   tabx2s: x = %6.1f, stat = %d\n", xt0[i], stat0[i]);
         continue;
      }

      if (stat1[i]) {
         printf("   tabs2x: t = %6.1f, stat = %d\n", time[i], stat1[i]);
         continue;
      }

      resid = fabs(xt1[i] - xt0[i]);
      if (resid > residmax) residmax = resid;

      if (resid > tol) {
         printf("   Closure error:\n");
         printf("      x = %20.15f\n", xt0[i]);
         printf("   -> t = %20.15f\n", time[i]);
         printf("   -> x = %20.15f\n", xt1[i]);
      }
   }

   tabfree(&tab);


   /* Now a 2-dimensional table. */
   printf("Two-dimensional test:\n");
   tab.flag = -1;
   if ((status = tabini(1, M, K, &tab))) {
      printf("tabini ERROR %d: %s.\n", status, tab_errmsg[status]);
      return 1;
   }

   tab.M = M;
   for (m = 0; m < tab.M; m++) {
      tab.K[m] = K[m];
      tab.map[m] = map[m];
      tab.crval[m] = crval[m];

      for (k = 0; k < tab.K[m]; k++) {
         tab.index[m][k] = (double)k;
      }
   }

   n = 0;
   z = 1.0 / ((K1-1) * (K2-1));
   for (k2 = 0; k2 < K2; k2++) {
      for (k1 = 0; k1 < K1; k1++) {
         tab.coord[n++] =  3.0*k1*k2*z;
         tab.coord[n++] = -1.0*(K1-k1-1)*k2*z + 0.01*k1;
      }
   }

   for (i = 0; i < 11; i++) {
      for (j = 0; j < 11; j++) {
         x0[i][j][0] = j*(K1-1)/10.0 - crval[0];
         x0[i][j][1] = i*(K2-1)/10.0 - crval[1];
      }
   }

   if ((status = tabx2s(&tab, 121, 2, (double *)x0, (double *)world,
                        stat0))) {
      printf("tabx2s ERROR %d: %s.\n", status, tab_errmsg[status]);
   }

   if ((status = tabs2x(&tab, 121, 2, (double *)world, (double *)x1,
                        stat1))) {
      printf("tabx2s ERROR %d: %s.\n", status, tab_errmsg[status]);
   }

   /* Test closure. */
   n = 0;
   residmax = 0.0;
   for (i = 0; i < 11; i++) {
      for (j = 0; j < 11; j++, n++) {
         if (stat0[n]) {
            printf("   tabx2s: x = (%6.1f,%6.1f), stat = %d\n", x0[i][j][0],
               x0[i][j][1], stat0[n]);
            continue;
         }

         if (stat1[n]) {
            printf("   tabs2x: s = (%6.1f,%6.1f), stat = %d\n",
               world[i][j][0], world[i][j][1], stat1[n]);
            continue;
         }

         for (m = 0; m < M; m++) {
            resid = fabs(x1[i][j][m] - x0[i][j][m]);
            if (resid > residmax) residmax = resid;

            if (resid > tol) {
               printf("   Closure error:\n");
               printf("      x = (%20.15f,%20.15f)\n", x0[i][j][0],
                 x0[i][j][1]);
               printf("   -> w = (%20.15f,%20.15f)\n", world[i][j][0],
                 world[i][j][1]);
               printf("   -> x = (%20.15f,%20.15f)\n", x1[i][j][0],
                 x1[i][j][1]);

               break;
            }
         }
      }
   }

   printf("Maximum closure residual = %.12E\n", residmax);

   tabfree(&tab);

   return 0;
}
