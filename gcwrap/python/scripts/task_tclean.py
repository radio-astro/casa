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
    timerange='',
    uvrange='',
    antenna='',
    scan='',
    observation='',
    intent='',

    ####### Image definition
    imagename='',
    imsize=[100,100],
    cellsize=['1.0arcsec','1.0arcsec'],
    phasecenter='J2000 19:59:28.500 +40.44.01.50',
    stokes='I',

    ## Spectral parameters
    specmode='mfs',
    reffreq='',
    nchan=1,
    start='',
    step='',
    frame='LSRK',
    veltype='',
    restfreq=[''],
    sysvel='',
    sysvelframe='',
    interpolation='',
    ## 
    ####### Gridding parameters
    gridmode='ft', 
    facets=1,

    wprojplanes=1,

    aterm=True,
    psterm=True,
    wbawp = True,
    conjbeams = True,
    cfcache = "",
    computepastep =360.0,
    rotatepastep =5.0,

    pblimit=0.01,
    normtype='flatnoise',

    outlierfile='',

    #### Weighting
    weighting='natural',
    robust=0.5,
    npixels=0,
    uvtaper=False,
    outertaper=[],
    innertaper=[],

    ##### Iteration control
    niter=0, 
    gain=0.1,
    threshold=0.0, 
    cycleniter=0, 
    cyclefactor=1.0,
    minpsffraction=0.1,
    maxpsffraction=0.8,
    interactive=False, 
    savemodel="none",

    ####### Deconvolution parameters
    deconvolver='hogbom',
    scales=[],
    ntaylorterms=1,
    restoringbeam=[],
    
    ####### State parameters
    parallel=False,
    clusterdef='',
    workdir=''):
    
    #####################################################
    #### Translate interface-specific parameters into ones used by ImagerParameters


    ### This is temporary..... get rid of it.
    if gridmode=='imagemosaic':
        mtype='imagemosaic'
    elif ntaylorterms>1:
        mtype='multiterm'
    else:
        mtype='default'


    ### Set the ftmachine.
    ftmachine = 'gridft'
    if gridmode=='standard':
        ftmachine='gridft'
    elif gridmode=='widefield':
        if wprojplanes>1:
            ftmachine='wprojectft'
    elif gridmode=='mosaic':
        ftmachine='mosaicft'
    elif gridmode=='imagemosaic':
        if wprojplanes>1:
            ftmachine='wprojectft'
    elif gridmode=='awproject':
        ftmachine='awprojectft'
    else:
        print 'Invalid gridmode'
        return

    ### Scratch column...
    usescratch=False
    readonly=True
    if savemodel=="virtual":
        readonly=False
    elif savemodel=="modelcolumn":
        usescratch=True
        readonly=False

    ### TEMP check. Remove
    if interactive==True:
        print "Interactive clean is not enabled yet."
        return;

    #####################################################

    #### Construct ImagerParameterss object
    from refimagerhelper import ImagerParameters

    # Put all parameters into dictionaries and check them. 
    paramList = ImagerParameters(
        msname =vis,
        field=field,
        spw=spw,
        timestr=timerange,
        uvdist=uvrange,
        antenna=antenna,
        scan=scan,
        obs=observation,
        state=intent,

        ### Image....
        imagename=imagename,
        #### Direction Image Coords
        imsize=imsize, 
        cellsize=cellsize, 
        phasecenter=phasecenter,
        stokes=stokes,

        ### Spectral Image Coords
        mode=specmode,
        reffreq=reffreq,
        nchan=nchan,
        start=start,
        step=step,
        frame=frame,
        veltype=veltype,
        restfreq=restfreq,
        sysvel=sysvel,
        sysvelframe=sysvelframe,
        interpolation=interpolation,

        ftmachine=ftmachine,
        facets=facets,

        wprojplanes=wprojplanes,
        
        ### Gridding....

        aterm=aterm,
        psterm=psterm,
        wbawp = wbawp,
        cfcache = cfcache,
        conjbeams = conjbeams,
        computepastep =computepastep,
        rotatepastep = rotatepastep,

        pblimit=pblimit,
        normtype=normtype,

        outlierfile=outlierfile,
        # startmodel=startmodel

        weighting=weighting,
        robust=robust,
        npixels=npixels,
        outertaper=outertaper,
        innertaper=innertaper,

        ### Deconvolution
        niter=niter,
        cycleniter=cycleniter,
        loopgain=gain,
        threshold=threshold,
        cyclefactor=cyclefactor,
        minpsffraction=minpsffraction, 
        maxpsffraction=maxpsffraction,
        interactive=interactive,

        algo=deconvolver,
        scales=scales,
        ntaylorterms=ntaylorterms,
        restoringbeam=restoringbeam,
        mtype=mtype,

        usescratch=usescratch,
        readonly=readonly,
        workdir=workdir
        )
    
    if paramList.checkParameters() == False:
        return False

    paramList.printParameters()

    pcube=False
    if parallel==True and nchan>1:
        pcube=True
        parallel=False

    ## Setup Imager objects, for different parallelization schemes.
    if parallel==False and pcube==False:
         imager = PySynthesisImager(params=paramList)
    elif parallel==True:
         imager = PyParallelContSynthesisImager(params=paramList,clusterdef=clusterdef)
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params=paramList,clusterdef=clusterdef)
    else:
         print 'Invalid parallel combination in doClean.'
         return False

    
    ## Init major cycle elements
    imager.initializeImagers()
    imager.initializeNormalizers()

    ## Init minor cycle elements
    if niter>0:
        imager.initializeDeconvolvers()
        imager.initializeIterationControl()

    ## Make PSF
    imager.makePSF()

    ## Make dirty image
    imager.runMajorCycle()

    ## Do deconvolution and iterations
    if niter>0 :
        while ( not imager.hasConverged() ):
            imager.runMinorCycle()
            imager.runMajorCycle()

    ## Restore images.
    if niter>0:
        imager.restoreImages()

    retrec={}
    if niter>0:
        retrec=imager.getSummary();

    ## Close tools.
    imager.deleteTools()

    return retrec

##################################################
#  Make separate tasks for all of these...
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
#
#
#    ## Init major cycle elements
#    if action=='makeresidual' or action=='makepsf' or action=='multicycle' or action=='setmodel':
#        imager.initializeImagers()
#        if action != 'setmodel':
#            imager.initializeNormalizers()
#
#    ## Init minor cycle elements
#    if action=='deconvolve' or action=='multicycle' or action=='restore':
#        imager.initializeDeconvolvers()
#        if action != 'restore':
#            imager.initializeIterationControl()
#
#    ## Make PSF
#    if action=='makeresidual' or action=='makepsf' or action=='multicycle':
#        imager.makePSF()
#
#    ## Make dirty image
#    if action=='makeresidual' or action=='multicycle':
#        imager.runMajorCycle()
#
#    ## Predict model (independent call)
#    if action=='setmodel':
#        imager.predictModel()
#
#    ## Do deconvolution and restore
#    if action=='multicycle' or action=='deconvolve':
#        while ( not imager.hasConverged() ):
#            imager.runMinorCycle()
#            if action=='multicycle':
#                imager.runMajorCycle()
#
#    ## Restore images.
#    if action=='multicycle' or action=='deconvolve' or action=='restore':
#        imager.restoreImages()
#
#
#    retrec={}
#    if action=='multicycle' or action=='deconvolve':
#        retrec=imager.getSummary();
#
#    ## Close tools.
#    imager.deleteTools()
#
#    return retrec
#
#######################################
