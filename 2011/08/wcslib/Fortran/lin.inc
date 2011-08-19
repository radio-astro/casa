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
* $Id: lin.inc,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=======================================================================

*     Functions.
      EXTERNAL  LINCPY, LINFREE, LINGET, LINGTD, LINGTI, LININI, LINP2X,
     :          LINPRT, LINPTD, LINPTI, LINPUT, LINSET, LINX2P
      INTEGER   LINCPY, LINFREE, LINGET, LINGTD, LINGTI, LININI, LINP2X,
     :          LINPRT, LINPTD, LINPTI, LINPUT, LINSET, LINX2P

*     Length of the LINPRM data structure (INTEGER array) on 64-bit
*     machines.  Only needs to be 14 on 32-bit machines.
      INTEGER   LINLEN
      PARAMETER (LINLEN = 22)

*     Codes for LIN data structure elements used by LINPUT and LINGET.
      INTEGER   LIN_CDELT, LIN_CRPIX, LIN_FLAG, LIN_NAXIS, LIN_PC

      PARAMETER (LIN_FLAG   = 100)
      PARAMETER (LIN_NAXIS  = 101)
      PARAMETER (LIN_CRPIX  = 102)
      PARAMETER (LIN_PC     = 103)
      PARAMETER (LIN_CDELT  = 104)

*     Codes for LIN data structure elements used by LINGET (only).
      INTEGER   LIN_IMGPIX, LIN_PIXIMG, LIN_UNITY

      PARAMETER (LIN_UNITY  = 200)
      PARAMETER (LIN_PIXIMG = 201)
      PARAMETER (LIN_IMGPIX = 202)

      CHARACTER LIN_ERRMSG(0:3)*80
      COMMON /LIN_DATA/ LIN_ERRMSG
