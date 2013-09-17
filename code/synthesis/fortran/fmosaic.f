
*=======================================================================
*     Copyright (C) 1999-2013
*     Associated Universities, Inc. Washington DC, USA.
*
*     This library is free software; you can redistribute it and/or
*     modify it under the terms of the GNU Library General Public
*     License as published by the Free Software Foundation; either
*     version 2 of the License, or (at your option) any later version.
*
*     This library is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU Library General Public License for more details.
*
*     You should have received a copy of the GNU Library General Public
*     License along with this library; if not, write to the Free
*     Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,
*     MA 02139, USA.
*
*     Correspondence concerning AIPS++ should be addressed as follows:
*            Internet email: aips2-request@nrao.edu.
*            Postal address: AIPS++ Project Office
*                            National Radio Astronomy Observatory
*                            520 Edgemont Road
*                            Charlottesville, VA 22903-2475 USA
*
*     $Id$
*-----------------------------------------------------------------------
C
C Grid a number of visibility records
C
            subroutine gmosd (uvw, dphase, values, nvispol, nvischan,
     $     dopsf, flag, rflag, weight, nrow, rownum,
     $     scale, offset, grid, nx, ny, npol, nchan, freq, c,
     $     support, convsize, sampling, convfunc, 
     $     chanmap, polmap,
     $     sumwt, weightgrid, convweight, doweightgrid, convplanemap, 
     $     nconvplane)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      complex values(nvispol, nvischan, nrow)
      double complex grid(nx, ny, npol, nchan)
     
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      real weight(nvischan, nrow), phase
      double precision sumwt(npol, nchan)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer dopsf
      double complex weightgrid(nx, ny, npol, nchan)
      integer doweightgrid

      double complex nvalue
      double complex nweight
      integer convsize, sampling
      integer nconvplane
      integer convplanemap(nrow)
      complex convfunc(convsize, convsize, nconvplane), cwt, crot
      complex convweight(convsize, convsize, nconvplane)
      integer sampsupp
      

      complex sconv(-sampling*(support+1):sampling*(support+1), 
     $     -sampling*(support+1):sampling*(support+1), nconvplane)
      complex sconv2(-sampling*(support+1):sampling*(support+1), 
     $     -sampling*(support+1):sampling*(support+1), nconvplane)
      real sumsconv
      real sumsconv2
      real ratioofbeam

      real norm
      real wt

      logical omos, doshift

      real pos(3)
      integer loc(2), off(2), iloc(2)
      integer iiloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan
      integer apol, achan, aconvplane, irow
      double precision pi
      data pi/3.14159265358979323846/
      real maxsconv2, minsconv2
      sampsupp=(support+1)*sampling
      irow=rownum

      sumsconv=0
      sumsconv2=0
      ratioofbeam=1.0

CCCCCCCCCCCCCCCCCCCCCCCC
C     minsconv2=1e17
C      maxsconv2=0.0
CCCCCCCCCCCCCCCCCCCCCCCC     
C      write(*,*) scale, offset
      do iz=1, nconvplane
         do iy=-sampsupp,sampsupp
            iloc(2)=iy+(convsize)/2+1
            do ix=-sampsupp,sampsupp
               iloc(1)=ix+(convsize)/2+1
               sconv(ix,iy,iz)=(convfunc(iloc(1), iloc(2),iz))
               sconv2(ix,iy,iz)=convweight(iloc(1), iloc(2),iz)
C               if(maxsconv2 .lt. abs(sconv2(ix, iy, iz))) then
C                  maxsconv2=abs(sconv2(ix, iy, iz))
C               end if 
C               if(minsconv2 .gt. abs(sconv2(ix, iy, iz))) then
C                  minsconv2=abs(sconv2(ix, iy, iz))
C               end if 
            end do
         end do
      end do

      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if

C      write(*,*) 'max of sconvs ', maxsconv2, minsconv2, sampsupp, 
C     $     convsize 



      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then 
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan).and.
     $              (weight(ichan,irow).ne.0.0)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c, 
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
C     If we are making a PSF then we don't want to phase
C     rotate but we do want to reproject uvw
                           if(dopsf.eq.1) then
                              nvalue=cmplx(weight(ichan,irow))
                           else
                              nvalue=weight(ichan,irow)*
     $                             (values(ipol,ichan,irow)*phasor)
                           end if
                           if(doweightgrid .gt. 0) then
                              nweight=cmplx(weight(ichan,irow))
                           end if
                           
C     norm will be the value we would get for the peak
C     at the phase center. We will want to normalize 
C     the final image by this term.
                           norm=0.0
                           if(sampling.eq.1) then
                              do iy=-support,support
                                 do ix=-support,support
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*sconv(ix,iy, aconvplane)

                                    if(doweightgrid .gt. 0) then
                                       iloc(1)=nx/2+1+ix
                                       iloc(2)=ny/2+1+iy
                                       weightgrid(iloc(1),iloc(2),
     $                                  apol,achan)= weightgrid(
     $                                  iloc(1),iloc(2),apol,achan)
     $                               + nweight*sconv2(ix,iy,aconvplane)

                                    end if
                                 end do
                              end do
                           else 
C                              write(*,*)off
                              do iy=-support, support
                                 iloc(2)=(sampling*iy)+off(2)
                                 do ix=-support, support
                                    iloc(1)=(sampling*ix)+off(1)
                                    cwt=sconv(iloc(1),
     $                                   iloc(2),aconvplane)
C                          write(*,*) support, iloc 
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*cwt
                                    if(doweightgrid .gt. 0) then
                                       cwt=sconv2(sampling*ix, 
     $                                  sampling*iy, 
     $                                      aconvplane)
                                       iiloc(1)=nx/2+1+ix
                                       iiloc(2)=ny/2+1+iy
                                       weightgrid(iiloc(1),iiloc(2),
     $                                      apol,achan)= weightgrid(
     $                                   iiloc(1),iiloc(2),apol,achan)
     $                                + nweight*cwt

                                    end if
                                 end do
                              end do
                           end if  
                           sumwt(apol, achan)= sumwt(apol,achan)+
     $                             weight(ichan,irow)
                        end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C

           subroutine gmosd2 (uvw, dphase, values, nvispol, nvischan,
     $     dopsf, flag, rflag, weight, nrow, rownum,
     $     scale, offset, grid, nx, ny, npol, nchan, freq, c,
     $     support, convsize, sampling, convfunc, 
     $     chanmap, polmap,
     $     sumwt, weightgrid, convweight, doweightgrid, convplanemap, 
     $     convchanmap, convpolmap, 
     $     nconvplane, nconvchan, nconvpol)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      complex values(nvispol, nvischan, nrow)
      double complex grid(nx, ny, npol, nchan)
     
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      real weight(nvischan, nrow), phase
      double precision sumwt(npol, nchan)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer dopsf
      double complex weightgrid(nx, ny, npol, nchan)
      integer doweightgrid

      double complex nvalue
      double complex nweight
      integer convsize, sampling
      integer nconvplane, nconvchan, nconvpol
      integer convplanemap(nrow)
      integer convchanmap(nvischan)
      integer convpolmap(nvispol)
      complex convfunc(convsize, convsize, nconvpol, nconvchan, 
     $ nconvplane), cwt, crot
      complex convweight(convsize, convsize, nconvpol, nconvchan, 
     $ nconvplane)
      integer sampsupp
      

      complex sconv(-sampling*(support+1):sampling*(support+1), 
     $     -sampling*(support+1):sampling*(support+1), nconvplane)
      complex sconv2(-sampling*(support+1):sampling*(support+1), 
     $     -sampling*(support+1):sampling*(support+1), nconvplane)
      real sumsconv
      real sumsconv2
      real ratioofbeam

      real norm
      real wt

      logical omos, doshift

      real pos(3)
      integer loc(2), off(2), iloc(2)
      integer iiloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan, xind, yind
      integer apol, achan, aconvplane, irow
      integer aconvpol, aconvchan, xind2, yind2
      double precision pi
      data pi/3.14159265358979323846/
      real maxsconv2, minsconv2
      sampsupp=(support+1)*sampling
      irow=rownum

      sumsconv=0
      sumsconv2=0
      ratioofbeam=1.0

CCCCCCCCCCCCCCCCCCCCCCCC
C     minsconv2=1e17
C      maxsconv2=0.0
CCCCCCCCCCCCCCCCCCCCCCCC     
C      write(*,*) scale, offset
C      do iz=1, nconvplane
C         do ichan=1, nconvchan
C            do ipol=1, nconvpol
C               do iy=-sampsupp,sampsupp
C                  iloc(2)=iy+(convsize)/2+1
C                  do ix=-sampsupp,sampsupp
C                     iloc(1)=ix+(convsize)/2+1
C                     sconv(ix,iy,iz)=(convfunc(iloc(1), iloc(2),ipol, 
C     $                ichan, iz))
C                     sconv2(ix,iy,iz)=convweight(iloc(1), iloc(2),iz)
CC               if(maxsconv2 .lt. abs(sconv2(ix, iy, iz))) then
CC                  maxsconv2=abs(sconv2(ix, iy, iz))
CC               end if 
CC               if(minsconv2 .gt. abs(sconv2(ix, iy, iz))) then
CC                  minsconv2=abs(sconv2(ix, iy, iz))
CC               end if 
C           end do
C         end do
C      end do

      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if

C      write(*,*) 'max of sconvs ', maxsconv2, minsconv2, sampsupp, 
C     $     convsize 



      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then 
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               aconvchan=convchanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan).and.
     $              (weight(ichan,irow).ne.0.0)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c, 
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        aconvpol=convpolmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
C     If we are making a PSF then we don't want to phase
C     rotate but we do want to reproject uvw
                           if(dopsf.eq.1) then
                              nvalue=cmplx(weight(ichan,irow))
                           else
                              nvalue=weight(ichan,irow)*
     $                             (values(ipol,ichan,irow)*phasor)
                           end if
                           if(doweightgrid .gt. 0) then
                              nweight=cmplx(weight(ichan,irow))
                           end if
                           
C     norm will be the value we would get for the peak
C     at the phase center. We will want to normalize 
C     the final image by this term.
                           norm=0.0
                           if(sampling.eq.1) then
                              do iy=-support,support
                                 yind=iy+(convsize)/2+1
                                 do ix=-support,support
                                    xind=ix+(convsize)/2+1
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*convfunc(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)

                                    if(doweightgrid .gt. 0) then
                                       iloc(1)=nx/2+1+ix
                                       iloc(2)=ny/2+1+iy
                                       weightgrid(iloc(1),iloc(2),
     $                                  apol,achan)= weightgrid(
     $                                  iloc(1),iloc(2),apol,achan)
     $                               + nweight*convweight(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)

                                    end if
                                 end do
                              end do
                           else 
C                              write(*,*)off
                              do iy=-support, support
                                 iloc(2)=(sampling*iy)+off(2)
                                 yind=iloc(2)+(convsize)/2+1
                                 do ix=-support, support
                                    iloc(1)=(sampling*ix)+off(1)
                                    xind=iloc(1)+(convsize)/2+1
                                    cwt=convfunc(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)
C                          write(*,*) support, iloc 
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*cwt
                                    if(doweightgrid .gt. 0) then
                                       xind2=sampling*ix+(convsize)/2+1
                                       yind2=sampling*iy+(convsize)/2+1
                                       cwt=convweight(xind2, 
     $                        yind2, aconvpol, aconvchan, aconvplane)
                                       iiloc(1)=nx/2+1+ix
                                       iiloc(2)=ny/2+1+iy
                                       weightgrid(iiloc(1),iiloc(2),
     $                                      apol,achan)= weightgrid(
     $                                   iiloc(1),iiloc(2),apol,achan)
     $                                + nweight*cwt

                                    end if
                                 end do
                              end do
                           end if  
                           sumwt(apol, achan)= sumwt(apol,achan)+
     $                             weight(ichan,irow)
                        end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C


      subroutine gmoss (uvw, dphase, values, nvispol, nvischan,
     $     dopsf, flag, rflag, weight, nrow, rownum,
     $     scale, offset, grid, nx, ny, npol, nchan, freq, c,
     $     support, convsize, sampling, convfunc, 
     $     chanmap, polmap,
     $     sumwt, weightgrid, convweight, doweightgrid, convplanemap, 
     $     nconvplane)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
     
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      real weight(nvischan, nrow), phase
      double precision sumwt(npol, nchan)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer dopsf
      complex weightgrid(nx, ny, npol, nchan)
      integer doweightgrid

      complex nvalue
      complex nweight
      integer convsize, sampling
      integer nconvplane
      integer convplanemap(nrow)
      complex convfunc(convsize, convsize, nconvplane), cwt, crot
      complex convweight(convsize, convsize, nconvplane)
      

      complex sconv(-(support+1)*sampling:(support+1)*sampling, 
     $     -(support+1)*sampling:(support+1)*sampling, nconvplane)
      complex sconv2(-(support+1)*sampling:(support+1)*sampling, 
     $     -(support+1)*sampling:(support+1)*sampling, nconvplane)
      real sumsconv
      real sumsconv2
      real ratioofbeam

      real norm
      real wt

      logical omos, doshift

      real pos(3)
      integer loc(2), off(2), iloc(2)
      integer iiloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan
      integer apol, achan, aconvplane, irow
      double precision pi
      data pi/3.14159265358979323846/
      integer sampsupp
      sampsupp=(support+1)*sampling
      irow=rownum

      sumsconv=0
      sumsconv2=0
      ratioofbeam=1.0
      do iz=1, nconvplane
         do iy=-sampsupp,sampsupp
            iloc(2)=iy+convsize/2+1
            do ix=-sampsupp,sampsupp
               iloc(1)=ix+convsize/2+1
               sconv(ix,iy,iz)=(convfunc(iloc(1), iloc(2),iz))
               sconv2(ix,iy,iz)=convweight(iloc(1), iloc(2),iz)
            end do
         end do
      end do
      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if

      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then 
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan).and.
     $              (weight(ichan,irow).ne.0.0)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c, 
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
C     If we are making a PSF then we don't want to phase
C     rotate but we do want to reproject uvw
                           if(dopsf.eq.1) then
                              nvalue=cmplx(weight(ichan,irow))
                           else
                              nvalue=weight(ichan,irow)*
     $                             (values(ipol,ichan,irow)*phasor)
                           end if
                           if(doweightgrid .gt. 0) then
                              nweight=cmplx(weight(ichan,irow))
                           end if
                           
C     norm will be the value we would get for the peak
C     at the phase center. We will want to normalize 
C     the final image by this term.
                           norm=0.0
                           if(sampling.eq.1) then
                              do iy=-support,support
                                 do ix=-support,support
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*sconv(ix,iy, aconvplane)

                                    if(doweightgrid .gt. 0) then
                                       iloc(1)=nx/2+1+ix
                                       iloc(2)=ny/2+1+iy
                                       weightgrid(iloc(1),iloc(2),
     $                                  apol,achan)= weightgrid(
     $                                  iloc(1),iloc(2),apol,achan)
     $                               + nweight*sconv2(ix,iy,aconvplane)

                                    end if
                                 end do
                              end do
                           else 
                              do iy=-support,support
                                 iloc(2)=(sampling*iy+off(2))+1
                                 do ix=-support, support
                                    iloc(1)=(sampling*ix+off(1))+1
                                    cwt=sconv(iloc(1),
     $                                   iloc(2),aconvplane)
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*cwt
                                    if(doweightgrid .gt. 0) then
                                       cwt=sconv2(iloc(1), iloc(2), 
     $                                      aconvplane)
                                       iiloc(1)=nx/2+1+ix
                                       iiloc(2)=ny/2+1+iy
                                       weightgrid(iiloc(1),iiloc(2),
     $                                      apol,achan)= weightgrid(
     $                                   iiloc(1),iiloc(2),apol,achan)
     $                                + nweight*cwt

                                    end if
                                 end do
                              end do
                           end if  
                           sumwt(apol, achan)= sumwt(apol,achan)+
     $                             weight(ichan,irow)
                        end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C
           subroutine gmoss2 (uvw, dphase, values, nvispol, nvischan,
     $     dopsf, flag, rflag, weight, nrow, rownum,
     $     scale, offset, grid, nx, ny, npol, nchan, freq, c,
     $     support, convsize, sampling, convfunc, 
     $     chanmap, polmap,
     $     sumwt, weightgrid, convweight, doweightgrid, convplanemap, 
     $     convchanmap, convpolmap, 
     $     nconvplane, nconvchan, nconvpol)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
     
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      real weight(nvischan, nrow), phase
      double precision sumwt(npol, nchan)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer dopsf
      complex weightgrid(nx, ny, npol, nchan)
      integer doweightgrid

      double complex nvalue
      double complex nweight
      integer convsize, sampling
      integer nconvplane, nconvchan, nconvpol
      integer convplanemap(nrow)
      integer convchanmap(nvischan)
      integer convpolmap(nvispol)
      complex convfunc(convsize, convsize, nconvpol, nconvchan, 
     $ nconvplane), cwt, crot
      complex convweight(convsize, convsize, nconvpol, nconvchan, 
     $ nconvplane)
      real sumsconv
      real sumsconv2
      real ratioofbeam

      real norm
      real wt

      logical omos, doshift

      real pos(3)
      integer loc(2), off(2), iloc(2)
      integer iiloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan, xind, yind
      integer apol, achan, aconvplane, irow
      integer aconvpol, aconvchan, xind2, yind2
      double precision pi
      data pi/3.14159265358979323846/
      real maxsconv2, minsconv2
      irow=rownum

      sumsconv=0
      sumsconv2=0
      ratioofbeam=1.0

      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if

C      write(*,*) 'max of sconvs ', maxsconv2, minsconv2, sampsupp, 
C     $     convsize 



      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then 
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               aconvchan=convchanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan).and.
     $              (weight(ichan,irow).ne.0.0)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c, 
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        aconvpol=convpolmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
C     If we are making a PSF then we don't want to phase
C     rotate but we do want to reproject uvw
                           if(dopsf.eq.1) then
                              nvalue=cmplx(weight(ichan,irow))
                           else
                              nvalue=weight(ichan,irow)*
     $                             (values(ipol,ichan,irow)*phasor)
                           end if
                           if(doweightgrid .gt. 0) then
                              nweight=cmplx(weight(ichan,irow))
                           end if
                           
C     norm will be the value we would get for the peak
C     at the phase center. We will want to normalize 
C     the final image by this term.
                           norm=0.0
                           if(sampling.eq.1) then
                              do iy=-support,support
                                 yind=iy+(convsize)/2+1
                                 do ix=-support,support
                                    xind=ix+(convsize)/2+1
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*convfunc(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)

                                    if(doweightgrid .gt. 0) then
                                       iloc(1)=nx/2+1+ix
                                       iloc(2)=ny/2+1+iy
                                       weightgrid(iloc(1),iloc(2),
     $                                  apol,achan)= weightgrid(
     $                                  iloc(1),iloc(2),apol,achan)
     $                               + nweight*convweight(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)

                                    end if
                                 end do
                              end do
                           else 
C                              write(*,*)off
                              do iy=-support, support
                                 iloc(2)=(sampling*iy)+off(2)
                                 yind=iloc(2)+(convsize)/2+1
                                 do ix=-support, support
                                    iloc(1)=(sampling*ix)+off(1)
                                    xind=iloc(1)+(convsize)/2+1
                                    cwt=convfunc(xind, yind, 
     $                                  aconvpol, aconvchan, aconvplane)
C         write(*,*) yind, xind, loc(1)+ix, loc(2)+iy, apol, achan, 
C     $ aconvpol, aconvchan, aconvplane 
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*cwt
                                    if(doweightgrid .gt. 0) then
                                       xind2=sampling*ix+(convsize)/2+1
                                       yind2=sampling*iy+(convsize)/2+1
                                       cwt=convweight(xind2, 
     $                                      yind2, aconvpol, aconvchan, 
     $                                      aconvplane)
                                       iiloc(1)=nx/2+1+ix
                                       iiloc(2)=ny/2+1+iy
                                       weightgrid(iiloc(1),iiloc(2),
     $                                      apol,achan)= weightgrid(
     $                                   iiloc(1),iiloc(2),apol,achan)
     $                                + nweight*cwt

                                    end if
                                 end do
                              end do
                           end if  
                           sumwt(apol, achan)= sumwt(apol,achan)+
     $                             weight(ichan,irow)
                        end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C



C Degrid a number of visibility records
C
      subroutine dmos (uvw, dphase, values, nvispol, nvischan,
     $     flag, rflag,
     $     nrow, rownum, scale, offset, grid, nx, ny, npol, nchan, freq,
     $     c, support, convsize, sampling, convfunc,
     $     chanmap, polmap, convplanemap, nconvplane)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      integer nconvplane
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer convplanemap(nrow)
      complex nvalue

      integer convsize, sampling
      complex convfunc(convsize, convsize, nconvplane), cwt, crot

      integer sampsupp
      
      complex sconv(-(support+1)*sampling:(support+1)*sampling, 
     $     -(support+1)*sampling:(support+1)*sampling, nconvplane)

      real norm, phase

      logical omos, doshift

      real pos(2)
      integer loc(2), off(2), iloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan
      integer apol, achan, aconvplane, irow
      real wt, wtx, wty
      double precision pi
      data pi/3.14159265358979323846/
      
      sampsupp=(support+1)*sampling
      irow=rownum

      do iz=1, nconvplane
         do iy=-sampsupp,sampsupp
            iloc(2)=iy+convsize/2+1
            do ix=-sampsupp,sampsupp
               iloc(1)=ix+convsize/2+1
               sconv(ix,iy,iz)=conjg(convfunc(iloc(1), iloc(2),iz))
            end do
         end do
      end do
      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
C
      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c,
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
                           nvalue=0.0
                           norm=0.0
                           if(sampling.eq.1) then
                             do iy=-support,support
                                 do ix=-support,support
                                    nvalue=nvalue+(sconv(ix,iy,
     $                               aconvplane))*
     $                                   grid(loc(1)+ix,loc(2)+iy,
     $                                   apol,achan)
                                 end do
                              end do
                           else
                              do iy=-support,support
                                 iloc(2)=sampling*iy+off(2)
                                 do ix=-support,support
                                    iloc(1)=ix*sampling
     $                                   +off(1)
                                    cwt=sconv(iloc(1),
     $                                   iloc(2),aconvplane)
                                    nvalue=nvalue+cwt*
     $                                   grid(loc(1)+ix,loc(2)+iy,
     $                                   apol,achan)
                                 end do
                              end do
                           end if 
                           values(ipol,ichan,irow)=nvalue*conjg(
     $                         phasor)
                       end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C

C Degrid a number of visibility records
C
      subroutine dmos2 (uvw, dphase, values, nvispol, nvischan,
     $     flag, rflag,
     $     nrow, rownum, scale, offset, grid, nx, ny, npol, nchan, freq,
     $     c, support, convsize, sampling, convfunc,
     $     chanmap, polmap, convplanemap, convchanmap, convpolmap, 
     $     nconvplane, nconvchan, nconvpol)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow
      integer nconvplane, nconvchan, nconvpol
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(2),
     $     offset(2)
      double precision dphase(nrow), uvdist
      double precision xlast, ylast
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      integer rownum
      integer support
      integer chanmap(nchan), polmap(npol)
      integer convplanemap(nrow), convchanmap(nvischan)
      integer convpolmap(nvispol)
      complex nvalue

      integer convsize, sampling
      complex convfunc(convsize, convsize, nconvpol, nconvchan, 
     $ nconvplane), cwt, crot

      integer sampsupp
      
C      complex sconv(-(support+1)*sampling:(support+1)*sampling, 
C     $     -(support+1)*sampling:(support+1)*sampling, nconvplane)

      real norm, phase

      logical omos, doshift

      real pos(2)
      integer loc(2), off(2), iloc(2)
      integer rbeg, rend
      integer ix, iy, iz, ipol, ichan, xind, yind
      integer apol, achan, aconvplane, irow
      integer aconvchan, aconvpol
      real wt, wtx, wty
      double precision pi
      data pi/3.14159265358979323846/
      
      sampsupp=(support+1)*sampling
      irow=rownum

C      do iz=1, nconvplane
C         do iy=-sampsupp,sampsupp
C            iloc(2)=iy+convsize/2+1
c            do ix=-sampsupp,sampsupp
C               iloc(1)=ix+convsize/2+1
C               sconv(ix,iy,iz)=conjg(convfunc(iloc(1), iloc(2),iz))
C            end do
C         end do
C      end do
      doshift=.FALSE.

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
C
      xlast=0.0
      ylast=0.0
      do irow=rbeg, rend
         aconvplane=convplanemap(irow)+1
         if(rflag(irow).eq.0) then
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               aconvchan=convchanmap(ichan)+1
               if((achan.ge.1).and.(achan.le.nchan)) then
                  call smos(uvw(1,irow), dphase(irow), freq(ichan), c,
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  if (omos(nx, ny, loc, support)) then
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        aconvpol=convpolmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
C          write(*,*) 'aindices', aconvplane, aconvchan, aconvpol
                           nvalue=0.0
                           norm=0.0
                           if(sampling.eq.1) then
                             do iy=-support,support
                                yind=iy+(convsize)/2+1
                                 do ix=-support,support
                                    xind=ix+(convsize)/2+1
                                    nvalue=nvalue+(convfunc(xind,yind,
     $                                aconvpol, aconvchan,
     $                               aconvplane))*
     $                                   grid(loc(1)+ix,loc(2)+iy,
     $                                   apol,achan)
                                 end do
                              end do
                           else
                              do iy=-support,support
                                 iloc(2)=sampling*iy+off(2)
                                 yind=iloc(2)+(convsize)/2+1
C        write(*,*) 'iloc(2)', iloc(2), off(2), yind
                                 do ix=-support,support
                                    iloc(1)=ix*sampling
     $                                   +off(1)
                                    xind=iloc(1)+(convsize)/2+1
C        write(*,*) 'iloc(1)', iloc(1), off(1), xind
                                    cwt=convfunc(xind, yind, aconvpol,
     $                                   aconvchan,aconvplane)
                                    nvalue=nvalue+cwt*
     $                                   grid(loc(1)+ix,loc(2)+iy,
     $                                   apol,achan)
                                 end do
                              end do
                           end if 
                           values(ipol,ichan,irow)=nvalue*conjg(
     $                         phasor)
                       end if
                     end do
                  end if
               end if
            end do
         end if
      end do
      return
      end
C


C Calculate gridded coordinates and the phasor needed for
C phase rotation. 
C
      subroutine smos (uvw, dphase, freq, c, scale, offset, 
     $     sampling, pos, loc, off, phasor)
      implicit none
      integer loc(2), off(2), sampling
      double precision uvw(3), freq, c, scale(2), offset(2)
      real pos(2)
      double precision dphase, phase
      complex phasor
      integer idim
      double precision pi
      data pi/3.14159265358979323846/

      do idim=1,2
         pos(idim)=scale(idim)*uvw(idim)*freq/c+
     $        (offset(idim)+1)
         loc(idim)=nint(pos(idim))
         if(sampling.gt.1) then
C            if((pos(idim)-loc(idim)) < 0.0)then
C               loc(idim)=loc(idim)-1
C            end if 
            off(idim)=nint((pos(idim)-real(loc(idim)))*real(-sampling))
C            if(off(idim).eq.sampling) then
C               off(idim)=0
C               loc(idim)=loc(idim)+1
C            end if
         else
            off(idim)=0
         end if
      end do
      phase=-2.0D0*pi*dphase*freq/c
      phasor=cmplx(cos(phase), sin(phase))
      return 
      end

      logical function omos (nx, ny, loc, support)
      implicit none
      integer nx, ny, nw, loc(2), support
      omos=(loc(1)-support.ge.1).and.(loc(1)+support.le.nx).and.
     $        (loc(2)-support.ge.1).and.(loc(2)+support.le.ny)
      return
      end


