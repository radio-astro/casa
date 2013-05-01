C   LAWRAP.f: AIPS++ FORTRAN wrappers for LAPACK
C   Copyright (C) 1994
C   Associated Universities, Inc. Washington DC, USA.
C
C   This library is free software; you can redistribute it and/or modify it
C   under the terms of the GNU Library General Public License as published by
C   the Free Software Foundation; either version 2 of the License, or (at your
C   option) any later version.
C
C   This library is distributed in the hope that it will be useful, but WITHOUT
C   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
C   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
C   License for more details.
C
C   You should have received a copy of the GNU Library General Public License
C   along with this library; if not, write to the Free Software Foundation,
C   Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
C
C   Correspondence concerning AIPS++ should be addressed as follows:
C          Internet email: aips2-request@nrao.edu.
C          Postal address: AIPS++ Project Office
C                          National Radio Astronomy Observatory
C                          520 Edgemont Road
C                          Charlottesville, VA 22903-2475 USA
C
C   $Id$

C     *******************************************************************
      SUBROUTINE SSOLVX(N, NRHS, A, LDA, AF, LDAF, IPIV, B, LDB, X, LDX,
     *                  FERR, BERR, INFO, R, C, WORK, IWORK)
C     *******************************************************************
      IMPLICIT NONE
 
      INTEGER N, NRHS, LDA, LDAF, LDB, LDX, INFO, IPIV(N), IWORK(N)
      REAL A(LDA,N), AF(LDAF,N), B(LDB,NRHS), X(LDX,NRHS), FERR(NRHS),
     *     BERR(NRHS), WORK(3*N), R(N), C(N)

C     Internal Variables
      REAL RCOND
      CHARACTER*1 EQUED
      EXTERNAL SGESVX
      INTRINSIC ISIGN

      EQUED = 'N'

      CALL SGESVX('F', 'N', N, NRHS, A, LDA, AF, LDAF, IPIV, EQUED, 
     *            R, C, B, LDB, X, LDX, RCOND, FERR, BERR, 
     *            WORK, IWORK, INFO)

C     Test for "N = no equilibration" => A is already factored.
      IF(EQUED.NE.'N') THEN
         INFO = INFO + ISIGN(20000,INFO)
      ENDIF

C     Test for reciprocal condition of A
      IF((RCOND.EQ.0).AND.(INFO.EQ.N+1)) THEN 
         INFO = INFO + ISIGN(10000,INFO)
      ENDIF
      
      RETURN
      END

      SUBROUTINE DSOLVX(N, NRHS, A, LDA, AF, LDAF, IPIV, B, LDB, X, LDX,
     *                  FERR, BERR, INFO, R, C, WORK, IWORK)
C     ****************************************************************
      IMPLICIT NONE

      INTEGER N, NRHS, LDA, LDAF, LDB, LDX, INFO, IPIV(N), IWORK(N)
      DOUBLE PRECISION A(LDA,N), AF(LDAF,N), B(LDB,NRHS), X(LDX,NRHS), 
     *                 FERR(NRHS), BERR(NRHS), WORK(3*N), R(N), C(N)

C     Internal Variables
      DOUBLE PRECISION RCOND
      CHARACTER*1 EQUED
      EXTERNAL DGESVX
      INTRINSIC ISIGN

      EQUED = 'N'

      CALL DGESVX('F', 'N', N, NRHS, A, LDA, AF, LDAF, IPIV, EQUED, 
     *            R, C, B, LDB, X, LDX, RCOND, FERR, BERR, 
     *            WORK, IWORK, INFO)

C     Test for "N = no equilibration" => A is already factored.
      IF(EQUED.NE.'N') THEN
         INFO = INFO + ISIGN(20000,INFO)
      ENDIF

C     Test for reciprocal condition of A
      IF((RCOND.EQ.0).AND.(INFO.EQ.N+1)) THEN 
         INFO = INFO + ISIGN(10000,INFO)
      ENDIF
      
      RETURN
      END


      SUBROUTINE CSOLVX(N, NRHS, A, LDA, AF, LDAF, IPIV, B, LDB, X, LDX,
     *                  FERR, BERR, INFO, R, C, WORK, RWORK)
C     *****************************************************************
      IMPLICIT NONE

      INTEGER N, NRHS, LDA, LDAF, LDB, LDX, INFO, IPIV(N)
      COMPLEX A(LDA,N), AF(LDAF,N), B(LDB,NRHS), X(LDX,NRHS), WORK(2*N)
      REAL FERR(NRHS), BERR(NRHS), RWORK(N), R(N), C(N)

C     Internal Variables
      REAL RCOND
      CHARACTER*1 EQUED
      EXTERNAL CGESVX
      INTRINSIC ISIGN

      EQUED = 'N'

      CALL CGESVX('F', 'N', N, NRHS, A, LDA, AF, LDAF, IPIV, EQUED, 
     *            R, C, B, LDB, X, LDX, RCOND, FERR, BERR, 
     *            WORK, RWORK, INFO)

C     Test for "N = no equilibration" => A is already factored.
      IF(EQUED.NE.'N') THEN
         INFO = INFO + ISIGN(20000,INFO)
      ENDIF

C     Test for reciprocal condition of A
      IF((RCOND.EQ.0).AND.(INFO.EQ.N+1)) THEN 
         INFO = INFO + ISIGN(10000,INFO)
      ENDIF
      
      RETURN
      END


      SUBROUTINE ZSOLVX(N, NRHS, A, LDA, AF, LDAF, IPIV, B, LDB, X, LDX,
     *                  FERR, BERR, INFO, R, C, WORK, RWORK)
C     ****************************************************************
      IMPLICIT NONE

      INTEGER N, NRHS, LDA, LDAF, LDB, LDX, INFO, IPIV(N)
      COMPLEX*16 A(LDA,N), AF(LDAF,N), B(LDB,NRHS), X(LDX,NRHS),
     *           WORK(2*N)    
      DOUBLE PRECISION FERR(NRHS), BERR(NRHS), RWORK(N), R(N), C(N)

C     Internal Variables
      CHARACTER*1 EQUED
      DOUBLE PRECISION RCOND
      EXTERNAL ZGESVX
      INTRINSIC ISIGN

      EQUED = 'N'

      CALL ZGESVX('F', 'N', N, NRHS, A, LDA, AF, LDAF, IPIV, EQUED, 
     *            R, C, B, LDB, X, LDX, RCOND, FERR, BERR, 
     *            WORK, RWORK, INFO)

C     Test for "N = no equilibration" => A is already factored.
      IF(EQUED.NE.'N') THEN
         INFO = INFO + ISIGN(20000,INFO)
      ENDIF

C     Test for reciprocal condition of A
      IF((RCOND.EQ.0).AND.(INFO.EQ.N+1)) THEN 
         INFO = INFO + ISIGN(10000,INFO)
      ENDIF
      
      RETURN
      END

C**************************************************************************
C   The following wrappers are for the test code
C**************************************************************************

      SUBROUTINE SBLDA( M, N, COND, DMAX, KL, KU, A, INFO )
*
*  -- Aips++ wrapper for LAPACK test routine.
*
*     .. Scalar Arguments ..
      INTEGER            INFO, KL, KU, M, N
      REAL               COND, DMAX
*     ..
*     .. Array Arguments ..
      REAL               A( M, * )
*     ..
*  Purpose 
*  =======
*  SBLDA creates a random matrix fitting the requirements specified by
*  the arguments, where appropriate.   
*  =======================================================================
*
*     .. Parameters ..
      INTEGER            NMAX
      PARAMETER          ( NMAX = 16 )
      INTEGER            MAXRHS
      PARAMETER          ( MAXRHS = 2 )
*     ..
*     Local Scalars ..
      INTEGER            LDA
*     ..
*     .. Local Arrays ..
      REAL               D(NMAX), WORK( NMAX, NMAX+MAXRHS+5 )
      INTEGER            ISEED( 4 )
*     ..
*     .. External Subroutines ..
      EXTERNAL           SLATMS
*     ..
*     .. Data Statements ..
      DATA               ISEED / 1990, 1991, 1992, 1993 / 
*     ..
*     .. Executable Statements ..
*
      LDA = M
*
      CALL SLATMS( M, N, 'Symmetric', ISEED, 'Nonsymmetric', D, 3, 
     $     COND, DMAX, KL, KU, "No Packing", A, LDA, WORK, INFO )
*
      RETURN
*
*     End of SBLDA
*
      END

      SUBROUTINE CNDNM(NMFLG, N, A, LDA, ANORM, RCOND, INFO)
*
*  -- Aips++ wrapper for LAPACK test routine.
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, N, NMFLG
      REAL               ANORM, RCOND
*     ..
*     .. Array Arguments ..
      REAL               A(LDA, *)
*     ..
*  Purpose
*  =======
*
*  CNDNM estimates the reciprocal of the condition number of a general
*  real matrix A which contains the Upper and Lower factors returned
*  by SGETRF.
*  ====================================================================
*
*     .. Local Arrays ..
      CHARACTER          NORM(2)
      INTEGER            IWORK(17)
      REAL               WORK(68)
*     ..
*     .. External Subroutines ..
      EXTERNAL           SGECON
*     ..
*     .. Data Statements ..
      DATA               NORM / 'O', 'I' /
*     ..
*     .. Executable Statements ..
*
      CALL SGECON( NORM(NMFLG), N, A, LDA, ANORM, RCOND, WORK, 
     $                   IWORK, INFO )

      RETURN
*
*     End of CNDNM
*
      END
