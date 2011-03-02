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
*   $Id: tprj1.f,v 4.3 2007/12/27 05:42:52 cal103 Exp $
*=======================================================================

      PROGRAM TPRJ1
*-----------------------------------------------------------------------
*
*   TPRJ1 tests forward and reverse spherical projections for closure.
*
*-----------------------------------------------------------------------
      INTEGER   J
      DOUBLE PRECISION PV(0:29)

      DOUBLE PRECISION PI
      PARAMETER (PI = 3.141592653589793238462643D0)

      DOUBLE PRECISION TOL
      PARAMETER (TOL = 1D-9)
*-----------------------------------------------------------------------
      WRITE (*, 10)
 10   FORMAT ('Testing closure of WCSLIB spherical projection ',
     :        'routines (tprj1.f)',/,
     :        '-----------------------------------------------',
     :        '------------------')

      DO 20 J = 0, 29
         PV(J) = 0D0
 20   CONTINUE

*     AZP: zenithal/azimuthal perspective.
      PV(1) = 0.5D0
      PV(2) =  30D0
      CALL PROJEX ('AZP', PV, 90,   5, TOL)

*     SZP: slant zenithal perspective.
      PV(1) = 0.5D0
      PV(2) = 210D0
      PV(3) =  60D0
      CALL PROJEX ('SZP', PV, 90, -90, TOL)

*     TAN: gnomonic.
      CALL PROJEX ('TAN', PV, 90,   5, TOL)

*     STG: stereographic.
      CALL PROJEX ('STG', PV, 90, -85, TOL)

*     SIN: orthographic/synthesis.
      PV(1) = -0.3D0
      PV(2) =  0.5D0
      CALL PROJEX ('SIN', PV, 90,  45, TOL)

*     ARC: zenithal/azimuthal equidistant.
      CALL PROJEX ('ARC', PV, 90, -90, TOL)

*     ZPN: zenithal/azimuthal polynomial.
      PV(0) =  0.00000D0
      PV(1) =  0.95000D0
      PV(2) = -0.02500D0
      PV(3) = -0.15833D0
      PV(4) =  0.00208D0
      PV(5) =  0.00792D0
      PV(6) = -0.00007D0
      PV(7) = -0.00019D0
      PV(8) =  0.00000D0
      PV(9) =  0.00000D0
      CALL PROJEX ('ZPN', PV, 90,  10, TOL)

*     ZEA: zenithal/azimuthal equal area.
      CALL PROJEX ('ZEA', PV, 90, -85, TOL)

*     AIR: Airy's zenithal projection.
      PV(1) = 45D0
      CALL PROJEX ('AIR', PV, 90, -85, TOL)

*     CYP: cylindrical perspective.
      PV(1) = 3.0D0
      PV(2) = 0.8D0
      CALL PROJEX ('CYP', PV, 90, -90, TOL)

*     CEA: cylindrical equal area.
      PV(1) = 0.75D0
      CALL PROJEX ('CEA', PV, 90, -90, TOL)

*     CAR: plate carree.
      CALL PROJEX ('CAR', PV, 90, -90, TOL)

*     MER: Mercator's.
      CALL PROJEX ('MER', PV, 85, -85, TOL)

*     SFL: Sanson-Flamsteed.
      CALL PROJEX ('SFL', PV, 90, -90, TOL)

*     PAR: parabolic.
      CALL PROJEX ('PAR', PV, 90, -90, TOL)

*     MOL: Mollweide's projection.
      CALL PROJEX ('MOL', PV, 90, -90, TOL)

*     AIT: Hammer-Aitoff.
      CALL PROJEX ('AIT', PV, 90, -90, TOL)

*     COP: conic perspective.
      PV(1) =  60D0
      PV(2) =  15D0
      CALL PROJEX ('COP', PV, 90, -25, TOL)

*     COE: conic equal area.
      PV(1) =  60D0
      PV(2) = -15D0
      CALL PROJEX ('COE', PV, 90, -90, TOL)

*     COD: conic equidistant.
      PV(1) = -60D0
      PV(2) =  15D0
      CALL PROJEX ('COD', PV, 90, -90, TOL)

*     COO: conic orthomorphic.
      PV(1) = -60D0
      PV(2) = -15D0
      CALL PROJEX ('COO', PV, 85, -90, TOL)

*     BON: Bonne's projection.
      PV(1) = 30D0
      CALL PROJEX ('BON', PV, 90, -90, TOL)

*     PCO: polyconic.
      CALL PROJEX ('PCO', PV, 90, -90, TOL)

*     TSC: tangential spherical cube.
      CALL PROJEX ('TSC', PV, 90, -90, TOL)

*     CSC: COBE quadrilateralized spherical cube.
      CALL PROJEX ('CSC', PV, 90, -90, 4D-2)

*     QSC: quadrilateralized spherical cube.
      CALL PROJEX ('QSC', PV, 90, -90, TOL)

*     HPX: HEALPix projection.
      PV(1) = 4D0
      PV(2) = 3D0
      CALL PROJEX ('HPX', PV, 90, -90, TOL)

      END


      SUBROUTINE PROJEX (PCODE, PV, NORTH, SOUTH, TOL)
*-----------------------------------------------------------------------
*   PROJEX exercises the spherical projection routines.
*
*   Given:
*      PCODE    C*3      Projection code.
*      PV       D(0:29)  Projection parameters.
*      NORTH    I        Northern cutoff latitude, degrees.
*      SOUTH    I        Southern cutoff latitude, degrees.
*      TOL      D        Reporting tolerance, degrees.
*-----------------------------------------------------------------------
      INTEGER   J, LAT, LNG, NORTH, SOUTH, STAT1(361), STAT2(361),
     :          STATUS
      DOUBLE PRECISION DLAT, DLATMX, DLNG, DLNGMX, DR, DRMAX, LAT1,
     :          LAT2(361), LNG1(361), LNG2(361), PV(0:29), R, THETA,
     :          TOL, X(361), X1(361), X2(361), Y(361), Y1(361), Y2(361)
      CHARACTER PCODE*3

      INCLUDE 'prj.inc'
      INTEGER   PRJ(PRJLEN)

      DOUBLE PRECISION D2R, PI
      PARAMETER (PI = 3.141592653589793238462643D0)
      PARAMETER (D2R = PI/180D0)
*-----------------------------------------------------------------------
      STATUS = PRJINI(PRJ)

      DO 10 J = 0, 29
         STATUS = PRJPUT (PRJ, PRJ_PV, PV(J), J)
 10   CONTINUE

      STATUS = PRJPUT (PRJ, PRJ_CODE, PCODE, 0)

*     Uncomment the next line to test alternative initializations of
*     projection parameters.
*     STATUS = PRJPUT (PRJ, PRJ_R0, 180D0/PI, 0)

      WRITE (*, 20) PCODE, NORTH, SOUTH, TOL
 20   FORMAT ('Testing ',A3,'; latitudes',I3,' to',I4,
     :        ', reporting tolerance',1PG8.1,' deg.')

      DLNGMX = 0D0
      DLATMX = 0D0

      DO 80 LAT = NORTH, SOUTH, -1
         LAT1 = DBLE(LAT)

         J = 1
         DO 30 LNG = -180, 180
            LNG1(J) = DBLE(LNG)
            J = J + 1
 30      CONTINUE

         STATUS = PRJS2X (PRJ, 361, 1, 1, 1, LNG1, LAT1, X, Y, STAT1)
         IF (STATUS.EQ.1) THEN
            WRITE (*, 40) PCODE, STATUS
 40         FORMAT (3X,A3,'(S2X) ERROR',I2)
            GO TO 80
         END IF

         STATUS = PRJX2S (PRJ, 361, 0, 1, 1, X, Y, LNG2, LAT2, STAT2)
         IF (STATUS.EQ.1) THEN
            WRITE (*, 50) PCODE, STATUS
 50         FORMAT (3X,A3,'(X2S) ERROR',I2)
            GO TO 80
         END IF

         LNG = -180
         DO 70 J = 1, 361
            IF (STAT1(J).NE.0) GO TO 70

            IF (STAT2(J).NE.0) THEN
               WRITE (*, 55) PCODE, LNG1(J), LAT1, X(J), Y(J), STAT2(J)
 55            FORMAT (3X,A3,'(X2S): lng1 =',F20.15,'  lat1 =',F20.15,/,
     :                 '                x =',F20.15,'     y =',F20.15,
     :                 '  ERROR',I3)
               GO TO 70
            END IF

            DLNG = ABS(LNG2(J) - LNG1(J))
            IF (DLNG.GT.180D0) DLNG = ABS(DLNG-360D0)
            IF (ABS(LAT).NE.90 .AND. DLNG.GT.DLNGMX) DLNGMX = DLNG
            DLAT = ABS(LAT2(J) - LAT1)
            IF (DLAT.GT.DLATMX) DLATMX = DLAT

            IF (DLAT.GT.TOL) THEN
               WRITE (*, 60) PCODE, LNG1(J), LAT1, X(J), Y(J), LNG2(J),
     :                       LAT2(J)
 60            FORMAT (8X,A3,': lng1 =',F20.15,'  lat1 =',F20.15,/,
     :                 8X,'        x =',F20.15,'     y =',F20.15,/,
     :                 8X,'     lng2 =',F20.15,'  lat2 =',F20.15)
            ELSE IF (ABS(LAT).NE.90) THEN
               IF (DLNG.GT.TOL) THEN
                  WRITE (*, 60) PCODE, LNG1(J), LAT1, X(J), Y(J),
     :                          LNG2(J), LAT2(J)
               END IF
            END IF
 70      CONTINUE
 80   CONTINUE

      WRITE (*, 90) DLNGMX, DLATMX
 90   FORMAT (13X,'Maximum residual (sky): lng',1P,E10.3,'   lat',E10.3)


*     Test closure at points close to the reference point.
      R = 1.0
      THETA = -180D0

      DRMAX = 0D0

      DO 140 J = 1, 12
         X1(1) = R*COS(THETA*D2R)
         Y1(1) = R*SIN(THETA*D2R)

         STATUS = PRJX2S (PRJ, 1, 1, 1, 1, X1, Y1, LNG1, LAT1, STAT2)
         IF (STATUS.NE.0) THEN
            WRITE (*, 100) PCODE, X1(1), Y1(1), STATUS
 100        FORMAT (8X,A3,'(X2S):   x1 =',F20.15,'    y1 =',F20.15,
     :              '  ERROR',I3)
            GO TO 130
         END IF

         STATUS = PRJS2X (PRJ, 1, 1, 1, 1, LNG1, LAT1, X2, Y2, STAT1)
         IF (STATUS.NE.0) THEN
            WRITE (*, 110) PCODE, X1(1), Y1(1), LNG1(1), LAT1, STATUS
 110        FORMAT (3X,A3,':   x1 =',F20.15,'    y1 =',F20.15,/,
     :              3X,'      lng =',F20.15,'   lat =',F20.15,'  ERROR',
     :              I3)
            GO TO 130
         END IF

         DR = SQRT((X2(1)-X1(1))**2 + (Y2(1)-Y1(1))**2)
         IF (DR.GT.DRMAX) DRMAX = DR
         IF (DR.GT.TOL) THEN
            WRITE (*, 120) PCODE, X1(1), Y1(1), LNG1(1), LAT1, X2(1),
     :                     Y2(1)
 120        FORMAT (8X,A3,':   x1 =',F20.15,'    y1 =',F20.15,/,
     :              8X,'      lng =',F20.15,'   lat =',F20.15,/,
     :              8X,'       x2 =',F20.15,'    y2 =',F20.15)
         END IF

 130     R = R/10D0
         THETA = THETA + 15D0
 140  CONTINUE

      WRITE (*, 150) DRMAX
 150  FORMAT (13X,'Maximum residual (ref):  dR',1PE10.3)


      RETURN
      END
