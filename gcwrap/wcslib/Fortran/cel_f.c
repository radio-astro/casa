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
  $Id: cel_f.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*===========================================================================*/

#include <cel.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define celini_ F77_FUNC(celini, CELINI)
#define celput_ F77_FUNC(celput, CELPUT)
#define celget_ F77_FUNC(celget, CELGET)
#define celprt_ F77_FUNC(celprt, CELPRT)
#define celset_ F77_FUNC(celset, CELSET)
#define celx2s_ F77_FUNC(celx2s, CELX2S)
#define cels2x_ F77_FUNC(cels2x, CELS2X)

#define celptc_ F77_FUNC(celptc, CELPTC)
#define celptd_ F77_FUNC(celptd, CELPTD)
#define celpti_ F77_FUNC(celpti, CELPTI)
#define celgtc_ F77_FUNC(celgtc, CELGTC)
#define celgtd_ F77_FUNC(celgtd, CELGTD)
#define celgti_ F77_FUNC(celgti, CELGTI)

#define CEL_FLAG   100
#define CEL_OFFSET 101
#define CEL_PHI0   102
#define CEL_THETA0 103
#define CEL_REF    104
#define CEL_PRJ    105

#define CEL_EULER  200
#define CEL_LATPRQ 201
#define CEL_ISOLAT 202

/*--------------------------------------------------------------------------*/

int celini_(int *cel)

{
  return celini((struct celprm *)cel);
}

/*--------------------------------------------------------------------------*/

int celput_(int *cel, const int *what, const void *value, const int *i)

{
  int k;
  int *icelp;
  const int    *ivalp;
  const double *dvalp;
  struct celprm *celp;

  /* Cast pointers. */
  celp  = (struct celprm *)cel;
  ivalp = (const int *)value;
  dvalp = (const double *)value;

  celp->flag = 0;

  switch (*what) {
  case CEL_FLAG:
    celp->flag = *ivalp;
    break;
  case CEL_OFFSET:
    celp->offset = *ivalp;
    break;
  case CEL_PHI0:
    celp->phi0 = *dvalp;
    break;
  case CEL_THETA0:
    celp->theta0 = *dvalp;
    break;
  case CEL_REF:
    celp->ref[*i-1] = *dvalp;
    break;
  case CEL_PRJ:
    k = (int *)(&(celp->prj)) - (int *)celp;
    icelp = cel + k;
    for (k = 0; k < PRJLEN; k++) {
      *(icelp++) = *(ivalp++);
    }
    break;
  default:
    return 1;
  }

  return 0;
}

int celptc_(int *cel, const int *what, const char *value, const int *i)
{
  return celput_(cel, what, value, i);
}

int celptd_(int *cel, const int *what, const double *value, const int *i)
{
  return celput_(cel, what, value, i);
}

int celpti_(int *cel, const int *what, const int *value, const int *i)
{
  return celput_(cel, what, value, i);
}

/*--------------------------------------------------------------------------*/

int celget_(const int *cel, const int *what, void *value)

{
  int k;
  int    *ivalp;
  double *dvalp;
  const int *icelp;
  const struct celprm *celp;

  /* Cast pointers. */
  celp  = (const struct celprm *)cel;
  ivalp = (int *)value;
  dvalp = (double *)value;

  switch (*what) {
  case CEL_FLAG:
    *ivalp = celp->flag;
    break;
  case CEL_OFFSET:
    *ivalp = celp->offset;
    break;
  case CEL_PHI0:
    *dvalp = celp->phi0;
    break;
  case CEL_THETA0:
    *dvalp = celp->theta0;
    break;
  case CEL_REF:
    for (k = 0; k < 4; k++) {
      *(dvalp++) = celp->ref[k];
    }
    break;
  case CEL_PRJ:
    k = (int *)(&(celp->prj)) - (int *)celp;
    icelp = cel + k;
    for (k = 0; k < PRJLEN; k++) {
      *(ivalp++) = *(icelp++);
    }
    break;
  case CEL_EULER:
    for (k = 0; k < 5; k++) {
       *(dvalp++) = celp->euler[k];
    }
    break;
  case CEL_LATPRQ:
    *ivalp = celp->latpreq;
    break;
  case CEL_ISOLAT:
    *ivalp = celp->isolat;
    break;
  default:
    return 1;
  }

  return 0;
}

int celgtc_(const int *cel, const int *what, char *value)
{
  return celget_(cel, what, value);
}

int celgtd_(const int *cel, const int *what, double *value)
{
  return celget_(cel, what, value);
}

int celgti_(const int *cel, const int *what, int *value)
{
  return celget_(cel, what, value);
}

/*--------------------------------------------------------------------------*/

int celprt_(int *cel)

{
  return celprt((struct celprm *)cel);
}

/*--------------------------------------------------------------------------*/

int celset_(int *cel)

{
  return celset((struct celprm *)cel);
}

/*--------------------------------------------------------------------------*/

int celx2s_(
  int *cel,
  const int *nx,
  const int *ny,
  const int *sxy,
  const int *sll,
  const double x[],
  const double y[],
  double phi[],
  double theta[],
  double lng[],
  double lat[],
  int stat[])

{
  return celx2s((struct celprm *)cel, *nx, *ny, *sxy, *sll, x, y, phi, theta,
                lng, lat, stat);
}

/*--------------------------------------------------------------------------*/

int cels2x_(
  int *cel,
  const int *nlng,
  const int *nlat,
  const int *sll,
  const int *sxy,
  const double lng[],
  const double lat[],
  double phi[],
  double theta[],
  double x[],
  double y[],
  int stat[])

{
  return cels2x((struct celprm *)cel, *nlng, *nlat, *sll, *sxy, lng, lat,
                phi, theta, x, y, stat);
}
