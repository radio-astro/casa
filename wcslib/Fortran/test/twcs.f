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
* $Id: twcs.f,v 4.7 2011/02/07 07:03:43 cal103 Exp $
*=======================================================================

      PROGRAM TWCS
*-----------------------------------------------------------------------
*
* TWCS1 tests WCSS2P and WCSP2S for closure on an oblique 2-D slice
* through a 4-D image with celestial, spectral and logarithmic
* coordinate axes.
*
*-----------------------------------------------------------------------
      DOUBLE PRECISION TOL
      PARAMETER (TOL = 1D-10)

*     Number of axes.
      INTEGER   N
      PARAMETER (N = 4)

      INTEGER   I, J, K, NAXIS, NPV, PVI(3), PVM(3)
      DOUBLE PRECISION CDELT(N), CRPIX(N), CRVAL(N), LATPOLE, LONPOLE,
     :          PC(N,N), PV(3), RESTFRQ, RESTWAV
      CHARACTER CTYPE(N)*72

      COMMON /HEADER/ NAXIS, NPV, CRPIX, PC, CDELT, CRVAL, LONPOLE,
     :                LATPOLE, RESTFRQ, RESTWAV, PVI, PVM, PV
      COMMON /HEADCH/ CTYPE

      DATA NAXIS   /N/
      DATA (CRPIX(J), J=1,N)
     :             /513D0,   0D0,   0D0,   0D0/
      DATA ((PC(I,J),J=1,N),I=1,N)
     :             /1.1D0,   0D0,   0D0,   0D0,
     :                0D0, 1.0D0,   0D0, 0.1D0,
     :                0D0,   0D0, 1.0D0,   0D0,
     :                0D0, 0.2D0,   0D0, 1.0D0/
      DATA (CDELT(I), I=1,N)
     :             /-9.635265432D-6, 1D0, 0.1D0, -1D0/
      DATA (CTYPE(I), I=1,N)
     :           /'WAVE-F2W', 'XLAT-BON', 'TIME-LOG', 'XLON-BON'/
      DATA (CRVAL(I), I=1,N)
     :             /0.214982042D0, -30D0, 1D0, 150D0/
      DATA LONPOLE /150D0/
      DATA LATPOLE /999D0/
      DATA RESTFRQ /1.42040575D9/
      DATA RESTWAV /0D0/

*     Set PVi_m keyvalues for the longitude axis (I = 4).  For test
*     purposes, these are set so that the fiducial native coordinates
*     are at the native pole, i.e. so that (phi0,theta0) = (0,90), but
*     without any fiducial offset, i.e. iwith PVi_0a == 0 (by default).
      DATA NPV     /3/
      DATA (PVI(K), PVM(K), PV(K), K=1,2)
     :             /4, 1,  0D0,
     :              4, 2, 90D0/

*     PVi_m keyvalues for the latitude axis (I = 2).
      DATA PVI(3), PVM(3), PV(3)
     :             /2, 1, -30D0/

      INTEGER   NELEM
      PARAMETER (NELEM = 9)

      INTEGER   LAT, LATIDX, LNG, LNGIDX, SPCIDX, STAT(0:360), STATUS
      DOUBLE PRECISION FREQ, IMG(NELEM,0:360), LAT1, LNG1, PHI(0:360),
     :          PIXEL1(NELEM,0:360), PIXEL2(NELEM,0:360), R, RESID,
     :          RESMAX, THETA(0:360), TIME, WORLD1(NELEM,0:360),
     :          WORLD2(NELEM,0:360)

      INCLUDE 'wcs.inc'
      INCLUDE 'cel.inc'
      INCLUDE 'prj.inc'
      INTEGER   WCS(WCSLEN)
      DOUBLE PRECISION DUMMY
      EQUIVALENCE (WCS,DUMMY)
*-----------------------------------------------------------------------
      WRITE (*, 10)
 10   FORMAT ('Testing closure of WCSLIB world coordinate ',
     :        'transformation routines (twcs.f)',/,
     :        '-------------------------------------------',
     :        '--------------------------------')


*     This routine simulates the actions of a FITS header parser.
      STATUS = WCSPUT (WCS, WCS_FLAG, -1, 0, 0)
      CALL PARSER (WCS)

      WRITE (*, 20) TOL
 20   FORMAT (/,'Reporting tolerance',1PG8.1,' pixel.')


*     Get indices.
      STATUS = WCSGET (WCS, WCS_LNG,  LNGIDX)
      STATUS = WCSGET (WCS, WCS_LAT,  LATIDX)
      STATUS = WCSGET (WCS, WCS_SPEC, SPCIDX)

*     Initialize non-celestial world coordinates.
      TIME = 1D0
      FREQ = 1.42040595D9 - 180D0 * 62500D0
      DO 30 K = 0, 360
        WORLD1(1,K) = 0D0
        WORLD1(2,K) = 0D0
        WORLD1(3,K) = 0D0
        WORLD1(4,K) = 0D0

        WORLD1(3,K) = TIME
        TIME = 1.01D0 * TIME

        WORLD1(SPCIDX,K) = 2.99792458D8 / FREQ
        FREQ = FREQ + 62500D0
 30   CONTINUE

      RESMAX = 0D0
      DO 110 LAT = 90, -90, -1
        LAT1 = DBLE(LAT)

        K = 0
        DO 40 LNG = -180, 180
          LNG1 = DBLE(LNG)

          WORLD1(LNGIDX,K) = LNG1
          WORLD1(LATIDX,K) = LAT1
          K = K + 1
 40     CONTINUE

        STATUS = WCSS2P (WCS, 361, NELEM, WORLD1, PHI, THETA, IMG,
     :                   PIXEL1, STAT)
        IF (STATUS.NE.0) THEN
          WRITE (*, 50) STATUS, LAT1
 50       FORMAT (3X,'WCSS2P(1) ERROR',I3,' (LAT1 =',F20.15, ')')
          GO TO 110
        END IF

        STATUS = WCSP2S (WCS, 361, NELEM, PIXEL1, IMG, PHI, THETA,
     :                   WORLD2, STAT)
        IF (STATUS.NE.0) THEN
          WRITE (*, 60) STATUS, LAT1
 60       FORMAT (3X,'WCSP2S ERROR',I3,' (LAT1 =',F20.15, ')')
          GO TO 110
        END IF

        STATUS = WCSS2P (WCS, 361, NELEM, WORLD2, PHI, THETA, IMG,
     :                   PIXEL2, STAT)
        IF (STATUS.NE.0) THEN
          WRITE (*, 70) STATUS, LAT1
 70       FORMAT (3X,'WCSS2P(2) ERROR',I3,' (LAT1 =',F20.15, ')')
          GO TO 110
        END IF

        DO 100 K = 0, 360
          RESID = 0D0
          DO 80 I = 1, NAXIS
            R = PIXEL2(I,K) - PIXEL1(I,K)
            RESID = RESID + R*R
 80       CONTINUE

          RESID = SQRT(RESID)
          IF (RESID.GT.RESMAX) RESMAX = RESID

          IF (RESID.GT.TOL) THEN
            WRITE (*, 90) (WORLD1(I,K), I=1,NAXIS),
     :                    (PIXEL1(I,K), I=1,NAXIS),
     :                    (WORLD2(I,K), I=1,NAXIS),
     :                    (PIXEL2(I,K), I=1,NAXIS)
 90         FORMAT (/,'Closure error:',/,
     :                'world1:',4F18.12,/,
     :                'pixel1:',4F18.12,/,
     :                'world2:',4F18.12,/,
     :                'pixel2:',4F18.12)
          END IF

          LNG1 = LNG1 + 1D0
 100    CONTINUE
 110  CONTINUE

      WRITE (*, 120) RESMAX
 120  FORMAT ('Maximum closure residual:',1P,G11.3,' pixel.')

      STATUS = WCSFREE(WCS)

      END

*-----------------------------------------------------------------------
      SUBROUTINE PARSER (WCS)
*-----------------------------------------------------------------------
* In practice a parser would read the FITS header until it encountered
* the NAXIS keyword which must occur near the start, before any of the
* WCS keywords.  It would then use WCSINI to allocate memory for arrays
* in the WCSPRM "data structure" and set default values.
*
* In this simulation the header keyvalues are set in the main program in
* variables passed in COMMON.
*-----------------------------------------------------------------------
*     Number of axes.
      INTEGER   N
      PARAMETER (N = 4)

      INTEGER   I, J, K, NAXIS, NPV, PVI(3), PVM(3), STATUS, WCS(*)
      DOUBLE PRECISION CDELT(N), CRPIX(N), CRVAL(N), LATPOLE, LONPOLE,
     :          PC(N,N), PV(3), RESTFRQ, RESTWAV
      CHARACTER CTYPE(N)*72

      INCLUDE 'wcs.inc'

      COMMON /HEADER/ NAXIS, NPV, CRPIX, PC, CDELT, CRVAL, LONPOLE,
     :                LATPOLE, RESTFRQ, RESTWAV, PVI, PVM, PV
      COMMON /HEADCH/ CTYPE
*-----------------------------------------------------------------------
      STATUS = WCSINI (NAXIS, WCS)

      DO 20 I = 1, NAXIS
         STATUS = WCSPUT (WCS, WCS_CRPIX, CRPIX(I), I, 0)

         DO 10 J = 1, NAXIS
            STATUS = WCSPUT (WCS, WCS_PC, PC(I,J), I, J)
 10      CONTINUE

         STATUS = WCSPUT (WCS, WCS_CDELT, CDELT(I), I, 0)
         STATUS = WCSPUT (WCS, WCS_CTYPE, CTYPE(I), I, 0)
         STATUS = WCSPUT (WCS, WCS_CRVAL, CRVAL(I), I, 0)
 20   CONTINUE

      STATUS = WCSPUT (WCS, WCS_LONPOLE, LONPOLE, 0, 0)
      STATUS = WCSPUT (WCS, WCS_LATPOLE, LATPOLE, 0, 0)

      STATUS = WCSPUT (WCS, WCS_RESTFRQ, RESTFRQ, 0, 0)
      STATUS = WCSPUT (WCS, WCS_RESTWAV, RESTWAV, 0, 0)

      DO 30 K = 1, NPV
         STATUS = WCSPUT (WCS, WCS_PV, PV(K), PVI(K), PVM(K))
 30   CONTINUE

*     Extract information from the FITS header.
      STATUS = WCSSET (WCS)
      IF (STATUS.NE.0) THEN
         WRITE (*, 40) STATUS
 40      FORMAT ('WCSSET ERROR',I3)
      END IF

      RETURN
      END
