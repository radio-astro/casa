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
*   $Id: wcs.inc,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*=======================================================================

*     Functions.
      INTEGER   WCSCOPY, WCSFREE, WCSGET, WCSINI, WCSMIX, WCSNPS,
     :          WCSNPV, WCSP2S, WCSPRT, WCSSPTR, WCSPUT, WCSS2P, WCSSET,
     :          WCSSUB

*     Length of the WCSPRM data structure (INTEGER array).
      INTEGER   WCSLEN
      PARAMETER (WCSLEN = 400)

*     Codes for WCS data structure elements used by WCSPUT and WCSGET.
      INTEGER   WCS_ALT, WCS_ALTLIN, WCS_CD, WCS_CDELT, WCS_CNAME,
     :          WCS_COLNUM, WCS_CRDER, WCS_CROTA, WCS_CRPIX, WCS_CRVAL,
     :          WCS_CSYER, WCS_CTYPE, WCS_CUNIT, WCS_DATEAVG,
     :          WCS_DATEOBS, WCS_EQUINOX, WCS_FLAG, WCS_LATPOLE,
     :          WCS_LONPOLE, WCS_MJDAVG, WCS_MJDOBS, WCS_NAXIS, WCS_NPS,
     :          WCS_NPSMAX, WCS_NPV, WCS_NPVMAX, WCS_OBSGEO, WCS_PC,
     :          WCS_PS, WCS_PV, WCS_RADESYS, WCS_RESTFRQ, WCS_RESTWAV,
     :          WCS_SPECSYS, WCS_SSYSOBS, WCS_SSYSSRC, WCS_VELOSYS,
     :          WCS_WCSNAME, WCS_ZSOURCE

      PARAMETER (WCS_FLAG     = 100)
      PARAMETER (WCS_NAXIS    = 101)
      PARAMETER (WCS_CRPIX    = 102)
      PARAMETER (WCS_PC       = 103)
      PARAMETER (WCS_CDELT    = 104)
      PARAMETER (WCS_CRVAL    = 105)
      PARAMETER (WCS_CUNIT    = 106)
      PARAMETER (WCS_CTYPE    = 107)
      PARAMETER (WCS_LONPOLE  = 108)
      PARAMETER (WCS_LATPOLE  = 109)
      PARAMETER (WCS_RESTFRQ  = 110)
      PARAMETER (WCS_RESTWAV  = 111)
      PARAMETER (WCS_NPV      = 112)
      PARAMETER (WCS_NPVMAX   = 113)
      PARAMETER (WCS_PV       = 114)
      PARAMETER (WCS_NPS      = 115)
      PARAMETER (WCS_NPSMAX   = 116)
      PARAMETER (WCS_PS       = 117)
      PARAMETER (WCS_ALTLIN   = 118)
      PARAMETER (WCS_CD       = 119)
      PARAMETER (WCS_CROTA    = 120)
      PARAMETER (WCS_ALT      = 121)
      PARAMETER (WCS_COLNUM   = 122)
      PARAMETER (WCS_WCSNAME  = 123)
      PARAMETER (WCS_CNAME    = 124)
      PARAMETER (WCS_CRDER    = 125)
      PARAMETER (WCS_CSYER    = 126)
      PARAMETER (WCS_RADESYS  = 127)
      PARAMETER (WCS_EQUINOX  = 128)
      PARAMETER (WCS_SPECSYS  = 129)
      PARAMETER (WCS_SSYSOBS  = 130)
      PARAMETER (WCS_VELOSYS  = 131)
      PARAMETER (WCS_SSYSSRC  = 132)
      PARAMETER (WCS_ZSOURCE  = 133)
      PARAMETER (WCS_OBSGEO   = 134)
      PARAMETER (WCS_DATEOBS  = 135)
      PARAMETER (WCS_DATEAVG  = 136)
      PARAMETER (WCS_MJDOBS   = 137)
      PARAMETER (WCS_MJDAVG   = 138)

*     Codes for WCS data structure elements used by WCSGET (only).
      INTEGER   WCS_CEL, WCS_CUBEFACE, WCS_LAT, WCS_LATTYP, WCS_LIN,
     :          WCS_LNG, WCS_LNGTYP, WCS_NTAB, WCS_NWTB, WCS_SPC,
     :          WCS_SPEC, WCS_TAB, WCS_TYPES, WCS_WTB

      PARAMETER (WCS_NTAB     = 200)
      PARAMETER (WCS_TAB      = 201)
      PARAMETER (WCS_NWTB     = 202)
      PARAMETER (WCS_WTB      = 203)
      PARAMETER (WCS_TYPES    = 204)
      PARAMETER (WCS_LNGTYP   = 205)
      PARAMETER (WCS_LATTYP   = 206)
      PARAMETER (WCS_LNG      = 207)
      PARAMETER (WCS_LAT      = 208)
      PARAMETER (WCS_SPEC     = 209)
      PARAMETER (WCS_CUBEFACE = 210)
      PARAMETER (WCS_LIN      = 211)
      PARAMETER (WCS_CEL      = 212)
      PARAMETER (WCS_SPC      = 213)

*     Flag bits for the AXES argument.
      INTEGER WCSSUB_CELESTIAL, WCSSUB_CUBEFACE, WCSSUB_LATITUDE,
     :        WCSSUB_LONGITUDE, WCSSUB_SPECTRAL, WCSSUB_STOKES

      PARAMETER (WCSSUB_LONGITUDE = 4096 +  1)
      PARAMETER (WCSSUB_LATITUDE  = 4096 +  2)
      PARAMETER (WCSSUB_CUBEFACE  = 4096 +  4)
      PARAMETER (WCSSUB_CELESTIAL = 4096 +  7)
      PARAMETER (WCSSUB_SPECTRAL  = 4096 +  8)
      PARAMETER (WCSSUB_STOKES    = 4096 + 16)

      CHARACTER WCS_ERRMSG(0:13)*80
      DATA WCS_ERRMSG /
     :   'Success',
     :   'Null wcsprm pointer passed',
     :   'Memory allocation failed',
     :   'Linear transformation matrix is singular',
     :   'Inconsistent or unrecognized coordinate axis types',
     :   'Invalid parameter value',
     :   'Invalid coordinate transformation parameters',
     :   'Ill-conditioned coordinate transformation parameters',
     :   'One or more of the pixel coordinates were invalid',
     :   'One or more of the world coordinates were invalid',
     :   'Invalid world coordinate',
     :   'No solution found in the specified interval',
     :   'Invalid subimage specification',
     :   'Non-separable subimage coordinate system'/
