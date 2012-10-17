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

def tclean(vis, field, spw,
           imagename,
           startmodel,
           niter,
           usescratch, clusterdef):

    #casalog.post('This is an empty task. It is meant only to build/test/maintain the interface for the refactored imaging code. When ready, it will be offered to users for testing.','WARN')

    # Put all parameters into dictionaries
    params={}
    params['dataselection']={'vis':vis, 'field':field, 'spw':spw, 'usescratch':usescratch}
    params['imagedefinition']={}
    params['imaging']={'startmodel':startmodel}
    params['deconvolution']={}
    params['iteration']={'niter':niter}
    params['other']={'clusterdef':clusterdef}

    # Instantiate the Imager class
    if len(clusterdef)==0:
        imager = PySynthesisImager(casalog,params)
    else:
        imager = ParallelPySynthesisImager(casalog,params)

    # Check input parameters (types, etc..)
    if not imager.checkParameters():
        return False

    # All setup steps
    imager.initialize()

    # Run major/minor cycle loops
    imager.runLoops()

    # Restore and Clean up.
    imager.finalize()

    # Save history - Params, Flux Cleaned, Peak Residual, Niter, TimeTaken, Image Names?
    return True


###################################################
##  Single-node imaging.
###################################################

class PySynthesisImager:
    """ Class to do imaging and deconvolution """

    def __init__(self,casalog,params):
        self.toolsi=None #gentools(['si'])[0]
        self.casalog = casalog
        self.params = params
        self.loopcontrols = {}
        self.casalog.origin('tclean')


    def checkParameters(self):
        self.casalog.origin('tclean.checkParameters')
        self.casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 

        errs = errs + self.checkAndFixSelectionPars( self.params['dataselection'] )

        ## If there are errors, print a message and exit.
        if len(errs) > 0:
            casalog.post('Parameter Errors : \n' + errs)
            return False
        return True

    def initialize(self, toolsi=None):
        self.casalog.origin('tclean.initialize')
        if toolsi==None:
            #self.toolsi = gentools(['si'])[0]
            self.toolsi = casac.synthesisimager() ##gentools(['si'])[0]
            toolsi = self.toolsi
        toolsi.selectdata(selpars=self.params['dataselection'])
        toolsi.defineimage(impars=self.params['imagedefinition'])
        toolsi.setupimaging(gridpars=self.params['imaging'])
        toolsi.setupdeconvolution(decpars=self.params['deconvolution'])
        toolsi.setupiteration(iterpars=self.params['iteration'])


    def runMajorCycle(self,toolsi=None):
        self.casalog.origin('tclean.runMajorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        self.loopcontrols.update( toolsi.runmajorcycle( self.loopcontrols ) ) 
        # In this prev statement, send in updated model to override default.


    def runMinorCycle(self, toolsi=None):
        self.casalog.origin('tclean.runMinorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        self.loopcontrols.update( toolsi.runminorcycle( self.loopcontrols ) )

    def runLoops(self):
        self.casalog.origin('tclean.runLoops')
        self.runMajorCycle()
        while not self.loopcontrols['stop']:  # Make the tool take loopcontrols as in/out ( not const ! )
            self.runMinorCycle()
            self.runMajorCycle()

    def finalize(self, toolsi=None):
        if toolsi==None:
            toolsi = self.toolsi
        toolsi.endloops( self.loopcontrols )
        #toolsi.done()

    ###### Start : Parameter-checking functions ##################
    def checkAndFixSelectionPars(self, selpars={} ):
        errs=""
        # vis, field, spw, etc must all be equal-length lists of strings.
        if not selpars.has_key('vis'):
            errs = errs + 'MS name(s) not specified'
        else:
            if type(selpars['vis'])==str:
                selpars['vis']=[selpars['vis']]
            nvis = len(selpars['vis'])
            selkeys = ['field','spw']
            for par in selkeys:
                if selpars.has_key(par):
                    if selpars[par]=='':
                        selpars[par] = []
                        for it in range(0,nvis):
                            selpars[par].append('')
                    if type(selpars[par])==str:
                        selpars[par]=[selpars[par]]
                    if len(selpars[par]) != nvis:
                        errs = errs + "Selection for " + par + " must be a list of " + str(nvis) + " selection strings\n"
                else:
                    errs = errs + "Selection for " + par + " is unspecified\n"

        return errs

    ###### End : Parameter-checking functions ##################



###################################################
##  Parallel imaging.
###################################################

class ParallelPySynthesisImager(PySynthesisImager):
    """ Class to do imaging and deconvolution, with major cycles distributed across cluster nodes """

    def __init__(self,casalog,params):
        PySynthesisImager.__init__(self,casalog,params)
        # Read params['other']['clusterdef'] to decide chunking.
        self.nchunks = len(params['other']['clusterdef'])
        # Initialize a list of synthesisimager tools
        self.toollist = []
        for ch in range(0,self.nchunks):
            #self.toollist.append( gentools(['si'])[0]  )
            self.toollist.append( casac.synthesisimager() ) ## gentools(['si'])[0]  )


    def initialize(self):
        selpars = copy.deepcopy( self.params['dataselection'] )
        for ch in range(0,self.nchunks):
            casalog.origin('parallel.tclean.runMinorCycle')
            casalog.post('Initialize for chunk '+str(ch))

            for dat in range(0,len( selpars['spw'] )):
                self.params['dataselection']['spw'][dat] = selpars['spw'][dat] + ':'+str(ch)

            PySynthesisImager.initialize(self, self.toollist[ch] )
        self.params['dataselection'] = copy.deepcopy(selpars)

    def runMajorCycle(self):
        lcontrols = dict(self.loopcontrols)
        for ch in range(0,self.nchunks):
            self.loopcontrols = dict(lcontrols)
            PySynthesisImager.runMajorCycle(self, self.toollist[ch] )  # Send in updated model as startmodel..
        self.gatherImages()

    def runMinorCycle(self):
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Set combined images for the minor cycle')
        PySynthesisImager.runMinorCycle(self, self.toollist[0] )  # Use the full list for spectral-cube deconv.
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Mark updated model as input to all major cycles') 

    def finalize(self):
        self.toollist[0].endloops( self.loopcontrols )
        #for ch in range(0,self.nchunks):
        #    self.toollist[ch].done()

    def gatherImages(self):
        casalog.origin('parallel.tclean.gatherimages')
        casalog.post('Gather images from all chunks')

