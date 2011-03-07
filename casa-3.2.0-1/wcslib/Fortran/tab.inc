*=======================================================================
*
*   WCSLIB 4.3 - an implementation of the FITS WCS standard.
*   Copyright (C) 1995-2007, Mark Calabretta
*
*   This file is part of WCSLIB.
*
*   WCSLIB is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as
*   published by the Free Software Foundation, either version 3 of
*   the License, or (at your option) any later version.
*
*   WCSLIB is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public
*   License along with WCSLIB.  If not, see http://www.gnu.org/licenses.
*
*   Correspondence concerning WCSLIB may be directed to:
*      Internet email: mcalabre@atnf.csiro.au
*      Postal address: Dr. Mark Calabretta
*                      Australia Telescope National Facility, CSIRO
*                      PO Box 76
*                      Epping NSW 1710
*                      AUSTRALIA
*
*   Author: Mark Calabretta, Australia Telescope National Facility
*   http://www.atnf.csiro.au/~mcalabre/index.html
*   $Id: tab.inc,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*=======================================================================

*     Functions.
      INTEGER   TABINI, TABCPY, TABPUT, TABGET, TABFREE, TABMEM, TABPRT,
     :          TABSET, TABX2S, TABS2X

*     Length of the TABPRM data structure (INTEGER array).
      INTEGER   TABLEN
      PARAMETER (TABLEN = 22)

*     Codes for TAB data structure elements used by TABPUT and TABGET.
      INTEGER   TAB_COORD, TAB_CRVAL, TAB_FLAG, TAB_INDEX, TAB_K, TAB_M,
     :          TAB_MAP

      PARAMETER (TAB_FLAG    = 100)
      PARAMETER (TAB_M       = 101)
      PARAMETER (TAB_K       = 102)
      PARAMETER (TAB_MAP     = 103)
      PARAMETER (TAB_CRVAL   = 104)
      PARAMETER (TAB_INDEX   = 105)
      PARAMETER (TAB_COORD   = 106)

*     Codes for TAB data structure elements used by TABGET (only).
      INTEGER   TAB_DELTA, TAB_EXTREMA, TAB_NC, TAB_P0, TAB_SENSE

      PARAMETER (TAB_NC      = 200)
      PARAMETER (TAB_SENSE   = 201)
      PARAMETER (TAB_P0      = 202)
      PARAMETER (TAB_DELTA   = 203)
      PARAMETER (TAB_EXTREMA = 204)

      CHARACTER TAB_ERRMSG(0:5)*80
      DATA TAB_ERRMSG /
     :   'Success',
     :   'Null tabprm pointer passed',
     :   'Memory allocation failed',
     :   'Invalid tabular parameters',
     :   'One or more of the x coordinates were invalid',
     :   'One or more of the world coordinates were invalid'/
