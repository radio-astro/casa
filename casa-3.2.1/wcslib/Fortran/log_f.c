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
    $Id: log_f.c,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*===========================================================================*/

#include <log.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define logx2s_ F77_FUNC(logx2s, LOGX2S)
#define logs2x_ F77_FUNC(logs2x, LOGS2X)

/*--------------------------------------------------------------------------*/

int logx2s_(
   const double *crval,
   const int *nx,
   const int *sx,
   const int *slogc,
   const double x[],
   double logc[],
   int stat[])

{
   return logx2s(*crval, *nx, *sx, *slogc, x, logc, stat);
}

/*--------------------------------------------------------------------------*/

int logs2x_(
   const double *crval,
   const int *nlogc,
   const int *slogc,
   const int *sx,
   const double logc[],
   double x[],
   int stat[])

{
   return logs2x(*crval, *nlogc, *slogc, *sx, logc, x, stat);
}
