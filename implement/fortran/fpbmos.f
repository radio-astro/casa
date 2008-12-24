*=======================================================================
*     -*- FORTRAN -*-
*     Copyright (C) 1999,2001,2002
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
*     Internet email: aips2-request@nrao.edu.
*     Postal address: AIPS++ Project Office
*     National Radio Astronomy Observatory
*     520 Edgemont Road
*     Charlottesville, VA 22903-2475 USA
*     
*     $Id: fpbwproj.f,v 1.13 2006/07/20 00:24:20 sbhatnag Exp $
*-----------------------------------------------------------------------
C     
C     Grid a number of visibility records
C     
      subroutine gpbmos (uvw, dphase, values, nvispol, nvischan,
     $     dopsf, flag, rflag, weight, nrow, rownum,
     $     scale, offset, grid, nx, ny, npol, nchan, freq, c,
     $     support, convsize, sampling, wconvsize, convfunc, 
     $     chanmap, polmap,polused,sumwt,
     $     ant1, ant2, nant, scanno, sigma,raoff, decoff,area,
     $     dograd,dopointingcorrection,npa,paindex,cfmap,conjcfmap,
     $     currentCFPA,actualPA,doavgpb,avgpb,cfRefFreq)
      
      
      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow,polused
      integer npa,paindex
      integer convsize, sampling, wconvsize,dograd,dopointingcorrection
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan),avgpb(nx,ny,npol,nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(3),
     $     offset(3),currentCFPA,actualPA,cfRefFreq
      double precision dphase(nrow), uvdist
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow)
      real weight(nvischan, nrow), phase
      double precision sumwt(npol, nchan)
      integer rownum
      integer support(wconvsize,polused,npa), rsupport
      integer chanmap(nchan), polmap(npol),cfmap(npol),conjcfmap(npol)
      integer dopsf, doavgpb
      
      complex nvalue
      
      complex convfunc(convsize, convsize, wconvsize, polused),
     $     cwt
      
      real norm
      complex cnorm,tcnorm
      real wt
      
      logical opbmos,mreindex
      external gcppeij
      
      real pos(3)
      integer loc(3), off(3), iloc(3),iu,iv
      integer rbeg, rend
      integer ix, iy, ipol, ichan, tx,ty
      integer apol, achan, irow, PolnPlane, ConjPlane
      double precision pi
      data pi/3.14159265358979323846/
      
      double precision griduvw(2),dPA, cDPA,sDPA
      double precision mysigma, ra1,ra2,dec1,dec2
      double precision sigma,area,lambda,cfscale
      complex pcwt, pdcwt1, pdcwt2,tt
      integer nant, scanno, ant1(nrow), ant2(nrow)
      integer convOrigin
      real raoff(nant), decoff(nant)
      
      
      integer ii,jj,kk,imax,jmax,pbdone
      real tmp
      
      irow=rownum
      pbdone=0
      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
      
      dPA = -(currentCFPA - actualPA)
      cDPA = cos(dPA)
      sDPA = sin(dPA)
      
      
      convOrigin = (convsize-1)/2
      do irow=rbeg, rend
         if(rflag(irow).eq.0) then 
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               
               lambda = c/freq(ichan)
               cfscale = cfRefFreq/freq(ichan)
               
               if((achan.ge.1).and.(achan.le.nchan).and.
     $              (weight(ichan,irow).ne.0.0)) then
                  call spbmos(uvw(1,irow), dphase(irow), 
     $                 freq(ichan), c, scale, offset, sampling, 
     $                 pos, loc, off, phasor)
                  iloc(3)=max(1, min(wconvsize, loc(3)))
                  rsupport=support(iloc(3),1,paindex)
		  rsupport = nint( (rsupport / cfscale)+0.5 )
                  if (opbmos(nx, ny, wconvsize, loc, rsupport)) then
                     PolnPlane=polused+1
                     
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
                           ConjPlane = cfmap(ipol)+1
                           PolnPlane = conjcfmap(ipol)+1

                           if(dopsf.eq.1) then
                              nvalue=cmplx(weight(ichan,irow))
                           else
                              nvalue=weight(ichan,irow)*
     $                             (values(ipol,ichan,irow)*phasor)
                           end if

                           norm=0.0
                           cnorm=cmplx(1.0,0.0)
                           cnorm=0.0
                           tcnorm=0.0
                           do iy=-rsupport,rsupport
                              iloc(2)=(iy*sampling+off(2))
     $                                  * cfscale
                              iv = iloc(2)
                              do ix=-rsupport,rsupport
                                 iloc(1)=(ix*sampling+off(1))
     $                                  * cfscale
                                 iu = iloc(1)
                                 if (mreindex(iu,iv,iloc(1),iloc(2),
     $                                sDPA,cDPA,
     $                                convOrigin, convSize)) then
                                    if (dopointingcorrection .eq. 1) 
     $                                   then
                                       griduvw(1) = (loc(1)-offset(1)+
     $                                      ix-1)
     $                                      /scale(1)-uvw(1,irow)/lambda
                                       griduvw(2) = (loc(2)-offset(2)+
     $                                      iy-1)
     $                                      /scale(2)-uvw(2,irow)/lambda
                                       ra1 = raoff(ant1(irow)+1)
                                       ra2 = raoff(ant2(irow)+1)
                                       dec1= decoff(ant1(irow)+1)
                                       dec2= decoff(ant2(irow)+1)
                                       call gcppeij(griduvw,area,
     $                                      ra1,dec1,ra2,dec2,
     $                                      dograd,pcwt,pdcwt1,pdcwt2,
     $                                      currentCFPA)
                                    else 
                                       pcwt=cmplx(1.0,0.0)
                                    endif
                                    if(uvw(3,irow).gt.0.0) then
                                       cwt=conjg(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),
     $                                      ConjPlane))
                                       pcwt=conjg(pcwt)
                                    else
                                       cwt=(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),
     $                                      PolnPlane))
                                       pcwt=conjg(pcwt)
                                    end if
c                                    cwt = cwt * pcwt
                                    if (dopsf .eq. 1) then
                                       cnorm=cnorm+abs(cwt*pcwt)
                                    else
                                       cnorm=cnorm+cwt
c     $                                      *weight(ichan,irow)
                                    endif
                                    norm=norm+weight(ichan,irow)*
     $                                   abs(cwt)*conjg(pcwt)
                                 endif
                              enddo
                           enddo

                           pbdone=pbdone+1
                           do iy=-rsupport,rsupport
                              iloc(2)=(iy*sampling+off(2))
     $                                  * cfscale
                              iv = iloc(2)
                              do ix=-rsupport,rsupport
                                 iloc(1)=(ix*sampling+off(1))
     $                                  * cfscale
                                 
                                 iu = iloc(1)
                                 
                                 if (mreindex(iu,iv,iloc(1),iloc(2),
     $                                sDPA,cDPA,convOrigin,convSize)) 
     $                                then
                                    if (dopointingcorrection .eq. 1) 
     $                                   then
                                       griduvw(1) = (loc(1)-offset(1)+
     $                                      ix-1)
     $                                      /scale(1)-uvw(1,irow)/lambda
                                       griduvw(2) = (loc(2)-offset(2)+
     $                                      iy-1)
     $                                      /scale(2)-uvw(2,irow)/lambda
                                       ra1 = raoff(ant1(irow)+1)
                                       ra2 = raoff(ant2(irow)+1)
                                       dec1= decoff(ant1(irow)+1)
                                       dec2= decoff(ant2(irow)+1)
                                       call gcppeij(griduvw,area,
     $                                      ra1,dec1,ra2,dec2,
     $                                      dograd,pcwt,pdcwt1,pdcwt2,
     $                                      currentCFPA)
                                    else 
                                       pcwt=cmplx(1.0,0.0)
                                    endif
                                    
                                    if(uvw(3,irow).gt.0.0) then
                                       cwt=conjg(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),
     $                                      ConjPlane))
                                       pcwt=conjg(pcwt)
                                    else
                                       cwt=(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),
     $                                      PolnPlane))
                                       pcwt=conjg(pcwt)
                                    end if
                                    cwt = cwt * pcwt
                                    if (dopsf .eq. 1) then
                                       cwt = abs(cwt)
                                    endif
                                    grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)=
     $                                   grid(loc(1)+ix,
     $                                   loc(2)+iy,apol,achan)+
     $                                   nvalue*cwt
     $                                   /cnorm
C     
C     For PBMosci, comment the above line "/cnorm"
                                    if(doavgpb .gt. 0 
c     $                                   .and. pbdone .eq. 1
     $                                   ) then
                                       tx=nx/2+1+ix
                                       ty=ny/2+1+iy
c$$$                                       write(*,*) tx,ty,real(cwt),
c$$$     $                                      imag(cwt),apol,achan
                                       avgpb(tx,ty,apol,achan)= 
     $                                      avgpb(tx,ty,apol,achan)
     $                                      +
     $                                      weight(ichan,irow)*
     $                                      abs(cwt)*conjg(pcwt)
     $                                      /cnorm
                                    end if
c                                    norm=norm+abs(cwt)*conjg(pcwt)
                                    tcnorm=tcnorm+(cwt/cnorm)
c     cnorm=cnorm+cwt
                                 endif
                              end do
                           end do
                           norm=real(tcnorm)
                           sumwt(apol,achan)=sumwt(apol,achan)+
     $                          weight(ichan,irow)*norm
C     
C     Use the following for PBMOS
C     
c$$$                           sumwt(apol,achan)=sumwt(apol,achan)+
c$$$     $                          weight(ichan,irow)
                        end if
                     end do
c     else
                  end if
               end if
            end do
         end if
      end do
      write(*,*) sumwt(1,1),norm,cnorm
      return
      end
C     
C     Degrid a number of visibility records
C     
      subroutine dpbmos0 (uvw, dphase, values, nvispol, nvischan,
     $     flag, rflag,
     $     nrow, rownum, scale, offset, grid, nx, ny, npol, nchan, freq,
     $     c, support, convsize, sampling, wconvsize, convfunc,
     $     chanmap, polmap,polused, ant1, ant2, nant, 
     $     scanno, sigma, raoff, decoff,area,dograd,
     $     dopointingcorrection,npa,paindex,cfmap,conjcfmap,
     $     currentCFPA,actualPA,cfRefFreq)
      
      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow,polused
      integer npa,paindex
      integer convsize, wconvsize, sampling
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(3),
     $     offset(3),currentCFPA,actualPA, dPA, sDPA, cDPA,cfRefFreq
      double precision dphase(nrow), uvdist
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow),cfmap(npol),conjcfmap(npol)
      integer rownum
      integer support(wconvsize,polused,npa), rsupport
      integer chanmap(*), polmap(*)
      
      integer nant, scanno, ant1(nrow), ant2(nrow),dograd,
     $     dopointingcorrection
      real raoff(nant), decoff(nant)
      double precision sigma,area,lambda,cfscale
      
      complex nvalue
      
      complex convfunc(convsize, convsize, wconvsize, polused),
     $     cwt, pcwt,pdcwt1,pdcwt2
      double precision griduvw(2)
      double precision mysigma, ra1,ra2,dec1,dec2
      
      complex norm(4)
      
      logical opbmos,mreindex
      external gcppeij
      
      real pos(3)
      integer loc(3), off(3), iloc(3)
      integer rbeg, rend
      integer ix, iy, ipol, ichan, PolnPlane, ConjPlane
      integer convOrigin
      integer apol, achan, irow
      real wt, wtx, wty
      double precision pi
      data pi/3.14159265358979323846/
      integer ii,jj,iu,iv
      
      complex tmp
      
      irow=rownum
      
      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
C     
      
      dPA = -(currentCFPA - actualPA)
      cDPA = cos(dPA)
      sDPA = sin(dPA)
      convOrigin = (convsize-1)/2
      
      do irow=rbeg, rend
         if(rflag(irow).eq.0) then
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               
               lambda = c/freq(ichan)
               cfscale = cfRefFreq/freq(ichan)
               
               if((achan.ge.1).and.(achan.le.nchan)) then
                  call spbmos(uvw(1,irow), dphase(irow), freq(ichan), c,
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  iloc(3)=max(1, min(wconvsize, loc(3)))
                  rsupport=support(iloc(3),1,paindex)
		  rsupport = nint( (rsupport / cfscale)+0.5 )
                  if (opbmos(nx, ny, wconvsize, loc, rsupport)) then
                     PolnPlane=0
                     
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
                           ConjPlane = cfmap(ipol)+1
                           PolnPlane = conjcfmap(ipol)+1
                           
                           nvalue=0.0
                           norm(apol)=cmplx(0.0,0.0)
                           pcwt=cmplx(1.0,0.0)
                           
                           do iy=-rsupport,rsupport
                              iloc(2)=1+(iy*sampling+off(2))+convOrigin
			      iloc(2) = iloc(2) * cfscale
                              iv = (iy*sampling+off(2))
                              do ix=-rsupport,rsupport
                                 iloc(1)=1+(ix*sampling+off(1))
     $                                +convOrigin
                                 iloc(1) = iloc(1) * cfscale
                                 iu = (ix*sampling+off(1))
                                 
                                 if(mreindex(iu,iv,iloc(1),iloc(2),
     $                                sDPA,cDPA, convOrigin, convSize)) 
     $                                then
                                    if (dopointingcorrection .eq. 1)then
                                       griduvw(1)=(loc(1)-offset(1)
     $                                      +ix-1)/scale(1)-uvw(1,irow)
     $                                      /lambda
                                       griduvw(2)=(loc(2)-offset(2)
     $                                      +iy-1)/scale(2)-uvw(2,irow)
     $                                      /lambda
                                       ra1 = raoff(ant1(irow)+1)
                                       ra2 = raoff(ant2(irow)+1)
                                       dec1= decoff(ant1(irow)+1)
                                       dec2= decoff(ant2(irow)+1)
                                       call gcppeij(griduvw,area,
     $                                      ra1,dec1,ra2,dec2,
     $                                      dograd,pcwt,pdcwt1,pdcwt2,
     $                                      currentCFPA)
                                    endif
                                    
                                    if(uvw(3,irow).gt.0.0) then
                                       cwt=conjg(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),ConjPlane))
                                       pcwt = (pcwt)
                                    else
                                       cwt=(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),PolnPlane))
                                       pcwt = (pcwt)
                                    endif
                                    nvalue=nvalue+(cwt)*(pcwt)*
     $                                   grid(loc(1)+ix,loc(2)+iy,apol,
     $                                   achan)
                                    norm(apol)=norm(apol)+(pcwt*cwt)
                                 endif
                              end do
                           end do
                           values(ipol,ichan,irow)=
     $                          nvalue*conjg(phasor)
     $                          /norm(apol)
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
C     Degrid a number of visibility records along with the grad. computations
C     
      subroutine dpbmosgrad (uvw, dphase, values, nvispol, nvischan,
     $     gazvalues, gelvalues, doconj,
     $     flag, rflag, nrow, rownum, scale, offset, grid, 
     $     nx, ny, npol, nchan, freq, c, support, convsize, sampling, 
     $     wconvsize, convfunc, chanmap, polmap,polused,ant1,ant2,nant, 
     $     scanno, sigma, raoff, decoff,area,dograd,
     $     dopointingcorrection,npa,paindex,cfmap,conjcfmap,
     $     currentCFPA,actualPA,cfRefFreq)
      
      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow,polused
      integer npa,paindex,doconj
      integer convsize, wconvsize, sampling
      complex values(nvispol, nvischan, nrow)
      complex gazvalues(nvispol, nvischan, nrow)
      complex gelvalues(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(3),
     $     offset(3),currentCFPA,actualPA, dPA, sDPA, cDPA,cfRefFreq
      double precision dphase(nrow), uvdist
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow),cfmap(npol),conjcfmap(npol)
      integer rownum
      integer support(wconvsize,polused,npa), rsupport
      integer chanmap(*), polmap(*)
      
      integer nant, scanno, ant1(nrow), ant2(nrow),dograd,
     $     dopointingcorrection
      real raoff(nant), decoff(nant)
      double precision sigma,area,lambda,cfscale
      
      complex nvalue,ngazvalue,ngelvalue
      
      complex convfunc(convsize, convsize, wconvsize, polused),
     $     cwt, pcwt,pdcwt1,pdcwt2
      double precision griduvw(2)
      double precision mysigma, ra1,ra2,dec1,dec2
      
      complex norm(4),gradaznorm(4),gradelnorm(4)
      
      logical opbmos,mreindex
      external gcppeij
      
      real pos(3)
      integer loc(3), off(3), iloc(3)
      integer rbeg, rend
      integer ix, iy, ipol, ichan, PolnPlane, ConjPlane
      integer convOrigin
      integer apol, achan, irow
      real wt, wtx, wty
      double precision pi
      data pi/3.14159265358979323846/
      integer ii,jj,iu,iv
      
      complex tmp
      
      irow=rownum
      
      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
C     
      dPA = -(currentCFPA - actualPA)
      
      cDPA = cos(dPA)
      sDPA = sin(dPA)
      convOrigin = (convsize-1)/2
      
      do irow=rbeg, rend
         if(rflag(irow).eq.0) then
            do ichan=1, nvischan
               achan=chanmap(ichan)+1
               
               lambda = c/freq(ichan)
               cfscale = cfRefFreq/freq(ichan)
               
               if((achan.ge.1).and.(achan.le.nchan)) then
                  call spbmos(uvw(1,irow), dphase(irow), freq(ichan), c,
     $                 scale, offset, sampling, pos, loc, off, phasor)
                  iloc(3)=max(1, min(wconvsize, loc(3)))
                  rsupport=support(iloc(3),1,paindex)
		  rsupport = nint( (rsupport / cfscale)+0.5 )
                  if (opbmos(nx, ny, wconvsize, loc, rsupport)) then
                     
                     do ipol=1, nvispol
                        apol=polmap(ipol)+1
                        if((flag(ipol,ichan,irow).ne.1).and.
     $                       (apol.ge.1).and.(apol.le.npol)) then
                           ConjPlane = conjcfmap(ipol)+1
                           PolnPlane = cfmap(ipol)+1
                           ConjPlane = cfmap(ipol)+1
                           PolnPlane = conjcfmap(ipol)+1
                           
                           nvalue=0.0
                           ngazvalue = 0.0
                           ngelvalue = 0.0
                           
                           norm(apol)=cmplx(0.0,0.0)
                           gradaznorm(apol)=cmplx(0.0,0.0)
                           gradelnorm(apol)=cmplx(0.0,0.0)
                           pcwt=cmplx(1.0,0.0)
                           
                           do iy=-rsupport,rsupport
                              iloc(2)=1+(iy*sampling+off(2))+convOrigin
			      iloc(2) = iloc(2) * cfscale
                              iv=(iy*sampling+off(2))
                              do ix=-rsupport,rsupport
                                 iloc(1)=1+(ix*sampling+off(1))
     $                                +convOrigin
     				 iloc(1) = iloc(1) * cfscale
                                 iu=(ix*sampling+off(1))
                                 
                                 if(mreindex(iu,iv,iloc(1),iloc(2),
     $                                sDPA,cDPA, convOrigin, convSize)) 
     $                                then
                                    if (dopointingcorrection .eq. 1)then
                                       griduvw(1)=(loc(1)-offset(1)
     $                                      +ix-1)/scale(1)-uvw(1,irow)
     $                                      /lambda
                                       griduvw(2)=(loc(2)-offset(2)
     $                                      +iy-1)/scale(2)-uvw(2,irow)
     $                                      /lambda
                                       ra1 = raoff(ant1(irow)+1)
                                       ra2 = raoff(ant2(irow)+1)
                                       dec1= decoff(ant1(irow)+1)
                                       dec2= decoff(ant2(irow)+1)
                                       call gcppeij(griduvw,area,
     $                                      ra1,dec1,ra2,dec2,
     $                                      dograd,pcwt,pdcwt1,pdcwt2,
     $                                      currentCFPA)
                                    endif
                                    if(uvw(3,irow).gt.0.0) then
                                       cwt=conjg(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),ConjPlane))
                                       pcwt = conjg(pcwt)
                                       pdcwt1 = conjg(pdcwt1)
                                       pdcwt2 = conjg(pdcwt2)
                                    else
                                       cwt=(convfunc(iloc(1),
     $                                      iloc(2), iloc(3),PolnPlane))
                                    endif
                                    cwt = cwt*pcwt
                                    nvalue=nvalue+(cwt)*
     $                                   grid(loc(1)+ix,loc(2)+iy,apol,
     $                                   achan)
                                    
                                    if ((doconj .eq. 1) .and. 
     $                                   (dograd .eq. 1)) then
                                       pdcwt1 = conjg(pdcwt1)
                                       pdcwt2 = conjg(pdcwt2)
                                    endif
                                    if (dograd .eq. 1) then
                                       pdcwt1 = pdcwt1*cwt
                                       pdcwt2 = pdcwt2*cwt
                                       ngazvalue=ngazvalue+(pdcwt1)*
     $                                      (grid(loc(1)+ix,loc(2)+iy,
     $                                      apol,achan))
                                       ngelvalue=ngelvalue+(pdcwt2)*
     $                                      (grid(loc(1)+ix,loc(2)+iy,
     $                                      apol,achan))
                                    endif
                                    
                                    norm(apol)=norm(apol)+(cwt)
                                    gradaznorm(apol)=gradaznorm(apol)+
     $                                   pdcwt1
                                    gradelnorm(apol)=gradelnorm(apol)+
     $                                   pdcwt1
                                 endif
                              end do
                           end do
                           values(ipol,ichan,irow)=
     $                          nvalue*conjg(phasor)
     $                          /norm(apol)
                           if (dograd .eq. 1) then
                              gazvalues(ipol,ichan,irow)=ngazvalue*
     $                             conjg(phasor)/norm(apol)
                              gelvalues(ipol,ichan,irow)=ngelvalue*
     $                             conjg(phasor)/norm(apol)
                           endif
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
      subroutine dpbmos (uvw, dphase, values, nvispol, nvischan,
     $     flag, rflag,
     $     nrow, rownum, scale, offset, grid, nx, ny, npol, nchan, freq,
     $     c, support, convsize, sampling, wconvsize, convfunc,
     $     chanmap, polmap,polused, ant1, ant2, nant, 
     $     scanno, sigma, raoff, decoff,area,dograd,
     $     dopointingcorrection,npa,paindex,cfmap,conjcfmap,
     $     currentCFPA,actualPA,cfRefFreq)

      implicit none
      integer nx, ny, npol, nchan, nvispol, nvischan, nrow,polused
      integer npa,paindex
      integer convsize, wconvsize, sampling
      complex values(nvispol, nvischan, nrow)
      complex grid(nx, ny, npol, nchan)
      double precision uvw(3, nrow), freq(nvischan), c, scale(3),
     $     offset(3),currentCFPA,actualPA, dPA, sDPA, cDPA,cfRefFreq
      double precision dphase(nrow), uvdist
      complex phasor
      integer flag(nvispol, nvischan, nrow)
      integer rflag(nrow),cfmap(npol),conjcfmap(npol)
      integer rownum
      integer support(wconvsize,polused,npa), rsupport
      integer chanmap(*), polmap(*)

      integer nant, scanno, ant1(nrow), ant2(nrow),dograd,
     $     dopointingcorrection
      real raoff(nant), decoff(nant)
      double precision sigma,area,lambda,cfscale

      complex nvalue

C      complex convfunc(convsize/2-1, convsize/2-1, wconvsize, polused),
      complex convfunc(convsize, convsize, wconvsize, polused),
     $     cwt, pcwt,pdcwt1,pdcwt2
      double precision griduvw(2)
      double precision mysigma, ra1,ra2,dec1,dec2

      complex norm(4)

      logical opbmos,reindex
c      external nwcppEij
      external gcppeij

      real pos(3)
      integer loc(3), off(3), iloc(3)
      integer rbeg, rend
      integer ix, iy, ipol, ichan, PolnPlane, ConjPlane
      integer convOrigin
      integer apol, achan, irow
      real wt, wtx, wty
      double precision pi
      data pi/3.14159265358979323846/
      integer ii,jj,iu,iv

      complex tmp

      irow=rownum

      if(irow.ge.0) then
         rbeg=irow+1
         rend=irow+1
      else 
         rbeg=1
         rend=nrow
      end if
C

      dPA = -(currentCFPA - actualPA)

      cDPA = cos(dPA)
      sDPA = sin(dPA)
      convOrigin = (convsize-1)/2

      do irow=rbeg, rend
         if(rflag(irow).eq.0) then
         do ichan=1, nvischan
            achan=chanmap(ichan)+1

            lambda = c/freq(ichan)
	    cfscale = cfRefFreq/freq(ichan)

            if((achan.ge.1).and.(achan.le.nchan)) then
               call spbmos(uvw(1,irow), dphase(irow), freq(ichan), c,
     $              scale, offset, sampling, pos, loc, off, phasor)
               iloc(3)=max(1, min(wconvsize, loc(3)))
               rsupport=support(iloc(3),1,paindex)
		  rsupport = nint( (rsupport / cfscale)+0.5 )
               if (opbmos(nx, ny, wconvsize, loc, rsupport)) then
C                  PolnPlane=polused+1
                  PolnPlane=0

c$$$                 do iy=-rsupport,rsupport
c$$$                     iloc(2)=1+(iy*sampling+off(2))+convOrigin
c$$$                     do ix=-rsupport,rsupport
c$$$                        iloc(1)=1+(ix*sampling+off(1))+convOrigin
c$$$                        cwt=(convfunc(iloc(1),
c$$$     $                       iloc(2), iloc(3),1))
c$$$                        tmp=(convfunc(iloc(1),
c$$$     $                       iloc(2), iloc(3),2))
c$$$                        write(*,*)abs(cwt),abs(tmp),
c$$$     $                       ix,iy
c$$$                     enddo
c$$$                     write(*,*)
c$$$                  enddo
c$$$                  stop

                  do ipol=1, nvispol
                     apol=polmap(ipol)+1
                     if((flag(ipol,ichan,irow).ne.1).and.
     $                    (apol.ge.1).and.(apol.le.npol)) then

c$$$                        ConjPlane = conjcfmap(ipol)+1
c$$$                        PolnPlane = cfmap(ipol)+1
C The following after feed_x -> -feed_x and PA -> PA + PI/2
                        ConjPlane = cfmap(ipol)+1
                        PolnPlane = conjcfmap(ipol)+1

                        nvalue=0.0
                        norm(apol)=cmplx(0.0,0.0)
                        pcwt=cmplx(1.0,0.0)

                        do iy=-rsupport,rsupport
                           iloc(2)=1+(iy*sampling+off(2))+convOrigin
			   iloc(2) = iloc(2) * cfscale
                           iv = (iy*sampling+off(2))
                           do ix=-rsupport,rsupport
                              iloc(1)=1+(ix*sampling+off(1))+convOrigin
			      iloc(1) = iloc(1) * cfscale
                              iu = (ix*sampling+off(1))

                              if(reindex(iu,iv,iloc(1),iloc(2),
     $                             sDPA,cDPA, convOrigin, convSize)) 
     $                             then

                                 if (dopointingcorrection .eq. 1) then
                                    griduvw(1) = (loc(1)-offset(1)+ix-1)
     $                                   /scale(1)-uvw(1,irow)/lambda
                                    griduvw(2) = (loc(2)-offset(2)+iy-1)
     $                                   /scale(2)-uvw(2,irow)/lambda
                                    ra1 = raoff(ant1(irow)+1)
                                    ra2 = raoff(ant2(irow)+1)
                                    dec1= decoff(ant1(irow)+1)
                                    dec2= decoff(ant2(irow)+1)
                                    call gcppeij(griduvw,area,
     $                                   ra1,dec1,ra2,dec2,
     $                                   dograd,pcwt,pdcwt1,pdcwt2,
     $                                   currentCFPA)
                                 endif

                                 if(uvw(3,irow).gt.0.0) then
                                    cwt=conjg(convfunc(iloc(1),
     $                                   iloc(2), iloc(3),ConjPlane))
                                    pcwt = (pcwt)
                                 else
                                    cwt=(convfunc(iloc(1),
     $                                   iloc(2), iloc(3),PolnPlane))
                                    pcwt = (pcwt)
                                 endif
                                 nvalue=nvalue+(cwt)*(pcwt)*
     $                              grid(loc(1)+ix,loc(2)+iy,apol,achan)
                                 norm(apol)=norm(apol)+(pcwt*cwt)
c                              norm(apol)=norm(apol)+cwt
c$$$                              write(*,*)abs(grid(loc(1)+ix,loc(2)+iy
c$$$     $                                ,apol,achan)),ix,iy,apol,abs(cwt),
c$$$     $                                (pcwt),dopointingcorrection
                              endif
                           end do
c                           write(*,*)
                        end do
c                        norm(apol) = norm(apol)/abs(norm(apol))
                        values(ipol,ichan,irow)=
     $                       nvalue*conjg(phasor)
c     $                       /norm(apol)
c$$$                        write (*,*) ipol,ichan,irow,values(ipol,ichan,
c$$$     $                       irow),nvalue,norm(apol)
c$$$                  stop
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
C     Calculate gridded coordinates and the phasor needed for
C     phase rotation. 
C     
      subroutine spbmos (uvw, dphase, freq, c, scale, offset, 
     $     sampling, pos, loc, off, phasor)
      implicit none
      integer loc(3), off(3), sampling
      double precision uvw(3), freq, c, scale(3), offset(3)
      real pos(3)
      double precision dphase, phase
      complex phasor
      integer idim
      double precision pi
      data pi/3.14159265358979323846/
      
      pos(3)=sqrt(abs(scale(3)*uvw(3)*freq/c))+offset(3)+1.0
      loc(3)=nint(pos(3))
      off(3)=0
      
      do idim=1,2
         pos(idim)=scale(idim)*uvw(idim)*freq/c+
     $        (offset(idim)+1.0)
         loc(idim)=nint(pos(idim))
         off(idim)=nint((loc(idim)-pos(idim))*sampling)
      end do
      
      phase=-2.0D0*pi*dphase*freq/c
      phasor=cmplx(cos(phase), sin(phase))
      return 
      end
      logical function opbmos (nx, ny, nw, loc, support)
      implicit none
      integer nx, ny, nw, loc(3), support
      opbmos=(support.gt.0).and.
     $     (loc(1)-support.ge.1).and.(loc(1)+support.le.nx).and.
     $     (loc(2)-support.ge.1).and.(loc(2)+support.le.ny).and.
     $     (loc(3).ge.1).and.(loc(3).le.nw)
      return
      end
      
      logical function mreindex(inx,iny,outx,outy,sinDPA,cosDPA,
     $     Origin, Size)
      integer inx,iny,outx,outy, Origin, Size
      double precision sinDPA, cosDPA
      integer ix,iy
      
      ix = nint(cosDPA*inx + sinDPA*iny+1)
      iy = nint(-sinDPA*inx + cosDPA*iny+1)
      outx = ix+Origin
      outy = iy+Origin
      
      mreindex=(outx .ge. 1 .and. 
     $     outx .le. Size .and.
     $     outy .ge. 1 .and.
     $     outy .le. Size)
      
      return
      end

