C  Include PIM common-block init

      INCLUDE 'pim.blckdt'

C  FPIMINIT
C  Wrapper around PIM initialization. 
C  Sets up various common-block variables, including datafile paths.
C  This is derived from a section of Bob Campbell's pimvlbi4.f code,
C  at the start of the PIMVLBI program. 
C
C*** Modifications to pimvlbi4.f code in this subroutine:
C*** 1. Useless variables removed
C*** 2. Call to set_up_vlbi removed, since data paths are now
C***    provided from the outside (and since source, station and GPS
C***    files are no longer read).
C
      SUBROUTINE fpiminit 
      IMPLICIT NONE

      INCLUDE 'pimwrapper.inc'
      INCLUDE 'dpath.inc'
      INCLUDE 'indirect.inc'
      INCLUDE 'logicuni.inc'
      INCLUDE 'grid.inc'
      INCLUDE 'ssnstuff.inc'

C      REAL umr,era,aquad,bquad,time,gh1(144)
C      CHARACTER fil1*12
C      INTEGER nmax
      INTEGER llfsct
      
C      COMMON /igrfstuff/ umr, era, aquad, bquad, nmax, time, gh1
C      COMMON /igrfstuff1/ fil1

C      umr = SNGL(degrad)

C  First, those constant PIM parameters (throughout all epoch, source
C     station loops)
C    Most of the variables in the COMMON block GRID as definded in
C    BLOCK DATA ITERATS are okay as they are (since not used anyway)
C
      usessn = 1
      ursisw = .FALSE.
      eswitch = .FALSE.
      geog = .TRUE.
      plasph = .TRUE.
      llfsct = 0
      CALL SETSCT(llfsct) 
      
C*** setup data paths
C*** original code was:
C***     CALL set_up_vlbi (lucgdb, pkpf, pimf, pigrf, psrc, psta, prtd)
C***      ...which reads path_nam_vlbi4.txt to get the database locations.
C*** Instead we just set up the paths in C code outside the wrapper.
      jopen=1
      CALL RDCGDB(pgdb,LUCGDB)
      RETURN
      END

C  FPIMSETALT
C  Sets up the altitude sampling grid, by copying it into the ZOUT array
C  (inside common block GRID).
C  This routine coded from scratch; original PIMVLBI4 has ZOUT filled by 
C  a hardwired DATA declaration (pimvlbi4.f).
C
C  Inputs:  REAL ZGRID(NZ)    array of altitudes
C           INTEGER NZ        size of array
C
      SUBROUTINE fpimsetalt(zgrid,nz)
      IMPLICIT NONE
      INTEGER i,nz
      REAL    zgrid(nz)
      INCLUDE 'grid.inc'
      
      nalt=nz
C      write(*,'(a,$)') 'fpimsetalt: '
      DO 120 i = 1, nalt
        zout(i)=zgrid(i)
C        write(*,'(F," ",$)') zout(i)
 120  CONTINUE
C      write(*,'(a)') ' '
      RETURN
      END
  
 
C  FPIMPRELOAD
C  Wrapper for PIM pre-computation initialization. 
C  This has eventually been pared down to just the N0DAY81 setting.
C
      SUBROUTINE fpimpreload()
      
      IMPLICIT NONE
      INCLUDE 'pimwrapper.inc'
      
      n0day81=-1
      
      RETURN
      END

C  FPIMLOAD
C  Wrapper for PIM time-slot initialization. 
C  Loads up PIM for a specific time slot.
C  This is derived from a section of Bob Campbell's pimvlbi4.f code,
C  starting with the top of the time loop ("i=1,nut"), up to the the stations
C  loop ("jsta=1,nsta").
C  
C  Purpose: performs a bunch of setup stuff, then calls PIM's READ_DBASES.
C  
C  Inputs:  INTEGER YRDYV         date: year and day, in YYDDD format
C           REAL*8  UTV           U.T., in hours 
C
C  Outputs: INTEGER YEAR,DAY      date, broken down into YYYY and DDD
C          
      SUBROUTINE fpimload (yrdyv,year,day,utv,sf107,sap)
      IMPLICIT NONE
      INCLUDE 'pimwrapper.inc'
C   Include some of PIM's common blocks
      INCLUDE 'dpath.inc'
      INCLUDE 'indirect.inc'
      INCLUDE 'ssnstuff.inc'
      
      INTEGER yrdyv,year,day
      REAL*8  utv
      REAL*8  sf107(*),sap(*)
      
      INTEGER nday81,nleapd,pyr,pdy,phr,pmin
      INTEGER i,month,idom,mday,mhr,iap
      REAL*8  psec,fut,x,x0,f107dp,ssndp,apdp(2),kpsp(2),utdp
      REAL    uthr,ut
      REAL*8  tmp

C*** Modifications to pimvlbi4.f code in this subroutine:
C*** 1. yrdyv(i),utv(i) replaced by yrdyv,utv. PIMVLBI uses the yrdyv and
C***    utv arrays to hold time slots, and has an outer loop iterate through
C***    them. Iteration over time is now handled by the IonosphModelPIM class,
C***    so this section of code is called for just one particular time slot.
C*** 2. "if( n0day81 .EQ. -1 )" replaces "if( i .EQ. 1)", since we no longer
C***    have a time loop in here. n0day81==-1 is the initial condition, set
C***    in fpimpreload() above.
C*** 3. The loop "llo=1,nllo" has been thrown out (had to do with delay rate).
C*** 4. The loop "lkp=1,nkploop" has also been thrown out. IMF-based
C***    uncertainties are now ignored...
C***    Note to myself: gotta clear this up w/Bob.

      year = yrdyv/1000
      day = yrdyv - year*1000
      IF (year .LT. 81) THEN
         year = year + 2000
      ELSE
         year = year + 1900
      END IF
      nday81 = (year-1981)*365 + day
      nleapd = (year-1981)/4
      nday81 = nday81 + nleapd
      IF (n0day81 .EQ. -1) n0day81 = nday81

C   Calculate the output time fields
C
      pyr = year
      pdy = day  
      phr = IDINT(utv)
      tmp = (utv - DBLE(phr))*6.d1
      pmin = IDINT(tmp)
      psec = (tmp - DBLE(pmin))*6.d1
      IF (psec .GT. (6.d1-1.d-9)) THEN
         pmin = pmin + 1
         psec = 0.d0
      END IF

C  First, derive the parabolically (2nd-order Lagrangian) interpolated
C   geophysical parameters (assume K_p measured at midpoint of their
C   3-hr intervals; time of F10.7 measurement adjusted for move from
C   Ottawa to Penticton via FUT)
C
      fut = 2.d1
      IF ((yrdyv .GE. 81000) .AND. (yrdyv .LE. 91181)) fut=1.7d1
      x = (utv-fut) / 2.4d1
      mday = nday81 - n0day81 + 2
      f107dp = x*(x-een)*sf107(mday-1)/twee - (x*x - een)*sf107(mday) 
     1        + x*(x+een)*sf107(mday+1)/twee
      ssndp = DMAX1(nul, DSQRT(93918.4d0 + 1117.3d0*f107dp)-406.37d0)

      mhr = IDINT(utv)/3 + 1
      x0 = 3.d0*DBLE(mhr) - 1.5d0
      x = (utv-x0) / 3.d0
      iap = (mday-1)*8 + mhr
      apdp(1) = x*(x-een)*sap(iap-1)/twee - (x*x - een)*sap(iap) +
     1         x*(x+een)*sap(iap+1)/twee
      kpsp(1) = ap_to_kp(SNGL(apdp(1)))


C  Now, adjust the geophysical parameters according to specified method
C   of treating the IMF data  (IMFNA=0 --> no change)
C
      IF (imfna .EQ. 1) THEN
         apdp(1) = 4.d0
         kpsp(1) = 1.0
      ELSE IF (imfna .EQ. 2) THEN
         apdp(2) = 4.d0
         kpsp(2) = 1.0
      END IF
      
      utdp=utv

C  Now ready to call PIM databases:  get all the COMMON-block PIM
C   variables/parameters into their proper names/TYPES
C
      CALL TIMMDM (year, day, month, idom)
      uthr = SNGL(utdp)
      ut = uthr * 3.6e3
      rf10p7 = SNGL(f107dp)
      f10p7 = rf10p7
      ssn = SNGL(ssndp)
      ap = SNGL(apdp(1))
      rkp = kpsp(1)
      ekp = MIN(rkp,6.5)
      ekp = MAX(ekp,0.0)
      DO 245 i=0,2
         kp(i) = ekp
245   CONTINUE

C  Call the PIM databases for this time & these geophysical parameters
C
      CALL INITPR
      CALL READ_DBASES (day,month, uthr, pusu,pmid,plow,plme,
     1                 paws, jopen)

      RETURN
      END

C------------------------------------------------------------------------  
C  FSLANTEDP
C  Wrapper around Bob Campbell's SLANTEDP subroutine.
C  Stores the input parameters into common blocks, and bounces to SLANTEDP.
C  Derived from pimvlbi4.f code (see the call to SLANTEDP), with trivial
C  modifications.
C  
C  Inputs:  INTEGER YEAR,DAY      date: year and day-of-year
C           REAL*8  UT            U.T., in hours 
C                   AZ,EL         line of sight (to source)
C                   LATV,LONV     coordinates of station
C           CHARACTER*1 STA       station and source IDs, for purposes
C                    *8 SRC          of debug output
C           INTEGER MAXALTPT      size of output arrays (below)
C
C  Outputs: REAL    ED()          E.D. values along slant              
C                   RNG()         range to each sample point
C                   LOSLAT(),
C                   LOSLON()      coordinates of each sample point
C
C  Note that the sampling grid is ultimately determined by a set of 
C  altitudes, given by ZOUT array inside the GRID common block. In the 
C  original PIMVLBI, ZOUT is a hardwired DATA declaration. 
C  We now setup ZOUT inside fpimpreload().
C
      SUBROUTINE fslantedp (year,day,ut8,az,el,latv,lonv,
     1                       naltpt,ed,rng,loslat,loslon)
      IMPLICIT NONE

C  Common block for PIM parameters:
      INCLUDE 'grid.inc'

      INTEGER year,day,naltpt,i
      REAL*8  ut8
      REAL az,el,latv,lonv,ut
      REAL ed(naltpt),rng(naltpt)
      REAL loslat(naltpt),loslon(naltpt)
      CHARACTER sta*1, src*8

      REAL edmax,rngmax
      INTEGER ldbg
      LOGICAL dbgflg
      
      dbgflg=.FALSE.
      ldbg=0
      sta='1'
      src='12345678'

C      Get all COMMON-block PIM variables/parameters into their proper
C       names/TYPES

      saz = az
      sel = el
      obslat = latv
      obslon = lonv
      ut = SNGL(ut8)*3600.
      
      
C      write(*,'(a,2i,5f)') 'fslantedp in: ',year,day,ut,obslon,obslat,
C     1     saz,sel

C       Get the electron densities along the LoS
C
      CALL slantedp (year,day,ut,dbgflg,ldbg,sta,src,
     1               ed,rng,edmax,rngmax,
     2               loslat,loslon)

C      write(*,'(a,$)') 'fslantedp out: '
      
C      do 200 i=1,rngmax
C        write(*,'(2f,$)') loslat(i),loslon(i)
C200   continue
C      write(*,'(a)') ' '
      

      RETURN
      END

C Stupid g77 compiler insists on adding TWO underscores after a function 
C name, rather than one. So the symbols for FORTRAN functions 
C look different on different platforms; calling them from C++ is major
C pain in the butt. Hence I'd rather provide wrapper subroutines:
      SUBROUTINE faptokp (ap,kp)
      REAL ap,kp,ap_to_kp
      kp = ap_to_kp(ap)
      RETURN
      END
