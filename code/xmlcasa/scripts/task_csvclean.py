import os
import shutil
from cleanhelper import *
from taskinit import *

def getbeams(restoringbeam):
	beams = []
	if((restoringbeam == ['']) or (len(restoringbeam) ==0)):
		return []
	resbmaj=''
	resbmin=''
	resbpa='0deg'
	if((type(restoringbeam) == list)  and len(restoringbeam)==1):
		restoringbeam=restoringbeam[0]
        
	if((type(restoringbeam)==str)):
		if(qa.quantity(restoringbeam)['unit'] == ''):
			restoringbeam=restoringbeam+'arcsec'
		resbmaj=qa.quantity(restoringbeam, 'arcsec')
		resbmin=qa.quantity(restoringbeam, 'arcsec')
        
	if(type(restoringbeam)==list):        
		resbmaj=qa.quantity(restoringbeam[0], 'arcsec')
		resbmin=qa.quantity(restoringbeam[1], 'arcsec')
		if(resbmaj['unit']==''):
			resbmaj=restoringbeam[0]+'arcsec'
        if(resbmin['unit']==''):
            resbmin=restoringbeam[1]+'arcsec'
        if(len(restoringbeam)==3):
            resbpa=qa.quantity(restoringbeam[2], 'deg')
            if(resbpa['unit']==''):
                resbmin=restoringbeam[2]+'deg'

	if((resbmaj != '') and (resbmin != '')): 
		im.setbeam(resbmaj, resbmin, resbpa)  
        beams = [resbmaj,resbmin, resbpa]
        print '**************************************************'
        print beams
        return beams
            
#	im.setbeam(resbmaj, resbmin, resbpa)  #
	beams = [resbmaj,resbmin, resbpa]#
	return beams

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
                   example: restoringbeam=['10arcsec'] circular Gaussian 
                            FWHM 10 arcseconds example:
                            restoringbeam=['10.0','5.0','45.0deg'] 10"x5" 
                            at 45 degrees
        
        interactive -- True expandable parameters - Not yet implemented!!!
                   npercycle -- this is the  number of iterations between each
                     interactive update of the mask.  It is important to modify
                     this number interactively during the cleaning, starting with
                     a low number like 20, but then increasing as more extended
                     emission is encountered.
                   chaniter -- specify how interactive CLEAN is performed, 
                     default: chaniter=False;
                     example: chaniter=True; step through channels 
                      WARNING: The interactive clean with chaniter=True for
                        optical velocity mode
                        (mode='velocity' and veltype='optical') is
                        NOT YET IMPLEMENTED.
            
    """
    

    #Python script	
	
    try:
        casalog.origin('csvclean')
    
        parsummary = 'vis="'+str(vis)+'", imagename="'+str(imagename)+'", '
        parsummary += 'field="'+str(field)+'", spw="'+str(spw)+'", '
        parsummary += 'imsize="'+str(imsize)+'", niter="'+str(niter)+'", '
        parsummary += 'weighting="'+str(weighting)+'", '
        parsummary += 'restoringbeam="'+str(restoringbeam)+'", '
        casalog.post(parsummary,'INFO')    
        
        if ((type(vis)==str) & (os.path.exists(vis))):
            im.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
    
        if (imagename == ""):
            raise Exception, "Must provide output image name in parameter imagename."            
        
        if os.path.exists(imagename):
            raise Exception, "Output image %s already exists - will not overwrite." % imagename

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

#        imCln=imtool.create()
#        imset=cleanhelper(imCln, vis)
#        imset.setrestoringbeam(restoringbeam)
           
        	
        # Create output names based on imagename parameter
        dirtyim = imagename+'dirty.image'
        psfim = imagename+'psf.image'
        modelname = imagename+'.model'
        imname = imagename+'.image'
        
        # make the dirty image and psf
        im.selectvis(vis=vis, spw=spw, field=field, usescratch=True)
        im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly)
        im.weight(weighting)
        comp = getbeams(restoringbeam)
        if (comp == []):
            raise Exception, "restoringbeam is empty or invalid"
        bmaj = comp[0]
        bmin = comp[1]
        bpa = comp[2]
        im.setbeam(bmaj,bmin,bpa)
        im.makeimage(type='corrected', image=dirtyim)
        im.makeimage(type='psf', image=psfim)
        im.done()
    
        # use deconvolver to do image plane deconvolution
        # using a mask image as the mask
        dc.open(dirty=dirtyim, psf=psfim)
        dc.clean(niter=niter, model=modelname)
        
        # create the restored image
        dc.restore(model=modelname, image=imname, bmaj=bmaj, bmin=bmin, bpa=bpa)    
#        dc.restore(model=modelname, image=imname)    
#        del imCln

        # Write history to output MS
#        ms.open(outputvis, nomodify=False)
#        ms.writehistory(message='taskname=csvclean', origin='csvclean')
#        ms.writehistory(message='vis         = "'+str(vis)+'"',
#                origin='csvclean')
#        ms.writehistory(message='imagename   = "'+str(imagename)+'"',
#                origin='csvclean')
#        ms.writehistory(message='field       = "'+str(field)+'"',
#                origin='csvclean')
#        ms.writehistory(message='spw         = '+str(spw), origin='csvclean')
#        ms.writehistory(message='nx     = "'+str(x)+'"',
#                origin='csvclean')
#        ms.writehistory(message='ny   = "'+str(ny)+'"',
#                origin='csvclean')
#        ms.writehistory(message='cellx        = '+str(cellx), origin='csvclean')
#        ms.writehistory(message='celly       = '+str(celly), origin='csvclean')
#        ms.writehistory(message='weight       = '+str(weight), origin='csvclean')
#        ms.writehistory(message='algorithm       = '+str(algorithm), origin='csvclean')
#        ms.close()
    
        return True

    except Exception, instance:
        casalog.post("Error ...", 'SEVERE')
        try:
            ms.close()
        except:
            casalog.post("MS closed.", 'INFO')
    raise Exception, instance
    

