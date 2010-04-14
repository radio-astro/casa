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
    $Id: wcsutil.h,v 4.3 2007/12/27 05:41:36 cal103 Exp $
*=============================================================================
*
*   Simple utility functions used by WCSLIB, undocumented and not intended for
*   external use - the interfaces may change without notice.
*
*===========================================================================*/

#ifndef WCSLIB_WCSUTIL
#define WCSLIB_WCSUTIL

void wcsutil_blank_fill(int, char []);
void wcsutil_null_fill(int, char []);

int  wcsutil_allEq(int, int, const double *);
void wcsutil_setAll(int, int, double *);
void wcsutil_setAli(int, int, int *);
void wcsutil_setBit(int, int*, int, int *);

#endif /* WCSLIB_WCSUTIL */
