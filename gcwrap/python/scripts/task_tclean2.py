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
import time;

from refimagerhelper import ImagerParameters

def tclean2(
    ####### Data Selection
    vis,#='',
    selectdata,
    field,#='',
    spw,#='',
    timerange,#='',
    uvrange,#='',
    antenna,#='',
    scan,#='',
    observation,#='',
    intent,#='',
    datacolumn,#='corrected',

    ####### Image definition
    imagename,#='',
    imsize,#=[100,100],
    cell,#=['1.0arcsec','1.0arcsec'],
    phasecenter,#='J2000 19:59:28.500 +40.44.01.50',
    stokes,#='I',
    projection,#='SIN',
    startmodel,#='',

    ## Spectral parameters
    specmode,#='mfs',
    reffreq,#='',
    nchan,#=1,
    start,#='',
    width,#='',
    outframe,#='LSRK',
    veltype,#='',
    restfreq,#=[''],
#    sysvel,#='',
#    sysvelframe,#='',
    interpolation,#='',
    ##
    ####### Gridding parameters
    gridder,#='ft',
    facets,#=1,
    chanchunks,#=1,

    wprojplanes,#=1,

    aterm,#=True,
    psterm,#=True,
    wbawp ,#= True,
    conjbeams ,#= True,
    cfcache ,#= "",
    computepastep ,#=360.0,
    rotatepastep ,#=360.0,

    pblimit,#=0.01,
    normtype,#='flatnoise',

    ####### Deconvolution parameters
    deconvolver,#='hogbom',
    scales,#=[],
    nterms,#=1,
    scalebias,#=0.6
    restoringbeam,#=[],

    ##### Outliers
    outlierfile,#='',

    ##### Weighting
    weighting,#='natural',
    robust,#=0.5,
    npixels,#=0,
    uvtaper,#=[''],


    ##### Iteration control
    niter,#=0,
    gain,#=0.1,
    threshold,#=0.0,
    cycleniter,#=0,
    cyclefactor,#=1.0,
    minpsffraction,#=0.1,
    maxpsffraction,#=0.8,
    interactive,#=False,
    usemask,#='user'
    mask,#='',
    pbmask,#=0.0
    maskthreshold,#=''
    maskresolution,#=''

    restart,#=True
    savemodel,#="none",
    makeimages,#='auto',

    calcres,#=True,
    calcpsf,#=True,
    restoremodel,#='auto',
    writepb,#=''auto',

    ####### State parameters
    ranks=[0]):

    #####################################################
    #### Sanity checks and controls
    #####################################################

    ### Move these checks elsewhere ?

    if specmode=='mfs' and nterms==1 and deconvolver == "mtmfs":
        casalog.post( "The MTMFS deconvolution algorithm (deconvolver='mtmfs') needs nterms>1.Please set nterms=2 (or more). ", "WARN", "task_tclean" )
        return

    if specmode!='mfs' and deconvolver=="mtmfs":
        casalog.post( "The MSMFS algorithm (deconvolver='mtmfs') applies only to specmode='mfs'.", "WARN", "task_tclean" )
        return

    #####################################################
    #### Construct ImagerParameters object
    #####################################################

    # mterm = True
    # dopointing = False
    # dopbcorr = True
    # workdir = ''
    # cflist = []

    try:
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
            datacolumn=datacolumn,

        ### Image....
        imagename=imagename,
            #### Direction Image Coords
        imsize=imsize,
            cell=cell,
            phasecenter=phasecenter,
            stokes=stokes,
            projection=projection,
            startmodel=startmodel,

        ### Spectral Image Coords
        specmode=specmode,
            reffreq=reffreq,
            nchan=nchan,
            start=start,
            width=width,
            outframe=outframe,
            veltype=veltype,
            restfreq=restfreq,
            sysvel='', #sysvel,
            sysvelframe='', #sysvelframe,
            interpolation=interpolation,

        gridder=gridder,
            #        ftmachine=ftmachine,
        facets=facets,
            chanchunks=chanchunks,

        wprojplanes=wprojplanes,

        ### Gridding....

        aterm=aterm,
            psterm=psterm,
            #mterm=mterm,
        wbawp = wbawp,
            cfcache = cfcache,
            #dopointing = dopointing,
            #dopbcorr = dopbcorr,
        conjbeams = conjbeams,
            computepastep =computepastep,
            rotatepastep = rotatepastep,

        pblimit=pblimit,
            normtype=normtype,

        outlierfile=outlierfile,
            restart=restart,

        weighting=weighting,
            robust=robust,
            npixels=npixels,
            uvtaper=uvtaper,

        ### Deconvolution
        niter=niter,
            cycleniter=cycleniter,
            loopgain=gain,
            threshold=threshold,
            cyclefactor=cyclefactor,
            minpsffraction=minpsffraction,
            maxpsffraction=maxpsffraction,
            interactive=interactive,

        deconvolver=deconvolver,
            scales=scales,
            nterms=nterms,
            scalebias=scalebias,
            restoringbeam=restoringbeam,

        usemask=usemask,
            mask=mask,
            pbmask=pbmask,
            maskthreshold=maskthreshold,
            maskresolution=maskresolution,

        savemodel=savemodel,
            #workdir=workdir,
            #cflist=cflist
        )

        cleanPars = {'savemodel' : savemodel,
                     'calcres' : calcres,
                     'calcpsf': calcpsf}

        retrec = parallelimager.clean(
            ranks,
            cleanPars,
            paramList.getSelPars(),
            paramList.getImagePars(),
            paramList.getGridPars(),
            paramList.getWeightPars(),
            paramList.getNormPars(),
            paramList.getDecPars(),
            paramList.getIterPars())

    except Exception as e:
        casalog.post('Exception from task_tclean2 : ' + str(e), "SEVERE", "task_tclean2")
        larg = list(e.args)
        larg[0] = 'Exception from task_tclean2 : ' + str(larg[0])
        e.args = tuple(larg)
        raise

    return retrec

##################################################
