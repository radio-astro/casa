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
* $Id: cel_data.f,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=======================================================================

      BLOCK DATA CEL_BLOCK_DATA

      CHARACTER CEL_ERRMSG(0:6)*80

      COMMON /CEL_DATA/ CEL_ERRMSG

      DATA CEL_ERRMSG /
     :  'Success',
     :  'Null celprm pointer passed',
     :  'Invalid projection parameters',
     :  'Invalid coordinate transformation parameters',
     :  'Ill-conditioned coordinate transformation parameters',
     :  'One or more of the (x,y) coordinates were invalid',
     :  'One or more of the (lng,lat) coordinates were invalid'/

      END
