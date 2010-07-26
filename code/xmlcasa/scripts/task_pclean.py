from parallel.pcont import *
import pdb
import shutil
import os



def pclean(vis, imagename, field, spw, imsize, cell,
           phasecenter, niter, weighting, restoringbeam,
           ftmachine, alg, wprojplanes, facats,
           majorcycles, painc, palimit, popbcorr,
           applyoffsets, cfcache, epjtablename,
           contclean, visinmem, mode,
           start, nchan, step, imagetilevol, chanchunk, scales, 
           hostnames, numcpuperhost):
              

    """ This task does an invert of the visibilities and deconvolve in the
	    image plane. It does not do a uvdata subtraction (aka Cotton-Schwab
		major cycle) of model visibility as in clean. - For ALMA Commissioning
    
         vis -- Name of input visibility file
               default: none; example: vis='ngc5921.ms'    
    
	     imagename -- Name of output CASA image. (only the prefix)
                   default: none; example: imagename='m2'
                   output images are:
                 m2.image; cleaned and restored image
                        With or without primary beam correction
                 m2dirty.image; dirty image
                 m2psf.image; point-spread function (dirty beam)
                 m2.model; image of clean components
                 m2.mask; image containing clean regions, when interative=True

    
         field -- Select fields in MS.  Use field id(s) or field name(s).
                    ['go listobs' to obtain the list id's or names]
                default: ''= all fields
                If field string is a non-negative integer, it is assumed to
                    be a field index otherwise, it is assumed to be a 
                    field name
                field='0~2'; field ids 0,1,2
                field='0,4,5~7'; field ids 0,4,5,6,7
                field='3C286,3C295'; field named 3C286 and 3C295
                field = '3,4C*'; field id 3, all names starting with 4C
    
         spw --Select spectral window/channels
                NOTE: This selects the data passed as the INPUT to mode
                default: ''=all spectral windows and channels
                  spw='0~2,4'; spectral windows 0,1,2,4 (all channels)
                  spw='0:5~61'; spw 0, channels 5 to 61
                  spw='<2';   spectral windows less than 2 (i.e. 0,1)
                  spw='0,10,3:3~45'; spw 0,10 all channels, spw 3, 
                                     channels 3 to 45.
                  spw='0~2:2~6'; spw 0,1,2 with channels 2 through 6 in each.
                  spw='0:0~10;15~60'; spectral window 0 with channels 
                                      0-10,15-60
                  spw='0:0~10,1:20~30,2:1;2;3'; spw 0, channels 0-10,
                        spw 1, channels 20-30, and spw 2, channels, 1,2 and 3

        imsize -- Image pixel size (x,y).  DOES NOT HAVE TO BE A POWER OF 2
                   default = [256,256]; example: imsize=[350,350]
                   imsize = 500 is equivalent to [500,500]
                   Avoid odd-numbered imsize.
    
        cell -- Cell size (x,y)
                   default= '1.0arcsec';
                   example: cell=['0.5arcsec,'0.5arcsec'] or
                   cell=['1arcmin', '1arcmin']
                   cell = '1arcsec' is equivalent to ['1arcsec','1arcsec']
                   NOTE:cell = 2.0 => ['2arcsec', '2arcsec']

        phasecenter -- direction measure  or fieldid for the mosaic center
                   default: '' => first field selected ; example: phasecenter=6
                   or phasecenter='J2000 19h30m00 -40d00m00'
    
        niter -- Maximum number iterations,
                   if niter=0, then no CLEANing is done ("invert" only)
                   default: 500; example: niter=5000
    
        weighting -- Weighting to apply to visibilities:
                   default='natural'; example: weighting='uniform';
                   Options: 'natural','uniform','briggs', 
                           'superuniform','briggsabs','radial'
    
        restoringbeam -- Output Gaussian restoring beam for CLEAN image
                   [bmaj, bmin, bpa] elliptical Gaussian restoring beam
                   default units are in arc-seconds for bmaj,bmin, degrees
                   for bpa default: restoringbeam=[]; Use PSF calculated
                   from dirty beam. 
                   example: restoringbeam=['10arcsec'] or restorinbeam='10arcsec', circular Gaussian.
                            FWHM 10 arcseconds example:
                            restoringbeam=['10.0','5.0','45.0deg'] 10"x5" 
                            at 45 degrees
        
	    interactive -- Create a mask interactively or not.
        		   default=False; example: interactive=True
        		   The viewer will open with the image displayed. Select the
        		   region for the mask and double click in the middle of it.
            
    """

    #Python script    

    for host in hostnames :
       os.system('ssh '+host+' killall -9 ipengine')
       shutil.rmtree(os.environ['HOME']+'/.casa/ipython/security', 
                     True)

    numcores=numcpuperhost
    #pdb.set_trace()


    if spw=='':
       spw='*'

    if imagename=='':
       imagename=vis

    if mode=='continum':
       pcont(msname=vis, 
          imagename=imagename, imsize=imsize, 
          pixsize=cell, phasecenter=phasecenter, 
          field=field, spw=spw, ftmachine=ftmachine, 
          wprojplanes=wprojplanes, 
          hostnames=hostnames, 
          numcpuperhost=numcores, 
          majorcycles=majorcycles, niter=niter, alg=alg,
          weight=weighting,
          contclean=contclean, visinmem=visinmem)
    else:
       pcube(msname=vis,
          imagename=imagename, imsize=imsize, 
          pixsize=cell, phasecenter=phasecenter, 
          field=field, spw=spw, ftmachine=ftmachine, 
          wprojplanes=wprojplanes, 
          hostnames=hostnames, 
          numcpuperhost=numcores, 
          majorcycles=majorcycles, niter=niter, alg=alg,
          weight=weighting, mode=mode, 
          start=start, nchan=nchan, step=step, 
          chanchunk=chanchunk, scales=scales,
          contclean=contclean, visinmem=visinmem)

parallel_clean=pclean
    

