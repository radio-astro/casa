      SUBROUTINE read_kpf107 (pkpf,maxdays, yrdy0,yrdy1,
     1                          ndays,sf107,sap)

C  Subroutine to read F10.7 and Ap from datafiles downloaded from NOAA
C    First attempt is to read from final database, maintained locally
C      from the final-processed data (about 1-2 month lag)
C    Second, from preliminary data for the immediate past
C    Finally, from forecase data for the near future (~30d)
C  This subroutine only reads the data in from the file (so the database
C    only has to opened once), and then does the 27-d averaging for 
C    F10.7 and the 24-hr averaging for Ap
C
C  The Ap data read from file are stored in 1-D arrays of length
C    8*(Ndays+3), rather than 2-D arrays (Ndays+3,8), to make forming 
C    the running 24-hr averages more straightforward
C
C  The returned arrays span:
C    SF107:  The 27-d averaged F10.7 starting at 20UT (17UT .LT. Jun91)
C             of the UT day before the 1st UT day having any data,
C             and continuing to 20UT of the UT day after the last day
C             having any data.
C    SAP:  The 24-hr averaged ap, sampled every 3 hours, starting with
C           the 00-03UT sample on the UT day before the 1st UT day
C           having any data and continuing to the 21-24UT sample on the
C           UT day after the last day having any data.
C
      INTEGER maxdays,istat, yrdy0,yrdy1, l,m,isamp
      INTEGER iyr0,idoy0,month0,idom0, iyr1,idoy1,month1,idom1
      INTEGER irec,nleapd,irec1,ndays, f107flg,kyr,kmo,kdy
      INTEGER rawap((14+3)*8)
      REAL*8 rawf(14+28), sf107(maxdays+2)
      REAL*8 sap((maxdays+2)*8),s0
      CHARACTER pkpf*80, flin*80, dumm*1
C---------------
C
C  MAXDAY - Dimension-size of number of days in data/simul duration 
C  YRDY0,YRDY1 - 1st & last days of data
C  NDAYS - Number of UT days with any data
C  IREC - #days since 0Jan81 == record number in KPF107.DAT
C  RAWF - array of daily F10.7 read from file - explicitly DIMENSIONED
C           because it's not a passed array: assmume MAXDAYS=14
C  SF107 - array of 27-d averaged F10.7
C  RAWAP - array of 3-hrly Ap read from file, stored as 1-D vector
C            explicitly DIMENSIONED as was RAWF
C  SAP - array of 24-hr averaged 3-hrly Ap
C
C---------------
C
C  Parse 1st & last YRDYs
C
      iyr0 = yrdy0/1000
      iyr1 = yrdy1/1000
      idoy0 = yrdy0 - iyr0*1000
      idoy1 = yrdy1 - iyr1*1000
      IF (iyr0 .LT. 81) THEN
         iyr0 = iyr0 + 2000
      ELSE
         iyr0 = iyr0 + 1900
      END IF
      IF (iyr1 .LT. 81) THEN
         iyr1 = iyr1 + 2000
      ELSE
         iyr1 = iyr1 + 1900
      END IF
      CALL TIMMDM(iyr0, idoy0, month0, idom0)
      CALL TIMMDM(iyr1, idoy1, month1, idom1)

C  Calculate record number in KPF107.DAT for day of 1st data,
C    adjusted for leap years (using IDOY takes care of leap-days in
C    the year IYR0)
C
      irec = (iyr0-1981)*365 + idoy0
      nleapd = (iyr0-1981)/4
      irec = irec + nleapd
      irec1 = (iyr1-1981)*365 + idoy1
      nleapd = (iyr1-1981)/4
      irec1 = irec1 + nleapd
      ndays = irec1 - irec + 1

      IF (irec .LT. 28) THEN
         WRITE (*,*) 'Too close to 0 Jan 1981: can''t compute F10.7'
         CALL A2STOP('')
      END IF
      IF (ndays .GT. 14) THEN
         WRITE (*,*) 'Currently limited to 2 weeks of data'
         CALL A2STOP('')
      END IF

      CALL STRCCT (pkpf,'kpf107.dat',32,flin,istat)
      OPEN (64, FILE=flin, STATUS='old')

C  Position file to the day before the first one contributing to 
C   the 27d averages to parabolically interpolate
C
      DO 105 l = 1, (irec-28)
 105     READ (64,'(a1)') dumm

C  Accumulate F10.7s to average, check FLUX QUALIFIERs & dates
C
      DO 120 l = 1, (28+ndays)
         READ (64,119,END=150)kyr,kmo,kdy,rawf(l),f107flg
         IF (f107flg .NE. 0) WRITE (*,118) kyr,kmo,kdy, f107flg
         IF (l .EQ. 28) THEN
            IF ((kmo .NE. month0) .OR. (kdy .NE. idom0)) THEN
               WRITE (*,117) kyr,kmo,kdy
            END IF
         END IF
 120  CONTINUE
      GOTO 131
 119  FORMAT (3i2, 25x,27x, 7x,f5.1,i1)
 118  FORMAT ('Caut: Non-zero F10.7 Flag at ',3(i2.2,1x),' : ',i1)
 117  FORMAT (5x,'Mismatch reading day for F10.7:  ',i2,'-',i2.2,
     1                '-',i2.2)

C  Desired data is later than end of "final-processed" database, 
C    need to check "preliminary" file and/or "forecast" file
C
 150  WRITE (*,'(/,a)') '*** Requested dates past end of KPF107.DAT ***'
      WRITE (*,'(a)') '    Preliminary and/or Forecast NOAA files should
     1be checked'
      WRITE (*,'(a)') '        ---  This not yet coded  ---'
      CALL A2STOP('')

C  Calculate the 27d averages to parabolically interpolate
C
 131  DO 130 l = 1, ndays+2
         sf107(l) = 0.0
         DO 135 m = 1, 27
            sf107(l) = sf107(l) + rawf(l+m-1)
 135     CONTINUE
         sf107(l) = sf107(l) / 27.0
 130  CONTINUE

         
C  Second, Kp:  obtain 24-hr averaged (8 of the 3-hr samples) &
C    parabolically interopolate -- 24-hr average chosen to reflect
C    time taken for magnetic changes to make themselves felt in
C    TECs in Midlat & Lowlat (not yet a well-defined relaxation
C    time - DTDecker).  Response times in Highlat (auroral regions)
C    is much quicker, so use of this average will not track rapid
C    e- density changes there.  And of course, PIM itself doesn't
C    attempt to model storms etal...
C
      REWIND (64)

C	Position file to the day before IREC; read in 3 days worth of ap
C
      DO 205 l = 1, (irec-3)
 205     READ (64, '(a1)') dumm

      DO 210 l = 1, ndays+3
         READ (64, 219) kyr, kmo, kdy, (rawap((l-1)*8+m),m=1,8)
         IF (l .EQ. 3) THEN
            IF ((kmo .NE. month0) .OR. (kdy .NE. idom0)) THEN
               WRITE (*,217) kyr,kmo,kdy
            END IF
         END IF
 210  CONTINUE
 219  FORMAT (3i2, 25x, 8i3)
 217  FORMAT (5x,'Mismatch reading day for Ap:  ',i2,'-',i2.2,
     1                '-',i2.2)

C  Calculate the 24hr averages to parabolically interpolate
C    do this by keeping track of 8-sample running average of MAGAP
C
      s0 = 0.d0
      DO 250 l=1,8
         s0 = s0 + DBLE(rawap(l))
 250  CONTINUE

      DO 270 isamp = 1, (ndays+2)*8
         s0 = s0 + DBLE(rawap(isamp+8)) - DBLE(rawap(isamp))
         sap(isamp) = s0 / 8.d0
 275  CONTINUE
 270  CONTINUE

      CLOSE (64)
      RETURN
      END

C==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==*==

      SUBROUTINE read_imf (pimf, yrdy0,yrdy1, ut0,ut1,
     1                       by,bysig,bz,bzsig, imfna)

C  Get values for the IMF By & Bz at the mid-point of the data
C   from the daily sampled GSFC/NSSDC files.  Note this isn't ideal
C   since the components can change sign more frequently than daily,
C   but doing so in PIM would cause discontinuities as, for example,
C   Bz changes from -/+ hence K_p changes from K_p/1.a
C     Query PLH about using coupling prop to sin^4(theta/2) as in
C      Hargreaves p200 in order to get a continuous effect on K_p.
C
C  If time of data is after file runs out, or if there is no IMF data
C    at the time of data, query how to treat the IMF parameters
C
      INTEGER yrdy0,yrdy1, imfna, irec0,irec1,irec, i,l,mback,mforw
      INTEGER iyr0,idoy0, iyr1,idoy1, ndays,nleapd
      REAL*8 byv(2),bzv(2), bysigv(2),bzsigv(2)
      REAL*8 by,bysig,bz,bzsig, ut0,ut1,utmid, x,x1,x2
      CHARACTER pimf*80, datstr*12, dumm*1
C------------
C
C  YRDY0,YRDY1 - 1st & last days of data
C  UT0,UT1 - 1st & last UTs
C  BY,BZ - Values of IMF components  (not yet just the sign)
C  BYSIG,BZSIG - Uncertainty of By,Bz
C  IMFNA - Controls how to treat IMF parameters
C           0 = no problem reading IMF from file
C           1 = IMF N/A:  assume K_p=1
C           2 = IMF N/A:  run PIM once each for K_p=1 & K_p=K_p
C  IREC - # days since 0Jan81 of midpoint of data
C  UTMID - UT of midpoint of data
C
C------------
C
C  Parse 1st & last YRDYs
C
      iyr0 = yrdy0/1000
      iyr1 = yrdy1/1000
      idoy0 = yrdy0 - iyr0*1000
      idoy1 = yrdy1 - iyr1*1000
      IF (iyr0 .LT. 81) THEN
         iyr0 = iyr0 + 2000
      ELSE
         iyr0 = iyr0 + 1900
      END IF
      IF (iyr1 .LT. 81) THEN
         iyr1 = iyr1 + 2000
      ELSE
         iyr1 = iyr1 + 1900
      END IF

C  Calculate record number in IMF24.DAT for day of 1st data,
C    adjusted for leap years (using IDOY takes care of leap-days in
C    the year IYR0)
C
      irec0 = (iyr0-1981)*365 + idoy0
      nleapd = (iyr0-1981)/4
      irec0 = irec0 + nleapd
      irec1 = (iyr1-1981)*365 + idoy1
      nleapd = (iyr1-1981)/4
      irec1 = irec1 + nleapd
      ndays = irec1 - irec0 + 1

C  Compute mean time of data
C
      utmid = (ut0+ut1)/2.d0 + DBLE(ndays-1)*1.2d1
      irec = irec0 + IDINT(utmid)/24
      utmid = DMOD(utmid,2.4d1)

      OPEN (65, FILE=pimf, STATUS='old')

C  Position file to the day of IREC; read in "today" data
C    As of 2/2/00, format of daily-averaged data from NSSDC/GSFC has 
C       changed - fortunately, this just affects FORMAT-119 because
C       IMF file entered by day-index rather than parsing DATSTR
C       (ie assumes there is a record for every day since 1981jan01)
C
      DO 105 l = 1, (irec-1)
 105     READ (65, '(a1)',END=150) dumm
      DO 110 i = 1,1
        READ (65, 119,END=150) datstr,byv(i),bzv(i),bysigv(i),bzsigv(i)
 110  CONTINUE
c 119  FORMAT (a11,15x,2f6.1,12x,2f6.1)
 119  FORMAT (a12,15x,2f6.1,12x,2f6.1)
cwrite (*,*) 'IMF data read at  ',datstr
      GOTO 201

C  Date of data past end of IMF file - query how to handle
C
 150  WRITE (*,'(/,a)') '*** Requested dates past end of IMF data ***'
      WRITE (*,'(a)') '  Handle IMF Bz by:'
      WRITE (*,'(a)') '     1:  Assume Bz North  (K_p=1)'
      WRITE (*,'(a,$)') '*or* 2:  Run PIM for both Bz North & South?'
C Commented out:
C      READ (*,*) imfna
C Instead use:
      imfna = 1
      WRITE (*,'(a)') '... 1 assumed'
C      

      by = 1.d0
      bysig = 0.d0
      bz = 1.d0
      bzsig = 0.d0
      GOTO 900

C  Test to see if the IMF data exist for this days; if so, all OK
C
 201  IF (byv(1) .LT. 5.d2) THEN
         by = byv(1)
         bysig = bysigv(1)
         bz = bzv(1)
         bzsig = bzsigv(1)
         imfna = 0
         WRITE (*,448) by,bysig
         WRITE (*,449) bz,bzsig
         GOTO 900
      END IF

C  If not, query whether to interpolate or not
C
      WRITE (*,'(/,a)') '*** A specific date is missing IMF Data ***'
      WRITE (*,'(a)') 'Handle IMF Bz by:'
      WRITE (*,'(a)') '     0:  Interpolating between closest dates with
     1 data'
      WRITE (*,'(a)') '     1:  Assuming Bz North  (K_p=1)'
      WRITE (*,'(a)') '*or* 2:  Running PIM for both Bz North & South'
      WRITE (*,'(a,$)') 'Enter choice:  ' 
C Commented out:
C      READ (*,*) imfna
C      WRITE (*,'(a1)') ' '
C Instead use:
      imfna = 0
      WRITE (*,'(a)') '... 1 assumed'
C      

      IF (imfna .EQ. 0) THEN

         mforw = 0
 220     READ (65,119,END=150) datstr,byv(2),bzv(2),bysigv(2),bzsigv(2)
         mforw = mforw + 1
         IF (byv(2) .GT. 5.d2) GOTO 220

         mback = 0
C  Get back to the beginning of the record after IREC
C    (i.e., just read a missing record, 
C            Loop245 positions to beginning of previous record)
         DO 230 l = 1, (mforw)
 230        BACKSPACE (65)
 240     DO 245 l = 1,2
 245        BACKSPACE (65)
         READ (65,119,END=150) datstr,byv(1),bzv(1),bysigv(1),bzsigv(1)
         mback = mback + 1
         IF (byv(1) .GT. 5.d2) GOTO 240

C  Linearly interpolate between the two times with defined IMF data
         x1 = -DBLE(mback)
         x2 = DBLE(mforw)
         x = (utmid - 1.2d1) / 2.4d1
         by = (byv(2)-byv(1)) * (x-x1) / (x2-x1) + byv(1) 
         bz = (bzv(2)-bzv(1)) * (x-x1) / (x2-x1) + bzv(1)
         bysig = (bysigv(2)-bysigv(1)) * (x-x1) / (x2-x1) + bysigv(1)
         bzsig = (bzsigv(2)-bzsigv(1)) * (x-x1) / (x2-x1) + bzsigv(1)
         WRITE (*,448) by,bysig
         WRITE (*,449) bz,bzsig
      
      ELSE 

         by = 1.d0
         bysig = 0.d0
         bz = 1.d0
         bzsig = 0.d0

      END IF

 448  FORMAT (5x,'IMF By =',f7.3,' +/- ',f5.3)
 449  FORMAT (5x,'IMF Bz =',f7.3,' +/- ',f5.3)

 900  CLOSE (65)
      RETURN
      END
