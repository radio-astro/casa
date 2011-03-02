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
*   $Id: wcsfix.inc,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*=======================================================================

*     Functions.
      INTEGER   WCSFIX

*     Flag bits for the RELAX argument.
      INTEGER   WCSFIX_CEL, WCSFIX_CYL, WCSFIX_DAT, WCSFIX_NWCS,
     :          WCSFIX_SPC, WCSFIX_UNIT

      PARAMETER (WCSFIX_DAT  = 1)
      PARAMETER (WCSFIX_UNIT = 2)
      PARAMETER (WCSFIX_CEL  = 3)
      PARAMETER (WCSFIX_SPC  = 4)
      PARAMETER (WCSFIX_CYL  = 5)
      PARAMETER (WCSFIX_NWCS = 5)

      CHARACTER WCSFIX_ERRMSG(0:10)*80
      DATA WCSFIX_ERRMSG /
     :   'Success',
     :   'Null wcsprm pointer passed',
     :   'Memory allocation failed',
     :   'Linear transformation matrix is singular',
     :   'Inconsistent or unrecognized coordinate axis types',
     :   'Invalid parameter value',
     :   'Invalid coordinate transformation parameters',
     :   'Ill-conditioned coordinate transformation parameters',
     :   'All of the corner pixel coordinates are invalid',
     :   'Could not determine reference pixel coordinate',
     :   'Could not determine reference pixel value'/
