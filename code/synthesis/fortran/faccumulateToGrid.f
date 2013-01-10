      subroutine faccumulateToGrid(grid, convFuncV, nvalue, wVal, 
     $     scaledSupport,scaledSampling,
     $     off, convOrigin, cfShape, loc,
     $     igrdpos, sinDPA, cosDPA,
     $     finitePointingOffset,
     $     doPSFOnly,
     $     norm,
     $     phaseGrad,
     $     imNX, imNY, imNP, imNC,
     $     cfNX, cfNY, cfNP, cfNC,
     $     phNX, phNY)

      implicit none
      integer imNX, imNY, imNC, imNP,
     $     vNX, vNY, vNC, vNP,
     $     cfNX, cfNY, cfNP, cfNC,
     $     phNX, phNY

      complex grid(imNX, imNY, imNP, imNC)
      complex convFuncV(cfNX, cfNY, cfNP, cfNC)
      complex nvalue
      double precision wVal
      integer scaledSupport(2)
      real scaledSampling(2)
      double precision off(2)
      integer convOrigin(4), cfShape(4), loc(4), igrdpos(4)
      double precision sinDPA, cosDPA
      integer finitePointingOffset, doPSFOnly
      double precision norm
      complex phaseGrad(phNX, phNY)

      integer l_phaseGradOriginX, l_phaseGradOriginY
      integer iloc(4), iCFPos(4)
      complex wt, cfArea

      complex fGetCFArea
      integer ix,iy
      integer l_igrdpos(4)

      data iloc/1,1,1,1/, iCFPos/1,1,1,1/
      l_igrdpos(3) = igrdpos(3)+1
      l_igrdpos(4) = igrdpos(4)+1
      norm=0.0
      l_phaseGradOriginX=phNX/2 + 1
      l_phaseGradOriginY=phNY/2 + 1

      cfArea = fGetCFArea(convFuncV, wVal, scaledSupport, 
     $     scaledSampling, off, convOrigin, cfShape,
     $     cfNX, cfNY, cfNP, cfNC)
      
      do iy=-scaledSupport(2),scaledSupport(2) 
         iloc(2)=nint(scaledSampling(2)*iy+off(2)-1)
         iCFPos(2)=iloc(2)+convOrigin(2)+1
         l_igrdpos(2) = loc(2)+iy+1
         do ix=-scaledSupport(1),scaledSupport(1)
            iloc(1)=nint(scaledSampling(1)*ix+off(1)-1)
            iCFPos(1) = iloc(1) + convOrigin(1) + 1
            l_igrdpos(1) = loc(1) + ix + 1
            
            wt = convFuncV(iCFPos(1), iCFPos(2), 
     $           iCFPos(3), iCFPos(4))/cfArea
            if (wVal > 0.0) wt = conjg(wt)

            norm = norm + real(wt)

            if ((finitePointingOffset .eq. 1) .and.
     $              (doPSFOnly .eq. 0)) then
               wt = wt * phaseGrad(iloc(1) + l_phaseGradOriginX,
     $              iloc(2) + l_phaseGradOriginY)
            endif

            grid(l_igrdpos(1), l_igrdpos(2), l_igrdpos(3), l_igrdpos(4)) 
     $           =grid(l_igrdpos(1), l_igrdpos(2), 
     $           l_igrdpos(3), l_igrdpos(4)) + nvalue * wt
         enddo
      enddo
      end
      
      subroutine dfaccumulateToGrid(grid, convFuncV, nvalue, wVal, 
     $     scaledSupport,scaledSampling,
     $     off, convOrigin, cfShape, loc,
     $     igrdpos, sinDPA, cosDPA,
     $     finitePointingOffset,
     $     doPSFOnly,
     $     norm,
     $     phaseGrad,
     $     imNX, imNY, imNP, imNC,
     $     cfNX, cfNY, cfNP, cfNC,
     $     phNX, phNY)

      implicit none
      integer imNX, imNY, imNC, imNP,
     $     vNX, vNY, vNC, vNP,
     $     cfNX, cfNY, cfNP, cfNC,
     $     phNX, phNY

      double complex grid(imNX, imNY, imNP, imNC)
      complex convFuncV(cfNX, cfNY, cfNP, cfNC)
      complex nvalue
      double precision wVal
      integer scaledSupport(2)
      real scaledSampling(2)
      double precision off(2)
      integer convOrigin(4), cfShape(4), loc(4), igrdpos(4)
      double precision sinDPA, cosDPA
      integer finitePointingOffset, doPSFOnly
      double precision norm
      complex phaseGrad(phNX, phNY)

      integer l_phaseGradOriginX, l_phaseGradOriginY
      integer iloc(4), iCFPos(4)
      complex wt, cfArea

      complex fGetCFArea
      integer ix,iy
      integer l_igrdpos(4)

      data iloc/1,1,1,1/, iCFPos/1,1,1,1/
      l_igrdpos(3) = igrdpos(3)+1
      l_igrdpos(4) = igrdpos(4)+1
      norm=0.0
      l_phaseGradOriginX=phNX/2 + 1
      l_phaseGradOriginY=phNY/2 + 1

      cfArea = fGetCFArea(convFuncV, wVal, scaledSupport, 
     $     scaledSampling, off, convOrigin, cfShape,
     $     cfNX, cfNY, cfNP, cfNC)
      
      do iy=-scaledSupport(2),scaledSupport(2) 
         iloc(2)=nint(scaledSampling(2)*iy+off(2)-1)
         iCFPos(2)=iloc(2)+convOrigin(2)+1
         l_igrdpos(2) = loc(2)+iy+1
         do ix=-scaledSupport(1),scaledSupport(1)
            iloc(1)=nint(scaledSampling(1)*ix+off(1)-1)
            iCFPos(1) = iloc(1) + convOrigin(1) + 1
            l_igrdpos(1) = loc(1) + ix + 1
            
            wt = convFuncV(iCFPos(1), iCFPos(2), 
     $           iCFPos(3), iCFPos(4))/cfArea
            if (wVal > 0.0) wt = conjg(wt)

            norm = norm + real(wt)

            if ((finitePointingOffset .eq. 1) .and.
     $              (doPSFOnly .eq. 0)) then
               wt = wt * phaseGrad(iloc(1) + l_phaseGradOriginX,
     $              iloc(2) + l_phaseGradOriginY)
            endif
c$$$            if ((nvalue .eq. 0.0) .or.(wt .eq. 0.0)) then
c$$$               write(*,*) "D: ", nvalue,wt,norm
c$$$            endif

            grid(l_igrdpos(1), l_igrdpos(2), l_igrdpos(3), l_igrdpos(4)) 
     $           =grid(l_igrdpos(1), l_igrdpos(2), 
     $           l_igrdpos(3), l_igrdpos(4)) + nvalue * wt
         enddo
      enddo
      end
      
      complex function fGetCFArea(convFuncV, wVal, support, sampling,
     $     off, convOrigin, cfShape,
     $     cfNX, cfNY, cfNP, cfNC)

      implicit none
      integer cfNX, cfNY, cfNP, cfNC
      complex convFuncV(cfNX, cfNY, cfNP, cfNC)
      double precision wVal
      integer support(2)
      real sampling(2)
      double precision off(2)
      integer convOrigin(4), cfShape(4)

      complex area, wt
      integer iloc(4), iCFPos(4)
      integer ix,iy
      data iloc/1,1,1,1/, iCFPos/1,1,1,1/

      area=0.0
      
      do iy=-support(2), support(2)
         iloc(2) = nint((sampling(2)*iy+off(2))-1)
         iCFPos(2) = iloc(2) + convOrigin(2) + 1
         do ix=-support(1), support(2)
            iloc(1) = nint((sampling(1)*ix+off(1))-1)
            iCFPos(1) = iloc(1) + convOrigin(1) + 1

            wt = convFuncV(iCFPos(1), iCFPos(2), iCFPos(3), iCFPos(4))
            if (wVal > 0.0) wt = conjg(wt)
            area = area + wt
         enddo
      enddo

      fGetCFArea = area
      return
      end
      
