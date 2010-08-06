import os
import shutil
import traceback
import pdb
import numpy as np
import sys
from cleanhelper import *
from taskinit import *
im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,dc,vp=gentools()

def csvclean(vis, imagename,field, spw, advise, mode, nchan, width, imsize, cell, phasecenter, niter, weighting, restoringbeam, interactive):

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
    
    try:

        casalog.origin('csvclean')
        ms = casac.homefinder.find_home_by_name('msHome').create()
        

        parsummary = 'vis="'+str(vis)+'", imagename="'+str(imagename)+'", '
        parsummary += 'field="'+str(field)+'", spw="'+str(spw)+'", '
        parsummary = 'cell="'+str(cell)+'",'
        parsummary = 'phasecenter='+str(phasecenter)+','
        parsummary += 'imsize='+str(imsize)+', niter='+str(niter)+', '
        parsummary += 'weighting="'+str(weighting)+'", '
        parsummary += 'restoringbeam="'+str(restoringbeam)+'", '
        parsummary += 'interactive='+str(interactive)+''
        casalog.post(parsummary,'INFO')    
        
#        if (not (type(vis)==str) & (os.path.exists(vis))):
#            raise Exception, 'Visibility data set not found - please verify the name'
        if ((type(vis)==str) & (os.path.exists(vis))):
            ms.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        if(not advise):
            if (imagename == ""):
                #            ms.close()
                raise Exception, "Must provide output image name in parameter imagename."            
        
            if os.path.exists(imagename):
                #            ms.close()
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

        if(type(phasecenter)==str):
            ### blank means take field[0]
            if (phasecenter==''):
                fieldoo=field
                if(fieldoo==''):
                    fieldoo='0'
                phasecenter=int(ms.msseltoindex(vis,field=fieldoo)['field'][0])
            else:
                tmppc=phasecenter
                try:
                    if(len(ms.msseltoindex(vis, field=phasecenter)['field']) > 0):
                        tmppc = int(ms.msseltoindex(vis,
                                                    field=phasecenter)['field'][0])
                    ##succesful must be string like '0' or 'NGC*'
                except Exception, instance:
                    #failed must be a string type J2000 18h00m00 10d00m00
                    tmppc = phasecenter
                phasecenter = tmppc
                

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
#        ms.close()
        
        # Add scratch columns if they don't exist
        #tb.open(vis)
        #hasit = tb.colnames().count('CORRECTED_DATA')>0
        #tb.close()
        #if not hasit:
        #	cb.open(vis)
        #	cb.close()
        		
        # make the dirty image and psf

        im.open(vis, usescratch=True)
        im.selectvis(spw=spw, field=field)
        spwsel=ms.msseltoindex(vis=vis, spw=spw)['spw']
        ch=ms.msseltoindex(vis=vis, spw=spw)['channel']
        if(nchan < 1):
            nchan=0
            for k in range(len(spwsel)):
                nchan += ch[k,2]-ch[k,1]+1
            nchan=nchan/width
            if(nchan < 1):
                nchan=1
        if(advise):
            tb.open(vis+'/SPECTRAL_WINDOW')
            allreffreq=tb.getcol('REF_FREQUENCY')
            reffreq=0.0
            if(len(allreffreq) > 1):
                reffreq=0.0;
                for f in  allreffreq:
                    reffreq+=f
                reffreq=reffreq/float(len(allreffreq))
            else:
                reffreq=allreffreq[0]
            tb.done()
            tb.open(vis+'/ANTENNA')
            diams=tb.getcol('DISH_DIAMETER')
            diam=np.min(diams)
            tb.done()
            fov=qa.quantity(3.0e8/reffreq/diam, 'rad')
            adv=im.advise(fieldofview=fov)
            cellx=qa.tos(adv['cell'], prec=4)
            celly=cellx
            myf = sys._getframe(len(inspect.stack())-1).f_globals
            myf['cell']=[cellx,cellx]
            myf['imsize']=[adv['pixels'], adv['pixels']]
            nx=ny=adv['pixels']
            myf['advise']=False
            return
        redopsf=True
        redokounter=0
        immode='mfs'
        if(mode=='cube'):
            immode='channel'
        while(redopsf):
            im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly, phasecenter=phasecenter, spw=spwsel.tolist(), mode=immode, start=ch[0,1], step=width, nchan=nchan)
            im.weight(weighting)
            try:
                im.makeimage(type='corrected', image=dirtyim)
                im.makeimage(type='psf', image=psfim)
                ###make an empty model
                im.make(modelname)
                if((redokounter==2) and (np.min(nx,ny) > 25)):
                    #pdb.set_trace()
                    ia.open(psfim)
                    csys=ia.coordsys()
                    rg.setcoordinates(csys=csys.torecord())
                    shp=ia.shape()
                    blc=['10pix', '10pix', '0pix', '0pix']
                    trc=[str(shp[0]-10)+'pix',str(shp[1]-10)+'pix',
                         str(shp[2]-1)+'pix', str(shp[3]-1)+'pix']
                    reg=rg.wbox(blc=blc, trc=trc)
                    ia.set(pixels=0, region=rg.complement(reg))
                    ia.done()
                #im.done()

                # Calculate bpa, bmin, bmaj if not given
                if restoringbeam == [''] or len(restoringbeam) == 0:
                    cx = nx/2
                    cy = ny/2
                    box = ''

                    #if (nx > 100 and ny > 100):
                    #    rrg = [cx-10, cy-10, cx+10, cy+10]
                    #    box = '%s,%s,%s,%s'%(rrg[0],rrg[1],rrg[2],rrg[3])
                    #ia.open(psfim)
                    #shp=ia.shape()
                    #coords = ia.fitcomponents(box=box)
                    #ia.close()
                    #if(coords['converged'] == True):
                coords=im.fitpsf(psfim)
                if(coords['return']):
                    bmaj=coords['bmaj']
                    bmin=coords['bmin']
                    bpa=coords['bpa']
                    redopsf=False
                else:
                    redopsf=True
                    nx=nx+1
                    ny=ny+1
                    redokounter += 1 
                    if(redokounter==3):
                        casalog.post('Failed to find a decent psf','SEVERE')
                        return False
                    else:
                        casalog.post('Trying new image with 1 extra pixel','WARN')
                        
            except :
            	redopsf=True
                nx=nx+1
                ny=ny+1
                redokounter += 1
                if(redokounter==3):
                    casalog.post('Failed to find a decent psf','SEVERE')
                    im.done()
                    return False
                else:
                    casalog.post('Trying new image with 1 extra pixel','WARN')
        im.done()            
        parsummary = 'restoringbeam values = [\'%s\',\'%s\',\'%s\']'%(qa.tos(bmaj),qa.tos(bmin),qa.tos(bpa))
        casalog.post(parsummary,'INFO')
        
        # Make a mask
        maskname=''
        if(interactive):
            maskname=imagename+'.mask'
            if(os.path.exists(maskname)):
                ia.open(dirtyim)
                csys=ia.coordsys()
                shp=ia.shape()
                ia.done()
                ia.open(maskname)
                ia.regrid(outfile='__tmpmask__', shape=shp, csys=csys.torecord(), axes=[0,1])
                ia.remove(True)
                ia.done()
                shutil.move('__tmpmask__', maskname)
            im.drawmask(dirtyim, maskname)

        # use deconvolver to do image plane deconvolution
        # using a mask image as the mask
        dc.open(dirty=dirtyim, psf=psfim)
        # NOTE: use the parameter mask which can be an empty
        #       string if no mask
        dc.clean(niter=niter, model=modelname, mask=maskname)
        
        # create the restored image
        if restoringbeam == [''] or len(restoringbeam) == 0:
            dc.restore(model=modelname, image=imname, bmaj=bmaj, bmin=bmin, bpa=bpa)
        else:
            dc.restore(model=modelname, image=imname, bmaj=restoringbeam[0], bmin=restoringbeam[1], bpa=restoringbeam[2])  
			
        dc.done()  
        return True

    except Exception, instance:
        im.close()
        ia.close()
        dc.close()
        print '*** Error *** ',instance
        casalog.post("Error ...", 'SEVERE')
        traceback.print_exc()
        raise Exception, instance

    

