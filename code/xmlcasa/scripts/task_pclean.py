from parallel.pcont import *
import pdb
import shutil
import os

def pclean(vis,
           imagename,
           imsize,
           cell,
           phasecenter,
           mask,
           field,
           spw,
           ftmachine,
           alg,
           majorcycles,
           niter,
           threshold,
           weighting,
           scales,
           mode,
            start,
            nchan,
            step,
            imagetilevol,
            chanchunk,
            iteractive,
           gridmode,
           wprojplanes,
           facats, 
           painc, 
           palimit,
           dopbcorr, 
           applyoffsets,
           cfcache,
           epjtablename,           
           contclean,
           visinmem,
           hostnames,
           numcpuperhost
          ):
              

    """ Invert and deconvolve images with parallel engines

        Form images from visibilities. Handles continuum and spectral line 
        cubes using module pcont and pcube respectively.
    """
    #Python script    

    for host in hostnames :
       os.system('ssh '+host+' killall -9 ipengine')
       shutil.rmtree(os.environ['HOME']+'/.casa/ipython/security', True)

    numcores=numcpuperhost
    if numcores<1:
       numcores=1
    #pdb.set_trace()

    if spw=='':
       spw='*'

    if imagename=='':
       imagename=vis

    if mode=='continum':
        pcont(msname=vis, imagename=imagename, 
              imsize=imsize, pixsize=cell, 
              phasecenter=phasecenter, 
              field=field, spw=spw, 
              ftmachine=ftmachine, alg=alg, 
              hostnames=hostnames, numcpuperhost=numcores, 
              majorcycles=majorcycles, niter=niter,
              threshold=threshold, weight=weighting, scales=scales,
              wprojplanes=wprojplanes, 
              contclean=contclean, visinmem=visinmem)
    else:
        
        pcube(msname=vis, imagename=imagename, 
              imsize=imsize, pixsize=cell, 
              phasecenter=phasecenter, 
              field=field, spw=spw, 
              ftmachine=ftmachine, alg=alg,
              hostnames=hostnames, numcpuperhost=numcores, 
              majorcycles=majorcycles, niter=niter, 
              threshold=threshold, weight=weighting, scales=scales,
              mode=mode, 
              wprojplanes=wprojplanes, 
              start=start, nchan=nchan, step=step, 
              chanchunk=chanchunk, 
              contclean=contclean, visinmem=visinmem)
parallel_clean=pclean
    

