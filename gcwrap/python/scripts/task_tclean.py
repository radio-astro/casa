################################################
# Refactored Clean task
#
# v1.0: 2012.10.05, U.R.V.
#
################################################

from taskinit import *

import os
import shutil
import numpy
from taskinit import *
import copy

from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager,PyParallelCubeSynthesisImager, ImagerParameters

def tclean(
    ####### Data Selection
    vis='', 
    field='', 
    spw='',
    scan='',
    ####### Image definition
    imagename='',
    nchan=1,
    imsize=[100,100],
    cellsize=['1.0arcsec','1.0arcsec'],
    phasecenter='J2000 19:59:28.500 +40.44.01.50',
    outlierfile='',
    ####### Gridding parameters
    ftmachine='GridFT', 
    ####### Deconvolution parameters
    algo='hogbom',
    niter=0, 
    cycleniter=0, 
    cyclefactor=1.0,
    minpsffraction=0.1,
    maxpsffraction=0.8,
    threshold=0.0, 
    loopgain=0.1,
    ####### State parameters
    action='makeresidual',
    usescratch=True, 
    interactive=False, 
    pmajor=False,
    clusterdef=''):
    
    from refimagerhelper import ImagerParameters

    # Put all parameters into dictionaries and check them. 
    paramList = ImagerParameters(msname =vis,field=field,spw=spw,scan=scan,
                                 usescratch=usescratch,
                                 outlierfile=outlierfile,
                                 imagename=imagename,nchan=nchan,imsize=imsize,
                                 cellsize=cellsize, phasecenter=phasecenter,
                                 ftmachine=ftmachine,weighting='natural',
                                 algo=algo,
                                 niter=niter,cycleniter=cycleniter,cyclefactor=cyclefactor,
                                 minpsffraction=minpsffraction, maxpsffraction=maxpsffraction,
                                 threshold=threshold,loopgain=loopgain,
                                 interactive=interactive)
    
    if paramList.checkParameters() == False:
        return False

    paramList.printParameters()

    pcube=False
    if pmajor==True and nchan>1:
        pcube=True
        pmajor=False

    ## Setup Imager objects, for different parallelization schemes.
    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params=paramList)
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params=paramList,clusterdef=clusterdef)
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params=paramList,clusterdef=clusterdef)
    else:
         print 'Invalid parallel combination in doClean.'
         return False

    ####################################################
    ##
    ##  multicycle : Full set of major and minor cycle iterations.
    ##  makepsf : Make psf, weight, sumwt
    ##  makeresidual : Make psf, weight, sumwt, dirty/residual image. Use starting model if supplied
    ##                         If psf,weight,sumwt already exist, detect and don't recompute.
    ##  deconvolve : Only minor cycle
    ##  setmodel : Only predict starting model (evaluate or virtual)
    ##  restore : Only setup deconvolvers, and restore
    ##
    ####################################################
    
    ## Init major cycle elements
    if action=='makeresidual' or action=='makepsf' or action=='multicycle' or action=='setmodel':
        imager.initializeImagers()
        if action != 'setmodel':
            imager.initializeNormalizers()

    ## Init minor cycle elements
    if action=='deconvolve' or action=='multicycle' or action=='restore':
        imager.initializeDeconvolvers()
        if action != 'restore':
            imager.initializeIterationControl()

    ## Make PSF
    if action=='makeresidual' or action=='makepsf' or action=='multicycle':
        imager.makePSF()

    ## Make dirty image
    if action=='makeresidual' or action=='multicycle':
        imager.runMajorCycle()

    ## Predict model (independent call)
    if action=='setmodel':
        imager.predictModel()

    ## Do deconvolution and restore
    if action=='multicycle' or action=='deconvolve':
        while ( not imager.hasConverged() ):
            imager.runMinorCycle()
            if action=='multicycle':
                imager.runMajorCycle()

    ## Restore images.
    if action=='multicycle' or action=='deconvolve' or action=='restore':
        imager.restoreImages()


    retrec={}
    if action=='multicycle' or action=='deconvolve':
        retrec=imager.getSummary();

    ## Close tools.
    imager.deleteTools()

    return retrec
