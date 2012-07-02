C*** issues: what to do with pim.blckdt?

C  FIGRFLOAD
C  Wrapper around IGRF initialization
C  Sets up various common-block variables.
C  This is derived from a section of Bob Campbell's pimvlbi4.f code,
C  at the start of the PIMVLBI program. 
C
C*** Modifications to pimvlbi4.f code in this subroutine:
C*** 1. Useless variables removed
C*** 2. Call to set_up_vlbi removed, since data paths are now
C***    provided from the outside (and since source, station and GPS
C***    files are no longer read).
C
      SUBROUTINE figrfload (yearigrf)
      IMPLICIT NONE
	    INTEGER nmax
	    REAL umr,era,aquad,bquad,time,gh1(144)
      CHARACTER*80 pigrf
      COMMON /igrfstuff/ umr, era, aquad, bquad, nmax, time, gh1
      COMMON /igrfwrapper/ pigrf
      REAL yearigrf,erequ,erpol
      REAL*8 pi,degrad,dimo
      PARAMETER (pi = 3.14159265358979323846d0)
      PARAMETER (degrad = pi / 1.8d2)

      umr = SNGL(degrad)
      era = 6371.2
      erequ = 6378.16
      erpol = 6356.775
      aquad = erequ * erequ
      bquad = erpol * erpol

      CALL feldcof (yearigrf, pigrf, dimo)

      RETURN
      END
