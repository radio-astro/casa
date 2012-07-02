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
  $Id: tspx.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=============================================================================
*
* tspx tests the spectral transformation routines for closure.
*
*---------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>

#include <spx.h>


#define NSPEC 9991

const double tol = 1.0e-9;

void closure (const char *, const char *, double, int (*)(), int (*)(),
              const double [], double []);

const double C = 2.99792458e8;


int main()

{
  int    stat[NSPEC], status;
  double restfrq, restwav, step;
  double awav[NSPEC], freq[NSPEC], spc1[NSPEC], spc2[NSPEC], velo[NSPEC],
         wave[NSPEC];
  struct spxprm spx;

  register int j, k;


  printf(
    "Testing closure of WCSLIB spectral transformation routines (tspx.c)\n"
    "-------------------------------------------------------------------\n");

  /* List status return messages. */
  printf("\nList of spx status return values:\n");
  for (status = 1; status <= 4; status++) {
    printf("%4d: %s.\n", status, spx_errmsg[status]);
  }

  restfrq = 1420.40595e6;
  restwav = C/restfrq;


  /* Exercise specx(). */
  printf("\nTesting spectral cross-conversions (specx).\n\n");
  if ((status = specx("VELO", 4.3e5, restfrq, restwav, &spx))) {
    printf("specx ERROR %d: %s.\n", status, spx_errmsg[status]);
    return 1;
  }

  printf("    restfrq:%20.12E\n", spx.restfrq);
  printf("    restwav:%20.12E\n", spx.restwav);
  printf("   wavetype:%3d\n",     spx.wavetype);
  printf("   velotype:%3d\n",     spx.velotype);
  printf("\n");
  printf("       freq:%20.12E\n", spx.freq);
  printf("       afrq:%20.12E\n", spx.afrq);
  printf("       ener:%20.12E\n", spx.ener);
  printf("       wavn:%20.12E\n", spx.wavn);
  printf("       vrad:%20.12E\n", spx.vrad);
  printf("       wave:%20.12E\n", spx.wave);
  printf("       vopt:%20.12E\n", spx.vopt);
  printf("       zopt:%20.12E\n", spx.zopt);
  printf("       awav:%20.12E\n", spx.awav);
  printf("       velo:%20.12E\n", spx.velo);
  printf("       beta:%20.12E\n", spx.beta);
  printf("\n");

  printf("dfreq/dafrq:%20.12E\n", spx.dfreqafrq);
  printf("dafrq/dfreq:%20.12E\n", spx.dafrqfreq);

  printf("dfreq/dener:%20.12E\n", spx.dfreqener);
  printf("dener/dfreq:%20.12E\n", spx.denerfreq);

  printf("dfreq/dwavn:%20.12E\n", spx.dfreqwavn);
  printf("dwavn/dfreq:%20.12E\n", spx.dwavnfreq);

  printf("dfreq/dvrad:%20.12E\n", spx.dfreqvrad);
  printf("dvrad/dfreq:%20.12E\n", spx.dvradfreq);

  printf("dfreq/dwave:%20.12E\n", spx.dfreqwave);
  printf("dwave/dfreq:%20.12E\n", spx.dwavefreq);

  printf("dfreq/dawav:%20.12E\n", spx.dfreqawav);
  printf("dawav/dfreq:%20.12E\n", spx.dawavfreq);

  printf("dfreq/dvelo:%20.12E\n", spx.dfreqvelo);
  printf("dvelo/dfreq:%20.12E\n", spx.dvelofreq);

  printf("dwave/dvopt:%20.12E\n", spx.dwavevopt);
  printf("dvopt/dwave:%20.12E\n", spx.dvoptwave);

  printf("dwave/dzopt:%20.12E\n", spx.dwavezopt);
  printf("dzopt/dwave:%20.12E\n", spx.dzoptwave);

  printf("dwave/dawav:%20.12E\n", spx.dwaveawav);
  printf("dawav/dwave:%20.12E\n", spx.dawavwave);

  printf("dwave/dvelo:%20.12E\n", spx.dwavevelo);
  printf("dvelo/dwave:%20.12E\n", spx.dvelowave);

  printf("dawav/dvelo:%20.12E\n", spx.dawavvelo);
  printf("dvelo/dawav:%20.12E\n", spx.dveloawav);

  printf("dvelo/dbeta:%20.12E\n", spx.dvelobeta);
  printf("dbeta/dvelo:%20.12E\n", spx.dbetavelo);
  printf("\n");


  /* Construct a linear velocity spectrum. */
  step = (2.0*C/NSPEC) / 2.0;
  for (j = 0, k = -NSPEC; j < NSPEC; j++, k += 2) {
    velo[j] = (k+1)*step;
  }
  printf("\nVelocity range: %.3f to %.3f km/s, step: %.3f km/s\n",
         velo[0]*1e-3, velo[NSPEC-1]*1e-3, (velo[1] - velo[0])*1e-3);

  /* Convert it to frequency. */
  velofreq(restfrq, NSPEC, 1, 1, velo, freq, stat);

  /* Test closure of all two-way combinations. */
  closure("freq", "afrq", 0.0,     freqafrq, afrqfreq, freq, spc1);
  closure("afrq", "freq", 0.0,     afrqfreq, freqafrq, spc1, spc2);

  closure("freq", "ener", 0.0,     freqener, enerfreq, freq, spc1);
  closure("ener", "freq", 0.0,     enerfreq, freqener, spc1, spc2);

  closure("freq", "wavn", 0.0,     freqwavn, wavnfreq, freq, spc1);
  closure("wavn", "freq", 0.0,     wavnfreq, freqwavn, spc1, spc2);

  closure("freq", "vrad", restfrq, freqvrad, vradfreq, freq, spc1);
  closure("vrad", "freq", restfrq, vradfreq, freqvrad, spc1, spc2);

  closure("freq", "wave", 0.0,     freqwave, wavefreq, freq, wave);
  closure("wave", "freq", 0.0,     wavefreq, freqwave, wave, spc2);

  closure("freq", "awav", 0.0,     freqawav, awavfreq, freq, awav);
  closure("awav", "freq", 0.0,     awavfreq, freqawav, awav, spc2);

  closure("freq", "velo", restfrq, freqvelo, velofreq, freq, velo);
  closure("velo", "freq", restfrq, velofreq, freqvelo, velo, spc2);

  closure("wave", "vopt", restwav, wavevopt, voptwave, wave, spc1);
  closure("vopt", "wave", restwav, voptwave, wavevopt, spc1, spc2);

  closure("wave", "zopt", restwav, wavezopt, zoptwave, wave, spc1);
  closure("zopt", "wave", restwav, zoptwave, wavezopt, spc1, spc2);

  closure("wave", "awav", 0.0,     waveawav, awavwave, wave, spc1);
  closure("awav", "wave", 0.0,     awavwave, waveawav, spc1, spc2);

  closure("wave", "velo", restwav, wavevelo, velowave, wave, spc1);
  closure("velo", "wave", restwav, velowave, wavevelo, spc1, spc2);

  closure("awav", "velo", restwav, awavvelo, veloawav, awav, spc1);
  closure("velo", "awav", restwav, veloawav, awavvelo, spc1, spc2);

  closure("velo", "beta", 0.0,     velobeta, betavelo, velo, spc1);
  closure("beta", "velo", 0.0,     betavelo, velobeta, spc1, spc2);

  return 0;
}

/*--------------------------------------------------------------------------*/

void closure (from, to, parm, fwd, rev, spec1, spec2)

const char *from, *to;
double parm;
int (*fwd)(SPX_ARGS);
int (*rev)(SPX_ARGS);
const double spec1[];
double spec2[];

{
  static char skip = '\0';
  int stat1[NSPEC], stat2[NSPEC], status;
  register int j;
  double clos[NSPEC], resid, residmax;

  /* Convert the first to the second. */
  if ((status = fwd(parm, NSPEC, 1, 1, spec1, spec2, stat1))) {
    printf("%s%s ERROR %d: %s.\n", from, to, status, spx_errmsg[status]);
  }

  /* Convert the second back to the first. */
  if ((status = rev(parm, NSPEC, 1, 1, spec2, clos, stat2))) {
    printf("%s%s ERROR %d: %s.\n", to, from, status, spx_errmsg[status]);
  }

  residmax = 0.0;

  /* Test closure. */
  for (j = 0; j < NSPEC; j++) {
    if (stat1[j]) {
      printf("%c%s%s: %s = %.12E -> %s = ???, stat = %d\n", skip, from, to,
             from, spec1[j], to, stat1[j]);
      skip = '\0';
      continue;
    }

    if (stat2[j]) {
      printf("%c%s%s: %s = %.12E -> %s = %.12E -> %s = ???, stat = %d\n",
             skip, to, from, from, spec1[j], to, spec2[j], from, stat2[j]);
      skip = '\0';
      continue;
    }

    if (spec1[j] == 0.0) {
      resid = fabs(clos[j] - spec1[j]);
    } else {
      resid = fabs((clos[j] - spec1[j])/spec1[j]);
      if (resid > residmax) residmax = resid;
    }

    if (resid > tol) {
      printf("%c%s%s: %s = %.12E -> %s = %.12E ->\n          %s = %.12E,  "
             "resid = %.12E\n", skip, from, to, from, spec1[j], to,
             spec2[j], from, clos[j], resid);
      skip = '\0';
    }
  }

  printf("%s%s: Maximum closure residual = %.12E\n", from, to, residmax);
  if (residmax > tol) {
    printf("\n");
    skip = '\0';
  } else {
    skip = '\n';
  }

  return;
}
