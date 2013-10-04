*=======================================================================
*
* PGSBOX 4.7 - draw curvilinear coordinate axes for PGPLOT.
* Copyright (C) 1997-2011, Mark Calabretta
*
* This file is part of PGSBOX.
*
* PGSBOX is free software: you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PGSBOX is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with PGSBOX.  If not, see http://www.gnu.org/licenses.
*
* Correspondence concerning PGSBOX may be directed to:
*   Internet email: mcalabre@atnf.csiro.au
*   Postal address: Dr. Mark Calabretta
*                   Australia Telescope National Facility, CSIRO
*                   PO Box 76
*                   Epping NSW 1710
*                   AUSTRALIA
*
* Author: Mark Calabretta, Australia Telescope National Facility
* http://www.atnf.csiro.au/~mcalabre/index.html
* $Id: fscan.f,v 4.7 2011/02/07 07:03:43 cal103 Exp $
*=======================================================================
*
* FSCAN defines an azimuth/frequency coordinate system for PGSBOX.
*
* Given:
*   OPCODE    I         Transformation code:
*                         +2: Compute a set of pixel coordinates that
*                             describe a path between this and the
*                             previous pair of world coordinates
*                             remembered from the last call with
*                             OPCODE = +1 or +2.
*                         +1: Compute pixel coordinates from world
*                             coordinates.
*                          0: Initialize.
*                         -1: Compute world coordinates from pixel
*                             coordinates.
*
*   NLC       I         Number of elements in NLCPRM (=1).
*
*   NLI       I         Number of elements in NLIPRM (=1).
*
*   NLD       I         Number of elements in NLDPRM (=7).
*
*   NLCPRM    C(NLC)*1  Character array (ignored).
*
*   NLIPRM    I(NLI)    Integer array (ignored).
*
* Given and/or returned:
*   NLDPRM    D(NLD)    Double precision coefficients (see below).
*
*   WORLD     D(2)      World coordinates.  WORLD(1) and WORLD(2)
*                       are the longitude in degrees, and
*                       log10(frequency/1Hz).
*                       Given if OPCODE > 0, returned if OPCODE < 0.
*
*   PIXEL     D(2)      Pixel coordinates.
*                       Given if OPCODE < 0, returned if OPCODE > 0.
*
*   CONTRL    I         Control flag for OPCODE = +2 (ignored, always
*                       set to 0 on return).
*
*   CONTXT    D(20)     Context elements (ignored).
*
* Returned:
*   IERR      I         Status return value:
*                         0: Success.
*                         1: Invalid coordinate transformation
*                            parameters.
*
* Notes:
*   The NLDPRM array is constructed as follows:
*     - (1)  Axis 1 reference pixel coordinate
*     - (2)  Axis 2 reference pixel coordinate
*     - (3)  Axis 1 reference pixel value (degree)
*     - (4)  Axis 2 reference pixel value
*     - (5)  Axis 1 coordinate increment (degree/pixel)
*     - (6)  Axis 2 coordinate increment
*     - (7)  Rate of change of NLDPRM(4) with x-pixel
*
*=======================================================================
      SUBROUTINE FSCAN (OPCODE, NLC, NLI, NLD, NLCPRM, NLIPRM, NLDPRM,
     :   WORLD, PIXEL, CONTRL, CONTXT, IERR)
*-----------------------------------------------------------------------
      INTEGER   CONTRL, IERR, NLC, NLD, NLI, NLIPRM(NLI), OPCODE
      DOUBLE PRECISION CONTXT(20), NLDPRM(NLD), PIXEL(2), S, WORLD(2)
      CHARACTER NLCPRM(NLC)*1
*-----------------------------------------------------------------------
      IERR = 0

      IF (OPCODE.GT.0) THEN
*       Compute pixel coordinates from world coordinates.
        PIXEL(1) =  NLDPRM(1) + (WORLD(1) - NLDPRM(3))/NLDPRM(5)
        S =  NLDPRM(4) + PIXEL(1)*NLDPRM(7)
        PIXEL(2) =  NLDPRM(2) + (WORLD(2) - S)/NLDPRM(6)

        CONTRL = 0

      ELSE IF (OPCODE.EQ.0) THEN
*       Initialize.
        IF (NLC.LT.1 .OR. NLI.LT.1 .OR. NLD.LT.7) IERR = 1
        IF (NLDPRM(5).EQ.0D0) IERR = 1
        IF (NLDPRM(6).EQ.0D0) IERR = 1
        IF (NLDPRM(7).EQ.0D0) IERR = 1

        CONTRL = 0

      ELSE IF (OPCODE.EQ.-1) THEN
*       Compute world coordinates from pixel coordinates.
        WORLD(1) = NLDPRM(3) + NLDPRM(5)*(PIXEL(1) - NLDPRM(1))
        WORLD(2) = NLDPRM(4) + NLDPRM(6)*(PIXEL(2) - NLDPRM(2)) +
     :                PIXEL(1)*NLDPRM(7)

      ELSE
        IERR = 1
      END IF


      RETURN
      END
