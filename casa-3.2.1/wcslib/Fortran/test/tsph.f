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
*   $Id: tsph.f,v 4.3 2007/12/27 05:42:52 cal103 Exp $
*=======================================================================

      PROGRAM TSPH
*-----------------------------------------------------------------------
*
*   TSPH tests the forward and reverse spherical coordinate transfor-
*   mation routines for closure.
*
*-----------------------------------------------------------------------
      INTEGER   J, LAT, LNG, SPHS2X, SPHX2S, STATUS
      DOUBLE PRECISION COSLAT, LNG1(361), LNG2(361), EUL(5), LAT1,
     :          LAT2(361), PHI(361), THETA(361), TOL, ZETA

      PARAMETER (TOL = 1D-12)

      DOUBLE PRECISION D2R, PI
      PARAMETER (PI = 3.141592653589793238462643D0)
      PARAMETER (D2R = PI/180D0)
*-----------------------------------------------------------------------
      WRITE (*, 5)
 5    FORMAT ('Testing closure of WCSLIB coordinate transformation ',
     :        'routines (tsph.f)',/,
     :        '----------------------------------------------------',
     :        '-----------------')

*     Set reference angles.
      EUL(1) =  90D0
      EUL(2) =  30D0
      EUL(3) = -90D0
      WRITE (*, 10) (EUL(J),J=1,3)
 10   FORMAT (/,'Celestial longitude and latitude of the native pole, ',
     :        'and native',/,'longitude of the celestial pole ',
     :        '(degrees):',3F10.4)

      EUL(4) = COS(EUL(2)*D2R)
      EUL(5) = SIN(EUL(2)*D2R)

      WRITE (*, 20) TOL
 20   FORMAT ('Reporting tolerance:',1PG8.1,' degrees of arc.')

      DO 70 LAT = 90, -90, -1
         LAT1 = DBLE(LAT)
         COSLAT = COS(LAT1*D2R)

         J = 1
         DO 30 LNG = -180, 180
            LNG1(J) = DBLE(LNG)
            J = J + 1
 30      CONTINUE

         STATUS = SPHS2X (EUL, 361, 1, 1, 1, LNG1, LAT1, PHI, THETA)
         STATUS = SPHX2S (EUL, 361, 0, 1, 1, PHI, THETA, LNG2, LAT2)

         DO 60 J = 1, 361
            IF (ABS(LAT2(J)-LAT1).GT.TOL .OR.
     :         (ABS(LNG2(J)-LNG1(J))*COSLAT.GT.TOL .AND.
     :          ABS(ABS(LNG2(J)-LNG1(J))-360D0)*COSLAT.GT.TOL)) THEN
               WRITE (*, 50) LNG1(J), LAT1, PHI(J), THETA(J), LNG2(J),
     :                       LAT2(J)
 50            FORMAT ('Unclosed: LNG1 =',F20.15,'  LAT1 =',F20.15,/,
     :                 '           PHI =',F20.15,' THETA =',F20.15,/,
     :                 '          LNG2 =',F20.15,'  LAT2 =',F20.15)
            END IF
 60      CONTINUE
 70   CONTINUE


*     Test closure at points close to the pole.
      DO 90 J = -1, 1, 2
         ZETA = 1D0
         LNG1(1) = -180D0

         DO 80 LAT = 1, 12
            LAT1 = DBLE(J)*(90D0 - ZETA)

            STATUS = SPHS2X (EUL, 1, 1, 1, 1, LNG1, LAT1, PHI, THETA)
            STATUS = SPHX2S (EUL, 1, 1, 1, 1, PHI, THETA, LNG2, LAT2)

            IF (ABS(LAT2(1)-LAT1).GT.TOL .OR.
     :         (ABS(LNG2(1)-LNG1(1))*COSLAT.GT.TOL .AND.
     :          ABS(ABS(LNG2(1)-LNG1(1))-360D0)*COSLAT.GT.TOL)) THEN
               WRITE (*, 50) LNG1(1), LAT1, PHI(1), THETA(1), LNG2(1),
     :                       LAT2(1)
            END IF

            ZETA = ZETA/10D0
            LNG1(1) = LNG1(1) + 30D0
 80      CONTINUE
 90   CONTINUE


      END
