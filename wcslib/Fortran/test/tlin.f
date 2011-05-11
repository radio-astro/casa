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
* $Id: tlin.f,v 4.7 2011/02/07 07:03:43 cal103 Exp $
*=======================================================================

      PROGRAM TLIN
*-----------------------------------------------------------------------
*
* TLIN tests the linear transformation routines supplied with WCSLIB.
*
*-----------------------------------------------------------------------
      INTEGER   NAXIS, NCOORD, NELEM
      PARAMETER (NAXIS  = 5)
      PARAMETER (NCOORD = 2)
      PARAMETER (NELEM  = 9)

      INTEGER   I, J, K, STATUS
      DOUBLE PRECISION CDELT(NAXIS), CRPIX(NAXIS), IMG(NELEM,2),
     :          PC(NAXIS,NAXIS), PIX(NELEM,2)

      INCLUDE 'lin.inc'
      INTEGER LIN(LINLEN)
      DOUBLE PRECISION DUMMY
      EQUIVALENCE (LIN,DUMMY)

      DATA (CRPIX(I), I=1,NAXIS)
     :           /256D0, 256D0,  64D0, 128D0,   1D0/
      DATA ((PC(I,J),J=1,NAXIS),I=1,NAXIS)
     :           /  1.0D0,   0.5D0,   0D0,   0D0,   0D0,
     :              0.5D0,   1.0D0,   0D0,   0D0,   0D0,
     :              0.0D0,   0.0D0,   1D0,   0D0,   0D0,
     :              0.0D0,   0.0D0,   0D0,   1D0,   0D0,
     :              0.0D0,   0.0D0,   0D0,   0D0,   1D0/
      DATA (CDELT(I), I=1,NAXIS)
     :           /  1.2D0,   2.3D0,   3.4D0,   4.5D0,   5.6D0/
      DATA ((PIX(I,J), I=1,NAXIS), J=1,2)
     :           /303.0D0, 265.0D0, 112.4D0, 144.5D0,  28.2D0,
     :             19.0D0,  57.0D0,   2.0D0,  15.0D0,  42.0D0/
*-----------------------------------------------------------------------
      WRITE (*, 10)
 10   FORMAT (
     :  'Testing WCSLIB linear transformation routines (tlin.f)',/,
     :  '------------------------------------------------------')

      STATUS = LINPUT (LIN, LIN_FLAG, -1, 0, 0)
      STATUS = LININI (NAXIS, LIN)

      DO 30 I = 1, NAXIS
        STATUS = LINPUT (LIN, LIN_CRPIX, CRPIX(I), I, 0)

        DO 20 J = 1, NAXIS
          STATUS = LINPUT (LIN, LIN_PC, PC(I,J), I, J)
 20     CONTINUE

        STATUS = LINPUT (LIN, LIN_CDELT, CDELT(I), I, 0)
 30   CONTINUE

      WRITE (*, *)
      DO 50 K = 1, NCOORD
        WRITE (*, 40) K, (PIX(J,K), J=1,NAXIS)
 40     FORMAT ('PIX',I2,':',10F14.8)
 50   CONTINUE

      STATUS = LINP2X (LIN, NCOORD, NELEM, PIX, IMG)
      IF (STATUS.NE.0) THEN
        WRITE (*, 60) STATUS
 60     FORMAT ('LINP2X ERROR',I3)
        GO TO 999
      END IF

      WRITE (*, *)
      DO 80 K = 1, NCOORD
        WRITE (*, 70) K, (IMG(J,K), J=1,NAXIS)
 70     FORMAT ('IMG',I2,':',10F14.8)
 80   CONTINUE

      STATUS = LINX2P (LIN, NCOORD, NELEM, IMG, PIX)
      IF (STATUS.NE.0) THEN
        WRITE (*, 90) STATUS
 90     FORMAT ('LINX2P ERROR',I3)
        GO TO 999
      END IF

      WRITE (*, *)
      DO 100 K = 1, NCOORD
        WRITE (*, 40) K, (PIX(J,K), J=1,NAXIS)
 100  CONTINUE

      STATUS = LINP2X (LIN, NCOORD, NELEM, PIX, IMG)
      IF (STATUS.NE.0) THEN
        WRITE (*, 60) STATUS
        GO TO 999
      END IF

      WRITE (*, *)
      DO 110 K = 1, NCOORD
        WRITE (*, 70) K, (IMG(J,K), J=1,NAXIS)
 110  CONTINUE

 999  END
