# sd task for image processing (press or basket)
import os
import numpy
import numpy.fft as npfft
import pylab as pl
import asap as sd
from taskinit import *
import time

def sdimprocess(sdimages, mode, numpoly, beamsize, smoothsize, direction, masklist, tmax, tmin, imagename, overwrite):

        casalog.origin('sdimprocess')

        # temporary filename
        tmpstr = time.ctime().replace( ' ', '_' ).replace( ':', '_' )
        tmpmskname = 'masked.'+tmpstr+'.im'
        tmpconvname = 'convolve2d.'+tmpstr+'.im'
        tmppolyname = 'polyfit.'+tmpstr+'.im'
        # set tempolary filename
        tmprealname = []
        tmpimagname = []
        if type(sdimages) == str:
            tmprealname.append( 'fft.'+tmpstr+'.real..0.im' )
            tmpimagname.append( 'fft.'+tmpstr+'.imag.0.im' )
        else:
            for i in range(len(sdimages)):
                tmprealname.append( 'fft.%s.%s.real.im' % (tmpstr,i) )
                tmpimagname.append( 'fft.%s.%s.imag.im' % (tmpstr,i) )

        try:
            # default output filename
            if imagename == '':
                imagename = 'sdimprocess.out.im'
                casalog.post( 'imagename=%s' % imagename )

            # threshold
            nolimit = 'nolimit'
            if tmin == 0.0 and tmax == 0.0:
                thresh = []
            elif tmin > tmax:
                casalog.post('tmin > tmax. Swapped.' )
                thresh = [tmax, tmin]
            elif tmin == tmax:
                if tmin > 0.0:
                    casalog.post( 'tmin == tmax. Use tmin as minumum threshold.' )
                    thresh = [tmin, nolimit]
                else:
                    casalog.post( 'tmin == tmax. Use tmax as maximum threshold.' )
                    thresh = [nolimit, tmin]
            else:
                thresh = [tmin, tmax]


            # main
            #casalog.post( 'mode = %s' % mode )
            if mode == 'press':
                ###
                # Pressed-out method (Sofue & Reich 1979)
                ###
                casalog.post( 'Apply Pressed-out method' )
                
                # check input file
                if type(sdimages) == list:
                    if len(sdimages) != 1:
                        raise Exception, "sdimages allows only one input file for pressed-out method." 
                    else:
                        sdimages = sdimages[0]

                # mask
                image = ia.newimagefromimage(infile=sdimages,outfile=tmpmskname)
                imshape = ia.shape()
                nx = imshape[0]
                ny = imshape[1]
                nchan = imshape[2]
                if len(thresh) == 0:
                    casalog.post( 'Use whole region' )
                else:
                    if len(imshape) == 4:
                        # with polarization axis
                        npol = imshape[3]
                        for ichan in range(nchan):
                            for ipol in range(npol):
                                pixmsk = image.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                                for ix in range(pixmsk.shape[0]):
                                    for iy in range(pixmsk.shape[1]):
                                        if thresh[0] == nolimit:
                                            if pixmsk[ix][iy] > thresh[1]:
                                                pixmsk[ix][iy] = 0.0
                                        elif thresh[1] == nolimit:
                                            if pixmsk[ix][iy] < thresh[0]:
                                                pixmsk[ix][iy] = 0.0
                                        else:
                                            if pixmsk[ix][iy] < thresh[0] or pixmsk[ix][iy] > thresh[1]:
                                                pixmsk[ix][iy] = 0.0
                            image.putchunk( pixmsk, [0,0,ichan,ipol] )
                    elif len(imshape) == 3:
                        # no polarization axis
                        for ichan in range(nchan):
                            pixmsk = image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                            for ix in range(pixmsk.shape[0]):
                                for iy in range(pixmsk.shape[1]):
                                    if thresh[0] == nolimit:
                                        if pixmsk[ix][iy] > thresh[1]:
                                            pixmsk[ix][iy] = 0.0
                                    elif thresh[1] == nolimit:
                                        if pixmsk[ix][iy] < thresh[0]:
                                            pixmsk[ix][iy] = 0.0
                                    else:
                                        if pixmsk[ix][iy] < thresh[0] or pixmsk[ix][iy] > thresh[1]:
                                            pixmsk[ix][iy] = 0.0
                        image.putchunk( pixorg, [0,0,ichan] )
                     

                # smoothing
                #bmajor = 0.0
                #bminor = 0.0
                if type(beamsize) == str:
                    qbeamsize = qa.quantity(beamsize)
                else:
                    qbeamsize = qa.quantity(beamsize,'arcsec')
                if type(smoothsize) == str:
                    #bmajor = smoothsize
                    #bminor = smoothsize
                    qsmoothsize = qa.quantity(smoothsize)
                else:
                    #bmajor = '%sarcsec' % (beamsize*smoothsize)
                    #bminor = '%sarcsec' % (beamsize*smoothsize)
                    qsmoothsize = qa.mul(qbeamsize,smoothsize)
                bmajor = qsmoothsize
                bminor = qsmoothsize
                convimage = image.convolve2d( outfile=tmpconvname, major=bmajor, minor=bminor, overwrite=True )
                convimage = ia.newimage( tmpconvname )

                # get dTij (original - smoothed)
                if len(imshape) == 4:
                    # with polarization axis
                    npol = imshape[3]
                    for ichan in range(nchan):
                        for ipol in range(npol):
                            pixmsk = image.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                            pixsmo = convimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan,ipol] )
                            pixsub = pixmsk - pixsmo
                            convimage.putchunk( pixsub, [0,0,ichan,ipol] )
                elif len(imshape) == 3:
                    for ichan in range(nchan):
                        # no polarization axis
                        pixmsk = image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                        pixsmo = convimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                        pixsub = pixmsk - pixsmo
                        convimage.putchunk( pixsub, [0,0,ichan] )
                
                # polynomial fit
                fitaxis = 0
                if direction == 0.0:
                    fitaxis = 0
                elif direction == 90.0:
                    fitaxis = 1
                else:
                    raise Exception, "Sorry, the task don't supports inclined scan with respect to horizontal or vertical axis, right now."
                polyimage = convimage.fitpolynomial( fitfile=tmppolyname, axis=fitaxis, order=numpoly, overwrite=True )
                polyimage = ia.newimage( tmppolyname )

                # subtract fitted image from original map
                imageorg = ia.newimage( sdimages )
                if len(imshape) == 4:
                    # with polarization axis
                    npol = imshape[3]
                    for ichan in range(nchan):
                        for ipol in range(npol):
                            pixorg = imageorg.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                            pixpol = polyimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan,ipol] )
                            pixsub = pixorg - pixsmo
                            polyimage.putchunk( pixsub, [0,0,ichan,ipol] )
                elif len(imshape) == 3:
                    # no polarization axis
                    for ichan in range(nchan):
                        pixorg = imageorg.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                        pixsmo = polyimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                        pixsub = pixorg - pixsmo
                        polyimage.putchunk( pixsub, [0,0,ichan] )

                # output
                polyimage.rename( imagename, overwrite=overwrite )
                #convimage.rename( imagename, overwrite=overwrite )

                polyimage.close()
                convimage.close()
                image.close()
                imageorg.close()
            elif mode == 'basket':
                ###
                # Basket-Weaving (Emerson & Grave 1988)
                ###
                casalog.post( 'Apply Basket-Weaving' )

                # check input file
                if type(sdimages) == str or (type(sdimages) == list and len(sdimages) <= 1):
                    raise Exception, "sdimages should be a list of input images for Basket-Weaving."

                # check direction
                if type(direction) == float:
                    raise Exception, 'direction must have at least two different direction.'
                else:
                    if len(direction) == 0:
                        raise Exception, 'direction must have at least two different direction.'
                    if len(direction) == len(sdimages):
                        dirs = direction
                    else:
                        casalog.post( 'direction information is extrapolated.' )
                        dirs = []
                        for i in range(len(sdimages)):
                            dirs.append(direction[i%len(direction)])

                # initial setup
                outimage = ia.newimagefromimage( infile=sdimages[0], outfile=imagename, overwrite=overwrite )
                imshape = outimage.shape()
                nx = imshape[0]
                ny = imshape[1]
                nchan = imshape[2]                

                # mask
                for i in range(len(sdimages)):
                    realimage = ia.newimagefromimage( infile=sdimages[i], outfile=tmprealname[i] )
                    imagimage = ia.newimagefromimage( infile=sdimages[i], outfile=tmpimagname[i] )
                    realimage.close()
                    imagimage.close()
                if len(thresh) == 0:
                    casalog.post( 'Use whole region' )
                else:
                    if len(imshape) == 4:
                        # with polarization axis
                        npol = imshape[3]
                        for i in range(len(sdimages)):
                            realimage = ia.newimage( tmprealname[i] )
                            for ichan in range(nchan):
                                for ipol in range(npol):
                                    pixmsk = realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                                    for ix in range(pixmsk.shape[0]):
                                        for iy in range(pixmsk.shape[1]):
                                            if thresh[0] == nolimit:
                                                if pixmsk[ix][iy] > thresh[1]:
                                                    pixmsk[ix][iy] = 0.0
                                            elif thresh[1] == nolimit:
                                                if pixmsk[ix][iy] < thresh[0]:
                                                    pixmsk[ix][iy] = 0.0
                                            else:
                                                if pixmsk[ix][iy] < thresh[0] or pixmsk[ix][iy] > thresh[1]:
                                                    pixmsk[ix][iy] = 0.0
                                    realimage.putchunk( pixmsk, [0,0,ichan,ipol] )
                            realimage.close()
                    elif len(imshape) == 3:
                        # no polarization axis
                        for i in range(len(sdimages)):
                            realimage = ia.newimage( tmprealname[i] )
                            for ichan in range(nchan):
                                pixmsk = image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                                for ix in range(pixmsk.shape[0]):
                                    for iy in range(pixmsk.shape[1]):
                                        if thresh[0] == nolimit:
                                            if pixmsk[ix][iy] > thresh[1]:
                                                pixmsk[ix][iy] = 0.0
                                        elif thresh[1] == nolimit:
                                            if pixmsk[ix][iy] < thresh[0]:
                                                pixmsk[ix][iy] = 0.0
                                        else:
                                            if pixmsk[ix][iy] < thresh[0] or pixmsk[ix][iy] > thresh[1]:
                                                pixmsk[ix][iy] = 0.0
                                realimage.putchunk( pixorg, [0,0,ichan] )
                            realimage.close()

                # set weight factor
                weights = numpy.ones( shape=(len(sdimages),nx,ny), dtype=float )
                eps = 1.0e-5
                dtor = numpy.pi / 180.0
                masks = []
                if type(masklist) == float:
                    for i in range(len(sdimages)):
                        masks.append( masklist )
                elif type(masklist) == list and len(sdimages) != len(masklist):
                    for i in range(len(sdimages)):
                        masks.append( masklist[i%len(masklist)] )
                for i in range(len(masks)):
                    masks[i] = 0.01 * masks[i]
                for i in range(len(sdimages)):
                    if abs(numpy.sin(direction[i]*dtor)) < eps:
                        # direction is around 0 deg
                        maskw = 0.5 * nx * masks[i] 
                        for ix in range(nx):
                            for iy in range(ny):
                                dd = abs( float(ix) - 0.5 * nx )
                                if dd < maskw:
                                    cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                                    weights[i][ix][iy] = 1.0 - cosd * cosd
                    elif abs(numpy.cos(direction[i]*dtor)) < eps:
                        # direction is around 90 deg
                        maskw = 0.5 * ny * masks[i]
                        for ix in range(nx):
                            for iy in range(ny):
                                dd = abs( float(iy) - 0.5 * ny )
                                if dd < maskw:
                                    cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                                    weights[i][ix][iy] = 1.0 - cosd * cosd
                    else:
                        maskw = 0.5 * sqrt( nx * ny ) * masks[i]
                        for ix in range(nx):
                            for iy in range(ny):
                                tand = numpy.tan((direction[i]-90.0)*dtor)
                                dd = abs( ix * tand - iy - 0.5 * nx * tand + 0.5 * ny )
                                dd = dd / sqrt( 1.0 + tand * tand )
                                if dd < maskw:
                                    cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                                    weights[i][ix][iy] = 1.0 - cosd * cosd
                    # shift
                    xshift = -((ny-1)/2)
                    yshift = -((nx-1)/2)
                    for ix in range(xshift,0,1):
                        tmp = weights[i,:,0].copy()
                        weights[i,:,0:ny-1] = weights[i,:,1:ny].copy()
                        weights[i,:,ny-1] = tmp
                    for iy in range(yshift,0,1):
                        tmp = weights[i,0:1].copy()
                        weights[i,0:nx-1] = weights[i,1:nx].copy()
                        weights[i,nx-1:nx] = tmp
              
         

                # FFT
                if len(imshape) == 4:
                    # with polarization axis
                    npol = imshape[3]
                    for i in range(len(sdimages)):
                        realimage = ia.newimage( tmprealname[i] )
                        imagimage = ia.newimage( tmpimagname[i] )
                        for ichan in range(nchan):
                            for ipol in range(npol):
                                pixval = realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] )
                                pixval = pixval.reshape((nx,ny))
                                pixfft = npfft.fft2( pixval )
                                pixfft = pixfft.reshape((nx,ny,1,1))
                                realimage.putchunk( pixfft.real, [0,0,ichan,ipol] )
                                imagimage.putchunk( pixfft.imag, [0,0,ichan,ipol] )
                                del pixval, pixfft
                        realimage.close()
                        imagimage.close()
                elif len(imshape) == 3:
                    # no polarization axis
                    for i in range(len(sdimages)):
                        realimage = ia.newimage( tmprealname[i] )
                        imagimage = ia.newimage( tmpimagname[i] )
                        for ichan in range(nchan):
                            pixval = realimage.getchunk( [0,0,ichan], [nxx-1,ny-1,ichan] )
                            pixval = pixval.reshape((nx,ny))
                            pixfft = npfft.fft2( pixval )
                            pixfft = pixfft.reshape((nx,ny,1))
                            realimage.putchunk( pixfft.real, [0,0,ichan] )
                            imagimage.putchunk( pixfft.imag, [0,0,ichan] )
                            del pixval, pixfft
                        raelimage.close()
                        imagimage.close()
                        
                # weighted mean
                if len(imshape) == 4:
                    npol = imshape[3]
                    for ichan in range(nchan):
                        for ipol in range(npol):
                            pixout = numpy.zeros( shape=(nx,ny), dtype=complex )
                            denom = numpy.zeros( shape=(nx,ny), dtype=float )
                            for i in range(len(sdimages)):
                                realimage = ia.newimage( tmprealname[i] )
                                imagimage = ia.newimage( tmpimagname[i] )
                                pixval = realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) + imagimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) * 1.0j
                                pixval = pixval.reshape((nx,ny))
                                pixout = pixout + pixval * weights[i]
                                denom = denom + weights[i]
                                realimage.close()
                                imagimage.close()
                            pixout = pixout / denom
                            pixout = pixout.reshape((nx,ny,1,1))
                            realimage = ia.newimage( tmprealname[0] )
                            imagimage = ia.newimage( tmpimagname[0] )
                            realimage.putchunk( pixout.real, [0,0,ichan,ipol] )
                            imagimage.putchunk( pixout.imag, [0,0,ichan,ipol] )
                            realimage.close()
                            imagimage.close()
                elif len(imshape) == 3:
                    for ichan in range(nchan):
                        pixout = numpy.zeros( shape=(nx,ny), dtype=complex )
                        denom = numpy.zeros( shape=(nx,ny), dtype=float )
                        for i in range(len(sdimages)):
                            realimage = ia.newimage( tmprealname[i] )
                            imagimage = ia.newimage( tmpimagname[i] )
                            pixval = image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                            pixval = pixval.reshape((nx,ny))
                            pixout = pixout + pixval * weights[i]
                            denom = denom + weights[i]
                            realimage.close()
                            imagimage.close()
                        pixout = pixout / denom
                        pixout = pixout.reshape((nx,ny,1))
                        realimage = ia.newimage( tmprealname[0] )
                        imagimage = ia.newimage( tmpimagname[0] )
                        realimage.putchunk( pixout.real, [0,0,ichan] )
                        imagimage.putchunk( pixout.imag, [0,0,ichan] )
                        realimage.close()
                        imagimage.close()
                
                # inverse FFT
                realimage = ia.newimage( tmprealname[0] )
                imagimage = ia.newimage( tmpimagname[0] )
                if len(imshape) == 4:
                    npol = imshape[3]
                    for ichan in range(nchan):
                        for ipol in range(npol):
                            pixval = realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) + imagimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) * 1.0j
                            pixval = pixval.reshape((nx,ny))
                            pixifft = npfft.ifft2( pixval )
                            pixifft = pixifft.reshape((nx,ny,1,1))
                            outimage.putchunk( pixifft.real, [0,0,ichan,ipol] )
                            del pixval, pixifft
                elif len(imshape) == 3:
                    for ichan in range(nchan):
                        pixval = realimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] ) + imagimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] ) * 1.0j
                        pixval = pixval.reshape((nx,ny))
                        pixifft = npfft.ifft2( pixval )
                        pixifft = pixfft.reshape((nx,ny,1))
                        outimage.putchunk( pixifft.real, [0,0,ichan] )
                        del pixval, pixifft
                realimage.close()
                imagimage.close()
                outimage.close()

        except Exception, instance:
            casalog.post( str(instance), priority = 'ERROR' )
            return
        finally:
            if os.path.exists( tmpmskname ):
                 os.system( 'rm -rf %s' % tmpmskname )
            if os.path.exists( tmpconvname ):
                os.system( 'rm -rf %s' % tmpconvname )
            if os.path.exists( tmppolyname ):
                os.system( 'rm -rf %s' % tmppolyname )
            for i in range( len(tmprealname) ):
                if os.path.exists( tmprealname[i] ):
                    os.system( 'rm -rf %s' % tmprealname[i] )
                if os.path.exists( tmpimagname[i] ):
                    os.system( 'rm -rf %s' % tmpimagname[i] )
