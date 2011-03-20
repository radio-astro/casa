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
* $Id: wcshdr.inc,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=======================================================================

*     Functions.
      EXTERNAL  WCSBDX, WCSBTH, WCSIDX, WCSPIH, WCSTAB, WCSVCOPY,
     :          WCSVFREE
      INTEGER   WCSBDX, WCSBTH, WCSIDX, WCSPIH, WCSTAB, WCSVCOPY,
     :          WCSVFREE

*     Flag bits for the RELAX argument.
      INTEGER   WCSHDR_all, WCSHDR_ALLIMG, WCSHDR_AUXIMG,
     :          WCSHDR_BIMGARR, WCSHDR_CD00i00j, WCSHDR_CNAMn,
     :          WCSHDR_CROTAia, WCSHDR_DOBSn, WCSHDR_EPOCHa,
     :          WCSHDR_IMGHEAD, WCSHDR_LONGKEY, WCSHDR_none,
     :          WCSHDR_PC00i00j, WCSHDR_PIXLIST, WCSHDR_PROJPn,
     :          WCSHDR_RADECSYS, WCSHDR_reject, WCSHDR_VELREFa,
     :          WCSHDR_VSOURCE

      PARAMETER (WCSHDR_none     = 0)
      PARAMETER (WCSHDR_all      = 2**20 - 1)
      PARAMETER (WCSHDR_reject   = 2**28)

      PARAMETER (WCSHDR_CROTAia  = 2**0)
      PARAMETER (WCSHDR_EPOCHa   = 2**1)
      PARAMETER (WCSHDR_VELREFa  = 2**2)
      PARAMETER (WCSHDR_CD00i00j = 2**3)
      PARAMETER (WCSHDR_PC00i00j = 2**4)
      PARAMETER (WCSHDR_PROJPn   = 2**5)
      PARAMETER (WCSHDR_RADECSYS = 2**6)
      PARAMETER (WCSHDR_VSOURCE  = 2**7)
      PARAMETER (WCSHDR_DOBSn    = 2**8)
      PARAMETER (WCSHDR_LONGKEY  = 2**9)
      PARAMETER (WCSHDR_CNAMn    = 2**10)
      PARAMETER (WCSHDR_AUXIMG   = 2**11)
      PARAMETER (WCSHDR_ALLIMG   = 2**12)

      PARAMETER (WCSHDR_IMGHEAD  = 2**16)
      PARAMETER (WCSHDR_BIMGARR  = 2**17)
      PARAMETER (WCSHDR_PIXLIST  = 2**18)

      CHARACTER WCSHDR_ERRMSG(0:3)*80
      COMMON /WCSHDR_DATA/ WCSHDR_ERRMSG
