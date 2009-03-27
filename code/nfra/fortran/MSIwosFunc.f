c	msp = MS_OPEN (msname)
c       call MS_CLOSE (msp)
c       call MS_LINS (msp, infnr, fp, bsize, buf, np, err)
c       call MS_FILE01 (msp, bsize, buf, err)
c         msname	C	name of the MeasurementSet
c         ptr           I4      pointer to an MSIwos object
c                               0 = open failed
c         infnr         I2      interferometer number as 256*ifw + ife
c                               if: 0=0X, 1=0Y, 2=1X, ... , 27=DY
c                               IFW=28  total power; IFE=IF number
c                               IFW=32  pointing info; IFE=telescope number
c         fp            I4      channel or band
c                               0 = average
c         bsize         I4      size of buf (in cos/sin pairs for MS_LINS)
c                                           (in bytes for MS_FILE01)
c         buf           *       buffer for cos/sin pairs or file01 structure
c         np            I4      nr of cos/sin pairs stored in buffer
c         err           I4      0 = okay
c                               1 = unknown fp
c                               2 = unknown antenna
c                               3 = unknown polarization
c                               4 = buffer too small
c                               5 = exception in MSIwos class


	integer*4 function MS_OPEN (msname)
c
	implicit none
c
        character*(*) msname
c
	byte      name(1024)
	integer*4 i,leng
c
c                       Functions
        integer*4 MSIWOS_OPEN
c
c
c                       Copy the name to a byte array and end it with a 0.
        leng = 1
        do i=1,len(msname)
           if (msname(i:i) .NE. ' ') then
              name(leng) = ichar(msname(i:i))
	      leng = leng+1
           endif
        enddo
        name(leng) = 0
        MS_OPEN = MSIWOS_OPEN (name)
        return
        end



	subroutine MS_CLOSE (msp)
c
	integer*4 msp
c
	call MSIWOS_CLOSE (msp)
	return
	end



	subroutine MS_LINS (msp, infnr, fp, bsize, buf, np, err)
c
	integer*2 infnr
	integer*4 msp, fp, bsize, np, err
	real*4    buf(2,*)
c
	err = MSIWOS_GETDATA (msp, infnr, fp, bsize, buf, np)
	return
	end



	subroutine MS_FILE01 (msp, bsize, buf, err)
c
	integer*4 msp, bsize, err
	byte      buf(*)
c
	err = MSIWOS_GETFILE01 (msp, bsize, buf)
	return
	end
