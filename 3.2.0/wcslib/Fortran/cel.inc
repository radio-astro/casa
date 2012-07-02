*=======================================================================
*
* WCSLIB 4.7 - an implementation of the FITS WCS standard.
* Copyright (C) 1995-2011, Mark Calabretta
*
* This file is part of WCSLIB.
*
* WCSLIB is free software: you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* WCSLIB is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with WCSLIB.  If not, see http://www.gnu.org/licenses.
*
* Correspondence concerning WCSLIB may be directed to:
*   Internet email: mcalabre@atnf.csiro.au
*   Postal address: Dr. Mark Calabretta
*                   Australia Telescope National Facility, CSIRO
*                   PO Box 76
*                   Epping NSW 1710
*                   AUSTRALIA
*
* Author: Mark Calabretta, Australia Telescope National Facility
* http://www.atnf.csiro.au/~mcalabre/index.html
* $Id: cel.inc,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=======================================================================

*     Functions.
      EXTERNAL  CELGET, CELGTC, CELGTD, CELGTI, CELINI, CELPRT, CELPTC,
     :          CELPTD, CELPTI, CELPUT, CELS2X, CELSET, CELX2S
      INTEGER   CELGET, CELGTC, CELGTD, CELGTI, CELINI, CELPRT, CELPTC,
     :          CELPTD, CELPTI, CELPUT, CELS2X, CELSET, CELX2S

*     Length of the CELPRM data structure (INTEGER array) on 64-bit
*     machines.  Only needs to be 140 on 32-bit machines.
      INTEGER   CELLEN
      PARAMETER (CELLEN = 142)

*     Codes for CEL data structure elements used by CELPUT and CELGET.
      INTEGER   CEL_FLAG, CEL_OFFSET, CEL_PHI0, CEL_PRJ, CEL_REF,
     :          CEL_THETA0

      PARAMETER (CEL_FLAG   = 100)
      PARAMETER (CEL_OFFSET = 101)
      PARAMETER (CEL_PHI0   = 102)
      PARAMETER (CEL_THETA0 = 103)
      PARAMETER (CEL_REF    = 104)
      PARAMETER (CEL_PRJ    = 105)

*     Codes for CEL data structure elements used by CELGET (only).
      INTEGER   CEL_EULER, CEL_ISOLAT, CEL_LATPRQ

      PARAMETER (CEL_EULER  = 200)
      PARAMETER (CEL_LATPRQ = 201)
      PARAMETER (CEL_ISOLAT = 202)

      CHARACTER CEL_ERRMSG(0:6)*80
      COMMON /CEL_DATA/ CEL_ERRMSG
