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
* $Id: fitshdr_data.f,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*=======================================================================

      BLOCK DATA FITSHDR_BLOCK_DATA

      CHARACTER FITSHDR_ERRMSG(0:2)*80

      COMMON /FITSHDR_DATA/ FITSHDR_ERRMSG

      DATA FITSHDR_ERRMSG /
     :  'Success',
     :  'Null fitskey pointer-pointer passed',
     :  'Memory allocation failed'/

      END
