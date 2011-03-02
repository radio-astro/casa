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
*   $Id: ttab3.f,v 4.3 2007/12/27 05:42:52 cal103 Exp $
*=======================================================================

      PROGRAM TTAB3
*-----------------------------------------------------------------------
*
*   TTAB3 tests the -TAB routines using PGPLOT for graphical display.
*   It constructs a table that approximates Bonne's projection and uses
*   it to draw a graticule.
*
*-----------------------------------------------------------------------

*     Set up the lookup table.
      INTEGER   K1, K2, M
      PARAMETER (M = 2, K1 = 271, K2 = 235)

      INTEGER   K(M), MAP(M)
      DOUBLE PRECISION CRVAL(M)
      DATA K     /K1, K2/
      DATA MAP   /0, 1/
      DATA CRVAL /135D0, 95D0/

      INTEGER   CI, I, IK, ILAT, ILNG, IM, J, STAT(K1,K2), STATUS
      REAL      XR(361), YR(361)
      DOUBLE PRECISION COORD(M,K1,K2), WORLD(M,361), X(K1), XY(M,361),
     :          Y(K2)

      INCLUDE 'prj.inc'
      INCLUDE 'tab.inc'
      INTEGER   PRJ(PRJLEN), TAB(TABLEN)
*-----------------------------------------------------------------------

      WRITE (*, 10)
 10   FORMAT ('Testing WCSLIB inverse coordinate lookup table ',
     :        'routines (ttab3.f)',/,
     :        '-----------------------------------------------',
     :        '------------------',/)


*     PGPLOT initialization.
      CALL PGBEG (0, '/xwindow', 1, 1)
      CALL PGVSTD ()
      CALL PGSCH (0.7)
      CALL PGWNAD (-135.0, 135.0, -95.0, 140.0)
      CALL PGBOX ('BC', 0.0, 0, 'BC', 0.0, 0)

      CALL PGSCR (0, 0.00, 0.00, 0.00)
      CALL PGSCR (1, 1.00, 1.00, 0.00)
      CALL PGSCR (2, 1.00, 1.00, 1.00)
      CALL PGSCR (3, 0.50, 0.50, 0.80)
      CALL PGSCR (4, 0.80, 0.50, 0.50)
      CALL PGSCR (5, 0.80, 0.80, 0.80)
      CALL PGSCR (6, 0.50, 0.50, 0.80)
      CALL PGSCR (7, 0.80, 0.50, 0.50)
      CALL PGSCR (8, 0.30, 0.50, 0.30)


*     Set up the lookup table.
      STATUS = TABPUT (TAB, TAB_FLAG, -1, 0, 0)
      STATUS = TABINI(M, K, TAB)
      IF (STATUS.NE.0) THEN
         WRITE (*, 20) STATUS
 20      FORMAT ('TABINI ERROR',I2,'.')
         GO TO 999
      END IF

      STATUS = TABPUT (TAB, TAB_M, M, 0, 0)
      DO 40 IM = 1, M
         STATUS = TABPUT (TAB, TAB_K,     K(IM),     IM, 0)
         STATUS = TABPUT (TAB, TAB_MAP,   MAP(IM),   IM, 0)
         STATUS = TABPUT (TAB, TAB_CRVAL, CRVAL(IM), IM, 0)

         DO 30 IK = 1, K(IM)
            STATUS = TABPUT (TAB, TAB_INDEX, DBLE(IK-1), IM, IK)
 30      CONTINUE
 40   CONTINUE

*     Set up the lookup table to approximate Bonne's projection.
      DO 50 I = 1, K1
         X(I) = 136 - I
 50   CONTINUE
      DO 60 J = 1, K2
         Y(J) = J - 96
 60   CONTINUE

      STATUS = PRJINI (PRJ)
      STATUS = PRJPUT (PRJ, PRJ_PV, 35D0, 1)
      STATUS = BONX2S (PRJ, K1, K2, 1, 2, X, Y, COORD(1,1,1),
     :                 COORD(2,1,1), STAT)

      IK = 1
      DO 80 J = 1, K2
         DO 70 I = 1, K1
            IF (STAT(I,J).NE.0) THEN
               COORD(1,I,J) = 999D0
               COORD(2,I,J) = 999D0
            END IF

            STATUS = TABPUT (TAB, TAB_COORD, COORD(1,I,J), IK, 0)
            STATUS = TABPUT (TAB, TAB_COORD, COORD(2,I,J), IK+1, 0)
            IK = IK + 2
 70      CONTINUE
 80   CONTINUE


*     Draw meridians.
      CI = 1
      DO 110 ILNG = 0, 0, 15
*      DO 110 ILNG = -180, 180, 15
         CI = CI + 1
         IF (CI.GT.7) CI = 2
         IF (ILNG.NE.0) THEN
            CALL PGSCI (CI)
         ELSE
            CALL PGSCI (1)
         END IF

         J = 0
         DO 90 ILAT = -90, 90
            J = J + 1
            WORLD(1,J) = DBLE(ILNG)
            WORLD(2,J) = DBLE(ILAT)
 90      CONTINUE

*        A fudge to account for the singularity at the poles.
         WORLD(1,1)   = 0D0
         WORLD(1,181) = 0D0

         STATUS = TABS2X (TAB, 181, 2, WORLD, XY, STAT)

         IK = 0
         DO 100 J = 1, 181
            IF (STAT(J,1).NE.0) THEN
               IF (IK.GT.1) CALL PGLINE (K, XR, YR)
               IK = 0
               GO TO 100
            END IF

            IK = IK + 1
            XR(IK) = XY(1,J)
            YR(IK) = XY(2,J)
 100     CONTINUE

         CALL PGLINE (IK, XR, YR)
 110  CONTINUE


*     Draw parallels.
      CI = 1
      DO 140 ILAT = -15, -15, 15
*      DO 140 ILAT = -75, 75, 15
         CI = CI + 1
         IF (CI.GT.7) CI = 2
         IF (ILAT.NE.0) THEN
            CALL PGSCI (CI)
         ELSE
            CALL PGSCI (1)
         END IF

         J = 0
         DO 120 ILNG = -180, 180
            J = J + 1
            WORLD(1,J) = DBLE(ILNG)
            WORLD(2,J) = DBLE(ILAT)
 120     CONTINUE

         STATUS = TABS2X (TAB, 361, 2, WORLD, XY, STAT)

         IK = 0
         DO 130 J = 1, 361
            IF (STAT(J,1).NE.0) THEN
               IF (IK.GT.1) CALL PGLINE (IK, XR, YR)
               IK = 0
               GO TO 130
            END IF

            IK = IK + 1
            XR(IK) = XY(1,J)
            YR(IK) = XY(2,J)
 130     CONTINUE

         CALL PGLINE (IK, XR, YR)
 140  CONTINUE

      CALL PGEND ()

 999  STATUS = TABFREE (TAB)

      END
