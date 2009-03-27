import os
from taskinit import *

def deconvolve(imagename,model,psf,alg,niter,gain,threshold,mask,scales,sigma,targetflux,prior):
    """ deconvolve: Image based deconvolver. The psf provided is
    deconvolved out of the image provided.

    	Keyword arguments:
    	imagename -- Name of input image to be deconvoled
    	model     -- Name of output to store found model, a.k.a clean components
    	psf       -- Name of psf image to use e.g  psf='mypsf.image' .
     	             But if the psf has 3 parameter, then
       		     a gaussian psf is assumed with the values representing
                     the major , minor and position angle  values
                     e.g  psf=['3arcsec', '2.5arcsec', '10deg']
    	alg       -- algorithm to use clark, hogbom or multiscale or mem. if multiscale 
       	             the parameter scale is used to define the number of scales.
    	niter     -- Number of iteration
    	gain      -- CLEAN gain parameter; fraction to remove from peak (< 1.0)
    	threshold -- deconvolution stopping threshold: if no peak above
       	             found above this level 
    	mask      -- mask image (same shape as image and psf) to limit region
       	             where deconvoltion is to occur
    	------parameters useful for multiscale only
    	scales     -- parameter needed for multiscale clean. default value [0,3,10]
    	------parameters useful for mem only
    	sigma     -- Estimated noise for image
    	targetflux -- Target total flux in image 
    	prior     -- Prior image to guide mem

    """
    
    try:
	casalog.origin('deconvolve')
        
        tmppsf=''
        tmpImagename=''
        if(len(psf)==0):
            raise Exception, "****give some psf please****"
        if(len(psf)==1):
            if (type(psf[0])==str and os.path.exists(psf[0])):
                psf=psf[0]
                ia.open(psf)
                psfaxis=len(ia.shape())
                ia.close()
                if(psfaxis<4):
                    modPsf=_add_axes(psf)
                    if modPsf is False:
                        raise Exception, "****problem with input psf image****"
                    else:
                        tmppsf='__decon_tmp_psf'
                        ia.fromimage(tmppsf,modPsf)
                        ia.close()
                        psf=tmppsf
                ia.open(imagename)
                imnaxis=len(ia.shape())
                ia.close()
                if(imnaxis<4):
                    tmpImagename=_add_axes(imagename)
                    if tmpImagename is False:
                        raise Exception, "****problem with input diry image****"
                    else:
                        imagename=tmpImagename
             
            else:
                raise Exception, "****psf file, %s does not exist****" % psf
            
        if(len(psf)==3):
            #We've got bmaj bmin bpa
            # add axes if the input dirty image does not have four axes
            ia.open(imagename)
            imnaxis=len(ia.shape())
            #csys=ia.coordsys()
            ia.close()
            if(imnaxis<4):
               tmpImagename=_add_axes(imagename)
               if tmpImagename is False:
                   raise Exception, "****problem with input diry image****"
               else:
                   imagename=tmpImagename
            tmppsf=model+'.psf'
            dc.open(imagename,psf='')
            print 
            dc.makegaussian(tmppsf,bmaj=psf[0],bmin=psf[1],
                            bpa=psf[2], normalize=False)
            dc.close()
            psf=tmppsf
        dc.open(imagename,psf=psf)
        if(alg=='multiscale'):
            dc.setscales(scalemethod='uservector', uservector=scales)
        if((alg=='hogbom') or (alg=='multiscale')):
            dc.clean(algorithm=alg, model=model, niter=niter, gain=gain,
                     mask=mask, threshold=qa.quantity(threshold, 'mJy'))
        elif(alg=='clark'):
            dc.clarkclean(niter=niter, model=model, mask=mask,
                          gain=gain, threshold=qa.quantity(threshold, 'mJy'))
        elif(alg=='mem'):
            dc.mem(niter=niter, sigma=sigma, targetflux=targetflux,
                   model=model, prior=prior)
        else:
            raise Exception, '****algorithm %s is not known****'%alg
        dc.restore(model=model, image=model+'.restored')
        dc.residual(model=model, image=model+'.residual')
        dc.close()    
        #if(len(tmppsf) != 0):
        #    os.system('rm -rf '+tmppsf)
        if(len(tmpImagename) != 0):
            #os.system('rm -rf '+tmpImagename)
            ia.removefile(tmpImagename)
        tb.clearlocks()
    except Exception, instance:
        print '*** Error ***',instance
        ###lets try to close if we can
        try:
            tb.clearlocks()
            dc.close()
        except: pass


# helper function to add degenerate axes
def _add_axes(inImage):
        tmpim=''
        tmpim2=''
        outImage=''
	ok=ia.open(inImage)
        if ok:
            tmpim='__decon_tmp_im'
            csys=ia.coordsys()
            isStokes=csys.findcoordinate('stokes')['return'] 
            isSpectral=csys.findcoordinate('spectral')['return'] 
            if not isStokes:
                ia.open(inImage)
                ia.adddegaxes(tmpim, stokes="I", overwrite=True)
                ia.close() 
                if not isSpectral:
                    tmpim2='__decon_tmp_im2'
                    ia.open(tmpim)
                    ia.adddegaxes(tmpim2, spectral=True, overwrite=True)
                    ia.remove()
                    outImage=tmpim2
                    
                else:
                    outImage=tmpim
            elif not isSpectral:
                ia.open(inImage)
                ia.adddegaxes(tmpim, spectral=True, overwrite=True)
                ia.close() 
                outImage=tmpim
            else:
                outImage=inImage 
            return outImage 
        else:
           return False
