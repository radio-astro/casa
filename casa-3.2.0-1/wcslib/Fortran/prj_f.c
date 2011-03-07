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
    $Id: prj_f.c,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*===========================================================================*/

#include <string.h>
#include <prj.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define prjini_ F77_FUNC(prjini, PRJINI)
#define prjput_ F77_FUNC(prjput, PRJPUT)
#define prjget_ F77_FUNC(prjget, PRJGET)
#define prjprt_ F77_FUNC(prjprt, PRJPRT)

#define PRJ_FLAG      100
#define PRJ_CODE      101
#define PRJ_R0        102
#define PRJ_PV        103
#define PRJ_PHI0      104
#define PRJ_THETA0    105
#define PRJ_BOUNDS    106

#define PRJ_NAME      200
#define PRJ_CATEGORY  201
#define PRJ_PVRANGE   202
#define PRJ_SIMPLEZEN 203
#define PRJ_EQUIAREAL 204
#define PRJ_CONFORMAL 205
#define PRJ_GLOBAL    206
#define PRJ_DIVERGENT 207
#define PRJ_X0        208
#define PRJ_Y0        209
#define PRJ_W         210
#define PRJ_N         211

/*--------------------------------------------------------------------------*/

int prjini_(int *prj)

{
   return prjini((struct prjprm *)prj);
}

/*--------------------------------------------------------------------------*/

int prjput_(int *prj, const int *what, const void *value, const int *m)

{
   const char *cvalp;
   const int  *ivalp;
   const double *dvalp;
   struct prjprm *prjp;

   /* Cast pointers. */
   prjp  = (struct prjprm *)prj;
   cvalp = (const char *)value;
   ivalp = (const int *)value;
   dvalp = (const double *)value;

   prjp->flag = 0;

   switch (*what) {
   case PRJ_FLAG:
      prjp->flag = *ivalp;
      break;
   case PRJ_CODE:
      strncpy(prjp->code, cvalp, 3);
      prjp->code[3] = '\0';
      break;
   case PRJ_R0:
      prjp->r0 = *dvalp;
      break;
   case PRJ_PV:
      prjp->pv[*m] = *dvalp;
      break;
   case PRJ_PHI0:
      prjp->phi0 = *dvalp;
      break;
   case PRJ_THETA0:
      prjp->theta0 = *dvalp;
      break;
   case PRJ_BOUNDS:
      prjp->bounds = *ivalp;
      break;
   default:
      return 1;
   }

   return 0;
}

/*--------------------------------------------------------------------------*/

int prjget_(const int *prj, const int *what, void *value)

{
   int m;
   char *cvalp;
   int  *ivalp;
   double *dvalp;
   const struct prjprm *prjp;

   /* Cast pointers. */
   prjp  = (const struct prjprm *)prj;
   cvalp = (char *)value;
   ivalp = (int *)value;
   dvalp = (double *)value;

   switch (*what) {
   case PRJ_FLAG:
      *ivalp = prjp->flag;
      break;
   case PRJ_CODE:
      strncpy(cvalp, prjp->code, 3);
      break;
   case PRJ_R0:
      *dvalp = prjp->r0;
      break;
   case PRJ_PV:
      for (m = 0; m < PVN; m++) {
         *(dvalp++) = prjp->pv[m];
      }
      break;
   case PRJ_PHI0:
      *dvalp = prjp->phi0;
      break;
   case PRJ_THETA0:
      *dvalp = prjp->theta0;
      break;
   case PRJ_BOUNDS:
      *ivalp = prjp->bounds;
      break;
   case PRJ_NAME:
      strncpy(cvalp, prjp->name, 40);
      break;
   case PRJ_CATEGORY:
      *ivalp = prjp->category;
      break;
   case PRJ_PVRANGE:
      *ivalp = prjp->pvrange;
      break;
   case PRJ_SIMPLEZEN:
      *ivalp = prjp->simplezen;
      break;
   case PRJ_EQUIAREAL:
      *ivalp = prjp->equiareal;
      break;
   case PRJ_CONFORMAL:
      *ivalp = prjp->conformal;
      break;
   case PRJ_GLOBAL:
      *ivalp = prjp->global;
      break;
   case PRJ_DIVERGENT:
      *ivalp = prjp->divergent;
      break;
   case PRJ_X0:
      *dvalp = prjp->x0;
      break;
   case PRJ_Y0:
      *dvalp = prjp->y0;
      break;
   case PRJ_W:
      for (m = 0; m < 10; m++) {
         *(dvalp++) = prjp->w[m];
      }
      break;
   case PRJ_N:
      *ivalp = prjp->n;
      break;
   default:
      return 1;
   }

   return 0;
}

/*--------------------------------------------------------------------------*/

int prjprt_(int *prj)

{
   return prjprt((struct prjprm *)prj);
}

/*--------------------------------------------------------------------------*/

#define PRJSET_FWRAP(pcode, PCODE) \
   int F77_FUNC(pcode##set, PCODE##SET)(int *prj) \
   {return prjset((struct prjprm *)prj);}


#define PRJS2X_FWRAP(pcode, PCODE) \
   int F77_FUNC(pcode##s2x, PCODE##S2X)( \
      int *prj, \
      const int *nphi, \
      const int *ntheta, \
      const int *spt, \
      const int *sxy, \
      const double phi[], \
      const double theta[], \
      double x[], \
      double y[], \
      int stat[]) \
   {return prj##s2x((struct prjprm *)prj, *nphi, *ntheta, *spt, *sxy, \
                     phi, theta, x, y, stat);}

#define PRJX2S_FWRAP(pcode, PCODE) \
   int F77_FUNC(pcode##x2s, PRJ##X2S)( \
      int *prj, \
      const int *nx, \
      const int *ny, \
      const int *sxy, \
      const int *spt, \
      const double x[], \
      const double y[], \
      double phi[], \
      double theta[], \
      int stat[]) \
   {return pcode##x2s((struct prjprm *)prj, *nx, *ny, *sxy, *spt, x, y, \
                     phi, theta, stat);}

#define PRJ_FWRAP(pcode, PCODE) \
   PRJSET_FWRAP(pcode, PCODE)   \
   PRJS2X_FWRAP(pcode, PCODE)   \
   PRJX2S_FWRAP(pcode, PCODE)

PRJ_FWRAP(prj, PRJ)
PRJ_FWRAP(azp, AZP)
PRJ_FWRAP(szp, SZP)
PRJ_FWRAP(tan, TAN)
PRJ_FWRAP(stg, STG)
PRJ_FWRAP(sin, SIN)
PRJ_FWRAP(arc, ARC)
PRJ_FWRAP(zpn, ZPN)
PRJ_FWRAP(zea, ZEA)
PRJ_FWRAP(air, AIR)
PRJ_FWRAP(cyp, CYP)
PRJ_FWRAP(cea, CEA)
PRJ_FWRAP(car, CAR)
PRJ_FWRAP(mer, MER)
PRJ_FWRAP(sfl, SFL)
PRJ_FWRAP(par, PAR)
PRJ_FWRAP(mol, MOL)
PRJ_FWRAP(ait, AIT)
PRJ_FWRAP(cop, COP)
PRJ_FWRAP(coe, COE)
PRJ_FWRAP(cod, COD)
PRJ_FWRAP(coo, COO)
PRJ_FWRAP(bon, BON)
PRJ_FWRAP(pco, PCO)
PRJ_FWRAP(tsc, TSC)
PRJ_FWRAP(csc, CSC)
PRJ_FWRAP(qsc, QSC)
PRJ_FWRAP(hpx, HPX)
