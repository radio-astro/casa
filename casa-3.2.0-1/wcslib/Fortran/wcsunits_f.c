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
    $Id: wcsunits_f.c,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*===========================================================================*/

#include <string.h>

#include <wcsutil.h>
#include <wcsunits.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define wcsunits_ F77_FUNC(wcsunits, WCSUNITS)
#define wcsutrn_  F77_FUNC(wcsutrn,  WCSUTRN)
#define wcsulex_  F77_FUNC(wcsulex,  WCSULEX)

/*--------------------------------------------------------------------------*/

int wcsunits_(
   const char have[72],
   const char want[72],
   double *scale,
   double *offset,
   double *power)

{
   char have_[72], want_[72];

   strncpy(have_, have, 72);
   strncpy(want_, want, 72);
   have_[71] = '\0';
   want_[71] = '\0';

   return wcsunits(have_, want_, scale, offset, power);
}

/*--------------------------------------------------------------------------*/

int wcsutrn_(
   const int *ctrl,
   char unitstr[72])

{
   int status;
   char unitstr_[72];

   strncpy(unitstr_, unitstr, 72);
   unitstr_[71] = '\0';

   status = wcsutrn(*ctrl, unitstr_);

   wcsutil_blank_fill(72, unitstr_);
   strncpy(unitstr, unitstr_, 72);

   return status;
}

/*--------------------------------------------------------------------------*/

int wcsulex_(
   const char unitstr[72],
   int *func,
   double *scale,
   double units[WCSUNITS_NTYPE])

{
   char unitstr_[72];

   strncpy(unitstr_, unitstr, 72);
   unitstr_[71] = '\0';

   return wcsulex(unitstr_, func, scale, units);
}
