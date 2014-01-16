# sd task for image processing (press or basket)
import os
import time
import numpy
import numpy.fft as npfft

from taskinit import casalog, gentools, utilstool, qatool

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def sdimprocess(infiles, mode, numpoly, beamsize, smoothsize, direction, masklist, tmax, tmin, outfile, overwrite):
    with sdutil.sdtask_manager(sdimprocess_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        
    
class sdimprocess_worker(sdutil.sdtask_interface):
    def __init__(self, **kwargs):
        super(sdimprocess_worker,self).__init__(**kwargs)

    def __del__(self, base=sdutil.sdtask_interface):
        # cleanup method must be called when the instance is
        # deleted
        self.cleanup()
        super(sdimprocess_worker,self).__del__()

    def initialize(self):
        self.parameter_check()
            
        # temporary filename
        tmpstr = time.ctime().replace( ' ', '_' ).replace( ':', '_' )
        self.tmpmskname = 'masked.'+tmpstr+'.im'
        self.tmpconvname = 'convolve2d.'+tmpstr+'.im'
        self.tmppolyname = 'polyfit.'+tmpstr+'.im'
        # set tempolary filename
        self.tmprealname = []
        self.tmpimagname = []
        self.image = None
        self.convimage = None
        self.polyimage = None
        self.imageorg = None
        self.realimage = None
        self.imagimage = None
        if type(self.infiles) == str:
            self.tmprealname.append( 'fft.'+tmpstr+'.real..0.im' )
            self.tmpimagname.append( 'fft.'+tmpstr+'.imag.0.im' )
        else:
            for i in range(len(self.infiles)):
                self.tmprealname.append( 'fft.%s.%s.real.im' % (tmpstr,i) )
                self.tmpimagname.append( 'fft.%s.%s.imag.im' % (tmpstr,i) )

        # default output filename
        if self.outfile == '':
            self.outfile = 'sdimprocess.out.im'
        casalog.post( 'outfile=%s' % self.outfile )

        # threshold
        self.nolimit = 'nolimit'
        if self.tmin == 0.0 and self.tmax == 0.0:
            self.thresh = []
        elif self.tmin > self.tmax:
            casalog.post('tmin > tmax. Swapped.' )
            self.thresh = [self.tmax, self.tmin]
        elif self.tmin == self.tmax:
            if self.tmin > 0.0:
                casalog.post( 'tmin == tmax. Use tmin as minumum threshold.' )
                self.thresh = [self.tmin, self.nolimit]
            else:
                casalog.post( 'tmin == tmax. Use tmax as maximum threshold.' )
                self.thresh = [self.nolimit, self.tmin]
        else:
            self.thresh = [self.tmin, self.tmax]

    def parameter_check(self):
        if self.mode.lower() == 'press':
            # Pressed-out method
            # check input file
            if type(self.infiles) == list:
                if len(self.infiles) != 1:
                    raise Exception, "infiles allows only one input file for pressed-out method." 
                else:
                    self.infiles = self.infiles[0]
            # check direction
            if type(self.direction) == list:
                if len(self.direction) != 1:
                    raise Exception, "direction allows only one direction for pressed-out method."
                else:
                    self.direction = self.direction[0]
        elif self.mode.lower() == 'basket':
            # FFT-based basket-weaving method
            # check input file
            if type(self.infiles) == str or \
                   (type(self.infiles) == list and len(self.infiles) < 2):
                raise Exception, "infiles should be a list of input images for Basket-Weaving."

            # check direction
            if type(self.direction) == float:
                raise Exception, 'direction must have at least two different direction.'
            else:
                if len(self.direction) < 2:
                    raise Exception, 'direction must have at least two different direction.'
        else:
            raise Exception, 'Unsupported processing mode: %s'%(self.mode)

    def execute(self):
        if self.mode.lower() == 'press':
           self.__execute_press()
        elif self.mode.lower() == 'basket':
            self.__execute_basket_weaving()

    def __execute_press(self):
        ###
        # Pressed-out method (Sofue & Reich 1979)
        ###
        casalog.post( 'Apply Pressed-out method' )

        # CAS-5410 Use private tools inside task scripts
        ia = gentools(['ia'])[0]

        # mask
        self.image = ia.newimagefromimage(infile=self.infiles,outfile=self.tmpmskname)
        imshape = self.image.shape()
        nx = imshape[0]
        ny = imshape[1]
        nchan = imshape[2]
        if len(self.thresh) == 0:
            casalog.post( 'Use whole region' )
        else:
            if len(imshape) == 4:
                # with polarization axis
                npol = imshape[3]
                for ichan in range(nchan):
                    for ipol in range(npol):
                        pixmsk = self.image.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                        for ix in range(pixmsk.shape[0]):
                            for iy in range(pixmsk.shape[1]):
                                if self.thresh[0] == self.nolimit:
                                    if pixmsk[ix][iy] > self.thresh[1]:
                                        pixmsk[ix][iy] = 0.0
                                elif self.thresh[1] == self.nolimit:
                                    if pixmsk[ix][iy] < self.thresh[0]:
                                        pixmsk[ix][iy] = 0.0
                                else:
                                    if pixmsk[ix][iy] < self.thresh[0] or pixmsk[ix][iy] > self.thresh[1]:
                                        pixmsk[ix][iy] = 0.0
                        self.image.putchunk( pixmsk, [0,0,ichan,ipol] )
            elif len(imshape) == 3:
                # no polarization axis
                for ichan in range(nchan):
                    pixmsk = self.image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                    for ix in range(pixmsk.shape[0]):
                        for iy in range(pixmsk.shape[1]):
                            if self.thresh[0] == self.nolimit:
                                if pixmsk[ix][iy] > self.thresh[1]:
                                    pixmsk[ix][iy] = 0.0
                            elif self.thresh[1] == self.nolimit:
                                if pixmsk[ix][iy] < self.thresh[0]:
                                    pixmsk[ix][iy] = 0.0
                            else:
                                if pixmsk[ix][iy] < self.thresh[0] or pixmsk[ix][iy] > self.thresh[1]:
                                    pixmsk[ix][iy] = 0.0
                    self.image.putchunk( pixmsk, [0,0,ichan] )


        # smoothing
        #bmajor = 0.0
        #bminor = 0.0
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()
        if type(self.beamsize) == str:
            qbeamsize = qa.quantity(self.beamsize)
        else:
            qbeamsize = qa.quantity(self.beamsize,'arcsec')
        if type(self.smoothsize) == str:
            #bmajor = smoothsize
            #bminor = smoothsize
            qsmoothsize = qa.quantity(self.smoothsize)
        else:
            #bmajor = '%sarcsec' % (beamsize*smoothsize)
            #bminor = '%sarcsec' % (beamsize*smoothsize)
            qsmoothsize = qa.mul(qbeamsize,self.smoothsize)
        bmajor = qsmoothsize
        bminor = qsmoothsize
        self.convimage = self.image.convolve2d( outfile=self.tmpconvname, major=bmajor, minor=bminor, overwrite=True )
        self.convimage.done()
        self.convimage = ia.newimage( self.tmpconvname )

        # get dTij (original - smoothed)
        if len(imshape) == 4:
            # with polarization axis
            npol = imshape[3]
            for ichan in range(nchan):
                for ipol in range(npol):
                    pixmsk = self.image.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                    pixsmo = self.convimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan,ipol] )
                    pixsub = pixmsk - pixsmo
                    self.convimage.putchunk( pixsub, [0,0,ichan,ipol] )
        elif len(imshape) == 3:
            for ichan in range(nchan):
                # no polarization axis
                pixmsk = self.image.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                pixsmo = self.convimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                pixsub = pixmsk - pixsmo
                self.convimage.putchunk( pixsub, [0,0,ichan] )

        # polynomial fit
        fitaxis = 0
        if self.direction == 0.0:
            fitaxis = 0
        elif self.direction == 90.0:
            fitaxis = 1
        else:
            raise Exception, "Sorry, the task don't support inclined scan with respect to horizontal or vertical axis, right now."
        # Replace duplicated method ia.fitpolynomial with
        # ia.fitprofile 
        #polyimage = convimage.fitpolynomial( fitfile=tmppolyname, axis=fitaxis, order=numpoly, overwrite=True )
        #polyimage.done()
        if os.path.exists( self.tmppolyname ):
            # CAS-5410 Use private tools inside task scripts
            cu = utilstool()
            cu.removetable([self.tmppolyname])
        self.convimage.setbrightnessunit('K')
        resultdic = self.convimage.fitprofile( model=self.tmppolyname, axis=fitaxis, poly=self.numpoly, ngauss=0, multifit=True, gmncomps=0 )
        polyimage = ia.newimage( self.tmppolyname )

        # subtract fitted image from original map
        imageorg = ia.newimage( self.infiles )
        if len(imshape) == 4:
            # with polarization axis
            npol = imshape[3]
            for ichan in range(nchan):
                for ipol in range(npol):
                    pixorg = imageorg.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                    pixpol = polyimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] )
                    pixsub = pixorg - pixpol
                    polyimage.putchunk( pixsub, [0,0,ichan,ipol] )
        elif len(imshape) == 3:
            # no polarization axis
            for ichan in range(nchan):
                pixorg = imageorg.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                pixpol = polyimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                pixsub = pixorg - pixpol
                polyimage.putchunk( pixsub, [0,0,ichan] )

        # output
        polyimage.rename( self.outfile, overwrite=self.overwrite )

        polyimage.done()
        self.convimage.done( remove=True )
        self.image.done()
        imageorg.done()

    def __execute_basket_weaving(self):
        ###
        # Basket-Weaving (Emerson & Grave 1988)
        ###
        casalog.post( 'Apply Basket-Weaving' )

        # CAS-5410 Use private tools inside task scripts
        ia = gentools(['ia'])[0]

        # initial setup
        outimage = ia.newimagefromimage( infile=self.infiles[0], outfile=self.outfile, overwrite=self.overwrite )
        imshape = outimage.shape()
        nx = imshape[0]
        ny = imshape[1]
        nchan = imshape[2]
        tmp=[]
        nfile = len(self.infiles)
        for i in xrange(nfile):
            tmp.append(numpy.zeros(imshape,dtype=float))
        maskedpixel=numpy.array(tmp)
        del tmp

        # direction
        dirs = []
        if len(self.direction) == nfile:
            dirs = self.direction
        else:
            casalog.post( 'direction information is extrapolated.' )
            for i in range(nfile):
                dirs.append(self.direction[i%len(direction)])

        # masklist
        masks = []
        if type(self.masklist) == float:
            for i in range(nfile):
                masks.append( self.masklist )
        elif type(self.masklist) == list and nfile != len(self.masklist):
            for i in range(nfile):
                masks.append( self.masklist[i%len(self.masklist)] )
        for i in range(len(masks)):
            masks[i] = 0.01 * masks[i]
        
        # mask
        for i in range(nfile):
            self.realimage = ia.newimagefromimage( infile=self.infiles[i], outfile=self.tmprealname[i] )
            self.imagimage = ia.newimagefromimage( infile=self.infiles[i], outfile=self.tmpimagname[i] )
            self.realimage.close()
            self.imagimage.close()
        if len(self.thresh) == 0:
            casalog.post( 'Use whole region' )
        else:
            if len(imshape) == 4:
                # with polarization axis
                npol = imshape[3]
                for i in range(nfile):
                    self.realimage = ia.newimage( self.tmprealname[i] )
                    for ichan in range(nchan):
                        for ipol in range(npol):
                            pixmsk = self.realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol])
                            for ix in range(pixmsk.shape[0]):
                                for iy in range(pixmsk.shape[1]):
                                    if self.thresh[0] == self.nolimit:
                                        if pixmsk[ix][iy] > self.thresh[1]:
                                            maskedpixel[i][ix][iy][ichan][ipol]=pixmsk[ix][iy]
                                            pixmsk[ix][iy] = 0.0
                                    elif self.thresh[1] == self.nolimit:
                                        if pixmsk[ix][iy] < self.thresh[0]:
                                            maskedpixel[i][ix][iy][ichan][ipol]=pixmsk[ix][iy]
                                            pixmsk[ix][iy] = 0.0
                                    else:
                                        if pixmsk[ix][iy] < self.thresh[0] or pixmsk[ix][iy] > self.thresh[1]:
                                            maskedpixel[i][ix][iy][ichan][ipol]=pixmsk[ix][iy]
                                            pixmsk[ix][iy] = 0.0
                            self.realimage.putchunk( pixmsk, [0,0,ichan,ipol] )
                    self.realimage.close()
            elif len(imshape) == 3:
                # no polarization axis
                for i in range(nfile):
                    self.realimage = ia.newimage( self.tmprealname[i] )
                    for ichan in range(nchan):
                        pixmsk = self.realimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan])
                        for ix in range(pixmsk.shape[0]):
                            for iy in range(pixmsk.shape[1]):
                                if self.thresh[0] == self.nolimit:
                                    if pixmsk[ix][iy] > self.thresh[1]:
                                        maskedpixel[i][ix][iy][ichan]=pixmsk[ix][iy]
                                        pixmsk[ix][iy] = 0.0
                                elif self.thresh[1] == self.nolimit:
                                    if pixmsk[ix][iy] < self.thresh[0]:
                                        maskedpixel[i][ix][iy][ichan]=pixmsk[ix][iy]
                                        pixmsk[ix][iy] = 0.0
                                else:
                                    if pixmsk[ix][iy] < self.thresh[0] or pixmsk[ix][iy] > self.thresh[1]:
                                        maskedpixel[i][ix][iy][ichan]=pixmsk[ix][iy]
                                        pixmsk[ix][iy] = 0.0
                        self.realimage.putchunk( pixmsk, [0,0,ichan] )
                    self.realimage.close()
        maskedvalue=None
        if any(maskedpixel.flatten()!=0.0):
                maskedvalue=maskedpixel.mean(axis=0)
        del maskedpixel

        # set weight factor
        weights = numpy.ones( shape=(nfile,nx,ny), dtype=float )
        eps = 1.0e-5
        dtor = numpy.pi / 180.0
        for i in range(nfile):
            if abs(numpy.sin(dirs[i]*dtor)) < eps:
                # direction is around 0 deg
                maskw = 0.5 * nx * masks[i] 
                for ix in range(nx):
                    for iy in range(ny):
                        dd = abs( float(ix) - 0.5 * (nx-1) )
                        if dd < maskw:
                            cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                            weights[i][ix][iy] = 1.0 - cosd * cosd
                        if weights[i][ix][iy] == 0.0:
                            weights[i][ix][iy] += eps*0.01
            elif abs(numpy.cos(dirs[i]*dtor)) < eps:
                # direction is around 90 deg
                maskw = 0.5 * ny * masks[i]
                for ix in range(nx):
                    for iy in range(ny):
                        dd = abs( float(iy) - 0.5 * (ny-1) )
                        if dd < maskw:
                            cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                            weights[i][ix][iy] = 1.0 - cosd * cosd
                        if weights[i][ix][iy] == 0.0:
                            weights[i][ix][iy] += eps*0.01
            else:
                maskw = 0.5 * sqrt( nx * ny ) * masks[i]
                for ix in range(nx):
                    for iy in range(ny):
                        tand = numpy.tan((dirs[i]-90.0)*dtor)
                        dd = abs( ix * tand - iy - 0.5 * (nx-1) * tand + 0.5 * (ny-1) )
                        dd = dd / sqrt( 1.0 + tand * tand )
                        if dd < maskw:
                            cosd = numpy.cos(0.5*numpy.pi*dd/maskw)
                            weights[i][ix][iy] = 1.0 - cosd * cosd
                        if weights[i][ix][iy] == 0.0:
                            weights[i][ix][iy] += eps*0.01 
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
            for i in range(nfile):
                self.realimage = ia.newimage( self.tmprealname[i] )
                self.imagimage = ia.newimage( self.tmpimagname[i] )
                for ichan in range(nchan):
                    for ipol in range(npol):
                        pixval = self.realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] )
                        pixval = pixval.reshape((nx,ny))
                        pixfft = npfft.fft2( pixval )
                        pixfft = pixfft.reshape((nx,ny,1,1))
                        self.realimage.putchunk( pixfft.real, [0,0,ichan,ipol] )
                        self.imagimage.putchunk( pixfft.imag, [0,0,ichan,ipol] )
                        del pixval, pixfft
                self.realimage.close()
                self.imagimage.close()
        elif len(imshape) == 3:
            # no polarization axis
            for i in range(nfile):
                self.realimage = ia.newimage( self.tmprealname[i] )
                self.imagimage = ia.newimage( self.tmpimagname[i] )
                for ichan in range(nchan):
                    pixval = self.realimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                    pixval = pixval.reshape((nx,ny))
                    pixfft = npfft.fft2( pixval )
                    pixfft = pixfft.reshape((nx,ny,1))
                    self.realimage.putchunk( pixfft.real, [0,0,ichan] )
                    self.imagimage.putchunk( pixfft.imag, [0,0,ichan] )
                    del pixval, pixfft
                self.realimage.close()
                self.imagimage.close()

        # weighted mean
        if len(imshape) == 4:
            npol = imshape[3]
            for ichan in range(nchan):
                for ipol in range(npol):
                    pixout = numpy.zeros( shape=(nx,ny), dtype=complex )
                    denom = numpy.zeros( shape=(nx,ny), dtype=float )
                    for i in range(nfile):
                        self.realimage = ia.newimage( self.tmprealname[i] )
                        self.imagimage = ia.newimage( self.tmpimagname[i] )
                        pixval = self.realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) + self.imagimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) * 1.0j
                        pixval = pixval.reshape((nx,ny))
                        pixout = pixout + pixval * weights[i]
                        denom = denom + weights[i]
                        self.realimage.close()
                        self.imagimage.close()
                    pixout = pixout / denom
                    pixout = pixout.reshape((nx,ny,1,1))
                    self.realimage = ia.newimage( self.tmprealname[0] )
                    self.imagimage = ia.newimage( self.tmpimagname[0] )
                    self.realimage.putchunk( pixout.real, [0,0,ichan,ipol] )
                    self.imagimage.putchunk( pixout.imag, [0,0,ichan,ipol] )
                    self.realimage.close()
                    self.imagimage.close()
        elif len(imshape) == 3:
            for ichan in range(nchan):
                pixout = numpy.zeros( shape=(nx,ny), dtype=complex )
                denom = numpy.zeros( shape=(nx,ny), dtype=float )
                for i in range(nfile):
                    self.realimage = ia.newimage( self.tmprealname[i] )
                    self.imagimage = ia.newimage( self.tmpimagname[i] )
                    pixval = self.realimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] )
                    pixval = pixval.reshape((nx,ny))
                    pixout = pixout + pixval * weights[i]
                    denom = denom + weights[i]
                    self.realimage.close()
                    self.imagimage.close()
                pixout = pixout / denom
                pixout = pixout.reshape((nx,ny,1))
                self.realimage = ia.newimage( self.tmprealname[0] )
                self.imagimage = ia.newimage( self.tmpimagname[0] )
                self.realimage.putchunk( pixout.real, [0,0,ichan] )
                self.imagimage.putchunk( pixout.imag, [0,0,ichan] )
                self.realimage.close()
                self.imagimage.close()

        # inverse FFT
        self.realimage = ia.newimage( self.tmprealname[0] )
        self.imagimage = ia.newimage( self.tmpimagname[0] )
        if len(imshape) == 4:
            npol = imshape[3]
            for ichan in range(nchan):
                for ipol in range(npol):
                    pixval = self.realimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) + self.imagimage.getchunk( [0,0,ichan,ipol], [nx-1,ny-1,ichan,ipol] ) * 1.0j
                    pixval = pixval.reshape((nx,ny))
                    pixifft = npfft.ifft2( pixval )
                    pixifft = pixifft.reshape((nx,ny,1,1))
                    outimage.putchunk( pixifft.real, [0,0,ichan,ipol] )
                    del pixval, pixifft
        elif len(imshape) == 3:
            for ichan in range(nchan):
                pixval = self.realimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] ) + self.imagimage.getchunk( [0,0,ichan], [nx-1,ny-1,ichan] ) * 1.0j
                pixval = pixval.reshape((nx,ny))
                pixifft = npfft.ifft2( pixval )
                pixifft = pixifft.reshape((nx,ny,1))
                outimage.putchunk( pixifft.real, [0,0,ichan] )
                del pixval, pixifft
        if maskedvalue is not None:
            outimage.putchunk(outimage.getchunk()+maskedvalue)
        self.realimage.close()
        self.imagimage.close()
        outimage.close()

    def finalize(self):
        pass

    def cleanup(self):
        # finalize image analysis tool
        if hasattr(self,'image') and self.image is not None:
            if self.image.isopen(): self.image.done()
        tools = ['convimage', 'imageorg', 'realimage', 'imagimage']
        for t in tools:
            if hasattr(self,t):
                v = getattr(self,t)
                if v and v.isopen():
                    v.done(remove=True)

        # remove tempolary files
        filelist = ['tmpmskname', 'tmpconvname', 'tmppolyname',
                    'tmprealname', 'tmpimagname']
        existing_files = []
        for s in filelist:
            if hasattr(self, s):
                f = getattr(self, s)
                if isinstance(f,list):
                    for g in f:
                        if os.path.exists(g):
                            existing_files.append(g)
                else:
                    if os.path.exists(f):
                        existing_files.append(f)
        # CAS-5410 Use private tools inside task scripts
        cu = utilstool()
        cu.removetable(existing_files)
    
