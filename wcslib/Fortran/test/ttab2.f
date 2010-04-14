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
*   $Id: ttab2.f,v 4.3 2007/12/27 05:42:52 cal103 Exp $
*=======================================================================

      PROGRAM TTAB2
*-----------------------------------------------------------------------
*
*   TTAB2 tests the -TAB routines using PGPLOT for graphical display.
*   It demonstrates the nature of linear interpolation in 2 dimensions
*   by contouring the interior a single 2 x 2 interpolation element as
*   the values in each corner change.
*
*-----------------------------------------------------------------------

*     Set up a 2 x 2 lookup table.
      INTEGER   K1, K2, M
      PARAMETER (M = 2, K1 = 2, K2 = 2)

      INTEGER   K(2), MAP(2)
      DOUBLE PRECISION CRVAL(2)
      DATA K     /K1, K2/
      DATA MAP   /0, 1/
      DATA CRVAL /0D0, 0D0/

*     Number of subdivisions on each side of the interpolation element.
      INTEGER   NP
      REAL      SCL
      PARAMETER (NP = 128)
      PARAMETER (SCL = 1.0/(NP-1))

      INTEGER   I, IM, IK, J, L, L1, L2, L3, STAT(NP*NP), STATUS
      REAL      ARRAY(NP,NP), CLEV(11), LTM(6)
      DOUBLE PRECISION X(M,NP,NP), WORLD(M,NP,NP)
      CHARACTER TEXT*80

      INCLUDE 'tab.inc'
      INTEGER   TAB(TABLEN)
*-----------------------------------------------------------------------

      WRITE (*, 10)
 10   FORMAT (
     :   'Testing WCSLIB coordinate lookup table routines (ttab2.f)',/,
     :   '---------------------------------------------------------',/)


*     PGPLOT initialization.
      CALL PGBEG (0, '/xwindow', 1, 1)
      CALL PGVSTD ()
      CALL PGSCH (0.7)

      LTM(1) = -SCL
      LTM(2) =  SCL
      LTM(3) =  0.0
      LTM(4) = -SCL
      LTM(5) =  0.0
      LTM(6) =  SCL

*     The viewport is slightly oversized.
      CALL PGWNAD (-0.1, 1.1, -0.1, 1.1)

      DO 20 L = 1, 11
         CLEV(L) = 0.1*(L-1)
 20   CONTINUE


*     Set up the lookup table.
      STATUS = TABPUT (TAB, TAB_FLAG, -1, 0, 0)
      STATUS = TABINI(M, K, TAB)
      IF (STATUS.NE.0) THEN
         WRITE (*, 30) STATUS
 30      FORMAT ('TABINI ERROR',I2,'.')
         GO TO 999
      END IF

      STATUS = TABPUT (TAB, TAB_M, M, 0, 0)
      DO 50 IM = 1, M
         STATUS = TABPUT (TAB, TAB_K,     K(IM),     IM, 0)
         STATUS = TABPUT (TAB, TAB_MAP,   MAP(IM),   IM, 0)
         STATUS = TABPUT (TAB, TAB_CRVAL, CRVAL(IM), IM, 0)

         DO 40 IK = 1, K(IM)
            STATUS = TABPUT (TAB, TAB_INDEX, DBLE(IK-1), IM, IK)
 40      CONTINUE
 50   CONTINUE

*     Subdivide the interpolation element.
      DO 70 I = 1, NP
         DO 60 J = 1, NP
            X(1,J,I) = (J-1)*(K1-1.0)/(NP-1.0) - CRVAL(1)
            X(2,J,I) = (I-1)*(K2-1.0)/(NP-1.0) - CRVAL(2)
 60      CONTINUE
 70   CONTINUE

*     The first coordinate element is static.
      STATUS = TABPUT (TAB, TAB_COORD, 0D0, 1, 0)
      STATUS = TABPUT (TAB, TAB_COORD, 0D0, 3, 0)
      STATUS = TABPUT (TAB, TAB_COORD, 0D0, 5, 0)
      STATUS = TABPUT (TAB, TAB_COORD, 0D0, 7, 0)

*     (k1,k2) = (1,1).
      STATUS = TABPUT (TAB, TAB_COORD, 0D0, 2, 0)

*     The second coordinate element varies in three of the corners.
      DO 170 L3 = 0, 100, 20
*        (k1,k2) = (2,2).
         STATUS = TABPUT (TAB, TAB_COORD, 0.01D0*L3, 8, 0)

         DO 160 L1 = 0, 100, 20
*           (k1,k2) = (1,2).
            STATUS = TABPUT (TAB, TAB_COORD, 0.01D0*L1, 4, 0)

            CALL PGPAGE ()
            DO 150 L2 = 0, 100, 2
*              (k1,k2) = (2,1).
               STATUS = TABPUT (TAB, TAB_COORD, 0.01D0*L2, 6, 0)

*              Compute coordinates within the interpolation element.
               STATUS = TABX2S (TAB, NP*NP, 2, X, WORLD, STAT)
               IF (STATUS.NE.0) THEN
                  WRITE (*, 80) STATUS
 80               FORMAT ('TABX2S ERROR',I2,'.')
               END IF

*              Start a new plot.
               CALL PGBBUF ()
               CALL PGERAS ()
               CALL PGSCI (1)
               CALL PGBOX ('BCNST', 0.0, 0, 'BCNSTV', 0.0, 0)

*              Draw the boundary of the interpolation element in red.
               CALL PGSCI (2)
               CALL PGMOVE (0.0, 0.0)
               CALL PGDRAW (1.0, 0.0)
               CALL PGDRAW (1.0, 1.0)
               CALL PGDRAW (0.0, 1.0)
               CALL PGDRAW (0.0, 0.0)

*              Label the value of the coordinate element in each corner.
               WRITE (TEXT, '(F3.1)') 0.0
               CALL PGTEXT (-0.05, -0.03, TEXT)
               WRITE (TEXT, '(F3.1)') 0.01*L1
               CALL PGTEXT ( 1.01, -0.03, TEXT)
               WRITE (TEXT, '(F4.2)') 0.01*L2
               CALL PGTEXT (-0.05,  1.01, TEXT)
               WRITE (TEXT, '(F3.1)') 0.01*L3
               CALL PGTEXT ( 1.01,  1.01, TEXT)

               CALL PGSCI (5)
*              Contour labelling: bottom.
               DO 90 L = 10, L1-1, 10
                  WRITE (TEXT, '(F3.1)') L*0.01
                  CALL PGTEXT ((FLOAT(L))/L1 - 0.02, -0.03, TEXT)
 90            CONTINUE

*              Contour labelling: left.
               DO 100 L = 10, L2-1, 10
                  WRITE (TEXT, '(F3.1)') L*0.01
                  CALL PGTEXT (-0.05, (FLOAT(L))/L2 - 0.01, TEXT)
 100           CONTINUE

*              Contour labelling: right.
               DO 110 L = 10, 99, 10
                  IF (L.LE.L1 .AND. L.LE.L3) GO TO 110
                  IF (L.GE.L1 .AND. L.GE.L3) GO TO 110
                  WRITE (TEXT, '(F3.1)') L*0.01
                  CALL PGTEXT (1.01, (FLOAT(L-L1))/(L3-L1) - 0.01, TEXT)
 110           CONTINUE

*              Contour labelling: top.
               DO 120 L = 10, 99, 10
                  IF (L.LE.L2 .AND. L.LE.L3) GO TO 120
                  IF (L.GE.L2 .AND. L.GE.L3) GO TO 120
                  WRITE (TEXT, '(F3.1)') L*0.01
                  CALL PGTEXT ((FLOAT(L-L2))/(L3-L2) - 0.02, 1.01, TEXT)
 120           CONTINUE

*              Draw contours for the second coordinate element.
               DO 140 I = 1, NP
                  DO 130 J = 1, NP
                     ARRAY(J,I) = WORLD(2,J,I)
 130              CONTINUE
 140           CONTINUE

               IF (IM.NE.0) THEN
                  CALL PGSCI (5)
               ELSE
                  CALL PGSCI (1)
               END IF

               CALL PGCONT (ARRAY, NP, NP, 1, NP, 1, NP, CLEV, 11, LTM)

               CALL PGEBUF ()
 150        CONTINUE
 160     CONTINUE
 170  CONTINUE

      CALL PGEND ()

 999  STATUS = TABFREE (TAB)

      END
