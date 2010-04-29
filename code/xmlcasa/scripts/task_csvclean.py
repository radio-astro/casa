import os
import shutil
import traceback
from cleanhelper import *
from taskinit import *


def csvclean(vis, imagename,field, spw, imsize, cell, niter, weighting, restoringbeam, interactive):

    """ Create a clean image using Hogbom and restore the residuals
    
         vis -- Name of input visibility file
               default: none; example: vis='ngc5921.ms'    
    
         imagename -- Pre-name of output images:
               default: none; example: imagename='m2'
               output images are:
                 m2.image; cleaned and restored image
                        With or without primary beam correction
                 m2.psf; point-spread function (dirty beam)
                 m2.flux;  relative sky sensitivity over field
                 m2.flux.pbcoverage;  relative pb coverage over field 
                                      (gets created only for ft='mosaic')
                 m2.model; image of clean components
                 m2.residual; image of residuals
                 m2.interactive.mask; image containing clean regions

    
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
                   example: restoringbeam=['10arcsec'] or '10arcsec' circular Gaussian 
                            FWHM 10 arcseconds example:
                            restoringbeam=['10.0','5.0','45.0deg'] 10"x5" 
                            at 45 degrees
        
        interactive -- Create a mask interactively or not.
                        default=False; example: interactive=True
            
    """
    

    #Python script    
    
    try:
        casalog.origin('csvclean')
    
        parsummary = 'vis="'+str(vis)+'", imagename="'+str(imagename)+'", '
        parsummary += 'field="'+str(field)+'", spw="'+str(spw)+'", '
        parsummary += 'imsize="'+str(imsize)+'", niter="'+str(niter)+'", '
        parsummary += 'weighting="'+str(weighting)+'", '
        parsummary += 'restoringbeam="'+str(restoringbeam)+'", '
        parsummary += 'interactive="'+str(interactive)+'"'
        casalog.post(parsummary,'INFO')    
        
        if (not (type(vis)==str) & (os.path.exists(vis))):
            raise Exception, 'Visibility data set not found - please verify the name'
    
        if (imagename == ""):
            raise Exception, "Must provide output image name in parameter imagename."            
        
        if os.path.exists(imagename):
            raise Exception, "Output image %s already exists - will not overwrite." % imagename
           
        if (field == ''):
        	field = '*'
        	
        if (spw == ''):
        	spw = '*'

        if ((type(imsize)==int)):
            imsize=[imsize,imsize]
    
        if ((len(imsize)==1)): 
            imsize=[imsize[0],imsize[0]]
        
        nx = imsize[0]
        ny = imsize[1]
      
        if ((type(cell)==int) | (type(cell)==float) | (type(cell)==str)):
            cell=[cell,cell]
               
        if ((len(cell)==1)):
            cell=[cell[0],cell[0]]

        cellx=cell[0]
        celly=cell[1]
        if((type(cell[0])==int) or (type(cell[0])==float)):
            cellx=qa.quantity(cell[0], 'arcsec')
            celly=qa.quantity(cell[1], 'arcsec')

        if restoringbeam == [''] or len(restoringbeam) == 0:
        	# calculate from fit below
            bmaj = ''
            bmin = ''
            bpa = ''
        else:        	
	        if (type(restoringbeam)==str):
	            restoringbeam=[restoringbeam,restoringbeam,'0deg']
	        if (type(restoringbeam)==list and (len(restoringbeam)==1)):
	            restoringbeam=[restoringbeam[0],restoringbeam[0],'0deg']
	        if (type(restoringbeam)==list and (len(restoringbeam)==2)):
	            restoringbeam=[restoringbeam[0],restoringbeam[1],'0deg']
	        if (type(restoringbeam)==list and (len(restoringbeam)==2)):
	            restoringbeam=[restoringbeam[0],restoringbeam[1],restoringbeam[2]]
	
	        if(qa.quantity(restoringbeam[0])['unit'] == ''):
	        	restoringbeam[0]=restoringbeam[0]+'arcsec'
	        if(qa.quantity(restoringbeam[1])['unit'] == ''):
	        	restoringbeam[1]=restoringbeam[1]+'arcsec'
	        if(qa.quantity(restoringbeam[2])['unit'] == ''):
	        	restoringbeam[2]=restoringbeam[2]+'deg'
	        
	        bmaj = restoringbeam[0]
	        bmin = restoringbeam[1]
	        bpa = restoringbeam[2]
                   
        # Create output names based on imagename parameter
        dirtyim = imagename+'dirty.image'
        psfim = imagename+'psf.image'
        modelname = imagename+'.model'
        imname = imagename+'.image'

        # Make sure all tables and images are closed
        tb.close()
        im.close()
        ia.close()
        dc.close()
        
        # Add scratch columns if they don't exist
        tb.open(vis)
        hasit = tb.colnames().count('CORRECTED_DATA')>0
        tb.close()
        if not hasit:
        	cb.open(vis)
        	cb.close()
        		
        # make the dirty image and psf
        im.open(vis)
        im.selectvis(spw=spw, field=field, usescratch=True)
        im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly)
        im.weight(weighting)
        
        im.makeimage(type='corrected', image=dirtyim)
        im.makeimage(type='psf', image=psfim)
        im.done()

        # Calculate bpa, bmin, bmaj if not given
        if restoringbeam == [''] or len(restoringbeam) == 0:
        	cx = nx/2
        	cy = ny/2
        	box = ''

        	if (nx > 100 and ny > 100):
        		rg = [cx-10, cy-10, cx+10, cy+10]
        		box = '%s,%s,%s,%s'%(rg[0],rg[1],rg[2],rg[3])
            	ia.open(psfim)            	
            	coords = ia.fitcomponents(box=box)
            	ia.close()
            	if(coords['converged'] == True):
	            	bmaj = coords['results']['component0']['shape']['majoraxis']['value']
	            	bmin = coords['results']['component0']['shape']['minoraxis']['value']
	            	bpa = coords['results']['component0']['shape']['positionangle']['value']
	            	bmaj = str(bmaj)+'arcsec'
	            	bmin = str(bmin)+'arcsec'
	            	bpa = str(bpa)+'deg'
            	
        parsummary = 'restoringbeam values = [\'%s\',\'%s\',\'%s\']'%(bmaj,bmin,bpa)
        casalog.post(parsummary,'INFO')
        
        # Make a mask
        maskname=''
        if(interactive):
            maskname=imagename+'.mask'
            im.drawmask(dirtyim, maskname)

        # use deconvolver to do image plane deconvolution
        # using a mask image as the mask
        dc.open(dirty=dirtyim, psf=psfim)
        # NOTE: use the parameter mask which can be an empty
        #       string if no mask
        dc.clean(niter=niter, model=modelname, mask=maskname)
        
        # create the restored image
        if restoringbeam == [''] or len(restoringbeam) == 0:
        	dc.restore(model=modelname, image=imname)
        else:
			dc.restore(model=modelname, image=imname, bmaj=bmaj, bmin=bmin, bpa=bpa)  
			
        dc.done()  
        return True

    except Exception, instance:
        print '*** Error *** ',instance
        casalog.post("Error ...", 'SEVERE')
        traceback.print_exc()
        raise Exception, instance

    

