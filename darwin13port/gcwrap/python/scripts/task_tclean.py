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

##from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager, PyParallelDeconvolver, PyParallelImagerHelper,ImagerParameters

def tclean(vis='', field='', spw='',
           outlierfile='',
           imagename='',nchan=1,imsize=[1,1],
           ftmachine='ft', startmodel='',
           algo='test',
           niter=0, cycleniter=0, cyclefactor=1.0,
           minpsffraction=0.1,maxpsffraction=0.8,
           threshold=0.0, loopgain=0.1,
           usescratch=True, interactive=False, 
           clusterdef='', pmajor=False):

    from refimagerhelper import ImagerParameters

    #casalog.post('This is an empty task. It is meant only to build/test/maintain the interface for the refactored imaging code. When ready, it will be offered to users for testing.','WARN')

    # Put all parameters into dictionaries and check them. 
    paramList = ImagerParameters(casalog=casalog,\
                                     vis=vis,field=field,spw=spw,usescratch=usescratch,\
                                     outlierfile=outlierfile,\
                                     imagename=imagename,nchan=nchan,imsize=imsize,\
                                     cellsize=[10.0,10.0], phasecenter='19:59:28.500 +40.44.01.50',\
                                     ftmachine=ftmachine,startmodel=startmodel,weighting='natural',\
                                     algo=algo,\
                                     niter=niter,cycleniter=cycleniter,cyclefactor=cyclefactor,\
                                     minpsffraction=minpsffraction, maxpsffraction=maxpsffraction,\
                                     threshold=threshold,loopgain=loopgain,\
                                     interactive=interactive)

    if paramList.checkParameters() == False:
        return False

    paramList.printParameters()

    if pmajor==False:
        from refimagerhelper import PySynthesisImager
        imager = PySynthesisImager(params=paramList)
    else:
        from refimagerhelper import PyParallelContSynthesisImager
        imager = PyParallelContSynthesisImager(params=paramList,clusterdef=clusterdef)

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeParallelSync()
    imager.initializeIterationControl()

    ### Run it.
    imager.runMajorMinorLoops()

    imager.restoreImages()

    imager.plotReport(imager.getSummary());

    imager.deleteTools()
