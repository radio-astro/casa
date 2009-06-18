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
*   $Id: wcsunits.inc,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*=======================================================================

*     Functions.
      INTEGER   WCSULEX, WCSUNITS, WCSUTRN

*     Array indices.
      INTEGER   WCSUNITS_BEAM, WCSUNITS_BIN, WCSUNITS_BIT,
     :          WCSUNITS_CHARGE, WCSUNITS_COUNT, WCSUNITS_LENGTH,
     :          WCSUNITS_LUMINTEN, WCSUNITS_MAGNITUDE, WCSUNITS_MASS,
     :          WCSUNITS_MOLE, WCSUNITS_NTYPE, WCSUNITS_PIXEL,
     :          WCSUNITS_PLANE_ANGLE, WCSUNITS_SOLID_ANGLE,
     :          WCSUNITS_SOLRATIO, WCSUNITS_TEMPERATURE, WCSUNITS_TIME,
     :          WCSUNITS_VOXEL

      PARAMETER (WCSUNITS_PLANE_ANGLE =  1)
      PARAMETER (WCSUNITS_SOLID_ANGLE =  2)
      PARAMETER (WCSUNITS_CHARGE      =  3)
      PARAMETER (WCSUNITS_MOLE        =  4)
      PARAMETER (WCSUNITS_TEMPERATURE =  5)
      PARAMETER (WCSUNITS_LUMINTEN    =  6)
      PARAMETER (WCSUNITS_MASS        =  7)
      PARAMETER (WCSUNITS_LENGTH      =  8)
      PARAMETER (WCSUNITS_TIME        =  9)
      PARAMETER (WCSUNITS_BEAM        = 10)
      PARAMETER (WCSUNITS_BIN         = 11)
      PARAMETER (WCSUNITS_BIT         = 12)
      PARAMETER (WCSUNITS_COUNT       = 13)
      PARAMETER (WCSUNITS_MAGNITUDE   = 14)
      PARAMETER (WCSUNITS_PIXEL       = 15)
      PARAMETER (WCSUNITS_SOLRATIO    = 16)
      PARAMETER (WCSUNITS_VOXEL       = 17)
      PARAMETER (WCSUNITS_NTYPE       = 17)

      CHARACTER WCSUNITS_ERRMSG(0:12)*40,
     :          WCSUNITS_TYPES(WCSUNITS_NTYPE)*18,
     :          WCSUNITS_UNITS(WCSUNITS_NTYPE)*9

      DATA WCSUNITS_ERRMSG /
     :   'Success',
     :   'Invalid numeric multiplier',
     :   'Dangling binary operator',
     :   'Invalid symbol in INITIAL context',
     :   'Function in invalid context',
     :   'Invalid symbol in EXPON context',
     :   'Unbalanced bracket',
     :   'Unbalanced parenthesis',
     :   'Consecutive binary operators',
     :   'Internal parser error',
     :   'Non-conformant unit specifications',
     :   'Non-conformant functions',
     :   'Potentially unsafe translation'/

      DATA WCSUNITS_TYPES /
     :   'plane angle', 'solid angle', 'charge', 'mole', 'temperature',
     :   'luminous intensity', 'mass', 'length', 'time', 'beam', 'bin',
     :   'bit', 'count', 'stellar magnitude', 'pixel', 'solar ratio',
     :   'voxel'/

      DATA WCSUNITS_UNITS /
     :   'degree', 'steradian', 'Coulomb', 'mole', 'Kelvin', 'candela',
     :   'kilogram', 'metre', 'second', '', '', '', '', '', '', '', ''/
